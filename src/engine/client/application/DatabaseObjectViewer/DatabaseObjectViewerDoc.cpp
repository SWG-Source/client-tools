// ======================================================================
//
// DatabaseObjectViewerDoc.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "StdAfx.h"
#include "DatabaseObjectViewerDoc.h"

#include "DatabaseObjectViewer.h"
#include "MainFrame.h"

#include <algorithm>

// ======================================================================

IMPLEMENT_DYNCREATE(DatabaseObjectViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(DatabaseObjectViewerDoc, CDocument)
	//{{AFX_MSG_MAP(DatabaseObjectViewerDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

DatabaseObjectViewerDoc::DatabaseObjectViewerDoc() :
	m_clusterMap (),
	m_selectedPlanet ()
{
	// TODO: add one-time construction code here

}

// ----------------------------------------------------------------------

DatabaseObjectViewerDoc::~DatabaseObjectViewerDoc()
{
}

// ----------------------------------------------------------------------

BOOL DatabaseObjectViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

// ----------------------------------------------------------------------

void DatabaseObjectViewerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void DatabaseObjectViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

// ----------------------------------------------------------------------

void DatabaseObjectViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

BOOL DatabaseObjectViewerDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	load ("worldsnapshot.txt", false);

	return load (lpszPathName, true);
}

bool DatabaseObjectViewerDoc::load (char const * const fileName, bool optimizeOnLoad)
{
	bool shouldOptimizeOnLoad = dynamic_cast<MainFrame const *> (AfxGetApp ()->GetMainWnd ())->getOptimizeOnLoad ();

	//-- open the config file
	CStdioFile infile;
	if (!infile.Open (fileName, CFile::modeRead | CFile::typeText))
		return FALSE;

	CStdioFile * outfile = 0;
	if (optimizeOnLoad && shouldOptimizeOnLoad)
	{
		outfile = new CStdioFile;
		if (!outfile->Open (CString (fileName) + "_optimized", CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		{
			delete outfile;
			outfile = 0;
		}
	}

	bool validHostNameFound = false;
	bool validPlanetNameFound = false;

	char type [1024];
	char networkIdBuffer [1024];
	float x;
	float y;
	float z;
	char planetNameBuffer [1024];

	std::string currentHostName;

	//-- read each line...
	CString line;
	while (infile.ReadString (line))
	{
		line.TrimLeft ();
		line.TrimRight ();
		if (line.GetLength () == 0)
			continue;

		if (line.Find ("rows selected") != -1)
			continue;

		//-- start parsing parameters
		sscanf (line, "%s %s %f %f %f %s", type, networkIdBuffer, &x, &y, &z, planetNameBuffer);

		//-- update currentCluster
		std::string possibleHostName (type);
		std::transform (possibleHostName.begin (), possibleHostName.end (), possibleHostName.begin (), tolower);
		if (DatabaseObjectViewerApp::isValidHostName (possibleHostName))
		{
			currentHostName = possibleHostName;

			OutputDebugString (possibleHostName.c_str ());
			OutputDebugString ("\n");

			validHostNameFound = true;

			if (outfile)
				outfile->WriteString (line + "\n");
		}
		else
		{
			if (currentHostName.length () == 0)
				continue;

			std::string possiblePlanetName (planetNameBuffer);
			std::transform (possiblePlanetName.begin (), possiblePlanetName.end (), possiblePlanetName.begin (), tolower);
			if (DatabaseObjectViewerApp::isValidPlanetName (planetNameBuffer))
			{
				ClusterMap::iterator clusterMapIter = m_clusterMap.find (currentHostName);
				if (clusterMapIter == m_clusterMap.end ())
				{
					PlanetMap planetMap;
					clusterMapIter = m_clusterMap.insert (std::make_pair (currentHostName, planetMap)).first;
				}

				PlanetMap & planetMap = clusterMapIter->second;
				PlanetMap::iterator planetMapIter = planetMap.find (possiblePlanetName);
				if (planetMapIter == planetMap.end ())
				{
					NodeList nodeList;
					planetMapIter = planetMap.insert (std::make_pair (possiblePlanetName, nodeList)).first;
				}

				NodeList & nodeList = planetMapIter->second;

				Node node;
				node.m_point.x = static_cast<int> (x);
				node.m_point.y = static_cast<int> (z);
				node.m_installation = type [0] == 'I' || type [0] == 'i';
				nodeList.push_back (node);

				validPlanetNameFound = true;

				if (outfile)
				{
					CString buffer;
					buffer.Format ("%s %s %1.0f %1.0f %1.0f %s\n", type, networkIdBuffer, x, y, z, planetNameBuffer);
					outfile->WriteString (buffer);
				}
			}
			else
			{
				OutputDebugString ("BAD LINE: ");
				OutputDebugString (line);
				OutputDebugString ("\n");
			}
		}
	}

	if (outfile)
	{
		delete outfile;
		outfile = 0;
	}

	if (!validHostNameFound || !validPlanetNameFound)
	{
		CString temp;
		temp.Format ("%s is not a valid DatabaseObjectViewer file", fileName);
		MessageBox (0, temp, "Error!", MB_OK);

		return false;
	}

	DatabaseObjectViewerApp::PlanetNameSet const & planetNameSet = dynamic_cast<DatabaseObjectViewerApp const *> (AfxGetApp ())->getPlanetNameSet ();
	setSelectedPlanet (*planetNameSet.begin ());

	checkAllHosts ();

	return true;
}

// ----------------------------------------------------------------------

void DatabaseObjectViewerDoc::checkAllHosts ()
{
	DatabaseObjectViewerApp::HostNameClusterNameMap const & hostNameClusterNameMap = dynamic_cast<DatabaseObjectViewerApp const *> (AfxGetApp ())->getHostNameClusterNameMap ();
	DatabaseObjectViewerApp::HostNameClusterNameMap::const_iterator end = hostNameClusterNameMap.end ();
	for (DatabaseObjectViewerApp::HostNameClusterNameMap::const_iterator iter = hostNameClusterNameMap.begin (); iter != end; ++iter)
		setHostEnabled (iter->first, true);
}

// ----------------------------------------------------------------------

void DatabaseObjectViewerDoc::clearAllHosts ()
{
	m_hostEnabledMap.clear ();
}

// ----------------------------------------------------------------------

void DatabaseObjectViewerDoc::setSelectedPlanet (std::string const & planetName)
{
	m_selectedPlanet = planetName;
}

// ----------------------------------------------------------------------

bool DatabaseObjectViewerDoc::getHostEnabled (std::string const & hostName) const
{
	return m_hostEnabledMap [hostName];
}

// ----------------------------------------------------------------------

void DatabaseObjectViewerDoc::setHostEnabled (std::string const & hostName, bool enabled)
{
	m_hostEnabledMap [hostName] = enabled;
}

// ----------------------------------------------------------------------

DatabaseObjectViewerDoc::ClusterMap const & DatabaseObjectViewerDoc::getClusterMap () const
{
	return m_clusterMap;
}

// ----------------------------------------------------------------------

std::string const & DatabaseObjectViewerDoc::getSelectedPlanet () const
{
	return m_selectedPlanet;
}

// ======================================================================
