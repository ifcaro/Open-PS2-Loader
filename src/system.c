/*
  Copyright 2009, Ifcaro
  Licenced under Academic Free License version 3.0
  Review OpenUsbLd README & LICENSE files for further details.
*/

#ifdef __DECI2_DEBUG
#include <iopcontrol_special.h>
#endif

#include "include/opl.h"
#include "include/gui.h"
#include "include/ethsupport.h"
#include "include/hddsupport.h"
#include "include/util.h"
#include "include/pad.h"
#include "include/system.h"
#include "include/ioman.h"
#include "include/ioprp.h"
#include "include/bdmsupport.h"
#include "include/OSDHistory.h"
#include "include/renderman.h"
#include "include/extern_irx.h"
#include "../ee_core/include/modules.h"

#include "include/pggsm.h"
#include "include/cheatman.h"

#ifdef PADEMU
#include <libds34bt.h>
#include <libds34usb.h>
#endif

#define NEWLIB_PORT_AWARE
#include <fileXio_rpc.h> // fileXioInit, fileXioExit, fileXioDevctl

typedef struct
{
    char VMC_filename[1024];
    int VMC_size_mb;
    int VMC_blocksize;
    int VMC_thread_priority;
    int VMC_card_slot;
} createVMCparam_t;

extern void *eecore_elf;
extern int size_eecore_elf;

extern void *elfldr_elf;
extern int size_elfldr_elf;

extern unsigned char IOPRP_img[];
extern unsigned int size_IOPRP_img;

extern unsigned char eesync_irx[];
extern unsigned int size_eesync_irx;

#define MAX_MODULES 32
static void *g_sysLoadedModBuffer[MAX_MODULES];

#define ELF_MAGIC   0x464c457f
#define ELF_PT_LOAD 1

typedef struct
{
    u8 ident[16]; // struct definition for ELF object header
    u16 type;
    u16 machine;
    u32 version;
    u32 entry;
    u32 phoff;
    u32 shoff;
    u32 flags;
    u16 ehsize;
    u16 phentsize;
    u16 phnum;
    u16 shentsize;
    u16 shnum;
    u16 shstrndx;
} elf_header_t;

typedef struct
{
    u32 type; // struct definition for ELF program section header
    u32 offset;
    void *vaddr;
    u32 paddr;
    u32 filesz;
    u32 memsz;
    u32 flags;
    u32 align;
} elf_pheader_t;

void guiWarning(const char *text, int count);
void guiEnd();
void menuEnd();
void lngEnd();
void thmEnd();
void rmEnd();

static void poweroffHandler(void *arg);

int sysLoadModuleBuffer(void *buffer, int size, int argc, char *argv)
{

    int i, id, ret, index = 0;

    // check we have not reached MAX_MODULES
    for (i = 0; i < MAX_MODULES; i++) {
        if (g_sysLoadedModBuffer[i] == NULL) {
            index = i;
            break;
        }
    }
    if (i == MAX_MODULES)
        return -1;

    // check if the module was already loaded
    for (i = 0; i < MAX_MODULES; i++) {
        if (g_sysLoadedModBuffer[i] == buffer) {
            return 0;
        }
    }

    // load the module
    id = SifExecModuleBuffer(buffer, size, argc, argv, &ret);
    if ((id < 0) || (ret))
        return -2;

    // add the module to the list
    g_sysLoadedModBuffer[index] = buffer;

    return 0;
}

#define OPL_SIF_CMD_BUFF_SIZE 1
static SifCmdHandlerData_t OplSifCmdbuffer[OPL_SIF_CMD_BUFF_SIZE];
static unsigned char dev9Initialized = 0, dev9Loaded = 0, dev9InitCount = 0;

void sysInitDev9(void)
{
    int ret;

    if (!dev9Initialized) {
        ret = sysLoadModuleBuffer(&ps2dev9_irx, size_ps2dev9_irx, 0, NULL);
        dev9Loaded = (ret == 0); // DEV9.IRX must have successfully loaded and returned RESIDENT END.
        dev9Initialized = 1;
    }

    dev9InitCount++;
}

void sysShutdownDev9(void)
{
    if (dev9InitCount > 0) {
        --dev9InitCount;

        if (dev9InitCount == 0) { /* Switch off DEV9 once nothing needs it. */
            if (dev9Loaded) {
                while (fileXioDevctl("dev9x:", DDIOC_OFF, NULL, 0, NULL, 0) < 0) {
                };
            }
        }
    }
}

