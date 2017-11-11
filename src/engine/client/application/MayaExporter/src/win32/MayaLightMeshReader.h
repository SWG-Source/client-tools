// ======================================================================
//
// MayaLightMeshReader.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MayaLightMeshReader_H
#define INCLUDED_MayaLightMeshReader_H

// ======================================================================

class Messenger;
class MFnMesh;

// ======================================================================
/**
 * A helper class to enumerate all the triangles in a Maya mesh.
 *
 * This is common functionality used all over the place.  'Light' in the
 * name is meant to convey 'not heavy', not 'Light' as in a light source.
 *
 * Currently the character system dot3 makes use of this class for two
 * different triangle traversal processes.  I'd like to see the
 * skeletal mesh triangle data collection process use this as well.
 */

class MayaLightMeshReader
{
public:

	typedef bool (*TriangleCallback)(void *context, int shaderIndex, int polyIndex, const int polyVertexIndex[3]);

public:

	static void install(Messenger *newMessenger);
	static void remove();

	static bool enumerateTriangles(const MFnMesh &mesh, void *context, TriangleCallback callback);

};

// ======================================================================

#endif
