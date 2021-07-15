VERSION = 1
SUBVERSION = 1
PATCHLEVEL = 0
EXTRAVERSION = Beta

# How to DEBUG?
# Simply type "make <debug mode>" to build OPL with the necessary debugging functionality.
# Debug modes:
#	debug		-	UI-side debug mode (UDPTTY)
#	iopcore_debug	-	UI-side + iopcore debug mode (UDPTTY).
#	ingame_debug	-	UI-side + in-game debug mode. IOP core modules will not be built as debug versions (UDPTTY).
#	eesio_debug	-	UI-side + eecore debug mode (EE SIO)
#	deci2_debug	-	UI-side + in-game DECI2 debug mode (EE-side only).

# I want to put my name in my custom build! How can I do it?
# Type "make LOCALVERSION=-foobar"

# ======== START OF CONFIGURABLE SECTION ========
# You can adjust the variables in this section to meet your needs.
# To enable a feature, set its variable's value to 1. To disable, change it to 0.
# Do not COMMENT out the variables!!
# You can also specify variables when executing make: "make RTL=1 IGS=1 PADEMU=1"

#Enables/disables Right-To-Left (RTL) language support
RTL ?= 0

#Enables/disables In Game Screenshot (IGS). NB: It depends on GSM and IGR to work
IGS ?= 1

#Enables/disables pad emulator
PADEMU ?= 1

#Enables/disables building of an edition of OPL that will support the DTL-T10000 (SDK v2.3+)
DTL_T10000 ?= 0

#Nor stripping neither compressing binary ELF after compiling.
NOT_PACKED ?= 0

# ======== END OF CONFIGURABLE SECTION. DO NOT MODIFY VARIABLES AFTER THIS POINT!! ========
DEBUG ?= 0
EESIO_DEBUG ?= 0
INGAME_DEBUG ?= 0
DECI2_DEBUG ?= 0

# ======== DO NOT MODIFY VALUES AFTER THIS POINT! UNLESS YOU KNOW WHAT YOU ARE DOING ========
REVISION = $(shell expr $(shell git rev-list --count HEAD) + 2)

GIT_HASH = $(shell git rev-parse --short=7 HEAD 2>/dev/null)
ifeq ($(shell git diff --quiet; echo $$?),1)
  DIRTY = -dirty
endif
ifneq ($(shell test -d .git; echo $$?),0)
  DIRTY = -dirty
endif

GIT_TAG = $(shell git describe --exact-match --tags 2>/dev/null)
OPL_VERSION = v$(VERSION).$(SUBVERSION).$(PATCHLEVEL)$(if $(EXTRAVERSION),-$(EXTRAVERSION))-$(REVISION)$(if $(GIT_HASH),-$(GIT_HASH))$(if $(DIRTY),$(DIRTY))$(if $(LOCALVERSION),-$(LOCALVERSION))

ifneq ($(GIT_TAG),)
ifneq ($(GIT_TAG),latest)
	# git revision is tagged
	OPL_VERSION = $(GIT_TAG)$(if $(DIRTY),$(DIRTY))
endif
endif

FRONTEND_OBJS = pad.o fntsys.o renderman.o menusys.o OSDHistory.o system.o lang.o config.o hdd.o dialogs.o \
		dia.o ioman.o texcache.o themes.o supportbase.o bdmsupport.o ethsupport.o hddsupport.o \
		appsupport.o gui.o guigame.o textures.o opl.o atlas.o nbns.o httpclient.o gsm.o cheatman.o sound.o ps2cnf.o

GFX_OBJS =	usb_icon.o hdd_icon.o eth_icon.o app_icon.o \
		cross_icon.o triangle_icon.o circle_icon.o square_icon.o select_icon.o start_icon.o \
		left_icon.o right_icon.o up_icon.o down_icon.o \
		load0.o load1.o load2.o load3.o load4.o load5.o load6.o load7.o \
		background.o info.o cover.o disc.o screen.o logo.o case.o \
		ELF.o HDL.o ISO.o UL.o CD.o DVD.o Aspect_s.o Aspect_w.o Aspect_w1.o Aspect_w2.o \
		Device_1.o Device_2.o Device_3.o Device_4.o Device_5.o Device_6.o Device_all.o \
		Rating_0.o Rating_1.o Rating_2.o Rating_3.o Rating_4.o Rating_5.o \
		Scan_240p.o Scan_240p1.o Scan_480i.o Scan_480p.o Scan_480p1.o Scan_480p2.o \
		Scan_480p3.o Scan_480p4.o Scan_480p5.o Scan_576i.o Scan_576p.o Scan_720p.o \
		Scan_1080i.o Scan_1080i2.o Scan_1080p.o Vmode_multi.o Vmode_ntsc.o Vmode_pal.o \
		poeveticanew.o icon_sys.o icon_icn.o

MISC_OBJS =	icon_sys_A.o icon_sys_J.o icon_sys_C.o conf_theme_OPL.o \
		boot.o cancel.o confirm.o cursor.o message.o transition.o

IOP_OBJS =	iomanx.o filexio.o ps2fs.o usbd.o bdmevent.o \
		bdm.o bdmfs_vfat.o usbmass_bd.o iLinkman.o IEEE1394_bd.o mx4sio_bd.o \
		ps2atad.o hdpro_atad.o poweroff.o ps2hdd.o xhdd.o genvmc.o lwnbdsvr.o \
		ps2dev9.o smsutils.o ps2ip.o smap.o isofs.o nbns-iop.o \
		sio2man.o padman.o mcman.o mcserv.o \
		httpclient-iop.o netman.o ps2ips.o \
		bdm_mcemu.o hdd_mcemu.o smb_mcemu.o \
		iremsndpatch.o apemodpatch.o f2techioppatch.o cleareffects.o resetspu.o \
		libsd.o audsrv.o

