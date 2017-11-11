//======================================================================
//
// CuiSetName.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiSetName.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMediatorTypes.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/Waypoint.h"
#include "sharedObject/NetworkIdManager.h"

//======================================================================
namespace
{
	uint32 static const crc_setWaypointName = Crc::normalizeAndCalculate("setWaypointName");
}
//----------------------------------------------------------------------

CuiSetName::CuiSetName       (UIPage & page) :
CuiInputBox (page)
{
}

//----------------------------------------------------------------------

CuiSetName::~CuiSetName      ()
{
}

//----------------------------------------------------------------------

void CuiSetName::handleResult (bool affirmative, Unicode::String const & s)
{
	CuiInputBox::handleResult (affirmative, s);

	if (affirmative)
	{
		NetworkId const & id = getObject ();
		Object const * const obj = NetworkIdManager::getObjectById (id);
		ClientObject const * const clientObject = obj ? obj->asClientObject () : 0;
		if (clientObject)
		{
			//remove leading @'s to prevent the name from being interpreted
			//as a code string
			int i = 0;
			int nameLength = s.length ();
			while(i < nameLength && s[i] == '@')
			{
				++i;
			}

			Unicode::String cleanedName = s.substr(i,nameLength);

			if (dynamic_cast<const ClientWaypointObject *>(clientObject))
				ClientCommandQueue::enqueueCommand (crc_setWaypointName, id, cleanedName);
			else
			{
				//-- we don't handle non-waypoints this way yet
			}
		}
	}
}

//----------------------------------------------------------------------

void CuiSetName::createNewInstance (Unicode::String const & params)
{
	CuiSetName * const setName = safe_cast<CuiSetName *>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::SetName, true));
	if (setName)
	{
		size_t const spacepos = params.find (' ');
		if (spacepos != std::string::npos)
		{
			NetworkId const id (Unicode::wideToNarrow (params.substr (0, spacepos)));
			setName->setObject       (id);
			setName->setQuestionText (params.substr (spacepos + 1));

			Object const * const obj = NetworkIdManager::getObjectById (id);
			ClientObject const * const clientObject = obj ? obj->asClientObject () : 0;
			if (clientObject)
			{
				setName->setAnswerText   (clientObject->getLocalizedName ());
			}
		}
	}
}

//======================================================================
