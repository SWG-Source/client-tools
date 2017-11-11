// ============================================================================
//
// NpcCuiMediatorFactorySetup.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ============================================================================

#include "FirstNpcEditor.h"
#include "NpcCuiMediatorFactorySetup.h"

#include "NpcCuiMediatorTypes.h"
#include "NpcCuiViewer.h"
#include "NpcCuiConsole.h"

#include "UIManager.h"
#include "UIPage.h"
#include "UIImage.h"
#include "UICanvas.h"
#include "UIData.h"

#include "clientUserInterface/CuiBackdrop.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMediatorFactory_Constructor.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/Production.h"
#include "sharedFoundation/ConfigFile.h"

#include <vector>

// ============================================================================

namespace NpcCuiMediatorFactorySetupNamespace
{
	bool s_installed;
	int addBackdrop (char const * source, char const * pageName);
	const UILowerString prop_backdropCount ("BackdropCount");
}

using namespace NpcCuiMediatorFactorySetupNamespace;

// ----------------------------------------------------------------------------

#define MAKE_NPC_CTOR(type,path) CuiMediatorFactory::addConstructor (CuiMediatorTypes::##type , new CuiMediatorFactory::Constructor <NpcCui##type        > (##path));

void NpcCuiMediatorFactorySetup::install () 
{
	DEBUG_FATAL (s_installed, ("already installed\n"));

#if PRODUCTION==0
	MAKE_NPC_CTOR (Console,                           "/Console");
#endif

	//-- setup the backgrounds
	{
		bool done = false;
		int maxBackdrops = 32;
		int i = 0;

		while(!done && i < maxBackdrops)
		{
			char const * backdrop = ConfigFile::getKeyString("NpcEditor", "backdrop", i++, 0);

			if(backdrop)
				IGNORE_RETURN(addBackdrop(backdrop, "/Back"));
			else
				done = true;
		}
	}

	CuiMediatorFactory::addConstructor (CuiMediatorTypes::Backdrop,
		new CuiMediatorFactory::Constructor <CuiBackdrop>("/Back"));

	MAKE_NPC_CTOR (Viewer, "/AvView");

	bool progressive = false;
	DebugFlags::registerFlag(progressive, "NpcEditor", "progressive");

	s_installed = true;
}

// ----------------------------------------------------------------------------

void NpcCuiMediatorFactorySetup::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed\n"));
	s_installed = false;
}

// ----------------------------------------------------------------------------

int NpcCuiMediatorFactorySetupNamespace::addBackdrop (char const * source, char const * pageName)
{
	int numBackdrops = 0;

	UIPage * const root = NON_NULL (UIManager::gUIManager ().GetRootPage ());
	NOT_NULL(root);

	UIPage * const page = NON_NULL (GET_UI_OBJ ((*root), UIPage, pageName));
	NOT_NULL(page);

	UIData * const codeData = GET_UI_OBJ((*page), UIData, "CodeData");
	NOT_NULL(codeData);

	if(!codeData->GetPropertyInteger (prop_backdropCount, numBackdrops))
		numBackdrops = 0;

	char buf [64];
	const size_t buf_size = sizeof (buf);

	IGNORE_RETURN(snprintf (buf, buf_size, "Backdrop%02d", numBackdrops));

	IGNORE_RETURN(codeData->SetPropertyInteger (prop_backdropCount, numBackdrops + 1));

	{
		UIImage * newImage = new UIImage();

		IGNORE_RETURN(newImage->SetSourceResource(UIUnicode::narrowToWide(source)));

		//-- scale the backdrop based on the size of the page
		IGNORE_RETURN(newImage->SetProperty(UIWidget::PropertyName::PackLocation, Unicode::narrowToWide("nfn,nfn")));
		IGNORE_RETURN(newImage->SetProperty(UIWidget::PropertyName::PackSize, Unicode::narrowToWide("a,a")));
		newImage->SetSize(page->GetSize());

		newImage->SetName(buf);
		newImage->SetVisible(false);

		IGNORE_RETURN(page->AddChild(newImage));
	} //lint !e429

	return numBackdrops;
}

// ============================================================================
