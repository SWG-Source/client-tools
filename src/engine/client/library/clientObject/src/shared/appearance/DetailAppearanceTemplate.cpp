//===================================================================
//
// DetailAppearanceTemplate.cpp
//
// Copyright 2000-2002 Sony Online Entertainment
// All Rights Reserved.
//
//===================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/DetailAppearanceTemplate.h"

#include "clientGraphics/RenderWorld.h"
#include "clientObject/ConfigClientObject.h"
#include "clientObject/DetailAppearance.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/ExtentList.h"
#include "sharedDebug/DataLint.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedUtility/FileName.h"
#include "sharedMath/IndexedTriangleList.h"

#include "dpvsModel.hpp"

#include <vector>
#include <algorithm>

//===================================================================

const Tag TAG_CHLD = TAG (C,H,L,D);
const Tag TAG_DTLA = TAG (D,T,L,A);
const Tag TAG_PIVT = TAG (P,I,V,T);
const Tag TAG_QUAD = TAG (Q,U,A,D);
const Tag TAG_RADR = TAG (R,A,D,R);
const Tag TAG_SPRT = TAG (S,P,R,T);
const Tag TAG_TEST = TAG (T,E,S,T);
const Tag TAG_WRIT = TAG (W,R,I,T);

namespace DetailAppearanceTemplateNamespace
{
#if _DEBUG
	bool ms_debugReport;
#endif

	bool ms_viewer;
	bool ms_skipL0;
}

using namespace DetailAppearanceTemplateNamespace;

//===================================================================

bool DetailAppearanceTemplate::ms_installed;

//===================================================================

void DetailAppearanceTemplate::install(const bool viewer)
{
	DEBUG_FATAL(ms_installed, ("DetailAppearanceTemplate already installed"));
	ms_installed = true;

	// add binding for our appearance template type to the AppearanceTemplateList
	// (allows AppearanceTemplateList::fetch(<detail appearance template filename> | <detail appearance iff>) to work)
	AppearanceTemplateList::assignBinding(TAG_DTLA, create);

#if _DEBUG
	DebugFlags::registerFlag (ms_debugReport, "ClientObject", "reportDetailAppearanceTemplate");
#endif

	ms_viewer = viewer;
	//Based on feedback from Andy Sommers, the value below is 375 * (3 / 4).  This is because, in client mode, the Memory
	//Manager will report 3/4 of the user's actual available memory.  The goal is to test their physical memory for 256MB or
	//less, and this will accomplish that basic purpose.  -- ARH
	ms_skipL0 = ConfigFile::getKeyBool ("ClientObject/DetailAppearanceTemplate", "skipL0", !ms_viewer && MemoryManager::getLimit () < ((375 * 3) / 4));
	REPORT_LOG (ms_skipL0, ("DetailAppearanceTemplate: skipping L0\n"));

	ExitChain::add (remove, "DetailAppearanceTemplate::remove");
}

//-------------------------------------------------------------------

void DetailAppearanceTemplate::remove(void)
{
	DEBUG_FATAL(!ms_installed, ("DetailAppearanceTemplate not installed"));
	ms_installed = false;

	AppearanceTemplateList::removeBinding(TAG_DTLA);
}

//-------------------------------------------------------------------

AppearanceTemplate *DetailAppearanceTemplate::create(const char *newName, Iff *iff)
{
	DEBUG_FATAL(!ms_installed, ("DetailAppearanceTemplate not installed"));
	return new DetailAppearanceTemplate(newName, iff);
}

//===================================================================

DetailAppearanceTemplate::DetailAppearanceTemplate(const char *newName, Iff *iff) :
	AppearanceTemplate(newName),
	m_childList (NON_NULL (new ChildList)),
	m_radarShape (0),
	m_testShape (0),
	m_writeShape (0),
	m_dpvsTestShape (0),
	m_dpvsWriteShape (0),
	m_usePivotPoint(false),
	m_disableLodCrossFade(false)
{
	DEBUG_FATAL(!ms_installed, ("DetailAppearanceTemplate not installed"));
	NOT_NULL (iff);

	load(*iff);

	if (!ms_viewer && ms_skipL0 && m_childList->size () > 1)
	{
		Child & child = m_childList->back ();

		if (child.appearanceTemplateName)
		{
			delete [] child.appearanceTemplateName;
			child.appearanceTemplateName = 0;
		}

		if (child.appearanceTemplate)
		{
			AppearanceTemplateList::release (child.appearanceTemplate);
			child.appearanceTemplate = 0;
		}

		m_childList->pop_back ();
	}

	// note: m_dpvsTestShape is now set via load().
	if (m_writeShape)
		m_dpvsWriteShape = RenderWorld::fetchMeshModel(*m_writeShape);
}

