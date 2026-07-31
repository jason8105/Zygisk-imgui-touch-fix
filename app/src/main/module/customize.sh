SKIPUNZIP=0

ui_print "- Installing Universal Zygisk ImGui Menu"
ui_print "- Architecture: $ARCH"

mkdir -p "$MODPATH/zygisk"

# Extract binaries
unzip -o "$ZIPFILE" "zygisk/*" -d "$MODPATH" 2>/dev/null || true

# Support both naming structures: zygisk/<abi>/libzygisk.so and zygisk/<abi>.so
mkdir -p "$MODPATH/zygisk/arm64-v8a" "$MODPATH/zygisk/armeabi-v7a"

if [ -f "$MODPATH/zygisk/arm64-v8a.so" ]; then
    cp -f "$MODPATH/zygisk/arm64-v8a.so" "$MODPATH/zygisk/arm64-v8a/libzygisk.so"
fi
if [ -f "$MODPATH/zygisk/arm64-v8a/libzygisk.so" ]; then
    cp -f "$MODPATH/zygisk/arm64-v8a/libzygisk.so" "$MODPATH/zygisk/arm64-v8a.so"
fi

if [ -f "$MODPATH/zygisk/armeabi-v7a.so" ]; then
    cp -f "$MODPATH/zygisk/armeabi-v7a.so" "$MODPATH/zygisk/armeabi-v7a/libzygisk.so"
fi
if [ -f "$MODPATH/zygisk/armeabi-v7a/libzygisk.so" ]; then
    cp -f "$MODPATH/zygisk/armeabi-v7a/libzygisk.so" "$MODPATH/zygisk/armeabi-v7a.so"
fi

ui_print "- Universal Zygisk ImGui Menu successfully installed!"
