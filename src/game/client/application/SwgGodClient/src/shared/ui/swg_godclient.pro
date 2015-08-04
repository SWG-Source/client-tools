TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release

FORMS	= BaseGameWindow.ui \
	BaseConsoleWindow.ui \
	BaseTreeBrowser.ui \
	BaseObjectEditor.ui \
	BaseBookmarkBrowser.ui \
	BaseSplashScreen.ui \
	BaseGroupObjectWindow.ui \
	BaseObjectTransformWindow.ui \
	BaseSnapToGridSettings.ui \
	BaseTriggerWindow.ui \
	BaseFilterWindow.ui \
	BaseRegionBrowser.ui \
	BaseSystemMessageWidget.ui \
	BaseFormWindow.ui \
	BaseGotoDialog.ui \
	BaseFavoritesWindow.ui

IMAGES	= images/hi16_action_1downarrow \
	images/hi16_action_align_to_terrain \
	images/hi16_action_apply_transform \
	images/hi16_action_back \
	images/hi16_action_bookmark \
	images/hi16_action_bookmark_add \
	images/hi16_action_bookmark_folder \
	images/hi16_action_bookmark_red \
	images/hi16_action_bookmark_red_add \
	images/hi16_action_bookmark_red_folder \
	images/hi16_action_bookmark_red_toolbar \
	images/hi16_action_bookmark_toolbar \
	images/hi16_action_bottom \
	images/hi16_action_center_ghosts \
	images/hi16_action_center_selection \
	images/hi16_action_console \
	images/hi16_action_down \
	images/hi16_action_drop_to_terrain \
	images/hi16_action_edit \
	images/hi16_action_editcopy \
	images/hi16_action_editcut \
	images/hi16_action_editdelete \
	images/hi16_action_editpaste \
	images/hi16_action_fileopen \
	images/hi16_action_finish \
	images/hi16_action_fit_ghosts \
	images/hi16_action_fit_selection \
	images/hi16_action_forward \
	images/hi16_action_gear \
	images/hi16_action_hud \
	images/hi16_action_random_rotate \
	images/hi16_action_redo \
	images/hi16_action_reload \
	images/hi16_action_revert \
	images/hi16_action_rotate_decrement45 \
	images/hi16_action_rotate_increment45 \
	images/hi16_action_rotate_reset \
	images/hi16_action_rotatemode_pitch \
	images/hi16_action_rotatemode_roll \
	images/hi16_action_rotatemode_yaw \
	images/hi16_action_start \
	images/hi16_action_top \
	images/hi16_action_undo \
	images/hi16_action_up \
	images/hi16_action_window_new \
	images/hi16_filesys_folder_blue \
	images/hi16_filesys_folder_blue_open \
	images/hi16_filesys_folder_cyan \
	images/hi16_filesys_folder_cyan_open \
	images/hi16_filesys_folder_green \
	images/hi16_filesys_folder_green_open \
	images/hi16_filesys_folder_grey \
	images/hi16_filesys_folder_grey_open \
	images/hi16_filesys_folder_orange \
	images/hi16_filesys_folder_orange_open \
	images/hi16_filesys_folder_red \
	images/hi16_filesys_folder_red_open \
	images/hi16_filesys_folder_violet \
	images/hi16_filesys_folder_violet_open \
	images/hi16_filesys_folder_yellow \
	images/hi16_filesys_folder_yellow_open \
	images/hi16_filesys_network \
	images/hi16_mime_document \
	images/hi16_mime_document2 \
	images/splash \
	images/sb_splash

TARGET	= swg_godclient
IMAGEFILE	= images.cpp
PROJECTNAME	= swg_godclient
