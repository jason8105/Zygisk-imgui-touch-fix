#!/system/bin/sh
SKIPUNZIP=0

ui_print "- Installing Universal ImGui Menu Zygisk Module"
if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
  ui_print "! Magisk version v24.0+ is required"
  abort "! Unsupported Magisk version"
fi

ui_print "- Zygisk module installed successfully!"
