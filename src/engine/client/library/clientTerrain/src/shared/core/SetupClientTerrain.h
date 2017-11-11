// ======================================================================
//
// SetupClientTerrain.h
// asommers 9-11-2000
//
// copyright 2000, verant interactive
//
// ======================================================================

#ifndef INCLUDED_SetupClientTerrain_H
#define INCLUDED_SetupClientTerrain_H

// ======================================================================

class SetupClientTerrain
{
public:

	static void install();
	static void remove();

	static bool isInstalled();

private:

	SetupClientTerrain();
	SetupClientTerrain(SetupClientTerrain const &);
	SetupClientTerrain & operator=(SetupClientTerrain const &);
};

// ======================================================================

#endif
