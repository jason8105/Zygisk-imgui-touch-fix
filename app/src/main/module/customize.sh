#!/system/bin/sh
SKIPUNZIP=0

ui_print "***********************************************"
ui_print "*   Universal ImGui Menu Zygisk Module        *"
ui_print "*   Magisk v24 - v26 Compatible               *"
ui_print "***********************************************"

if [ ! -d "$MODPATH/zygisk" ]; then
    ui_print "! Failed: Zygisk library folder missing in module zip"
    abort
fi

ui_print "- Architecture detected: $ARCH"
ui_print "- Installation successful!"
