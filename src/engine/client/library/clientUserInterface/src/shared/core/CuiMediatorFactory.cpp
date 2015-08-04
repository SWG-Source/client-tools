// ======================================================================
//
// CuiMediatorFactory.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMediatorFactory_Constructor.h"

#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediator.h"
#include "clientUserInterface/CuiMediatorTypes.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "sharedFoundation/PointerDeleter.h"
#include <algorithm>
#include <map>
#include <vector>

// ======================================================================

namespace
{
	class MediatorCompareNoCase
	{
	public:
		bool operator()( const std::string & a, const std::string & b ) const { return _stricmp( a.c_str (), b.c_str () ) < 0; };
	};
	
	typedef std::map<std::string, CuiMediatorFactory::ConstructorBase *, MediatorCompareNoCase>  MediatorMap;
	MediatorMap * s_mediators;
	bool          s_deleting = false;

	typedef std::map<std::string, CuiMediator *> MediatorsActivatedMap;
	MediatorsActivatedMap * s_mediatorActivatedThisFrame;
}

namespace CuiMediatorNamespace
{
	extern void install();
	extern void remove();
}

//----------------------------------------------------------------------

bool CuiMediatorFactory::ms_installed = false;

//-----------------------------------------------------------------

void CuiMediatorFactory::install ()
{
	DEBUG_FATAL (ms_installed, ("already installed\n"));
	s_mediators = new MediatorMap;
	s_mediatorActivatedThisFrame = new MediatorsActivatedMap;
	ms_installed = true;

	CuiMediatorNamespace::install();
}

//-----------------------------------------------------------------

void          CuiMediatorFactory::deleteMediators ()
{
	s_deleting = true;	
	{
		for (MediatorMap::const_iterator it = s_mediators->begin (); it != s_mediators->end (); ++it)
		{
			if (!(*it).second)
				continue;
			
			(*it).second->detachMediator ();
		}
	}
	
	s_deleting = false;
}

//----------------------------------------------------------------------

void CuiMediatorFactory::remove ()
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	CuiMediatorNamespace::remove();

	deleteMediators ();

	//-- delete the constructors

	std::for_each (s_mediators->begin (), s_mediators->end (), PointerDeleterPairSecond ());
	delete s_mediators;
	s_mediators = 0;

	delete s_mediatorActivatedThisFrame;
	s_mediatorActivatedThisFrame = 0;

	ms_installed = false;
}

//----------------------------------------------------------------------

CuiMediatorFactory::ConstructorBase * CuiMediatorFactory::findConstructor (const char * namePtr)
{
	NOT_NULL (namePtr);
	DEBUG_FATAL (!ms_installed, ("not installed\n"));
	NOT_NULL (s_mediators);

	const MediatorMap::iterator fit = s_mediators->find (namePtr);

	if (fit != s_mediators->end ())
		return ((*fit).second);

#if _DEBUG
	for (MediatorMap::const_iterator it = s_mediators->begin (); it != s_mediators->end (); ++it)
		DEBUG_REPORT_LOG (true, ("0x%08x '%s'\n", &(*it).first, (*it).first.c_str ()));
	WARNING (true, ("No such mediator for name ptr 0x%08x, '%s' found among %d mediators", namePtr, namePtr, s_mediators->size ()));
#endif

	return 0;
}

//----------------------------------------------------------------------

CuiMediator * CuiMediatorFactory::create      (const char * namePtr, UIPage & page)
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));
	if (s_deleting)
		return 0;

	ConstructorBase * const ctor = findConstructor (namePtr);

	return ctor ? ctor->create (page) : 0;
}

//-----------------------------------------------------------------

CuiMediator * CuiMediatorFactory::get            (const char * namePtr, bool create)
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));
	if (s_deleting)
		return 0;

	ConstructorBase * const ctor = findConstructor (namePtr);

	if (ctor)
	{
		if (!ctor->isDuplicateOnly ())
			return ctor->get (create);
		else
			return getInWorkspace (namePtr, create);
	}

	return 0;
}

//-----------------------------------------------------------------

bool          CuiMediatorFactory::isValidName (const char * namePtr)
{
	NOT_NULL (namePtr);
	DEBUG_FATAL (!ms_installed, ("not installed\n"));
	NOT_NULL (s_mediators);
	return  s_mediators->find (namePtr) != s_mediators->end ();
}

//-----------------------------------------------------------------

