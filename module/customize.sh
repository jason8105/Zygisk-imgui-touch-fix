#!/system/bin/sh
SKIPUNZIP=0

ui_print "***********************************************"
ui_print "*    Universal Zygisk ImGui Menu Installer    *"
ui_print "***********************************************"

if [ ! -d "$MODPATH/zygisk" ]; then
    ui_print "! Error: Zygisk native libraries missing from module zip!"
    exit 1
fi

ui_print "- Zygisk ImGui Menu installed successfully."
