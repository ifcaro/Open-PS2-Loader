#include "include/opl.h"
#include "include/lang.h"
#include "include/gui.h"
#include "include/supportbase.h"
#include "include/bdmsupport.h"
#include "include/util.h"
#include "include/themes.h"
#include "include/textures.h"
#include "include/ioman.h"
#include "include/system.h"
#include "include/extern_irx.h"
#include "include/cheatman.h"
#include "modules/iopcore/common/cdvd_config.h"

#define NEWLIB_PORT_AWARE
#include <fileXio_rpc.h> // fileXioIoctl, fileXioDevctl

static char bdmPrefix[40]; // Contains the full path to the folder where all the games are.
static int bdmULSizePrev = -2;
static time_t bdmModifiedCDPrev;
static time_t bdmModifiedDVDPrev;
static int bdmGameCount = 0;
static base_game_info_t *bdmGames;
static char bdmDriver[5];

static int fireWireModLoaded = 0;
static int mx4sioModLoaded = 0;

// forward declaration
static item_list_t bdmGameList;

// Identifies the partition that the specified file is stored on and generates a full path to it.
int bdmFindPartition(char *target, const char *name, int write)
{
    int i, fd;
    char path[256];

    for (i = 0; i < MAX_BDM_DEVICES; i++) {
        if (gBDMPrefix[0] != '\0')
            sprintf(path, "mass%d:%s/%s", i, gBDMPrefix, name);
        else
            sprintf(path, "mass%d:%s", i, name);
        if (write)
            fd = open(path, O_WRONLY | O_TRUNC | O_CREAT, 0666);
        else
            fd = open(path, O_RDONLY);

        if (fd >= 0) {
            if (gBDMPrefix[0] != '\0')
                sprintf(target, "mass%d:%s/", i, gBDMPrefix);
            else
                sprintf(target, "mass%d:", i);
            close(fd);
            return 1;
        }
    }

    // default to first partition (for themes, ...)
    if (gBDMPrefix[0] != '\0')
        sprintf(target, "mass0:%s/", gBDMPrefix);
    else
        sprintf(target, "mass0:");
    return 0;
}

static unsigned int BdmGeneration = 0;

static void bdmEventHandler(void *packet, void *opt)
{
    BdmGeneration++;
}

static void bdmLoadBlockDeviceModules(void)
{
    if (gEnableFW && !fireWireModLoaded) {
        // Load iLink Block Device drivers
        sysLoadModuleBuffer(&iLinkman_irx, size_iLinkman_irx, 0, NULL);
        sysLoadModuleBuffer(&IEEE1394_bd_irx, size_IEEE1394_bd_irx, 0, NULL);

        fireWireModLoaded = 1;
    }

    if (gEnableMX4SIO && !mx4sioModLoaded) {
        // Load MX4SIO Block Device drivers
        sysLoadModuleBuffer(&mx4sio_bd_irx, size_mx4sio_bd_irx, 0, NULL);

        mx4sioModLoaded = 1;
    }
}

void bdmLoadModules(void)
{
    LOG("BDMSUPPORT LoadModules\n");

    // Load Block Device Manager (BDM)
    sysLoadModuleBuffer(&bdm_irx, size_bdm_irx, 0, NULL);

    // Load VFAT (mass:) driver
    sysLoadModuleBuffer(&bdmfs_vfat_irx, size_bdmfs_vfat_irx, 0, NULL);

    // Load USB Block Device drivers
    sysLoadModuleBuffer(&usbd_irx, size_usbd_irx, 0, NULL);
    sysLoadModuleBuffer(&usbmass_bd_irx, size_usbmass_bd_irx, 0, NULL);

    // Load Optional Block Device drivers
    bdmLoadBlockDeviceModules();

    sysLoadModuleBuffer(&bdmevent_irx, size_bdmevent_irx, 0, NULL);
    SifAddCmdHandler(0, &bdmEventHandler, NULL);

    LOG("BDMSUPPORT Modules loaded\n");
}

void bdmInit(void)
{
    LOG("BDMSUPPORT Init\n");
    bdmULSizePrev = -2;
    bdmModifiedCDPrev = 0;
    bdmModifiedDVDPrev = 0;
    bdmGameCount = 0;
    bdmGames = NULL;
    configGetInt(configGetByType(CONFIG_OPL), "usb_frames_delay", &bdmGameList.delay);
    bdmGameList.enabled = 1;
}

item_list_t *bdmGetObject(int initOnly)
{
    if (initOnly && !bdmGameList.enabled)
        return NULL;
    return &bdmGameList;
}

