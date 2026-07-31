#!/system/bin/sh
SKIPUNZIP=0

ui_print "****************************************"
ui_print "*   Universal Zygisk ImGui Menu Installer *"
ui_print "****************************************"

if [ "$ARCH" != "arm" ] && [ "$ARCH" != "arm64" ] && [ "$ARCH" != "x86" ] && [ "$ARCH" != "x64" ]; then
    ui_print "! Unsupported architecture: $ARCH"
    abort
fi

ui_print "- Installing Zygisk Module for $ARCH"
