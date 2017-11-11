// ======================================================================
//
// ComponentAppearanceTemplate.cpp
// Copyright 2000 - 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/ComponentAppearanceTemplate.h"

#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/ComponentAppearance.h"
#include "clientObject/MeshAppearanceTemplate.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedUtility/FileName.h"

#include "dpvsModel.hpp"

#include <unordered_set>

//===================================================================

bool ComponentAppearanceTemplate::ms_installed;

//===================================================================

const Tag TAG_CMPA = TAG (C,M,P,A);
const Tag TAG_PART = TAG (P,A,R,T);
const Tag TAG_RADR = TAG (R,A,D,R);

//===================================================================

void ComponentAppearanceTemplate::install (void)
{
	DEBUG_FATAL (ms_installed, ("ComponentAppearanceTemplate already installed"));
	ms_installed = true;

	//-- add binding 
	AppearanceTemplateList::assignBinding (TAG_CMPA, create);

	ExitChain::add (remove, "ComponentAppearanceTemplate::remove");
}

// ----------------------------------------------------------------------

void ComponentAppearanceTemplate::remove(void)
{
	DEBUG_FATAL (!ms_installed, ("ComponentAppearanceTemplate not installed"));
	ms_installed = false;

	//-- remove binding
	AppearanceTemplateList::removeBinding (TAG_CMPA);
}

// ----------------------------------------------------------------------

AppearanceTemplate* ComponentAppearanceTemplate::create (const char* filename, Iff* iff)
{
	return new ComponentAppearanceTemplate (filename, iff);
}

//===================================================================

ComponentAppearanceTemplate::ComponentAppearanceTemplate (const char* filename, Iff* iff) : 
	AppearanceTemplate (filename),
	m_componentList (NON_NULL (new ComponentList)),
	m_dpvsTestShape (0),
	m_radarShape (0)
{
	NOT_NULL(iff);
	load (*iff);
}

//-------------------------------------------------------------------

ComponentAppearanceTemplate::~ComponentAppearanceTemplate (void)
{
	int i;
	for (i = 0; i < getNumberOfComponents (); i++)
		AppearanceTemplateList::release (getAppearanceTemplate (i));

	m_componentList->clear ();
	delete m_componentList;
	m_componentList = 0;

	if (m_dpvsTestShape)
	{
		IGNORE_RETURN (m_dpvsTestShape->release ());
		m_dpvsTestShape = 0;
	}

	if (m_radarShape)
	{
		delete m_radarShape;
		m_radarShape = 0;
	}
}

//-------------------------------------------------------------------

Appearance* ComponentAppearanceTemplate::createAppearance () const
{
	return new ComponentAppearance (this);
}

//-------------------------------------------------------------------

void ComponentAppearanceTemplate::preloadAssets() const
{
	AppearanceTemplate::preloadAssets();

	int const numberOfComponents = getNumberOfComponents ();
	for (int i = 0; i < numberOfComponents; ++i)
		getAppearanceTemplate (i)->preloadAssets ();
}

//-------------------------------------------------------------------

void ComponentAppearanceTemplate::loadParts( Iff & iff )
{
	while(iff.getNumberOfBlocksLeft())
	{
		iff.enterChunk (TAG_PART);

			char * name = iff.read_string();

			Component component;
			component.appearanceTemplate = AppearanceTemplateList::fetch (FileName (FileName::P_appearance, name));
			component.transform = iff.read_floatTransform ();
			m_componentList->push_back (component);

			delete [] name;

		iff.exitChunk (TAG_PART);
	}
}

// ----------------------------------------------------------------------

void ComponentAppearanceTemplate::loadParts_old(Iff & iff)
{
	while(iff.getNumberOfBlocksLeft ())
	{
		iff.enterChunk (TAG_PART);

			char * name = iff.read_string();

			Component component;
			component.appearanceTemplate = AppearanceTemplateList::fetch (FileName (FileName::P_appearance, name));
			component.transform.setPosition_p (iff.read_floatVector ());
			component.transform.yaw_l   (convertDegreesToRadians (iff.read_float ()));
			component.transform.pitch_l (convertDegreesToRadians (iff.read_float ()));
			component.transform.roll_l  (convertDegreesToRadians (iff.read_float ()));

			m_componentList->push_back (component);

			delete [] name;

		iff.exitChunk (TAG_PART);
	}
}
// ----------------------------------------------------------------------

void ComponentAppearanceTemplate::load ( Iff & iff )
{
	iff.enterForm (TAG_CMPA);

#ifdef _DEBUG
		DEBUG_WARNING (DataLint::isEnabled () && iff.getCurrentName () < TAG_0005, ("ComponentAppearanceTemplate [%s] loading older version -- this asset will need to be reexported", getCrcName ().getString ()));
#endif

		switch (iff.getCurrentName())
		{
			case TAG_0001:
				load_0001 (iff);
				break;

			case TAG_0002:
				load_0002 (iff);
				break;

			case TAG_0003:
				load_0003 (iff);
				break;

			case TAG_0004:
				load_0004 (iff);
				break;

			case TAG_0005:
				load_0005 (iff);
				break;

			default:
				FATAL (true, ("ComponentAppearanceTemplate::load - unsupported format"));
				break;
		}

	iff.exitForm (TAG_CMPA);
}

