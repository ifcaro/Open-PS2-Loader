enum OPL_MODULE_ID {
    // Basic modules
    OPL_MODULE_ID_UDNL = 1,
    OPL_MODULE_ID_IOPRP,
    OPL_MODULE_ID_IMGDRV,
    OPL_MODULE_ID_RESETSPU,

    // USB mode modules
    OPL_MODULE_ID_USBD,
    OPL_MODULE_ID_USBMASSBD,

    // iLink mode modules
    OPL_MODULE_ID_ILINK,
    OPL_MODULE_ID_ILINKBD,

    // mx4sio mode modules
    OPL_MODULE_ID_MX4SIOBD,

    // SMB mode modules
    OPL_MODULE_ID_SMSTCPIP,
    OPL_MODULE_ID_SMAP,
    OPL_MODULE_ID_SMBINIT,

    // VMC module
    OPL_MODULE_ID_MCEMU,

    OPL_MODULE_ID_PADEMU,

    // Debugging modules
    OPL_MODULE_ID_UDPTTY,
    OPL_MODULE_ID_IOPTRAP,
    OPL_MODULE_ID_DRVTIF,
    OPL_MODULE_ID_TIFINET,

    // Special patches
    OPL_MODULE_ID_IOP_PATCH,

    OPL_MODULE_ID_COUNT
};

typedef struct
{
    void *ptr;
    unsigned int info; // Upper 8 bits = module ID
} irxptr_t;

typedef struct
{
    irxptr_t *modules;
    int count;
} irxtab_t;

// Macros for working with module information.
#define GET_OPL_MOD_ID(x)   ((x) >> 24)
#define SET_OPL_MOD_ID(x)   ((x) << 24)
#define GET_OPL_MOD_SIZE(x) ((x)&0x00FFFFFF)
