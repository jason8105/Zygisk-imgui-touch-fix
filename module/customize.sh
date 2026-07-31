#!/sbin/sh
SKIPUNZIP=0

ui_print "- Installing Universal ImGui Zygisk Module..."
ui_print "- Target Magisk: v24.0 - v26.x compatible"

# Check zygisk architecture files
if [ -d "$ZIPFILE/zygisk" ]; then
  ui_print "- Found Zygisk native libraries"
else
  ui_print "! Zygisk native libraries missing from zip"
  exit 1
fi

ui_print "- Universal ImGui Zygisk Module installed successfully!"
