SKIPUNZIP=0

ui_print "- Installing Universal Zygisk ImGui Menu Module..."
ui_print "- Device Architecture: $ARCH"

mkdir -p "$MODPATH/zygisk"

# Ensure both zygisk/<abi>/libzygisk.so and zygisk/<abi>.so exist for full Magisk v24-26 compatibility
if [ "$ARCH" = "arm64" ]; then
    if [ -f "$MODPATH/zygisk/arm64-v8a/libzygisk.so" ] && [ ! -f "$MODPATH/zygisk/arm64-v8a.so" ]; then
        cp "$MODPATH/zygisk/arm64-v8a/libzygisk.so" "$MODPATH/zygisk/arm64-v8a.so"
    fi
elif [ "$ARCH" = "arm" ]; then
    if [ -f "$MODPATH/zygisk/armeabi-v7a/libzygisk.so" ] && [ ! -f "$MODPATH/zygisk/armeabi-v7a.so" ]; then
        cp "$MODPATH/zygisk/armeabi-v7a/libzygisk.so" "$MODPATH/zygisk/armeabi-v7a.so"
    fi
elif [ "$ARCH" = "x64" ]; then
    if [ -f "$MODPATH/zygisk/x86_64/libzygisk.so" ] && [ ! -f "$MODPATH/zygisk/x86_64.so" ]; then
        cp "$MODPATH/zygisk/x86_64/libzygisk.so" "$MODPATH/zygisk/x86_64.so"
    fi
elif [ "$ARCH" = "x86" ]; then
    if [ -f "$MODPATH/zygisk/x86/libzygisk.so" ] && [ ! -f "$MODPATH/zygisk/x86.so" ]; then
        cp "$MODPATH/zygisk/x86/libzygisk.so" "$MODPATH/zygisk/x86.so"
    fi
fi

ui_print "- Module installation successful."