EECORE_OBJS = ee_core.o ioprp.o util.o \
		elfldr.o udnl.o imgdrv.o eesync.o \
		bdm_cdvdman.o IOPRP_img.o smb_cdvdman.o \
		hdd_cdvdman.o hdd_hdpro_cdvdman.o cdvdfsv.o \
		ingame_smstcpip.o smap_ingame.o smbman.o smbinit.o

EE_BIN = opl.elf
EE_BIN_STRIPPED = opl_stripped.elf
EE_BIN_PACKED = OPNPS2LD.ELF
EE_VPKD = OPNPS2LD-$(OPL_VERSION)
EE_SRC_DIR = src/
EE_OBJS_DIR = obj/
EE_ASM_DIR = asm/

MAPFILE = opl.map
EE_LDFLAGS += -Wl,-Map,$(MAPFILE)

EE_LIBS = -L$(PS2SDK)/ports/lib -L$(GSKIT)/lib -L./lib -lgskit -ldmakit -lgskit_toolkit -lpoweroff -lfileXio -lpatches -ljpeg -lpng -lz -ldebug -lm -lmc -lfreetype -lvux -lcdvd -lnetman -lps2ips -laudsrv -lpadx
EE_INCS += -I$(PS2SDK)/ports/include -I$(PS2SDK)/ports/include/freetype2 -I$(GSKIT)/include -I$(GSKIT)/ee/dma/include -I$(GSKIT)/ee/gs/include -I$(GSKIT)/ee/toolkit/include -Imodules/iopcore/common -Imodules/network/common -Imodules/hdd/common -Iinclude

BIN2C = $(PS2SDK)/bin/bin2c
BIN2S = $(PS2SDK)/bin/bin2s
BIN2O = $(PS2SDK)/bin/bin2o

# WARNING: Only extra spaces are allowed and ignored at the beginning of the conditional directives (ifeq, ifneq, ifdef, ifndef, else and endif)
# but a tab is not allowed; if the line begins with a tab, it will be considered part of a recipe for a rule!

ifeq ($(RTL),1)
  EE_CFLAGS += -D__RTL
endif

ifeq ($(DTL_T10000),1)
  EE_CFLAGS += -D_DTL_T10000
  EECORE_EXTRA_FLAGS += DTL_T10000=1
  UDNL_SRC = modules/iopcore/udnl-t300
  UDNL_OUT = modules/iopcore/udnl-t300/udnl.irx
else
  UDNL_SRC = modules/iopcore/udnl
  UDNL_OUT = modules/iopcore/udnl/udnl.irx
endif

ifeq ($(IGS),1)
  EE_CFLAGS += -DIGS
  IGS_FLAGS = IGS=1
else
  IGS_FLAGS = IGS=0
endif

ifeq ($(PADEMU),1)
  IOP_OBJS += bt_pademu.o usb_pademu.o ds34usb.o ds34bt.o libds34usb.a libds34bt.a
  EE_CFLAGS += -DPADEMU
  EE_INCS += -Imodules/ds34bt/ee -Imodules/ds34usb/ee
  PADEMU_FLAGS = PADEMU=1
else
  PADEMU_FLAGS = PADEMU=0
endif

ifeq ($(DEBUG),1)
  EE_CFLAGS += -D__DEBUG -g
  ifeq ($(DECI2_DEBUG),1)
    EE_OBJS += debug.o drvtif_irx.o tifinet_irx.o deci2_img.o
    EE_LDFLAGS += -liopreboot
  else
    EE_OBJS += debug.o udptty.o ioptrap.o ps2link.o
  endif
  MOD_DEBUG_FLAGS = DEBUG=1
  ifeq ($(IOPCORE_DEBUG),1)
    EE_CFLAGS += -D__INGAME_DEBUG
    EECORE_EXTRA_FLAGS = LOAD_DEBUG_MODULES=1
    CDVDMAN_DEBUG_FLAGS = IOPCORE_DEBUG=1
    MCEMU_DEBUG_FLAGS = IOPCORE_DEBUG=1
    SMSTCPIP_INGAME_CFLAGS =
    IOP_OBJS += udptty-ingame.o
  else ifeq ($(EESIO_DEBUG),1)
    EE_CFLAGS += -D__EESIO_DEBUG
    EECORE_EXTRA_FLAGS += EESIO_DEBUG=1
  else ifeq ($(INGAME_DEBUG),1)
    EE_CFLAGS += -D__INGAME_DEBUG
    EECORE_EXTRA_FLAGS = LOAD_DEBUG_MODULES=1
    CDVDMAN_DEBUG_FLAGS = IOPCORE_DEBUG=1
    SMSTCPIP_INGAME_CFLAGS =
    ifeq ($(DECI2_DEBUG),1)
      EE_CFLAGS += -D__DECI2_DEBUG
      EECORE_EXTRA_FLAGS += DECI2_DEBUG=1
      IOP_OBJS += drvtif_ingame_irx.o tifinet_ingame_irx.o
      DECI2_DEBUG=1
      CDVDMAN_DEBUG_FLAGS = USE_DEV9=1 #(clear IOPCORE_DEBUG) dsidb cannot be used to handle exceptions or set breakpoints, so disable output to save resources.
    else
      IOP_OBJS += udptty-ingame.o
    endif
  endif
