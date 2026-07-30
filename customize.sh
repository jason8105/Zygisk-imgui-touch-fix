#!/system/bin/sh
SKIPUNZIP=0

# Ensure the zygisk directory exists in the module
mkdir -p "$MODPATH/zygisk"

# The .so files are placed into /zygisk/ by the zip task
# Magisk handles the loading based on the .so filename (arm64-v8a.so / armeabi-v7a.so)
ui_print "- Universal ImGui Touch Fix installed"
