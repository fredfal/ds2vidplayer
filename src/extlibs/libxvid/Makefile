ifeq ($(strip $(DS2SDKPATH)),)
$(error "Please set DS2SDKPATH in your environment. export DS2SDKPATH=<path to>ds2sdk")
endif

CROSS := /opt/mipsel-4.1.2-nopic/bin/

FS_DIR = $(DS2SDKPATH)/libsrc/fs/
CONSOLE_DIR = $(DS2SDKPATH)/libsrc/console
KEY_DIR = $(DS2SDKPATH)/libsrc/key
ZLIB_DIR = $(DS2SDKPATH)/libsrc/zlib

CC = $(CROSS)mipsel-linux-gcc

INC := -I$(FS_DIR) -I$(DS2SDKPATH)/include -I$(ZLIB_DIR) -include "stdio.h" -include "fs_api.h" -include "ds2_malloc.h"
CFLAGS := -mips32 -O3 -mno-abicalls -fno-pic -fno-builtin \
          -fno-exceptions -ffunction-sections -mno-long-calls\
          -fomit-frame-pointer -msoft-float -G 4 -Wmissing-include-dirs -fgcse-sm -fgcse-las -fgcse-after-reload \
          -fweb -fpeel-loops $(INC)
AR = $(CROSS)mipsel-linux-ar
ARFLAGS = rcsv
LDFLAGS = -L$(DS2SDKPATH)/lib
LIBS=-lds2a

RANLIB=$(CROSS)/../mipsel-linux/bin/ranlib

BUILD_DIR = xvidcore/build/generic
TARGET = libxvidcore.a
TARGET_DIR = =build
TARGET_PATH = $(BUILD_DIR)/$(TARGET_DIR)

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
all: $(CURDIR)/../$(TARGET)

$(CURDIR)/../$(TARGET): $(TARGET_PATH)/$(TARGET)
	ls $(TARGET_PATH)
	cp $(TARGET_PATH)/$(TARGET) $(CURDIR)/../$(TARGET)

$(TARGET_PATH)/$(TARGET): $(BUILD_DIR)/platform.inc FORCE
	make -C $(BUILD_DIR) $(TARGET)

FORCE:
	

$(BUILD_DIR)/platform.inc:
	cd $(BUILD_DIR); CC=$(CC) CFLAGS="$(CFLAGS)" AR=$(AR) ARFLAGS="$(ARFLAGS)" LDFLAGS="$(LDFLAGS)" RANLIB=$(RANLIB) LIBS="$(LIBS)" ./configure --host=mipsel-linux --enable-static --disable-shared --disable-pthread

clean:
	cd $(BUILD_DIR); make clean
	rm -f $(CURDIR)/../$(TARGET) $(BUILD_DIR)/platform.inc
