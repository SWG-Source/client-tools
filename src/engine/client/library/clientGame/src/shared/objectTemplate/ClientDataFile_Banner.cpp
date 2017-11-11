// ======================================================================
//
// ClientDataFile_Banner.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientDataFile_Banner.h"

#include "clientGame/ClientWorld.h"
#include "clientGame/BannerController.h"
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

namespace ClientDataFileBannerNamespace
{
	Tag const TAG_APPR = TAG(A,P,P,R);
	Tag const TAG_BANR = TAG(B,A,N,R);
	Tag const TAG_HPNT = TAG(H,P,N,T);
	Tag const TAG_LIFT = TAG(L,I,F,T);
	Tag const TAG_MASP = TAG(M,A,S,P);
	Tag const TAG_WSCL = TAG(W,S,C,L);

	float const cs_defaultLiftRadiansPerUnitSpeed = convertDegreesToRadians(90.0f) / 10.0f; // yield 90 degrees of x-axis yaw for 10 meters-per-second wind.
}

using namespace ClientDataFileBannerNamespace;

// ======================================================================
// class Banner: PUBLIC STATIC
// ======================================================================

ClientDataFile::Banner *ClientDataFile::Banner::load_0000(Iff &iff, char const *filename)
{
	return new Banner(iff, filename);
}

// ======================================================================
// class Banner: PUBLIC
// ======================================================================

ClientDataFile::Banner::~Banner()
{
	if (m_bannerAppearanceTemplate)
	{
		AppearanceTemplateList::release(m_bannerAppearanceTemplate);
		m_bannerAppearanceTemplate = 0;
	}
}

// ----------------------------------------------------------------------

void ClientDataFile::Banner::apply(Object *parentObject) const
{
	if (!parentObject)
		return;

	CustomizationData *customizationData        = NULL;
	CustomizationDataProperty *const cdProperty = dynamic_cast<CustomizationDataProperty*>(parentObject->getProperty(CustomizationDataProperty::getClassPropertyId()));
	if (cdProperty)
		customizationData = cdProperty->fetchCustomizationData();

	//-- Create the banner Object.
	// @todo convert to movement directly to the hardpoint since the hardpoint
	//       position likely doesn't change.  !!NOTE!! asynchronously loaded appearances,
	//       such as the base of the banner, do not have hardpoint information available
	//       at this point so I'm keeping it a hardpoint object for now.
	//
	//       Note: hardpoint object can't yaw, we need to add another object that is a child of the hardpoint object.
	HardpointObject *const bannerParentObject = new HardpointObject(m_hardpointName);
	NOT_NULL(bannerParentObject);

	parentObject->addChildObject_o(bannerParentObject);

	MemoryBlockManagedObject *const bannerObject = new MemoryBlockManagedObject();

	//-- Create and attach the appearance.
	Appearance *bannerAppearance = 0;
	if (m_bannerAppearanceTemplate)
		bannerAppearance = m_bannerAppearanceTemplate->createAppearance();
	else
		bannerAppearance = AppearanceTemplateList::createAppearance(m_bannerAppearanceTemplateName.getString());

	bannerObject->setAppearance(bannerAppearance);

	//-- Set customization data for appearance to be the same as the parent object's.  This allows customizations provided for the parent to be passed on to the flag/banner.
	if (customizationData)
	{
		bannerAppearance->setCustomizationData(customizationData);
		customizationData->release();
	}

	//-- Create and attach the controller.
	bannerObject->setController(new BannerController(bannerObject, m_windSpeedScale, m_liftRadiansPerUnitSpeed, m_maxAnimationSpeed));

	//-- Setup notifications for banner object.
	RenderWorld::addObjectNotifications(*bannerObject);

	//-- Attach banner object to parent.
	bannerParentObject->addChildObject_o(bannerObject);
}

// ----------------------------------------------------------------------

void ClientDataFile::Banner::preloadAssets() const
{
	//-- Remember old template.
	AppearanceTemplate const *oldTemplate = m_bannerAppearanceTemplate;

	//-- Preload template.
	m_bannerAppearanceTemplate = AppearanceTemplateList::fetch(m_bannerAppearanceTemplateName.getString());

	//-- Release old template.
	if (oldTemplate)
		AppearanceTemplateList::release(oldTemplate);
}

// ======================================================================
// class Banner: PRIVATE
// ======================================================================

ClientDataFile::Banner::Banner(Iff &iff, char const *filename) :
	m_bannerAppearanceTemplateName(),
	m_bannerAppearanceTemplate(0),
	m_hardpointName("banner", false),
	m_windSpeedScale(1.0f),
	m_liftRadiansPerUnitSpeed(cs_defaultLiftRadiansPerUnitSpeed),
	m_maxAnimationSpeed(10.0f)
{
	UNREF(filename);

	iff.enterForm(TAG_BANR);

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
							m_bannerAppearanceTemplateName.set(buffer, true);

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

				case TAG_LIFT:
					{
						iff.enterChunk(TAG_LIFT);

							m_liftRadiansPerUnitSpeed = convertDegreesToRadians(iff.read_float());

						iff.exitChunk(TAG_LIFT);
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
						DEBUG_FATAL(true, ("ClientDataFile::Banner unsupported data block with name [%s] in client data file [%s].", tagName, filename));
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

	iff.exitForm(TAG_BANR);
}

// ======================================================================
