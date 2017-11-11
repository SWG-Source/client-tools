// ============================================================================
//
// ParticleEmitterShape.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleEmitterShape.h"

#include "sharedRandom/Random.h"

// ============================================================================
//
// ParticleEmitterShape
//
// ============================================================================

//-----------------------------------------------------------------------------
ParticleEmitterShape *ParticleEmitterShape::createEmitterShape(Shape const shape)
{
	ParticleEmitterShape *particleEmitterShape = NULL;

	switch (shape)
	{
		case ParticleEmitterShape::Shape_circle:
			{
				particleEmitterShape = new ParticleEmitterShapeCircle;
				break;
			}
		case ParticleEmitterShape::Shape_sphere:
			{
				particleEmitterShape = new ParticleEmitterShapeSphere;
				break;
			}
		case ParticleEmitterShape::Shape_rectangle:
			{
				particleEmitterShape = new ParticleEmitterShapeRectangle;
				break;
			}
		case ParticleEmitterShape::Shape_cube:
			{
				particleEmitterShape = new ParticleEmitterShapeCube;
				break;
			}
		case ParticleEmitterShape::Shape_line:
			{
				particleEmitterShape = new ParticleEmitterShapeLine;
				break;
			}
		case ParticleEmitterShape::Shape_x:
			{
				particleEmitterShape = new ParticleEmitterShapeX;
				break;
			}
		default:
			{
				DEBUG_FATAL(1, ("ParticleEmitterShape::createEmitterShape() - Unknown shape enum"));
				break;
			}
	}

	return particleEmitterShape;
}

// ============================================================================
//
// ParticleEmitterShapeCircle
//
// ============================================================================

//-----------------------------------------------------------------------------
Vector ParticleEmitterShapeCircle::getValue() const
{
	float const rotation = Random::randomReal(-PI, PI);
	float x = cosf(rotation);
	float y = 0.0f;
	float z = sinf(rotation);

	return Vector(x, y, z);
}

//-----------------------------------------------------------------------------
ParticleEmitterShape::Shape ParticleEmitterShapeCircle::getEnum() const
{
	return ParticleEmitterShape::Shape_circle;
}

//-----------------------------------------------------------------------------
ParticleEmitterShape *ParticleEmitterShapeCircle::createEmitterShape() const
{
	return new ParticleEmitterShapeCircle;
}

// ============================================================================
//
// ParticleEmitterShapeSphere
//
// ============================================================================

//-----------------------------------------------------------------------------
Vector ParticleEmitterShapeSphere::getValue() const
{
	float const rotation1 = Random::randomReal(-PI, PI);
	float const rotation2 = Random::randomReal(-PI, PI);
	float const x1 = 1.0f;
	//float const y1 = 0.0f;
	//float const z1 = 0.0f;

	// Rotate around the z axis

	float const x2 = x1 * cosf(rotation1);
	float const y2 = x1 * sinf(rotation1);
	//float const z2 = z1;

	// Rotation around the x axis

	float const x3 = x2;
	float const y3 = y2 * cosf(rotation2);
	float const z3 = y2 * sinf(rotation2);

	return Vector(x3, y3, z3);
}

//-----------------------------------------------------------------------------
ParticleEmitterShape::Shape ParticleEmitterShapeSphere::getEnum() const
{
	return ParticleEmitterShape::Shape_sphere;
}

//-----------------------------------------------------------------------------
ParticleEmitterShape *ParticleEmitterShapeSphere::createEmitterShape() const
{
	return new ParticleEmitterShapeSphere;
}

// ============================================================================
//
// ParticleEmitterShapeRectangle
//
// ============================================================================

//-----------------------------------------------------------------------------
Vector ParticleEmitterShapeRectangle::getValue() const
{
	// This should always return a point that is on the perimeter of a unit
	// rectangle on the XZ plane

	int rand2 = Random::random(0, 1);
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	if (rand2 == 0)
	{
		x = (Random::random(0, 1)) ? 1.0f : -1.0f;
	}
	else
	{
		x = Random::randomReal(-1.0f, 1.0f);
	}

	if (rand2 == 1)
	{
		z = (Random::random(0, 1)) ? 1.0f : -1.0f;
	}
	else
	{
		z = Random::randomReal(-1.0f, 1.0f);
	}

	return Vector(x, y, z);
}

//-----------------------------------------------------------------------------
ParticleEmitterShape::Shape ParticleEmitterShapeRectangle::getEnum() const
{
	return ParticleEmitterShape::Shape_rectangle;
}

//-----------------------------------------------------------------------------
ParticleEmitterShape *ParticleEmitterShapeRectangle::createEmitterShape() const
{
	return new ParticleEmitterShapeRectangle;
}

// ============================================================================
//
// ParticleEmitterShapeCube
//
// ============================================================================

//-----------------------------------------------------------------------------
Vector ParticleEmitterShapeCube::getValue() const
{
	// This should always return a point that is on the perimeter of a unit
	// cube

	int rand3 = Random::random(0, 2);

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	if (rand3 == 0)
	{
		x = (Random::random(0, 1)) ? 1.0f : -1.0f;
	}
	else
	{
		x = Random::randomReal(-1.0f, 1.0f);
	}

	if (rand3 == 1)
	{
		y = (Random::random(0, 1)) ? 1.0f : -1.0f;
	}
	else
	{
		y = Random::randomReal(-1.0f, 1.0f);
	}

	if (rand3 == 2)
	{
		z = (Random::random(0, 1)) ? 1.0f : -1.0f;
	}
	else
	{
		z = Random::randomReal(-1.0f, 1.0f);
	}

	return Vector(x, y, z);
}

//-----------------------------------------------------------------------------
ParticleEmitterShape::Shape ParticleEmitterShapeCube::getEnum() const
{
	return ParticleEmitterShape::Shape_cube;
}

//-----------------------------------------------------------------------------
ParticleEmitterShape *ParticleEmitterShapeCube::createEmitterShape() const
{
	return new ParticleEmitterShapeCube;
}

// ============================================================================
//
// ParticleEmitterShapeLine
//
// ============================================================================

//-----------------------------------------------------------------------------
Vector ParticleEmitterShapeLine::getValue() const
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 1.0f;

	if (Random::random(0, 1))
	{
		z = -z;
	}

	return Vector(x, y, z);
}

//-----------------------------------------------------------------------------
ParticleEmitterShape::Shape ParticleEmitterShapeLine::getEnum() const
{
	return ParticleEmitterShape::Shape_line;
}

//-----------------------------------------------------------------------------
ParticleEmitterShape *ParticleEmitterShapeLine::createEmitterShape() const
{
	return new ParticleEmitterShapeLine;
}

// ============================================================================
//
// ParticleEmitterShapeX
//
// ============================================================================

//-----------------------------------------------------------------------------
Vector ParticleEmitterShapeX::getValue() const
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	if (Random::random(0, 1))
	{
		x = 1.0f;
	}
	else
	{
		z = 1.0f;
	}

	if (Random::random(0, 1))
	{
		x = -x;
		z = -z;
	}

	return Vector(x, y, z);
}

//-----------------------------------------------------------------------------
ParticleEmitterShape::Shape ParticleEmitterShapeX::getEnum() const
{
	return ParticleEmitterShape::Shape_x;
}

//-----------------------------------------------------------------------------
ParticleEmitterShape *ParticleEmitterShapeX::createEmitterShape() const
{
	return new ParticleEmitterShapeX;
}

// ============================================================================
