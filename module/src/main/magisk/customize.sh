SKIPUNZIP=0
ui_print "****************************************"
ui_print "    Zygisk Unity ImGui Touch Fix        "
ui_print "****************************************"

if [ "$ARCH" != "arm64" ]; then
  abort "Error: Unsupported architecture ($ARCH). Only arm64-v8a is supported."
fi

ui_print "- Installing Zygisk library..."
mkdir -p "$MODPATH/zygisk"
if [ -f "$MODPATH/zygisk/arm64-v8a/libzygisk_touch_fix.so" ]; then
  mv "$MODPATH/zygisk/arm64-v8a/libzygisk_touch_fix.so" "$MODPATH/zygisk/arm64.so"
  rm -rf "$MODPATH/zygisk/arm64-v8a"
fi

ui_print "- Installation completed successfully!"
