#!/sbin/sh
SKIPUNZIP=0

ui_print "****************************************************"
ui_print "*     Zygisk Universal ImGui Menu Installer        *"
ui_print "****************************************************"

if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
  ui_print "! Magisk version $MAGISK_VER ($MAGISK_VER_CODE) is not supported."
  ui_print "! Minimum required version is Magisk v24.0 (24000)."
  abort
fi

if [ "$ZYGISK_ENABLED" != "1" ]; then
  ui_print "! Zygisk is not enabled in Magisk settings."
  ui_print "! Please enable Zygisk and restart your device."
  abort
fi

ui_print "- Installing Zygisk module binaries..."
set_perm_recursive $MODPATH 0 0 0755 0644
ui_print "- Installation complete. Please reboot."
