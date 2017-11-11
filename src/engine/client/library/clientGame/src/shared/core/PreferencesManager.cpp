//
// PreferencesManager.cpp
// asommers 2000-10-25
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/PreferencesManager.h"

#include "sharedFile/Iff.h"

//-------------------------------------------------------------------

const real real_color161 = CONST_REAL (161) / CONST_REAL (255);
const VectorArgb PreferencesManager::color161 (CONST_REAL (1), real_color161, real_color161, real_color161);

//-------------------------------------------------------------------

PreferencesManager::PreferencesManager (void) :
	dirty (true)
{
	cameraMode          = 0;
	centerOn            = 0;
	movementMode        = 0;
	zsortSelectionBoxes = true;

	//-- camera preferences
	fogEnabled          = false;
	fogDensity          = CONST_REAL (0.01);
	nearDistance        = 1;
	farDistance         = 2500;
	fov                 = 90;
	startDistance       = 100;
	endDistance         = 150;
	fogMode             = 0;

	clearColor          = color161;

	showGrid = true;
	showAxis = true;
}

//-------------------------------------------------------------------

PreferencesManager::~PreferencesManager (void)
{
}

//-------------------------------------------------------------------

void PreferencesManager::load_0000 (Iff& iff)
{
	iff.enterChunk (TAG_0000);
	{
		cameraMode          = iff.read_int32 ();
		centerOn            = iff.read_int32 ();
		endDistance         = iff.read_float ();
		farDistance         = iff.read_float ();
		fogDensity          = iff.read_float ();
		fogEnabled          = iff.read_int32 () != 0;
		fogMode             = iff.read_int32 ();
		fogColor            = iff.read_floatVectorArgb ();
		fov                 = iff.read_int32 ();
		movementMode        = iff.read_int32 ();
		nearDistance        = iff.read_float ();
		startDistance       = iff.read_float ();
		zsortSelectionBoxes = iff.read_int32 () != 0;
		clearColor          = color161;
		showAxis            = true;
		showGrid            = true;
	}
	iff.exitChunk ();
}

void PreferencesManager::load_0001 (Iff& iff)
{
	iff.enterChunk (TAG_0001);
	{
		cameraMode          = iff.read_int32 ();
		centerOn            = iff.read_int32 ();
		endDistance         = iff.read_float ();
		farDistance         = iff.read_float ();
		fogDensity          = iff.read_float ();
		fogEnabled          = iff.read_int32 () != 0;
		fogMode             = iff.read_int32 ();
		fogColor            = iff.read_floatVectorArgb ();
		fov                 = iff.read_int32 ();
		movementMode        = iff.read_int32 ();
		nearDistance        = iff.read_float ();
		startDistance       = iff.read_float ();
		zsortSelectionBoxes = iff.read_int32 () != 0;
		clearColor          = iff.read_floatVectorArgb ();
		showAxis            = true;
		showGrid            = true;
	}
	iff.exitChunk ();
}

void PreferencesManager::load_0002 (Iff& iff)
{
	iff.enterChunk (TAG_0002);
	{
		cameraMode          = iff.read_int32 ();
		centerOn            = iff.read_int32 ();
		endDistance         = iff.read_float ();
		farDistance         = iff.read_float ();
		fogDensity          = iff.read_float ();
		fogEnabled          = iff.read_int32 () != 0;
		fogMode             = iff.read_int32 ();
		fogColor            = iff.read_floatVectorArgb ();
		fov                 = iff.read_int32 ();
		movementMode        = iff.read_int32 ();
		nearDistance        = iff.read_float ();
		startDistance       = iff.read_float ();
		zsortSelectionBoxes = iff.read_int32 () != 0;
		clearColor          = iff.read_floatVectorArgb ();
		showGrid            = iff.read_int32 () != 0;
		showAxis            = iff.read_int32 () != 0;
	}
	iff.exitChunk ();
}

//-------------------------------------------------------------------

void PreferencesManager::load (Iff& iff)
{
	setDirty ();

	iff.enterForm (TAG (P,R,E,F));
	{
		switch (iff.getCurrentName ())
		{
		case TAG_0000:
			load_0000 (iff);
			break;

		case TAG_0001:
			load_0001 (iff);
			break;

		case TAG_0002:
			load_0002 (iff);
			break;
		}
	}
	iff.exitForm ();
}

//-------------------------------------------------------------------
	
void PreferencesManager::save (Iff& iff) const
{
	iff.insertForm (TAG (P,R,E,F));
	{
		iff.insertChunk (TAG_0002);
		{
			iff.insertChunkData (cameraMode);
			iff.insertChunkData (centerOn);
			iff.insertChunkData (endDistance);
			iff.insertChunkData (farDistance);
			iff.insertChunkData (fogDensity);
			iff.insertChunkData (fogEnabled ? static_cast<int32> (1) : static_cast<int32> (0));
			iff.insertChunkData (fogMode);
			iff.insertChunkFloatVectorArgb (fogColor);
			iff.insertChunkData (fov);
			iff.insertChunkData (movementMode);
			iff.insertChunkData (nearDistance);
			iff.insertChunkData (startDistance);
			iff.insertChunkData (zsortSelectionBoxes ? static_cast<int32> (1) : static_cast<int32> (0));
			iff.insertChunkFloatVectorArgb (clearColor);
			iff.insertChunkData (showGrid ? static_cast<int32> (1) : static_cast<int32> (0));
			iff.insertChunkData (showAxis ? static_cast<int32> (1) : static_cast<int32> (0));
		}
		iff.exitChunk ();
	}
	iff.exitForm ();
}

//-------------------------------------------------------------------

