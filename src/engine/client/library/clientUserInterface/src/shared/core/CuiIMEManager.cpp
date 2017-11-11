// ======================================================================
//
// CuiIMEManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ====================================================================== 

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiIMEManager.h"
#include "clientUserInterface/CuiMediator.h"
#include "clientUserInterface/CuiMediatorFactory.h"

#include "clientUserInterface/IMEManager.h"
#include "sharedFoundation/ExitChain.h"

// ======================================================================

namespace CuiIMEManagerNamespace
{
	CuiIMEManager * s_theManager = 0;

	void remove();

	void imeOpenStatusNotifyFunction(bool open);
}

using namespace CuiIMEManagerNamespace;

// ======================================================================

void CuiIMEManager::install()
{
	//@TODO This install is not matched correctly with the remove because there is no SetupClientUserInterface
	if(s_theManager)
		return;
	
	ExitChain::add(CuiIMEManagerNamespace::remove, "CuiIMEManagerNamespace::remove");

	s_theManager = new CuiIMEManager();

	IMEManager::SetIMEOpenStatusNotifyFunction(imeOpenStatusNotifyFunction);
}

// ----------------------------------------------------------------------

void CuiIMEManagerNamespace::remove()
{
	DEBUG_FATAL(!s_theManager, ("CuiIMEManager::remove: not installed"));

	delete s_theManager;
	s_theManager = 0;
}

// ----------------------------------------------------------------------

void CuiIMEManagerNamespace::imeOpenStatusNotifyFunction(bool open)
{
	CuiIMEManager *theManager = CuiIMEManager::GetCuiIMEManager();
	if(theManager)
		theManager->NotifyIMEOpenStatus(open);
}

// ----------------------------------------------------------------------

void CuiIMEManager::NotifyIMEOpenStatus(bool open)
{
	if (mIMEIndicatorType) 
	{
		if(open && GetEnabled())
			CuiMediatorFactory::activateInWorkspace (mIMEIndicatorType);
		else
			CuiMediatorFactory::deactivateInWorkspace (mIMEIndicatorType);
	}
}

// ----------------------------------------------------------------------

CuiIMEManager *CuiIMEManager::GetCuiIMEManager()
{
	DEBUG_FATAL(!s_theManager, ("CuiIMEManager::GetCuiIMEManager: not installed"));
	return s_theManager;
}

// ======================================================================

CuiIMEManager::CuiIMEManager() :
UIIMEManager(),
mIMEIndicatorType(0)
{
}

// ----------------------------------------------------------------------

CuiIMEManager::~CuiIMEManager()
{
}

// ----------------------------------------------------------------------

Unicode::String& CuiIMEManager::GetCompString() 
{ 
	return IMEManager::GetCompString(); 
}

// ----------------------------------------------------------------------

unsigned char * CuiIMEManager::GetCompStringAttrs() 
{ 
	return IMEManager::GetCompStringAttrs(); 
}

// ----------------------------------------------------------------------

unsigned int CuiIMEManager::GetCompCaratIndex() 
{ 
	return IMEManager::GetCompCaratIndex(); 
}

// ----------------------------------------------------------------------

CandidateList_t& CuiIMEManager::GetCandidateList() 
{ 
	return IMEManager::GetCandidateList(); 
}

// ----------------------------------------------------------------------

void CuiIMEManager::SetEnabled(bool const enabled) 
{ 
	IMEManager::SetEnabled(enabled); 
}

// ----------------------------------------------------------------------

void CuiIMEManager::SetContextWidget(UIWidget *contextWidget)
{ 
	IMEManager::SetContextWidget(contextWidget); 
}

// ----------------------------------------------------------------------

UIWidget *CuiIMEManager::GetContextWidget()
{ 
	return IMEManager::GetContextWidget(); 
}

// ----------------------------------------------------------------------

bool CuiIMEManager::GetEnabled() 
{ 
	return IMEManager::GetEnabled(); 
}

// ----------------------------------------------------------------------

int CuiIMEManager::GetSelectedCandidateIndex() 
{ 
	return IMEManager::GetSelectedCandidateIndex(); 
}

// ----------------------------------------------------------------------

int CuiIMEManager::GetSelectedCandidate() 
{ 
	return IMEManager::GetSelectedCandidate(); 
}

// ----------------------------------------------------------------------

int CuiIMEManager::GetNumCandidates() 
{ 
	return IMEManager::GetNumCandidates(); 
}

// ----------------------------------------------------------------------

void CuiIMEManager::SetMaxCandidatesPerPage(unsigned int const maxCandsPerPage) 
{ 
	IMEManager::SetMaxCandidatesPerPage(maxCandsPerPage); 
}

// ----------------------------------------------------------------------

void CuiIMEManager::ResendCharacter(unsigned short const ch) 
{ 
	IMEManager::ResendCharacter(ch); 
}

// ----------------------------------------------------------------------

void CuiIMEManager::SetConsumeEnter(bool const consume) 
{
	IMEManager::SetConsumeEnter(consume); 
}

// ----------------------------------------------------------------------

bool CuiIMEManager::GetConsumeEnter() 
{ 
	return IMEManager::GetConsumeEnter(); 
}

// ----------------------------------------------------------------------

bool CuiIMEManager::GetCandidateListOpen() 
{ 
	return IMEManager::GetCandidateListOpen(); 
}

//----------------------------------------------------------------------

void CuiIMEManager::SetIMEIndicator(CuiMediatorTypes::Type type)
{
	if (mIMEIndicatorType && (mIMEIndicatorType != type))
	{
		CuiMediatorFactory::deactivateInWorkspace(mIMEIndicatorType);
	}

	mIMEIndicatorType = type;
}

// ======================================================================

