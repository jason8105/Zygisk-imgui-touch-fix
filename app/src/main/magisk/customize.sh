ui_print "*************************************************"
ui_print "   Zygisk ImGui Universal Touch-Fixed Module     "
ui_print "*************************************************"

# Check architecture
ABI=""
case "%ARCH%" in
  arm|armeabi-v7a) ABI="armeabi-v7a" ;;
  arm64|aarch64) ABI="arm64-v8a" ;;
  x86) ABI="x86" ;;
  x64|x86_64) ABI="x86_64" ;;
  *) ABI="arm64-v8a" ;;
esac

ui_print "- Target Arch: $ABI"

if [ ! -f "$MODPATH/zygisk/$ABI/libzygisk.so" ]; then
  ui_print "! Warning: Native library for $ABI not found directly, checking fallback paths..."
  # Try to find libzygisk.so anywhere inside zygisk/
  found_lib=$(find "$MODPATH/zygisk" -name "libzygisk.so" | head -n 1)
  if [ -n "$found_lib" ]; then
    ui_print "- Found native library at $found_lib"
  else
    ui_print "! Error: libzygisk.so could not be located in module package!"
  fi
fi

ui_print "- Installation complete! Please reboot."
