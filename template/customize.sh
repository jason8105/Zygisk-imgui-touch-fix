#!/sbin/sh
SKIPUNZIP=0

ui_print "***************************************************"
ui_print "* Universal Zygisk ImGui Menu Installer          *"
ui_print "***************************************************"

if [ "$BOOTMODE" != true ]; then
  ui_print "! Installation supported via Magisk app"
fi

ui_print "- Installing module files..."
