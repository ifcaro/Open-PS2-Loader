/*
 Copyright 2010, Volca
 Licenced under Academic Free License version 3.0
 Review OpenUsbLd README & LICENSE files for further details.
 */

#include "include/opl.h"
#include "include/gui.h"
#include "include/renderman.h"
#include "include/menusys.h"
#include "include/fntsys.h"
#include "include/ioman.h"
#include "include/lang.h"
#include "include/themes.h"
#include "include/pad.h"
#include "include/util.h"
#include "include/config.h"
#include "include/system.h"
#include "include/ethsupport.h"
#include "include/compatupd.h"
#include "include/pggsm.h"
#include "include/cheatman.h"
#include "include/sound.h"
#include "include/guigame.h"

#include <limits.h>
#include <stdlib.h>
#include <libvux.h>

// Last Played Auto Start
#include <time.h>

static int gScheduledOps;
static int gCompletedOps;
static int gTerminate;
static int gInitComplete;

static gui_callback_t gFrameHook;

static s32 gSemaId;
static s32 gGUILockSemaId;
static ee_sema_t gQueueSema;

static int screenWidth;
static int screenHeight;

static int showThmPopup;
static int showLngPopup;

static clock_t popupTimer = 0;

// forward decl.
static void guiShow();

#ifdef __DEBUG

// debug version displays an FPS meter
static clock_t prevtime = 0;
static clock_t curtime = 0;
static float fps = 0.0f;

extern GSGLOBAL *gsGlobal;
#endif

// Global data
int guiInactiveFrames;
int guiFrameId;

struct gui_update_list_t
{
    struct gui_update_t *item;
    struct gui_update_list_t *next;
};

struct gui_update_list_t *gUpdateList;
struct gui_update_list_t *gUpdateEnd;

typedef struct
{
    void (*handleInput)(void);
    void (*renderScreen)(void);
    short inMenu;
} gui_screen_handler_t;

static gui_screen_handler_t screenHandlers[] = {{&menuHandleInputMain, &menuRenderMain, 0},
                                                {&menuHandleInputMenu, &menuRenderMenu, 1},
                                                {&menuHandleInputInfo, &menuRenderInfo, 1},
                                                {&menuHandleInputGameMenu, &menuRenderGameMenu, 1},
                                                {&menuHandleInputAppMenu, &menuRenderAppMenu, 1}};

// default screen handler (menu screen)
static gui_screen_handler_t *screenHandler = &screenHandlers[GUI_SCREEN_MENU];

// screen transition handling
static gui_screen_handler_t *screenHandlerTarget = NULL;
static int transIndex;

// Helper perlin noise data
#define PLASMA_H              32
#define PLASMA_W              32
#define PLASMA_ROWS_PER_FRAME 6
#define FADE_SIZE             256

static GSTEXTURE gBackgroundTex;
static int pperm[512];
static float fadetbl[FADE_SIZE + 1];

static VU_VECTOR pgrad3[12] = {{1, 1, 0, 1}, {-1, 1, 0, 1}, {1, -1, 0, 1}, {-1, -1, 0, 1}, {1, 0, 1, 1}, {-1, 0, 1, 1}, {1, 0, -1, 1}, {-1, 0, -1, 1}, {0, 1, 1, 1}, {0, -1, 1, 1}, {0, 1, -1, 1}, {0, -1, -1, 1}};

void guiReloadScreenExtents()
{
    rmGetScreenExtents(&screenWidth, &screenHeight);
}

void guiInit(void)
{
    guiFrameId = 0;
    guiInactiveFrames = 0;

    gFrameHook = NULL;
    gTerminate = 0;
    gInitComplete = 0;
    gScheduledOps = 0;
    gCompletedOps = 0;

    gUpdateList = NULL;
    gUpdateEnd = NULL;

    gQueueSema.init_count = 1;
    gQueueSema.max_count = 1;
    gQueueSema.option = 0;

    gSemaId = CreateSema(&gQueueSema);
    gGUILockSemaId = CreateSema(&gQueueSema);

    guiReloadScreenExtents();

    // background texture - for perlin
    gBackgroundTex.Width = PLASMA_W;
    gBackgroundTex.Height = PLASMA_H;
    gBackgroundTex.Mem = memalign(128, PLASMA_W * PLASMA_H * 4);
    gBackgroundTex.PSM = GS_PSM_CT32;
    gBackgroundTex.Filter = GS_FILTER_LINEAR;
    gBackgroundTex.Vram = 0;
    gBackgroundTex.VramClut = 0;
    gBackgroundTex.Clut = NULL;

    // Precalculate the values for the perlin noise plasma
    int i;
    for (i = 0; i < 256; ++i) {
        pperm[i] = rand() % 256;
        pperm[i + 256] = pperm[i];
    }

    for (i = 0; i <= FADE_SIZE; ++i) {
        float t = (float)(i) / FADE_SIZE;

        fadetbl[i] = t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }
}

void guiEnd()
{
    if (gBackgroundTex.Mem)
        free(gBackgroundTex.Mem);

    DeleteSema(gSemaId);
    DeleteSema(gGUILockSemaId);
}

void guiLock(void)
{
    WaitSema(gGUILockSemaId);
}

void guiUnlock(void)
{
    SignalSema(gGUILockSemaId);
}

void guiStartFrame(void)
{
    guiLock();
    rmStartFrame();
    guiFrameId++;
}

void guiEndFrame(void)
{
    rmEndFrame();
#ifdef __DEBUG
    // Measure time directly after vsync
    prevtime = curtime;
    curtime = clock();
#endif
    guiUnlock();
}

void guiShowAbout()
{
    char OPLVersion[40];
    char OPLBuildDetails[40];

    snprintf(OPLVersion, sizeof(OPLVersion), _l(_STR_OPL_VER), OPL_VERSION);
    diaSetLabel(diaAbout, ABOUT_TITLE, OPLVersion);

    snprintf(OPLBuildDetails, sizeof(OPLBuildDetails), "GSM %s"
#ifdef __RTL
                                                       " - RTL"
#endif
#ifdef IGS
                                                       " - IGS %s"
#endif
#ifdef PADEMU
                                                       " - PADEMU"
#endif
             // Version numbers
             ,
             GSM_VERSION
#ifdef IGS
             ,
             IGS_VERSION
#endif
    );
    diaSetLabel(diaAbout, ABOUT_BUILD_DETAILS, OPLBuildDetails);

    diaExecuteDialog(diaAbout, -1, 1, NULL);
}

void guiCheckNotifications(int checkTheme, int checkLang)
{
    if (gEnableNotifications) {
        if (checkTheme) {
            if (thmGetGuiValue() != 0)
                showThmPopup = 1;
        }

        if (checkLang) {
            if (lngGetGuiValue() != 0)
                showLngPopup = 1;
        }
    }
}

static void guiResetNotifications(void)
{
    showThmPopup = 0;
    showLngPopup = 0;
    popupTimer = 0;
}

static void guiRenderNotifications(char *type, char *path, int y)
{
    char notification[128];
    char *col_pos;
    int x;

    snprintf(notification, sizeof(notification), _l(_STR_NOTIFICATIONS), type, path);
    if ((col_pos = strchr(notification, ':')) != NULL)
        *(col_pos + 1) = '\0';

    x = screenWidth - rmUnScaleX(fntCalcDimensions(gTheme->fonts[0], notification)) - 10;

    rmDrawRect(x - 10, y, screenWidth - x, MENU_ITEM_HEIGHT + 10, gColDarker);
    fntRenderString(gTheme->fonts[0], x - 5, y + 5, ALIGN_NONE, 0, 0, notification, gTheme->textColor);
}

static void guiShowNotifications(void)
{
    int y = 10;
    int yadd = 35;
    clock_t currentTime;

    currentTime = clock();
    if (showThmPopup || showLngPopup || showCfgPopup) {
        if (!popupTimer) {
            popupTimer = clock() + 5000 * (CLOCKS_PER_SEC / 1000);
            sfxPlay(SFX_MESSAGE);
        }

        if (showCfgPopup) {
            guiRenderNotifications("CFG", configGetDir(), y);
            y += yadd;
        }

        if (showThmPopup) {
            guiRenderNotifications("THM", thmGetFilePath(thmGetGuiValue()), y);
            y += yadd;
        }

        if (showLngPopup)
            guiRenderNotifications("LNG", lngGetFilePath(lngGetGuiValue()), y);

        if (currentTime >= popupTimer) {
            guiResetNotifications();
            showCfgPopup = 0;
        }
    }
}

static int guiNetCompatUpdRefresh(int modified)
{
    int result;
    unsigned int done, total;

    if ((result = oplGetUpdateGameCompatStatus(&done, &total)) == OPL_COMPAT_UPDATE_STAT_WIP) {
        diaSetInt(diaNetCompatUpdate, NETUPD_PROGRESS, (done == 0 || total == 0) ? 0 : (int)((float)done / total * 100.0f));
    }

    return result;
}

