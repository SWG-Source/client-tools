//======================================================================
//
// CuiResourceManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiResourceManager_H
#define INCLUDED_CuiResourceManager_H

//======================================================================

#include "Unicode.h"

class ClientInstallationSynchronizedUi;
class CachedNetworkId;
class ClientObject;
class InstallationObject;
class MessageQueueGenericResponse;
class NetworkId;

//----------------------------------------------------------------------

class CuiResourceManager
{
public:

	class Messages
	{
	public:
		class PoolsChanged;
		class EmptyCompleted;
	};

	typedef ClientInstallationSynchronizedUi Synchronized;

	static void                             install ();
	static void                             remove  ();

	static const CachedNetworkId &          getHarvesterId  ();
	static ClientObject *                   getHarvester    ();
	static InstallationObject *             getHarvesterInstallation ();
	static Synchronized *                   getSynchronized ();
	static void                             setHarvester    (ClientObject * obj);
	static void                             setHarvester    (const NetworkId & id);
	static void                             startListening   ();
	static void                             stopListening    ();

	static void                             emptyHopper (const NetworkId & resourceTypeId, int amount, bool discard, bool confirmed);
	static void                             dumpAll();

	static void                             requestResourceData ();

	static void                             receiveGenericResponse (const MessageQueueGenericResponse & resp);

	struct ResourceInfo
	{
		Unicode::String name;
		uint8           efficiency;

		ResourceInfo (const Unicode::String & theName, uint8 theEfficiency);
		ResourceInfo ();
	};

	typedef stdmap<NetworkId, ResourceInfo>::fwd ResourceInfoMap;
	static ResourceInfoMap &                 getResourceInfoMap ();

	static const ResourceInfo *              findResourceInfo (const NetworkId & resourceId);

};

//======================================================================

#endif
