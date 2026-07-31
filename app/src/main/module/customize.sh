SKIPUNZIP=0

ui_print "- Installing Universal Zygisk ImGui Module..."

if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
  ui_print "! Magisk version 24.0+ is required"
  abort "! Zygisk is required for this module"
fi

if [ "$ZYGISK_ENABLED" != "1" ]; then
  ui_print "! Zygisk is not enabled in Magisk settings"
  ui_print "! Please enable Zygisk and reboot before installing"
  abort "! Zygisk must be enabled"
fi

ui_print "- Target Architecture: $ARCH"
ui_print "- Module installation successfully initialized!"
