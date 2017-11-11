// ======================================================================
//
// FirstTextureBuilder.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_FirstTextureBuilder_H
#define INCLUDED_FirstTextureBuilder_H

// ======================================================================

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ======================================================================

#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers
#define NOMINMAX

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcview.h>
#include <afxdtctl.h>       // MFC support for Internet Explorer 4 Common Controls

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/FirstSharedFoundation.h"

#include <memory>               // for std::auto_ptr

#define BOOST_SMART_PTR_CONVERSION
#include "boost/smart_ptr.hpp"  // for boost::shared_ptr and friends

// ======================================================================

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
//}}AFX_INSERT_LOCATION

#endif
