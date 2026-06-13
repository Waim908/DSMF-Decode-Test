# msvc build toolchain on linux
# https://github.com/mstorsjo/msvc-wine
PATH="/path/to/msvc/bin/x64/:$PATH" CC="cl" make -j$(nproc)
