TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release

FORMS	= BaseNewNpcDialog.ui \
	BaseMainWindow.ui \
	BaseTemplateConfigDialog.ui \
	BaseSaveDialog.ui \
	BasePaletteColorPicker.ui \
	BaseConsoleWindow.ui \
	BaseCustomVariableWidget.ui

IMAGES	= images/compile.bmp \
	images/console.bmp \
	images/exit.bmp \
	images/new.bmp \
	images/open.bmp \
	images/p4.bmp \
	images/random.bmp \
	images/save.bmp \
	images/saveas.bmp \
	images/templates.bmp \
	images/wearable_add.bmp \
	images/wearable_remove.bmp \
	images/wearable_search.bmp \
	images/zoomhead.bmp \
	images/zoomroot.bmp

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
