#!/sbin/sh

if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
  ui_print "! Magisk v24.0+ is required for Zygisk"
  abort "! Unsupported Magisk version"
fi

ui_print "- Installing Universal Zygisk ImGui Menu..."
ui_print "- Target Architecture: $ARCH"

mkdir -p "$MODPATH/zygisk"

ui_print "- Universal Zygisk ImGui Menu installed successfully!"
