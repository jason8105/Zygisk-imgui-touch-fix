#!/system/bin/sh
SKIPUNZIP=0

if ! $BOOTMODE; then
  ui_print "- Installing from recovery is not supported"
  abort "! Please install inside Magisk app"
fi

ui_print "- Installing Zygisk Universal ImGui Menu..."
ui_print "- Target Magisk Version: $MAGISK_VER ($MAGISK_VER_CODE)"
ui_print "- Extracting zygisk shared libraries..."
