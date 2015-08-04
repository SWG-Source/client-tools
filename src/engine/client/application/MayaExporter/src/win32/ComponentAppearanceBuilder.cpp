// ======================================================================
//
// ComponentAppearanceBuilder.cpp
// Portions Copyright 1999, Bootprint Entertainment, Inc.
// Portions Copyright 2000 - 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "ComponentAppearanceBuilder.h"

#include "sharedCollision/BoxExtent.h"
#include "sharedFile/Iff.h"
#include "sharedMath/IndexedTriangleList.h"
#include "ExporterLog.h"
#include "Messenger.h"

static const Tag TAG_CMPA = TAG(C,M,P,A);
static const Tag TAG_PART = TAG(P,A,R,T);
static const Tag TAG_RADR = TAG (R,A,D,R);

extern Messenger* messenger;

// ----------------------------------------------------------------------

ComponentAppearanceBuilder::ComponentAppearanceBuilder(MObject const & rootMayaObject, Vector const & minVector, Vector const & maxVector) :
	MeshBuilder(rootMayaObject),
	m_parts (),
	m_radarShape (0)
{
	//-- Create the box extent for this appearance.
	BoxExtent *const extent = new BoxExtent ();
	extent->setMin (minVector);
	extent->setMax (maxVector);
	extent->calculateCenterAndRadius ();

	//-- Set the box extent to be written by the MeshBuilder.
	MeshBuilder::attachExtent (extent);
}

ComponentAppearanceBuilder::~ComponentAppearanceBuilder()
{
	if (m_radarShape)
	{
		delete m_radarShape;
		m_radarShape = 0;
	}
}

// ----------------------------------------------------------------------

bool ComponentAppearanceBuilder::addPart ( char const * partName, Transform transform )
{
	m_parts.push_back( Part(partName,transform) );

	return true;
}

// ----------------------------------------------------------------------

void ComponentAppearanceBuilder::addRadarShape (const IndexedTriangleList* radarShape)
{
	if (m_radarShape)
		delete m_radarShape;

	m_radarShape = radarShape->clone ();
}

//-------------------------------------------------------------------

bool ComponentAppearanceBuilder::write( Iff & iff )
{
	iff.insertForm(TAG_CMPA);

		iff.insertForm(TAG_0005);

			MeshBuilder::write(iff);

			writeRadarShape (iff);

			for(uint i = 0; i < m_parts.size(); i++)
			{
				Part & p = m_parts[i];

				iff.insertChunk (TAG_PART);

					iff.insertChunkString( p.m_name.c_str() );
					iff.insertChunkFloatTransform( p.m_transform );

				iff.exitChunk (TAG_PART);
			}

		iff.exitForm(TAG_0005);
		
	iff.exitForm(TAG_CMPA);

	return true;
}

// ----------------------------------------------------------------------

void ComponentAppearanceBuilder::writeRadarShape (Iff& iff) const
{
	iff.insertForm (TAG_RADR);

		iff.insertChunk (TAG_INFO);

			iff.insertChunkData (m_radarShape != 0 ? static_cast<int> (1) : static_cast<int> (0));

		iff.exitChunk ();

		if (m_radarShape)
			m_radarShape->write (iff);

	iff.exitForm ();
}

// ----------------------------------------------------------------------

bool ComponentAppearanceBuilder::hasFrames (void) const
{
	return false;
}