CuiMediator *          CuiMediatorFactory::activate       (const char * namePtr, const char * previousMediatorNamePtr, bool create)
{
	NOT_NULL (namePtr);
	UNREF (previousMediatorNamePtr);
	DEBUG_FATAL (!ms_installed, ("not installed\n"));
	NOT_NULL (s_mediators);

	ConstructorBase * const ctor = findConstructor (namePtr);

	if (ctor && ctor->isDuplicateOnly ())
		return activateInWorkspace (namePtr);

	CuiMediator * const mediator = get (namePtr, create);
	if (mediator)
	{
		// @todo fix previous menu id
		mediator->setPreviousMediator (previousMediatorNamePtr);
		mediator->activate ();
		// @todo raise to top
		CuiManager::raiseToTop ();

		CuiMediator * const console = CuiMediatorFactory::get (CuiMediatorTypes::Console, false);

		if (console && console->isActive ())
			console->getPage ().SetFocus ();

		CuiMessageBox::ensureFocus  ();
	}

	return mediator;
}

//-----------------------------------------------------------------

bool  CuiMediatorFactory::deactivate     (const char * namePtr, bool const create)
{
	NOT_NULL (namePtr);
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	ConstructorBase * const ctor = findConstructor (namePtr);

	if (ctor && ctor->isDuplicateOnly ())
		return deactivateInWorkspace (namePtr) != 0;

	CuiMediator * const mediator = get (namePtr, create);
	if (mediator)
	{
		mediator->deactivate ();
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool CuiMediatorFactory::toggle                    (const char * namePtr)
{
	CuiMediator const * const mediator = CuiMediatorFactory::get (namePtr, false);
	if (mediator && mediator->isActive ())
	{
		CuiMediatorFactory::deactivate (namePtr);
		return false;
	}
	else
	{
		CuiMediatorFactory::activate (namePtr);
		return true;
	}
}

//-----------------------------------------------------------------

void CuiMediatorFactory::deactivateAll ()
{
	for (MediatorMap::const_iterator it = s_mediators->begin (); it != s_mediators->end (); ++it)
	{
		ConstructorBase * const ctor = (*it).second;

		if (!ctor->isDuplicateOnly ())
		{
			CuiMediator * const med = ctor->get (false);
			if (med)
				med->deactivate ();
		}
	}
}

//----------------------------------------------------------------------

bool CuiMediatorFactory::attemptRelease (const char * namePtr)
{
	NOT_NULL (namePtr);

	const MediatorMap::iterator fit = s_mediators->find (namePtr);
	
	if (fit != s_mediators->end ())
	{
		ConstructorBase * const ctor = (*fit).second;
		if (ctor)
		{
			CuiMediator * const mediator = ctor->get (false);
			if (mediator)
			{
				mediator->deactivate ();				
			}
			ctor->detachMediator ();
			return true;
		}	
	}
	else
		WARNING (true, ("CuiMediatorFactory::attemptRelease failed [%s]", namePtr));

	return false;
}

//-----------------------------------------------------------------

void CuiMediatorFactory::addConstructor (const char * namePtr, ConstructorBase * constructor)
{
	NOT_NULL (namePtr);
	DEBUG_FATAL (!ms_installed, ("not installed\n"));
	NOT_NULL (s_mediators);
	
	MEM_OWN (constructor);

	DEBUG_FATAL (isValidName (namePtr), ("don't add twice.\n"));
	std::pair<MediatorMap::iterator, bool> retval = s_mediators->insert (std::make_pair (std::string (namePtr), constructor));

	DEBUG_FATAL (!retval.second, ("could not add.\n"));
//	DEBUG_REPORT_LOG (true, ("Added mediator 0x%08x '%s' from 0x%08x\n", (*retval.first).first, (*retval.first).first, namePtr));
}

//-----------------------------------------------------------------

void  CuiMediatorFactory::getMediatorNames (stdvector<std::string>::fwd & result)
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));
	NOT_NULL (s_mediators);

	for (MediatorMap::const_iterator it = s_mediators->begin (); it != s_mediators->end (); ++it)
		result.push_back ((*it).first);
}

//-----------------------------------------------------------------

void CuiMediatorFactory::getMediators (stdvector<CuiMediator *>::fwd & result)
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	if (!s_mediators)
		return;

	for (MediatorMap::const_iterator it = s_mediators->begin (); it != s_mediators->end (); ++it)
	{
		CuiMediator * const med = (*it).second->get (false);
		if (med)
			result.push_back (med);
	}
}