void sysReset(int modload_mask)
{
#ifdef PADEMU
    ds34usb_reset();
    ds34bt_reset();
#endif
    fileXioExit();
    SifExitIopHeap();
    SifLoadFileExit();
    SifExitRpc();

    SifInitRpc(0);

#ifdef _DTL_T10000
    while (!SifIopReset("rom0:UDNL", 0))
        ;
#else
#ifdef __DECI2_DEBUG
    while (!SifIopRebootBuffer(&deci2_img, size_deci2_img))
        ;
#else
    while (!SifIopReset("", 0))
        ;
#endif
#endif

    dev9Initialized = 0;
    while (!SifIopSync())
        ;

    SifInitRpc(0);
    SifSetCmdBuffer(OplSifCmdbuffer, OPL_SIF_CMD_BUFF_SIZE);
    sceCdInit(SCECdINoD);

    // init loadfile & iopheap services
    SifLoadFileInit();
    SifInitIopHeap();

    // apply sbv patches
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();

    // clears modules list
    memset((void *)&g_sysLoadedModBuffer[0], 0, MAX_MODULES * 4);

    // load modules
    sysLoadModuleBuffer(&iomanx_irx, size_iomanx_irx, 0, NULL);
    sysLoadModuleBuffer(&filexio_irx, size_filexio_irx, 0, NULL);

    sysLoadModuleBuffer(&sio2man_irx, size_sio2man_irx, 0, NULL);

    if (modload_mask & SYS_LOAD_MC_MODULES) {
        sysLoadModuleBuffer(&mcman_irx, size_mcman_irx, 0, NULL);
        sysLoadModuleBuffer(&mcserv_irx, size_mcserv_irx, 0, NULL);
    }

    sysLoadModuleBuffer(&padman_irx, size_padman_irx, 0, NULL);

    sysLoadModuleBuffer(&poweroff_irx, size_poweroff_irx, 0, NULL);

    if (modload_mask & SYS_LOAD_USB_MODULES) {
        bdmLoadModules();
    }
    if (modload_mask & SYS_LOAD_ISOFS_MODULE) {
        sysLoadModuleBuffer(&isofs_irx, size_isofs_irx, 0, NULL);
    }

    sysLoadModuleBuffer(&genvmc_irx, size_genvmc_irx, 0, NULL);

    sysLoadModuleBuffer(&libsd_irx, size_libsd_irx, 0, NULL);
    sysLoadModuleBuffer(&audsrv_irx, size_audsrv_irx, 0, NULL);

#ifdef PADEMU
    int ds3pads = 1; // only one pad enabled

    ds34usb_deinit();
    ds34bt_deinit();

    if (modload_mask & SYS_LOAD_USB_MODULES) {
        sysLoadModuleBuffer(&ds34usb_irx, size_ds34usb_irx, 4, (char *)&ds3pads);
        sysLoadModuleBuffer(&ds34bt_irx, size_ds34bt_irx, 4, (char *)&ds3pads);

        ds34usb_init();
        ds34bt_init();
    }
#endif

    fileXioInit();
    poweroffInit();
    poweroffSetCallback(&poweroffHandler, NULL);
}

static void poweroffHandler(void *arg)
{
    sysPowerOff();
}

void sysPowerOff(void)
{
    deinit(NO_EXCEPTION, IO_MODE_SELECTED_NONE);
    poweroffShutdown();
}

static unsigned int crctab[0x400];

unsigned int USBA_crc32(const char *string)
{
    int crc, table, count, byte;

    for (table = 0; table < 256; table++) {
        crc = table << 24;

        for (count = 8; count > 0; count--) {
            if (crc < 0)
                crc = crc << 1;
            else
                crc = (crc << 1) ^ 0x04C11DB7;
        }
        crctab[255 - table] = crc;
    }

    do {
        byte = string[count++];
        crc = crctab[byte ^ ((crc >> 24) & 0xFF)] ^ ((crc << 8) & 0xFFFFFF00);
    } while (string[count - 1] != 0);

    return crc;
}

int sysGetDiscID(char *hexDiscID)
{
    u8 key[16];

    if (sceCdStatus() == SCECdErOPENS) // If tray is open, error
        return -1;

    while (sceCdGetDiskType() == SCECdDETCT) {
        ;
    }
    if (sceCdGetDiskType() == SCECdNODISC)
        return -1;

    sceCdDiskReady(0);
    LOG("SYSTEM Disc drive is ready\n");
    int cdmode = sceCdGetDiskType();
    if (cdmode == SCECdNODISC)
        return -1;

    if ((cdmode != SCECdPS2DVD) && (cdmode != SCECdPS2CD) && (cdmode != SCECdPS2CDDA)) {
        sceCdStop();
        sceCdSync(0);
        LOG("SYSTEM Disc stopped, Disc is not ps2 disc!\n");
        return -2;
    }

    LOG("SYSTEM Disc standby\n");
    sceCdStandby();
    sceCdSync(0);

    LOG("SYSTEM Disc read key\n");
    if (sceCdReadKey(0, 0, 0x4b, key) == 0) {
        LOG("SYSTEM Cannot read CD/DVD key.\n");
        sceCdStop();
        sceCdSync(0);
        LOG("SYSTEM Disc stopped\n");
        return -3;
    }

    sceCdStop();

    // convert to hexadecimal string
    snprintf(hexDiscID, 15, "%02X %02X %02X %02X %02X", key[10], key[11], key[12], key[13], key[14]);
    LOG("SYSTEM PS2 Disc ID = %s\n", hexDiscID);

    sceCdSync(0);
    LOG("SYSTEM Disc stopped\n");

    return 1;
}

