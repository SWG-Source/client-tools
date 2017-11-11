// ======================================================================
//
// ClientStructureFootprintObject.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientStructureFootprintObject.h"

#include "clientGame/ClientStructureFootprintObjectNotification.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/RibbonAppearance.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/ObjectList.h"
#include "sharedObject/StructureFootprint.h"
#include "sharedTerrain/TerrainObject.h"

#include <vector>

// ======================================================================
// ClientStructureFootprintObjectNamespace
// ======================================================================

namespace ClientStructureFootprintObjectNamespace
{
	struct VectorInt
	{
	public:

		VectorInt (const int x, const int z) :
			m_x (x),
			m_z (z)
		{
		}

		VectorInt (const Vector& v) :
			m_x (static_cast<int> (v.x)),
			m_z (static_cast<int> (v.z))
		{
		}

		bool operator== (const VectorInt& rhs) const
		{
			return m_x == rhs.m_x && m_z == rhs.m_z;
		}

	public:

		int m_x;
		int m_z;
	};

	typedef std::pair<VectorInt, VectorInt> Border;
	typedef std::vector<Border> BorderList;
	typedef std::vector<Vector> PointList;

	void processLot (BorderList& borderList, const StructureFootprint& structureFootprint, const int i, const int j);
	void processBorder (BorderList& borderList, const Border& border);
	void createPointList (BorderList& borderList, PointList& pointList);

	const Tag            TAG_CSFO = TAG (C,S,F,O);
	TemporaryCrcString   ms_appearanceTemplateName;
	VectorArgb           ms_color (0.5f, 1.f, 0.f, 0.f);
	float                ms_width  = 0.025f;
	float                ms_height = 1.f;
}

using namespace ClientStructureFootprintObjectNamespace;

// ======================================================================
// STATIC PUBLIC ClientStructureFootprintObject
// ======================================================================

void ClientStructureFootprintObject::install ()
{
	InstallTimer const installTimer("ClientStructureFootprintObject::install");

	Iff iff;
	if (iff.open ("object/clientstructurefootprintobject.iff", true))
	{
		iff.enterForm (TAG_CSFO);

			iff.enterChunk (TAG_0000);

				ms_color  = iff.read_floatVectorArgb ();
				ms_width  = iff.read_float ();
				ms_height = iff.read_float ();

				char buffer [256];
				iff.read_string (buffer, 256);
				ms_appearanceTemplateName.set (buffer, true);

			iff.exitChunk (TAG_0000);

		iff.exitForm (TAG_CSFO);
	}
}

// ======================================================================
// PUBLIC ClientStructureFootprintObject
// ======================================================================

ClientStructureFootprintObject::ClientStructureFootprintObject (const StructureFootprint& structureFootprint) :
	Object (),
	m_objectList (new ObjectList),
	m_ribbonObject (0)
{
	addNotification (ClientStructureFootprintObjectNotification::getInstance ());
	RenderWorld::addObjectNotifications (*this);

	create (structureFootprint);
}

// ----------------------------------------------------------------------

ClientStructureFootprintObject::~ClientStructureFootprintObject ()
{
	removeNotification (ClientStructureFootprintObjectNotification::getInstance ());

	//-- these are added as child objects, so there is no need to delete them
	delete m_objectList;
	m_ribbonObject = 0;
}

// ----------------------------------------------------------------------

void ClientStructureFootprintObject::resetBoundary ()
{
	//-- resnap all objects to the terrain
	const TerrainObject* const terrainObject = TerrainObject::getConstInstance ();
	NOT_NULL (terrainObject);

	const Vector center_w = getPosition_w ();

	RibbonAppearance* const ribbonAppearance = safe_cast<RibbonAppearance*> (m_ribbonObject->getAppearance ());

	PointList pointList = ribbonAppearance->getPointList ();

	uint i;
	for (i = 0; i < pointList.size (); ++i)
	{
		Object* const childObject = getChildObject (static_cast<int> (i));

		Vector point = rotateTranslate_o2w (pointList [i]);
		IGNORE_RETURN (terrainObject->getHeight (point, point.y));
		point = rotateTranslate_w2o (point);
		childObject->setPosition_p (point);

		pointList [i] = point + Vector::unitY * ms_height;
	}

	ribbonAppearance->setPointList (pointList);
}

// ======================================================================
// PRIVATE ClientStructureFootprintObject
// ======================================================================

