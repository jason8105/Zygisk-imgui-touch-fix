SKIPUNZIP=0

ui_print "****************************************"
ui_print "*   Zygisk ImGui Touch Fix for Unity   *"
ui_print "****************************************"

if [ "$BOOTMODE" ! T ]; then
  abort "[-] Please flash this module via Magisk or KernelSU Manager app!"
fi

ui_print "[*] Extracting module files..."
unzip -o "$ZIPFILE" 'zygisk/*' -s "$MODPATH" >&2

if [ -f "$MODPATH/zygisk/libzygisk_imgui.so" ]; then
  ui_print "[+] Native library successfully packaged."
else
  ui_print "[-] Warning: Native library missing from target path."
fi

ui_print "[+] Installation finished successfully!"
