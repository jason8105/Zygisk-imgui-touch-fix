SKIPUNZIP=0

ui_print "*************************************************"
ui_print "      Zygisk ImGui Universal Touch Fix           "
ui_print "      Compatible with Magisk v24 - v26           "
ui_print "*************************************************"

if [ "$BOOTMODE" ! = true ]; then
  abort "[-] Please install this module through Magisk App!"
fi

if [ -z "$ARCH" ] || [ -z "$API" ]; then
  abort "[-] Critical environment variables missing."
fi

ui_print "[*] Target Architecture: $ARCH"
ui_print "[*] Android API Level: $API"

# Extract native libraries based on architecture
ui_print "[*] Installing Zygisk binary..."
mkdir -p "$MODPATH/zygisk"

if [ "$ARCH" = "arm64" ]; then
  if [ -f "$MODPATH/system/lib64/libzygisk_imgui.so" ]; then
    cp -af "$MODPATH/system/lib64/libzygisk_imgui.so" "$MODPATH/zygisk/arm64-v8a.so"
  elif [ -f "$MODPATH/lib/arm64-v8a/libzygisk_imgui.so" ]; then
    cp -af "$MODPATH/lib/arm64-v8a/libzygisk_imgui.so" "$MODPATH/zygisk/arm64-v8a.so"
  fi
elif [ "$ARCH" = "arm" ]; then
  if [ -f "$MODPATH/system/lib/libzygisk_imgui.so" ]; then
    cp -af "$MODPATH/system/lib/libzygisk_imgui.so" "$MODPATH/zygisk/armeabi-v7a.so"
  elif [ -f "$MODPATH/lib/armeabi-v7a/libzygisk_imgui.so" ]; then
    cp -af "$MODPATH/lib/armeabi-v7a/libzygisk_imgui.so" "$MODPATH/zygisk/armeabi-v7a.so"
  fi
else
  abort "[-] Unsupported architecture: $ARCH"
fi

# Clean up temporary directories if needed
rm -rf "$MODPATH/system" "$MODPATH/lib"

set_perm_recursive "$MODPATH" 0 0 0755 0644
ui_print "[+] Installation completed successfully!"
