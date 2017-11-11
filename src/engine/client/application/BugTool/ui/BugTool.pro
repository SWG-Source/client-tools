unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= BaseMainWindow.ui 
IMAGES	= images/letter_s_dark_red.png images/folder_blue.png images/folder_blue_open.png images/folder_green.png images/folder_green_open.png images/folder_grey.png images/folder_grey_open.png images/folder_orange.png images/folder_orange_open.png images/folder_red.png images/folder_red_open.png images/folder_yellow.png images/folder_yellow_open.png images/logical_animation_name.png 
TEMPLATE	=app
CONFIG	+= qt warn_on release
DBFILE	= BugTool.db
LANGUAGE	= C++
