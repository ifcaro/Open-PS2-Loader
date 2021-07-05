#include "include/dialogs.h"
#include "include/opl.h"
#include "include/dia.h"
#include "include/lang.h"
#include "include/gui.h"
#include "include/guigame.h"

#include <stdio.h>

// Network Config Menu
struct UIItem diaNetConfig[] = {
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_NETCONFIG}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_SHOW_ADVANCED_OPTS}}},
    {UI_SPACER},
    {UI_BOOL, NETCFG_SHOW_ADVANCED_OPTS, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_ETH_OPMODE}}},
    {UI_SPACER},
    {UI_ENUM, NETCFG_ETHOPMODE, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_CAT_PS2}}},
    {UI_BREAK},

    // ---- IP address type ----
    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_IP_ADDRESS_TYPE}}},
    {UI_SPACER},
    {UI_ENUM, NETCFG_PS2_IP_ADDR_TYPE, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    // ---- IP address ----
    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_IP_ADDRESS}}},
    {UI_SPACER},
    {UI_INT, NETCFG_PS2_IP_ADDR_0, 1, 1, -1, 0, 0, {.intvalue = {192, 192, 0, 255}}},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {".", -1}}},
    {UI_INT, NETCFG_PS2_IP_ADDR_1, 1, 1, -1, 0, 0, {.intvalue = {168, 168, 0, 255}}},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {".", -1}}},
    {UI_INT, NETCFG_PS2_IP_ADDR_2, 1, 1, -1, 0, 0, {.intvalue = {0, 0, 0, 255}}},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {".", -1}}},
    {UI_INT, NETCFG_PS2_IP_ADDR_3, 1, 1, -1, 0, 0, {.intvalue = {10, 10, 0, 255}}},
    {UI_BREAK},

    //  ---- Netmask ----
    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_MASK}}},
    {UI_SPACER},
    {UI_INT, NETCFG_PS2_NETMASK_0, 1, 1, -1, 0, 0, {.intvalue = {255, 255, 0, 255}}},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {".", -1}}},
    {UI_INT, NETCFG_PS2_NETMASK_1, 1, 1, -1, 0, 0, {.intvalue = {255, 255, 0, 255}}},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {".", -1}}},
    {UI_INT, NETCFG_PS2_NETMASK_2, 1, 1, -1, 0, 0, {.intvalue = {255, 255, 0, 255}}},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {".", -1}}},
    {UI_INT, NETCFG_PS2_NETMASK_3, 1, 1, -1, 0, 0, {.intvalue = {0, 0, 0, 255}}},
    {UI_BREAK},

    //  ---- Gateway ----
    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_GATEWAY}}},
    {UI_SPACER},
    {UI_INT, NETCFG_PS2_GATEWAY_0, 1, 1, -1, 0, 0, {.intvalue = {192, 192, 0, 255}}},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {".", -1}}},
    {UI_INT, NETCFG_PS2_GATEWAY_1, 1, 1, -1, 0, 0, {.intvalue = {168, 168, 0, 255}}},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {".", -1}}},
    {UI_INT, NETCFG_PS2_GATEWAY_2, 1, 1, -1, 0, 0, {.intvalue = {0, 0, 0, 255}}},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {".", -1}}},
    {UI_INT, NETCFG_PS2_GATEWAY_3, 1, 1, -1, 0, 0, {.intvalue = {1, 1, 0, 255}}},
    {UI_BREAK},

    //  ---- DNS server ----
    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_DNS_SERVER}}},
    {UI_SPACER},
    {UI_INT, NETCFG_PS2_DNS_0, 1, 1, -1, 0, 0, {.intvalue = {192, 192, 0, 255}}},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {".", -1}}},
    {UI_INT, NETCFG_PS2_DNS_1, 1, 1, -1, 0, 0, {.intvalue = {168, 168, 0, 255}}},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {".", -1}}},
    {UI_INT, NETCFG_PS2_DNS_2, 1, 1, -1, 0, 0, {.intvalue = {0, 0, 0, 255}}},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {".", -1}}},
    {UI_INT, NETCFG_PS2_DNS_3, 1, 1, -1, 0, 0, {.intvalue = {1, 1, 0, 255}}},
    {UI_SPLITTER},

    //  ---- SMB Server ----
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_CAT_SMB_SERVER}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_ADDRESS_TYPE}}},
    {UI_SPACER},
    {UI_ENUM, NETCFG_SHARE_ADDR_TYPE, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_ADDRESS}}},
    {UI_SPACER},
    {UI_STRING, NETCFG_SHARE_NB_ADDR, 1, 0, -1, 0, 0, {.stringvalue = {"", "", NULL}}},
    {UI_INT, NETCFG_SHARE_IP_ADDR_0, 1, 1, -1, 0, 0, {.intvalue = {192, 192, 0, 255}}},
    {UI_LABEL, NETCFG_SHARE_IP_ADDR_DOT_0, 1, 1, -1, 0, 0, {.label = {".", -1}}},
    {UI_INT, NETCFG_SHARE_IP_ADDR_1, 1, 1, -1, 0, 0, {.intvalue = {168, 168, 0, 255}}},
    {UI_LABEL, NETCFG_SHARE_IP_ADDR_DOT_1, 1, 1, -1, 0, 0, {.label = {".", -1}}},
    {UI_INT, NETCFG_SHARE_IP_ADDR_2, 1, 1, -1, 0, 0, {.intvalue = {0, 0, 0, 255}}},
    {UI_LABEL, NETCFG_SHARE_IP_ADDR_DOT_2, 1, 1, -1, 0, 0, {.label = {".", -1}}},
    {UI_INT, NETCFG_SHARE_IP_ADDR_3, 1, 1, -1, 0, 0, {.intvalue = {1, 1, 0, 255}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_PORT}}},
    {UI_SPACER},
    {UI_INT, NETCFG_SHARE_PORT, 1, 1, -1, 0, 0, {.intvalue = {445, 445, 0, 1024}}},
    {UI_BREAK},

    {UI_BREAK},

    //  ---- SMB share name ----
    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_SHARE}}},
    {UI_SPACER},
    {UI_STRING, NETCFG_SHARE_NAME, 1, 1, _STR_HINT_SHARENAME, 0, 0, {.stringvalue = {"PS2SMB", "PS2SMB", NULL}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_USER}}},
    {UI_SPACER},
    {UI_STRING, NETCFG_SHARE_USERNAME, 1, 1, -1, 0, 0, {.stringvalue = {"GUEST", "GUEST", NULL}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_PASSWORD}}},
    {UI_SPACER},
    {UI_PASSWORD, NETCFG_SHARE_PASSWORD, 1, 1, _STR_HINT_GUEST, 0, 0, {.stringvalue = {"", "", NULL}}},
    {UI_BREAK},

    // buttons
    {UI_BREAK},
    {UI_BUTTON, NETCFG_OK, 1, 1, -1, 0, 0, {.label = {NULL, -1}}},
    {UI_SPACER},
    {UI_BUTTON, NETCFG_RECONNECT, 1, 1, -1, 0, 0, {.label = {NULL, _STR_RECONNECT}}},
    {UI_BREAK},

    // end of dialog
    {UI_TERMINATOR}};

