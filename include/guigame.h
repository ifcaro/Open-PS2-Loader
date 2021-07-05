#ifndef __GUIGAME_H
#define __GUIGAME_H

#define SETTINGS_GLOBAL  0
#define SETTINGS_PERGAME 1

int guiGameAltStartupNameHandler(char *text, int maxLen);

char *gameConfigSource(void);

int guiGameVmcNameHandler(char *text, int maxLen);
void guiGameShowVMCMenu(int id, item_list_t *support);
void guiGameShowCompatConfig(int id, item_list_t *support, config_set_t *configSet);
void guiGameShowGSConfig(void);
void guiGameShowCheatConfig(void);

#ifdef PADEMU
void guiGameShowPadEmuConfig(int forceGlobal);
void guiGameSavePadEmuGlobalConfig(config_set_t *configGame);
#endif

void guiGameLoadConfig(item_list_t *support, config_set_t *configSet);
int guiGameSaveConfig(config_set_t *configSet, item_list_t *support);
void guiGameTestSettings(int id, item_list_t *support, config_set_t *configSet);

void guiGameRemoveSettings(config_set_t *configSet);
void guiGameRemoveGlobalSettings(config_set_t *configGame);
#endif
