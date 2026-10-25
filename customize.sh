#!/system/bin/sh
SKIPUNZIP=0
ui_print "- Installing SystemComponent"
set_perm_recursive $MODPATH 0 0 0755 0644