else
  EE_CFLAGS += -O2
  SMSTCPIP_INGAME_CFLAGS = INGAME_DRIVER=1
endif

EE_CFLAGS += -fsingle-precision-constant -DOPL_VERSION=\"$(OPL_VERSION)\"

# There are a few places where the config key/value are truncated, so disable these warnings
EE_CFLAGS += -Wno-format-truncation -Wno-stringop-truncation
EE_OBJS += $(FRONTEND_OBJS) $(GFX_OBJS) $(MISC_OBJS) $(EECORE_OBJS) $(IOP_OBJS)
EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

.SILENT:

.PHONY: all release debug iopcore_debug eesio_debug ingame_debug deci2_debug clean rebuild pc_tools pc_tools_win32 oplversion format format-check ps2sdk-not-setup

ifdef PS2SDK

all:
	echo "Building Open PS2 Loader $(OPL_VERSION)..."
	echo "-Interface"
ifneq ($(NOT_PACKED),1)
	$(MAKE) $(EE_BIN_PACKED)
else
	$(MAKE) $(EE_BIN)
endif

release: $(EE_VPKD).ZIP

debug:
	$(MAKE) DEBUG=1 all

iopcore_debug:
	$(MAKE) DEBUG=1 IOPCORE_DEBUG=1 all

eesio_debug:
	$(MAKE) DEBUG=1 EESIO_DEBUG=1 all

ingame_debug:
	$(MAKE) DEBUG=1 INGAME_DEBUG=1 all

deci2_debug:
	$(MAKE) DEBUG=1 INGAME_DEBUG=1 DECI2_DEBUG=1 all

clean:
	echo "Cleaning..."
	echo "-Interface"
	rm -fr $(MAPFILE) $(EE_BIN) $(EE_BIN_PACKED) $(EE_BIN_STRIPPED) $(EE_VPKD).* $(EE_OBJS_DIR) $(EE_ASM_DIR)
	echo "-EE core"
	$(MAKE) -C ee_core clean
	echo "-Elf Loader"
	$(MAKE) -C elfldr clean
	echo "-IOP core"
	echo " -udnl-t300"
	$(MAKE) -C modules/iopcore/udnl-t300 clean
	echo " -udnl"
	$(MAKE) -C modules/iopcore/udnl clean
	echo " -imgdrv"
	$(MAKE) -C modules/iopcore/imgdrv clean
	echo " -eesync"
	$(MAKE) -C modules/iopcore/eesync clean
	echo " -cdvdman"
	$(MAKE) -C modules/iopcore/cdvdman USE_BDM=1 clean
	$(MAKE) -C modules/iopcore/cdvdman USE_SMB=1 clean
	$(MAKE) -C modules/iopcore/cdvdman USE_HDD=1 clean
	$(MAKE) -C modules/iopcore/cdvdman USE_HDPRO=1 clean
	echo " -cdvdfsv"
	$(MAKE) -C modules/iopcore/cdvdfsv clean
	echo " -resetspu"
	$(MAKE) -C modules/iopcore/resetspu clean
	echo "  -patches"
	echo "   -iremsnd"
	$(MAKE) -C modules/iopcore/patches/iremsndpatch clean
	echo "   -apemod"
	$(MAKE) -C modules/iopcore/patches/apemodpatch clean
	echo "   -f2techiop"
	$(MAKE) -C modules/iopcore/patches/f2techioppatch clean
	echo "   -cleareffects"
	$(MAKE) -C modules/iopcore/patches/cleareffects clean
	echo " -isofs"
	$(MAKE) -C modules/isofs clean
	echo " -bdmevent"
	$(MAKE) -C modules/bdmevent clean
	echo " -SMSUTILS"
	$(MAKE) -C modules/network/SMSUTILS clean
	echo " -SMSTCPIP"
	$(MAKE) -C modules/network/SMSTCPIP clean
	echo " -in-game SMAP"
	$(MAKE) -C modules/network/smap-ingame clean
	echo " -smbinit"
	$(MAKE) -C modules/network/smbinit clean
	echo " -nbns"
	$(MAKE) -C modules/network/nbns clean
	echo " -httpclient"
	$(MAKE) -C modules/network/httpclient clean
	echo " -xhdd"
	$(MAKE) -C modules/hdd/xhdd clean
	echo " -mcemu"
	$(MAKE) -C modules/mcemu USE_BDM=1 clean
	$(MAKE) -C modules/mcemu USE_HDD=1 clean
	$(MAKE) -C modules/mcemu USE_SMB=1 clean
	echo " -genvmc"
	$(MAKE) -C modules/vmc/genvmc clean
	echo " -lwnbdsvr"
	$(MAKE) -C modules/network/lwnbdsvr clean
	echo " -udptty-ingame"
	$(MAKE) -C modules/debug/udptty-ingame clean
	echo " -ioptrap"
	$(MAKE) -C modules/debug/ioptrap clean
	echo " -ps2link"
	$(MAKE) -C modules/debug/ps2link clean
	echo " -ds34usb"
	$(MAKE) -C modules/ds34usb clean
	echo " -ds34bt"
	$(MAKE) -C modules/ds34bt clean
	echo " -pademu"
	$(MAKE) -C modules/pademu USE_BT=1 clean
	$(MAKE) -C modules/pademu USE_USB=1 clean
	echo "-pc tools"
	$(MAKE) -C pc clean

