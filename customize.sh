#!/system/bin/sh
SKIPUNZIP=0

ui_print "- Installing Universal ImGui Zygisk Module..."

ZYGISK_DIR="$MODPATH/zygisk"
mkdir -p "$ZYGISK_DIR"

# Extract precompiled zygisk binaries if present in temp dir
if [ -d "$TMPDIR/zygisk" ]; then
    cp -rf "$TMPDIR/zygisk/"* "$ZYGISK_DIR/"
fi

ui_print "- Module installation completed."
