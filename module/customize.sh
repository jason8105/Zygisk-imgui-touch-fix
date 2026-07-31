SKIPUNZIP=0

ui_print "--------------------------------------------------"
ui_print " Installing Zygisk Universal Touch ImGui Menu"
ui_print "--------------------------------------------------"

if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
  ui_print "! Magisk version 24.0+ (v24-v26) is required for Zygisk"
  abort "! Unsupported Magisk version: $MAGISK_VER_CODE"
fi

if [ -f "$ZIPFILE" ]; then
  ui_print "- Extracting module native libraries..."
fi

ui_print "- Installation completed successfully."
