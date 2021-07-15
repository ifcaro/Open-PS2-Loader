#ifndef __HDD_SUPPORT_H
#define __HDD_SUPPORT_H

#include "include/iosupport.h"
#include "include/hdd.h"

#define HDD_MODE_UPDATE_DELAY MENU_UPD_DELAY_NOUPDATE

#define HDL_GAME_NAME_MAX 64

// APA Partition
#define APA_IOCTL2_GETHEADER 0x6836

typedef struct
{
    char partition_name[APA_IDMAX + 1];
    char name[HDL_GAME_NAME_MAX + 1];
    char startup[8 + 1 + 3 + 1];
    u8 hdl_compat_flags;
    u8 ops2l_compat_flags;
    u8 dma_type;
    u8 dma_mode;
    u8 disctype;
    u32 layer_break;
    u32 start_sector;
    u32 total_size_in_kb;
} hdl_game_info_t;

typedef struct
{
    u32 count;
    hdl_game_info_t *games;
} hdl_games_list_t;

typedef struct
{
    u32 start;
    u32 length;
} apa_subs;

#include "include/mcemu.h"
typedef struct
{
    int active;                 /* Activation flag */
    apa_subs parts[5];          /* Vmc file Apa partitions */
    pfs_blockinfo_t blocks[10]; /* Vmc file Pfs inode */
    int flags;                  /* Card flag */
    vmc_spec_t specs;           /* Card specifications */
} hdd_vmc_infos_t;

int hddCheck(void);
u32 hddGetTotalSectors(void);
int hddIs48bit(void);
int hddSetTransferMode(int type, int mode);
void hddSetIdleTimeout(int timeout);
void hddSetIdleImmediate(void);
int hddGetHDLGamelist(hdl_games_list_t *game_list);
void hddFreeHDLGamelist(hdl_games_list_t *game_list);
int hddSetHDLGameInfo(hdl_game_info_t *ginfo);
int hddDeleteHDLGame(hdl_game_info_t *ginfo);

void hddInit();
item_list_t *hddGetObject(int initOnly);
void hddLoadModules(void);
void hddMountAppsPartition(void);

extern char gOPLPart[128];

#endif