// Settings Menu
struct UIItem diaConfig[] = {
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_SETTINGS}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_DEBUG}}},
    {UI_SPACER},
    {UI_BOOL, CFG_DEBUG, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_PS2LOGO}}},
    {UI_SPACER},
    {UI_BOOL, CFG_PS2LOGO, 1, 1, _STR_HINT_PS2LOGO, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_CACHE_HDD_GAME_LIST}}},
    {UI_SPACER},
    {UI_BOOL, CFG_HDDGAMELISTCACHE, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_EXITTO}}},
    {UI_SPACER},
    {UI_STRING, CFG_EXITTO, 1, 1, _STR_HINT_EXITPATH, 0, 0, {.stringvalue = {"", "", NULL}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_ENABLE_WRITE}}},
    {UI_SPACER},
    {UI_BOOL, CFG_ENWRITEOP, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_LASTPLAYED}}},
    {UI_SPACER},
    {UI_BOOL, CFG_LASTPLAYED, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_SPACER},
    {UI_LABEL, CFG_LBL_AUTOSTARTLAST, 1, 1, -1, 0, 0, {.label = {NULL, _STR_AUTOSTARTLAST}}},
    {UI_SPACER},
    {UI_INT, CFG_AUTOSTARTLAST, 1, 1, _STR_HINT_AUTOSTARTLAST, 0, 0, {.intvalue = {0, 0, 0, 9}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_BDM_PREFIX}}},
    {UI_SPACER},
    {UI_STRING, CFG_BDMPREFIX, 1, 1, -1, 0, 0, {.stringvalue = {"", "", NULL}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_ETH_PREFIX}}},
    {UI_SPACER},
    {UI_STRING, CFG_ETHPREFIX, 1, 1, -1, 0, 0, {.stringvalue = {"", "", NULL}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_HDD_SPINDOWN}}},
    {UI_SPACER},
    {UI_INT, CFG_HDDSPINDOWN, 1, 1, _STR_HINT_SPINDOWN, 0, 0, {.intvalue = {20, 20, 0, 20}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_BDMMODE}}},
    {UI_SPACER},
    {UI_ENUM, CFG_BDMMODE, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {"- FireWire", -1}}},
    {UI_SPACER},
    {UI_BOOL, CFG_ENABLEFW, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_HDDMODE}}},
    {UI_SPACER},
    {UI_ENUM, CFG_HDDMODE, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_ETHMODE}}},
    {UI_SPACER},
    {UI_ENUM, CFG_ETHMODE, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_APPMODE}}},
    {UI_SPACER},
    {UI_ENUM, CFG_APPMODE, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_DEFDEVICE}}},
    {UI_SPACER},
    {UI_ENUM, CFG_DEFDEVICE, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    // buttons
    {UI_OK, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_OK}}},
    {UI_BREAK},

    // end of dialog
    {UI_TERMINATOR}};

