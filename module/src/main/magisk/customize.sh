SKIPUNZIP=0

ui_print "****************************************"
ui_print "      Zygisk ImGui Touch Fix            "
ui_print "****************************************"

# Check architecture
if [ "$ARCH" != "arm64" ] && [ "$ARCH" != "arm" ]; then
  abort "Unsupported architecture: $ARCH. Only ARM/ARM64 are supported."
fi

ui_print "- Extracting module files..."
unzip -o "$ZIPFILE" 'zygisk/*' -d "$MODPATH" >&2

if [ -f "$MODPATH/zygisk/arm64-v8a/libzygisk.so" ]; then
  mv "$MODPATH/zygisk/arm64-v8a"/* "$MODPATH/zygisk/" 2>/dev/null
  rmdir "$MODPATH/zygisk/arm64-v8a" 2>/dev/null
elif [ -f "$MODPATH/zygisk/armeabi-v7a/libzygisk.so" ]; then
  mv "$MODPATH/zygisk/armeabi-v7a"/* "$MODPATH/zygisk/" 2>/dev/null
  rmdir "$MODPATH/zygisk/armeabi-v7a" 2>/dev/null
fi

set_perm_recursive "$MODPATH" 0 0 0755 0644
