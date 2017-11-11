//======================================================================
//
// CuiRecipeManager.h
// copyright (c) 2009 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiRecipeManager_H
#define INCLUDED_CuiRecipeManager_H

class NetworkId;
class CuiRecipeManager
{
public:
	static void install();
	static void remove();
	static void setRecipeObject(NetworkId const & id);
	static NetworkId const & getRecipeOID();
};

#endif