// Display Settings Menu
struct UIItem diaUIConfig[] = {
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_GFX_SETTINGS}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_THEME}}},
    {UI_SPACER},
    {UI_ENUM, UICFG_THEME, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_LANGUAGE}}},
    {UI_SPACER},
    {UI_ENUM, UICFG_LANG, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_AUTOSORT}}},
    {UI_SPACER},
    {UI_BOOL, UICFG_AUTOSORT, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_AUTOREFRESH}}},
    {UI_SPACER},
    {UI_BOOL, UICFG_AUTOREFRESH, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_COVERART}}},
    {UI_SPACER},
    {UI_BOOL, UICFG_COVERART, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_ENABLE_NOTIFICATIONS}}},
    {UI_SPACER},
    {UI_BOOL, UICFG_NOTIFICATIONS, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, -30, 0, {.label = {NULL, _STR_TXTCOLOR}}},
    {UI_SPACER},
    {UI_COLOUR, UICFG_TXTCOL, 1, 1, -1, -10, 17, {.colourvalue = {0, 0}}},
    {UI_SPACER},
    {UI_LABEL, 0, 1, 1, -1, -30, 0, {.label = {NULL, _STR_SELCOLOR}}},
    {UI_SPACER},
    {UI_COLOUR, UICFG_SELCOL, 1, 1, -1, -10, 17, {.colourvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -30, 0, {.label = {NULL, _STR_UICOLOR}}},
    {UI_SPACER},
    {UI_COLOUR, UICFG_UICOL, 1, 1, -1, -10, 17, {.colourvalue = {0, 0}}},
    {UI_SPACER},
    {UI_LABEL, 0, 1, 1, -1, -30, 0, {.label = {NULL, _STR_BGCOLOR}}},
    {UI_SPACER},
    {UI_COLOUR, UICFG_BGCOL, 1, 1, -1, -10, 17, {.colourvalue = {0, 0}}},
    {UI_BREAK},

    {UI_BUTTON, UICFG_RESETCOL, 1, 1, -1, 0, 0, {.label = {NULL, _STR_RESETCOLOR}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_VMODE}}},
    {UI_SPACER},
    {UI_ENUM, UICFG_VMODE, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_XOFFSET}}},
    {UI_SPACER},
    {UI_INT, UICFG_XOFF, 1, 1, -1, 0, 0, {.intvalue = {0, 0, -300, 300}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_YOFFSET}}},
    {UI_SPACER},
    {UI_INT, UICFG_YOFF, 1, 1, -1, 0, 0, {.intvalue = {0, 0, -300, 300}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_OVERSCAN}}},
    {UI_SPACER},
    {UI_INT, UICFG_OVERSCAN, 1, 1, -1, 0, 0, {.intvalue = {0, 0, 0, 100}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_WIDE_SCREEN}}},
    {UI_SPACER},
    {UI_BOOL, UICFG_WIDESCREEN, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    // buttons
    {UI_OK, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_OK}}},
    {UI_BREAK},

    // end of dialog
    {UI_TERMINATOR}};