static void guiShowNetCompatUpdateResult(int result)
{
    switch (result) {
        case OPL_COMPAT_UPDATE_STAT_DONE:
            // Completed with no errors.
            guiMsgBox(_l(_STR_NET_UPDATE_DONE), 0, NULL);
            break;
        case OPL_COMPAT_UPDATE_STAT_ERROR:
            // Completed with errors.
            guiMsgBox(_l(_STR_NET_UPDATE_FAILED), 0, NULL);
            break;
        case OPL_COMPAT_UPDATE_STAT_CONN_ERROR:
            // Completed with errors.
            guiMsgBox(_l(_STR_NET_UPDATE_CONN_FAILED), 0, NULL);
            break;
        case OPL_COMPAT_UPDATE_STAT_ABORTED:
            // User-aborted.
            guiMsgBox(_l(_STR_NET_UPDATE_CANCELLED), 0, NULL);
            break;
    }
}

void guiShowNetCompatUpdate(void)
{
    int ret, UpdateAll;
    u8 done, started;
    void *UpdateFunction;

    diaSetVisible(diaNetCompatUpdate, NETUPD_BTN_START, 1);
    diaSetVisible(diaNetCompatUpdate, NETUPD_BTN_CANCEL, 0);
    diaSetVisible(diaNetCompatUpdate, NETUPD_PROGRESS_LBL, 0);
    diaSetVisible(diaNetCompatUpdate, NETUPD_PROGRESS_PERC_LBL, 0);
    diaSetVisible(diaNetCompatUpdate, NETUPD_PROGRESS, 0);
    diaSetInt(diaNetCompatUpdate, NETUPD_OPT_UPD_ALL, 0);
    diaSetEnabled(diaNetCompatUpdate, NETUPD_OPT_UPD_ALL, 1);

    done = 0;
    started = 0;
    UpdateFunction = NULL;
    while (!done) {
        ret = diaExecuteDialog(diaNetCompatUpdate, -1, 1, UpdateFunction);
        switch (ret) {
            case NETUPD_BTN_START:
                if (guiMsgBox(_l(_STR_CONFIRMATION_SETTINGS_UPDATE), 1, NULL)) {
                    guiRenderTextScreen(_l(_STR_PLEASE_WAIT));

                    if ((ret = ethLoadInitModules()) == 0) {
                        diaSetVisible(diaNetCompatUpdate, NETUPD_BTN_START, 0);
                        diaSetVisible(diaNetCompatUpdate, NETUPD_BTN_CANCEL, 1);
                        diaSetVisible(diaNetCompatUpdate, NETUPD_PROGRESS_LBL, 1);
                        diaSetVisible(diaNetCompatUpdate, NETUPD_PROGRESS_PERC_LBL, 1);
                        diaSetVisible(diaNetCompatUpdate, NETUPD_PROGRESS, 1);
                        diaSetEnabled(diaNetCompatUpdate, NETUPD_OPT_UPD_ALL, 0);

                        diaGetInt(diaNetCompatUpdate, NETUPD_OPT_UPD_ALL, &UpdateAll);
                        oplUpdateGameCompat(UpdateAll);
                        UpdateFunction = &guiNetCompatUpdRefresh;
                        started = 1;
                    } else {
                        ethDisplayErrorStatus();
                    }
                }
                break;
            case UIID_BTN_CANCEL: // If the user pressed the cancel button.
            case NETUPD_BTN_CANCEL:
                if (started) {
                    if (guiMsgBox(_l(_STR_CONFIRMATION_CANCEL_UPDATE), 1, NULL)) {
                        guiRenderTextScreen(_l(_STR_PLEASE_WAIT));
                        oplAbortUpdateGameCompat();
                        // The process truly ends when the UI callback gets the update from the worker thread that the process has ended.
                    }
                } else {
                    done = 1;
                    started = 0;
                }
                break;
            default:
                guiShowNetCompatUpdateResult(ret);
                done = 1;
                started = 0;
                UpdateFunction = NULL;
                break;
        }
    }
}

void guiShowNetCompatUpdateSingle(int id, item_list_t *support, config_set_t *configSet)
{
    int ConfigSource, result;

    ConfigSource = CONFIG_SOURCE_DEFAULT;
    configGetInt(configSet, CONFIG_ITEM_CONFIGSOURCE, &ConfigSource);

    if (guiMsgBox(_l(_STR_CONFIRMATION_SETTINGS_UPDATE), 1, NULL)) {
        guiRenderTextScreen(_l(_STR_PLEASE_WAIT));

        if ((result = ethLoadInitModules()) == 0) {
            if ((result = oplUpdateGameCompatSingle(id, support, configSet)) == OPL_COMPAT_UPDATE_STAT_DONE) {
                configSetInt(configSet, CONFIG_ITEM_CONFIGSOURCE, CONFIG_SOURCE_DLOAD);
            }
            guiShowNetCompatUpdateResult(result);
        } else {
            ethDisplayErrorStatus();
        }
    }
}

static int guiUpdater(int modified)
{
    int showAutoStartLast;

    if (modified) {
        diaGetInt(diaConfig, CFG_LASTPLAYED, &showAutoStartLast);
        diaSetVisible(diaConfig, CFG_LBL_AUTOSTARTLAST, showAutoStartLast);
        diaSetVisible(diaConfig, CFG_AUTOSTARTLAST, showAutoStartLast);

        diaGetInt(diaConfig, CFG_BDMMODE, &gBDMStartMode);
        diaSetEnabled(diaConfig, CFG_ENABLEFW, gBDMStartMode);
    }
    return 0;
}

void guiShowConfig()
{
    // configure the enumerations
    const char *deviceNames[] = {_l(_STR_BDM_GAMES), _l(_STR_NET_GAMES), _l(_STR_HDD_GAMES), NULL};
    const char *deviceModes[] = {_l(_STR_OFF), _l(_STR_MANUAL), _l(_STR_AUTO), NULL};

    diaSetEnum(diaConfig, CFG_DEFDEVICE, deviceNames);
    diaSetEnum(diaConfig, CFG_BDMMODE, deviceModes);
    diaSetEnum(diaConfig, CFG_HDDMODE, deviceModes);
    diaSetEnum(diaConfig, CFG_ETHMODE, deviceModes);
    diaSetEnum(diaConfig, CFG_APPMODE, deviceModes);

    diaSetInt(diaConfig, CFG_DEBUG, gDisableDebug);
    diaSetInt(diaConfig, CFG_PS2LOGO, gPS2Logo);
    diaSetInt(diaConfig, CFG_HDDGAMELISTCACHE, gHDDGameListCache);
    diaSetString(diaConfig, CFG_EXITTO, gExitPath);
    diaSetInt(diaConfig, CFG_ENWRITEOP, gEnableWrite);
    diaSetInt(diaConfig, CFG_HDDSPINDOWN, gHDDSpindown);
    diaSetString(diaConfig, CFG_BDMPREFIX, gBDMPrefix);
    diaSetString(diaConfig, CFG_ETHPREFIX, gETHPrefix);
    diaSetInt(diaConfig, CFG_LASTPLAYED, gRememberLastPlayed);
    diaSetInt(diaConfig, CFG_AUTOSTARTLAST, gAutoStartLastPlayed);
    diaSetVisible(diaConfig, CFG_AUTOSTARTLAST, gRememberLastPlayed);
    diaSetVisible(diaConfig, CFG_LBL_AUTOSTARTLAST, gRememberLastPlayed);

    diaSetInt(diaConfig, CFG_DEFDEVICE, gDefaultDevice);
    diaSetInt(diaConfig, CFG_BDMMODE, gBDMStartMode);
    diaSetInt(diaConfig, CFG_HDDMODE, gHDDStartMode);
    diaSetInt(diaConfig, CFG_ETHMODE, gETHStartMode);
    diaSetInt(diaConfig, CFG_APPMODE, gAPPStartMode);

    diaSetInt(diaConfig, CFG_ENABLEFW, gEnableFW);

    int ret = diaExecuteDialog(diaConfig, -1, 1, &guiUpdater);
    if (ret) {
        diaGetInt(diaConfig, CFG_DEBUG, &gDisableDebug);
        diaGetInt(diaConfig, CFG_PS2LOGO, &gPS2Logo);
        diaGetInt(diaConfig, CFG_HDDGAMELISTCACHE, &gHDDGameListCache);
        diaGetString(diaConfig, CFG_EXITTO, gExitPath, sizeof(gExitPath));
        diaGetInt(diaConfig, CFG_ENWRITEOP, &gEnableWrite);
        diaGetInt(diaConfig, CFG_HDDSPINDOWN, &gHDDSpindown);
        diaGetString(diaConfig, CFG_BDMPREFIX, gBDMPrefix, sizeof(gBDMPrefix));
        diaGetString(diaConfig, CFG_ETHPREFIX, gETHPrefix, sizeof(gETHPrefix));
        diaGetInt(diaConfig, CFG_LASTPLAYED, &gRememberLastPlayed);
        diaGetInt(diaConfig, CFG_AUTOSTARTLAST, &gAutoStartLastPlayed);
        DisableCron = 1; // Disable Auto Start Last Played counter (we don't want to call it right after enable it on GUI)
        diaGetInt(diaConfig, CFG_DEFDEVICE, &gDefaultDevice);
        diaGetInt(diaConfig, CFG_HDDMODE, &gHDDStartMode);
        diaGetInt(diaConfig, CFG_ETHMODE, &gETHStartMode);
        diaGetInt(diaConfig, CFG_APPMODE, &gAPPStartMode);

        diaGetInt(diaConfig, CFG_ENABLEFW, &gEnableFW);

        applyConfig(-1, -1);
        menuReinitMainMenu();
    }
}

