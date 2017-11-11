//===================================================================
//
// InteriorEnvironmentBlockManager.h
// asommers
//
// copyright 2002, sony online entertainment
// 
//===================================================================

#ifndef INCLUDED_InteriorEnvironmentBlockManager_H
#define INCLUDED_InteriorEnvironmentBlockManager_H

//===================================================================

class InteriorEnvironmentBlock;

//===================================================================

class InteriorEnvironmentBlockManager
{
public:

	static void install ();

	static const InteriorEnvironmentBlock* getEnvironmentBlock (const char* pobShortName, const char* cellName);
	static const InteriorEnvironmentBlock* getDefaultEnvironmentBlock ();

private:

	InteriorEnvironmentBlockManager ();
	~InteriorEnvironmentBlockManager ();
	InteriorEnvironmentBlockManager (const InteriorEnvironmentBlockManager&);
	InteriorEnvironmentBlockManager& operator= (const InteriorEnvironmentBlockManager&);
};

//===================================================================

#endif
