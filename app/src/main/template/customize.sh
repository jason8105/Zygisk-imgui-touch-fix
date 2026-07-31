SKIPUNZIP=0

ui_print "- Installing Zygisk Universal ImGui Menu..."
ui_print "- Target Magisk Version: v24.0 - v26.x"

if [ ! -d "$ZIPFILE/zygisk" ]; then
    ui_print "! ERROR: Zygisk native libraries not found in zip!"
    exit 1
fi

ui_print "- Installation completed successfully."