//-------------------------------------------------------------------

void ComponentAppearanceTemplate::load_0001 ( Iff & iff )
{
	iff.enterForm (TAG_0001);

		loadParts_old(iff);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void ComponentAppearanceTemplate::load_0002 ( Iff & iff )
{
	iff.enterForm (TAG_0002);

		loadParts(iff);

	iff.exitForm (TAG_0002);
}

// ----------------------------------------------------------------------
// Version 3 - added 4-12-2002

void ComponentAppearanceTemplate::load_0003 ( Iff & iff )
{
	iff.enterForm(TAG_0003);

		AppearanceTemplate::load(iff);
		loadParts(iff);

	iff.exitForm(TAG_0003);

}

// ----------------------------------------------------------------------
// Version 4 - added 11-13-2002

void ComponentAppearanceTemplate::load_0004 ( Iff & iff )
{
	iff.enterForm(TAG_0004);

		AppearanceTemplate::load(iff);
		loadParts(iff);

		//-- Create the DPVS test shape from the bounding geometry in
		//   the AppearanceTemplate.  Version 0004 is the first version where
		//   this data is valid.
		createTestShapeFromTemplate ();

	iff.exitForm(TAG_0004);
}

// ----------------------------------------------------------------------

void ComponentAppearanceTemplate::load_0005 (Iff& iff)
{
	iff.enterForm (TAG_0005);

		AppearanceTemplate::load (iff);
		loadRadarShape (iff);
		loadParts (iff);
		createTestShapeFromTemplate ();

	iff.exitForm(TAG_0005);
}

//-------------------------------------------------------------------

int ComponentAppearanceTemplate::getNumberOfComponents (void) const
{
	return static_cast<int> (m_componentList->size ());
}

//-------------------------------------------------------------------

const ComponentAppearanceTemplate::Component& ComponentAppearanceTemplate::getComponent (int componentIndex) const
{
	DEBUG_FATAL (componentIndex < 0 || componentIndex >= getNumberOfComponents (), ("componentIndex out of range"));
	return (*m_componentList) [static_cast<uint> (componentIndex)];
}

//-------------------------------------------------------------------

const AppearanceTemplate* ComponentAppearanceTemplate::getAppearanceTemplate (int componentIndex) const
{
	return getComponent (componentIndex).appearanceTemplate;
}

//-------------------------------------------------------------------

const Transform& ComponentAppearanceTemplate::getTransform (int componentIndex) const
{
	return getComponent (componentIndex).transform;
}

// ----------------------------------------------------------------------

DPVS::Model *ComponentAppearanceTemplate::fetchDpvsTestShape() const
{
	if (m_dpvsTestShape)
	{
		//-- fetch reference for caller.
		m_dpvsTestShape->addReference();
	}

	return m_dpvsTestShape;
}

// ----------------------------------------------------------------------

void ComponentAppearanceTemplate::setDpvsTestShape(DPVS::Model *testShape)
{
	//-- Check for assignment to self.
	if (testShape == m_dpvsTestShape)
		return;

	//-- Bump up ref count on new test shape.
	if (testShape)
		testShape->addReference();

	//-- Reduce ref count on old test shape.
	if (m_dpvsTestShape)
		IGNORE_RETURN (m_dpvsTestShape->release());

	//-- Do the assignment.
	m_dpvsTestShape = testShape;
}

// ----------------------------------------------------------------------

void ComponentAppearanceTemplate::createTestShapeFromTemplate ()
{
	// We should not try to call this function if we've already set the DPVS object.
	IS_NULL (m_dpvsTestShape);

	// create the dpvs test shape
	const Extent *extent = getExtent();
	if (extent)
	{
		const BoxExtent *boxExtent = dynamic_cast<const BoxExtent*>(extent);
		if (boxExtent)
			m_dpvsTestShape = RenderWorld::fetchBoxModel(boxExtent->getBox());
	}
}

// ----------------------------------------------------------------------

const IndexedTriangleList* ComponentAppearanceTemplate::getRadarShape () const
{
	return m_radarShape;
}

//-------------------------------------------------------------------

void ComponentAppearanceTemplate::loadRadarShape (Iff& iff)
{
	iff.enterForm (TAG_RADR);

		iff.enterChunk (TAG_INFO);

			bool hasRadarShape = iff.read_int32 () != 0;

		iff.exitChunk (TAG_INFO);
		
		if (hasRadarShape)
			m_radarShape = new IndexedTriangleList (iff);
		
	iff.exitForm (TAG_RADR);
}

// ======================================================================
