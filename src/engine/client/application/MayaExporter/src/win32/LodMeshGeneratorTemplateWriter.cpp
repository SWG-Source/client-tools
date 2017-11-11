// ======================================================================
//
// LodMeshGeneratorTemplateWriter.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "LodMeshGeneratorTemplateWriter.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/PointerDeleter.h"

#include <algorithm>
#include <string>
#include <vector>

// ======================================================================

const Tag TAG_MLOD = TAG(M,L,O,D);

// ======================================================================

LodMeshGeneratorTemplateWriter::LodMeshGeneratorTemplateWriter() :
	m_levelNames(new StringVector)
{
}

// ----------------------------------------------------------------------

LodMeshGeneratorTemplateWriter::~LodMeshGeneratorTemplateWriter()
{
	delete m_levelNames;
}

// ----------------------------------------------------------------------
/**
 * Add the path name of a detail level of the LodMeshGeneratorTemplate.
 *
 * It is assumed that the caller calls this function starting with
 * the highest detail level first, followed by the next highest detail
 * and so forth.
 *
 * @param pathName  the path name (TreeFile-relative) for the next detail
 *                  level MeshGeneratorTemplate for the Lod.
 */

void LodMeshGeneratorTemplateWriter::addDetailLevelByName(const std::string &pathName)
{
	m_levelNames->push_back(pathName);
}

// ----------------------------------------------------------------------

void LodMeshGeneratorTemplateWriter::write(Iff &iff) const
{
	iff.insertForm(TAG_MLOD);
		iff.insertForm(TAG_0000);

			//-- Write # detail levels.
			iff.insertChunk(TAG_INFO);
				iff.insertChunkData(static_cast<int16>(m_levelNames->size()));
			iff.exitChunk(TAG_INFO);

			//-- Write contents for each detail level.
			const StringVector::const_iterator endIt = m_levelNames->end();
			for (StringVector::const_iterator it = m_levelNames->begin(); it != endIt; ++it)
			{
				iff.insertChunk(TAG_NAME);
					iff.insertChunkString(it->c_str());
				iff.exitChunk(TAG_NAME);
			}

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_MLOD);
}

// ======================================================================

