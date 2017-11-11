// ======================================================================
//
// MayaConversions.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "MayaConversions.h"

#include "sharedMath/Quaternion.h"

// ======================================================================

Quaternion MayaConversions::convertRotation(const MEulerRotation &euler)
{
	Quaternion qx(static_cast<real>(euler.x), Vector::unitX);
	Quaternion qy(static_cast<real>(-euler.y), Vector::unitY);
	Quaternion qz(static_cast<real>(-euler.z), Vector::unitZ);

	switch (euler.order)
	{
		case MEulerRotation::kXYZ: return qz * (qy * qx);
		case MEulerRotation::kYZX: return qx * (qz * qy);
		case MEulerRotation::kZXY: return qy * (qx * qz);
		case MEulerRotation::kXZY: return qy * (qz * qx);
		case MEulerRotation::kYXZ: return qz * (qx * qy);
		case MEulerRotation::kZYX: return qx * (qy * qz);
	}

	FATAL(true, ("should not reach here\n"));
	return Quaternion(); //lint !e527 // unreachable // true, needed for MSVC
}

// ----------------------------------------------------------------------

void MayaConversions::convertMayaUvToZeroOneRange(const float mayaUv[2], float &u, float &v)
{
	// ensure uvs are within 0.0f - 1.0f
	u = mayaUv[0] - static_cast<float>(floor(static_cast<double>(mayaUv[0])));
	v = 1.0f - (mayaUv[1] - static_cast<float>(floor(static_cast<double>(mayaUv[1]))));
}

// ======================================================================
