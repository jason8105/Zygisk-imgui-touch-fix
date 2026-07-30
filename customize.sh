SKIPUNZIP=0
ui_print "************************************************"
ui_print "       Zygisk ImGui Universal Touch-Fixed       "
ui_print "************************************************"
if [ "$BOOTMODE" ] && [ "$RAPID" ]; then
  ui_print "- Installing in Magisk environment"
else
  ui_print "- Magisk environment detected"
fi