static int bdmNeedsUpdate(void)
{
    char path[256];
    static unsigned int OldGeneration = 0;
    static unsigned char ThemesLoaded = 0;
    static unsigned char LanguagesLoaded = 0;
    int result = 0;
    struct stat st;

    ioPutRequest(IO_CUSTOM_SIMPLEACTION, &bdmLoadBlockDeviceModules);

    if (bdmULSizePrev != -2 && OldGeneration == BdmGeneration)
        return 0;
    OldGeneration = BdmGeneration;

    bdmFindPartition(bdmPrefix, "ul.cfg", 0);

    sprintf(path, "%sCD", bdmPrefix);
    if (stat(path, &st) != 0)
        st.st_mtime = 0;
    if (bdmModifiedCDPrev != st.st_mtime) {
        bdmModifiedCDPrev = st.st_mtime;
        result = 1;
    }

    sprintf(path, "%sDVD", bdmPrefix);
    if (stat(path, &st) != 0)
        st.st_mtime = 0;
    if (bdmModifiedDVDPrev != st.st_mtime) {
        bdmModifiedDVDPrev = st.st_mtime;
        result = 1;
    }

    if (!sbIsSameSize(bdmPrefix, bdmULSizePrev))
        result = 1;

    // update Themes
    if (!ThemesLoaded) {
        sprintf(path, "%sTHM", bdmPrefix);
        if (thmAddElements(path, "/", 1) > 0)
            ThemesLoaded = 1;
    }

    // update Languages
    if (!LanguagesLoaded) {
        sprintf(path, "%sLNG", bdmPrefix);
        if (lngAddLanguages(path, "/", bdmGameList.mode) > 0)
            LanguagesLoaded = 1;
    }

    sbCreateFolders(bdmPrefix, 1);

    return result;
}

static int bdmUpdateGameList(void)
{
    sbReadList(&bdmGames, bdmPrefix, &bdmULSizePrev, &bdmGameCount);
    return bdmGameCount;
}

static int bdmGetGameCount(void)
{
    return bdmGameCount;
}

static void *bdmGetGame(int id)
{
    return (void *)&bdmGames[id];
}

static char *bdmGetGameName(int id)
{
    return bdmGames[id].name;
}

static int bdmGetGameNameLength(int id)
{
    return ((bdmGames[id].format != GAME_FORMAT_USBLD) ? ISO_GAME_NAME_MAX + 1 : UL_GAME_NAME_MAX + 1);
}

static char *bdmGetGameStartup(int id)
{
    return bdmGames[id].startup;
}

static void bdmDeleteGame(int id)
{
    sbDelete(&bdmGames, bdmPrefix, "/", bdmGameCount, id);
    bdmULSizePrev = -2;
}

static void bdmRenameGame(int id, char *newName)
{
    sbRename(&bdmGames, bdmPrefix, "/", bdmGameCount, id, newName);
    bdmULSizePrev = -2;
}

