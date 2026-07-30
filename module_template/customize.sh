SKIPUNZIP=0
ui_print "****************************************"
ui_print "    Zygisk ImGui Universal Touch Fix    "
ui_print "****************************************"

if [ "$ARCH" != "arm64" ] && [ "$ARCH" != "arm" ] && [ "$ARCH" != "x86" ] && [ "$ARCH" != "x64" ]; then
  abort "! Unsupported platform: $ARCH"
fi

ui_print "- Extracting module files..."
unzip -o "$ZIPFILE" 'zygisk/*' -d $MODPATH >&2
ui_print "- Installation complete!"
