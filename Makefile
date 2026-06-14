# Toolchain selection: specify CC on command line or auto-detect
# Examples:
#   make CC=x86_64-w64-mingw32-gcc    (Linux cross-compile)
#   make CC=gcc                       (Windows MinGW)
#   make CC=cl.exe                    (Windows MSVC)

UNAME_S := $(shell uname -s 2>/dev/null || echo Windows)

# Check if CC was explicitly specified by user
CC_ORIGIN := $(origin CC)
ifeq ($(CC_ORIGIN),command line)
    USER_CC = 1
else ifeq ($(CC_ORIGIN),environment)
    USER_CC = 1
else ifeq ($(CC_ORIGIN),environment override)
    USER_CC = 1
else
    USER_CC = 0
endif

ifeq ($(USER_CC),1)
    # User specified CC, detect toolchain from it
    ifneq ($(findstring cl,$(CC)),)
        TOOLCHAIN = msvc
    else
        TOOLCHAIN = mingw
    endif
else
    # Auto-detect
    ifeq ($(UNAME_S),Linux)
        CC = x86_64-w64-mingw32-gcc
        TOOLCHAIN = mingw
    else ifneq ($(shell where cl.exe 2>nul),)
        CC = cl.exe
        TOOLCHAIN = msvc
    else
        CC = gcc
        TOOLCHAIN = mingw
    endif
endif

# Set RC and shell commands based on toolchain
ifeq ($(TOOLCHAIN),msvc)
    RC ?= rc.exe
    RM = del
    MKDIR = mkdir
    RMDIR = del /q obj\*.obj 2>nul || exit 0
else
    ifeq ($(UNAME_S),Linux)
        RC ?= x86_64-w64-mingw32-windres
        RM = rm -f
        MKDIR = mkdir -p
        RMDIR = rm -rf obj
    else
        RC ?= windres
        RM = del
        MKDIR = mkdir
        RMDIR = del /q obj\*.o 2>nul || exit 0
    endif
endif

# Set executable name based on toolchain
ifeq ($(TOOLCHAIN),msvc)
    EXE_NAME=DSMF-Decode-Test-msvc.exe
else
    EXE_NAME=DSMF-Decode-Test-mingw.exe
endif

# Set flags and file extensions based on toolchain
ifeq ($(TOOLCHAIN),msvc)
    INCLUDE_DIR=/I./include
    CFLAGS=/O2 /std:c11 /utf-8 /DUNICODE /D_UNICODE /DCOBJMACROS /DWINVER=0x0601 /D_WIN32_WINNT=0x0601 /W4 /wd4100 /wd4189
    LDFLAGS=/SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup \
        strmiids.lib mfplat.lib mf.lib mfreadwrite.lib mfuuid.lib evr.lib \
        d3d9.lib dxva2.lib d3d11.lib d3d12.lib dxgi.lib dxguid.lib \
        comctl32.lib gdi32.lib user32.lib ole32.lib uuid.lib comdlg32.lib shlwapi.lib shell32.lib winmm.lib
    OBJ_EXT = .obj
    RES_EXT = .res
    RES_FLAGS = /nologo /I./include /I./res /fo
else
    INCLUDE_DIR=-I./include
    CFLAGS=-O3 -s -std=c99 -DUNICODE -D_UNICODE -DCOBJMACROS -DWINVER=0x0601 -D_WIN32_WINNT=0x0601 -Wall -Wno-unused-variable
    LDFLAGS=-s -mwindows -municode \
        -lstrmiids -lmfplat -lmf -lmfreadwrite -lmfuuid -levr \
        -ld3d9 -ldxva2 -ld3d11 -ld3d12 -ldxgi -ldxguid \
        -lcomctl32 -lgdi32 -luser32 -lole32 -loleaut32 -luuid -lcomdlg32 -lshlwapi -lshell32 -lwinmm
    OBJ_EXT = .o
    RES_EXT = .o
    RES_FLAGS = --codepage=65001 -I./include -I./res
endif

OBJS=obj/main$(OBJ_EXT) obj/directshow_decoder$(OBJ_EXT) obj/mf_decoder$(OBJ_EXT) obj/dxva2_helper$(OBJ_EXT) obj/d3d11_video_helper$(OBJ_EXT) obj/d3d12_video_helper$(OBJ_EXT) obj/app_config$(OBJ_EXT) obj/lang$(OBJ_EXT)
RES=obj/resource$(RES_EXT)

# Detect build flag changes and force recompilation
CURFLAGS := $(CFLAGS) $(INCLUDE_DIR)
SAVEDFLAGS := $(shell cat .cflags 2>/dev/null)

ifneq ($(CURFLAGS),$(SAVEDFLAGS))
  $(shell rm -rf obj)
  $(shell echo '$(CURFLAGS)' > .cflags)
  $(info Build flags changed, forcing clean build...)
endif

all: ${EXE_NAME}

${EXE_NAME}: ${OBJS} ${RES}
ifeq ($(TOOLCHAIN),msvc)
	link.exe /OUT:${EXE_NAME} ${OBJS} ${RES} ${LDFLAGS}
else
	${CC} -o ${EXE_NAME} ${OBJS} ${RES} ${LDFLAGS}
endif

obj/%$(OBJ_EXT): src/%.c obj
ifeq ($(TOOLCHAIN),msvc)
	${CC} ${CFLAGS} ${INCLUDE_DIR} /c $< /Fo$@
else
	${CC} ${CFLAGS} ${INCLUDE_DIR} -c $< -o $@
endif

obj/resource$(RES_EXT): res/resource.rc res/Application.manifest res/main.ico obj
ifeq ($(TOOLCHAIN),msvc)
	${RC} ${RES_FLAGS}$@ $<
else
	${RC} ${RES_FLAGS} $< $@
endif

clean:
	${RMDIR}
	${RM} ${EXE_NAME}
	${RM} .cflags

obj:
	${MKDIR} obj