static int curTheme = -1;

static int guiUIUpdater(int modified)
{
    if (modified) {
        int temp, x, y;
        diaGetInt(diaUIConfig, UICFG_THEME, &temp);
        if (temp != curTheme) {
            curTheme = temp;
            if (temp == 0) {
                // Display the default theme's colours.
                diaSetItemType(diaUIConfig, UICFG_BGCOL, UI_COLOUR); // Must be correctly set before doing the diaS/GetColor !!
                diaSetItemType(diaUIConfig, UICFG_UICOL, UI_COLOUR);
                diaSetItemType(diaUIConfig, UICFG_TXTCOL, UI_COLOUR);
                diaSetItemType(diaUIConfig, UICFG_SELCOL, UI_COLOUR);
                diaSetColor(diaUIConfig, UICFG_BGCOL, gDefaultBgColor);
                diaSetColor(diaUIConfig, UICFG_UICOL, gDefaultUITextColor);
                diaSetColor(diaUIConfig, UICFG_TXTCOL, gDefaultTextColor);
                diaSetColor(diaUIConfig, UICFG_SELCOL, gDefaultSelTextColor);
            } else if (temp == thmGetGuiValue()) {
                // Display the current theme's colours.
                diaSetItemType(diaUIConfig, UICFG_BGCOL, UI_COLOUR);
                diaSetItemType(diaUIConfig, UICFG_UICOL, UI_COLOUR);
                diaSetItemType(diaUIConfig, UICFG_TXTCOL, UI_COLOUR);
                diaSetItemType(diaUIConfig, UICFG_SELCOL, UI_COLOUR);
                diaSetColor(diaUIConfig, UICFG_BGCOL, gTheme->bgColor);
                diaSetU64Color(diaUIConfig, UICFG_UICOL, gTheme->uiTextColor);
                diaSetU64Color(diaUIConfig, UICFG_TXTCOL, gTheme->textColor);
                diaSetU64Color(diaUIConfig, UICFG_SELCOL, gTheme->selTextColor);
            } else {
                // When another theme is highlighted in the list, its colours are not known. Don't show any colours.
                diaSetItemType(diaUIConfig, UICFG_BGCOL, UI_SPACER);
                diaSetItemType(diaUIConfig, UICFG_UICOL, UI_SPACER);
                diaSetItemType(diaUIConfig, UICFG_TXTCOL, UI_SPACER);
                diaSetItemType(diaUIConfig, UICFG_SELCOL, UI_SPACER);
            }

            // The user cannot adjust the current theme's colours.
            temp = !temp;
            diaSetEnabled(diaUIConfig, UICFG_BGCOL, temp);
            diaSetEnabled(diaUIConfig, UICFG_UICOL, temp);
            diaSetEnabled(diaUIConfig, UICFG_TXTCOL, temp);
            diaSetEnabled(diaUIConfig, UICFG_SELCOL, temp);
            diaSetEnabled(diaUIConfig, UICFG_RESETCOL, temp);
        }

        diaGetInt(diaUIConfig, UICFG_XOFF, &x);
        diaGetInt(diaUIConfig, UICFG_YOFF, &y);
        if ((x != gXOff) || (y != gYOff)) {
            gXOff = x;
            gYOff = y;
            rmSetDisplayOffset(x, y);
        }
        diaGetInt(diaUIConfig, UICFG_OVERSCAN, &temp);
        if (temp != gOverscan) {
            gOverscan = temp;
            rmSetOverscan(gOverscan);
            guiUpdateScreenScale();
        }
        diaGetInt(diaUIConfig, UICFG_WIDESCREEN, &temp);
        if (temp != gWideScreen) {
            gWideScreen = temp;
            rmSetAspectRatio((gWideScreen == 0) ? RM_ARATIO_4_3 : RM_ARATIO_16_9);
            guiUpdateScreenScale();
        }
    }

    return 0;
}

void guiShowUIConfig(void)
{
    int themeID = -1, langID = -1;
    curTheme = -1;
    showCfgPopup = 0;
    guiResetNotifications();

    // clang-format off
    const char *vmodeNames[] = {_l(_STR_AUTO)
        , "PAL 640x512i @50Hz 24bit"
        , "NTSC 640x448i @60Hz 24bit"
        , "EDTV 640x448p @60Hz 24bit"
        , "EDTV 640x512p @50Hz 24bit"
        , "VGA 640x480p @60Hz 24bit"
        , "PAL 704x576i @50Hz 24bit (HIRES)"
        , "NTSC 704x480i @60Hz 24bit (HIRES)"
        , "EDTV 704x480p @60Hz 24bit (HIRES)"
        , "EDTV 704x576p @50Hz 24bit (HIRES)"
        , "HDTV 1280x720p @60Hz 16bit (HIRES)"
        , "HDTV 1920x1080i @60Hz 16bit (HIRES)"
        , NULL};
    // clang-format on
    int previousVMode;

reselect_video_mode:
    previousVMode = gVMode;
    diaSetEnum(diaUIConfig, UICFG_THEME, (const char **)thmGetGuiList());
    diaSetEnum(diaUIConfig, UICFG_LANG, (const char **)lngGetGuiList());
    diaSetEnum(diaUIConfig, UICFG_VMODE, vmodeNames);
    diaSetInt(diaUIConfig, UICFG_THEME, thmGetGuiValue());
    diaSetInt(diaUIConfig, UICFG_LANG, lngGetGuiValue());
    diaSetInt(diaUIConfig, UICFG_AUTOSORT, gAutosort);
    diaSetInt(diaUIConfig, UICFG_AUTOREFRESH, gAutoRefresh);
    diaSetInt(diaUIConfig, UICFG_NOTIFICATIONS, gEnableNotifications);
    diaSetInt(diaUIConfig, UICFG_COVERART, gEnableArt);
    diaSetInt(diaUIConfig, UICFG_WIDESCREEN, gWideScreen);
    diaSetInt(diaUIConfig, UICFG_VMODE, gVMode);
    diaSetInt(diaUIConfig, UICFG_XOFF, gXOff);
    diaSetInt(diaUIConfig, UICFG_YOFF, gYOff);
    diaSetInt(diaUIConfig, UICFG_OVERSCAN, gOverscan);
    guiUIUpdater(1);

    int ret = diaExecuteDialog(diaUIConfig, -1, 1, guiUIUpdater);
    if (ret) {
        diaGetInt(diaUIConfig, UICFG_LANG, &langID);
        diaGetInt(diaUIConfig, UICFG_THEME, &themeID);
        if (themeID == 0) {
            diaGetColor(diaUIConfig, UICFG_BGCOL, gDefaultBgColor);
            diaGetColor(diaUIConfig, UICFG_UICOL, gDefaultUITextColor);
            diaGetColor(diaUIConfig, UICFG_TXTCOL, gDefaultTextColor);
            diaGetColor(diaUIConfig, UICFG_SELCOL, gDefaultSelTextColor);
        }
        diaGetInt(diaUIConfig, UICFG_AUTOSORT, &gAutosort);
        diaGetInt(diaUIConfig, UICFG_AUTOREFRESH, &gAutoRefresh);
        diaGetInt(diaUIConfig, UICFG_NOTIFICATIONS, &gEnableNotifications);
        diaGetInt(diaUIConfig, UICFG_COVERART, &gEnableArt);
        diaGetInt(diaUIConfig, UICFG_WIDESCREEN, &gWideScreen);
        diaGetInt(diaUIConfig, UICFG_VMODE, &gVMode);
        diaGetInt(diaUIConfig, UICFG_XOFF, &gXOff);
        diaGetInt(diaUIConfig, UICFG_YOFF, &gYOff);
        diaGetInt(diaUIConfig, UICFG_OVERSCAN, &gOverscan);

        if (ret == UICFG_RESETCOL)
            setDefaultColors();

        applyConfig(themeID, langID);
        sfxInit(0);
    }

    if (previousVMode != gVMode) {
        if (guiConfirmVideoMode() == 0) {
            // Restore previous video mode, without changing the theme & language settings.
            gVMode = previousVMode;
            applyConfig(themeID, langID);
            goto reselect_video_mode;
        }
    }
}

