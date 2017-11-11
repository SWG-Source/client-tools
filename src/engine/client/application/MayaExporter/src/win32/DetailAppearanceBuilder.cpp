// ======================================================================
//
// DetailAppearanceBuilder.cpp
// Portions Copyright 1999, Bootprint Entertainment, Inc.
// Portions Copyright 2000 - 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "DetailAppearanceBuilder.h"

#include "sharedCollision/BoxExtent.h"
#include "sharedFile/Iff.h"
#include "sharedMath/IndexedTriangleList.h"
#include "ExporterLog.h"
#include "Messenger.h"

#include <algorithm>

static const Tag TAG_DTLA = TAG (D,T,L,A);
static const Tag TAG_CHLD = TAG (C,H,L,D);
static const Tag TAG_FLOR = TAG (F,L,O,R);
static const Tag TAG_PIVT = TAG (P,I,V,T);
static const Tag TAG_RADR = TAG (R,A,D,R);
static const Tag TAG_TEST = TAG (T,E,S,T);
static const Tag TAG_WRIT = TAG (W,R,I,T);

extern Messenger* messenger;

//-------------------------------------------------------------------

DetailAppearanceBuilder::Entry::Entry (int id, const char* name, real nearDistance, real farDistance) :
	m_id (id),
	m_name (name),
	m_nearDistance (nearDistance),
	m_farDistance (farDistance)
{
}

//-------------------------------------------------------------------

DetailAppearanceBuilder::Entry::~Entry (void)
{
}

//-------------------------------------------------------------------

DetailAppearanceBuilder::DetailAppearanceBuilder(
	MObject const & rootMayaObject, 
	bool const usePivotPoint, 
	bool const disableLodCrossFade,
	Vector const & minVector, 
	Vector const & maxVector
) :
	MeshBuilder(rootMayaObject),
	m_entryList (),
	m_radarShape (0),
	m_testShape (0),
	m_writeShape (0),
	m_usePivotPoint(usePivotPoint),
	m_disableLodCrossFade(disableLodCrossFade)
{
	//-- Create the box extent for this appearance.
	BoxExtent *const extent = new BoxExtent ();
	extent->setMin (minVector);
	extent->setMax (maxVector);
	extent->calculateCenterAndRadius ();

	//-- Set the box extent to be written by the MeshBuilder.
	MeshBuilder::attachExtent (extent);
}

//-------------------------------------------------------------------

DetailAppearanceBuilder::~DetailAppearanceBuilder (void)
{
	clear ();
}

//-------------------------------------------------------------------

bool DetailAppearanceBuilder::write (const char* filename) const
{
	Iff iff (1024);
	write (iff);

	return iff.write (filename, true);
}

//-------------------------------------------------------------------

void DetailAppearanceBuilder::writeEntries ( Iff & iff ) const
{
	iff.insertChunk (TAG_INFO);

		//-- write out the table of distances
		uint i;
		for (i = 0; i < m_entryList.size(); i++)
		{
			const Entry* const entry = m_entryList[i];

			iff.insertChunkData (entry->m_id);
			iff.insertChunkData (entry->m_nearDistance);
			iff.insertChunkData (entry->m_farDistance);
		}

	iff.exitChunk (TAG_INFO);

	iff.insertForm (TAG_DATA);

		//-- write out the id/name pairs
		for (i = 0; i < m_entryList.size(); i++)
		{
			const Entry* const entry = m_entryList[i];

			iff.insertChunk (TAG_CHLD);

				iff.insertChunkData (entry->m_id);
				iff.insertChunkString (entry->m_name.c_str());

			iff.exitChunk ();
		}

	iff.exitForm ();
}

// ----------

void DetailAppearanceBuilder::writeRadarShape (Iff& iff) const
{
	iff.insertForm (TAG_RADR);

		iff.insertChunk (TAG_INFO);

			iff.insertChunkData (m_radarShape != 0 ? static_cast<int> (1) : static_cast<int> (0));

		iff.exitChunk ();

		if (m_radarShape)
			m_radarShape->write (iff);

	iff.exitForm ();
}

// ----------

