ifeq ($(strip $(DS2SDKPATH)),)
$(error "Please set DS2SDKPATH in your environment. export DS2SDKPATH=<path to>ds2sdk")
endif

# CROSS :=#
CROSS := /opt/mipsel-4.1.2-nopic/bin/

CC = $(CROSS)mipsel-linux-gcc
AR = $(CROSS)mipsel-linux-ar rcsv
MXU_AS	= $(CROSS)mxu_as
LD	= $(CROSS)mipsel-linux-ld
OBJCOPY	= $(CROSS)mipsel-linux-objcopy
NM	= $(CROSS)mipsel-linux-nm
OBJDUMP	= $(CROSS)mipsel-linux-objdump

TOPDIR = .
SRC_DIR = $(TOPDIR)/src
FS_DIR = $(DS2SDKPATH)/libsrc/fs
CONSOLE_DIR = $(DS2SDKPATH)/libsrc/console
KEY_DIR = $(DS2SDKPATH)/libsrc/key
ZLIB_DIR = $(DS2SDKPATH)/libsrc/zlib
XVID_DIR = $(SRC_DIR)/extlibs/libxvid/xvidcore/src
MAD_DIR = $(SRC_DIR)/extlibs/libmad/libmad-0.15.1b

SRC := $(SRC_DIR)/main.c \
	$(SRC_DIR)/ds2_main.c \
	$(SRC_DIR)/video.c \
	$(SRC_DIR)/avi_buffer.c\
	$(SRC_DIR)/player.c \
	$(SRC_DIR)/sound.c \

XSRC := $(SRC_DIR)/jz4740_specific.c
	

EXTLIBS := -lxvidcore -lmad
LIBS := $(EXTLIBS) -lds2a -lds2b -lm -lc -lgcc

INC :=  -I$(DS2SDKPATH)/include -I$(FS_DIR) -I$(SRC_DIR) -I$(CONSOLE_DIR) -I$(KEY_DIR) -I$(ZLIB_DIR) -I$(XVID_DIR) -I$(MAD_DIR)

CFLAGS := -mips32 -O3 -mno-abicalls -fno-pic -fno-builtin \
	   -fno-exceptions -ffunction-sections -mno-long-calls\
	   -fomit-frame-pointer -msoft-float -G 4 -fgcse-sm -fgcse-las -fgcse-after-reload \
          -fweb -fpeel-loops

LDFLAGS := -L$(DS2SDKPATH)/lib -L$(SRC_DIR)/extlibs/

LINKS := $(DS2SDKPATH)/specs/link.xn
STARTS := $(DS2SDKPATH)/specs/start.S
STARTO := start.o

OBJS	:= $(SRC:.c=.o)
XOBJS	:= $(XSRC:.c=.xo)

APP	:= ds2vidplayer.elf

all: $(APP)
	$(OBJCOPY) -O binary $(APP) ds2vidplayer.bin
	$(OBJDUMP) -d $(APP) > ds2vidplayer.dump
	$(NM) $(APP) | sort > ds2vidplayer.sym
	$(OBJDUMP) -h $(APP) > ds2vidplayer.map
	$(DS2SDKPATH)/tools/makeplug ds2vidplayer.bin d2vidplayer.plg

$(APP):	$(EXTLIBS) depend $(XOBJS) $(OBJS) $(STARTO) $(LINKS)
	$(CC) -nostdlib -static -T $(LINKS) $(LDFLAGS) -o $@ $(STARTO) $(XOBJS) $(OBJS) $(LIBS)

$(EXTLIBS): extlibs

$(STARTO):
	$(CC) $(CFLAGS) $(INC) -o $@ -c $(STARTS)

%.mid: %.c
	$(CC) $(CFLAGS) $(INC) -S -o $@ -c $<
%.o: %.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<
%.xo: %.S
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<
%.S: %.mid
	$(MXU_AS) $< > $@

extlibs:
	cd $(SRC_DIR)/extlibs; make

clean:
	rm -fr $(OBJS) $(XOBJS) $(OTHER) *.bin *.sym *.map *.dump *.lib *.plg *.elf *.o depend
	make -C $(SRC_DIR)/extlibs clean

depend:
	$(CC) -MM $(CFLAGS) $(INC) $(SSRC) $(SRC) > $@

sinclude depend

