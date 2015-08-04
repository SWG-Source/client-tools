// ======================================================================
//
// LodSkeletonTemplateWriter.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "LodSkeletonTemplateWriter.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/PointerDeleter.h"
#include "SkeletonTemplateWriter.h"

#include <algorithm>
#include <vector>

// ======================================================================

const Tag TAG_SLOD = TAG(S,L,O,D);

// ======================================================================

LodSkeletonTemplateWriter::LodSkeletonTemplateWriter() :
	m_levelWriters(new WriterVector)
{
}

// ----------------------------------------------------------------------

LodSkeletonTemplateWriter::~LodSkeletonTemplateWriter()
{
	std::for_each(m_levelWriters->begin(), m_levelWriters->end(), PointerDeleter());
	delete m_levelWriters;
}

// ----------------------------------------------------------------------
/**
 * Add a writer for a detail level of the LodSkeletonTemplate.
 *
 * It is assumed that the caller calls this function starting with
 * the highest detail writer first, followed by the next highest detail
 * and so forth.
 *
 * This instance takes ownership of the writer and will delete it from
 * the heap upon destruction.
 *
 * @param writer  the heap-based SkeletonTemplateWriter instance for the
 *                detail level being added.
 */

void LodSkeletonTemplateWriter::addDetailLevel(SkeletonTemplateWriter *writer)
{
	NOT_NULL(writer);
	m_levelWriters->push_back(writer);
}

// ----------------------------------------------------------------------

void LodSkeletonTemplateWriter::write(Iff &iff) const
{
	iff.insertForm(TAG_SLOD);
		iff.insertForm(TAG_0000);

			//-- Write # detail levels.
			iff.insertChunk(TAG_INFO);
				iff.insertChunkData(static_cast<int16>(m_levelWriters->size()));
			iff.exitChunk(TAG_INFO);

			//-- Write contents for each detail level.
			const WriterVector::const_iterator endIt = m_levelWriters->end();
			for (WriterVector::const_iterator it = m_levelWriters->begin(); it != endIt; ++it)
			{
				NOT_NULL(*it);
				(*it)->write(&iff);
			}

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_SLOD);
}

// ======================================================================

