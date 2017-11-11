TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release

FORMS	= BaseQuestWidget.ui \
	BaseMainWindow.ui \
	BaseHelpDialog.ui

IMAGES	= images/compile.bmp \
	images/console.bmp \
	images/exit.bmp \
	images/new.bmp \
	images/open.bmp \
	images/p4.bmp \
	images/random.bmp \
	images/save.bmp \
	images/saveas.bmp \
	images/export.bmp \
	images/close.bmp \
	images/table.bmp \
	images/cascade.png \
	images/tile.png \
	images/tile_horizontal.png \
	images/about.png

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
