#!/sbin/sh
# Magisk module install script
ui_print "Installing Zygisk ImGui Touch Fix..."
MODPATH=/data/adb/modules/zygisk_imgui
mkdir -p $MODPATH/lib
cp -f $ZIPFILE/lib/*.so $MODPATH/lib/
set_perm_recursive $MODPATH 0 0 0755 0644
