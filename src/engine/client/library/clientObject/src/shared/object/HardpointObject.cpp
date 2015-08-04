// ======================================================================
//
// HardpointObject.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/HardpointObject.h"

#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"

#ifdef _DEBUG
#include <set>
#include <string>
#endif

// ======================================================================

namespace HardpointObjectNamespace
{
#ifdef _DEBUG
	typedef std::set<std::pair<uint32, uint32> > StringPairSet;
	StringPairSet ms_warningSet;
#endif
}

using namespace HardpointObjectNamespace;

// ======================================================================
// STATIC PUBLIC HardpointObject
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL (HardpointObject, true, 0, 0, 0);

// ======================================================================
// PUBLIC HardpointObject
// ======================================================================

HardpointObject::HardpointObject () :
	Object (),
	m_hardpointName ()
#ifdef _DEBUG
	,
	m_missingHardpointWarning (false)
#endif
{
}

// ----------------------------------------------------------------------

HardpointObject::HardpointObject (const CrcString& hardpointName) :
	Object (),
	m_hardpointName (hardpointName)
#ifdef _DEBUG
	,
	m_missingHardpointWarning (false)
#endif
{
}

// ----------------------------------------------------------------------

HardpointObject::~HardpointObject ()
{
}

// ----------------------------------------------------------------------

float HardpointObject::alter (float elapsedTime)
{
	snapToPosition ();
	return Object::alter (elapsedTime);
}

//----------------------------------------------------------------------

void HardpointObject::snapToPosition ()
{
	//-- snap object position to parent's hardpoint position
	if (m_hardpointName.getString () && *m_hardpointName.getString ())
	{
		const Object* const object = getAttachedTo ();
		if (object)
		{
			const Appearance* const appearance = object->getAppearance ();
			if (appearance && appearance->isLoaded())
			{
				Transform hardpointTransform(Transform::IF_none);
				if (appearance->findHardpoint(m_hardpointName, hardpointTransform))
				{
					if (hardpointTransform != getTransform_o2p())
						setTransform_o2p (hardpointTransform);
				}
#ifdef _DEBUG
				else
				{
					if (!m_missingHardpointWarning)
					{
						m_missingHardpointWarning = true;

						AppearanceTemplate const * appearanceTemplate = appearance->getAppearanceTemplate();
						char const * templateName = "(appearance without template)";
						if (appearanceTemplate)
							templateName = appearanceTemplate->getName();
						if (templateName == NULL) 
							templateName = "(templateName is NULL)";
						if (templateName[0] == '\0') 
							templateName = "(templateName is blank)";
							
						std::pair<uint32, uint32> warning(Crc::calculate(templateName), Crc::calculate(m_hardpointName.getString()));
						if (ms_warningSet.find(warning) == ms_warningSet.end())
						{
							DEBUG_WARNING(true, ("HardpointObject::alter - could not find hardpoint %s for obj [%s] on appearance template %s", m_hardpointName.getString (), getDebugName(), templateName));
							ms_warningSet.insert(warning);
						}
					}
				}
#endif
			}
		}
	}
}

// ----------------------------------------------------------------------

void HardpointObject::setHardpointName (const CrcString& hardpointName)
{
	m_hardpointName.CrcString::set(hardpointName);
}

// ======================================================================
