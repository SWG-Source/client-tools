//===================================================================
//
// WorldSnapshotViewerDoc.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "FirstWorldSnapshotViewer.h"
#include "WorldSnapshotDataView.h"

#include "Resource.h"
#include "sharedFile/Iff.h"
#include "sharedUtility/WorldSnapshotReaderWriter.h"
#include "WorldSnapshotViewerDoc.h"

//===================================================================

IMPLEMENT_DYNCREATE(WorldSnapshotViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(WorldSnapshotViewerDoc, CDocument)
	//{{AFX_MSG_MAP(WorldSnapshotViewerDoc)
	ON_COMMAND(ID_BUTTON_EXPORT, OnButtonExport)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//===================================================================

WorldSnapshotViewerDoc::WorldSnapshotViewerDoc() :
	m_worldSnapshotReader (new WorldSnapshotReaderWriter),
	m_selectedPosition_p (),
	m_selectedPosition_w (),
	m_numberOfNodes (0),
	m_totalNumberOfNodes (0)
{
}

//-------------------------------------------------------------------

WorldSnapshotViewerDoc::~WorldSnapshotViewerDoc()
{
	delete m_worldSnapshotReader;
}

//-------------------------------------------------------------------

BOOL WorldSnapshotViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

//-------------------------------------------------------------------

void WorldSnapshotViewerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	}
	else
	{
	}
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void WorldSnapshotViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void WorldSnapshotViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

BOOL WorldSnapshotViewerDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	m_worldSnapshotReader->clear ();

	bool success = false;

	//-- see if it's an Iff file
	FILE* infile = fopen (lpszPathName, "rt");

	if (infile &&
		getc (infile) == 'F' &&
		getc (infile) == 'O' &&
		getc (infile) == 'R' &&
		getc (infile) == 'M')
	{
		fclose (infile);

		FatalSetThrowExceptions (true);

		try
		{
			Iff iff;
			if (iff.open (lpszPathName, true))
			{
				m_worldSnapshotReader->load (lpszPathName);

				success = true;
			}
			else
				success = false;
		}
		catch (...)
		{
			success = false;
		}

		UpdateAllViews (0);
	}

	if (!success)
	{
		CString tmp;
		tmp.Format ("%s is not a valid .iff file, or its first block is not a FORM", lpszPathName);
		MessageBox (AfxGetApp ()->GetMainWnd ()->m_hWnd, tmp, "Error", MB_ICONEXCLAMATION);
	}

	return success;
}

//-------------------------------------------------------------------

void WorldSnapshotViewerDoc::setSelectedPosition_p (const Vector& selectedPosition_p)
{
	m_selectedPosition_p = selectedPosition_p;
}

//-------------------------------------------------------------------

const Vector& WorldSnapshotViewerDoc::getSelectedPosition_p () const
{
	return m_selectedPosition_p;
}

//-------------------------------------------------------------------

void WorldSnapshotViewerDoc::setSelectedPosition_w (const Vector& selectedPosition_w)
{
	m_selectedPosition_w = selectedPosition_w;
}

//-------------------------------------------------------------------

const Vector& WorldSnapshotViewerDoc::getSelectedPosition_w () const
{
	return m_selectedPosition_w;
}

//-------------------------------------------------------------------

void WorldSnapshotViewerDoc::setNumberOfNodes (int numberOfNodes)
{
	m_numberOfNodes = numberOfNodes;
}

//-------------------------------------------------------------------

int WorldSnapshotViewerDoc::getNumberOfNodes () const
{
	return m_numberOfNodes;
}

//-------------------------------------------------------------------

void WorldSnapshotViewerDoc::setTotalNumberOfNodes (int totalNumberOfNodes)
{
	m_totalNumberOfNodes = totalNumberOfNodes;
}

//-------------------------------------------------------------------

int WorldSnapshotViewerDoc::getTotalNumberOfNodes () const
{
	return m_totalNumberOfNodes;
}

//-------------------------------------------------------------------

void WorldSnapshotViewerDoc::OnButtonExport() 
{
	CString fileName = GetPathName ();

	int index = fileName.ReverseFind ('.');
	if (index != -1)
		fileName = fileName.Left (index);

	index = fileName.ReverseFind ('\\');
	if (index != -1)
		fileName = fileName.Right (fileName.GetLength () - 1 - index);

	if (m_worldSnapshotReader->getNumberOfNodes () > 0)
	{
		CStdioFile outfile;
		if (outfile.Open (".\\worldsnapshot.txt", CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite | CFile::typeText))
		{
			//-- seek to the end of the log file
			outfile.SeekToEnd ();

			outfile.WriteString ("WORLDSNAPSHOT\n");

			int i;
			for (i = 0; i < m_worldSnapshotReader->getNumberOfNodes (); ++i)
			{
				WorldSnapshotReaderWriter::Node const * const node = m_worldSnapshotReader->getNode (i);
				Vector const & position_w = node->getTransform_p ().getPosition_p ();

				CString buffer;
				buffer.Format ("B %i %1.0f %1.0f %1.0f %s\n", node->getNetworkIdInt (), position_w.x, position_w.y, position_w.z, fileName);
				outfile.WriteString (buffer);
			}
		}
	}
	else
		MessageBox (0, "There are no objects to export.", 0, MB_OK);
}

//===================================================================