// Per-Game Modes Menu
struct UIItem diaCompatConfig[] = {
    {UI_LABEL, 0, 1, 1, -1, -30, 0, {.label = {NULL, _STR_COMPAT_SETTINGS}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_MODE1}}},
    {UI_SPACER},
    {UI_BOOL, COMPAT_MODE_BASE, 1, 1, _STR_HINT_MODE1, -10, 0, {.intvalue = {0, 0}}},
    {UI_SPACER},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_MODE2}}},
    {UI_SPACER},
    {UI_BOOL, COMPAT_MODE_BASE + 1, 1, 1, _STR_HINT_MODE2, -10, 0, {.intvalue = {0, 0}}},
    {UI_SPACER},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_MODE3}}},
    {UI_SPACER},
    {UI_BOOL, COMPAT_MODE_BASE + 2, 1, 1, _STR_HINT_MODE3, -10, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_MODE4}}},
    {UI_SPACER},
    {UI_BOOL, COMPAT_MODE_BASE + 3, 1, 1, _STR_HINT_MODE4, -10, 0, {.intvalue = {0, 0}}},
    {UI_SPACER},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_MODE5}}},
    {UI_SPACER},
    {UI_BOOL, COMPAT_MODE_BASE + 4, 1, 1, _STR_HINT_MODE5, -10, 0, {.intvalue = {0, 0}}},
    {UI_SPACER},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_MODE6}}},
    {UI_SPACER},
    {UI_BOOL, COMPAT_MODE_BASE + 5, 1, 1, _STR_HINT_MODE6, -10, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_BUTTON, COMPAT_DL_DEFAULTS, 1, 1, -1, 0, 0, {.label = {NULL, _STR_DL_DEFAULTS}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, -30, 0, {.label = {NULL, _STR_DMA_MODE}}},
    {UI_SPACER},
    {UI_ENUM, COMPAT_DMA, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, -30, 0, {.label = {NULL, _STR_GAME_ID}}},
    {UI_SPACER},
    {UI_STRING, COMPAT_GAMEID, 1, 1, -1, 0, 0, {.stringvalue = {"", "", NULL}}},
    {UI_SPACER},
    {UI_BUTTON, COMPAT_LOADFROMDISC, 1, 1, -1, 0, 0, {.label = {NULL, _STR_LOAD_FROM_DISC}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, -30, 0, {.label = {NULL, _STR_ALTSTARTUP}}},
    {UI_SPACER},
    {UI_STRING, COMPAT_ALTSTARTUP, 1, 1, -1, 0, 0, {.stringvalue = {"", "", &guiGameAltStartupNameHandler}}},
    {UI_BREAK},

    // buttons
    {UI_OK, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_OK}}},
    {UI_BREAK},

    // end of dialog
    {UI_TERMINATOR}};

// Per-Game VMC Menu
struct UIItem diaVMCConfig[] = {
    {UI_LABEL, 0, 1, 1, -1, -30, 0, {.label = {NULL, _STR_VMC_SCREEN}}},
    {UI_SPLITTER},

    // VMC
    {UI_LABEL, 0, 1, 1, -1, -30, 0, {.label = {NULL, _STR_VMC_SLOT1}}},
    {UI_SPACER},
    {UI_BUTTON, COMPAT_VMC1_DEFINE, 1, 1, -1, 0, 0, {.label = {NULL, -1}}},
    {UI_SPACER},
    {UI_BUTTON, COMPAT_VMC1_ACTION, 1, 1, -1, 0, 0, {.label = {NULL, -1}}},
    {UI_BREAK},
    {UI_LABEL, 0, 1, 1, -1, -30, 0, {.label = {NULL, _STR_VMC_SLOT2}}},
    {UI_SPACER},
    {UI_BUTTON, COMPAT_VMC2_DEFINE, 1, 1, -1, 0, 0, {.label = {NULL, -1}}},
    {UI_SPACER},
    {UI_BUTTON, COMPAT_VMC2_ACTION, 1, 1, -1, 0, 0, {.label = {NULL, -1}}},
    {UI_BREAK},

    // buttons
    {UI_OK, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_OK}}},
    {UI_BREAK},

    // end of dialog
    {UI_TERMINATOR}};

// Per-Game Game Settings > VMC Menu
struct UIItem diaVMC[] = {
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_VMC_SCREEN}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, -20, 0, {.label = {NULL, _STR_VMC_NAME}}},
    {UI_SPACER},
    {UI_STRING, VMC_NAME, 1, 1, -1, 0, 0, {.stringvalue = {"", "", &guiGameVmcNameHandler}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -20, 0, {.label = {NULL, _STR_SIZE}}},
    {UI_SPACER},
    {UI_ENUM, VMC_SIZE, 1, 1, _STR_HINT_VMC_SIZE, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -20, 0, {.label = {NULL, _STR_VMC_STATUS}}},
    {UI_SPACER},
    {UI_LABEL, VMC_STATUS, 0, 1, -1, 0, 0, {.label = {NULL, -1}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -20, 0, {.label = {NULL, _STR_VMC_PROGRESS}}},
    {UI_SPACER},
    {UI_INT, VMC_PROGRESS, 0, 1, -1, 0, 0, {.intvalue = {0, 0, 0, 100}}},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {"%", -1}}},
    {UI_BREAK},

    // buttons
    {UI_BUTTON, VMC_BUTTON_CREATE, 1, 1, -1, 0, 0, {.label = {NULL, -1}}},
    {UI_SPACER},
    {UI_BUTTON, VMC_BUTTON_DELETE, 1, 1, -1, 0, 0, {.label = {NULL, _STR_DELETE}}},
    {UI_BREAK},

    // end of dialog
    {UI_TERMINATOR}};