//-----------------------------------------------------------------

bool CuiMediatorFactory::findMediatorCanonicalName (const std::string & name, std::string & canonicalName)
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	const size_t abbrev_len = name.size ();

	for (MediatorMap::const_iterator it = s_mediators->begin (); it != s_mediators->end (); ++it)
	{
		if (!_strnicmp (name.c_str (), (*it).first.c_str (), abbrev_len))
		{
			IGNORE_RETURN (canonicalName.assign ((*it).first));
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

CuiMediator * CuiMediatorFactory::activateInWorkspace       (const char * namePtr, bool ignoreExisting, bool settingsAutoSizeLocation)
{
	const MediatorsActivatedMap::iterator fit = s_mediatorActivatedThisFrame->find (namePtr);

	if (fit != s_mediatorActivatedThisFrame->end ())
		return ((*fit).second);
	
	CuiWorkspace * const ws = CuiWorkspace::getGameWorkspace ();
	NOT_NULL (ws);
	CuiMediator * const mediator = getInWorkspace (namePtr, true, ignoreExisting, settingsAutoSizeLocation);
	NOT_NULL (mediator);

	if (ws->hasMediator (*mediator) && !mediator->isOpenNextFrame())
		mediator->open ();
	else
		mediator->openNextFrame ();

	s_mediatorActivatedThisFrame->insert(std::make_pair(std::string (namePtr), mediator));
	return mediator;
}

//----------------------------------------------------------------------

CuiMediator * CuiMediatorFactory::deactivateInWorkspace     (const char * namePtr)
{
	const MediatorsActivatedMap::iterator fit = s_mediatorActivatedThisFrame->find (namePtr);

	if (fit != s_mediatorActivatedThisFrame->end ())
	{
		s_mediatorActivatedThisFrame->erase(fit);
	}
	CuiWorkspace * const ws = CuiWorkspace::getGameWorkspace ();
	NOT_NULL (ws);
	CuiMediator * const mediator = getInWorkspace (namePtr, false);
	if (mediator)
		mediator->closeNextFrame ();

	return mediator;
}

//----------------------------------------------------------------------

CuiMediator * CuiMediatorFactory::toggleInWorkspace         (const char * namePtr)
{
	CuiWorkspace * const ws = CuiWorkspace::getGameWorkspace ();
	NOT_NULL (ws);
	CuiMediator * const mediator = getInWorkspace (namePtr, false);
	if (mediator && mediator->isActive ())
		return deactivateInWorkspace (namePtr);
	else
		return activateInWorkspace (namePtr);
}

//----------------------------------------------------------------------

CuiMediator * CuiMediatorFactory::getInWorkspace            (const char * namePtr, bool create, bool ignoreExisting, bool settingsAutoSizeLocation)
{
	CuiWorkspace * const ws = CuiWorkspace::getGameWorkspace ();
	if (!ws)
	{
		DEBUG_FATAL (create, ("CuiMediatorFactory::getInWorkspace [%s] create but no workspace", namePtr)); 
		return 0;
	}

	ConstructorBase * const ctor = findConstructor (namePtr);
	
	if (ctor)
	{
		CuiMediator * mediator = 0;
		
		if (!ignoreExisting)
		{
			mediator = ws->findMediatorByType (ctor->getTypeId ());
			if (mediator)
				return mediator;
		}

		if (create)
		{
			mediator = ctor->createInto (ws->getPage ());
			ws->getPage ().RemoveChild (&mediator->getPage ());
			NOT_NULL (mediator);
			if (settingsAutoSizeLocation)
				mediator->setSettingsAutoSizeLocation (true, true);
		}

		return mediator;
	}

	return 0;
}

//----------------------------------------------------------------------

bool          CuiMediatorFactory::test (std::string & result)
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));

	bool error = false;

	UNREF (result);

	std::vector<std::string> names;

	getMediatorNames (names);

	for (std::vector<std::string>::const_iterator it = names.begin (); it != names.end (); ++it)
	{
		CuiMediator * const mediator = get ((*it).c_str ());
		NOT_NULL (mediator);

		mediator->activate ();

		if (mediator->test (result))
			return true;

		mediator->deactivate ();
	}

	deleteMediators ();

	return error;
}

//----------------------------------------------------------------------

void CuiMediatorFactory::clearActivatedThisFrame()
{
	s_mediatorActivatedThisFrame->clear();
}

// ======================================================================