rebuild: clean all

run: $(EE_BIN)
	ps2client -h 192.168.1.10 execee host:$(EE_BIN)

sim: $(EE_BIN)
	PCSX2 --elf=$(PWD)/$(EE_BIN) --nodisc --nogui

pc_tools:
	echo "Building iso2opl, opl2iso and genvmc..."
	$(MAKE) _WIN32=0 -C pc

pc_tools_win32:
	echo "Building WIN32 iso2opl, opl2iso and genvmc..."
	$(MAKE) _WIN32=1 -C pc

format:
	find . -type f -a \( -iname \*.h -o -iname \*.c \) | xargs clang-format -i

format-check:
	@! find . -type f -a \( -iname \*.h -o -iname \*.c \) | xargs clang-format -style=file -output-replacements-xml | grep "<replacement " >/dev/null

$(EE_ASM_DIR):
	@mkdir -p $@

$(EE_OBJS_DIR):
	@mkdir -p $@

.PHONY: DETAILED_CHANGELOG
DETAILED_CHANGELOG:
	sh make_changelog.sh

$(EE_BIN_STRIPPED): $(EE_BIN)
	echo "Stripping..."
	$(EE_STRIP) -o $@ $<

$(EE_BIN_PACKED): $(EE_BIN_STRIPPED)
	echo "Compressing..."
	ps2-packer $< $@ > /dev/null

$(EE_VPKD).ELF: $(EE_BIN_PACKED)
	cp -f $< $@

$(EE_VPKD).ZIP: $(EE_VPKD).ELF DETAILED_CHANGELOG CREDITS LICENSE README.md
	zip -r $@ $^
	echo "Package Complete: $@"

ee_core/ee_core.elf: ee_core
	echo "-EE core"
	$(MAKE) $(IGS_FLAGS) $(PADEMU_FLAGS) $(EECORE_EXTRA_FLAGS) -C $<

$(EE_ASM_DIR)ee_core.s: ee_core/ee_core.elf | $(EE_ASM_DIR)
	$(BIN2S) $< $@ eecore_elf

elfldr/elfldr.elf: elfldr
	echo "-Elf Loader"
	$(MAKE) -C $<

$(EE_ASM_DIR)elfldr.s: elfldr/elfldr.elf | $(EE_ASM_DIR)
	$(BIN2S) $< $@ elfldr_elf

$(UDNL_OUT): $(UDNL_SRC)
	echo "-IOP core"
	$(MAKE) -C $<

$(EE_ASM_DIR)udnl.s: $(UDNL_OUT) | $(EE_ASM_DIR)
	$(BIN2S) $(UDNL_OUT) $@ udnl_irx

modules/iopcore/imgdrv/imgdrv.irx: modules/iopcore/imgdrv
	$(MAKE) -C $<

$(EE_ASM_DIR)imgdrv.s: modules/iopcore/imgdrv/imgdrv.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ imgdrv_irx

modules/iopcore/eesync/eesync.irx: modules/iopcore/eesync
	$(MAKE) -C $<

$(EE_ASM_DIR)eesync.s: modules/iopcore/eesync/eesync.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ eesync_irx

modules/iopcore/cdvdman/bdm_cdvdman.irx: modules/iopcore/cdvdman
	$(MAKE) $(CDVDMAN_PS2LOGO_FLAGS) $(CDVDMAN_DEBUG_FLAGS) USE_BDM=1 -C $< all

$(EE_ASM_DIR)bdm_cdvdman.s: modules/iopcore/cdvdman/bdm_cdvdman.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ bdm_cdvdman_irx

modules/iopcore/cdvdman/smb_cdvdman.irx: modules/iopcore/cdvdman
	$(MAKE) $(CDVDMAN_PS2LOGO_FLAGS) $(CDVDMAN_DEBUG_FLAGS) USE_SMB=1 -C $< all

$(EE_ASM_DIR)smb_cdvdman.s: modules/iopcore/cdvdman/smb_cdvdman.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ smb_cdvdman_irx

modules/iopcore/cdvdman/hdd_cdvdman.irx: modules/iopcore/cdvdman
	$(MAKE) $(CDVDMAN_PS2LOGO_FLAGS) $(CDVDMAN_DEBUG_FLAGS) USE_HDD=1 -C $< all

$(EE_ASM_DIR)hdd_cdvdman.s: modules/iopcore/cdvdman/hdd_cdvdman.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ hdd_cdvdman_irx

modules/iopcore/cdvdman/hdd_hdpro_cdvdman.irx: modules/iopcore/cdvdman
	$(MAKE) $(CDVDMAN_PS2LOGO_FLAGS) $(CDVDMAN_DEBUG_FLAGS) USE_HDPRO=1 -C $< all

$(EE_ASM_DIR)hdd_hdpro_cdvdman.s: modules/iopcore/cdvdman/hdd_hdpro_cdvdman.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ hdd_hdpro_cdvdman_irx

modules/iopcore/cdvdfsv/cdvdfsv.irx: modules/iopcore/cdvdfsv
	$(MAKE) -C $<

$(EE_ASM_DIR)cdvdfsv.s: modules/iopcore/cdvdfsv/cdvdfsv.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ cdvdfsv_irx

modules/iopcore/patches/iremsndpatch/iremsndpatch.irx: modules/iopcore/patches/iremsndpatch
	$(MAKE) -C $<

