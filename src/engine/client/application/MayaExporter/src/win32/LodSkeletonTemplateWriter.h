// ======================================================================
//
// LodSkeletonTemplateWriter.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_LodSkeletonTemplateWriter_H
#define INCLUDED_LodSkeletonTemplateWriter_H

// ======================================================================

class Iff;
class SkeletonTemplateWriter;

// ======================================================================

class LodSkeletonTemplateWriter
{
public:

	LodSkeletonTemplateWriter();
	~LodSkeletonTemplateWriter();
	
	void  addDetailLevel(SkeletonTemplateWriter *writer);
	void  write(Iff &iff) const;

private:

	typedef stdvector<SkeletonTemplateWriter*>::fwd  WriterVector;

private:

	// Disabled.
	LodSkeletonTemplateWriter(const LodSkeletonTemplateWriter&);
	LodSkeletonTemplateWriter &operator =(const LodSkeletonTemplateWriter&);

private:

	WriterVector *m_levelWriters;

};

// ======================================================================

#endif