//-------------------------------------------------------------------

DetailAppearanceTemplate::~DetailAppearanceTemplate(void)
{
	uint i;
	for (i = 0; i < m_childList->size (); ++i)
	{
		delete [] (*m_childList) [i].appearanceTemplateName;
		(*m_childList) [i].appearanceTemplateName = 0;

		AppearanceTemplateList::release ((*m_childList) [i].appearanceTemplate);
		(*m_childList) [i].appearanceTemplate = 0;
	}

	m_childList->clear ();
	delete m_childList;
	m_childList = 0;

	if (m_radarShape)
		delete m_radarShape;

	if (m_testShape)
		delete m_testShape;

	if (m_dpvsTestShape)
	{
		IGNORE_RETURN(m_dpvsTestShape->release());
		m_dpvsTestShape = 0;
	}

	if (m_writeShape)
		delete m_writeShape;

	if (m_dpvsWriteShape)
	{
		IGNORE_RETURN(m_dpvsWriteShape->release());
		m_dpvsWriteShape = 0;
	}
}

//-------------------------------------------------------------------

void DetailAppearanceTemplate::preloadAssets() const
{
	AppearanceTemplate::preloadAssets();

	int const numberOfDetailLevels = getDetailLevelCount();
	for (int i = 0; i < numberOfDetailLevels; ++i)
	{
		createAppearanceTemplate(i);
		getAppearanceTemplate(i)->preloadAssets();
	}
}

//-------------------------------------------------------------------

Appearance *DetailAppearanceTemplate::createAppearance() const
{
	return new DetailAppearance(this);
}

//-------------------------------------------------------------------

int DetailAppearanceTemplate::getDetailLevelCount(void) const
{
	return static_cast<int> (m_childList->size ());
}

//-------------------------------------------------------------------

const char *DetailAppearanceTemplate::getAppearanceTemplateName(int detailLevel) const
{
	DEBUG_FATAL(detailLevel >= getDetailLevelCount (), ("specified detail level %d out of valid range [0 - %d)", detailLevel, getDetailLevelCount ()));
	return (*m_childList) [static_cast<uint> (detailLevel)].appearanceTemplateName;
}

//-------------------------------------------------------------------

const AppearanceTemplate *DetailAppearanceTemplate::getAppearanceTemplate(int detailLevel) const
{
	DEBUG_FATAL(detailLevel >= getDetailLevelCount (), ("specified detail level %d out of valid range [0 - %d)", detailLevel, getDetailLevelCount ()));
	return (*m_childList) [static_cast<uint> (detailLevel)].appearanceTemplate;
}

//-------------------------------------------------------------------

real DetailAppearanceTemplate::getNearDistance(int detailLevel) const
{
	DEBUG_FATAL(detailLevel >= getDetailLevelCount (), ("specified detail level %d out of valid range [0 - %d)", detailLevel, getDetailLevelCount ()));
	return (*m_childList) [static_cast<uint> (detailLevel)].nearDistance;
}

//-------------------------------------------------------------------

real DetailAppearanceTemplate::getFarDistance(int detailLevel) const
{
	DEBUG_FATAL(detailLevel >= getDetailLevelCount (), ("specified detail level %d out of valid range [0 - %d)", detailLevel, getDetailLevelCount ()));
	return (*m_childList) [static_cast<uint> (detailLevel)].farDistance;
}

//-------------------------------------------------------------------

void DetailAppearanceTemplate::createAppearanceTemplate(int detailLevel) const
{
	if (!getAppearanceTemplate (detailLevel))
	{
		bool reenableAsynchronousLoading = false;
		if (!m_dpvsTestShape && detailLevel == 0 && AsynchronousLoader::isEnabled())
		{
			AsynchronousLoader::disable();
			reenableAsynchronousLoading = true;
		}

		(*m_childList) [static_cast<ChildList::size_type>(detailLevel)].appearanceTemplate = AppearanceTemplateList::fetch (getAppearanceTemplateName (detailLevel));
		DEBUG_REPORT_LOG_PRINT (ms_debugReport, ("%s demand loaded %s\n", getName (), getAppearanceTemplateName (detailLevel)));

		if (reenableAsynchronousLoading)
			AsynchronousLoader::enable();
	}
}

//-------------------------------------------------------------------

