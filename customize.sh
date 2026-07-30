SKIPUNZIP=0

ui_print "*************************************************"
ui_print "      Zygisk ImGui Universal Touch Fix           "
ui_print "*************************************************"

if [ "$BOOTMODE" ] && [ "$KEEPRUNNING" ]; then
  ui_print "- Installing from Magisk App"
else
  ui_print "- Installing via Recovery / Sideload"
fi

# Check architecture
ABI=$(getprop ro.product.cpu.abi)
ui_print "- Device ABI: $ABI"

if [ "$ARCH" = "arm64" ] || [ "$ARCH" = "x86_64" ] || [ "$ARCH" = "armeabi-v7a" ] || [ "$ARCH" = "x86" ]; then
  ui_print "- Target architecture supported: $ARCH"
else
  ui_print "! Unsupported architecture: $ARCH"
  abort "Aborting installation!"
fi

# Set permissions
ui_print "- Setting up module directory structure..."
ui_print "- Extracting Zygisk binaries..."
set_perm_recursive $MODPATH 0 0 0755 0644
if [ -d "$MODPATH/system/bin" ]; then
  set_perm_recursive $MODPATH/system/bin 0 0 0755 0755
fi

ui_print "*************************************************"
ui_print "      Installation Completed Successfully!       "
ui_print "*************************************************"
