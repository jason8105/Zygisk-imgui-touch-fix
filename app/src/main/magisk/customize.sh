SKIPUNZIP=0
ui_print "****************************************"
ui_print "       Zygisk ImGui Universal Menu      "
ui_print "****************************************"
if [ "$BOOTMODE" ! = true ]; then
  abort "Error: Please install this module via Magisk Manager/KernelSU app!"
fi