// Per-Game Game Settings > GSM Menu (--Bat--)
struct UIItem diaGSConfig[] = {
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_GSM_SETTINGS}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_SETTINGS_SOURCE}}},
    {UI_SPACER},
    {UI_ENUM, GSMCFG_GSMSOURCE, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_ENABLEGSM}}},
    {UI_SPACER},
    {UI_BOOL, GSMCFG_ENABLEGSM, 1, 1, _STR_HINT_ENABLEGSM, 0, 0, {.intvalue = {1, 1}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_GSMVMODE}}},
    {UI_SPACER},
    {UI_ENUM, GSMCFG_GSMVMODE, 1, 1, _STR_HINT_GSMVMODE, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_XOFFSET}}},
    {UI_SPACER},
    {UI_INT, GSMCFG_GSMXOFFSET, 1, 1, _STR_HINT_XOFFSET, -5, 0, {.intvalue = {0, 0, -100, 100}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_YOFFSET}}},
    {UI_SPACER},
    {UI_INT, GSMCFG_GSMYOFFSET, 1, 1, _STR_HINT_YOFFSET, -5, 0, {.intvalue = {0, 0, -100, 100}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_GSM_FIELD_FIX}}},
    {UI_SPACER},
    {UI_BOOL, GSMCFG_GSMFIELDFIX, 1, 1, _STR_HINT_GSM_FIELD_FIX, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    // buttons
    {UI_OK, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_OK}}},
    {UI_BREAK},

    // end of dialog
    {UI_TERMINATOR}};

// Per Game Settings > Cheat Menu --Bat--
struct UIItem diaCheatConfig[] = {
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_CHEAT_SETTINGS}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_SETTINGS_SOURCE}}},
    {UI_SPACER},
    {UI_ENUM, CHTCFG_CHEATSOURCE, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_ENABLECHEAT}}},
    {UI_SPACER},
    {UI_BOOL, CHTCFG_ENABLECHEAT, 1, 1, _STR_HINT_ENABLECHEAT, 0, 0, {.intvalue = {1, 1}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_CHEATMODE}}},
    {UI_SPACER},
    {UI_ENUM, CHTCFG_CHEATMODE, 1, 1, _STR_HINT_CHEATMODE, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    // buttons
    {UI_OK, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_OK}}},
    {UI_BREAK},

    // end of dialog
    {UI_TERMINATOR}};

