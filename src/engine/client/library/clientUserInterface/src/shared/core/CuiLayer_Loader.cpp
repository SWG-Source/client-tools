//======================================================================
//
// CuiLayer_Loader.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiLayer_Loader.h"

#include "UIStandardLoader.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiGenericRenderer.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "clientUserInterface/CuiWidget3dObjectViewer.h"
#include "clientUserInterface/CuiWidget3dPaperdoll.h"
#include "clientUserInterface/CuiWidgetGroundRadar.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"

//======================================================================

namespace
{
	const UIStandardLoader<CuiWidget3dObjectViewer>        s_loader1;
	const UIStandardLoader<CuiWidget3dPaperdoll>           s_loader2;
	const UIStandardLoader<CuiWidgetGroundRadar>           s_loader3;
	const UIStandardLoader<CuiWidget3dObjectListViewer>    s_loader4;
	const UIStandardLoader<CuiGenericRenderer>             s_loader5;
}

//-------------------------------------------------------------------
//
// Loader
//-----------------------------------------------------------------

CuiLayer::Loader::Loader () :
UILoader ()
{
	AddToken (s_loader1.GetTypeName(), &s_loader1);
	AddToken (s_loader2.GetTypeName(), &s_loader2);
	AddToken (s_loader3.GetTypeName(), &s_loader3);
	AddToken (s_loader4.GetTypeName(), &s_loader4);
	AddToken (s_loader5.GetTypeName(), &s_loader5);
}

//----------------------------------------------------------------------

bool CuiLayer::Loader::LoadStringFromResource( const UINarrowString &ResourceName, UINarrowString &Out )
{
	const std::string filename = ConfigClientUserInterface::getUiRootPath () + ResourceName;
	AbstractFile * file = TreeFile::open (filename.c_str (), AbstractFile::PriorityData, false);
	if(!file)
		return false;
	
	int size = file->length();
	if (size == 0)
		return false;

	byte* buffer = file->readEntireFileAndClose();
	delete file;
	file = 0;
	
	IGNORE_RETURN (Out.assign (reinterpret_cast<char*>(buffer), static_cast<size_t> (size)));
	delete [] buffer;
	buffer = 0;
	
	return true;
}

//----------------------------------------------------------------------

void CuiLayer::Loader::install ()
{
	CuiWidget3dObjectListViewer::install ();
	ExitChain::add (CuiLayer::Loader::remove, "CuiLayer::Loader::remove");
}

//----------------------------------------------------------------------

void CuiLayer::Loader::remove ()
{

}

//======================================================================
