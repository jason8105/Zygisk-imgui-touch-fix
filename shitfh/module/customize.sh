SKIPUNZIP=0

ui_print "- Installing Universal ImGui Menu (Zygisk)..."
ui_print "- Magisk Version Target: v24 - v26"
ui_print "- Current Device Architecture: $ARCH"

if [ -d "$MODPATH/zygisk" ]; then
    ui_print "- Zygisk binaries verified successfully."
else
    ui_print "! Warning: zygisk directory missing in installation package."
fi
