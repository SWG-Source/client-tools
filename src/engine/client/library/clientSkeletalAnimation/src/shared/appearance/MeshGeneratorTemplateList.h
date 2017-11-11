// ======================================================================
//
// MeshGeneratorTemplateList.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_MeshGeneratorTemplateList_H
#define INCLUDED_MeshGeneratorTemplateList_H

// ==================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class CrcString;
class Iff;
class LessPointerComparator;
class MeshGenerator;
class MeshGeneratorTemplate;

// ==================================================================

class MeshGeneratorTemplateList
{
friend class MeshGeneratorTemplate;

public:

	typedef MeshGeneratorTemplate *(*CreationFunction)(Iff *iff, const CrcString &name);

public:

	static void                         install();

	static bool                         registerMeshGeneratorTemplate(Tag meshGeneratorTag, CreationFunction creationFunction, bool supportsAsynchronousLoading, CrcString const &fileExtension);
	static bool                         deregisterMeshGeneratorTemplate(Tag meshGeneratorTag);

	static const MeshGeneratorTemplate *fetch(const CrcString &filename);
	static const MeshGeneratorTemplate *fetch(Iff &iff);

	static void                         assignAsynchronousLoaderFunctions();

	static void                         debugDump();

private:

	static void                         stopTracking(const MeshGeneratorTemplate &meshGeneratorTemplate);
	static const MeshGeneratorTemplate *fetch(const CrcString &filename, bool create);

	static const void                  *asynchronousLoaderFetchNoCreate(char const *fileName);
	static void                         asynchronousLoaderRelease(void const *meshGeneratorTemplate);

private:

	// Disabled.
	MeshGeneratorTemplateList();
	MeshGeneratorTemplateList(const MeshGeneratorTemplateList&);
	MeshGeneratorTemplateList& operator =(const MeshGeneratorTemplateList&);

};

// ==================================================================

#endif