void sysExecExit(void)
{
    // Deinitialize without shutting down active devices.
    deinit(NO_EXCEPTION, IO_MODE_SELECTED_ALL);
    Exit(0);
}

// Module bits
#define CORE_IRX_USB   0x01
#define CORE_IRX_ETH   0x02
#define CORE_IRX_SMB   0x04
#define CORE_IRX_HDD   0x08
#define CORE_IRX_VMC   0x10
#define CORE_IRX_DEBUG 0x20
#define CORE_IRX_DECI2 0x40
#define CORE_IRX_ILINK 0x80

typedef struct
{
    char *game;
    char *mode;
    void *module;
    int *module_size;
} patchlist_t;

// Blank string for mode = all modes.
static const patchlist_t iop_patch_list[] = {
    {"SLUS_205.61", "", &iremsndpatch_irx, &size_iremsndpatch_irx},     // Disaster Report
    {"SLES_513.01", "", &iremsndpatch_irx, &size_iremsndpatch_irx},     // SOS: The Final Escape
    {"SLPS_251.13", "", &iremsndpatch_irx, &size_iremsndpatch_irx},     // Zettai Zetsumei Toshi
    {"SLES_535.08", "", &apemodpatch_irx, &size_apemodpatch_irx},       // Ultimate Pro Pinball
    {"SLUS_204.13", "", &f2techioppatch_irx, &size_f2techioppatch_irx}, // Shadow Man: 2econd Coming (NTSC-U/C)
    {"SLES_504.46", "", &f2techioppatch_irx, &size_f2techioppatch_irx}, // Shadow Man: 2econd Coming (PAL)
    {"SLES_506.08", "", &f2techioppatch_irx, &size_f2techioppatch_irx}, // Shadow Man: 2econd Coming (PAL German)
    {NULL, NULL, NULL, NULL},                                           // Terminator
};

static unsigned int addIopPatch(const char *mode_str, const char *startup, irxptr_t *tab)
{
    const patchlist_t *p;
    int i;

    for (i = 0; iop_patch_list[i].game != NULL; i++) {
        p = &iop_patch_list[i];

        if (!strcmp(p->game, startup) && (p->mode[0] == '\0' || !strcmp(p->mode, startup))) {
            tab->info = (*(p->module_size)) | SET_OPL_MOD_ID(OPL_MODULE_ID_IOP_PATCH);
            tab->ptr = (void *)p->module;
            return 1;
        }
    }

    return 0;
}

typedef struct
{
    char *game;
    void *addr;
} modStorageSetting_t;

static const modStorageSetting_t mod_storage_location_list[] = {
    {"SLUS_209.77", (void *)0x01fc7000}, // Virtua Quest
    {"SLPM_656.32", (void *)0x01fc7000}, // Virtua Fighter Cyber Generation: Judgment Six No Yabou
    {NULL, NULL},                        // Terminator
};

static void *GetModStorageLocation(const char *startup, unsigned compatFlags)
{
    const modStorageSetting_t *p;
    int i;

    for (i = 0; mod_storage_location_list[i].game != NULL; i++) {
        p = &mod_storage_location_list[i];

        if (!strcmp(p->game, startup)) {
            return p->addr;
        }
    }

    return ((void *)OPL_MOD_STORAGE);
}

