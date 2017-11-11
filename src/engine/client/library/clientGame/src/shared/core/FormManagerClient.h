//======================================================================
//
// FormManagerClient.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_FormManagerClient_H
#define INCLUDED_FormManagerClient_H

//======================================================================

#include "sharedGame/FormManager.h"

class CellProperty;
class NetworkId;
class Vector;

//----------------------------------------------------------------------

class FormManagerClient : public FormManager
{
public:

	struct Messages
	{
		struct EditFormData
		{
			typedef std::pair<NetworkId, Unicode::String > Payload;
		};
	};

	static void install ();
	static void remove ();
	static void sendCreateObjectData(std::string const & templateName, Vector const & position, NetworkId const & cell, FormManager::UnpackedFormData const & dataMap);
	static void sendEditObjectData(NetworkId const & objectToEdit, FormManager::UnpackedFormData const & dataMap);
	static void requestEditObject(NetworkId const & objectToEdit);
	static void handleReceiveEditObjectData(NetworkId const & objectToEdit, FormManager::UnpackedFormData const & dataMap);
};

//======================================================================

#endif
