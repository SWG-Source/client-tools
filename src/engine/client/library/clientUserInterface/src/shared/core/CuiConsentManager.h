//======================================================================
//
// CuiConsentManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiConsentManager_H
#define INCLUDED_CuiConsentManager_H

//======================================================================

class CuiConsent;
class NetworkId;

//----------------------------------------------------------------------

class CuiConsentManager
{
public:
	typedef void (*CallbackFunction) (int id, bool response);

	static void              install ();
	static void              remove  ();

	static int               askLocalConsent(const Unicode::String& question, CallbackFunction);
	static void              showConsentWindow(const Unicode::String& question, int id, bool clientOnly);
	static void              handleResponse(int id, bool response);
	static void              dismissLocalConsentRequest(int id);
	static void              setWindowAtPlacement(CuiConsent*, int placement);

private:
	static stdmap<int, CallbackFunction>::fwd m_consentMap;
	static stdmap<int, CuiConsent*>::fwd m_widgetMap;
	static stdmap<int, CuiConsent*>::fwd m_placementMap;
	static int m_nextId;
};

//======================================================================

#endif