static unsigned int sendIrxKernelRAM(const char *startup, const char *mode_str, unsigned int modules, void *ModuleStorage, int size_cdvdman_irx, void **cdvdman_irx, int size_mcemu_irx, void **mcemu_irx)
{ // Send IOP modules that core must use to Kernel RAM
    irxtab_t *irxtable;
    irxptr_t *irxptr_tab;
    void *irxptr, *ioprp_image;
    int i, modcount;
    unsigned int curIrxSize, size_ioprp_image, total_size;

    if (!strcmp(mode_str, "BDM_USB_MODE"))
        modules |= CORE_IRX_USB;
    else if (!strcmp(mode_str, "BDM_ILK_MODE"))
        modules |= CORE_IRX_ILINK;
    else if (!strcmp(mode_str, "ETH_MODE"))
        modules |= CORE_IRX_ETH | CORE_IRX_SMB;
    else
        modules |= CORE_IRX_HDD;

    irxtable = (irxtab_t *)ModuleStorage;
    irxptr_tab = (irxptr_t *)((unsigned char *)irxtable + sizeof(irxtab_t));
    size_ioprp_image = size_IOPRP_img + size_cdvdman_irx + size_cdvdfsv_irx + size_eesync_irx + 256;
    LOG("IOPRP image size calculated: %d\n", size_ioprp_image);
    ioprp_image = malloc(size_ioprp_image);
    size_ioprp_image = patch_IOPRP_image(ioprp_image, cdvdman_irx, size_cdvdman_irx);
    LOG("IOPRP image size actual:     %d\n", size_ioprp_image);

    modcount = 0;
    // Basic modules
    irxptr_tab[modcount].info = size_udnl_irx | SET_OPL_MOD_ID(OPL_MODULE_ID_UDNL);
    irxptr_tab[modcount++].ptr = (void *)&udnl_irx;
    irxptr_tab[modcount].info = size_ioprp_image | SET_OPL_MOD_ID(OPL_MODULE_ID_IOPRP);
    irxptr_tab[modcount++].ptr = ioprp_image;
    irxptr_tab[modcount].info = size_imgdrv_irx | SET_OPL_MOD_ID(OPL_MODULE_ID_IMGDRV);
    irxptr_tab[modcount++].ptr = (void *)&imgdrv_irx;
    irxptr_tab[modcount].info = size_resetspu_irx | SET_OPL_MOD_ID(OPL_MODULE_ID_RESETSPU);
    irxptr_tab[modcount++].ptr = (void *)&resetspu_irx;

#ifdef PADEMU
#define PADEMU_ARG || gEnablePadEmu
#else
#define PADEMU_ARG
#endif
    if ((modules & CORE_IRX_USB) PADEMU_ARG) {
        irxptr_tab[modcount].info = size_usbd_irx | SET_OPL_MOD_ID(OPL_MODULE_ID_USBD);
        irxptr_tab[modcount++].ptr = (void *)&usbd_irx;
    }
    if (modules & CORE_IRX_USB) {
        irxptr_tab[modcount].info = size_usbmass_bd_irx | SET_OPL_MOD_ID(OPL_MODULE_ID_USBMASSBD);
        irxptr_tab[modcount++].ptr = (void *)&usbmass_bd_irx;
    }
    if (modules & CORE_IRX_ILINK) {
        irxptr_tab[modcount].info = size_iLinkman_irx | SET_OPL_MOD_ID(OPL_MODULE_ID_ILINK);
        irxptr_tab[modcount++].ptr = (void *)&iLinkman_irx;
        irxptr_tab[modcount].info = size_IEEE1394_bd_irx | SET_OPL_MOD_ID(OPL_MODULE_ID_ILINKBD);
        irxptr_tab[modcount++].ptr = (void *)&IEEE1394_bd_irx;
    }
    if (modules & CORE_IRX_ETH) {
        irxptr_tab[modcount].info = size_smap_ingame_irx | SET_OPL_MOD_ID(OPL_MODULE_ID_SMAP);
        irxptr_tab[modcount++].ptr = (void *)&smap_ingame_irx;
        irxptr_tab[modcount].info = size_ingame_smstcpip_irx | SET_OPL_MOD_ID(OPL_MODULE_ID_SMSTCPIP);
        irxptr_tab[modcount++].ptr = (void *)&ingame_smstcpip_irx;
    }
    if (modules & CORE_IRX_SMB) {
        irxptr_tab[modcount].info = size_smbinit_irx | SET_OPL_MOD_ID(OPL_MODULE_ID_SMBINIT);
        irxptr_tab[modcount++].ptr = (void *)&smbinit_irx;
    }

    if (modules & CORE_IRX_VMC) {
        irxptr_tab[modcount].info = size_mcemu_irx | SET_OPL_MOD_ID(OPL_MODULE_ID_MCEMU);
        irxptr_tab[modcount++].ptr = (void *)mcemu_irx;
    }

#ifdef PADEMU
    if (gEnablePadEmu) {
        if (gPadEmuSettings & 0xFF) {
            irxptr_tab[modcount].info = size_bt_pademu_irx | SET_OPL_MOD_ID(OPL_MODULE_ID_PADEMU);
            irxptr_tab[modcount++].ptr = (void *)&bt_pademu_irx;
        } else {
            irxptr_tab[modcount].info = size_usb_pademu_irx | SET_OPL_MOD_ID(OPL_MODULE_ID_PADEMU);
            irxptr_tab[modcount++].ptr = (void *)&usb_pademu_irx;
        }
    }
#endif

#ifdef __INGAME_DEBUG
#ifdef __DECI2_DEBUG
    if (modules & CORE_IRX_DECI2) {
        irxptr_tab[modcount].info = size_drvtif_ingame_irx | SET_OPL_MOD_ID(OPL_MODULE_ID_DRVTIF);
        irxptr_tab[modcount++].ptr = (void *)&drvtif_ingame_irx;
        irxptr_tab[modcount].info = size_tifinet_ingame_irx | SET_OPL_MOD_ID(OPL_MODULE_ID_TIFINET);
        irxptr_tab[modcount++].ptr = (void *)&tifinet_ingame_irx;
    }
#else
    if (modules & CORE_IRX_DEBUG) {
        irxptr_tab[modcount].info = size_udptty_ingame_irx | SET_OPL_MOD_ID(OPL_MODULE_ID_UDPTTY);
        irxptr_tab[modcount++].ptr = (void *)&udptty_ingame_irx;
        irxptr_tab[modcount].info = size_ioptrap_irx | SET_OPL_MOD_ID(OPL_MODULE_ID_IOPTRAP);
        irxptr_tab[modcount++].ptr = (void *)&ioptrap_irx;
    }
#endif
#endif

    modcount += addIopPatch(mode_str, startup, &irxptr_tab[modcount]);

    irxtable->modules = irxptr_tab;
    irxtable->count = modcount;

#ifdef __DECI2_DEBUG
    // For DECI2 debugging mode, the UDNL module will have to be stored within kernel RAM because there isn't enough space below user RAM.
    // total_size will hence not include the IOPRP image, but it's okay because the EE core is interested in protecting the module storage within user RAM.
    irxptr = (void *)0x00033000;
    LOG("SYSTEM DECI2 UDNL address start: %p end: %p\n", irxptr, irxptr + GET_OPL_MOD_SIZE(irxptr_tab[0].info));
    DI();
    ee_kmode_enter();
    memcpy((void *)(0x80000000 | (unsigned int)irxptr), irxptr_tab[0].ptr, GET_OPL_MOD_SIZE(irxptr_tab[0].info));
    ee_kmode_exit();
    EI();

    irxptr_tab[0].ptr = irxptr; // UDNL is the first module.
#endif

    total_size = (sizeof(irxtab_t) + sizeof(irxptr_t) * modcount + 0xF) & ~0xF;
    irxptr = (void *)((((unsigned int)irxptr_tab + sizeof(irxptr_t) * modcount) + 0xF) & ~0xF);

#ifdef __DECI2_DEBUG
    for (i = 1; i < modcount; i++) {
#else
    for (i = 0; i < modcount; i++) {
#endif
        curIrxSize = GET_OPL_MOD_SIZE(irxptr_tab[i].info);

        if (curIrxSize > 0) {
            LOG("SYSTEM IRX %u address start: %p end: %p\n", GET_OPL_MOD_ID(irxptr_tab[i].info), irxptr, irxptr + curIrxSize);
            memcpy(irxptr, irxptr_tab[i].ptr, curIrxSize);

            irxptr_tab[i].ptr = irxptr;
            irxptr += ((curIrxSize + 0xF) & ~0xF);
            total_size += ((curIrxSize + 0xF) & ~0xF);
        } else {
            irxptr_tab[i].ptr = NULL;
        }
    }

    free(ioprp_image);

    LOG("SYSTEM IRX STORAGE %p - %p\n", ModuleStorage, (u8 *)ModuleStorage + total_size);

    return total_size;
}

#ifdef __DECI2_DEBUG
/*
    Look for the start of the EE DECI2 manager initialization function.

    The stock EE kernel has no reset function, but the EE kernel is most likely already primed to self-destruct and in need of a good reset.
    What happens is that the OSD initializes the EE DECI2 TTY protocol at startup, but the EE DECI2 manager is never aware that the OSDSYS ever loads other programs.

    As a result, the EE kernel crashes immediately when the EE TTY gets used (when the IOP side of DECI2 comes up), when it invokes whatever that exists at the OSD's old ETTY handler's location. :(

    Must be run in kernel mode.
*/
static int ResetDECI2(void)
{
    int result;
    unsigned int i, *ptr;
    void (*pDeci2ManagerInit)(void);
    static const unsigned int Deci2ManagerInitPattern[] = {
        0x3c02bf80, // lui v0, $bf80
        0x3c04bfc0, // lui a0, $bfc0
        0x34423800, // ori v0, v0, $3800
        0x34840102  // ori a0, a0, $0102
    };

    result = -1;
    ptr = (void *)0x80000000;
    for (i = 0; i < 0x20000 / 4; i++) {
        if (ptr[i + 0] == Deci2ManagerInitPattern[0] &&
            ptr[i + 1] == Deci2ManagerInitPattern[1] &&
            ptr[i + 2] == Deci2ManagerInitPattern[2] &&
            ptr[i + 3] == Deci2ManagerInitPattern[3]) {
            pDeci2ManagerInit = (void *)&ptr[i - 14];
            pDeci2ManagerInit();
            result = 0;
            break;
        }
    }

    return result;
}

int sysInitDECI2(void)
{
    int result;

    DI();
    ee_kmode_enter();

    result = ResetDECI2();

    ee_kmode_exit();
    EI();

    return result;
}
#endif

/*  Returns the patch location of LoadExecPS2(), which resides in kernel memory.
 *  Patches the kernel to use the EELOAD module at the specified location.
 *  Must be run in kernel mode.
 */
static void *initLoadExecPS2(void *new_eeload)
{
    void *result;

    /* The pattern of the code in LoadExecPS2() that prepares the kernel for copying EELOAD from rom0: */
    static const unsigned int initEELOADCopyPattern[] = {
        0x8FA30010, /* lw       v1, 0x0010(sp) */
        0x0240302D, /* daddu    a2, s2, zero */
        0x8FA50014, /* lw       a1, 0x0014(sp) */
        0x8C67000C, /* lw       a3, 0x000C(v1) */
        0x18E00009, /* blez     a3, +9 <- The kernel will skip the EELOAD copying loop if the value in $a3 is less than, or equal to 0. Lets do that... */
    };

    u32 *p;

    result = NULL;
    /* Find the part of LoadExecPS2() that initilizes the EELOAD copying loop's variables */
    for (p = (u32 *)0x80001000; p < (u32 *)0x80030000; p++) {
        if (memcmp(p, &initEELOADCopyPattern, sizeof(initEELOADCopyPattern)) == 0) {
            p[1] = 0x3C120000 | (u16)((u32)new_eeload >> 16);    /* lui s2, HI16(new_eeload) */
            p[2] = 0x36520000 | (u16)((u32)new_eeload & 0xFFFF); /* ori s2, s2, LO16(new_eeload) */
            p[3] = 0x24070000;                                   /* li a3, 0 <- Disable the EELOAD copying loop */
            result = (void *)p;
            break; /* All done. */
        }
    }

    return result;
}

/*  Gets the address of the jump to the function that initializes user memory.
 *  Pathces the kernel to begin erasure of memory from the specified address.
 *  Must be run in kernel mode.
 */
static void *initInitializeUserMemory(void *start)
{
    u32 *p;
    void *result;

    result = NULL;
    for (p = (unsigned int *)0x80001000; p < (unsigned int *)0x80030000; p++) {
        /*
         * Search for function call and where $a0 is set.
         *  lui  $a0, 0x0008
         *  jal  InitializeUserMemory
         *  ori  $a0, $a0, 0x2000
         */
        if (p[0] == 0x3c040008 && (p[1] & 0xfc000000) == 0x0c000000 && p[2] == 0x34842000) {
            p[0] = 0x3c040000 | ((unsigned int)start >> 16);
            p[2] = 0x34840000 | ((unsigned int)start & 0xffff);
            result = (void *)p;
            break;
        }
    }

    return result;
}

static int initKernel(void *eeload, void *modStorageEnd, void **eeloadCopy, void **initUserMemory)
{
    DI();
    ee_kmode_enter();

#ifdef __DECI2_DEBUG
    ResetDECI2();
#endif
    *eeloadCopy = initLoadExecPS2(eeload);
    *initUserMemory = initInitializeUserMemory(modStorageEnd);

    ee_kmode_exit();
    EI();

    return ((*eeloadCopy != NULL && *initUserMemory != NULL) ? 0 : -1);
}

void sysLaunchLoaderElf(const char *filename, const char *mode_str, int size_cdvdman_irx, void **cdvdman_irx, int size_mcemu_irx, void **mcemu_irx, int EnablePS2Logo, unsigned int compatflags)
{
    unsigned int modules, ModuleStorageSize;
    void *ModuleStorage, *ModuleStorageEnd;
    u8 local_ip_address[4], local_netmask[4], local_gateway[4];
    u8 *boot_elf = NULL;
    elf_header_t *eh;
    elf_pheader_t *eph;
    void *pdata;
    int argc, i;
    char ElfPath[32];
    char *argv[7 + GSM_ARGS];
    char ModStorageConfig[32];
    char KernelConfig[32];
    char config_str[256];
    char gsm_config_str[256];
    void *eeloadCopy, *initUserMemory;

    ethGetNetConfig(local_ip_address, local_netmask, local_gateway);
#if (!defined(__DEBUG) && !defined(_DTL_T10000))
    AddHistoryRecordUsingFullPath(filename);
#endif

    if (gExitPath[0] == '\0')
        strncpy(gExitPath, "Browser", sizeof(gExitPath));

    // Disable sound effects via libsd, to prevent some games with improper initialization from inadvertently using digital effect settings from other software.
    sysLoadModuleBuffer(&cleareffects_irx, size_cleareffects_irx, 0, NULL);

    // Wipe the low user memory region, since this region might not be wiped after OPL's EE core is installed.
    // Start wiping from 0x00084000 instead (as the HDD Browser does), as the alarm patch is installed at 0x00082000.
    memset((void *)0x00084000, 0, 0x00100000 - 0x00084000);

    modules = 0;
    ModuleStorage = GetModStorageLocation(filename, compatflags);

#ifdef __DECI2_DEBUG
    modules |= CORE_IRX_DECI2 | CORE_IRX_ETH;
#elif defined(__INGAME_DEBUG)
    modules |= CORE_IRX_DEBUG | CORE_IRX_ETH;
#endif

    modules |= CORE_IRX_VMC;

    LOG("SYSTEM LaunchLoaderElf loading modules\n");
    ModuleStorageSize = (sendIrxKernelRAM(filename, mode_str, modules, ModuleStorage, size_cdvdman_irx, cdvdman_irx, size_mcemu_irx, mcemu_irx) + 0x3F) & ~0x3F;

    ModuleStorageEnd = (void *)((u8 *)ModuleStorage + ModuleStorageSize);
    sprintf(ModStorageConfig, "%u %u", (unsigned int)ModuleStorage, (unsigned int)ModuleStorageEnd);

    // NB: LOADER.ELF is embedded
    boot_elf = (u8 *)&eecore_elf;
    eh = (elf_header_t *)boot_elf;
    eph = (elf_pheader_t *)(boot_elf + eh->phoff);

    // Scan through the ELF's program headers and copy them into RAM, then
    // zero out any non-loaded regions.
    for (i = 0; i < eh->phnum; i++) {
        if (eph[i].type != ELF_PT_LOAD)
            continue;

        pdata = (void *)(boot_elf + eph[i].offset);
        memcpy(eph[i].vaddr, pdata, eph[i].filesz);

        if (eph[i].memsz > eph[i].filesz)
            memset(eph[i].vaddr + eph[i].filesz, 0, eph[i].memsz - eph[i].filesz);
    }

    // Get the kernel to use our EELOAD module and to begin erasure after module storage. EE core will erase any memory before the module storage (if any).
    if (initKernel((void *)eh->entry, ModuleStorageEnd, &eeloadCopy, &initUserMemory) != 0) { // Should not happen, but...
        LOG("Error - kernel is unsupported.\n");
        asm volatile("break\n");
    }
    sprintf(KernelConfig, "%u %u", (unsigned int)eeloadCopy, (unsigned int)initUserMemory);

#ifdef PADEMU
#define PADEMU_SPECIFIER " %d, %u"
#define PADEMU_ARGUMENT  , gEnablePadEmu, (unsigned int)(gPadEmuSettings >> 8)
#else
#define PADEMU_SPECIFIER
#define PADEMU_ARGUMENT
#endif

    argc = 0;
    sprintf(config_str, "%s %d %s %d %u.%u.%u.%u %u.%u.%u.%u %u.%u.%u.%u %d %u %d" PADEMU_SPECIFIER,
            mode_str, gDisableDebug, gExitPath, gHDDSpindown,
            local_ip_address[0], local_ip_address[1], local_ip_address[2], local_ip_address[3],
            local_netmask[0], local_netmask[1], local_netmask[2], local_netmask[3],
            local_gateway[0], local_gateway[1], local_gateway[2], local_gateway[3],
            gETHOpMode,
            GetCheatsEnabled() ? (unsigned int)GetCheatsList() : 0,
            GetGSMEnabled() PADEMU_ARGUMENT);
    argv[argc] = config_str;
    argc++;

    argv[argc] = KernelConfig;
    argc++;

    argv[argc] = ModStorageConfig;
    argc++;

    argv[argc] = (char *)filename;
    argc++;

    char cmask[10];
    snprintf(cmask, 10, "%d", compatflags);
    argv[argc] = cmask;
    argc++;

    PrepareGSM(gsm_config_str);
    argv[argc] = gsm_config_str;
    argc++;

    // PS2LOGO Caller, based on l_oliveira & SP193 tips
    // Don't call LoadExecPS2 here because it will wipe all memory above the EE core, making it impossible to pass data via pointers.
    if (EnablePS2Logo) {
        argv[argc] = "rom0:PS2LOGO";
        argc++;
    }

    snprintf(ElfPath, sizeof(ElfPath), "cdrom0:\\%s;1", filename);
    argv[argc] = ElfPath;
    argc++;

#ifdef __DEBUG
    LOG("Starting ee_core with following arguments:\n");
    for (i = 0; i < argc; i++) {
        LOG("[%d] %s\n", i, argv[i]);
    }
#endif

    // Let's go.
    fileXioExit();
    SifExitRpc();

    FlushCache(0);
    FlushCache(2);

    ExecPS2((void *)eh->entry, NULL, argc, argv);
}

int sysExecElf(const char *path)
{
    u8 *boot_elf = NULL;
    elf_header_t *eh;
    elf_pheader_t *eph;
    void *pdata;
    int i;
    char *elf_argv[2];
    char argv[256];
    int elf_argc = 1;

    // NB: ELFLDR.ELF is embedded
    boot_elf = (u8 *)&elfldr_elf;
    eh = (elf_header_t *)boot_elf;
    if (_lw((u32)&eh->ident) != ELF_MAGIC)
        while (1)
            ;

    eph = (elf_pheader_t *)(boot_elf + eh->phoff);

    // Scan through the ELF's program headers and copy them into RAM, then
    // zero out any non-loaded regions.
    for (i = 0; i < eh->phnum; i++) {
        if (eph[i].type != ELF_PT_LOAD)
            continue;

        pdata = (void *)(boot_elf + eph[i].offset);
        memcpy(eph[i].vaddr, pdata, eph[i].filesz);

        if (eph[i].memsz > eph[i].filesz)
            memset(eph[i].vaddr + eph[i].filesz, 0, eph[i].memsz - eph[i].filesz);
    }

    // Let's go.
    fileXioExit();
    SifExitRpc();

    elf_argv[0] = (char *)path;

    if (strncmp(path, "pfs", 3) == 0) {
        snprintf(argv, sizeof(argv), "%s:%s", gOPLPart, elf_argv[0]);
        elf_argv[1] = argv;
        elf_argc++;
    }

    FlushCache(0);
    FlushCache(2);

    ExecPS2((void *)eh->entry, NULL, elf_argc, elf_argv);

    return 0;
}

int sysCheckMC(void)
{
    DIR *mc0_root_dir = opendir("mc0:");
    if (mc0_root_dir != NULL) {
        closedir(mc0_root_dir);
        return 0;
    }

    DIR *mc1_root_dir = opendir("mc1:");
    if (mc1_root_dir != NULL) {
        closedir(mc1_root_dir);
        return 1;
    }

    return -11;
}

// createSize == -1 : delete, createSize == 0 : probing, createSize > 0 : creation
int sysCheckVMC(const char *prefix, const char *sep, char *name, int createSize, vmc_superblock_t *vmc_superblock)
{
    int size = -1;
    char path[256];
    snprintf(path, sizeof(path), "%sVMC%s%s.bin", prefix, sep, name);

    if (createSize == -1)
        unlink(path);
    else {
        int fd = open(path, O_RDONLY, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);
        if (fd >= 0) {
            size = lseek(fd, 0, SEEK_END);

            if (vmc_superblock) {
                memset(vmc_superblock, 0, sizeof(vmc_superblock_t));
                lseek(fd, 0, SEEK_SET);
                read(fd, (void *)vmc_superblock, sizeof(vmc_superblock_t));

                LOG("SYSTEM File size  : 0x%X\n", size);
                LOG("SYSTEM Magic      : %s\n", vmc_superblock->magic);
                LOG("SYSTEM Card type  : %d\n", vmc_superblock->mc_type);
                LOG("SYSTEM Flags      : 0x%X\n", (vmc_superblock->mc_flag & 0xFF) | 0x100);
                LOG("SYSTEM Page_size  : 0x%X\n", vmc_superblock->page_size);
                LOG("SYSTEM Block_size : 0x%X\n", vmc_superblock->pages_per_block);
                LOG("SYSTEM Card_size  : 0x%X\n", vmc_superblock->pages_per_cluster * vmc_superblock->clusters_per_card);

                if (!strncmp(vmc_superblock->magic, "Sony PS2 Memory Card Format", 27) && vmc_superblock->mc_type == 0x2 && size == vmc_superblock->pages_per_cluster * vmc_superblock->clusters_per_card * vmc_superblock->page_size) {
                    LOG("SYSTEM VMC file structure valid: %s\n", path);
                } else
                    size = 0;
            }

            if (size % 1048576) // invalid size, should be a an integer (8, 16, 32, 64, ...)
                size = 0;
            else
                size /= 1048576;

            close(fd);

            if (createSize && (createSize != size))
                unlink(path);
        }


        if (createSize && (createSize != size)) {
            createVMCparam_t createParam;
            strcpy(createParam.VMC_filename, path);
            createParam.VMC_size_mb = createSize;
            createParam.VMC_blocksize = 16;
            createParam.VMC_thread_priority = 0x0f;
            createParam.VMC_card_slot = -1;
            fileXioDevctl("genvmc:", 0xC0DE0001, (void *)&createParam, sizeof(createParam), NULL, 0);
        }
    }
    return size;
}
