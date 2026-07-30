SKIPUNZIP=0
ui_print "***********************************"
ui_print "   ZyCheats Magisk Module Installer"
ui_print "***********************************"
if [ "$ARCH" != "arm64" ]; then
  abort "Only arm64 architecture is supported!"
fi
ui_print "Installation complete!"
