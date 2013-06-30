ifeq ($(strip $(DS2SDKPATH)),)
$(error "Please set DS2SDKPATH in your environment. export DS2SDKPATH=<path to>ds2sdk")
endif

DIRS := libxvid libmad libsamplerate

all:
	for DIR in $(DIRS); do make -C $$DIR; done

clean:
	rm *.a
	for DIR in $(DIRS); do make -C $$DIR clean; done
