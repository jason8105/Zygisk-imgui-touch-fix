SKIPUNZIP=0
ui_print "****************************************"
ui_print "    Zygisk ImGui Universal Touch Fix    "
ui_print "****************************************"

if [ "$ARCH" != "arm64" ] && [ "$ARCH" != "arm" ] && [ "$ARCH" != "x64" ] && [ "$ARCH" != "x86" ]; then
  abort "! Unsupported platform: $ARCH"
fi

ui_print "- Extracting module files..."
unzip -o "$ZIPFILE" 'zygisk/*' -d $MODPATH >&2

if [ -f "$MODPATH/zygisk/arm64-v8a/libzygisk.so" ] || [ -f "$MODPATH/zygisk/armeabi-v7a/libzygisk.so" ]; then
  ui_print "- Native binaries installed successfully!"
else
  abort "! Error: libzygisk.so not found in module package!"
fi

set_perm_recursive $MODPATH 0 0 0755 0644