$(EE_ASM_DIR)iremsndpatch.s: modules/iopcore/patches/iremsndpatch/iremsndpatch.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ iremsndpatch_irx

modules/iopcore/patches/apemodpatch/apemodpatch.irx: modules/iopcore/patches/apemodpatch
	$(MAKE) -C $<

$(EE_ASM_DIR)apemodpatch.s: modules/iopcore/patches/apemodpatch/apemodpatch.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ apemodpatch_irx

modules/iopcore/patches/f2techioppatch/f2techioppatch.irx: modules/iopcore/patches/f2techioppatch
	$(MAKE) -C $<

$(EE_ASM_DIR)f2techioppatch.s: modules/iopcore/patches/f2techioppatch/f2techioppatch.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ f2techioppatch_irx

modules/iopcore/patches/cleareffects/cleareffects.irx: modules/iopcore/patches/cleareffects
	$(MAKE) -C $<

$(EE_ASM_DIR)cleareffects.s: modules/iopcore/patches/cleareffects/cleareffects.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ cleareffects_irx

modules/iopcore/resetspu/resetspu.irx: modules/iopcore/resetspu
	$(MAKE) -C $<

$(EE_ASM_DIR)resetspu.s: modules/iopcore/resetspu/resetspu.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ resetspu_irx

modules/mcemu/bdm_mcemu.irx: modules/mcemu
	$(MAKE) $(MCEMU_DEBUG_FLAGS) $(PADEMU_FLAGS) USE_BDM=1 -C $< all

$(EE_ASM_DIR)bdm_mcemu.s: modules/mcemu/bdm_mcemu.irx
	$(BIN2S) $< $@ bdm_mcemu_irx

modules/mcemu/hdd_mcemu.irx: modules/mcemu
	$(MAKE) $(MCEMU_DEBUG_FLAGS) $(PADEMU_FLAGS) USE_HDD=1 -C $< all

$(EE_ASM_DIR)hdd_mcemu.s: modules/mcemu/hdd_mcemu.irx
	$(BIN2S) $< $@ hdd_mcemu_irx

modules/mcemu/smb_mcemu.irx: modules/mcemu
	$(MAKE) $(MCEMU_DEBUG_FLAGS) $(PADEMU_FLAGS) USE_SMB=1 -C $< all

$(EE_ASM_DIR)smb_mcemu.s: modules/mcemu/smb_mcemu.irx
	$(BIN2S) $< $@ smb_mcemu_irx

modules/isofs/isofs.irx: modules/isofs
	$(MAKE) -C $<

$(EE_ASM_DIR)isofs.s: modules/isofs/isofs.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ isofs_irx

$(EE_ASM_DIR)usbd.s: $(PS2SDK)/iop/irx/usbd.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ usbd_irx

$(EE_ASM_DIR)libsd.s: $(PS2SDK)/iop/irx/libsd.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ libsd_irx

$(EE_ASM_DIR)audsrv.s: $(PS2SDK)/iop/irx/audsrv.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ audsrv_irx

$(EE_OBJS_DIR)libds34bt.a: modules/ds34bt/ee/libds34bt.a
	cp $< $@

modules/ds34bt/ee/libds34bt.a: modules/ds34bt/ee
	$(MAKE) -C $<

modules/ds34bt/iop/ds34bt.irx: modules/ds34bt/iop
	$(MAKE) -C $<

$(EE_ASM_DIR)ds34bt.s: modules/ds34bt/iop/ds34bt.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ds34bt_irx

$(EE_OBJS_DIR)libds34usb.a: modules/ds34usb/ee/libds34usb.a
	cp $< $@

modules/ds34usb/ee/libds34usb.a: modules/ds34usb/ee
	$(MAKE) -C $<

modules/ds34usb/iop/ds34usb.irx: modules/ds34usb/iop
	$(MAKE) -C $<

$(EE_ASM_DIR)ds34usb.s: modules/ds34usb/iop/ds34usb.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ds34usb_irx

modules/pademu/bt_pademu.irx: modules/pademu
	$(MAKE) -C $< USE_BT=1

$(EE_ASM_DIR)bt_pademu.s: modules/pademu/bt_pademu.irx
	$(BIN2S) $< $@ bt_pademu_irx

modules/pademu/usb_pademu.irx: modules/pademu
	$(MAKE) -C $< USE_USB=1

$(EE_ASM_DIR)usb_pademu.s: modules/pademu/usb_pademu.irx
	$(BIN2S) $< $@ usb_pademu_irx

$(EE_ASM_DIR)bdm.s: $(PS2SDK)/iop/irx/bdm.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ bdm_irx

$(EE_ASM_DIR)bdmfs_vfat.s: $(PS2SDK)/iop/irx/bdmfs_vfat.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ bdmfs_vfat_irx

$(EE_ASM_DIR)iLinkman.s: $(PS2SDK)/iop/irx/iLinkman.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ iLinkman_irx

ifeq ($(DEBUG),1)
# block device drivers with printf's
$(EE_ASM_DIR)usbmass_bd.s: $(PS2SDK)/iop/irx/usbmass_bd.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ usbmass_bd_irx

$(EE_ASM_DIR)IEEE1394_bd.s: $(PS2SDK)/iop/irx/IEEE1394_bd.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ IEEE1394_bd_irx

