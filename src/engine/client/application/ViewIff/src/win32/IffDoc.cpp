//
// IffDataView.cpp
// asommers 6-12-99
//
// copyright 1999, bootprint entertainment
//

//-------------------------------------------------------------------

#include "FirstViewIff.h"
#include "viewiff.h"

#include "sharedFile/Iff.h"
#include "IffDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(IffDoc, CDocument)

BEGIN_MESSAGE_MAP(IffDoc, CDocument)
	//{{AFX_MSG_MAP(IffDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

IffDoc::IffDoc() :
	iffTreeList ()
{
}

IffDoc::~IffDoc()
{
	int i;
	for (i = 0; i < iffTreeList.getNumberOfElements (); i++)
		delete iffTreeList [i];
}

//-------------------------------------------------------------------

BOOL IffDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

//-------------------------------------------------------------------

void IffDoc::Serialize(CArchive& ar)
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
void IffDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void IffDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

BOOL IffDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

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
			Iff iff (lpszPathName);

			int n = iff.getNumberOfBlocksLeft ();

			int i;
			for (i = 0; i < n; i++)
			{
				IffNode* node = new IffNode;
				node->fill (iff);

				iffTreeList.add (node);
			}

			success = true;
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

IffDoc::IffNode::IffNode (void) :
	data (0),
	length (0),
	list (0),
	listCount (0),
	handle (0)
{
}

IffDoc::IffNode::~IffNode (void)
{
	delete [] data;
	data   = 0;
	length = 0;

	int i;
	for (i = 0; i < listCount; i++)
	{
		delete list [i];
		list [i] = 0;
	}

	delete [] list;
	list      = 0;
	listCount = 0;
}

//-------------------------------------------------------------------

void IffDoc::IffNode::fill (Iff& iff)
{
	//-- keep name
	nameTag = iff.getCurrentName ();
	ConvertTagToString (nameTag, name);

	//-- do we have a form or chunk?
	if (iff.isCurrentForm ())
	{
		//-- only the list of IffNodes will be valid
		iff.enterForm ();

			length    = 0;
			type      = T_form;

			data      = 0;

			listCount = iff.getNumberOfBlocksLeft ();
			list      = new IffNode* [listCount];

			int i;
			for (i = 0; i < listCount; i++)
			{
				list [i] = new IffNode ();
				list [i]->fill (iff);
			}

		iff.exitForm ();
	}
	else
		if (iff.isCurrentChunk ())
		{
			//-- only the chunk data will be valid
			iff.enterChunk ();

				type = T_chunk;

				length = iff.getChunkLengthTotal ();
				data   = iff.readRest_char ();

				list = 0;
				listCount = 0;

			iff.exitChunk ();
		}
		else
		{
			FATAL (true, ("not form or chunk"));
		}
}

//-------------------------------------------------------------------

void IffDoc::realFillTreeCtrl (CTreeCtrl& treeCtrl, IffDoc::IffNode* node, int currentLevel, HTREEITEM currentHandle)
{
	NOT_NULL (node);

	CString buffer;
	HTREEITEM handle;

	if (node->type == IffNode::T_chunk)
	{
		buffer.Format ("CHUNK %s [%04X (%i)]", node->name, node->length, node->length);
		handle = treeCtrl.InsertItem (buffer, currentHandle);
		treeCtrl.Expand (currentHandle, TVE_EXPAND);

		node->handle = handle;		
	}
	else
	{
		FATAL (!node->type == IffNode::T_form, ("not form or chunk!"));

		buffer.Format ("FORM %s [%04X (%i)]", node->name, node->length, node->length);
		handle = treeCtrl.InsertItem (buffer, currentHandle);
		treeCtrl.Expand (currentHandle, TVE_EXPAND);

		node->handle = handle;		

		int i;
		for (i = 0; i < node->listCount; i++)
			realFillTreeCtrl (treeCtrl, node->list [i], currentLevel+1, handle);
	}
}

