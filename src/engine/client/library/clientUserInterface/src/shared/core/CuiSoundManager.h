//======================================================================
//
// CuiSoundManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiSoundManager_H
#define INCLUDED_CuiSoundManager_H

//======================================================================

class CuiSoundManager
{
public:

	static void                play (const std::string & name);
	static void                restart(const std::string & name);
	static const std::string & findSoundPath (const std::string & name);
};

//======================================================================

#endif
