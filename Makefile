OBJS=obj/main.o obj/directshow_decoder.o obj/mf_decoder.o obj/dxva2_helper.o obj/d3d11_video_helper.o obj/d3d12_video_helper.o
RES=obj/resource.o
EXE_NAME=DSMF-Decode-Test.exe

# Detect OS and toolchain
UNAME_S := $(shell uname -s 2>/dev/null || echo Windows)

ifeq ($(UNAME_S),Linux)
    # Linux cross-compilation with mingw-w64
    CC = x86_64-w64-mingw32-gcc
    RC = x86_64-w64-mingw32-windres
    RM = rm -f
    MKDIR = mkdir -p
    RMDIR = rm -rf obj
    TOOLCHAIN = mingw
else
    # Windows: detect MSVC or MinGW
    ifneq ($(shell where cl.exe 2>nul),)
        # MSVC detected
        CC = cl.exe
        RC = rc.exe
        RM = del
        MKDIR = mkdir
        RMDIR = del /q obj\*.obj 2>nul || exit 0
        TOOLCHAIN = msvc
    else
        # MinGW fallback
        CC = gcc
        RC = windres
        RM = del
        MKDIR = mkdir
        RMDIR = del /q obj\*.o 2>nul || exit 0
        TOOLCHAIN = mingw
    endif
endif

# Set flags based on toolchain
ifeq ($(TOOLCHAIN),msvc)
    INCLUDE_DIR=/I./include
    CFLAGS=/O2 /std:c11 /DUNICODE /D_UNICODE /DCOBJMACROS /DWINVER=0x0601 /D_WIN32_WINNT=0x0601 /W4 /wd4100 /wd4189
    LDFLAGS=/SUBSYSTEM:WINDOWS /ENTRY:wmainCRTStartup \
        strmiids.lib mfplat.lib mf.lib mfreadwrite.lib mfuuid.lib evr.lib \
        d3d9.lib dxva2.lib d3d11.lib d3d12.lib dxgi.lib \
        comctl32.lib gdi32.lib user32.lib ole32.lib uuid.lib comdlg32.lib shlwapi.lib winmm.lib
else
    INCLUDE_DIR=-I./include
    CFLAGS=-O2 -s -std=c99 -DUNICODE -D_UNICODE -DCOBJMACROS -DWINVER=0x0601 -D_WIN32_WINNT=0x0601 -Wall -Wno-unused-variable
    LDFLAGS=-s -mwindows -municode \
        -lstrmiids -lmfplat -lmf -lmfreadwrite -lmfuuid -levr \
        -ld3d9 -ldxva2 -ld3d11 -ld3d12 -ldxgi \
        -lcomctl32 -lgdi32 -luser32 -lole32 -luuid -lcomdlg32 -lshlwapi -lwinmm
endif

# Detect build flag changes and force recompilation
CURFLAGS := $(CFLAGS) $(INCLUDE_DIR)
SAVEDFLAGS := $(shell cat .cflags 2>/dev/null)

ifneq ($(CURFLAGS),$(SAVEDFLAGS))
  $(shell rm -rf obj)
  $(shell echo '$(CURFLAGS)' > .cflags)
  $(info Build flags changed, forcing clean build...)
endif

ifeq ($(TOOLCHAIN),msvc)
    # MSVC object files use .obj extension
    OBJS=obj/main.obj obj/directshow_decoder.obj obj/mf_decoder.obj obj/dxva2_helper.obj obj/d3d11_video_helper.obj obj/d3d12_video_helper.obj
    RES=obj/resource.res

    all: ${EXE_NAME}

    ${EXE_NAME}: ${OBJS} ${RES}
        link.exe /OUT:${EXE_NAME} ${OBJS} ${RES} ${LDFLAGS}

    obj/%.obj: src/%.c obj
        ${CC} ${CFLAGS} ${INCLUDE_DIR} /c $< /Fo$@

    obj/resource.res: res/resource.rc res/Application.manifest obj
        ${RC} /nologo /I./include /I./res /fo$@ $<
else
    # MinGW object files use .o extension
    RES=obj/resource.o

    all: ${EXE_NAME}

    ${EXE_NAME}: ${OBJS} ${RES}
        ${CC} -o ${EXE_NAME} ${OBJS} ${RES} ${LDFLAGS}

    obj/%.o: src/%.c obj
        ${CC} ${CFLAGS} ${INCLUDE_DIR} -c $< -o $@

    obj/resource.o: res/resource.rc res/Application.manifest obj
        ${RC} --codepage=65001 -I./include -I./res $< $@
endif

clean:
    ${RMDIR}
    ${RM} ${EXE_NAME}
    ${RM} .cflags

obj:
    ${MKDIR} obj