static void bdmLaunchGame(int id, config_set_t *configSet)
{
    int i, fd, index, compatmask = 0;
    int EnablePS2Logo = 0;
    int result;
    unsigned int start;
    unsigned int startCluster;
    char partname[256], filename[32];
    base_game_info_t *game = &bdmGames[id];
    struct cdvdman_settings_bdm *settings;
    u32 layer1_start, layer1_offset;
    unsigned short int layer1_part;

    char vmc_name[32], vmc_path[256], have_error = 0;
    int vmc_id, size_mcemu_irx = 0;
    bdm_vmc_infos_t bdm_vmc_infos;
    vmc_superblock_t vmc_superblock;

    for (vmc_id = 0; vmc_id < 2; vmc_id++) {
        memset(&bdm_vmc_infos, 0, sizeof(bdm_vmc_infos_t));
        configGetVMC(configSet, vmc_name, sizeof(vmc_name), vmc_id);
        if (vmc_name[0]) {
            have_error = 1;
            if (sysCheckVMC(bdmPrefix, "/", vmc_name, 0, &vmc_superblock) > 0) {
                bdm_vmc_infos.flags = vmc_superblock.mc_flag & 0xFF;
                bdm_vmc_infos.flags |= 0x100;
                bdm_vmc_infos.specs.page_size = vmc_superblock.page_size;
                bdm_vmc_infos.specs.block_size = vmc_superblock.pages_per_block;
                bdm_vmc_infos.specs.card_size = vmc_superblock.pages_per_cluster * vmc_superblock.clusters_per_card;

                sprintf(vmc_path, "%sVMC/%s.bin", bdmPrefix, vmc_name);

                fd = open(vmc_path, O_RDONLY);
                if (fd >= 0) {
                    if ((start = (unsigned int)fileXioIoctl(fd, USBMASS_IOCTL_GET_LBA, vmc_path)) != 0 && (startCluster = (unsigned int)fileXioIoctl(fd, USBMASS_IOCTL_GET_CLUSTER, vmc_path)) != 0) {

                        // Check VMC cluster chain for fragmentation (write operation can cause damage to the filesystem).
                        if (fileXioIoctl(fd, USBMASS_IOCTL_CHECK_CHAIN, "") == 1) {
                            LOG("USBSUPPORT Cluster Chain OK\n");
                            have_error = 0;
                            bdm_vmc_infos.active = 1;
                            bdm_vmc_infos.start_sector = start;
                            LOG("USBSUPPORT VMC slot %d start: 0x%X\n", vmc_id, start);
                        } else {
                            LOG("USBSUPPORT Cluster Chain NG\n");
                            have_error = 2;
                        }
                    }

                    close(fd);
                }
            }
        }

        if (have_error) {
            char error[256];
            if (have_error == 2) // VMC file is fragmented
                snprintf(error, sizeof(error), _l(_STR_ERR_VMC_FRAGMENTED_CONTINUE), vmc_name, (vmc_id + 1));
            else
                snprintf(error, sizeof(error), _l(_STR_ERR_VMC_CONTINUE), vmc_name, (vmc_id + 1));
            if (!guiMsgBox(error, 1, NULL)) {
                return;
            }
        }

        for (i = 0; i < size_bdm_mcemu_irx; i++) {
            if (((u32 *)&bdm_mcemu_irx)[i] == (0xC0DEFAC0 + vmc_id)) {
                if (bdm_vmc_infos.active)
                    size_mcemu_irx = size_bdm_mcemu_irx;
                memcpy(&((u32 *)&bdm_mcemu_irx)[i], &bdm_vmc_infos, sizeof(bdm_vmc_infos_t));
                break;
            }
        }
    }

    void **irx = &bdm_cdvdman_irx;
    int irx_size = size_bdm_cdvdman_irx;
    compatmask = sbPrepare(game, configSet, irx_size, irx, &index);
    settings = (struct cdvdman_settings_bdm *)((u8 *)irx + index);
    for (i = 0; i < game->parts; i++) {
        sbCreatePath(game, partname, bdmPrefix, "/", i);
        fd = open(partname, O_RDONLY);
        if (fd >= 0) {
            int *pBDMDriver = (int *)bdmDriver;
            *pBDMDriver = fileXioIoctl(fd, USBMASS_IOCTL_GET_DRIVERNAME, "");
            LOG("bdmDriver=%s\n", bdmDriver);

            settings->LBAs[i] = fileXioIoctl(fd, USBMASS_IOCTL_GET_LBA, "");
            if (fileXioIoctl(fd, USBMASS_IOCTL_CHECK_CHAIN, "") != 1) {
                close(fd);
                // Game is fragmented. Do not continue.
                if (settings != NULL)
                    sbUnprepare(&settings->common);

                guiMsgBox(_l(_STR_ERR_FRAGMENTED), 0, NULL);
                return;
            }

            if ((gPS2Logo) && (i == 0))
                EnablePS2Logo = CheckPS2Logo(fd, 0);

            close(fd);
        } else {
            // Unable to open part of the game. Do not continue.
            if (settings != NULL)
                sbUnprepare(&settings->common);
            guiMsgBox(_l(_STR_ERR_FILE_INVALID), 0, NULL);
            return;
        }
    }

    // Initialize layer 1 information.
    sbCreatePath(game, partname, bdmPrefix, "/", 0);
    layer1_start = sbGetISO9660MaxLBA(partname);

    switch (game->format) {
        case GAME_FORMAT_USBLD:
            layer1_part = layer1_start / 0x80000;
            layer1_offset = layer1_start % 0x80000;
            sbCreatePath(game, partname, bdmPrefix, "/", layer1_part);
            break;
        default: // Raw ISO9660 disc image; one part.
            layer1_part = 0;
            layer1_offset = layer1_start;
    }

    if (sbProbeISO9660(partname, game, layer1_offset) != 0) {
        layer1_start = 0;
        LOG("DVD detected.\n");
    } else {
        layer1_start -= 16;
        LOG("DVD-DL layer 1 @ part %u sector 0x%lx.\n", layer1_part, layer1_offset);
    }
    settings->common.layer1_start = layer1_start;

    if ((result = sbLoadCheats(bdmPrefix, game->startup)) < 0) {
        switch (result) {
            case -ENOENT:
                guiWarning(_l(_STR_NO_CHEATS_FOUND), 10);
                break;
            default:
                guiWarning(_l(_STR_ERR_CHEATS_LOAD_FAILED), 10);
        }
    }

    if (gRememberLastPlayed) {
        configSetStr(configGetByType(CONFIG_LAST), "last_played", game->startup);
        saveConfig(CONFIG_LAST, 0);
    }

    if (configGetStrCopy(configSet, CONFIG_ITEM_ALTSTARTUP, filename, sizeof(filename)) == 0)
        strcpy(filename, game->startup);
    deinit(NO_EXCEPTION, BDM_MODE); // CAREFUL: deinit will call bdmCleanUp, so bdmGames/game will be freed

    if (!strcmp(bdmDriver, "usb"))
        sysLaunchLoaderElf(filename, "BDM_USB_MODE", irx_size, irx, size_mcemu_irx, &bdm_mcemu_irx, EnablePS2Logo, compatmask);
    else if (!strcmp(bdmDriver, "sd") && strlen(bdmDriver) == 2)
        sysLaunchLoaderElf(filename, "BDM_ILK_MODE", irx_size, irx, size_mcemu_irx, &bdm_mcemu_irx, EnablePS2Logo, compatmask);
    else if (!strcmp(bdmDriver, "sdc") && strlen(bdmDriver) == 3)
        sysLaunchLoaderElf(filename, "BDM_M4S_MODE", irx_size, irx, size_mcemu_irx, &bdm_mcemu_irx, EnablePS2Logo, compatmask);
}