#ifdef PADEMU
struct UIItem diaPadEmuConfig[] = {
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_PADEMU_SETTINGS}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, -45, 0, {.label = {NULL, _STR_SETTINGS_SOURCE}}},
    {UI_SPACER},
    {UI_ENUM, PADCFG_PADEMU_SOURCE, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -45, 0, {.label = {NULL, _STR_PADEMU_ENABLE}}},
    {UI_SPACER},
    {UI_BOOL, PADCFG_PADEMU_ENABLE, 1, 1, _STR_HINT_PADEMU_ENABLE, 0, 0, {.intvalue = {1, 1}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -45, 0, {.label = {NULL, _STR_PADEMU_MODE}}},
    {UI_SPACER},
    {UI_ENUM, PADCFG_PADEMU_MODE, 1, 1, _STR_HINT_PADEMU_MODE, 0, 0, {.intvalue = {1, 1}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -45, 0, {.label = {NULL, _STR_MTAP_ENABLE}}},
    {UI_SPACER},
    {UI_BOOL, PADCFG_PADEMU_MTAP, 1, 1, _STR_HINT_MTAP_ENABLE, 0, 0, {.intvalue = {1, 1}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -45, 0, {.label = {NULL, _STR_MTAP_PORT}}},
    {UI_SPACER},
    {UI_INT, PADCFG_PADEMU_MTAP_PORT, 1, 1, _STR_HINT_MTAP_PORT, 0, 0, {.intvalue = {1, 1, 1, 2}}},
    {UI_BREAK},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -45, 0, {.label = {NULL, _STR_PADPORT}}},
    {UI_SPACER},
    {UI_ENUM, PADCFG_PADPORT, 1, 1, _STR_HINT_PAD_PORT, 0, 0, {.intvalue = {1, 1}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -45, 0, {.label = {NULL, _STR_PADEMU_PORT}}},
    {UI_SPACER},
    {UI_BOOL, PADCFG_PADEMU_PORT, 1, 1, _STR_HINT_PADEMU_PORT, 0, 0, {.intvalue = {1, 1}}},
    {UI_BREAK},
    {UI_LABEL, 0, 1, 1, -1, -45, 0, {.label = {NULL, _STR_PADEMU_VIB}}},
    {UI_SPACER},
    {UI_BOOL, PADCFG_PADEMU_VIB, 1, 1, _STR_HINT_PADEMU_VIB, 0, 0, {.intvalue = {1, 1}}},
    {UI_BREAK},

    {UI_SPLITTER},
    {UI_BREAK},

    {UI_LABEL, PADCFG_USBDG_MAC_STR, 1, 1, -1, -45, 0, {.label = {NULL, _STR_USBDG_MAC}}},
    {UI_SPACER},
    {UI_LABEL, PADCFG_USBDG_MAC, 1, 1, -1, 0, 0, {.label = {"", -1}}},
    {UI_BREAK},
    {UI_LABEL, PADCFG_PAD_MAC_STR, 1, 1, -1, -45, 0, {.label = {NULL, _STR_PAD_MAC}}},
    {UI_SPACER},
    {UI_LABEL, PADCFG_PAD_MAC, 1, 1, -1, 0, 0, {.label = {"", -1}}},
    {UI_BREAK},

    {UI_LABEL, PADCFG_PAIR_STR, 1, 1, -1, -45, 0, {.label = {NULL, _STR_PAIR_PAD}}},
    {UI_SPACER},
    {UI_BUTTON, PADCFG_PAIR, 1, 1, _STR_HINT_PAIRPAD, 0, 0, {.label = {NULL, _STR_PAIR}}},
    {UI_BREAK},

    {UI_LABEL, PADCFG_PADEMU_WORKAROUND_STR, 1, 1, -1, -45, 0, {.label = {NULL, _STR_PADEMU_WORKAROUND}}},
    {UI_SPACER},
    {UI_BOOL, PADCFG_PADEMU_WORKAROUND, 1, 1, _STR_HINT_PADEMU_WORKAROUND, 0, 0, {.intvalue = {1, 1}}},
    {UI_BREAK},

    {UI_BREAK},
    {UI_BUTTON, PADCFG_BTINFO, 1, 1, _STR_HINT_BTINFO, 0, 0, {.label = {NULL, _STR_BTINFO}}},
    {UI_BREAK},

    {UI_OK, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_OK}}},

    // end of dialog
    {UI_TERMINATOR}};

