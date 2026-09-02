#!/bin/sh

CC="gcc"
CFLAGS="-O2 -Wall -Wextra"
TARGET="BOX"
SRC="rescuebox.c"

echo "Cleaning old binary..."
rm -f "$TARGET"

echo "Compiling BARECRATE..."

if $CC $CFLAGS "$SRC" -o "$TARGET"; then
    echo "Build successful: $TARGET"
else
    echo "Build failed."
    exit 1
fi

echo "Installing BusyBox applets..."

if ./box/busybox --install -s ./bin; then
    echo "BusyBox applets installed."
else
    echo "BusyBox installation failed."
    exit 1
fi

echo "Build complete."
