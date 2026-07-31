SKIPUNZIP=0

if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
  ui_print "! Magisk v24.0+ is required for Zygisk"
  abort "! Unsupported Magisk version"
fi

ui_print "- Installing Universal ImGui Menu (Zygisk)..."
ui_print "- Target Magisk Version: $MAGISK_VER ($MAGISK_VER_CODE)"
ui_print "- Universal Touch & ImGui Engine Ready."
