// ======================================================================
//
// CuiAction.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiAction.h"

#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================

namespace CuiActionNamespace
{
	//----------------------------------------------------------------------
	
	Object * findClientOnlyObjectFromParam (const Unicode::String & param)
	{
		if (!param.empty () && param [0] == '@')
		{
			const uint32 uniqueId = static_cast<uint32>(atol (Unicode::wideToNarrow (param.substr (1)).c_str ()));
			return ClientObject::findClientObjectByUniqueId (uniqueId);
		}
		
		return 0;
	}
	
	//----------------------------------------------------------------------	
}

using namespace CuiActionNamespace;

//----------------------------------------------------------------------

CuiAction::CuiAction ()
{
}

//-----------------------------------------------------------------

CuiAction::~CuiAction ()
{
}

//----------------------------------------------------------------------

void  CuiAction::parseParams (const Unicode::String & params, StringVector & sv)
{
	size_t whichToken = 0;
	size_t tokenStart = 0;
	size_t tokenEnd   = 0;

	Unicode::tokenize   (params, sv, 0, whichToken, tokenStart, tokenEnd);
}

//----------------------------------------------------------------------

Object * CuiAction::findObjectFromParam (const Unicode::String & param)
{
	if (param.empty ())
		return 0;

	if (isdigit(param[0]) || (param[0] == '-' && isdigit(param[1])))
		return NetworkIdManager::getObjectById (NetworkId (Unicode::wideToNarrow (param)));
	else if (param [0] == '@')
		return findClientOnlyObjectFromParam (param);
	else
	{
		Object * const player = Game::getPlayer ();

		if (player)
		{
			const Vector & pos_w = player->getPosition_w ();
			return ClientWorld::findObjectByLocalizedAbbrev (pos_w, param);
		}
	}

	return 0;
}

//----------------------------------------------------------------------

Object * CuiAction::findObjectFromFirstParam (const Unicode::String & params, bool useLookAt, bool mustExist, const std::string & id)
{
	Object * obj = 0;
	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		obj = 0;
	else
	{
		if (params.empty ())
		{
			if (useLookAt)
				obj = player->getLookAtTarget ().getObject ();
		}
		else
		{
			static StringVector sv;
			sv.clear ();
			
			parseParams (params, sv);
			if (!sv.empty ())
			{
				obj = findObjectFromParam (sv [0]);

				if (!obj && useLookAt)
					obj = player->getLookAtTarget ().getObject ();
				
				if (!obj)
				{
					if (mustExist)
					{
						Unicode::String result;
						if (id.empty ())
							CuiStringVariablesManager::process (CuiStringIds::action_target_not_found_prose, Unicode::emptyString, sv [0], Unicode::emptyString, result);
						else
							CuiStringVariablesManager::process (CuiStringIds::action_target_not_found_id_prose, Unicode::narrowToWide (id), sv [0], Unicode::emptyString, result);
						
						CuiMessageBox::createInfoBox       (result);
					}
					return 0;
				}
			}
		}
	}

	if (!obj && useLookAt)
		obj = player->getLookAtTarget ().getObject ();

	if (mustExist && obj == 0)
	{
		if (id.empty ())
		{
			CuiMessageBox::createInfoBox       (CuiStringIds::action_no_target_prose.localize ());
		}
		else
		{
			Unicode::String result;
			CuiStringVariablesManager::process (CuiStringIds::action_no_target_prose, id, std::string (), result);
			CuiMessageBox::createInfoBox       (result);
		}
	}

	return obj;
}

// ======================================================================

