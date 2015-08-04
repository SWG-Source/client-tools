//======================================================================
//
// CuiSurveyManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiSurveyManager_H
#define INCLUDED_CuiSurveyManager_H

//======================================================================

class NetworkId;

//----------------------------------------------------------------------

class CuiSurveyManager
{
public:
	static void              install ();
	static void              remove  ();

	static void              doSurvey();
	static void              doGetSample();
	static void              setCurrentTool(const NetworkId& toolId);
	static const NetworkId&  getCurrentTool();
	static void              setDefaultResourceForTool(const NetworkId& toolId, const Unicode::String& resourceName);
	static Unicode::String   getDefaultResourceForTool(const NetworkId& toolId);

private:
	static NetworkId         m_currentTool;
	static stdmap<NetworkId, Unicode::String>::fwd m_toolToResourceMap;
};

//======================================================================

#endif
