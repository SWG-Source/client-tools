// ======================================================================
//
// PluginMain.h
// Portions Copyright 1999, Bootprint Entertainment Inc.
// Portions Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_PluginMain_H
#define INCLUDED_PluginMain_H

// ======================================================================

// -TRF- do not change the order of these!!!  if you add a new directory
//       to track, put it on the end.
// -EAS- also note that any addtions to this enum must be matched by a call to SetDirectoryCommand::registerDirectory in StartEngine()
enum
{
	APPEARANCE_WRITE_DIR_INDEX,
	SHADER_TEMPLATE_WRITE_DIR_INDEX,
	SHADER_TEMPLATE_REFERENCE_DIR_INDEX,
	EFFECT_REFERENCE_DIR_INDEX,
	TEXTURE_REFERENCE_DIR_INDEX,
	TEXTURE_WRITE_DIR_INDEX,
	TEXTURE_RENDERER_REFERENCE_DIR_INDEX,
	ANIMATION_WRITE_DIR_INDEX,
	AUTHOR_INDEX,
	SKELETON_TEMPLATE_WRITE_DIR_INDEX,
	SKELETON_TEMPLATE_REFERENCE_DIR_INDEX,
	LOG_DIR_INDEX,
	SAT_WRITE_DIR_INDEX,
	APPEARANCE_REFERENCE_DIR_INDEX,
	ASSET_DB_NAME_DIR_INDEX,
	ASSET_DB_LOGINNAME_DIR_INDEX,
	ASSET_DB_PASSWORD_DIR_INDEX,
	ASSET_DB_ACTIVATED_INDEX,
	VIEWER_LOCATION_INDEX,
	PERFORCE_BRANCH_LIST_INDEX,
	ALIENBRAIN_PROJECT_NAME_INDEX,
	ACTIVE_DRIVE_INDEX
};

// ======================================================================

HINSTANCE GetPluginInstanceHandle(void);
void      StopEngine(bool isCleanShutdown = true);

// ======================================================================

#endif
