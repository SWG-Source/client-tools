// ======================================================================
//
// FirstSwgDraftSchematicEditor.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_FirstSwgDraftSchematicEditor_H
#define INCLUDED_FirstSwgDraftSchematicEditor_H

// ======================================================================

#pragma warning (disable: 4018 4100 4245 4503 4786)

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcview.h>
#include <cassert>
#include <vector>

// ======================================================================

template <typename T, typename U>
T safe_cast(U u)
{
	if (!u)
		return 0;

	T t = dynamic_cast<T>(u);
	assert(t);
	return t;
}

// ----------------------------------------------------------------------

class PointerDeleter
{
public:
	template <typename PointerType>
	void operator ()(PointerType pointer) const
		{
			delete pointer;
		}
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
