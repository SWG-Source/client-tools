//===================================================================
//
// PreloadedAssetManager.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_PreloadedAssetManager_H
#define INCLUDED_PreloadedAssetManager_H

//===================================================================

class PreloadedAssetManager
{
public:

	static void install ();

	static void addSoundTemplate(char const * fileName);

private:

	static void remove ();
};

//===================================================================

#endif
