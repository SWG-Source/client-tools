// NetworkGetHostName.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#include "sharedNetwork/FirstSharedNetwork.h"

#include "sharedNetwork/Address.h"
#include "sharedNetwork/NetworkHandler.h"

#include <winsock.h>

//-----------------------------------------------------------------------

struct HN
{
	HN();
	std::string hostName;
};

//-----------------------------------------------------------------------

HN::HN()
{
	char name[512] = {"\0"};
	if(gethostname(name, sizeof(name)) == 0)
	{
		Address a(name, 0);
		hostName = a.getHostAddress();//name;
	}
}

//-----------------------------------------------------------------------

const std::string & NetworkHandler::getHostName()
{
	static HN hn;
	return hn.hostName;
}

//-----------------------------------------------------------------------

const std::vector<std::pair<std::string, std::string> > & NetworkHandler::getInterfaceAddresses()
{
	static std::vector<std::pair<std::string, std::string> > s;
	return s;
}

//-----------------------------------------------------------------------
