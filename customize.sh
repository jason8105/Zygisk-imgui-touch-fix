#!/system/bin/sh
SKIPUNZIP=0

ui_print "****************************************"
ui_print "* Universal Zygisk ImGui Menu Installer *"
ui_print "****************************************"

if [ "$BOOTMODE" != "true" ]; then
  ui_print "! Please install this module inside Magisk Manager / KernelSU"
fi

if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
  ui_print "! Magisk version lower than 24.0 (24000) is not supported!"
  abort "! Minimum Magisk version required: v24.0"
fi

ui_print "- Installing Universal Zygisk ImGui Menu..."
ui_print "- Device Architecture: $ARCH"
ui_print "- Installation successful!"