struct UIItem diaPadEmuInfo[] = {
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_BTINFO}}}, {UI_SPACER},

    {UI_SPLITTER},
    {UI_LABEL, 0, 1, 1, -1, -45, 0, {.label = {"VID:", -1}}},
    {UI_LABEL, PADCFG_VID, 1, 1, -1, -45, 0, {.label = {"", -1}}},
    {UI_BREAK},
    {UI_LABEL, 0, 1, 1, -1, -45, 0, {.label = {"PID:", -1}}},
    {UI_LABEL, PADCFG_PID, 1, 1, -1, -45, 0, {.label = {"", -1}}},
    {UI_BREAK},
    {UI_LABEL, 0, 1, 1, -1, -45, 0, {.label = {"REV:", -1}}},
    {UI_LABEL, PADCFG_REV, 1, 1, -1, -45, 0, {.label = {"", -1}}},
    {UI_BREAK},
    {UI_LABEL, 0, 1, 1, -1, -45, 0, {.label = {NULL, _STR_HCIVER}}},
    {UI_LABEL, PADCFG_HCIVER, 1, 1, -1, -45, 0, {.label = {"", -1}}},
    {UI_BREAK},
    {UI_LABEL, 0, 1, 1, -1, -45, 0, {.label = {NULL, _STR_LMPVER}}},
    {UI_LABEL, PADCFG_LMPVER, 1, 1, -1, -45, 0, {.label = {"", -1}}},
    {UI_BREAK},
    {UI_LABEL, 0, 1, 1, -1, -45, 0, {.label = {NULL, _STR_MANUFACTURER}}},
    {UI_LABEL, PADCFG_MANID, 1, 1, -1, -45, 0, {.label = {"", -1}}},
    {UI_BREAK},
    {UI_LABEL, 0, 1, 1, -1, -45, 0, {.label = {NULL, _STR_SUPFEATURES}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"00.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 0, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"08.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 8, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"16.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 16, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"24.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 24, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"32.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 32, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"40.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 40, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"48.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 48, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"56.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 56, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"01.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 1, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"09.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 9, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"17.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 17, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"25.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 25, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"33.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 33, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"41.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 41, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"49.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 49, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"57.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 57, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"02.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 2, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"10.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 10, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"18.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 18, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"26.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 26, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"34.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 34, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"42.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 42, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"50.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 50, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"58.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 58, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"03.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 3, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"11.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 11, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"19.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 19, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"27.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 27, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"35.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 35, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"43.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 43, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"51.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 51, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"59.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 59, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"04.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 4, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"12.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 12, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"20.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 20, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"28.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 28, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"36.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 36, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"44.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 44, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"52.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 52, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"60.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 60, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"05.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 5, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"13.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 13, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"21.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 21, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"29.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 29, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"37.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 37, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"45.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 45, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"53.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 53, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"61.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 61, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"06.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 6, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"14.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 14, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"22.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 22, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"30.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 30, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"38.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 38, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"46.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 46, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"54.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 54, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"62.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 62, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"07.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 7, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"15.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 15, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"23.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 23, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"31.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 31, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"39.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 39, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"47.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 47, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"55.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 55, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_LABEL, 0, 1, 1, -1, -4, 0, {.label = {"63.", -1}}},
    {UI_LABEL, PADCFG_FEAT_START + 63, 0, 1, -1, -7, 0, {.label = {"", -1}}},
    {UI_BREAK},
    {UI_BREAK},

    {UI_LABEL, PADCFG_BT_SUPPORTED, 1, 1, -1, -45, 0, {.label = {"", -1}}},
    {UI_BREAK},
    {UI_OK, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_OK}}},

    // end of dialog
    {UI_TERMINATOR}};
#endif

// About Menu
struct UIItem diaAbout[] = {
    {UI_LABEL, ABOUT_TITLE, 1, 1, -1, 0, 0, {.label = {NULL, -1}}},
    {UI_SPLITTER},

    // Coders
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_DEVS}}},
    {UI_BREAK},

    {UI_SPACER},
    {UI_LABEL, 0, 1, 1, -1, 0, 15, {.label = {"BatRastard - belek666 - crazyc - dlanor - doctorxyz", -1}}},
    {UI_BREAK},

    {UI_SPACER},
    {UI_LABEL, 0, 1, 1, -1, 0, 15, {.label = {"hominem.te.esse - ifcaro - izdubar - jimmikaelkael", -1}}},
    {UI_BREAK},

    {UI_SPACER},
    {UI_LABEL, 0, 1, 1, -1, 0, 15, {.label = {"Maximus32 - misfire - Polo35 - reprep - SP193 - volca", -1}}},
    {UI_BREAK},

    {UI_SPACER},
    {UI_LABEL, 0, 1, 1, -1, 0, 15, {.label = {"... and the anonymous ...", -1}}},
    {UI_BREAK},

    {UI_BREAK},

    // Quality Assurance
    {UI_LABEL, 0, 1, 1, -1, 0, 15, {.label = {NULL, _STR_QANDA}}},
    {UI_BREAK},

    {UI_SPACER},
    {UI_LABEL, 0, 1, 1, -1, 0, 15, {.label = {"algol - Berion - El_Patas - EP - gledson999 - jolek - lee4", -1}}},
    {UI_BREAK},

    {UI_SPACER},
    {UI_LABEL, 0, 1, 1, -1, 0, 15, {.label = {"LocalH - RandQalan - ShaolinAssassin - yoshi314 - zero35", -1}}},
    {UI_BREAK},

    {UI_BREAK},

    // Network update
    {UI_LABEL, 0, 1, 1, -1, 0, 15, {.label = {NULL, _STR_NET_UPDATE}}},
    {UI_BREAK},

    {UI_SPACER},
    {UI_LABEL, 0, 1, 1, -1, 0, 15, {.label = {"icyson55", -1}}},
    {UI_BREAK},

    // Build Options
    {UI_BREAK},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_BUILD_DETAILS}}},
    {UI_SPACER},
    {UI_LABEL, ABOUT_BUILD_DETAILS, 1, 1, -1, 0, 0, {.label = {NULL, -1}}},
    {UI_BREAK},

    // Support details
    {UI_BREAK},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_FORUM_DETAILS}}},
    {UI_SPACER},
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {"psx-place.com", -1}}},
    {UI_BREAK},

    // buttons
    {UI_OK, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_OK}}},
    {UI_BREAK},

    // end of dialog
    {UI_TERMINATOR}};

