SKIPUNZIP=1

ui_print "- Installing Zygisk Universal ImGui Touch Fix"

# Extract zip contents to module path
unzip -o "$ZIPFILE" 'zygisk/*' -d "$MODPATH"
unzip -o "$ZIPFILE" 'module.prop' -d "$MODPATH"

# Set permissions
set_perm_recursive "$MODPATH" 0 0 0755 0644

ui_print "- Module installation completed successfully."
