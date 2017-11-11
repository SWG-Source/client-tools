#ifndef INC_NXN_INTEGRATORSDK_H
#define INC_NXN_INTEGRATORSDK_H

//---------------------------------------------------------------------------
//	library specific stuff
//---------------------------------------------------------------------------
#include ".\NxNVersion.h"

#ifndef BUILD_NXN_INTEGRATOR_SDK_DLL
	#pragma comment(lib, NXN_XDK_LIB_NAME)
#endif

//---------------------------------------------------------------------------
//	class forward declarations
//---------------------------------------------------------------------------
#include ".\NxNFwdDecl.h"
#include ".\NxNClassList.h"

#include ".\NxNErrors.h"
#include ".\NxNExport.h"

#include ".\NxNString.h"
#include ".\NxNArray.h"
#include ".\NxNParam.h"
#include ".\NxNPath.h"

#include ".\NxNObject.h"

#include ".\NxNCommand.h"
#include ".\NxNProperty.h"
#include ".\NxNType.h"
#include ".\NxNBrowseFilter.h"
#include ".\NxNNode.h"
#include ".\NxNIntegrator.h"
#include ".\NxNResponse.h"
#include ".\NxNMenu.h"
#include ".\NxNMapper.h"

#include ".\NxNVirtualNode.h"
#include ".\NxNDbNode.h"
#include ".\NxNNodeList.h"

#include ".\NxNDbNodeList.h"
#include ".\NxNFile.h"
#include ".\NxNFolder.h"
#include ".\NxNProject.h"
#include ".\NxNWorkspace.h"

#include ".\NxNGlobalSelection.h"

#include ".\NxNEventMsg.h"
#include ".\NxNEventTarget.h"
#include ".\NxNEventManager.h"

#include ".\NxNConstants.h"									// NxN COM constants

DECLARE_INTERNAL_CLASSLIST()

#endif // INC_NXN_INTEGRATORSDK_H