$(EE_ASM_DIR)mx4sio_bd.s: $(PS2SDK)/iop/irx/mx4sio_bd.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ mx4sio_bd_irx
else
# block device drivers without printf's
$(EE_ASM_DIR)usbmass_bd.s: $(PS2SDK)/iop/irx/usbmass_bd_mini.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ usbmass_bd_irx

$(EE_ASM_DIR)IEEE1394_bd.s: $(PS2SDK)/iop/irx/IEEE1394_bd_mini.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ IEEE1394_bd_irx

$(EE_ASM_DIR)mx4sio_bd.s: $(PS2SDK)/iop/irx/mx4sio_bd_mini.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ mx4sio_bd_irx
endif

modules/bdmevent/bdmevent.irx: modules/bdmevent
	$(MAKE) -C $<

$(EE_ASM_DIR)bdmevent.s: modules/bdmevent/bdmevent.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ bdmevent_irx

$(EE_ASM_DIR)ps2dev9.s: $(PS2SDK)/iop/irx/ps2dev9.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ps2dev9_irx

modules/network/SMSUTILS/SMSUTILS.irx: modules/network/SMSUTILS
	$(MAKE) -C $<

$(EE_ASM_DIR)smsutils.s: modules/network/SMSUTILS/SMSUTILS.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ smsutils_irx

$(EE_ASM_DIR)ps2ip.s: $(PS2SDK)/iop/irx/ps2ip-nm.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ps2ip_irx

modules/network/SMSTCPIP/SMSTCPIP.irx: modules/network/SMSTCPIP
	$(MAKE) $(SMSTCPIP_INGAME_CFLAGS) -C $< rebuild

$(EE_ASM_DIR)ingame_smstcpip.s: modules/network/SMSTCPIP/SMSTCPIP.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ingame_smstcpip_irx

modules/network/smap-ingame/smap.irx: modules/network/smap-ingame
	$(MAKE) -C $<

$(EE_ASM_DIR)smap_ingame.s: modules/network/smap-ingame/smap.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ smap_ingame_irx

$(EE_ASM_DIR)smap.s: $(PS2SDK)/iop/irx/smap.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ smap_irx

$(EE_ASM_DIR)netman.s: $(PS2SDK)/iop/irx/netman.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ netman_irx

$(EE_ASM_DIR)ps2ips.s: $(PS2SDK)/iop/irx/ps2ips.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ps2ips_irx

$(EE_ASM_DIR)smbman.s: $(PS2SDK)/iop/irx/smbman.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ smbman_irx

modules/network/smbinit/smbinit.irx: modules/network/smbinit
	$(MAKE) -C $<

$(EE_ASM_DIR)smbinit.s: modules/network/smbinit/smbinit.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ smbinit_irx

$(EE_ASM_DIR)ps2atad.s: $(PS2SDK)/iop/irx/ps2atad.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ps2atad_irx

$(EE_ASM_DIR)hdpro_atad.s: $(PS2SDK)/iop/irx/hdproatad.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ hdpro_atad_irx

$(EE_ASM_DIR)poweroff.s: $(PS2SDK)/iop/irx/poweroff.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ poweroff_irx

modules/hdd/xhdd/xhdd.irx: modules/hdd/xhdd
	$(MAKE) -C $<

$(EE_ASM_DIR)xhdd.s: modules/hdd/xhdd/xhdd.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ xhdd_irx

$(EE_ASM_DIR)ps2hdd.s: $(PS2SDK)/iop/irx/ps2hdd-osd.irx
	$(BIN2S) $< $@ ps2hdd_irx

$(EE_ASM_DIR)ps2fs.s: $(PS2SDK)/iop/irx/ps2fs-osd.irx
	$(BIN2S) $< $@ ps2fs_irx

modules/vmc/genvmc/genvmc.irx: modules/vmc/genvmc
	$(MAKE) $(MOD_DEBUG_FLAGS) -C $<

$(EE_ASM_DIR)genvmc.s: modules/vmc/genvmc/genvmc.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ genvmc_irx

modules/network/lwnbdsvr/lwnbdsvr.irx: modules/network/lwnbdsvr
	$(MAKE) -C $<

$(EE_ASM_DIR)lwnbdsvr.s: modules/network/lwnbdsvr/lwnbdsvr.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ lwnbdsvr_irx

$(EE_ASM_DIR)udptty.s: $(PS2SDK)/iop/irx/udptty.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ udptty_irx

modules/debug/udptty-ingame/udptty.irx: modules/debug/udptty-ingame
	$(MAKE) -C $<

$(EE_ASM_DIR)udptty-ingame.s: modules/debug/udptty-ingame/udptty.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ udptty_ingame_irx

modules/debug/ioptrap/ioptrap.irx: modules/debug/ioptrap
	$(MAKE) -C $<

$(EE_ASM_DIR)ioptrap.s: modules/debug/ioptrap/ioptrap.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ioptrap_irx

modules/debug/ps2link/ps2link.irx: modules/debug/ps2link
	$(MAKE) -C $<

$(EE_ASM_DIR)ps2link.s: modules/debug/ps2link/ps2link.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ps2link_irx

modules/network/nbns/nbns.irx: modules/network/nbns
	$(MAKE) -C $<

$(EE_ASM_DIR)nbns-iop.s: modules/network/nbns/nbns.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ nbns_irx

modules/network/httpclient/httpclient.irx: modules/network/httpclient
	$(MAKE) -C $<

$(EE_ASM_DIR)httpclient-iop.s: modules/network/httpclient/httpclient.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ httpclient_irx

