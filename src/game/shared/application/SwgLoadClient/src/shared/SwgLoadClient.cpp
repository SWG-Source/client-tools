// SwgLoadClient.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstSwgLoadClient.h"
#include "Client.h"
#include "ConfigSwgLoadClient.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Timer.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedLog/Log.h"
#include "sharedRandom/Random.h"
#include "swgServerNetworkMessages/SetupSwgServerNetworkMessages.h"
#include "swgSharedNetworkMessages/SetupSwgSharedNetworkMessages.h"
#include "SwgLoadClient.h"
#include <string>

//-----------------------------------------------------------------------

SwgLoadClient::SwgLoadClient() :
clientCreateTimer(new Timer(ConfigSwgLoadClient::getClientCreateDelay())),
clients(),
done(false)
{
/*
	int loadCount = ConfigSwgLoadClient::getLoadCount();
	int i;
	char nameBuf[128] = {"\0"};
	for(i = 0; i < loadCount; ++i)
	{
		snprintf(nameBuf, sizeof(nameBuf), "loadClient_%d", i);
		std::string loginId(nameBuf);
		Client * c = new Client(loginId);
		c->login(ConfigSwgLoadClient::getLoginServerAddress(), ConfigSwgLoadClient::getLoginServerPort());
		clients.push_back(c);
		LOG("startup", ("Created client %s", nameBuf));
	}
*/
}

//-----------------------------------------------------------------------

SwgLoadClient::~SwgLoadClient()
{
	std::vector<Client *>::const_iterator i;
	for(i = clients.begin(); i != clients.end(); ++i)
	{
		Client * c = (*i);
		delete c;
	}
}

//-----------------------------------------------------------------------

SwgLoadClient & SwgLoadClient::instance()
{
	static SwgLoadClient c;
	return c;
}

//-----------------------------------------------------------------------

void SwgLoadClient::makeClient()
{
	char nameBuf[128] = {"\0"};
	Random::setSeed(clients.size());
	snprintf(nameBuf, sizeof(nameBuf), "%s_l%d_", ConfigSwgLoadClient::getNamePrefix(), clients.size());
	std::string loginId(nameBuf);
	for(int i = 0; i < 5; ++i)
	{
		char ch[2];
		ch[1] = 0;
		ch[0] = (char)Random::random(65, 90);
		loginId += ch;
	}
	Client * c = new Client(loginId);
	c->login(ConfigSwgLoadClient::getLoginServerAddress(), ConfigSwgLoadClient::getLoginServerPort());
	clients.push_back(c);
	LOG("startup", ("Created client %s", nameBuf));
}

//-----------------------------------------------------------------------

void SwgLoadClient::quit()
{
	instance().done = true;
}

//-----------------------------------------------------------------------

void SwgLoadClient::run()
{
	LOG("startup", ("SwgLoadClient starting"));
	SetupSharedNetworkMessages::install();
	SetupSwgSharedNetworkMessages::install();
	SetupSwgServerNetworkMessages::install();
	while(! instance().done)
	{
		instance().update();
	}
}

//-----------------------------------------------------------------------

void SwgLoadClient::update()
{
	Clock::update();
	NetworkHandler::update();
	NetworkHandler::dispatch();
	if (done)
		return;
	std::vector<Client *>::iterator i;

	int loadCount = ConfigSwgLoadClient::getLoadCount();
	bool needClients = true;
	if(needClients)
	{
		if(clientCreateTimer->updateZero(Clock::frameTime()))
		{
			if(loadCount > static_cast<int>(clients.size()))
			{
				makeClient();
			}
			else
			{
				needClients = false;
			}
		}
	}

	for(i = clients.begin(); i != clients.end(); ++i)
	{
		(*i)->update();
	}
	NetworkHandler::update();
	Os::sleep(1);
}

//-----------------------------------------------------------------------

