// ======================================================================
//
// MayaConversions.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef MAYA_CONVERSIONS_H
#define MAYA_CONVERSIONS_H

// ======================================================================

#include "maya/MEulerRotation.h"
#include "maya/MFloatPoint.h"
#include "maya/MFloatVector.h"
#include "maya/MVector.h"
#include "sharedMath/Vector.h"

class Quaternion;

// ======================================================================

class MayaConversions
{
public:

	static Vector      convertPoint(const MFloatPoint &mayaPoint);

	static Quaternion  convertRotation(const MEulerRotation &euler);
	static Vector      convertRotationAsVector(const MEulerRotation &euler);

	static Vector      convertVector(const MVector &mayaVector);
	static Vector      convertVector(const MFloatVector &mayaVector);

	static void        convertMayaUvToZeroOneRange(const float mayaUv[2], float &u, float &v);
};

// ======================================================================

inline Vector MayaConversions::convertPoint(const MFloatPoint &mayaPoint)
{
	return Vector(static_cast<real>(-mayaPoint.x), static_cast<real>(mayaPoint.y), static_cast<real>(mayaPoint.z));
}

// ----------------------------------------------------------------------

inline Vector MayaConversions::convertVector(const MVector &mayaVector)
{
	return Vector(static_cast<real>(-mayaVector.x), static_cast<real>(mayaVector.y), static_cast<real>(mayaVector.z));
}

// ----------------------------------------------------------------------

inline Vector MayaConversions::convertVector(const MFloatVector &mayaVector)
{
	return Vector(static_cast<real>(-mayaVector.x), static_cast<real>(mayaVector.y), static_cast<real>(mayaVector.z));
}

// ----------------------------------------------------------------------

inline Vector MayaConversions::convertRotationAsVector(const MEulerRotation &euler)
{
	return Vector(static_cast<real>(euler.x), static_cast<real>(-euler.y), static_cast<real>(-euler.z));
}

// ======================================================================

#endif
