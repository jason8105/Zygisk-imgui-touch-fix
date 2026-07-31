#!/sbin/sh
SKIPUNZIP=0

ui_print "--------------------------------------"
ui_print " Installing Universal Zygisk ImGui    "
ui_print " Target Magisk: v24.0 - v26.x         "
ui_print "--------------------------------------"

# Verify Zygisk is enabled
if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
  ui_print "! Magisk version $MAGISK_VER ($MAGISK_VER_CODE) is not supported."
  ui_print "! Please upgrade to Magisk v24.0 or higher."
  abort
fi

ui_print "- Zygisk module files extracted successfully."
