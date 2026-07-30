SKIPUNZIP=0
ui_print "****************************************"
ui_print "    Zygisk ImGui Universal Touch Fix    "
ui_print "****************************************"

if [ "$ARCH" != "arm64" ] && [ "$ARCH" != "arm" ] && [ "$ARCH" != "x86" ] && [ "$ARCH" != "x86_64" ]; then
  abort "! Unsupported architecture: $ARCH"
fi

ui_print "- Extracting Zygisk module files"
unzip -o "$ZIPFILE" 'zygisk/*' -d "$MODPATH" >&2
unzip -o "$ZIPFILE" 'module.prop' -d "$MODPATH" >&2

set_perm_recursive "$MODPATH" 0 0 0755 0644
