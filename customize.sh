SKIPUNZIP=1

ui_print "*************************************************"
ui_print "*      Zygisk ImGui Universal Touch Fix        *"
ui_print "*         Magisk 24-26 Compatible               *"
ui_print "*************************************************"

if [ "$BOOTMODE" ] && [ "$API" ]; then
  ui_print "- Device API Level: $API"
else
  ui_print "- Error: Unable to determine boot mode or API level."
  abort "[-] Aborting installation."
fi

if [ "$API" -lt 26 ]; then
  ui_print "[-] Error: Android 8.0 (API 26) or higher is required."
  abort "[-] Aborting installation."
fi

ui_print "- Extracting module files..."
unzip -o "$ZIPFILE" 'system/*' -d "$MODPATH" >&2
unzip -o "$ZIPFILE" 'zygisk/*' -d "$MODPATH" >&2
unzip -o "$ZIPFILE" 'module.prop' -d "$MODPATH" >&2

set_perm_recursive "$MODPATH" 0 0 0755 0644

if [ -d "$MODPATH/zygisk" ]; then
  ui_print "- Zygisk directory successfully populated."
else
  ui_print "[-] Warning: Zygisk libs missing from zip package!"
fi

ui_print "- Installation completed successfully!"
