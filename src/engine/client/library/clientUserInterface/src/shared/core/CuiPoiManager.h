//======================================================================
//
// CuiPoiManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiPoiManager_H
#define INCLUDED_CuiPoiManager_H

//======================================================================

class ClientWaypointObject;

//----------------------------------------------------------------------

class CuiPoiManager
{
public:
	static void          install ();
	static void          remove  ();
	static void          reloadData();
	static const stdvector<ClientWaypointObject*>::fwd & getData();

private:
	static void          clearData();
	static void          loadData();

private:
	static stdvector<ClientWaypointObject*>::fwd ms_poiWps;
};

//======================================================================

#endif
