SKIPUNZIP=0

if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
  ui_print "! Magisk version $MAGISK_VER ($MAGISK_VER_CODE) is not supported. Minimum required is v24.0 (24000)."
  abort "! Please update Magisk."
fi

ui_print "- Installing Zygisk Universal ImGui Menu..."
ui_print "- Universal touch fix applied across Unity, Unreal Engine, and Native C++ games."
