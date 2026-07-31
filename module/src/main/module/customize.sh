SKIPUNZIP=0

ui_print "- Installing Zygisk Universal ImGui Module..."

if [ "$ARCH" = "arm" ] || [ "$ARCH" = "arm64" ] || [ "$ARCH" = "x86" ] || [ "$ARCH" = "x64" ]; then
    ui_print "- Target Architecture: $ARCH"
else
    abort "! Unsupported Architecture: $ARCH"
fi
