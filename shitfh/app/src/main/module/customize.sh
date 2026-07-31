#!/system/bin/sh
SKIPUNZIP=0

ui_print "***********************************************"
ui_print "*   Universal ImGui Menu Zygisk Module Installer  *"
ui_print "***********************************************"

if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
    ui_print "! Magisk version v24.0+ is required"
    abort "! Installed Magisk version code: $MAGISK_VER_CODE"
fi

if [ "$ZYGISK_ENABLE" != "1" ]; then
    ui_print "! Zygisk is disabled in Magisk settings"
    ui_print "! Please enable Zygisk and reboot before installing"
    abort "! Zygisk is NOT enabled"
fi

ui_print "- Extracting Zygisk module libraries..."
