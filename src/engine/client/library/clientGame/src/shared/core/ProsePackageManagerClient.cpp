//======================================================================
//
// ProsePackageManagerClient.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ProsePackageManagerClient.h"

#include "clientUserInterface/CuiStringGrammarManager.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedGame/ProsePackage.h"
#include "clientGame/ClientObject.h"
#include "sharedGame/OutOfBand.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedObject/NetworkIdManager.h"

//======================================================================

bool ProsePackageManagerClient::appendTranslation (const ProsePackage & pp, Unicode::String & result)
{
	CuiStringVariablesData cvar;
	cvar.source = safe_cast<ClientObject *>(NetworkIdManager::getObjectById(pp.actor.id ));
	cvar.target = safe_cast<ClientObject *>(NetworkIdManager::getObjectById(pp.target.id));
	cvar.other  = safe_cast<ClientObject *>(NetworkIdManager::getObjectById(pp.other.id ));

	if (!pp.actor.str.empty ())
		cvar.sourceName = pp.actor.str;
	else if (pp.actor.stringId.isValid ())
		cvar.sourceName = pp.actor.stringId.localize ();

	if (!pp.target.str.empty ())
		cvar.targetName = pp.target.str;
	else if (pp.target.stringId.isValid ())
		cvar.targetName = pp.target.stringId.localize ();

	if (!pp.other.str.empty ())
		cvar.otherName = pp.other.str;
	else if (pp.other.stringId.isValid ())
		cvar.otherName = pp.other.stringId.localize ();
	
	cvar.digit_i = pp.digitInteger;
	cvar.digit_f = pp.digitFloat;
	
	Unicode::String raw;
	
	if (pp.complexGrammar)
	{
		Unicode::String raw;
		if (!CuiStringGrammarManager::getRawMessage (pp.stringId, cvar.source, cvar.target, raw))
		{
			WARNING (true, ("ProsePackageManagerClient unable to find appropriate string"));
			return false;
		}
	}
	else
	{
		raw = pp.stringId.localize ();
	}
	
	Unicode::String localResult;
	CuiStringVariablesManager::process (raw, cvar, localResult);
	result += localResult;

	return true;
}

//----------------------------------------------------------------------

bool ProsePackageManagerClient::setTranslation (const ProsePackage & pp, Unicode::String & result)
{
	CuiStringVariablesData cvar;
	cvar.source = safe_cast<ClientObject *>(NetworkIdManager::getObjectById(pp.actor.id ));
	cvar.target = safe_cast<ClientObject *>(NetworkIdManager::getObjectById(pp.target.id));
	cvar.other  = safe_cast<ClientObject *>(NetworkIdManager::getObjectById(pp.other.id ));

	if (!pp.actor.str.empty ())
		cvar.sourceName = pp.actor.str;
	else if (pp.actor.stringId.isValid ())
		cvar.sourceName = pp.actor.stringId.localize ();

	if (!pp.target.str.empty ())
		cvar.targetName = pp.target.str;
	else if (pp.target.stringId.isValid ())
		cvar.targetName = pp.target.stringId.localize ();

	if (!pp.other.str.empty ())
		cvar.otherName = pp.other.str;
	else if (pp.other.stringId.isValid ())
		cvar.otherName = pp.other.stringId.localize ();
	
	cvar.digit_i = pp.digitInteger;
	cvar.digit_f = pp.digitFloat;
	
	Unicode::String raw;
	
	if (pp.complexGrammar)
	{
		Unicode::String raw;
		if (!CuiStringGrammarManager::getRawMessage (pp.stringId, cvar.source, cvar.target, raw))
		{
			WARNING (true, ("ProsePackageManagerClient unable to find appropriate string"));
			return false;
		}
	}
	else
	{
		raw = pp.stringId.localize ();
	}
	
	CuiStringVariablesManager::process (raw, cvar, result);

	return true;
}

//----------------------------------------------------------------------

int  ProsePackageManagerClient::appendAllProsePackages (const Unicode::String & oob, Unicode::String & result, int positionStart, int positionEnd)
{
	int count = 0;
	typedef OutOfBand<ProsePackage> OutOfBandProsePackage;
	
	const OutOfBandPackager::OutOfBandBaseVector & oobv = OutOfBandPackager::unpack (oob, OutOfBandPackager::OT_prosePackage);
	const OutOfBandPackager::OutOfBandBaseVector::const_iterator end = oobv.end ();
	
	for (OutOfBandPackager::OutOfBandBaseVector::const_iterator it = oobv.begin (); it != end; ++it)
	{
		OutOfBandBase * const oobBase = *it;

		const int pos = oobBase->getPosition ();

		const OutOfBandProsePackage * const oobpp = safe_cast <const OutOfBandProsePackage *>(*it);
		const ProsePackage * const pp = oobpp->getObject ();

		//-- skip this entry, it is not in the desired range
		if (positionStart < positionEnd && (pos < positionStart || pos >= positionEnd))
		{
		}
		else
		{
			if (!result.empty ())
				result.append (1, '\n');
			
			ProsePackageManagerClient::appendTranslation (*pp, result);
			
			++count;
		}

		delete pp;		
		delete oobBase;
	}

	return count;
}

//======================================================================
