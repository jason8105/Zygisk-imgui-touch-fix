SKIPUNZIP=0

if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
  ui_print "! Magisk version 24.0+ required"
  abort "! Unsupported Magisk version"
fi

ui_print "- Installing Zygisk ImGui Universal Module..."