void IffDoc::fillTreeCtrl (CTreeCtrl& treeCtrl)
{
	if (iffTreeList.getNumberOfElements ())
	{
		treeCtrl.DeleteAllItems ();

		int i;
		for (i = 0; i < iffTreeList.getNumberOfElements (); i++)
			realFillTreeCtrl (treeCtrl, iffTreeList [i], 0, TVI_ROOT);
	}
}

//-------------------------------------------------------------------

const IffDoc::IffNode* IffDoc::findNode (const IffDoc::IffNode* node, HTREEITEM handle) const
{
	if (handle == node->handle)
		return node;
	else
	{
		if (node->type == IffNode::T_form)
		{
			int i;
			for (i = 0; i < node->listCount; i++)
			{
				const IffNode* foundNode = findNode (node->list [i], handle);
				if (foundNode)
					return foundNode;
			}
		}
	}

	return 0;
}

const IffDoc::IffNode* IffDoc::findNode (HTREEITEM handle) const
{
	int i;
	for (i = 0; i < iffTreeList.getNumberOfElements (); i++)
	{
		const IffNode* node = findNode (iffTreeList [i], handle);

		if (node)
			return node;
	}

	return 0;
}

static void indent(int indention, FILE * file)
{
	for (int i = 0; i < indention; ++i)
		fputs("\t", file);
}

static bool isstring(uint8 * buffer, int max)
{
	int count = 0;

	for(int i=0; i<max; ++i)
	{
		int const c = buffer[i];

		if(isprint(c))
			count++;
		else if(count >= 4 && c == 0)
			return true;
		else
			return false;
	}

	return false;
}

void IffDoc::IffNode::dump (int indention, FILE * file) const
{
	int const maxBytes = 20;

	// indent

	if (type == T_form)
	{
		indent(indention, file);
		fprintf (file, "form \"%s\"\n", name);

		indent(indention, file);
		fprintf (file, "{\n");

		for (int i = 0; i < listCount; i++)
			list [i]->dump(indention+1, file);

		indent(indention, file);
		fprintf (file, "}\n");
	}
	else
	{
		int count = 0;
		uint8 const * start = reinterpret_cast<uint8*>(data);
		bool needNewLine = false;

		indent(indention, file);
		fprintf (file, "chunk \"%s\"\n", name);

		indent(indention, file);
		fprintf (file, "{\n");

		for (int i = 0; i < length; i++)
		{
			int const c = static_cast<int>(reinterpret_cast<uint8*>(data)[i]);

			if(i % maxBytes == 0 || needNewLine)
			{
				if(i || needNewLine)
					fprintf(file, "\n");
				indent(indention+1, file);
				fprintf (file, "uint8 ");
			}

			if(i % maxBytes && !needNewLine)
				fprintf(file, ",");
			fprintf (file, "0x%02x", c);

			//-- if this character is non-printable but the next character is
			//     part of a string, then a new line is needed.
			needNewLine = !isprint(c) && isstring(&reinterpret_cast<uint8*>(data)[i] + 1, length - i - 1);

			//-- keep a running counter of printable characters
			if(isprint(c))
				count++;

			//-- if there are 4 or more printable characters & they are null terminated
			//     print them as a string
			if(count >= 4 && c == 0)
			{
				fprintf(file, "\n");
				indent(indention+1, file);
				fprintf (file, "// cstring \"%s\"", start);

				needNewLine = true;
			}
			
			//-- reset counter & string start pointer on first non-printable character
			if(!isprint(c))
			{
				count = 0;
				start = &reinterpret_cast<uint8*>(data)[i] + 1;
			}
		}
		fprintf(file, "\n");

		indent(indention, file);
		fprintf (file, "}\n");
	}
}

void IffDoc::dump (char const * fileName) const
{
	FILE * file = fopen(fileName, "wt");

	if (file)
	{
		for (int i = 0; i < iffTreeList.getNumberOfElements (); i++)
			iffTreeList [i]->dump(0, file);

		fclose(file);
	}
}

//-------------------------------------------------------------------

