#!/sbin/sh
SKIPUNZIP=0

if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
  ui_print "! Magisk version v24.0+ is required."
  abort "! Magisk $MAGISK_VER ($MAGISK_VER_CODE) detected. Please update Magisk."
fi

ui_print "- Installing Universal Zygisk ImGui Menu..."
ui_print "- Magisk version: $MAGISK_VER ($MAGISK_VER_CODE)"
ui_print "- Injecting Zygisk native libraries..."

# Verification of module library layout
if [ -f "$MODPATH/zygisk/arm64-v8a/libzygisk.so" ] || [ -f "$MODPATH/zygisk/armeabi-v7a/libzygisk.so" ]; then
  ui_print "- Zygisk shared libraries extracted successfully."
else
  ui_print "! Error: Missing libzygisk.so in module package."
  abort "! Installation failed."
fi

set_permissions() {
  set_perm_recursive $MODPATH 0 0 0755 0644
}