Appearance* DetailAppearanceTemplate::createAppearance(int detailLevel) const
{
	createAppearanceTemplate(detailLevel);
	return getAppearanceTemplate (detailLevel)->createAppearance ();
}

//-------------------------------------------------------------------

const IndexedTriangleList* DetailAppearanceTemplate::getRadarShape () const
{
	return m_radarShape;
}

//-------------------------------------------------------------------

const IndexedTriangleList* DetailAppearanceTemplate::getTestShape () const
{
	return m_testShape;
}

//-------------------------------------------------------------------

const IndexedTriangleList* DetailAppearanceTemplate::getWriteShape () const
{
	return m_writeShape;
}

// ----------------------------------------------------------------------

DPVS::Model* DetailAppearanceTemplate::fetchDpvsTestShape () const
{
	//-- Fetch reference for caller.
	if (m_dpvsTestShape)
		m_dpvsTestShape->addReference ();

	return m_dpvsTestShape;
}

//-------------------------------------------------------------------

void DetailAppearanceTemplate::setDpvsTestShape (DPVS::Model *testShape)
{
	//-- Check for assignment to self.
	if (testShape == m_dpvsTestShape)
		return;

	//-- Bump up ref count on new test shape.
	if (testShape)
		testShape->addReference ();

	//-- Reduce ref count on old test shape.
	if (m_dpvsTestShape)
		IGNORE_RETURN (m_dpvsTestShape->release ());

	//-- Do the assignment.
	m_dpvsTestShape = testShape;
}

// ----------------------------------------------------------------------

DetailAppearanceTemplate::Child * DetailAppearanceTemplate::findChild ( int id )
{
	for(uint i = 0; i < m_childList->size(); i++)
	{
		Child & child = m_childList->at(i);

		if(child.id == id) return &child;
	}

	return NULL;
}

// ----------------------------------------------------------------------

bool DetailAppearanceTemplate::loadEntries ( Iff & iff )
{
	iff.enterChunk (TAG_INFO);

		while(iff.getChunkLengthLeft())
		{
			int  id           = iff.read_int32 ();
			real nearDistance = iff.read_float ();
			real farDistance  = iff.read_float ();

			Child child;

			child.id = id;
			child.nearDistance = nearDistance;
			child.farDistance = farDistance;
			child.appearanceTemplateName = 0;
			child.appearanceTemplate = 0;

 			m_childList->push_back(child);
		}

	iff.exitChunk (TAG_INFO);

	iff.enterForm (TAG_DATA);

		while(iff.getNumberOfBlocksLeft())
		{
			iff.enterChunk (TAG_CHLD);

				int id = iff.read_int32 ();
				char* name = iff.read_string ();

				Child * child = findChild(id);
				NOT_NULL (child);

				child->appearanceTemplateName = DuplicateString (FileName (FileName::P_appearance, name));

				delete [] name;

			iff.exitChunk (TAG_CHLD);
		}

	iff.exitForm (TAG_DATA);

	if (ConfigClientObject::getDetailAppearancesWithoutSprites())
	{
		bool done = true;
		do
		{
			// check if the lowest LOD is a sprite, major hack to keep from loading the whole apperance template
			Iff iff2((*m_childList)[0].appearanceTemplateName);
			if (iff2.getCurrentName() == TAG_QUAD || iff2.getCurrentName() == TAG_SPRT)
			{
				// make the first non-sprite LOD distance include the distance for the sprite as well
				(*m_childList)[1].nearDistance = (*m_childList)[0].nearDistance;

				// get rid of the first element in the list
				// @todo this should be done without a memory allocation, but oh well
				ChildList childList;
				childList.reserve(m_childList->size() - 1);
				childList.assign(m_childList->begin() + 1, m_childList->end());
				childList.swap(*m_childList);
				delete [] childList[0].appearanceTemplateName;

				// not done yet
				done = false;
			}
			else
				done = true;

		} while (!done);
	}

	return true;
}

// ----------------------------------------------------------------------

void DetailAppearanceTemplate::fixNearFarDistance ( void )
{
	for(uint i = 0; i < m_childList->size() - 1; i++)
	{
		m_childList->at(i).nearDistance = m_childList->at(i+1).farDistance;
	}

	m_childList->back().nearDistance = 0;
}

// ----------------------------------------------------------------------

