ui_print "*************************************************"
ui_print "       Zygisk ImGui Universal Touch Fix          "
ui_print "*************************************************"

if [ "$BOOTMODE" != true ]; then
    abort "Error: Please install this module via Magisk App!"
fi

if [ -z "$ARCH" ] || [ -z "$API" ]; then
    abort "Error: Unable to detect architecture or API level."
fi

if [ "$API" -lt 26 ]; then
    abort "Error: Minimum Android 8.0 (API 26) required."
fi

ui_print "- Extracting module files..."
unzip -o "$ZIPFILE" 'zygisk/*' -d "$MODPATH" >&2

if [ -f "$MODPATH/zygisk/arm64-v8a/libzygisk.so" ]; then
    ui_print "- Successfully installed arm64-v8a library."
elif [ -f "$MODPATH/zygisk/armeabi-v7a/libzygisk.so" ]; then
    ui_print "- Successfully installed armeabi-v7a library."
else
    ui_print "! Warning: Native library not found in expected paths."
fi

set_perm_recursive "$MODPATH" 0 0 0755 0644
ui_print "- Installation completed successfully!"
