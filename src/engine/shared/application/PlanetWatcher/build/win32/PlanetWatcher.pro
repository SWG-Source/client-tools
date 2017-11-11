TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release

FORMS	= ../../src/win32/ui/BasePlanetWatcher.ui \
	../../src/win32/ui/BaseServerConnectionDialog.ui \
	../../src/win32/ui/BaseFilterDialog.ui

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
