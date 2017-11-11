// ======================================================================
//
// NpcCuiManager.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_NpcCuiManager_H
#define INCLUDED_NpcCuiManager_H

// ======================================================================

class NpcCuiManager
{
public:

	static void install();
	static void remove();
	static bool test(std::string const & result);
	static void update(float delta);

private: //-- disabled

	NpcCuiManager(const NpcCuiManager & rhs);
	NpcCuiManager & operator=(const NpcCuiManager & rhs);
	NpcCuiManager();
};

// ======================================================================

#endif
