// ======================================================================
//
// LodMeshGeneratorTemplateWriter.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_LodMeshGeneratorTemplateWriter_H
#define INCLUDED_LodMeshGeneratorTemplateWriter_H

// ======================================================================

class Iff;

// ======================================================================

class LodMeshGeneratorTemplateWriter
{
public:

	LodMeshGeneratorTemplateWriter();
	~LodMeshGeneratorTemplateWriter();
	
	void  addDetailLevelByName(const std::string &pathName);
	void  write(Iff &iff) const;

private:

	typedef stdvector<std::string>::fwd  StringVector;

private:

	// Disabled.
	LodMeshGeneratorTemplateWriter(const LodMeshGeneratorTemplateWriter&);
	LodMeshGeneratorTemplateWriter &operator =(const LodMeshGeneratorTemplateWriter&);

private:

	StringVector *m_levelNames;

};

// ======================================================================

#endif