static int netConfigUpdater(int modified)
{
    int showAdvancedOptions, isNetBIOS, isDHCPEnabled, i;

    if (modified) {
        diaGetInt(diaNetConfig, NETCFG_SHOW_ADVANCED_OPTS, &showAdvancedOptions);

        diaGetInt(diaNetConfig, NETCFG_PS2_IP_ADDR_TYPE, &isDHCPEnabled);
        diaGetInt(diaNetConfig, NETCFG_SHARE_ADDR_TYPE, &isNetBIOS);
        diaSetVisible(diaNetConfig, NETCFG_SHARE_NB_ADDR, isNetBIOS);

        for (i = 0; i < 4; i++) {
            diaSetVisible(diaNetConfig, NETCFG_SHARE_IP_ADDR_0 + i, !isNetBIOS);

            diaSetEnabled(diaNetConfig, NETCFG_PS2_IP_ADDR_0 + i, !isDHCPEnabled);
            diaSetEnabled(diaNetConfig, NETCFG_PS2_NETMASK_0 + i, !isDHCPEnabled);
            diaSetEnabled(diaNetConfig, NETCFG_PS2_GATEWAY_0 + i, !isDHCPEnabled);
            diaSetEnabled(diaNetConfig, NETCFG_PS2_DNS_0 + i, !isDHCPEnabled);
        }

        for (i = 0; i < 3; i++)
            diaSetVisible(diaNetConfig, NETCFG_SHARE_IP_ADDR_DOT_0 + i, !isNetBIOS);

        diaSetEnabled(diaNetConfig, NETCFG_SHARE_PORT, showAdvancedOptions);
        diaSetEnabled(diaNetConfig, NETCFG_ETHOPMODE, showAdvancedOptions);
    }

    return 0;
}

void guiShowNetConfig(void)
{
    size_t i;
    const char *ethOpModes[] = {_l(_STR_AUTO), _l(_STR_ETH_100MFDX), _l(_STR_ETH_100MHDX), _l(_STR_ETH_10MFDX), _l(_STR_ETH_10MHDX), NULL};
    const char *addrConfModes[] = {_l(_STR_ADDR_TYPE_IP), _l(_STR_ADDR_TYPE_NETBIOS), NULL};
    const char *ipAddrConfModes[] = {_l(_STR_IP_ADDRESS_TYPE_STATIC), _l(_STR_IP_ADDRESS_TYPE_DHCP), NULL};
    diaSetEnum(diaNetConfig, NETCFG_PS2_IP_ADDR_TYPE, ipAddrConfModes);
    diaSetEnum(diaNetConfig, NETCFG_SHARE_ADDR_TYPE, addrConfModes);
    diaSetEnum(diaNetConfig, NETCFG_ETHOPMODE, ethOpModes);

    // upload current values
    diaSetInt(diaNetConfig, NETCFG_SHOW_ADVANCED_OPTS, 0);
    diaSetEnabled(diaNetConfig, NETCFG_ETHOPMODE, 0);
    diaSetEnabled(diaNetConfig, NETCFG_SHARE_PORT, 0);

    diaSetInt(diaNetConfig, NETCFG_PS2_IP_ADDR_TYPE, ps2_ip_use_dhcp);
    diaSetInt(diaNetConfig, NETCFG_SHARE_ADDR_TYPE, gPCShareAddressIsNetBIOS);
    diaSetVisible(diaNetConfig, NETCFG_SHARE_NB_ADDR, gPCShareAddressIsNetBIOS);
    diaSetInt(diaNetConfig, NETCFG_SHARE_NB_ADDR, gPCShareAddressIsNetBIOS);
    diaSetString(diaNetConfig, NETCFG_SHARE_NB_ADDR, gPCShareNBAddress);

    for (i = 0; i < 4; ++i) {
        diaSetEnabled(diaNetConfig, NETCFG_PS2_IP_ADDR_0 + i, !ps2_ip_use_dhcp);
        diaSetEnabled(diaNetConfig, NETCFG_PS2_NETMASK_0 + i, !ps2_ip_use_dhcp);
        diaSetEnabled(diaNetConfig, NETCFG_PS2_GATEWAY_0 + i, !ps2_ip_use_dhcp);
        diaSetEnabled(diaNetConfig, NETCFG_PS2_DNS_0 + i, !ps2_ip_use_dhcp);

        diaSetVisible(diaNetConfig, NETCFG_SHARE_IP_ADDR_0 + i, !gPCShareAddressIsNetBIOS);
        diaSetInt(diaNetConfig, NETCFG_PS2_IP_ADDR_0 + i, ps2_ip[i]);
        diaSetInt(diaNetConfig, NETCFG_PS2_NETMASK_0 + i, ps2_netmask[i]);
        diaSetInt(diaNetConfig, NETCFG_PS2_GATEWAY_0 + i, ps2_gateway[i]);
        diaSetInt(diaNetConfig, NETCFG_PS2_DNS_0 + i, ps2_dns[i]);
        diaSetInt(diaNetConfig, NETCFG_SHARE_IP_ADDR_0 + i, pc_ip[i]);
    }

    for (i = 0; i < 3; ++i)
        diaSetVisible(diaNetConfig, NETCFG_SHARE_IP_ADDR_DOT_0 + i, !gPCShareAddressIsNetBIOS);

    diaSetInt(diaNetConfig, NETCFG_SHARE_PORT, gPCPort);
    diaSetString(diaNetConfig, NETCFG_SHARE_NAME, gPCShareName);
    diaSetString(diaNetConfig, NETCFG_SHARE_USERNAME, gPCUserName);
    diaSetString(diaNetConfig, NETCFG_SHARE_PASSWORD, gPCPassword);
    diaSetInt(diaNetConfig, NETCFG_ETHOPMODE, gETHOpMode);

    // Update the spacer item between the OK and reconnect buttons (See dialogs.c).
    if (gNetworkStartup == 0) {
        diaSetLabel(diaNetConfig, NETCFG_OK, _l(_STR_OK));
        diaSetVisible(diaNetConfig, NETCFG_RECONNECT, 1);
    } else if (gNetworkStartup >= ERROR_ETH_SMB_CONN) {
        diaSetLabel(diaNetConfig, NETCFG_OK, _l(_STR_RECONNECT));
        diaSetVisible(diaNetConfig, NETCFG_RECONNECT, 0);
    } else {
        diaSetLabel(diaNetConfig, NETCFG_OK, _l(_STR_OK));
        diaSetVisible(diaNetConfig, NETCFG_RECONNECT, 0);
    }

    int result = diaExecuteDialog(diaNetConfig, -1, 1, &netConfigUpdater);
    if (result) {
        // Store values
        diaGetInt(diaNetConfig, NETCFG_PS2_IP_ADDR_TYPE, &ps2_ip_use_dhcp);
        diaGetInt(diaNetConfig, NETCFG_SHARE_ADDR_TYPE, &gPCShareAddressIsNetBIOS);
        diaGetString(diaNetConfig, NETCFG_SHARE_NB_ADDR, gPCShareNBAddress, sizeof(gPCShareNBAddress));

        for (i = 0; i < 4; ++i) {
            diaGetInt(diaNetConfig, NETCFG_PS2_IP_ADDR_0 + i, &ps2_ip[i]);
            diaGetInt(diaNetConfig, NETCFG_PS2_NETMASK_0 + i, &ps2_netmask[i]);
            diaGetInt(diaNetConfig, NETCFG_PS2_GATEWAY_0 + i, &ps2_gateway[i]);
            diaGetInt(diaNetConfig, NETCFG_PS2_DNS_0 + i, &ps2_dns[i]);
            diaGetInt(diaNetConfig, NETCFG_SHARE_IP_ADDR_0 + i, &pc_ip[i]);
        }
        diaGetInt(diaNetConfig, NETCFG_ETHOPMODE, &gETHOpMode);

        diaGetInt(diaNetConfig, NETCFG_SHARE_PORT, &gPCPort);
        diaGetString(diaNetConfig, NETCFG_SHARE_NAME, gPCShareName, sizeof(gPCShareName));
        diaGetString(diaNetConfig, NETCFG_SHARE_USERNAME, gPCUserName, sizeof(gPCUserName));
        diaGetString(diaNetConfig, NETCFG_SHARE_PASSWORD, gPCPassword, sizeof(gPCPassword));

        if (result == NETCFG_RECONNECT && gNetworkStartup < ERROR_ETH_SMB_CONN)
            gNetworkStartup = ERROR_ETH_SMB_LOGON;

        applyConfig(-1, -1);
    }
}

