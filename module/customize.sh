#!/system/bin/sh
SKIPUNZIP=0

ui_print "- Installing Zygisk Universal ImGui Menu"

if ! $ZYGISK_ENABLED; then
  ui_print "! Zygisk is not enabled in Magisk settings"
  ui_print "! Please enable Zygisk and reboot"
  abort "! Aborting installation"
fi

ui_print "- Zygisk framework detected"
ui_print "- Universal Touch-Fixed ImGui Module installed successfully"