void DetailAppearanceTemplate::computeBetterNearFarDistances ()
{
	typedef std::vector<float>  FloatVector;

	//-- First average out each child's far distance with the near distance of
	//   the next detail level.
	int const childCount = static_cast<int>(m_childList->size());

	FloatVector  averageFarDistances(static_cast<size_t>(childCount));
	FloatVector  separationDistances(static_cast<size_t>(childCount));
	{
		float average;

		for (int i = childCount - 1; i >= 0; --i)
		{
			size_t const nearIndex = static_cast<size_t>(i);
			size_t const farIndex  = static_cast<size_t>(i - 1);

			if (i > 0)
			{
				average = 0.5f * ((*m_childList)[nearIndex].farDistance + (*m_childList)[farIndex].nearDistance);
				(*m_childList)[nearIndex].farDistance = average;
				(*m_childList)[farIndex].nearDistance = average;
			}
			else
			{
				average = (*m_childList)[nearIndex].farDistance;
				(*m_childList)[nearIndex].farDistance = average;
			}

			averageFarDistances[nearIndex] = average;

			Child const &child = (*m_childList)[nearIndex];
			separationDistances[nearIndex] = child.farDistance - child.nearDistance;
		}
	}

	//-- Now compute better near-far distances based on distance between LOD averages.
	float const detailOverlapFraction = ConfigClientObject::getDetailOverlapFraction ();
	float const detailOverlapCap      = 0.5f * ConfigClientObject::getDetailOverlapCap ();

	{
		for (int i = childCount - 1; i >= 1; --i)
		{
			size_t const nearIndex = static_cast<size_t>(i);
			size_t const farIndex  = static_cast<size_t>(i - 1);

			float const averageFarDistance = averageFarDistances[nearIndex];

			// Extend the far distance for the nearer LOD some percentage of the complete range for the further LOD.
			(*m_childList)[nearIndex].farDistance = averageFarDistance + std::min(detailOverlapFraction * separationDistances[farIndex],  detailOverlapCap);
			(*m_childList)[farIndex].nearDistance = averageFarDistance - std::min(detailOverlapFraction * separationDistances[nearIndex], detailOverlapCap);
		}
	}
}

// ----------------------------------------------------------------------

bool DetailAppearanceTemplate::loadRadarShape (Iff& iff)
{
	iff.enterForm (TAG_RADR);

		iff.enterChunk (TAG_INFO);

			bool hasRadarShape = iff.read_int32 () != 0;

		iff.exitChunk (TAG_INFO);

		if (hasRadarShape)
			m_radarShape = new IndexedTriangleList (iff);

	iff.exitForm (TAG_RADR);

	return true;
}

//-------------------------------------------------------------------

bool DetailAppearanceTemplate::loadTestShape ( Iff & iff )
{
	iff.enterForm (TAG_TEST);

		iff.enterChunk (TAG_INFO);

			bool hasTestShape = iff.read_int32 () != 0;

		iff.exitChunk (TAG_INFO);

		if (hasTestShape)
			m_testShape = new IndexedTriangleList (iff);

	iff.exitForm (TAG_TEST);

	return true;
}

// ----------------------------------------------------------------------

bool DetailAppearanceTemplate::loadWriteShape ( Iff & iff )
{
	iff.enterForm (TAG_WRIT);

		iff.enterChunk (TAG_INFO);

			bool hasWriteShape = iff.read_int32 () != 0;

		iff.exitChunk (TAG_INFO);

		if (hasWriteShape)
			m_writeShape = new IndexedTriangleList (iff);

	iff.exitForm (TAG_WRIT);

	return true;
}

//-------------------------------------------------------------------

bool childSorter( DetailAppearanceTemplate::Child const & A, DetailAppearanceTemplate::Child const & B )
{
	return A.farDistance > B.farDistance;
}

