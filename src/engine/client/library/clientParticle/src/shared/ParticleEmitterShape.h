// ============================================================================
//
// ParticleEmitterShape.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleEmitterShape_H
#define INCLUDED_ParticleEmitterShape_H

#include "sharedMath/Vector.h"

class ParticleEmitterShapeCircle;
class ParticleEmitterShapeSphere;
class ParticleEmitterShapeRectangle;
class ParticleEmitterShapeCube;
class ParticleEmitterShapeLine;
class ParticleEmitterShapeX;

//-----------------------------------------------------------------------------
class ParticleEmitterShape
{
public:

	// If you add any additional derived particle emitter shapes, you should add
	// the appropriate enum to allow easier file I/O!

	enum Shape
	{
		Shape_circle = 0,
		Shape_sphere,
		Shape_rectangle,
		Shape_cube,
		Shape_line,
		Shape_x
	};

	static ParticleEmitterShape * createEmitterShape(Shape const shape);

	virtual Vector                getValue() const = 0;
	virtual Shape                 getEnum() const = 0;
	virtual ParticleEmitterShape *createEmitterShape() const = 0;
};

//-----------------------------------------------------------------------------
class ParticleEmitterShapeCircle : public ParticleEmitterShape
{
public:

	virtual Vector                getValue() const;
	virtual Shape                 getEnum() const;
	virtual ParticleEmitterShape *createEmitterShape() const;
};

//-----------------------------------------------------------------------------
class ParticleEmitterShapeSphere : public ParticleEmitterShape
{
public:

	virtual Vector                getValue() const;
	virtual Shape                 getEnum() const;
	virtual ParticleEmitterShape *createEmitterShape() const;
};

//-----------------------------------------------------------------------------
class ParticleEmitterShapeRectangle : public ParticleEmitterShape
{
public:

	virtual Vector                getValue() const;
	virtual Shape                 getEnum() const;
	virtual ParticleEmitterShape *createEmitterShape() const;
};

//-----------------------------------------------------------------------------
class ParticleEmitterShapeCube : public ParticleEmitterShape
{
public:

	virtual Vector                getValue() const;
	virtual Shape                 getEnum() const;
	virtual ParticleEmitterShape *createEmitterShape() const;
};

//-----------------------------------------------------------------------------
class ParticleEmitterShapeLine : public ParticleEmitterShape
{
public:

	virtual Vector                getValue() const;
	virtual Shape                 getEnum() const;
	virtual ParticleEmitterShape *createEmitterShape() const;
};

//-----------------------------------------------------------------------------
class ParticleEmitterShapeX : public ParticleEmitterShape
{
public:

	virtual Vector                getValue() const;
	virtual Shape                 getEnum() const;
	virtual ParticleEmitterShape *createEmitterShape() const;
};

//=============================================================================

#endif // INCLUDED_ParticleEmitterShape_H