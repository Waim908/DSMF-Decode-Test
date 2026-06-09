OBJS=obj/main.o obj/directshow_decoder.o obj/mf_decoder.o obj/dxva2_helper.o obj/d3d11_video_helper.o obj/d3d12_video_helper.o
RES=obj/resource.o
INCLUDE_DIR=-I./include
EXE_NAME=DSMF-Decode-Test.exe

CFLAGS=-O2 -s -std=c99 -DUNICODE -D_UNICODE -DCOBJMACROS -DWINVER=0x0601 -D_WIN32_WINNT=0x0601 -Wall -Wno-unused-variable
LDFLAGS=-s -mwindows -municode \
    -lstrmiids -lmfplat -lmf -lmfreadwrite -lmfuuid -levr \
    -ld3d9 -ldxva2 -ld3d11 -ld3d12 -ldxgi \
    -lcomctl32 -lgdi32 -luser32 -lole32 -luuid -lcomdlg32 -lshlwapi -lwinmm

# Detect OS and set compiler/toolchain accordingly
UNAME_S := $(shell uname -s 2>/dev/null || echo Windows)

ifeq ($(UNAME_S),Linux)
    # Linux cross-compilation with mingw-w64
    CC = x86_64-w64-mingw32-gcc
    RC = x86_64-w64-mingw32-windres
    RM = rm -f
    MKDIR = mkdir -p
    RMDIR = rm -rf obj
else
    # Windows native compilation
    CC = gcc
    RC = windres
    RM = del
    MKDIR = mkdir
    RMDIR = del /q obj\*.o 2>nul || exit 0
endif

# Detect build flag changes and force recompilation
CURFLAGS := $(CFLAGS) $(INCLUDE_DIR)
SAVEDFLAGS := $(shell cat .cflags 2>/dev/null)

ifneq ($(CURFLAGS),$(SAVEDFLAGS))
  $(shell rm -rf obj)
  $(shell echo '$(CURFLAGS)' > .cflags)
  $(info Build flags changed, forcing clean rebuild...)
endif

all: ${EXE_NAME}

${EXE_NAME}: ${OBJS} ${RES}
	${CC} -o ${EXE_NAME} ${OBJS} ${RES} ${LDFLAGS}

clean:
	${RMDIR}
	${RM} ${EXE_NAME}
	${RM} .cflags

obj:
	${MKDIR} obj

obj/%.o: src/%.c obj
	${CC} ${CFLAGS} ${INCLUDE_DIR} -c $< -o $@

obj/resource.o: res/resource.rc res/Application.manifest obj
	${RC} -I./include -I./res $< $@