$(EE_ASM_DIR)iomanx.s: $(PS2SDK)/iop/irx/iomanX.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ iomanx_irx

$(EE_ASM_DIR)filexio.s: $(PS2SDK)/iop/irx/fileXio.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ filexio_irx

$(EE_ASM_DIR)sio2man.s: $(PS2SDK)/iop/irx/freesio2.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ sio2man_irx

$(EE_ASM_DIR)padman.s: $(PS2SDK)/iop/irx/freepad.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ padman_irx

$(EE_ASM_DIR)mcman.s: $(PS2SDK)/iop/irx/mcman.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ mcman_irx

$(EE_ASM_DIR)mcserv.s: $(PS2SDK)/iop/irx/mcserv.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ mcserv_irx

$(EE_ASM_DIR)load0.s: gfx/load0.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ load0_png

$(EE_ASM_DIR)load1.s: gfx/load1.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ load1_png

$(EE_ASM_DIR)load2.s: gfx/load2.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ load2_png

$(EE_ASM_DIR)load3.s: gfx/load3.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ load3_png

$(EE_ASM_DIR)load4.s: gfx/load4.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ load4_png

$(EE_ASM_DIR)load5.s: gfx/load5.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ load5_png

$(EE_ASM_DIR)load6.s: gfx/load6.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ load6_png

$(EE_ASM_DIR)load7.s: gfx/load7.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ load7_png

$(EE_ASM_DIR)usb_icon.s: gfx/usb.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ usb_png

$(EE_ASM_DIR)hdd_icon.s: gfx/hdd.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ hdd_png

$(EE_ASM_DIR)eth_icon.s: gfx/eth.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ eth_png

$(EE_ASM_DIR)app_icon.s: gfx/app.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ app_png

$(EE_ASM_DIR)cross_icon.s: gfx/cross.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ cross_png

$(EE_ASM_DIR)triangle_icon.s: gfx/triangle.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ triangle_png

$(EE_ASM_DIR)circle_icon.s: gfx/circle.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ circle_png

$(EE_ASM_DIR)square_icon.s: gfx/square.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ square_png

$(EE_ASM_DIR)select_icon.s: gfx/select.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ select_png

$(EE_ASM_DIR)start_icon.s: gfx/start.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ start_png

$(EE_ASM_DIR)left_icon.s: gfx/left.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ left_png

$(EE_ASM_DIR)right_icon.s: gfx/right.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ right_png

$(EE_ASM_DIR)up_icon.s: gfx/up.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ up_png

$(EE_ASM_DIR)down_icon.s: gfx/down.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ down_png

$(EE_ASM_DIR)background.s: gfx/background.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ background_png

$(EE_ASM_DIR)info.s: gfx/info.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ info_png

$(EE_ASM_DIR)cover.s: gfx/cover.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ cover_png

$(EE_ASM_DIR)disc.s: gfx/disc.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ disc_png

$(EE_ASM_DIR)screen.s: gfx/screen.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ screen_png

$(EE_ASM_DIR)ELF.s: gfx/ELF.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ELF_png

$(EE_ASM_DIR)HDL.s: gfx/HDL.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ HDL_png

$(EE_ASM_DIR)ISO.s: gfx/ISO.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ISO_png

$(EE_ASM_DIR)UL.s: gfx/UL.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ UL_png

$(EE_ASM_DIR)CD.s: gfx/CD.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ CD_png

$(EE_ASM_DIR)DVD.s: gfx/DVD.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ DVD_png

$(EE_ASM_DIR)Aspect_s.s: gfx/Aspect_s.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Aspect_s_png

$(EE_ASM_DIR)Aspect_w.s: gfx/Aspect_w.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Aspect_w_png

$(EE_ASM_DIR)Aspect_w1.s: gfx/Aspect_w1.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Aspect_w1_png

$(EE_ASM_DIR)Aspect_w2.s: gfx/Aspect_w2.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Aspect_w2_png

$(EE_ASM_DIR)Device_1.s: gfx/Device_1.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Device_1_png

$(EE_ASM_DIR)Device_2.s: gfx/Device_2.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Device_2_png

$(EE_ASM_DIR)Device_3.s: gfx/Device_3.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Device_3_png

$(EE_ASM_DIR)Device_4.s: gfx/Device_4.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Device_4_png

$(EE_ASM_DIR)Device_5.s: gfx/Device_5.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Device_5_png

$(EE_ASM_DIR)Device_6.s: gfx/Device_6.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Device_6_png

$(EE_ASM_DIR)Device_all.s: gfx/Device_all.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Device_all_png

$(EE_ASM_DIR)Rating_0.s: gfx/Rating_0.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Rating_0_png

$(EE_ASM_DIR)Rating_1.s: gfx/Rating_1.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Rating_1_png

$(EE_ASM_DIR)Rating_2.s: gfx/Rating_2.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Rating_2_png

$(EE_ASM_DIR)Rating_3.s: gfx/Rating_3.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Rating_3_png

$(EE_ASM_DIR)Rating_4.s: gfx/Rating_4.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Rating_4_png

$(EE_ASM_DIR)Rating_5.s: gfx/Rating_5.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Rating_5_png

$(EE_ASM_DIR)Scan_240p.s: gfx/Scan_240p.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Scan_240p_png

$(EE_ASM_DIR)Scan_240p1.s: gfx/Scan_240p1.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Scan_240p1_png

$(EE_ASM_DIR)Scan_480i.s: gfx/Scan_480i.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Scan_480i_png

