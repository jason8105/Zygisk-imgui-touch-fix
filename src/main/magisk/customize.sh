ui_print "**************************************************"
ui_print "       Zygisk ImGui Universal Touch Fix          "
ui_print "       Magisk v24-26 Compatible                  "
ui_print "**************************************************"

if [ "$BOOTMODE" ! = true ]; then
  abort "[-] Please install this module from within Magisk Manager."
fi

# Ensure architecture support
ABI=""
case "%ARCH%" in
  arm|armeabi-v7a) ABI="armeabi-v7a" ;;
  arm64|aarch64) ABI="arm64-v8a" ;;
  x86) ABI="x86" ;;
  x86_64) ABI="x86_64" ;;
  *) abort "[-] Unsupported architecture: %ARCH%" ;;
es

ui_print "[+] Target Architecture: $ABI"
ui_print "[+] Installation completed successfully!"
