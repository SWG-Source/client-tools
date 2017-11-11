// ======================================================================
//
// ClientBuffBuilderManager.cpp
// copyright (c) 2006 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientBuffBuilderManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMessageDispatch/Transceiver.h"

// ======================================================================

namespace ClientBuffBuilderManagerNamespace
{
	class MyCallback : public MessageDispatch::Callback
	{
	public:
		MyCallback ()
		: MessageDispatch::Callback ()
		{
			
		}

	};

	bool ms_installed = false;
	MyCallback * s_callback = 0;
}

using namespace ClientBuffBuilderManagerNamespace;

// ======================================================================

void ClientBuffBuilderManager::install ()
{
	InstallTimer const installTimer("ClientBuffBuilderManager::install");

	DEBUG_FATAL(ms_installed, ("Already Installed"));
	ms_installed = true;
	s_callback = new MyCallback();
	ExitChain::add(ClientBuffBuilderManager::remove, "ClientBuffBuilderManager::remove", 0, false);
}

//----------------------------------------------------------------------

void ClientBuffBuilderManager::remove ()
{
	delete s_callback;
	s_callback = NULL;
	DEBUG_FATAL(!ms_installed, ("Not Installed"));
	ms_installed = false;
}

// =====================================================================



