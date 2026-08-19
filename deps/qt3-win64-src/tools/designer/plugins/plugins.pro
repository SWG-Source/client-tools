TEMPLATE    = subdirs
PLUGIN_DIRS = wizards \
	      cppeditor \
	      dlg \
	      glade \
	      rc \
	      kdevdlg
shared:SUBDIRS *= $$PLUGIN_DIRS
dll:SUBDIRS *= $$PLUGIN_DIRS
