BUILD_FOLDER := bin
OUTPUT_PRX   := $(shell basename $(CURDIR))
TARGET       := $(BUILD_FOLDER)/$(OUTPUT_PRX)
TARGET_ELF   := $(BUILD_FOLDER)/$(OUTPUT_PRX)
TARGETSTUB   := $(OUTPUT_PRX).so
O_FLAG       := -O3

# Libraries linked into the ELF.
LIBS := -lGoldHEN_Hook -lkernel -lSceLibcInternal -lSceSysmodule

EXTRAFLAGS := -D__USE_KLOG__ -D__FINAL__=1

# You likely won't need to touch anything below this point.
# Root vars
TOOLCHAIN     := $(OO_PS4_TOOLCHAIN)
GH_SDK        := $(GOLDHEN_SDK)
PROJDIR       := ../$(shell basename $(CURDIR))/source
INTDIR        := ../$(shell basename $(CURDIR))/build
INCLUDEDIR    := ../$(shell basename $(CURDIR))/include

# Define objects to build
CFILES      := $(wildcard $(PROJDIR)/*.c)
CPPFILES    := $(wildcard $(PROJDIR)/*.cpp)
COMMONFILES := $(wildcard $(COMMONDIR)/*.cpp)
OBJS        := $(patsubst $(PROJDIR)/%.c, $(INTDIR)/%.o, $(CFILES)) $(patsubst $(PROJDIR)/%.cpp, $(INTDIR)/%.o, $(CPPFILES)) $(patsubst $(COMMONDIR)/%.cpp, $(INTDIR)/%.o, $(COMMONFILES))
STUBOBJS    := $(patsubst $(PROJDIR)/%.c, $(INTDIR)/%.o, $(CFILES)) $(patsubst $(PROJDIR)/%.cpp, $(INTDIR)/%.o.stub, $(CPPFILES)) $(patsubst $(COMMONDIR)/%.cpp, $(INTDIR)/%.o.stub, $(COMMONFILES))

# Define final C/C++ flags
CFLAGS      := --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -c $(EXTRAFLAGS) -isysroot $(TOOLCHAIN) -isystem $(TOOLCHAIN)/include -I$(GH_SDK)/include -I$(INCLUDEDIR) -I$(COMMON_DIR) $(O_FLAG)
CXXFLAGS    := $(CFLAGS) -isystem $(TOOLCHAIN)/$(INCLUDEDIR)/c++/v1
LDFLAGS     := -m elf_x86_64 -pie --script $(TOOLCHAIN)/link.x -e _init --eh-frame-hdr -L$(TOOLCHAIN)/lib -L$(GH_SDK) $(LIBS) 

# Create the build and bin directories, in case they don't exist already
_unused     := $(shell mkdir -p $(INTDIR))
_unused     := $(shell mkdir -p $(BUILD_FOLDER))

# Check for linux vs macOS and account for clang/ld path
UNAME_S     := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
        CC      := clang
        CCX     := clang++
        LD      := ld.lld
        CDIR    := linux
endif
ifeq ($(UNAME_S),Darwin)
        CC      := /usr/local/opt/llvm/bin/clang
        CCX     := /usr/local/opt/llvm/bin/clang++
        LD      := /usr/local/opt/llvm/bin/ld.lld
        CDIR    := macos
endif

$(TARGET): $(INTDIR) $(OBJS)
	$(LD) $(GH_SDK)/build/crtprx.o $(INTDIR)/*.o -o $(TARGET_ELF).elf $(LDFLAGS)
	$(TOOLCHAIN)/bin/$(CDIR)/create-fself -in=$(TARGET_ELF).elf -out=$(TARGET_ELF).oelf --lib=$(TARGET).prx --paid 0x3800000000000011

$(TARGETSTUB): $(INTDIR) $(STUBOBJS)
	$(CC) $(INTDIR)/*.o.stub -o $(TARGETSTUB) -target x86_64-pc-linux-gnu -shared -fuse-ld=lld -ffreestanding -nostdlib -fno-builtin -L$(TOOLCHAIN)/lib $(LIBS)

$(INTDIR)/%.o: $(PROJDIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

$(INTDIR)/%.o: $(PROJDIR)/%.cpp
	$(CCX) $(CXXFLAGS) -o $@ $<

$(INTDIR)/%.o.stub: $(PROJDIR)/%.c
	$(CC) -target x86_64-pc-linux-gnu -ffreestanding -nostdlib -fno-builtin -fPIC $(O_FLAG) -s -c -o $@ $<

$(INTDIR)/%.o.stub: $(PROJDIR)/%.cpp
	$(CCX) -target x86_64-pc-linux-gnu -ffreestanding -nostdlib -fno-builtin -fPIC $(O_FLAG) -s -c -o $@ $<

.PHONY: clean
.DEFAULT_GOAL := all

all: $(TARGET)

clean:
	rm -rf $(BUILD_FOLDER) $(INTDIR) $(OBJS)