void ClientStructureFootprintObject::create (const StructureFootprint& structureFootprint)
{
	BorderList borderList;
	PointList  pointList;

	//-- process the footprint filling out the point list
	{
		int sj = 0;//structureFootprint.getHeight () - 1;
		int j;
		for (j = 0; j < structureFootprint.getHeight (); ++j)
		{
			int si = 0;
			int i;
			for (i = 0; i < structureFootprint.getWidth (); ++i)
			{
				if (structureFootprint.getLotType (si, sj) == LT_structure)
					processLot (borderList, structureFootprint, si, sj);

				++si;
			}

			++sj;
		}
	}

	createPointList (borderList, pointList);

	//-- take the point list and create the ribbon appearance and the markers
	{
		//-- create beacons
		uint i;
		for (i = 0; i < pointList.size (); ++i)
		{
			Object* const childObject = new Object ();
			childObject->setAppearance (AppearanceTemplateList::createAppearance (ms_appearanceTemplateName.getString ()));
			childObject->setPosition_p (pointList [i]);
			RenderWorld::addObjectNotifications (*childObject);
			addChildObject_p (childObject);

			m_objectList->addObject (childObject);
		}

		//-- create ribbon
		m_ribbonObject = new Object ();
		m_ribbonObject->setAppearance (new RibbonAppearance (pointList, ms_width, ms_color));
		RenderWorld::addObjectNotifications (*m_ribbonObject);
		addChildObject_p (m_ribbonObject);
	}
}

// ----------------------------------------------------------------------

void ClientStructureFootprintObjectNamespace::processLot (BorderList& borderList, const StructureFootprint& structureFootprint, const int i, const int j)
{
	const TerrainObject* const terrainObject = TerrainObject::getConstInstance ();
	NOT_NULL (terrainObject);

	const float chunkWidthInMeters = terrainObject->getChunkWidthInMeters ();
	const int   pivotX = structureFootprint.getPivotX ();
	const int   pivotZ = structureFootprint.getPivotZ ();
	const float chunkWidthInMeters_2 =  chunkWidthInMeters * 0.5f;
	const float x0 = (i + 0 - pivotX) * chunkWidthInMeters - chunkWidthInMeters_2;
	const float x1 = (i + 1 - pivotX) * chunkWidthInMeters - chunkWidthInMeters_2;
	const float z0 = (j + 0 - pivotZ) * chunkWidthInMeters - chunkWidthInMeters_2;
	const float z1 = (j + 1 - pivotZ) * chunkWidthInMeters - chunkWidthInMeters_2;

	Vector corners [4];
	corners [0].set (x0, 0.f, z0);
	corners [1].set (x0, 0.f, z1);
	corners [2].set (x1, 0.f, z1);
	corners [3].set (x1, 0.f, z0);

	//-- draw border
	bool top    = false;
	bool bottom = false;
	bool left   = false;
	bool right  = false;
	structureFootprint.getBorder (i, j, top, bottom, left, right);

	if (top)
		processBorder (borderList, std::make_pair (VectorInt (corners [3]), VectorInt (corners [0])));

	if (bottom)
		processBorder (borderList, std::make_pair (VectorInt (corners [1]), VectorInt (corners [2])));

	if (left)
		processBorder (borderList, std::make_pair (VectorInt (corners [0]), VectorInt (corners [1])));

	if (right)
		processBorder (borderList, std::make_pair (VectorInt (corners [2]), VectorInt (corners [3])));
}

// ----------------------------------------------------------------------

void ClientStructureFootprintObjectNamespace::processBorder (BorderList& borderList, const Border& border)
{
	DEBUG_FATAL (border.first == border.second, ("edge contains zero length"));

	uint i;
	for (i = 0; i < borderList.size (); ++i)
		if (borderList [i] == border)
			return;

	borderList.push_back (border);
	//DEBUG_REPORT_LOG_PRINT (true, ("%i  a = %i %i   b = %i %i\n", i, borderList [i].first.m_x, borderList [i].first.m_z, borderList [i].second.m_x, borderList [i].second.m_z));
}

// ----------------------------------------------------------------------

void ClientStructureFootprintObjectNamespace::createPointList (BorderList& borderList, PointList& pointList)
{
	uint edge = 0;

	while (!borderList.empty ())
	{
		//-- remove edge
		Border border = borderList [edge];
		borderList.erase (borderList.begin () + edge);

		//-- add first to point list
		pointList.push_back (Vector (static_cast<float> (border.first.m_x), 0.f, static_cast<float> (border.first.m_z)));

		if (!borderList.empty ())
		{
			//-- search for second as first
			uint i;
			for (i = 0; i < borderList.size (); ++i)
				if (borderList [i].first == border.second)
				{
					edge = i;
					break;
				}

			if (i == borderList.size ())
			{
				DEBUG_REPORT_LOG_PRINT (true, ("border = %i %i\n", border.second.m_x, border.second.m_z));

				uint j;
				for (j = 0; j < borderList.size (); ++j)
					DEBUG_REPORT_LOG_PRINT (true, ("%i  a = %i %i   b = %i %i\n", j, borderList [j].first.m_x, borderList [j].first.m_z, borderList [j].second.m_x, borderList [j].second.m_z));

				DEBUG_FATAL (true, ("link not found. check the structure footprint for orphaned lots (holes)"));
			}
		}
	}
}

// ======================================================================
