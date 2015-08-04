// ======================================================================
//
// SetupUi.cpp
// asommers 
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "_precompile.h"
#include "SetupUi.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIImageFragment.h"
#include "UIImageFrame.h"
#include "UIImageStyle.h"
#include "UIPacking.h"
#include "UIPage.h"
#include "UIRectangleStyle.h"
#include "UIScriptEngine.h"
#include "UIText.h"
#include "UIWidgetBoundaries.h"
#include "UIWidgetRectangleStyles.h"
#include "UiMemoryBlockManager.h"

#include <cassert>

// ======================================================================

namespace SetupUiNamespace
{
	bool ms_installed;
}

using namespace SetupUiNamespace;

// ======================================================================
// PUBLIC SetupUi::Data
// ======================================================================
	
SetupUi::Data::Data () :
	m_reportSetFlagsFunction (0),
	m_reportPrintfFunction (0),
  m_memoryManagerVerifyFunction (0),
	m_memoryBlockManagerDebugDumpOnRemove (false)
{
}

// ======================================================================
// STATIC PUBLIC SetupUi
// ======================================================================

void SetupUi::install (const SetupUi::Data& data)
{
	UI_ASSERT (!ms_installed);
	ms_installed = true;

	//-- install debug reporting mechanism
	UiReport::install (data.m_reportSetFlagsFunction, data.m_reportPrintfFunction, data.m_profilerEnterFunction, data.m_profilerLeaveFunction, data.m_memoryManagerVerifyFunction);
}

// ----------------------------------------------------------------------

void SetupUi::remove ()
{
	UI_ASSERT (ms_installed);
	ms_installed = false;
}

// ======================================================================
