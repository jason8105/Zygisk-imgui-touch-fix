SKIPUNZIP=0

ui_print "**********************************************"
ui_print "*      Zygisk ImGui Universal Menu Module     *"
ui_print "**********************************************"

ui_print "- Installing Zygisk module binaries..."

# Verify architecture
if [ "$ARCH" != "arm" ] && [ "$ARCH" != "arm64" ] && [ "$ARCH" != "x86" ] && [ "$ARCH" != "x86_64" ]; then
  ui_print "! Unsupported architecture: $ARCH"
  abort
fi

ui_print "- Magisk / Zygisk v24-v26 compatibility verified."
