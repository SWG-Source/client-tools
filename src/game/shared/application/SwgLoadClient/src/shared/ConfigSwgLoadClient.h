// ConfigSwgLoadClient.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ConfigSwgLoadClient_H
#define	_INCLUDED_ConfigSwgLoadClient_H

//-----------------------------------------------------------------------

class ConfigSwgLoadClient
{
public:
	struct Data
	{
		float           chatEventTimerMax;
		float           chatEventTimerMin;
		float           clientCreateDelay;
		const char *    clusterName;
		const char *    creationLocation;
		int             loadCount;
		const char *    loginServerAddress;
		int             loginServerPort;
		const char *    namePrefix;
		bool            resetStart;
		float           socialEventTimerMax;
		float           socialEventTimerMin;
		float           startX;
		float           startZ;
		float           shipSpeed;
		float           shipLoiterRadius;
		bool            shipLoiterInCube;
		float           shipLoiterCenterX;
		float           shipLoiterCenterY;
		float           shipLoiterCenterZ;
		const char *    scriptSetupText;
	};



	static const float           getChatEventTimerMax   ();
	static const float           getChatEventTimerMin   ();
	static const float           getClientCreateDelay   ();
	static const char * const    getClusterName         ();
	static const char * const    getCreationLocation    ();
	static const int             getLoadCount           ();
	static const char * const    getLoginServerAddress  ();
	static const unsigned short  getLoginServerPort     ();
	static const char * const    getNamePrefix          ();
	static const bool            getResetStart          ();
	static const float           getSocialEventTimerMax ();
	static const float           getSocialEventTimerMin ();
	static const float           getStartX              ();
	static const float           getStartZ              ();
	static const float           getShipSpeed           ();
	static const float           getShipLoiterRadius    ();
	static const bool            getShipLoiterInCube    ();
	static const float           getShipLoiterCenterX   ();
	static const float           getShipLoiterCenterY   ();
	static const float           getShipLoiterCenterZ   ();
	static const char * const    getScriptSetupText     ();

	static void                  install                ();
	static void                  remove                 ();

private:
	static Data *	data;
};

//-----------------------------------------------------------------------

inline const float ConfigSwgLoadClient::getChatEventTimerMax()
{
	return data->chatEventTimerMax;
}

//-----------------------------------------------------------------------

inline const float ConfigSwgLoadClient::getChatEventTimerMin()
{
	return data->chatEventTimerMin;
}

//-----------------------------------------------------------------------

inline const float ConfigSwgLoadClient::getClientCreateDelay()
{
	return data->clientCreateDelay;
}

//-----------------------------------------------------------------------

inline const char * const ConfigSwgLoadClient::getClusterName()
{
	return data->clusterName;
}

//-----------------------------------------------------------------------

inline const char * const ConfigSwgLoadClient::getCreationLocation()
{
	return data->creationLocation;
}

//-----------------------------------------------------------------------

inline const int ConfigSwgLoadClient::getLoadCount()
{
	return data->loadCount;
}

//-----------------------------------------------------------------------

inline const char * const ConfigSwgLoadClient::getLoginServerAddress()
{
	return data->loginServerAddress;
}

//-----------------------------------------------------------------------

inline const unsigned short ConfigSwgLoadClient::getLoginServerPort()
{
	return (unsigned short)data->loginServerPort;
}

//-----------------------------------------------------------------------

inline const char * const ConfigSwgLoadClient::getNamePrefix()
{
	return data->namePrefix;
}

//-----------------------------------------------------------------------

inline const float ConfigSwgLoadClient::getSocialEventTimerMax()
{
	return data->socialEventTimerMax;
}

//-----------------------------------------------------------------------

inline const float ConfigSwgLoadClient::getSocialEventTimerMin()
{
	return data->socialEventTimerMin;
}

//-----------------------------------------------------------------------

inline const bool ConfigSwgLoadClient::getResetStart()
{
	return data->resetStart;
}

//-----------------------------------------------------------------------

inline const float ConfigSwgLoadClient::getStartX()
{
	return data->startX;
}

//-----------------------------------------------------------------------

inline const float ConfigSwgLoadClient::getStartZ()
{
	return data->startZ;
}

//-----------------------------------------------------------------------

inline const float ConfigSwgLoadClient::getShipSpeed()
{
	return data->shipSpeed;
}

//-----------------------------------------------------------------------

inline const float ConfigSwgLoadClient::getShipLoiterRadius()
{
	return data->shipLoiterRadius;
}

//-----------------------------------------------------------------------

inline const bool ConfigSwgLoadClient::getShipLoiterInCube()
{
	return data->shipLoiterInCube;
}

//-----------------------------------------------------------------------

inline const float ConfigSwgLoadClient::getShipLoiterCenterX()
{
	return data->shipLoiterCenterX;
}

//-----------------------------------------------------------------------

inline const float ConfigSwgLoadClient::getShipLoiterCenterY()
{
	return data->shipLoiterCenterY;
}

//-----------------------------------------------------------------------

inline const float ConfigSwgLoadClient::getShipLoiterCenterZ()
{
	return data->shipLoiterCenterZ;
}

//-----------------------------------------------------------------------

inline const char * const ConfigSwgLoadClient::getScriptSetupText()
{
	return data->scriptSetupText;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ConfigSwgLoadClient_H
