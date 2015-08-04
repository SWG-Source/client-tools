#ifndef INC_NXN_FWDDECL_H
#define INC_NXN_FWDDECL_H

/*! \file		NxNFwdDecl.h
 *
 *  \brief		Collection of all class and type forward declarations.
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.0
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-08-May-2000 file created.
 *		[ap]-09-May-2001 Added type forward declaration section
 *		[ap]-09-May-2001 New type for 64 bit integers introduced "TNxNHugeInt".
 *	\endmod
 */

//---------------------------------------------------------------------------
//	class forward declarations
//---------------------------------------------------------------------------

// hierarchy level 1
class CNxNObject;

// hierarchy level 2
class CNxNSmartIntegrator;
class CNxNMapper;
class CNxNIntegrator;
class CNxNResponse;
class CNxNNode;
class CNxNProperty;
class CNxNCommand;
class CNxNBrowseFilter;
class CNxNMenu;
class CNxNEventManager;

// hierarchy level 3
class CNxNDbNode;
class CNxNVirtualNode;
class CNxNDiskItem;
class CNxNNodeList;

// hierarchy level 4
class CNxNWorkspace;
class CNxNProject;
class CNxNFolder;
class CNxNFile;
class CNxNDbNodeList;
class CNxNVersionControlItem;

// hierarchy level 5
class CNxNGlobalSelection;
class CNxNDiskFolder;
class CNxNDiskFile;
class CNxNDiskItemList;
class CNxNHistory;
class CNxNHistoryLabel;
class CNxNHistoryVersion;

// parameter classes
class CNxNBSTR;
class CNxNString;
class CNxNPath;
class CNxNType;
class CNxNEvent;

// interface classes
class CNxNEventTarget;

//---------------------------------------------------------------------------
//	structure forward declarations
//---------------------------------------------------------------------------
struct CNxNIterator;

//---------------------------------------------------------------------------
//	type forward declarations
//---------------------------------------------------------------------------
typedef __int64                 TNxNHugeInt;

#endif // INC_NXN_FWDDECL_H