void DetailAppearanceTemplate::load (Iff& iff)
{
	iff.enterForm (TAG_DTLA);

#ifdef _DEBUG
		DEBUG_WARNING (DataLint::isEnabled () && iff.getCurrentName () < TAG_0007, ("DetailAppearanceTemplate [%s] loading older version -- this asset will need to be reexported", getCrcName ().getString ()));
#endif

		Tag versionTag = iff.getCurrentName();
		int version = ConvertTagToInt(versionTag);
		if (version<1 || version>8)
		{
			FATAL (true, ("DetailAppearanceTemplate::load - unsupported format"));
		}

		//-------------------------------------------------------------------
		iff.enterForm(versionTag);
		{
			// ---------------------------------------
			if (version>=4)
			{
				AppearanceTemplate::load(iff);
			}
			// ---------------------------------------

			// ---------------------------------------
			if (version>=6)
			{
				iff.enterChunk (TAG_PIVT);

				uint8 lodFlags = iff.read_uint8();
				m_usePivotPoint = (lodFlags&1)!=0;
				if (version>=8)
				{
					m_disableLodCrossFade = (lodFlags&2)!=0;
				}

				iff.exitChunk (TAG_PIVT);
			}
			// ---------------------------------------

			// ---------------------------------------
			IGNORE_RETURN(loadEntries(iff));
			// ---------------------------------------

			// ---------------------------------------
			if (version>=7)
			{
				IGNORE_RETURN (loadRadarShape(iff));
			}
			// ---------------------------------------

			// ---------------------------------------
			if (version>=2)
			{
				IGNORE_RETURN(loadTestShape(iff));
				IGNORE_RETURN(loadWriteShape(iff));
			}
			// ---------------------------------------

			// ---------------------------------------
			if (version==3 || version==4)
			{
				IGNORE_RETURN(AppearanceTemplate::loadFloors(iff));
				// floor loading was moved into Appearance after version 4.
			}
			// ---------------------------------------
		}
		iff.exitForm(versionTag);
		//-------------------------------------------------------------------

	iff.exitForm (TAG_DTLA);

	// ----------

	const bool requiresNearFarFix  = version<=4;
	const bool hasValidBoundingBox = version>=6;

	// ----------

	std::sort(m_childList->begin(),m_childList->end(),childSorter);

	// Fixup near/far distances if required.
	if (requiresNearFarFix)
		fixNearFarDistance ();

	// Rebuild near/far distances with better algorithm.
#if 0
	DEBUG_REPORT_LOG(true, ("Pre-LOD fixup:\n"));
	debugDump();
#endif

	computeBetterNearFarDistances ();

#if 0
	DEBUG_REPORT_LOG(true, ("Post-LOD fixup:\n"));
	debugDump();
#endif


	// Create the DPVS test shape if possible.
	createDpvsTestShape (hasValidBoundingBox);
}

// ----------------------------------------------------------------------

void DetailAppearanceTemplate::createDpvsTestShape (bool hasValidBoundingBox)
{
	//-- If the data specified a test shape, use that test shape to generate the DPVS test shape.
	if (m_testShape && !ConfigClientObject::getDisableMeshTestShapes ())
	{
		// Generate the DPVS test shape model based on the data-specified test shape.
		m_dpvsTestShape = RenderWorld::fetchMeshModel (*m_testShape);

		// Generate the bounding box extent based on the test shape if we don't have a valid bounding box extent.
		if (!hasValidBoundingBox)
		{
			typedef std::vector<Vector>  VectorVector;

			VectorVector const& vertices  = m_testShape->getVertices ();
			BoxExtent *const    boxExtent = new BoxExtent ();

			//-- Generate bounding box from test shape vertices.
			VectorVector::const_iterator const endIt = vertices.end();
			for (VectorVector::const_iterator it = vertices.begin(); it != endIt; ++it)
				boxExtent->updateMinAndMax (*it);

			//-- Calculate the bounding sphere for the extent.
			boxExtent->calculateCenterAndRadius ();

			//-- Set the extent in this AppearanceTemplate.
			setExtent (ExtentList::fetch (boxExtent));
		}
	}
	else if (hasValidBoundingBox)
	{
		// Generate DPVS test shape based on bounding box extent.
		BoxExtent const *const boxExtent = safe_cast<BoxExtent const*> (getExtent ());
		NOT_NULL (boxExtent);

		m_dpvsTestShape = RenderWorld::fetchBoxModel (boxExtent->getBox ());
	}

	//-- If m_dpvsTestShape is NULL, it is expected that the first DetailAppearance that can
	//   will calculate its extents, then set the extents and the DPVS test shape for this
	//   DetailAppearanceTemplate.
}

// ----------------------------------------------------------------------

void DetailAppearanceTemplate::debugDump() const
{
#ifdef _DEBUG

	int const childCount = static_cast<int>(m_childList->size());
	DEBUG_REPORT_LOG(true, ("====\nDetailAppearanceTemplate: [%d] entries.\n", childCount));

	for (int i = childCount - 1; i >= 0; --i)
	{
		Child const &child = (*m_childList)[static_cast<size_t>(i)];
		DEBUG_REPORT_LOG(true, ("-- child [%d]: near distance=[%.2f], far distance=[%.2f], appearance=[%s].\n", (childCount - 1) - i, child.nearDistance, child.farDistance, child.appearanceTemplateName ? child.appearanceTemplateName : "<NULL appearance name>"));
	}

	DEBUG_REPORT_LOG(true, ("====\n"));

#endif
}

// ======================================================================
