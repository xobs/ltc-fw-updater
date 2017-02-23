DIRNAME    = $(shell basename $(shell pwd))
PACKAGE    = $(firstword $(subst -, , $(DIRNAME)))
LIBS       =
VERSION    = v1.0
ADD_CFLAGS =
ADD_LFLAGS =
EXTRA_DIST =

CC         = $(TRGT)gcc
CXX        = $(TRGT)gcc
OBJCOPY    = $(TRGT)objcopy

DESTDIR ?= /usr
PREFIX ?= $(DESTDIR)

LDSCRIPT = ld/KL02Z32-app.ld
DBG_CFLAGS = -ggdb -g -DDEBUG -Wall
DBG_LFLAGS = -ggdb -g -Wall
CFLAGS     = $(ADD_CFLAGS) \
             -DVERSION=\"$(VERSION)\" \
             -Iinclude \
             -fsingle-precision-constant -Wall -Wextra \
             -mcpu=cortex-m0plus -mfloat-abi=soft -mthumb \
             -fno-builtin \
             -ffunction-sections -fdata-sections -fno-common \
             -fomit-frame-pointer -falign-functions=16 -nostdlib -Os
CXXFLAGS   = $(CFLAGS) -std=c++11 -fno-rtti -fno-exceptions
LFLAGS     = $(ADD_LFLAGS) $(CFLAGS) \
             -nostartfiles -nostdlib -nodefaultlibs \
             -Wl,--gc-sections \
             -Wl,--no-warn-mismatch,--script=$(LDSCRIPT),--build-id=none

OBJ_DIR    = .obj/

CSOURCES   = $(wildcard src/*.c)
CPPSOURCES = $(wildcard src/*.cpp)
ASOURCES   = $(wildcard src/*.S)
COBJS      = $(addprefix $(OBJ_DIR)/, $(notdir $(CSOURCES:.c=.o)))
CXXOBJS    = $(addprefix $(OBJ_DIR)/, $(notdir $(CPPSOURCES:.cpp=.o)))
AOBJS      = $(addprefix $(OBJ_DIR)/, $(notdir $(ASOURCES:.S=.o)))
OBJECTS    = $(COBJS) $(CXXOBJS) $(AOBJS)
VPATH = . ../common

QUIET      = @

ALL        = all
TARGET     = $(PACKAGE).elf
DEBUG      = debug
REBUILD    = rebuild
DREBUILD   = drebuild
CLEAN      = clean
CHANGELOG  = ChangeLog.txt
DISTCLEAN  = distclean
DIST       = dist
DDIST      = dailydist
INSTALL    = install
INIT       = init

$(ALL): $(TARGET)

$(OBJECTS): | $(OBJ_DIR)

$(TARGET): $(OBJECTS) $(LDSCRIPT)
	$(QUIET) echo "  LD       $@"
	$(QUIET) $(CXX) $(OBJECTS) $(LFLAGS) -o $@
	$(QUIET) echo "  OBJCOPY  $(PACKAGE).bin"
	$(QUIET) $(OBJCOPY) -O binary $@ $(PACKAGE).bin
	$(QUIET) echo "  SYMBOL   $(PACKAGE).symbol"
	$(QUIET) $(OBJCOPY) --only-keep-debug $< $(PACKAGE).symbol 2> /dev/null

$(DEBUG): CFLAGS += $(DBG_CFLAGS)
$(DEBUG): LFLAGS += $(DBG_LFLAGS)
CFLAGS += $(DBG_CFLAGS)
LFLAGS += $(DBG_LFLAGS)
$(DEBUG): $(TARGET)

$(OBJ_DIR):
	$(QUIET) mkdir -p $(OBJ_DIR)

$(COBJS) : $(OBJ_DIR)/%.o : src/%.c Makefile
	$(QUIET) echo "  CC       $<	$(notdir $@)"
	$(QUIET) $(CC) -c $< $(CFLAGS) -o $@ -MMD

$(OBJ_DIR)/%.o: src/%.cpp
	$(QUIET) echo "  CXX      $<	$(notdir $@)"
	$(QUIET) $(CXX) -c $< $(CXXFLAGS) -o $@ -MMD

$(OBJ_DIR)/%.o: src/%.S
	$(QUIET) echo "  AS       $<	$(notdir $@)"
	$(QUIET) $(CC) -x assembler-with-cpp -c $< $(CFLAGS) -o $@ -MMD

#$(OBJ_DIR)/crc16.o : CFLAGS += -O3

.PHONY: $(CLEAN) $(DISTCLEAN) $(DIST) $(REBUILD) $(DREBUILD) $(INSTALL) \
        $(CHANGELOG) $(INIT)

$(CLEAN):
	$(QUIET) rm -f $(wildcard $(OBJ_DIR)/*.d)
	$(QUIET) rm -f $(wildcard $(OBJ_DIR)/*.o)
	$(QUIET) rm -f $(TARGET) $(PACKAGE).bin $(PACKAGE).symbol

$(DISTCLEAN): $(CLEAN)
	$(QUIET) rm -rf $(OBJ_DIR) $(wildcard $(TARGET)-*.tar.gz)

include $(wildcard $(OBJ_DIR)/*.d)
