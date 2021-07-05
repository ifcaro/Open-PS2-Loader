#ifndef __GUI_H
#define __GUI_H

#include "include/iosupport.h"
#include "include/opl.h"
#include "include/texcache.h"
#include "include/dialogs.h"
#include "include/menusys.h"

typedef enum {
    // Informs gui that init is over and main gui can be rendered
    GUI_INIT_DONE = 1,
    GUI_OP_ADD_MENU,
    GUI_OP_APPEND_MENU,
    GUI_OP_SELECT_MENU,
    GUI_OP_CLEAR_SUBMENU,
    GUI_OP_SORT,
    GUI_OP_ADD_HINT
} gui_op_type_t;

/** a single GUI update in a package form */
struct gui_update_t
{
    gui_op_type_t type;

    struct
    {
        menu_item_t *menu;
        submenu_list_t **subMenu;
    } menu;

    union
    {
        struct
        {
            int icon_id;
            char *text;
            int id;
            int text_id;
            int selected;
        } submenu;

        struct
        { // hint for the given menu
            int icon_id;
            int text_id;
        } hint;
    };
};

typedef void (*gui_callback_t)(void);

extern int guiInactiveFrames;
extern int guiFrameId;

#define GUI_SCREEN_MAIN      0
#define GUI_SCREEN_MENU      1
#define GUI_SCREEN_INFO      2
#define GUI_SCREEN_GAME_MENU 3
#define GUI_SCREEN_APP_MENU  4

void guiSwitchScreen(int target);

void guiReloadScreenExtents();

/** Initializes the GUI */
void guiInit();

/** Clean-up the GUI */
void guiEnd();

/** Locks gui for direct gui data updates */
void guiLock();

/** Unlocks gui after direct gui data updates */
void guiUnlock();

/** invokes the intro loop */
void guiIntroLoop();

/** invokes the main loop */
void guiMainLoop();

/** Hooks a single per-frame callback */
void guiSetFrameHook(gui_callback_t cback);

// Deffered update handling:
/* Note:
All the GUI operation requests can be deffered to the proper time
when rendering is not going on. This allows us, for example, to schedule
updates of the menu from another thread without stalls.
*/

/** Detects if a given deffered operation is already complete
 * @param opid The operation id, as returned from guiDeferUpdate
 * @return 1 if the operation was already completed, 0 otherwise */
int guiGetOpCompleted(int opid);

/** Defers the given update to an appropriate time.
 * @param op The operation to defer
 * @return int The operation serial id
 */
int guiDeferUpdate(struct gui_update_t *op);

void guiExecDeferredOps(void);

/** Allocates a new deffered operation */
struct gui_update_t *guiOpCreate(gui_op_type_t type);

/** For completeness, the deffered operations are destroyed automatically */
void guiDestroyOp(struct gui_update_t *op);

int guiShowKeyboard(char *value, int maxLength);
int guiMsgBox(const char *text, int addAccept, struct UIItem *ui);

void guiUpdateScrollSpeed(void);
void guiUpdateScreenScale(void);

void guiDrawBGPlasma();
int guiDrawIconAndText(int iconId, int textId, int font, int x, int y, u64 color);
void guiDrawSubMenuHints(void);

int guiAlignMenuHints(menu_hint_item_t *hint, int font, int width);
int guiAlignSubMenuHints(int hintCount, int *textID, int *iconID, int font, int width, int align);

void guiShowNetCompatUpdate(void);
void guiShowNetCompatUpdateSingle(int id, item_list_t *support, config_set_t *configSet);
void guiShowAbout();
void guiShowConfig();
void guiShowUIConfig();
void guiShowAudioConfig();
void guiShowControllerConfig();
void guiShowNetConfig();
void guiShowParentalLockConfig();

void guiCheckNotifications(int checkTheme, int checkLang);

/** Renders the given string on screen for the given function until it's io finishes
 * @note The ptr pointer is watched for it's value. The IO is considered finished when the value becomes zero.
 * @param ptr The finished state pointer (1 unfinished, 0 finished)
 * @param message The message to display while working
 * @param type the io operation type
 * @param data the data for the operation
 */
void guiHandleDeferedIO(int *ptr, const char *message, int type, void *data);

void guiGameHandleDeferedIO(int *ptr, struct UIItem *ui, int type, void *data);

/** Renders a single frame with a specified message on the screen
 */
void guiRenderTextScreen(const char *message);

void guiWarning(const char *text, int count);

int guiConfirmVideoMode(void);

int guiGameShowRemoveSettings(config_set_t *configSet, config_set_t *configGame);

#endif
