//======================================================================
//
// FormManagerClient.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/FormManagerClient.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/Exitchain.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedMath/Vector.h"

#include "clientGame/ClientCommandQueue.h"

#include <algorithm>
#include <map>

//======================================================================

namespace FormManagerClientNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const FormManagerClient::Messages::EditFormData::Payload &, FormManagerClient::Messages::EditFormData>
			s_editFormData;
	}

	Unicode::String const cs_space(Unicode::narrowToWide(" "));

	bool s_installed = false;
}

using namespace FormManagerClientNamespace;

//======================================================================

void FormManagerClient::install ()
{
	InstallTimer const installTimer("FormManagerClient::install");

	FormManager::install();

	DEBUG_FATAL (s_installed, ("already installed"));
	if(s_installed)
		return;

	s_installed = true;

	ExitChain::add(remove,"FormManagerClient::remove");
}

//----------------------------------------------------------------------

void FormManagerClient::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	if(!s_installed)
		return;

	s_installed = false;
}

//----------------------------------------------------------------------

void FormManagerClient::sendCreateObjectData(std::string const & templateName, Vector const & position, NetworkId const & cell, FormManager::UnpackedFormData const & dataMap)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	if(!s_installed)
		return;

	//param format is "<CREATE_OBJECT> <template> <x> <y> <z> <cell> <datamap>"

	char buf[1024];
	IGNORE_RETURN(snprintf(buf, sizeof(buf), "%d %s %f %f %f %s", FormManager::CREATE_OBJECT, templateName.c_str(), position.x, position.y, position.z, cell.getValueString().c_str()));
	Unicode::String params(Unicode::narrowToWide(buf));
	FormManager::PackedFormData const packedData = FormManager::packFormData(dataMap);
	params += cs_space + packedData;

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(FormManager::getFormCommandName(), NetworkId::cms_invalid, params));
}

//----------------------------------------------------------------------

void FormManagerClient::sendEditObjectData(NetworkId const & networkId, FormManager::UnpackedFormData const & dataMap)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	if(!s_installed)
		return;

	//param format is "<EDIT_OBJECT> <datamap>"

	char buf[1024];
	IGNORE_RETURN(snprintf(buf, sizeof(buf), "%d ", FormManager::EDIT_OBJECT));
	Unicode::String params(Unicode::narrowToWide(buf));
	FormManager::PackedFormData const packedData = FormManager::packFormData(dataMap);
	params += cs_space + packedData;

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(FormManager::getFormCommandName(), networkId, params));
}

//----------------------------------------------------------------------

void FormManagerClient::requestEditObject(NetworkId const & networkId)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	if(!s_installed)
		return;

	//param format is "<REQUEST_EDIT_OBJECT> <oid>"

	Unicode::String const space(Unicode::narrowToWide(" "));

	char buf[1024];
	IGNORE_RETURN(snprintf(buf, sizeof(buf), "%d", FormManager::REQUEST_EDIT_OBJECT));

	Unicode::String const params(Unicode::narrowToWide(buf));

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(FormManager::getFormCommandName(), networkId, params));
}

//----------------------------------------------------------------------

void FormManagerClient::handleReceiveEditObjectData(NetworkId const & objectToEdit, FormManager::UnpackedFormData const & dataMap)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	if(!s_installed)
		return;

	FormManager::PackedFormData const packedData = FormManager::packFormData(dataMap);
	Transceivers::s_editFormData.emitMessage(Messages::EditFormData::Payload (objectToEdit, packedData));
}

//======================================================================
