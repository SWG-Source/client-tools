// ======================================================================
//
// DatabaseObjectViewer.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DatabaseObjectViewer_H
#define INCLUDED_DatabaseObjectViewer_H

// ======================================================================

#include "Resource.h"

class DatabaseObjectViewerDoc;

// ======================================================================

class DatabaseObjectViewerApp : public CWinApp
{
public:

	typedef std::set<std::string> PlanetNameSet;
	typedef std::map<std::string, std::string> HostNameClusterNameMap;

public:

	static bool isValidPlanetName (std::string const & planetName);
	static bool isValidHostName (std::string const & hostName);

public:

	DatabaseObjectViewerApp();

	PlanetNameSet const & getPlanetNameSet () const;
	HostNameClusterNameMap const & getHostNameClusterNameMap () const;

	DatabaseObjectViewerDoc * getDocument ();
	DatabaseObjectViewerDoc const * getDocument () const;

	//{{AFX_VIRTUAL(DatabaseObjectViewerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(DatabaseObjectViewerApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void loadConfigFile ();

private:

	PlanetNameSet m_planetNameSet;
	HostNameClusterNameMap m_hostNameClusterNameMap;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