$(EE_ASM_DIR)Scan_480p.s: gfx/Scan_480p.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Scan_480p_png

$(EE_ASM_DIR)Scan_480p1.s: gfx/Scan_480p1.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Scan_480p1_png

$(EE_ASM_DIR)Scan_480p2.s: gfx/Scan_480p2.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Scan_480p2_png

$(EE_ASM_DIR)Scan_480p3.s: gfx/Scan_480p3.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Scan_480p3_png

$(EE_ASM_DIR)Scan_480p4.s: gfx/Scan_480p4.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Scan_480p4_png

$(EE_ASM_DIR)Scan_480p5.s: gfx/Scan_480p5.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Scan_480p5_png

$(EE_ASM_DIR)Scan_576i.s: gfx/Scan_576i.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Scan_576i_png

$(EE_ASM_DIR)Scan_576p.s: gfx/Scan_576p.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Scan_576p_png

$(EE_ASM_DIR)Scan_720p.s: gfx/Scan_720p.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Scan_720p_png

$(EE_ASM_DIR)Scan_1080i.s: gfx/Scan_1080i.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Scan_1080i_png

$(EE_ASM_DIR)Scan_1080i2.s: gfx/Scan_1080i2.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Scan_1080i2_png

$(EE_ASM_DIR)Scan_1080p.s: gfx/Scan_1080p.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Scan_1080p_png

$(EE_ASM_DIR)Vmode_multi.s: gfx/Vmode_multi.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Vmode_multi_png

$(EE_ASM_DIR)Vmode_ntsc.s: gfx/Vmode_ntsc.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Vmode_ntsc_png

$(EE_ASM_DIR)Vmode_pal.s: gfx/Vmode_pal.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ Vmode_pal_png

$(EE_ASM_DIR)logo.s: gfx/logo.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ logo_png

$(EE_ASM_DIR)case.s: gfx/case.png | $(EE_ASM_DIR)
	$(BIN2S) $< $@ case_png

$(EE_ASM_DIR)poeveticanew.s: thirdparty/PoeVeticaNew.ttf | $(EE_ASM_DIR)
	$(BIN2S) $< $@ poeveticanew_raw

$(EE_ASM_DIR)icon_sys.s: gfx/icon.sys | $(EE_ASM_DIR)
	$(BIN2S) $< $@ icon_sys

$(EE_ASM_DIR)icon_icn.s: gfx/opl.icn | $(EE_ASM_DIR)
	$(BIN2S) $< $@ icon_icn

$(EE_ASM_DIR)icon_sys_A.s: misc/icon_A.sys | $(EE_ASM_DIR)
	$(BIN2S) $< $@ icon_sys_A

$(EE_ASM_DIR)icon_sys_J.s: misc/icon_J.sys | $(EE_ASM_DIR)
	$(BIN2S) $< $@ icon_sys_J

$(EE_ASM_DIR)icon_sys_C.s: misc/icon_C.sys | $(EE_ASM_DIR)
	$(BIN2S) $< $@ icon_sys_C

$(EE_ASM_DIR)conf_theme_OPL.s: misc/conf_theme_OPL.cfg | $(EE_ASM_DIR)
	$(BIN2S) $< $@ conf_theme_OPL_cfg

$(EE_ASM_DIR)boot.s: misc/boot.adp | $(EE_ASM_DIR)
	$(BIN2S) $< $@ boot_adp

$(EE_ASM_DIR)cancel.s: misc/cancel.adp | $(EE_ASM_DIR)
	$(BIN2S) $< $@ cancel_adp

$(EE_ASM_DIR)confirm.s: misc/confirm.adp | $(EE_ASM_DIR)
	$(BIN2S) $< $@ confirm_adp

$(EE_ASM_DIR)cursor.s: misc/cursor.adp | $(EE_ASM_DIR)
	$(BIN2S) $< $@ cursor_adp

$(EE_ASM_DIR)message.s: misc/message.adp | $(EE_ASM_DIR)
	$(BIN2S) $< $@ message_adp

$(EE_ASM_DIR)transition.s: misc/transition.adp | $(EE_ASM_DIR)
	$(BIN2S) $< $@ transition_adp

$(EE_ASM_DIR)IOPRP_img.s: modules/iopcore/IOPRP.img | $(EE_ASM_DIR)
	$(BIN2S) $< $@ IOPRP_img

$(EE_ASM_DIR)drvtif_ingame_irx.s: modules/debug/drvtif-ingame.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ drvtif_ingame_irx

$(EE_ASM_DIR)tifinet_ingame_irx.s: modules/debug/tifinet-ingame.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ tifinet_ingame_irx

$(EE_ASM_DIR)drvtif_irx.s: modules/debug/drvtif.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ drvtif_irx

$(EE_ASM_DIR)tifinet_irx.s: modules/debug/tifinet.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ tifinet_irx

$(EE_ASM_DIR)deci2_img.s: modules/debug/deci2.img | $(EE_ASM_DIR)
	$(BIN2S) $< $@ deci2_img

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.c | $(EE_OBJS_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)%.o: $(EE_ASM_DIR)%.s | $(EE_OBJS_DIR)
	$(EE_AS) $(EE_ASFLAGS) $< -o $@

endif

ifndef PS2SDK
ps2sdk-not-setup:
	@echo "PS2SDK is not setup. Please setup PS2SDK before building this project"
endif

oplversion:
	@echo $(OPL_VERSION)

ifdef PS2SDK
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
endif
