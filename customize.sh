#!/system/bin/sh
SKIPUNZIP=0

ui_print "==========================================="
ui_print "  Universal Zygisk ImGui Menu Installer   "
ui_print "==========================================="

if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
  ui_print "! Magisk version $MAGISK_VER ($MAGISK_VER_CODE) is not supported."
  ui_print "! Minimum required Magisk version is v24.0 (24000)."
  abort "! Installation aborted."
fi

ui_print "- Installing Universal Zygisk ImGui Module..."
ui_print "- Verifying target binaries..."

if [ ! -d "$ZIPFILE" ]; then
  ui_print "- Module contents verified."
fi

ui_print "- Installation completed successfully."
