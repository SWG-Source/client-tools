// ======================================================================
//
// IconLoader.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_IconLoader_H
#define INCLUDED_IconLoader_H

// ======================================================================

#include <qpixmap.h>
#include <qiconset.h>


/**
* IconLoader is a static class used mainly to enumerate and fetch pixmaps from
* the pixmap collection defined in the Qt Designer.  The pixmap data is stored
* in the Designer-generated images.cpp file.
*/

class IconLoader
{
public:

	static const char * const hi16_action_undo;
	static const char * const hi16_action_redo;

	static const char * const hi16_action_revert;
	static const char * const hi16_action_reload;

	static const char * const hi16_action_edit;
	static const char * const hi16_action_editdelete;
	static const char * const hi16_action_editcut;
	static const char * const hi16_action_editcopy;
	static const char * const hi16_action_editpaste;
	static const char * const hi16_action_window_new;

	static const char * const hi16_action_up;
	static const char * const hi16_action_down;
	static const char * const hi16_action_back;
	static const char * const hi16_action_forward;
	static const char * const hi16_action_top;
	static const char * const hi16_action_bottom;
	static const char * const hi16_action_finish;
	static const char * const hi16_action_start;

	static const char * const hi16_action_bookmark;
	static const char * const hi16_action_bookmark_add;
	static const char * const hi16_action_bookmark_folder;
	static const char * const hi16_action_bookmark_toolbar;
	static const char * const hi16_action_bookmark_red;
	static const char * const hi16_action_bookmark_red_add;
	static const char * const hi16_action_bookmark_red_folder;
	static const char * const hi16_action_bookmark_red_toolbar;

	static const char * const hi16_action_drop_to_terrain;
	static const char * const hi16_action_random_rotate;
	static const char * const hi16_action_apply_transform;
	static const char * const hi16_action_gear;
	static const char * const hi16_action_rotate_reset;
	static const char * const hi16_action_align_to_terrain;

	static const char * const hi16_action_rotatemode_yaw;
	static const char * const hi16_action_rotatemode_pitch;
	static const char * const hi16_action_rotatemode_roll;

	static const char * const hi16_action_rotate_increment45;
	static const char * const hi16_action_rotate_decrement45;

	static const char * const hi16_action_renderWireframe;
	static const char * const hi16_action_renderTextured;
	static const char * const hi16_action_showObjectNames;
	static const char * const hi16_action_showObjectNetworkIds;
	static const char * const hi16_action_terrainShowLOD;
	static const char * const hi16_action_terrainFreezeLOD;
	static const char * const hi16_action_terrainAdjustLOD;
	static const char * const hi16_action_terrainShowVertexNormals;
	static const char * const hi16_action_terrainShowFaceNormals;
	static const char * const hi16_action_terrainAdjustRadial;
	static const char * const hi16_action_hud;

	static const char * const hi16_action_center_selection;
	static const char * const hi16_action_center_ghosts;
	static const char * const hi16_action_fit_selection;
	static const char * const hi16_action_fit_ghosts;

	static const char * const hi16_action_console;

	static const char * const hi16_filesys_folder;
	static const char * const hi16_filesys_folder_open;
	static const char * const hi16_filesys_folder_blue;
	static const char * const hi16_filesys_folder_blue_open;
	static const char * const hi16_filesys_folder_cyan;
	static const char * const hi16_filesys_folder_cyan_open;
	static const char * const hi16_filesys_folder_green;
	static const char * const hi16_filesys_folder_green_open;
	static const char * const hi16_filesys_folder_grey;
	static const char * const hi16_filesys_folder_grey_open;
	static const char * const hi16_filesys_folder_orange;
	static const char * const hi16_filesys_folder_orange_open;
	static const char * const hi16_filesys_folder_red;
	static const char * const hi16_filesys_folder_red_open;
	static const char * const hi16_filesys_folder_violet;
	static const char * const hi16_filesys_folder_violet_open;
	static const char * const hi16_filesys_folder_yellow;
	static const char * const hi16_filesys_folder_yellow_open;
	static const char * const hi16_filesys_network;

	static const char * const hi16_mime_document;
	static const char * const hi16_mime_document2;

	static const QIconSet fetchIcon   (const char * name, QIconSet::Size size);
	static const QPixmap  fetchPixmap (const char * name);

private:
	//disabled
	IconLoader();
	IconLoader(const IconLoader & rhs);
	IconLoader& operator=(const IconLoader & rhs);
};

#define IL_PIXMAP(name) IconLoader::fetchPixmap(IconLoader::##name)

// ======================================================================

#endif
