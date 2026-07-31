#!/system/bin/sh
SKIPUNZIP=0

ui_print "****************************************"
ui_print "* Universal Zygisk ImGui Menu Installer *"
ui_print "****************************************"

if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
  ui_print "! Magisk v24.0 or higher is required!"
  abort
fi

if [ -f "$ZIPFILE" ]; then
  ui_print "- Extracting module files..."
fi

ui_print "- Installation complete."
