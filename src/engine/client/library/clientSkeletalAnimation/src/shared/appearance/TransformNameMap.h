// ======================================================================
//
// TransformNameMap.h
// Copyright 2001 - 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDE_TransformNameMap_H
#define INCLUDE_TransformNameMap_H

// ======================================================================

class CrcString;

// ======================================================================

class TransformNameMap
{
public:

	virtual ~TransformNameMap() {}

	virtual void             findTransformIndex(CrcString const &name, int *transformIndex, bool *found) const = 0;
	virtual int              getTransformIndex(CrcString const &name) const = 0;

	virtual int              getTransformCount() const = 0;
	virtual CrcString const &getTransformName(int index) const = 0;

};

// ======================================================================

#endif
