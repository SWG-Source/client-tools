#ifndef __DPVSVERSION_HPP
#define __DPVSVERSION_HPP
/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2001 Hybrid Graphics, Ltd.
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Graphics, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Hybrid Graphics and legal action against the party in breach.
 *
 * Description: 	DPVS file version information
 *
 *		Sets up following definitions (these can be set by hand as well)
 *		that are used to fill in data for the file version information in
 *		the Win32 file-version resources that are compiled into each DLL.
 *
 *		DPVS_RCFILE_VERSION    - file version information for DLL modules
 *		DPVS_RCFILE_VERSIONSTR - string version of the above
 *		DPVS_RCFILE_PRODVER    - product version information for DLL modules
 *		DPVS_RCFILE_PRODVERSTR - string version of the above
 *		DPVS_RCFILE_PRODNAME   - Default product name for all DLL modules
 *		DPVS_RCFILE_COPYRIGHT  - Copyright line for all DLL modules
 *		DPVS_RCFILE_TRADEMARK  - Trademark line for all DLL modules
 *		DPVS_RCFILE_COMMENTS   - comment line for all DLL modules
 *		DPVS_RCFILE_COMPANY    - Company line for all DLL modules
 *
 *		DPVS_VERSION			- 8.8.8 fixed point macro
 *
 * $Archive: /dpvs/implementation/include/dpvsVersion.hpp $
 * $Author: wili $ 
 * $Revision: #11 $
 * $Modtime: 8.10.02 11:34 $
 * $Date: 2004/11/12 $
 * 
 ******************************************************************************/

//------------------------------------------------------------------------
// When you update the DPVS revision number you need to update both
// the three macro definitions below _and_ the DPVS_RCFILE_VERSIONSTR.
//------------------------------------------------------------------------

#define DPVS_VERSION_MAJOR	5
#define DPVS_VERSION_MINOR	11
#define DPVS_VERSION_REVIS  12
#define DPVS_RCFILE_VERSIONSTR		"5, 11, 12\0"

//------------------------------------------------------------------------
// NOTE: Resource compiler can't deal with parameterized macros, so these four can not use
// the DECLARE_DPVS_FILE_VERxxx() macros.
//------------------------------------------------------------------------

#define DPVS_RCFILE_VERSION		DPVS_VERSION_MAJOR,DPVS_VERSION_MINOR,DPVS_VERSION_REVIS
#define DPVS_RCFILE_PRODVER		DPVS_RCFILE_VERSION
#define DPVS_RCFILE_PRODVERSTR	DPVS_RCFILE_VERSIONSTR
#define DPVS_RCFILE_PRODNAME	"dPVS\0"
#define DPVS_RCFILE_COPYRIGHT	"Copyright © 1999-2004 Hybrid Graphics, Ltd.\0"
#define DPVS_COPYRIGHT			"Copyright (c) 1999-2004 Hybrid Graphics, Ltd. All Rights Reserved."
#define DPVS_BUILD_TIME			__DATE__ " " __TIME__
#define DPVS_RCFILE_TRADEMARK	"dPVS\0"
#define DPVS_RCFILE_COMMENTS	"For further information contact dpvs@hybrid.fi\0"
#define DPVS_RCFILE_COMPANY		"Hybrid Graphics, Ltd.\0"

#define DECLARE_DPVS_RCFILE_VERSTR(ma,mi,re) (#ma ", " #mi ", " #re "\0")
#define DECLARE_DPVS_RCFILE_VERHEX(ma,mi,re) ((ma<<16)+(mi<<8)+(re))

//------------------------------------------------------------------------
// current revision number in 8.8.8 fixed point
//------------------------------------------------------------------------

#define DPVS_FILE_VERSIONHEX DECLARE_DPVS_RCFILE_VERHEX(DPVS_VERSION_MAJOR,DPVS_VERSION_MINOR,DPVS_VERSION_REVIS)
#define DPVS_FILE_VERSIONSTR DECLARE_DPVS_RCFILE_VERSTR(DPVS_VERSION_MAJOR,DPVS_VERSION_MINOR,DPVS_VERSION_REVIS)

//------------------------------------------------------------------------
#endif // __DPVSVERSION_HPP

