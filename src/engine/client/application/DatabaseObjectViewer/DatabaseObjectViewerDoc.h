// ======================================================================
//
// DatabaseObjectViewerDoc.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DatabaseObjectViewerDoc_H
#define INCLUDED_DatabaseObjectViewerDoc_H

// ======================================================================

class DatabaseObjectViewerDoc : public CDocument
{
public:

	struct Node
	{
		CPoint m_point;
		bool   m_installation;
	};

	typedef std::vector<Node> NodeList;
	typedef std::map<std::string, NodeList> PlanetMap;
	typedef std::map<std::string, PlanetMap> ClusterMap;

public:

	ClusterMap const & getClusterMap () const;
	std::string const & getSelectedPlanet () const;

	void checkAllHosts ();
	void clearAllHosts ();
	void setSelectedPlanet (std::string const & planetName);
	bool getHostEnabled (std::string const & hostName) const;
	void setHostEnabled (std::string const & hostName, bool enabled);

protected: 

	DatabaseObjectViewerDoc();
	DECLARE_DYNCREATE(DatabaseObjectViewerDoc)

	bool load (char const * fileName, bool optimizeOnLoad);

public:

	//{{AFX_VIRTUAL(DatabaseObjectViewerDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

public:

	virtual ~DatabaseObjectViewerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//{{AFX_MSG(DatabaseObjectViewerDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	ClusterMap m_clusterMap;
	std::string m_selectedPlanet;

	typedef std::map<std::string, bool> HostEnabledMap;
	HostEnabledMap mutable m_hostEnabledMap;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
