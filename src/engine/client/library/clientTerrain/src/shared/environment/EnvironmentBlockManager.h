//===================================================================
//
// EnvironmentBlockManager.h
// asommers
//
// copyright 2002, sony online entertainment
// 
//===================================================================

#ifndef INCLUDED_EnvironmentBlockManager_H
#define INCLUDED_EnvironmentBlockManager_H

//===================================================================

class EnvironmentBlock;
class EnvironmentGroup;

//===================================================================

class EnvironmentBlockManager
{
public:

	EnvironmentBlockManager (const EnvironmentGroup* environmentGroup, const char* fileName);
	virtual ~EnvironmentBlockManager ();

	const EnvironmentBlock* getEnvironmentBlock (int familyId, int weatherIndex) const;
	const EnvironmentBlock* getDefaultEnvironmentBlock () const;

private:

	void load (const EnvironmentGroup* environmentGroup, const char* fileName);

private:

	EnvironmentBlockManager ();
	EnvironmentBlockManager (const EnvironmentBlockManager&);
	EnvironmentBlockManager& operator= (const EnvironmentBlockManager&);

private:

	typedef stdmap<int, EnvironmentBlock*>::fwd EnvironmentBlockMap;
	EnvironmentBlockMap* const m_environmentBlockMap;
	EnvironmentBlock* const    m_defaultEnvironmentBlock;
};

//===================================================================

#endif
