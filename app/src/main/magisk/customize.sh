ui_print "****************************************"
ui_print "      Zygisk ImGui Universal Menu       "
ui_print "****************************************"

ABI=""
if [ "$ARCH" = "arm64" ]; then
  ABI="arm64-v8a"
elif [ "$ARCH" = "arm" ]; then
  ABI="armeabi-v7a"
elif [ "$ARCH" = "x86" ]; then
  ABI="x86"
elif [ "$ARCH" = "x64" ]; then
  ABI="x86_64"
else
  ABI="arm64-v8a"
fi

ui_print "- Installing for ABI: $ABI"
ui_print "- Extracting Zygisk libraries..."

unzip -o "$ZIPFILE" "zygisk/$ABI/*" -d "$MODPATH/" >&2

if [ -f "$MODPATH/zygisk/$ABI/libzygisk.so" ]; then
  mv "$MODPATH/zygisk/$ABI/libzygisk.so" "$MODPATH/zygisk/libzygisk.so"
  rm -rf "$MODPATH/zygisk/arm64-v8a" "$MODPATH/zygisk/armeabi-v7a" "$MODPATH/zygisk/x86" "$MODPATH/zygisk/x86_64"
  ui_print "- Zygisk library installed successfully!"
else
  ui_print "! Warning: libzygisk.so not found for $ABI!"
end

set_perm_recursive "$MODPATH" 0 0 0755 0644
