// ======================================================================
//
// ClientDataFile_Flag.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientDataFile_Flag.h"

#include "clientGame/ClientWorld.h"
#include "clientGame/FlagController.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/HardpointObject.h"
#include "sharedFile/Iff.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/MemoryBlockManagedObject.h"

// ======================================================================

namespace ClientDataFileFlagNamespace
{
	Tag const TAG_APPR = TAG(A,P,P,R);
	Tag const TAG_FLAG = TAG(F,L,A,G);
	Tag const TAG_HPNT = TAG(H,P,N,T);
	Tag const TAG_MASP = TAG(M,A,S,P);
	Tag const TAG_WSCL = TAG(W,S,C,L);
}

using namespace ClientDataFileFlagNamespace;

// ======================================================================
// class Flag: PUBLIC STATIC
// ======================================================================

ClientDataFile::Flag *ClientDataFile::Flag::load_0000(Iff &iff, char const *filename)
{
	return new Flag(iff, filename);
}

// ======================================================================
// class Flag: PUBLIC
// ======================================================================

ClientDataFile::Flag::~Flag()
{
	if (m_flagAppearanceTemplate)
	{
		AppearanceTemplateList::release(m_flagAppearanceTemplate);
		m_flagAppearanceTemplate = 0;
	}
}

// ----------------------------------------------------------------------

void ClientDataFile::Flag::apply(Object *parentObject) const
{
	if (!parentObject)
		return;

	CustomizationData *customizationData        = NULL;
	CustomizationDataProperty *const cdProperty = dynamic_cast<CustomizationDataProperty*>(parentObject->getProperty(CustomizationDataProperty::getClassPropertyId()));
	if (cdProperty)
		customizationData = cdProperty->fetchCustomizationData();

	//-- Create the flag Object.
	// @todo convert to movement directly to the hardpoint since the hardpoint
	//       position likely doesn't change.  !!NOTE!! asynchronously loaded appearances,
	//       such as the base of the flag, do not have hardpoint information available
	//       at this point so I'm keeping it a hardpoint object for now.
	//
	//       Note: hardpoint object can't yaw, we need to add another object that is a child of the hardpoint object.
	HardpointObject *const flagParentObject = new HardpointObject(m_hardpointName);
	NOT_NULL(flagParentObject);

	parentObject->addChildObject_o(flagParentObject);

	MemoryBlockManagedObject *const flagObject = new MemoryBlockManagedObject();

	//-- Create and attach the appearance.
	Appearance *flagAppearance = 0;
	if (m_flagAppearanceTemplate)
		flagAppearance = m_flagAppearanceTemplate->createAppearance();
	else
		flagAppearance = AppearanceTemplateList::createAppearance(m_flagAppearanceTemplateName.getString());

	flagObject->setAppearance(flagAppearance);

	//-- Set customization data for appearance to be the same as the parent object's.  This allows customizations provided for the parent to be passed on to the flag/banner.
	if (customizationData)
	{
		flagAppearance->setCustomizationData(customizationData);
		customizationData->release();
	}

	//-- Create and attach the controller.
	flagObject->setController(new FlagController(flagObject, m_windSpeedScale, m_maxAnimationSpeed));

	//-- Setup notifications for flag object.
	RenderWorld::addObjectNotifications(*flagObject);

	//-- Attach flag object to parent.
	flagParentObject->addChildObject_o(flagObject);
}

// ----------------------------------------------------------------------

void ClientDataFile::Flag::preloadAssets() const
{
	//-- Remember old template.
	AppearanceTemplate const *oldTemplate = m_flagAppearanceTemplate;

	//-- Preload template.
	m_flagAppearanceTemplate = AppearanceTemplateList::fetch(m_flagAppearanceTemplateName.getString());

	//-- Release old template.
	if (oldTemplate)
		AppearanceTemplateList::release(oldTemplate);
}

// ======================================================================
// class Flag: PRIVATE
// ======================================================================

ClientDataFile::Flag::Flag(Iff &iff, char const *filename) :
	m_flagAppearanceTemplateName(),
	m_flagAppearanceTemplate(0),
	m_hardpointName("flag", false),
	m_windSpeedScale(1.0f),
	m_maxAnimationSpeed(10.0f)
{
	UNREF(filename);

	iff.enterForm(TAG_FLAG);

		char buffer[MAX_PATH];

		while (!iff.atEndOfForm())
		{
			Tag const currentTag = iff.getCurrentName();
			switch (currentTag)
			{
				case TAG_APPR:
					{
						iff.enterChunk(TAG_APPR);

							iff.read_string(buffer, sizeof(buffer) - 1);
							m_flagAppearanceTemplateName.set(buffer, true);

						iff.exitChunk(TAG_APPR);
					}
					break;

				case TAG_HPNT:
					{
						iff.enterChunk(TAG_HPNT);
				
							iff.read_string(buffer, sizeof(buffer) - 1);
							m_hardpointName.set(buffer, true);

						iff.exitChunk(TAG_HPNT);
					}
					break;

				case TAG_WSCL:
					{
						iff.enterChunk(TAG_WSCL);

							m_windSpeedScale = iff.read_float();

						iff.exitChunk(TAG_WSCL);
					}
					break;

				case TAG_MASP:
					{
						iff.enterChunk(TAG_MASP);

							m_maxAnimationSpeed = iff.read_float();

						iff.exitChunk(TAG_MASP);
					}
					break;

				default:
					{
#ifdef _DEBUG
						char tagName[5];
						ConvertTagToString(currentTag, tagName);
						DEBUG_FATAL(true, ("ClientDataFile::Flag unsupported data block with name [%s] in client data file [%s].", tagName, filename));
#endif
						if (iff.isCurrentForm ()) //lint !e527 // unreachable // reachable in release.
						{
							iff.enterForm ();
							iff.exitForm (true);
						}
						else
						{
							iff.enterChunk ();
							iff.exitChunk (true);
						}
					}
			}
		}

	iff.exitForm(TAG_FLAG);
}

// ======================================================================
