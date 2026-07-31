#!/system/bin/sh
SKIPUNZIP=0

if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
  ui_print "Error: Magisk v24.0 or higher is required for Zygisk!"
  exit 1
fi

if [ "$ZYGISK_ENABLED" != "1" ]; then
  ui_print "Warning: Zygisk is not enabled in Magisk settings!"
fi

ui_print "- Installing Zygisk Universal ImGui Menu..."
