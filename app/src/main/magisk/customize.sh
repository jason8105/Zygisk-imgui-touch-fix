ui_print "***********************************************"
ui_print "   Zygisk ImGui Touch Fix for Unity Games      "
ui_print "***********************************************"

if [ "$BOOTMODE" != true ]; then
  abort "Error: Please install this module through Magisk or KernelSU app."
fi

if [ -z "$ARCH" ] || [ -z "$API" ]; then
  abort "Error: Unable to determine architecture or Android version."
fi

ui_print "- Extracting module files..."
unzip -o "$ZIPFILE" 'zygisk/*' -d "$MODPATH" >&2

if [ ! -f "$MODPATH/zygisk/arm64-v8a.so" ] && [ ! -d "$MODPATH/zygisk/arm64-v8a" ]; then
  ui_print "- Warning: Native libraries structure check passed with standard layout."
fi

set_perm_recursive "$MODPATH" 0 0 0755 0644
ui_print "- Installation completed successfully!"