void guiShowParentalLockConfig(void)
{
    int result;
    char password[CONFIG_KEY_VALUE_LEN];
    config_set_t *configOPL = configGetByType(CONFIG_OPL);

    // Set current values
    configGetStrCopy(configOPL, CONFIG_OPL_PARENTAL_LOCK_PWD, password, CONFIG_KEY_VALUE_LEN); // This will return the current password, or a blank string if it is not set.
    diaSetString(diaParentalLockConfig, CFG_PARENLOCK_PASSWORD, password);

    result = diaExecuteDialog(diaParentalLockConfig, -1, 1, NULL);
    if (result) {
        diaGetString(diaParentalLockConfig, CFG_PARENLOCK_PASSWORD, password, CONFIG_KEY_VALUE_LEN);

        if (strlen(password) > 0) {
            if (strncmp(OPL_PARENTAL_LOCK_MASTER_PASS, password, CONFIG_KEY_VALUE_LEN) != 0) {
                // Store password
                configSetStr(configOPL, CONFIG_OPL_PARENTAL_LOCK_PWD, password);
            } else {
                // Password not acceptable (i.e. master password entered).
                guiMsgBox(_l(_STR_PARENLOCK_INVALID_PASSWORD), 0, NULL);
            }
        } else {
            configRemoveKey(configOPL, CONFIG_OPL_PARENTAL_LOCK_PWD);

            guiMsgBox(_l(_STR_PARENLOCK_DISABLE_WARNING), 0, diaParentalLockConfig);
        }

        menuSetParentalLockCheckState(1);
    }
}

static void guiSetAudioSettingsState(void)
{
    diaGetInt(diaAudioConfig, CFG_SFX, &gEnableSFX);
    diaGetInt(diaAudioConfig, CFG_BOOT_SND, &gEnableBootSND);
    diaGetInt(diaAudioConfig, CFG_SFX_VOLUME, &gSFXVolume);
    diaGetInt(diaAudioConfig, CFG_BOOT_SND_VOLUME, &gBootSndVolume);
    sfxVolume();
}

static int guiAudioUpdater(int modified)
{
    if (modified) {
        guiSetAudioSettingsState();
    }

    return 0;
}

void guiShowAudioConfig(void)
{
    diaSetInt(diaAudioConfig, CFG_SFX, gEnableSFX);
    diaSetInt(diaAudioConfig, CFG_BOOT_SND, gEnableBootSND);
    diaSetInt(diaAudioConfig, CFG_SFX_VOLUME, gSFXVolume);
    diaSetInt(diaAudioConfig, CFG_BOOT_SND_VOLUME, gBootSndVolume);

    diaExecuteDialog(diaAudioConfig, -1, 1, guiAudioUpdater);
}

void guiShowControllerConfig(void)
{
    int value;

    // configure the enumerations
    const char *scrollSpeeds[] = {_l(_STR_SLOW), _l(_STR_MEDIUM), _l(_STR_FAST), NULL};
    const char *selectButtons[] = {_l(_STR_CIRCLE), _l(_STR_CROSS), NULL};

    diaSetEnum(diaControllerConfig, UICFG_SCROLL, scrollSpeeds);
    diaSetEnum(diaControllerConfig, CFG_SELECTBUTTON, selectButtons);

    diaSetInt(diaControllerConfig, UICFG_SCROLL, gScrollSpeed);
    diaSetInt(diaControllerConfig, CFG_SELECTBUTTON, gSelectButton == KEY_CIRCLE ? 0 : 1);

    int result = diaExecuteDialog(diaControllerConfig, -1, 1, NULL);
    if (result) {
        diaGetInt(diaControllerConfig, UICFG_SCROLL, &gScrollSpeed);

        if (diaGetInt(diaControllerConfig, CFG_SELECTBUTTON, &value))
            gSelectButton = value == 0 ? KEY_CIRCLE : KEY_CROSS;
        else
            gSelectButton = KEY_CIRCLE;
#ifdef PADEMU
        if (result == PADEMU_GLOBAL_BUTTON)
            guiGameShowPadEmuConfig(1);
#endif
        applyConfig(-1, -1);
    }
}

int guiShowKeyboard(char *value, int maxLength)
{
    char tmp[maxLength];
    strncpy(tmp, value, maxLength);

    int result = diaShowKeyb(tmp, maxLength, 0, NULL);
    if (result) {
        strncpy(value, tmp, maxLength);
        value[maxLength - 1] = '\0';
    }

    return result;
}

int guiGetOpCompleted(int opid)
{
    return gCompletedOps > opid;
}

int guiDeferUpdate(struct gui_update_t *op)
{
    WaitSema(gSemaId);

    struct gui_update_list_t *up = (struct gui_update_list_t *)malloc(sizeof(struct gui_update_list_t));
    up->item = op;
    up->next = NULL;

    if (!gUpdateList) {
        gUpdateList = up;
        gUpdateEnd = gUpdateList;
    } else {
        gUpdateEnd->next = up;
        gUpdateEnd = up;
    }

    SignalSema(gSemaId);

    return gScheduledOps++;
}

static void guiHandleOp(struct gui_update_t *item)
{
    submenu_list_t *result = NULL;

    switch (item->type) {
        case GUI_INIT_DONE:
            gInitComplete = 1;
            break;

        case GUI_OP_ADD_MENU:
            menuAppendItem(item->menu.menu);
            break;

        case GUI_OP_APPEND_MENU:
            result = submenuAppendItem(item->menu.subMenu, item->submenu.icon_id,
                                       item->submenu.text, item->submenu.id, item->submenu.text_id);

            if (!item->menu.menu->submenu) { // first subitem in list
                item->menu.menu->submenu = result;
                item->menu.menu->current = result;
                item->menu.menu->pagestart = result;
            } else if (item->submenu.selected) { // remember last played game feature
                item->menu.menu->current = result;
                item->menu.menu->pagestart = result;
                item->menu.menu->remindLast = 1;

                // Last Played Auto Start
                if ((gAutoStartLastPlayed) && !(KeyPressedOnce))
                    DisableCron = 0; // Release Auto Start Last Played counter
            }

            break;

        case GUI_OP_SELECT_MENU:
            menuSetSelectedItem(item->menu.menu);
            screenHandler = &screenHandlers[GUI_SCREEN_MAIN];
            break;

        case GUI_OP_CLEAR_SUBMENU:
            submenuDestroy(item->menu.subMenu);
            item->menu.menu->submenu = NULL;
            item->menu.menu->current = NULL;
            item->menu.menu->pagestart = NULL;
            break;

        case GUI_OP_SORT:
            submenuSort(item->menu.subMenu);
            item->menu.menu->submenu = *item->menu.subMenu;

            if (!item->menu.menu->remindLast)
                item->menu.menu->current = item->menu.menu->submenu;

            item->menu.menu->pagestart = item->menu.menu->current;
            break;

        case GUI_OP_ADD_HINT:
            // append the hint list in the menu item
            menuAddHint(item->menu.menu, item->hint.text_id, item->hint.icon_id);
            break;

        default:
            LOG("GUI: ??? (%d)\n", item->type);
    }
}

static void guiHandleDeferredOps(void)
{

    WaitSema(gSemaId);
    while (gUpdateList) {

        guiHandleOp(gUpdateList->item);

        struct gui_update_list_t *td = gUpdateList;
        gUpdateList = gUpdateList->next;

        free(td);

        gCompletedOps++;
    }
    SignalSema(gSemaId);

    gUpdateEnd = NULL;
}

void guiExecDeferredOps(void)
{
    // Clears deferred operations list by executing them.
    guiHandleDeferredOps();
}

static void guiDrawBusy(int alpha)
{
    if (gTheme->loadingIcon) {
        GSTEXTURE *texture = thmGetTexture(LOAD0_ICON + (guiFrameId >> 1) % gTheme->loadingIconCount);
        if (texture && texture->Mem) {
            u64 mycolor = GS_SETREG_RGBA(0x80, 0x80, 0x80, alpha);
            rmDrawPixmap(texture, gTheme->loadingIcon->posX, gTheme->loadingIcon->posY, gTheme->loadingIcon->aligned, gTheme->loadingIcon->width, gTheme->loadingIcon->height, gTheme->loadingIcon->scaled, mycolor);
        }
    }
}

static void guiRenderGreeting(int alpha)
{
    u64 mycolor = GS_SETREG_RGBA(0x1C, 0x1C, 0x1C, alpha);
    rmDrawRect(0, 0, screenWidth, screenHeight, mycolor);

    GSTEXTURE *logo = thmGetTexture(LOGO_PICTURE);
    if (logo) {
        mycolor = GS_SETREG_RGBA(0x80, 0x80, 0x80, alpha);
        rmDrawPixmap(logo, screenWidth >> 1, gTheme->usedHeight >> 1, ALIGN_CENTER, logo->Width, logo->Height, SCALING_RATIO, mycolor);
    }
}