// Network Update Menu
struct UIItem diaNetCompatUpdate[] = {
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_NET_UPDATE}}},
    {UI_SPLITTER},

    {UI_LABEL, NETUPD_OPT_UPD_ALL_LBL, 1, 1, -1, -40, 0, {.label = {NULL, _STR_NET_UPDATE_ALL}}},
    {UI_SPACER},
    {UI_BOOL, NETUPD_OPT_UPD_ALL, 0, 1, _STR_NET_UPDATE_HINT, 0, 0, {.intvalue = {0, 0, 0, 1}}},
    {UI_BREAK},

    {UI_LABEL, NETUPD_PROGRESS_LBL, 1, 1, -1, -40, 0, {.label = {NULL, _STR_VMC_PROGRESS}}},
    {UI_SPACER},
    {UI_INT, NETUPD_PROGRESS, 0, 1, -1, 0, 0, {.intvalue = {0, 0, 0, 100}}},
    {UI_LABEL, NETUPD_PROGRESS_PERC_LBL, 1, 1, -1, 0, 0, {.label = {"%", -1}}},
    {UI_BREAK},

    // buttons
    {UI_BUTTON, NETUPD_BTN_START, 1, 1, -1, 0, 0, {.label = {NULL, _STR_START}}},
    {UI_SPACER},
    {UI_BUTTON, NETUPD_BTN_CANCEL, 1, 1, -1, 0, 0, {.label = {NULL, _STR_CANCEL}}},
    {UI_BREAK},

    // end of dialog
    {UI_TERMINATOR}};

// Parental Lock Config Menu
struct UIItem diaParentalLockConfig[] = {
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_PARENLOCKCONFIG}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_PARENLOCK_PASSWORD}}},
    {UI_SPACER},
    {UI_PASSWORD, CFG_PARENLOCK_PASSWORD, 1, 1, _STR_PARENLOCK_PASSWORD_HINT, 0, 0, {.stringvalue = {"", "", NULL}}},
    {UI_BREAK},

    // buttons
    {UI_OK, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_OK}}},
    {UI_BREAK},

    // end of dialog
    {UI_TERMINATOR}};

// Audio Settings Menu
struct UIItem diaAudioConfig[] = {
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_AUDIO_SETTINGS}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_SFX}}},
    {UI_SPACER},
    {UI_BOOL, CFG_SFX, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_BOOT_SND}}},
    {UI_SPACER},
    {UI_BOOL, CFG_BOOT_SND, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_SFX_VOLUME}}},
    {UI_SPACER},
    {UI_INT, CFG_SFX_VOLUME, 1, 1, -1, 0, 0, {.intvalue = {0, 0, 0, 100}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_BOOT_SND_VOLUME}}},
    {UI_SPACER},
    {UI_INT, CFG_BOOT_SND_VOLUME, 1, 1, -1, 0, 0, {.intvalue = {0, 0, 0, 100}}},
    {UI_BREAK},

    // buttons
    {UI_OK, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_OK}}},
    {UI_BREAK},
    // end of dialog
    {UI_TERMINATOR}};

// Controller Settings Menu
struct UIItem diaControllerConfig[] = {
    {UI_LABEL, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_CONTROLLER_SETTINGS}}},
    {UI_SPLITTER},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_SCROLLING}}},
    {UI_SPACER},
    {UI_ENUM, UICFG_SCROLL, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},

    {UI_LABEL, 0, 1, 1, -1, -40, 0, {.label = {NULL, _STR_SELECTBUTTON}}},
    {UI_SPACER},
    {UI_ENUM, CFG_SELECTBUTTON, 1, 1, -1, 0, 0, {.intvalue = {0, 0}}},
    {UI_BREAK},
#ifdef PADEMU
    {UI_BREAK},
    {UI_BUTTON, PADEMU_GLOBAL_BUTTON, 1, 1, -1, 0, 0, {.label = {NULL, _STR_PADEMUCONFIG}}},
    {UI_BREAK},
#endif
    // buttons
    {UI_OK, 0, 1, 1, -1, 0, 0, {.label = {NULL, _STR_OK}}},
    {UI_BREAK},
    // end of dialog
    {UI_TERMINATOR}};
