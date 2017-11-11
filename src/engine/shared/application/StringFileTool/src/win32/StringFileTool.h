// ======================================================================
//
// StringFileTool.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_StringFileTool_H
#define INCLUDED_StringFileTool_H

// ======================================================================

class StringFileTool : public CWinApp
{
public:
	
	enum IconType
	{
		IT_equal,
		IT_notEqual,
		IT_left,
		IT_right,
		IT_merged,
		IT_conflict
	};

	enum ShowType
	{
		ST_all,
		ST_differences,
		ST_notEqual,
		ST_left,
		ST_right,
		ST_COUNT
	};

public:

	static char const * const * getShowNames();
	// Convert a CString to a Unicode::String
	static void convertToUnicodeString(CString string, Unicode::String& unicodeString);

public:

	StringFileTool();

	//{{AFX_VIRTUAL(StringFileTool)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(StringFileTool)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