static config_set_t *bdmGetConfig(int id)
{
    return sbPopulateConfig(&bdmGames[id], bdmPrefix, "/");
}

static int bdmGetImage(char *folder, int isRelative, char *value, char *suffix, GSTEXTURE *resultTex, short psm)
{
    char path[256];
    if (isRelative)
        snprintf(path, sizeof(path), "%s%s/%s_%s", bdmPrefix, folder, value, suffix);
    else
        snprintf(path, sizeof(path), "%s%s_%s", folder, value, suffix);
    return texDiscoverLoad(resultTex, path, -1, psm);
}

// This may be called, even if bdmInit() was not.
static void bdmCleanUp(int exception)
{
    if (bdmGameList.enabled) {
        LOG("USBSUPPORT CleanUp\n");

        free(bdmGames);

        //      if ((exception & UNMOUNT_EXCEPTION) == 0)
        //          ...
    }
}

// This may be called, even if bdmInit() was not.
static void bdmShutdown(void)
{
    if (bdmGameList.enabled) {
        LOG("USBSUPPORT Shutdown\n");

        free(bdmGames);
    }

    // As required by some (typically 2.5") HDDs, issue the SCSI STOP UNIT command to avoid causing an emergency park.
    fileXioDevctl("mass:", USBMASS_DEVCTL_STOP_ALL, NULL, 0, NULL, 0);
}

static int bdmCheckVMC(char *name, int createSize)
{
    return sysCheckVMC(bdmPrefix, "/", name, createSize, NULL);
}

static void bdmGetAppsPath(char *path, int max)
{
    snprintf(path, max, "%sAPPS", bdmPrefix);
}

static void bdmGetLegacyAppsPath(char *path, int max)
{
    snprintf(path, max, "%sconf_apps.cfg", bdmPrefix);
}

static void bdmGetLegacyAppsInfo(char *path, int max, char *name)
{
    snprintf(path, max, "%sCFG/%s.cfg", bdmPrefix, name);
}

static item_list_t bdmGameList = {
    BDM_MODE, 2, 0, 0, MENU_MIN_INACTIVE_FRAMES, BDM_MODE_UPDATE_DELAY, "BDM Games", _STR_BDM_GAMES, &bdmGetAppsPath, &bdmGetLegacyAppsPath, &bdmGetLegacyAppsInfo, &bdmInit, &bdmNeedsUpdate,
    &bdmUpdateGameList, &bdmGetGameCount, &bdmGetGame, &bdmGetGameName, &bdmGetGameNameLength, &bdmGetGameStartup, &bdmDeleteGame, &bdmRenameGame,
    &bdmLaunchGame, &bdmGetConfig, &bdmGetImage, &bdmCleanUp, &bdmShutdown, &bdmCheckVMC, USB_ICON};