void DetailAppearanceBuilder::writeTestShape ( Iff & iff ) const
{
	iff.insertForm (TAG_TEST);

		iff.insertChunk (TAG_INFO);

			iff.insertChunkData (m_testShape != 0 ? static_cast<int> (1) : static_cast<int> (0));

		iff.exitChunk (TAG_INFO);

		if (m_testShape)
			m_testShape->write (iff);

	iff.exitForm ();
}

// ----------

void DetailAppearanceBuilder::writeWriteShape ( Iff & iff ) const
{
	iff.insertForm (TAG_WRIT);

		iff.insertChunk (TAG_INFO);

			iff.insertChunkData (m_writeShape != 0 ? static_cast<int> (1) : static_cast<int> (0));

		iff.exitChunk (TAG_INFO);

		if (m_writeShape)
			m_writeShape->write (iff);

	iff.exitForm ();
}

// ----------------------------------------------------------------------

bool DetailAppearanceBuilder::entrySorter( DetailAppearanceBuilder::Entry const * A, DetailAppearanceBuilder::Entry const * B )
{
	return A->m_farDistance > B->m_farDistance;
}


bool DetailAppearanceBuilder::sort(void)
{
	std::sort(m_entryList.begin(),m_entryList.end(),entrySorter);
	
	for(uint i = 0; i < m_entryList.size(); i++)
	{
		Entry const * e = m_entryList[i];

		MESSENGER_REJECT ((e->m_nearDistance > e->m_farDistance) , ("DetailAppearanceBuilder::sort - Found a LOD level with bad distances. Check your LOD thresholds."));
	}

	return true;
}

// ----------------------------------------------------------------------

bool DetailAppearanceBuilder::write (Iff& iff) const
{
	const Tag versionTag = TAG_0008;

	iff.insertForm (TAG_DTLA);

		iff.insertForm (versionTag);

			MeshBuilder::write(iff);

			uint8 lodFlags=0;
			if (m_usePivotPoint)
			{
				lodFlags|=1;
			}
			if (m_disableLodCrossFade)
			{
				lodFlags|=2;
			}

			iff.insertChunk (TAG_PIVT);
				iff.insertChunkData(lodFlags);
			iff.exitChunk (TAG_PIVT);
			
			writeEntries(iff);

			writeRadarShape (iff);

			writeTestShape(iff);

			writeWriteShape(iff);

		iff.exitForm(versionTag);

	iff.exitForm (TAG_DTLA);

	return true;
}

//-------------------------------------------------------------------

void DetailAppearanceBuilder::clear (void)
{
	uint i;
	for (i = 0; i < m_entryList.size(); ++i)
	{
		delete m_entryList [i];
		m_entryList [i] = 0;
	}

	m_entryList.clear ();

	if (m_radarShape)
	{
		delete m_radarShape;
		m_radarShape = 0;
	}

	if (m_testShape)
	{
		delete m_testShape;
		m_testShape = 0;
	}

	if (m_writeShape)
	{
		delete m_writeShape;
		m_writeShape = 0;
	}
}

//-------------------------------------------------------------------

void DetailAppearanceBuilder::addAppearance (int id, const char* name, real nearDistance, real farDistance)
{
	MESSENGER_LOG(("Adding detail appearance %s, id %d, distance %f - %f\n",name,id,nearDistance,farDistance));

	Entry* entry = new Entry (id, name, nearDistance, farDistance);
	m_entryList.push_back(entry);
}

//-------------------------------------------------------------------

void DetailAppearanceBuilder::addRadarShape (const IndexedTriangleList* radarShape)
{
	if (m_radarShape)
		delete m_radarShape;

	m_radarShape = radarShape->clone ();
}

//-------------------------------------------------------------------

void DetailAppearanceBuilder::addTestShape (const IndexedTriangleList* testShape)
{
	if (m_testShape)
		delete m_testShape;

	m_testShape = testShape->clone ();
}

//-------------------------------------------------------------------

void DetailAppearanceBuilder::addWriteShape (const IndexedTriangleList* writeShape)
{
	if (m_writeShape)
		delete m_writeShape;

	m_writeShape = writeShape->clone ();
}

// ----------------------------------------------------------------------

bool DetailAppearanceBuilder::hasFrames ( void ) const
{
	return false;
}