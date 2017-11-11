//======================================================================
//
// CuiRecipeManager.cpp
// copyright (c) 2009 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiRecipeManager.h"

#include "sharedFoundation/NetworkId.h"

namespace CuiRecipeManagerNamespace
{
	bool	  ms_installed = false;
	NetworkId ms_recipeId = NetworkId::cms_invalid;
}
using namespace CuiRecipeManagerNamespace;

void CuiRecipeManager::install()
{
	DEBUG_FATAL (ms_installed, ("installed"));
	ms_installed = true;
}

void CuiRecipeManager::remove()
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	ms_installed = false;
}

void CuiRecipeManager::setRecipeObject(NetworkId const & id)
{
	ms_recipeId = id;
}

NetworkId const & CuiRecipeManager::getRecipeOID()
{
	return ms_recipeId;
}