static float mix(float a, float b, float t)
{
    return a + (b - a) * t;
}

static float fade(float t)
{
    return fadetbl[(int)(t * FADE_SIZE)];
}

// The same as mix, but with 8 (2*4) values mixed at once
static void VU0MixVec(VU_VECTOR *a, VU_VECTOR *b, float mix, VU_VECTOR *res)
{
    asm volatile(
#if __GNUC__ > 3
        "lqc2           $vf1, (%[a])\n"        // load the first vector
        "lqc2           $vf2, (%[b])\n"        // load the second vector
        "qmtc2          %[mix], $vf3\n"        // move the mix value from reg to VU
        "vaddw.x        $vf5, $vf0, $vf0\n"    // vf5.x = 1
        "vsub.x         $vf4x, $vf5x, $vf3x\n" // subtract 1 - vf3,x, store the result in vf4.x
        "vmulax.xyzw    $ACC, $vf1, $vf3x\n"   // multiply vf1 by vf3.x, store the result in ACC
        "vmaddx.xyzw    $vf1, $vf2, $vf4x\n"   // multiply vf2 by vf4.x add ACC, store the result in vf1
        "sqc2           $vf1, (%[res])\n"      // transfer the result in acc to the ee
#else
        "lqc2           vf1, (%[a])\n"      // load the first vector
        "lqc2           vf2, (%[b])\n"      // load the second vector
        "qmtc2          %[mix], vf3\n"      // move the mix value from reg to VU
        "vaddw.x        vf5, vf00, vf00\n"  // vf5.x = 1
        "vsub.x         vf4x, vf5x, vf3x\n" // subtract 1 - vf3,x, store the result in vf4.x
        "vmulax.xyzw    ACC, vf1, vf3x\n"   // multiply vf1 by vf3.x, store the result in ACC
        "vmaddx.xyzw    vf1, vf2, vf4x\n"   // multiply vf2 by vf4.x add ACC, store the result in vf1
        "sqc2           vf1, (%[res])\n"    // transfer the result in acc to the ee
#endif
        : [res] "+r"(res), "=m"(*res)
        : [a] "r"(a), [b] "r"(b), [mix] "r"(mix), "m"(*a), "m"(*b));
}

static float guiCalcPerlin(float x, float y, float z)
{
    // Taken from: http://people.opera.com/patrickl/experiments/canvas/plasma/perlin-noise-classical.js
    // By Sean McCullough

    // Find unit grid cell containing point
    int X = floorf(x);
    int Y = floorf(y);
    int Z = floorf(z);

    // Get relative xyz coordinates of point within that cell
    x = x - X;
    y = y - Y;
    z = z - Z;

    // Wrap the integer cells at 255 (smaller integer period can be introduced here)
    X = X & 255;
    Y = Y & 255;
    Z = Z & 255;

    // Calculate a set of eight hashed gradient indices
    int gi000 = pperm[X + pperm[Y + pperm[Z]]] % 12;
    int gi001 = pperm[X + pperm[Y + pperm[Z + 1]]] % 12;
    int gi010 = pperm[X + pperm[Y + 1 + pperm[Z]]] % 12;
    int gi011 = pperm[X + pperm[Y + 1 + pperm[Z + 1]]] % 12;
    int gi100 = pperm[X + 1 + pperm[Y + pperm[Z]]] % 12;
    int gi101 = pperm[X + 1 + pperm[Y + pperm[Z + 1]]] % 12;
    int gi110 = pperm[X + 1 + pperm[Y + 1 + pperm[Z]]] % 12;
    int gi111 = pperm[X + 1 + pperm[Y + 1 + pperm[Z + 1]]] % 12;

    // The gradients of each corner are now:
    // g000 = grad3[gi000];
    // g001 = grad3[gi001];
    // g010 = grad3[gi010];
    // g011 = grad3[gi011];
    // g100 = grad3[gi100];
    // g101 = grad3[gi101];
    // g110 = grad3[gi110];
    // g111 = grad3[gi111];
    // Calculate noise contributions from each of the eight corners
    VU_VECTOR vec;
    vec.x = x;
    vec.y = y;
    vec.z = z;
    vec.w = 1;

    VU_VECTOR a, b;

    // float n000
    a.x = Vu0DotProduct(&pgrad3[gi000], &vec);

    vec.y -= 1;

    // float n010
    a.z = Vu0DotProduct(&pgrad3[gi010], &vec);

    vec.x -= 1;

    // float n110
    b.z = Vu0DotProduct(&pgrad3[gi110], &vec);

    vec.y += 1;

    // float n100
    b.x = Vu0DotProduct(&pgrad3[gi100], &vec);

    vec.z -= 1;

    // float n101
    b.y = Vu0DotProduct(&pgrad3[gi101], &vec);

    vec.y -= 1;

    // float n111
    b.w = Vu0DotProduct(&pgrad3[gi111], &vec);

    vec.x += 1;

    // float n011
    a.w = Vu0DotProduct(&pgrad3[gi011], &vec);

    vec.y += 1;

    // float n001
    a.y = Vu0DotProduct(&pgrad3[gi001], &vec);

    // Compute the fade curve value for each of x, y, z
    float u = fade(x);
    float v = fade(y);
    float w = fade(z);

    // TODO: Low priority... This could be done on VU0 (xyzw for the first 4 mixes)
    // The result in sw
    // Interpolate along x the contributions from each of the corners
    VU_VECTOR rv;
    VU0MixVec(&b, &a, u, &rv);

    // TODO: The VU0MixVec could as well mix the results (as follows) - might improve performance...
    // Interpolate the four results along y
    float nxy0 = mix(rv.x, rv.z, v);
    float nxy1 = mix(rv.y, rv.w, v);
    // Interpolate the two last results along z
    float nxyz = mix(nxy0, nxy1, w);

    return nxyz;
}

static float dir = 0.02;
static float perz = -100;
static int pery = 0;
static unsigned char curbgColor[3] = {0, 0, 0};

static int cdirection(unsigned char a, unsigned char b)
{
    if (a == b)
        return 0;
    else if (a > b)
        return -1;
    else
        return 1;
}

void guiDrawBGPlasma()
{
    int x, y;

    // transition the colors
    curbgColor[0] += cdirection(curbgColor[0], gTheme->bgColor[0]);
    curbgColor[1] += cdirection(curbgColor[1], gTheme->bgColor[1]);
    curbgColor[2] += cdirection(curbgColor[2], gTheme->bgColor[2]);

    // it's PLASMA_ROWS_PER_FRAME rows a frame to stop being a resource hog
    if (pery >= PLASMA_H) {
        pery = 0;
        perz += dir;

        if (perz > 100.0f || perz < -100.0f)
            dir = -dir;
    }

    u32 *buf = gBackgroundTex.Mem + PLASMA_W * pery;
    int ymax = pery + PLASMA_ROWS_PER_FRAME;

    if (ymax > PLASMA_H)
        ymax = PLASMA_H;

    for (y = pery; y < ymax; y++) {
        for (x = 0; x < PLASMA_W; x++) {
            u32 fper = guiCalcPerlin((float)(2 * x) / PLASMA_W, (float)(2 * y) / PLASMA_H, perz) * 0x80 + 0x80;

            *buf = GS_SETREG_RGBA(
                (u32)(fper * curbgColor[0]) >> 8,
                (u32)(fper * curbgColor[1]) >> 8,
                (u32)(fper * curbgColor[2]) >> 8,
                0x80);

            ++buf;
        }
    }

    pery = ymax;
    rmInvalidateTexture(&gBackgroundTex);
    rmDrawPixmap(&gBackgroundTex, 0, 0, ALIGN_NONE, screenWidth, screenHeight, SCALING_NONE, gDefaultCol);
}

int guiDrawIconAndText(int iconId, int textId, int font, int x, int y, u64 color)
{
    GSTEXTURE *iconTex = thmGetTexture(iconId);
    int w = (iconTex->Width * 20) / iconTex->Height;
    int h = 20;

    if (iconTex && iconTex->Mem) {
        y += h >> 1;
        rmDrawPixmap(iconTex, x, y, ALIGN_VCENTER, w, h, SCALING_RATIO, gDefaultCol);
        x += rmWideScale(w) + 2;
    } else {
        // HACK: font is aligned to VCENTER, the default height icon height is 20
        y += 10;
    }

    x = fntRenderString(font, x, y, ALIGN_VCENTER, 0, 0, _l(textId), color);

    return x;
}

int guiAlignMenuHints(menu_hint_item_t *hint, int font, int width)
{
    int x = screenWidth;
    int w;

    for (; hint; hint = hint->next) {
        GSTEXTURE *iconTex = thmGetTexture(hint->icon_id);
        w = (iconTex->Width * 20) / iconTex->Height;
        char *text = _l(hint->text_id);

        x -= rmWideScale(w) + 2;
        x -= rmUnScaleX(fntCalcDimensions(font, text));
        if (hint->next != NULL)
            x -= width;
    }

    // align center
    x /= 2;

    return x;
}

