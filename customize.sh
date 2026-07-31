#!/system/bin/sh
SKIPUNZIP=0

ui_print "***************************************************"
ui_print " Installing Zygisk Universal Touch ImGui Menu Module"
ui_print " Target Magisk: v24.0 - v26.x                     "
ui_print "***************************************************"

if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
  ui_print "! Magisk version 24.0+ is required!"
  abort "! Please update Magisk to v24+."
fi

ui_print "- Unpacking module files..."
