// ======================================================================
//
// DustAppearance.cpp
// Copyright 2000-2001, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/DustAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Watcher.h"
#include "sharedMath/Sphere.h"
#include "sharedObject/AlterResult.h"
#include "sharedRandom/Random.h"

#include "dpvsObject.hpp"

#include <vector>

// ======================================================================

namespace DustAppearanceNamespace
{
	bool ms_disableRender;

	void remove();
}

using namespace DustAppearanceNamespace;

// ======================================================================

void DustAppearanceNamespace::remove()
{
	DebugFlags::unregisterFlag(ms_disableRender);
}

// ======================================================================

class DustAppearance::LocalShaderPrimitive : public ShaderPrimitive
{
public:

	LocalShaderPrimitive(Object const * referenceObject, int numberOfDust, float dustRadius);
	virtual ~LocalShaderPrimitive();

	virtual const Vector getPosition_w() const;
	virtual float getDepthSquaredSortKey() const;
	virtual int getVertexBufferSortKey() const;
	virtual const StaticShader &prepareToView() const;
	virtual void prepareToDraw() const;
	virtual void draw() const;

	float getRadius() const;

private:

	LocalShaderPrimitive(LocalShaderPrimitive const &);
	LocalShaderPrimitive & operator=(LocalShaderPrimitive const &);

	VertexBufferFormat LocalShaderPrimitive::getVertexBufferFormat() const;

private:

	ConstWatcher<Object> m_referenceObject;
	float const m_dustRadius;
	float const m_dustRadiusSquared;
	float const m_ooDustRadiusSquared;
 	mutable Vector m_oldOrigin;
	mutable Vector m_oldReferenceObjectPosition;
	typedef std::vector<Vector> PointList;
	mutable PointList m_pointList;
};

// ======================================================================

VertexBufferFormat DustAppearance::LocalShaderPrimitive::getVertexBufferFormat() const
{
	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();
	return format;
}

// ======================================================================

DustAppearance::LocalShaderPrimitive::LocalShaderPrimitive(Object const * const referenceObject, int const numberOfDust, float const dustRadius) :
	ShaderPrimitive(),
	m_referenceObject(referenceObject),
	m_dustRadius(dustRadius),
	m_dustRadiusSquared(sqr(m_dustRadius)),
	m_ooDustRadiusSquared(1.0f / m_dustRadiusSquared),
	m_oldOrigin(),
	m_pointList()
{
	m_pointList.reserve(numberOfDust);
	for (int i = 0; i < numberOfDust; ++i)
	{
		Vector const point = Vector::randomUnit() * (m_dustRadius * sqrt(Random::randomReal(0.f, 1.f)));
		m_pointList.push_back(point);
	}
}

// ----------------------------------------------------------------------

DustAppearance::LocalShaderPrimitive::~LocalShaderPrimitive()
{
}

// ----------------------------------------------------------------------

Vector const DustAppearance::LocalShaderPrimitive::getPosition_w() const
{
	return Vector::zero;
}

// ----------------------------------------------------------------------

float DustAppearance::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return 0.0f;
}

// ----------------------------------------------------------------------

int DustAppearance::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return 0;
}

// ----------------------------------------------------------------------

StaticShader const & DustAppearance::LocalShaderPrimitive::prepareToView() const
{
	return ShaderTemplateList::get3dVertexColorACStaticShader();
}

// ----------------------------------------------------------------------

void DustAppearance::LocalShaderPrimitive::prepareToDraw() const
{
	int const numberOfPoints = static_cast<int>(m_pointList.size());

	DynamicVertexBuffer vertexBuffer(getVertexBufferFormat());
	vertexBuffer.lock(numberOfPoints * 2);

		Vector const newOrigin = ShaderPrimitiveSorter::getCurrentCameraPosition();
		Vector const newReferenceObjectPosition = m_referenceObject ? m_referenceObject->getPosition_w() : m_oldReferenceObjectPosition;
		Vector delta = newReferenceObjectPosition - m_oldReferenceObjectPosition;
		if (delta.magnitudeSquared() > 1.f)
			delta.normalize();

		VertexBufferWriteIterator v = vertexBuffer.begin();

		for (int i = 0; i < numberOfPoints; ++i)
		{
			Vector & position = m_pointList[i];

			// check for the dust still within the proper sphere
			Vector point = position - newOrigin;
			float magnitudeSquared = point.magnitudeSquared();
			if (magnitudeSquared > m_dustRadiusSquared)
			{
				// dust fell out of the sphere, so mirror its position in its old sphere into the new sphere
				point = position - m_oldOrigin;
				point = -point;
				point += newOrigin;
				m_pointList[i] = point;
				magnitudeSquared = point.magnitudeSquared();
			}

			uint8 const alpha = static_cast<uint8>(128 - static_cast<int>(clamp(0.f, magnitudeSquared * m_ooDustRadiusSquared * 128.f, 128.f)));
			PackedArgb const color(alpha, 255, 255, 255);

			v.setPosition(position);
			v.setColor0(color);
			++v;

			v.setPosition(position - delta);
			v.setColor0(color);
			++v;
		}

		m_oldOrigin = newOrigin;
		m_oldReferenceObjectPosition = newReferenceObjectPosition;

	vertexBuffer.unlock();

	Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
	Graphics::setVertexBuffer(vertexBuffer);
}

// ----------------------------------------------------------------------

void DustAppearance::LocalShaderPrimitive::draw() const
{
	Graphics::drawLineList();
}

// ----------------------------------------------------------------------

float DustAppearance::LocalShaderPrimitive::getRadius() const
{
	return m_dustRadius;
}

// ======================================================================

void DustAppearance::install()
{
	InstallTimer const installTimer("DustAppearance::install");

	DebugFlags::registerFlag(ms_disableRender, "ClientGame/DustAppearance", "disableRender");
	ExitChain::add(DustAppearanceNamespace::remove, "DustAppearanceNamespace::remove");
}

// ======================================================================

DustAppearance::DustAppearance(Object const * referenceObject, int const numberOfDust, float const dustRadius) :
	Appearance(0),
	m_localShaderPrimitive(new LocalShaderPrimitive(referenceObject, numberOfDust, dustRadius)),
	m_dpvsObject(NULL)
{
	m_dpvsObject = RenderWorld::createObject(this, m_localShaderPrimitive->getRadius());

	useRenderEffectsFlag(true);
}

// ----------------------------------------------------------------------

DustAppearance::~DustAppearance(void)
{
	delete m_localShaderPrimitive;
	m_dpvsObject->release();
}

// ----------------------------------------------------------------------

DPVS::Object * DustAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

// ----------------------------------------------------------------------

void DustAppearance::render() const
{
	if (ms_disableRender)
		return;

	ShaderPrimitiveSorter::add(*m_localShaderPrimitive);
}

// ======================================================================

