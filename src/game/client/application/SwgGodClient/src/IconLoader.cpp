// ======================================================================
//
// IconLoader.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "IconLoader.h"

#include <qmime.h>
#include <qdragobject.h>

// ======================================================================

#define ICON_NAME(name) const char * const IconLoader::##name = #name
#define DEFAULT_ICON_NAME(name) const char * const IconLoader::##name = ""

ICON_NAME (hi16_action_undo);
ICON_NAME (hi16_action_redo);

ICON_NAME (hi16_action_revert);
ICON_NAME (hi16_action_reload);
ICON_NAME (hi16_action_edit);
ICON_NAME (hi16_action_editdelete);
ICON_NAME (hi16_action_editcut);
ICON_NAME (hi16_action_editcopy);
ICON_NAME (hi16_action_editpaste);
ICON_NAME (hi16_action_window_new);

ICON_NAME (hi16_action_up);
ICON_NAME (hi16_action_down);
ICON_NAME (hi16_action_back);
ICON_NAME (hi16_action_forward);
ICON_NAME (hi16_action_top);
ICON_NAME (hi16_action_bottom);
ICON_NAME (hi16_action_finish);
ICON_NAME (hi16_action_start);

ICON_NAME (hi16_action_bookmark);
ICON_NAME (hi16_action_bookmark_add);
ICON_NAME (hi16_action_bookmark_folder);
ICON_NAME (hi16_action_bookmark_toolbar);
ICON_NAME (hi16_action_bookmark_red);
ICON_NAME (hi16_action_bookmark_red_add);
ICON_NAME (hi16_action_bookmark_red_folder);
ICON_NAME (hi16_action_bookmark_red_toolbar);

ICON_NAME (hi16_action_drop_to_terrain);
ICON_NAME (hi16_action_random_rotate);
ICON_NAME (hi16_action_apply_transform);
ICON_NAME (hi16_action_rotate_reset);
ICON_NAME (hi16_action_align_to_terrain);

ICON_NAME (hi16_action_rotatemode_yaw);
ICON_NAME (hi16_action_rotatemode_pitch);
ICON_NAME (hi16_action_rotatemode_roll);
ICON_NAME (hi16_action_rotate_increment45);
ICON_NAME (hi16_action_rotate_decrement45);

DEFAULT_ICON_NAME (hi16_action_renderWireframe);
DEFAULT_ICON_NAME (hi16_action_renderTextured);
DEFAULT_ICON_NAME (hi16_action_showObjectNames);
DEFAULT_ICON_NAME (hi16_action_showObjectNetworkIds);
DEFAULT_ICON_NAME (hi16_action_terrainShowLOD);
DEFAULT_ICON_NAME (hi16_action_terrainFreezeLOD);
DEFAULT_ICON_NAME (hi16_action_terrainAdjustLOD);
DEFAULT_ICON_NAME (hi16_action_terrainShowVertexNormals);
DEFAULT_ICON_NAME (hi16_action_terrainShowFaceNormals);
DEFAULT_ICON_NAME (hi16_action_terrainAdjustRadial);

ICON_NAME (hi16_action_hud);
ICON_NAME (hi16_action_center_selection);
ICON_NAME (hi16_action_center_ghosts);
ICON_NAME (hi16_action_fit_selection);
ICON_NAME (hi16_action_fit_ghosts);
ICON_NAME (hi16_action_console);

ICON_NAME (hi16_filesys_folder);
ICON_NAME (hi16_filesys_folder_open);
ICON_NAME (hi16_filesys_folder_blue);
ICON_NAME (hi16_filesys_folder_blue_open);
ICON_NAME (hi16_filesys_folder_cyan);
ICON_NAME (hi16_filesys_folder_cyan_open);
ICON_NAME (hi16_filesys_folder_green);
ICON_NAME (hi16_filesys_folder_green_open);
ICON_NAME (hi16_filesys_folder_grey);
ICON_NAME (hi16_filesys_folder_grey_open);
ICON_NAME (hi16_filesys_folder_orange);
ICON_NAME (hi16_filesys_folder_orange_open);
ICON_NAME (hi16_filesys_folder_red);
ICON_NAME (hi16_filesys_folder_red_open);
ICON_NAME (hi16_filesys_folder_violet);
ICON_NAME (hi16_filesys_folder_violet_open);
ICON_NAME (hi16_filesys_folder_yellow);
ICON_NAME (hi16_filesys_folder_yellow_open);
ICON_NAME (hi16_filesys_network);

ICON_NAME (hi16_action_gear);
ICON_NAME (hi16_mime_document);
ICON_NAME (hi16_mime_document2);

#undef ICON_NAME

/* XPM */ 
namespace
{
	/* XPM */
	const char *s_default_pixmap[] = {
		/* columns rows colors chars-per-pixel */
		"13 13 16 1",
		"  c Gray0",
		". c #800000",
		"X c #008000",
		"o c #808000",
		"O c #000080",
		"+ c #800080",
		"@ c #008080",
		"# c #c0c0c0",
		"$ c #808080",
		"% c Red",
		"& c Green",
		"* c Yellow",
		"= c Blue",
		"- c Magenta",
		"; c Cyan",
		": c Gray100",
		/* pixels */
		":::::::::::::",
		":::::&&&:::::",
		":::&&==&&&:::",
		"::&&&==&&&&::",
		":&&&&&&&&&&&:",
		":&&&&&&&&&&&:",
		"&&&&&==&&&&&&",
		"&&&&&==&&&&&&",
		":&&&&==&&&&&:",
		":&&&&==&&&&&:",
		"::&&&==&&&&::",
		"::&&&==&&&:::",
		":::::&&&:::::"
	};
}
//-----------------------------------------------------------------
const QIconSet IconLoader::fetchIcon   (const char * const name, const QIconSet::Size size)
{
	return QIconSet (fetchPixmap (name), size);
}
//-----------------------------------------------------------------
const QPixmap  IconLoader::fetchPixmap (const char * name)
{
	//	QPixmap pix (s_default_pixmap);
	QPixmap pix;
	
	if (name && *name)
	{
		const QMimeSource * const m = name ? QMimeSourceFactory::defaultFactory()->data( name ) : 0;
		
		if (m)
			IGNORE_RETURN (QImageDrag::decode( m, pix ));
		else {
			WARNING (name && strlen (name), ("unable to load icon: %s\n", name));
		}
	}
	
	return pix;
}
// ======================================================================
