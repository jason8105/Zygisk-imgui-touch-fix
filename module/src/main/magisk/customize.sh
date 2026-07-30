ui_print "*************************************************"
ui_print "     Zygisk ImGui Universal Touch-Fixed Menu     "
ui_print "*************************************************"

if [ "$BOOTMODE" != true ]; then
    abort "Error: Please install this module from within Magisk Manager / KernelSU / APatch while booted!"
fi

if [ -z "$ARCH" ] || [ -z "$API" ]; then
    ui_print "Extracting system properties..."
    ARCH=$(grep_prop ro.product.cpu.abi)
fi

ui_print "- Installing for architecture: $ARCH"
ui_print "- Target API: $API"

if [ "$API" -lt 24 ]; then
    abort "Error: Minimum Android 7.0 (API 24) required!"
fi

ui_print "- Extracting module files..."
unzip -o "$ZIPFILE" 'zygisk/*' -d "$MODPATH" >&2
unzip -o "$ZIPFILE" 'module.prop' -d "$MODPATH" >&2
unzip -o "$ZIPFILE" 'customize.sh' -d "$MODPATH" >&2

set_perm_recursive "$MODPATH" 0 0 0755 0644
if [ -d "$MODPATH/zygisk" ]; then
    set_perm_recursive "$MODPATH/zygisk" 0 0 0755 0755
fi

ui_print "*************************************************"
ui_print " Installation Complete! Reboot Required.         "
ui_print "*************************************************"
