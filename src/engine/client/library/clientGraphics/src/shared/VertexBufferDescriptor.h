// ======================================================================
//
// VertexBufferDescriptor.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_VertexBufferDescriptor_H
#define INCLUDED_VertexBufferDescriptor_H

// ======================================================================

struct VertexBufferDescriptor
{
	// Compiler generated assignment operator, copy constructor, and destructor are all correct
	enum
	{
		MAX_TEXTURE_COORDINATE_SETS = 8
	};

	int8   vertexSize;
	int8   offsetPosition;
	int8   offsetOoz;
	int8   offsetNormal;
	int8   offsetPointSize;
	int8   offsetColor0;
	int8   offsetColor1;
	int8   offsetTextureCoordinateSet[MAX_TEXTURE_COORDINATE_SETS];

	VertexBufferDescriptor();
	bool operator ==(const VertexBufferDescriptor &rhs) const;
	bool operator !=(const VertexBufferDescriptor &rhs) const;
};

// ======================================================================

inline VertexBufferDescriptor::VertexBufferDescriptor()
:
	vertexSize(0),
	offsetPosition(-1),
	offsetOoz(-1),
	offsetNormal(-1),
	offsetPointSize(-1),
	offsetColor0(-1),
	offsetColor1(-1)
{
	for (int i = 0; i < MAX_TEXTURE_COORDINATE_SETS; ++i)
		offsetTextureCoordinateSet[i] = -1;
}

// ----------------------------------------------------------------------

inline bool VertexBufferDescriptor::operator ==(const VertexBufferDescriptor &rhs) const
{
	DEBUG_FATAL(MAX_TEXTURE_COORDINATE_SETS != 8, ("Fix this code if the number of texture coordinate sets changes"));
	return
		vertexSize                    == rhs.vertexSize                    &&
		offsetPosition                == rhs.offsetPosition                &&
		offsetOoz                     == rhs.offsetOoz                     &&
		offsetNormal                  == rhs.offsetNormal                  &&
		offsetPointSize               == rhs.offsetPointSize               &&
		offsetColor0                  == rhs.offsetColor0                  &&
		offsetColor1                  == rhs.offsetColor1                  &&
		offsetTextureCoordinateSet[0] == rhs.offsetTextureCoordinateSet[0] &&
		offsetTextureCoordinateSet[1] == rhs.offsetTextureCoordinateSet[1] &&
		offsetTextureCoordinateSet[2] == rhs.offsetTextureCoordinateSet[2] &&
		offsetTextureCoordinateSet[3] == rhs.offsetTextureCoordinateSet[3] &&
		offsetTextureCoordinateSet[4] == rhs.offsetTextureCoordinateSet[4] &&
		offsetTextureCoordinateSet[5] == rhs.offsetTextureCoordinateSet[5] &&
		offsetTextureCoordinateSet[6] == rhs.offsetTextureCoordinateSet[6] &&
		offsetTextureCoordinateSet[7] == rhs.offsetTextureCoordinateSet[7];
}

// ----------------------------------------------------------------------

inline bool VertexBufferDescriptor::operator !=(const VertexBufferDescriptor &rhs) const
{
	return !(*this == rhs);
}

// ======================================================================

#endif
