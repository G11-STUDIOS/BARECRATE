#!/bin/sh

CC="gcc"
CFLAGS="-O2 -Wall -Wextra"
TARGET="BOX"
SRC="rescuebox.c"

echo "Cleaning old binary..."
rm -f "$TARGET"

echo "Compiling BARECRATE..."
$CC $CFLAGS "$SRC" -o "$TARGET"

if [ $? -eq 0 ]; then
    echo "Build successful: $TARGET"
else
    echo "Build failed."
    exit 1
fi
./box/busybox --install -s ./bin