int guiAlignSubMenuHints(int hintCount, int *textID, int *iconID, int font, int width, int align)
{
    int x = screenWidth;
    int i, w;

    for (i = 0; i < hintCount; i++) {
        GSTEXTURE *iconTex = thmGetTexture(iconID[i]);
        w = (iconTex->Width * 20) / iconTex->Height;
        char *text = _l(textID[i]);

        x -= rmWideScale(w) + 2;
        x -= rmUnScaleX(fntCalcDimensions(font, text));
        if (i != (hintCount - 1))
            x -= width;
    }

    if (align == 1) // align center
        x /= 2;

    if (align == 2) // align right
        x -= 20;

    return x;
}

void guiDrawSubMenuHints(void)
{
    int subMenuHints[2] = {_STR_SELECT, _STR_GAMES_LIST};
    int subMenuIcons[2] = {CIRCLE_ICON, CROSS_ICON};

    int x = guiAlignSubMenuHints(2, subMenuHints, subMenuIcons, gTheme->fonts[0], 12, 2);
    int y = gTheme->usedHeight - 32;

    x = guiDrawIconAndText(gSelectButton == KEY_CIRCLE ? subMenuIcons[0] : subMenuIcons[1], subMenuHints[0], gTheme->fonts[0], x, y, gTheme->textColor);
    x += 12;
    x = guiDrawIconAndText(gSelectButton == KEY_CIRCLE ? subMenuIcons[1] : subMenuIcons[0], subMenuHints[1], gTheme->fonts[0], x, y, gTheme->textColor);
}

static int endIntro = 0; // Break intro loop and start 'Last Played Auto Start' countdown
static void guiDrawOverlays()
{
    // are there any pending operations?
    int pending = ioHasPendingRequests();
    static int busyAlpha = 0x00; // Fully transparant

    if (!pending) {
        // Fade out
        if (busyAlpha > 0x00)
            busyAlpha -= 0x02;
    } else {
        // Fade in
        if (busyAlpha < 0x80)
            busyAlpha += 0x02;
    }

    if (busyAlpha > 0x00)
        guiDrawBusy(busyAlpha);

#ifdef __DEBUG
    char text[20];
    int x = screenWidth - 120;
    int y = 15;
    int yadd = 15;

    snprintf(text, sizeof(text), "VRAM:");
    fntRenderString(gTheme->fonts[0], x, y, ALIGN_LEFT, 0, 0, text, GS_SETREG_RGBA(0x60, 0x60, 0x60, 0x80));
    y += yadd;

    snprintf(text, sizeof(text), "%dKiB FIXED", gsGlobal->CurrentPointer / 1024);
    fntRenderString(gTheme->fonts[0], x, y, ALIGN_LEFT, 0, 0, text, GS_SETREG_RGBA(0x60, 0x60, 0x60, 0x80));
    y += yadd;

    snprintf(text, sizeof(text), "%dKiB TEXMAN", ((4 * 1024 * 1024) - gsGlobal->CurrentPointer) / 1024);
    fntRenderString(gTheme->fonts[0], x, y, ALIGN_LEFT, 0, 0, text, GS_SETREG_RGBA(0x60, 0x60, 0x60, 0x80));
    y += yadd;
    y += yadd; // Empty line

    if (prevtime != 0) {
        clock_t diff = curtime - prevtime;
        // Raw FPS value with 2 decimal places
        float rawfps = ((100 * CLOCKS_PER_SEC) / diff) / 100.0f;

        if (fps == 0.0f)
            fps = rawfps;
        else
            fps = fps * 0.9f + rawfps / 10.0f; // Smooth FPS value

        snprintf(text, sizeof(text), "%.1f FPS", fps);
        fntRenderString(gTheme->fonts[0], x, y, ALIGN_LEFT, 0, 0, text, GS_SETREG_RGBA(0x60, 0x60, 0x60, 0x80));
        y += yadd;
    }
#endif

    // Last Played Auto Start
    if (!pending && DisableCron == 0 && endIntro) {
        if (CronStart == 0) {
            CronStart = clock() / CLOCKS_PER_SEC;
        } else {
            char strAutoStartInNSecs[21];
            clock_t CronCurrent;

            CronCurrent = clock() / CLOCKS_PER_SEC;
            RemainSecs = gAutoStartLastPlayed - (CronCurrent - CronStart);
            snprintf(strAutoStartInNSecs, sizeof(strAutoStartInNSecs), _l(_STR_AUTO_START_IN_N_SECS), RemainSecs);
            fntRenderString(gTheme->fonts[0], screenWidth / 2, screenHeight / 2, ALIGN_CENTER, 0, 0, strAutoStartInNSecs, GS_SETREG_RGBA(0x60, 0x60, 0x60, 0x80));
        }
    }

    // BLURT output
    // if (!gDisableDebug)
    //     fntRenderString(gTheme->fonts[0], 0, screenHeight - 24, ALIGN_NONE, 0, 0, blurttext, GS_SETREG_RGBA(255, 255, 0, 128));
}

static void guiReadPads()
{
    if (readPads())
        guiInactiveFrames = 0;
    else
        guiInactiveFrames++;
}

// renders the screen and handles inputs. Also handles screen transitions between numerous
// screen handlers. Fade transition code written by Maximus32
static void guiShow()
{
    // is there a transmission effect going on or are
    // we in a normal rendering state?
    if (screenHandlerTarget) {
        u8 alpha;
        const u8 transition_frames = 26;
        if (transIndex < (transition_frames / 2)) {
            // Fade-out old screen
            // index: 0..7
            // alpha: 1..8 * transition_step
            screenHandler->renderScreen();
            alpha = fade((float)(transIndex + 1) / (transition_frames / 2)) * 0x80;
        } else {
            // Fade-in new screen
            // index: 8..15
            // alpha: 8..1 * transition_step
            screenHandlerTarget->renderScreen();
            alpha = fade((float)(transition_frames - transIndex) / (transition_frames / 2)) * 0x80;
        }

        // Overlay the actual "fade"
        rmDrawRect(0, 0, screenWidth, screenHeight, GS_SETREG_RGBA(0x00, 0x00, 0x00, alpha));

        // Advance the effect
        transIndex++;
        if (transIndex >= transition_frames) {
            screenHandler = screenHandlerTarget;
            screenHandlerTarget = NULL;
        }
    } else
        // render with the set screen handler
        screenHandler->renderScreen();
}

void guiIntroLoop(void)
{
    int greetingAlpha = 0x80;
    const int fadeFrameCount = 0x80 / 2;
    const int fadeDuration = (fadeFrameCount * 1000) / 55; // Average between 50 and 60 fps
    clock_t tFadeDelayEnd = 0;

    while (!endIntro) {
        guiStartFrame();

        if (greetingAlpha < 0x80)
            guiShow();

        if (greetingAlpha > 0)
            guiRenderGreeting(greetingAlpha);

        // Initialize boot sound
        if (gInitComplete && !tFadeDelayEnd && gEnableBootSND) {
            // Start playing sound
            sfxPlay(SFX_BOOT);
            // Calculate transition delay
            tFadeDelayEnd = clock() + (sfxGetSoundDuration(SFX_BOOT) - fadeDuration) * (CLOCKS_PER_SEC / 1000);
        }

        if (gInitComplete && clock() >= tFadeDelayEnd)
            greetingAlpha -= 2;

        if (greetingAlpha <= 0)
            endIntro = 1;

        guiDrawOverlays();

        guiHandleDeferredOps();

        guiEndFrame();

        if (!screenHandlerTarget && screenHandler)
            screenHandler->handleInput();
    }
}

void guiMainLoop(void)
{
    guiResetNotifications();
    guiCheckNotifications(1, 1);

    while (!gTerminate) {
        guiStartFrame();

        // Read the pad states to prepare for input processing in the screen handler
        guiReadPads();

        // handle inputs and render screen
        guiShow();

        // Render overlaying gui thingies :)
        guiDrawOverlays();

        if (gEnableNotifications)
            guiShowNotifications();

        // handle deferred operations
        guiHandleDeferredOps();

        guiEndFrame();

        // if not transiting, handle input
        // done here so we can use renderman if needed
        if (!screenHandlerTarget && screenHandler)
            screenHandler->handleInput();

        if (gFrameHook)
            gFrameHook();
    }
}

void guiSetFrameHook(gui_callback_t cback)
{
    gFrameHook = cback;
}

void guiSwitchScreen(int target)
{
    sfxPlay(SFX_TRANSITION);
    transIndex = 0;
    screenHandlerTarget = &screenHandlers[target];
}

