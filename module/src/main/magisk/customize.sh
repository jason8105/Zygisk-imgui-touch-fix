SKIPUNZIP=0

ui_print "****************************************"
ui_print "     Zygisk Unity ImGui Touch Fix       "
ui_print "****************************************"

if [ "$BOOTMODE" ! T ]; then
  abort "[-] Please flash this module via Magisk or KernelSU app!"
fi

ui_print "[+] Extracting module files..."
unzip -o "$ZIPFILE" 'zygisk/*' -d $MODPATH >&2

ui_print "[+] Installation complete!"
