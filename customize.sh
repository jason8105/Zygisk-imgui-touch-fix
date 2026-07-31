#!/system/bin/sh
SKIPUNZIP=0

ui_print "--------------------------------------"
ui_print "- Installing Zygisk Universal ImGui Menu"
ui_print "--------------------------------------"

# Verify Magisk version and Zygisk state
if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
  ui_print "! Magisk version 24.0+ is required"
  abort "! Unsupported Magisk version"
fi

# Print target device architecture
ARCH=$(getprop ro.product.cpu.abi)
ui_print "- Target ABI: $ARCH"

# Create zygisk directory structure if needed
mkdir -p "$MODPATH/zygisk"

ui_print "- Installation successful!"