struct gui_update_t *guiOpCreate(gui_op_type_t type)
{
    struct gui_update_t *op = (struct gui_update_t *)malloc(sizeof(struct gui_update_t));
    memset(op, 0, sizeof(struct gui_update_t));
    op->type = type;
    return op;
}

void guiUpdateScrollSpeed(void)
{
    // sanitize the settings
    if ((gScrollSpeed < 0) || (gScrollSpeed > 2))
        gScrollSpeed = 1;

    // update the pad delays for KEY_UP and KEY_DOWN
    // default delay is 7
    // fast - 100 ms
    // medium - 300 ms
    // slow - 500 ms
    setButtonDelay(KEY_UP, 500 - gScrollSpeed * 200); // 0,1,2 -> 500, 300, 100
    setButtonDelay(KEY_DOWN, 500 - gScrollSpeed * 200);
}

void guiUpdateScreenScale(void)
{
    fntUpdateAspectRatio();
}

int guiMsgBox(const char *text, int addAccept, struct UIItem *ui)
{
    int terminate = 0;

    sfxPlay(SFX_MESSAGE);

    while (!terminate) {
        guiStartFrame();

        readPads();

        if (getKeyOn(gSelectButton == KEY_CIRCLE ? KEY_CROSS : KEY_CIRCLE))
            terminate = 1;
        else if (getKeyOn(gSelectButton))
            terminate = 2;

        if (ui)
            diaRenderUI(ui, screenHandler->inMenu, NULL, 0);
        else
            guiShow();

        rmDrawRect(0, 0, screenWidth, screenHeight, gColDarker);

        rmDrawLine(50, 75, screenWidth - 50, 75, gColWhite);
        rmDrawLine(50, 410, screenWidth - 50, 410, gColWhite);

        fntRenderString(gTheme->fonts[0], screenWidth >> 1, gTheme->usedHeight >> 1, ALIGN_CENTER, 0, 0, text, gTheme->textColor);
        guiDrawIconAndText(gSelectButton == KEY_CIRCLE ? CROSS_ICON : CIRCLE_ICON, _STR_BACK, gTheme->fonts[0], 500, 417, gTheme->selTextColor);
        if (addAccept)
            guiDrawIconAndText(gSelectButton == KEY_CIRCLE ? CIRCLE_ICON : CROSS_ICON, _STR_ACCEPT, gTheme->fonts[0], 70, 417, gTheme->selTextColor);

        guiEndFrame();
    }

    if (terminate == 1) {
        sfxPlay(SFX_CANCEL);
    }
    if (terminate == 2) {
        sfxPlay(SFX_CONFIRM);
    }

    return terminate - 1;
}

void guiHandleDeferedIO(int *ptr, const char *message, int type, void *data)
{
    ioPutRequest(type, data);

    while (*ptr)
        guiRenderTextScreen(message);
}

void guiGameHandleDeferedIO(int *ptr, struct UIItem *ui, int type, void *data)
{
    ioPutRequest(type, data);

    while (*ptr) {
        guiStartFrame();
        if (ui)
            diaRenderUI(ui, screenHandler->inMenu, NULL, 0);
        else
            guiShow();
        guiEndFrame();
    }
}

void guiRenderTextScreen(const char *message)
{
    guiStartFrame();

    guiShow();

    rmDrawRect(0, 0, screenWidth, screenHeight, gColDarker);

    fntRenderString(gTheme->fonts[0], screenWidth >> 1, gTheme->usedHeight >> 1, ALIGN_CENTER, 0, 0, message, gTheme->textColor);

    guiDrawOverlays();

    guiEndFrame();
}

void guiWarning(const char *text, int count)
{
    guiStartFrame();

    guiShow();

    rmDrawRect(0, 0, screenWidth, screenHeight, gColDarker);

    rmDrawLine(50, 75, screenWidth - 50, 75, gColWhite);
    rmDrawLine(50, 410, screenWidth - 50, 410, gColWhite);

    fntRenderString(gTheme->fonts[0], screenWidth >> 1, gTheme->usedHeight >> 1, ALIGN_CENTER, screenWidth, screenHeight, text, gTheme->textColor);

    guiEndFrame();

    delay(count);
}

int guiConfirmVideoMode(void)
{
    clock_t timeEnd;
    int terminate = 0;

    sfxPlay(SFX_MESSAGE);

    timeEnd = clock() + OPL_VMODE_CHANGE_CONFIRMATION_TIMEOUT_MS * (CLOCKS_PER_SEC / 1000);
    while (!terminate) {
        guiStartFrame();

        readPads();

        if (getKeyOn(gSelectButton == KEY_CIRCLE ? KEY_CROSS : KEY_CIRCLE))
            terminate = 1;
        else if (getKeyOn(gSelectButton))
            terminate = 2;

        // If the user fails to respond within the timeout period, deem it as a cancel operation.
        if (clock() > timeEnd)
            terminate = 1;

        guiShow();

        rmDrawRect(0, 0, screenWidth, screenHeight, gColDarker);

        rmDrawLine(50, 75, screenWidth - 50, 75, gColWhite);
        rmDrawLine(50, 410, screenWidth - 50, 410, gColWhite);

        fntRenderString(gTheme->fonts[0], screenWidth >> 1, gTheme->usedHeight >> 1, ALIGN_CENTER, 0, 0, _l(_STR_CFM_VMODE_CHG), gTheme->textColor);
        guiDrawIconAndText(gSelectButton == KEY_CIRCLE ? CROSS_ICON : CIRCLE_ICON, _STR_BACK, gTheme->fonts[0], 500, 417, gTheme->selTextColor);
        guiDrawIconAndText(gSelectButton == KEY_CIRCLE ? CIRCLE_ICON : CROSS_ICON, _STR_ACCEPT, gTheme->fonts[0], 70, 417, gTheme->selTextColor);

        guiEndFrame();
    }

    if (terminate == 1) {
        sfxPlay(SFX_CANCEL);
    }
    if (terminate == 2) {
        sfxPlay(SFX_CONFIRM);
    }

    return terminate - 1;
}

int guiGameShowRemoveSettings(config_set_t *configSet, config_set_t *configGame)
{
    int terminate = 0;
    char message[256];

    sfxPlay(SFX_MESSAGE);

    while (!terminate) {
        guiStartFrame();

        readPads();

        if (getKeyOn(gSelectButton == KEY_CIRCLE ? KEY_CROSS : KEY_CIRCLE))
            terminate = 1;
        else if (getKeyOn(gSelectButton))
            terminate = 2;
        else if (getKeyOn(KEY_SQUARE))
            terminate = 3;
        else if (getKeyOn(KEY_TRIANGLE))
            terminate = 4;

        guiShow();

        rmDrawRect(0, 0, screenWidth, screenHeight, gColDarker);

        rmDrawLine(50, 75, screenWidth - 50, 75, gColWhite);
        rmDrawLine(50, 410, screenWidth - 50, 410, gColWhite);

        fntRenderString(gTheme->fonts[0], screenWidth >> 1, gTheme->usedHeight >> 1, ALIGN_CENTER, 0, 0, _l(_STR_GAME_SETTINGS_PROMPT), gTheme->textColor);

        guiDrawIconAndText(gSelectButton == KEY_CIRCLE ? CROSS_ICON : CIRCLE_ICON, _STR_BACK, gTheme->fonts[0], 500, 417, gTheme->selTextColor);
        guiDrawIconAndText(SQUARE_ICON, _STR_GLOBAL_SETTINGS, gTheme->fonts[0], 213, 417, gTheme->selTextColor);
        guiDrawIconAndText(TRIANGLE_ICON, _STR_ALL_SETTINGS, gTheme->fonts[0], 356, 417, gTheme->selTextColor);
        guiDrawIconAndText(gSelectButton == KEY_CIRCLE ? CIRCLE_ICON : CROSS_ICON, _STR_PERGAME_SETTINGS, gTheme->fonts[0], 70, 417, gTheme->selTextColor);

        guiEndFrame();
    }

    if (terminate == 1) {
        sfxPlay(SFX_CANCEL);
        return 0;
    } else if (terminate == 2) {
        guiGameRemoveSettings(configSet);
        snprintf(message, sizeof(message), _l(_STR_GAME_SETTINGS_REMOVED), _l(_STR_PERGAME_SETTINGS));
    } else if (terminate == 3) {
        guiGameRemoveGlobalSettings(configGame);
        snprintf(message, sizeof(message), _l(_STR_GAME_SETTINGS_REMOVED), _l(_STR_GLOBAL_SETTINGS));
    } else if (terminate == 4) {
        guiGameRemoveSettings(configSet);
        guiGameRemoveGlobalSettings(configGame);
        snprintf(message, sizeof(message), _l(_STR_GAME_SETTINGS_REMOVED), _l(_STR_ALL_SETTINGS));
    }
    sfxPlay(SFX_CONFIRM);
    guiMsgBox(message, 0, NULL);

    return 1;
}
