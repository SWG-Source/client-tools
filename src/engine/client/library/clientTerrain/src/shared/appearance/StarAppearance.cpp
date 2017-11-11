// ======================================================================
//
// StarAppearance.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/StarAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "clientGraphics/SystemVertexBuffer.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedFoundation/Tag.h"
#include "sharedImage/Image.h"
#include "sharedImage/ImageFormatList.h"
#include "sharedMath/Sphere.h"
#include "sharedRandom/RandomGenerator.h"

#include "dpvsObject.hpp"

#include <algorithm>
#include <vector>

// ======================================================================
// StarAppearanceNamespace
// ======================================================================

namespace StarAppearanceNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	int const cms_maximumVertexBufferSize = 128 * 1024;

	Tag const TAG_MAIN = TAG(M,A,I,N);

	bool ms_noRenderStars;
	bool ms_debugReport;

	void remove();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool isValid(const Image* const image)
	{
		if (image->getWidth() == 0)
			return false;

		if (image->getPixelFormat() == Image::PF_bgra_8888)
			return true;

		if (image->getPixelFormat() == Image::PF_abgr_8888)
			return true;

		if (image->getPixelFormat() == Image::PF_rgba_8888)
			return true;

		if (image->getPixelFormat() == Image::PF_argb_8888)
			return true;

		return false;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const PackedArgb getPixel(const Image* const image, int x, int y)
	{
		PackedArgb result = PackedArgb::solidBlack;

		const uint8* data = image->lockReadOnly();

			data += y * image->getStride() + x * image->getBytesPerPixel();

			switch (image->getPixelFormat())
			{
			case Image::PF_bgra_8888:
				result.setB(*data++);
				result.setG(*data++);
				result.setR(*data++);
				result.setA(*data++);
				break;

			case Image::PF_abgr_8888:
				result.setA(*data++);
				result.setR(*data++);
				result.setG(*data++);
				result.setB(*data++);
				break;

			case Image::PF_rgba_8888:
				result.setR(*data++);
				result.setG(*data++);
				result.setB(*data++);
				result.setA(*data++);
				break;

			case Image::PF_argb_8888:
				result.setA(*data++);
				result.setR(*data++);
				result.setG(*data++);
				result.setB(*data++);
				break;
			}

		image->unlock();

		return result;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void loadColorRamp(const char* fileName, PackedArgb*& colorList, int& numberOfColors)
	{
		//-- load the image
		Image* const image = fileName ? ImageFormatList::loadImage(fileName) : 0;
		if (image && isValid(image))
		{
			//-- extract the color ramps
			numberOfColors = image->getWidth();
			colorList = new PackedArgb[numberOfColors];

			int i;
			for (i = 0; i < numberOfColors; ++i)
				colorList[i] = getPixel(image, i, 0);

			delete image;
		}
		else
		{
			DEBUG_WARNING(fileName && *fileName,("EnvironmentBlock::loadColorRamps - image[%s] loaded, but is not in the appropriate format(256w x 8h x 32b tga)", fileName));

			if (image)
				delete image;

			//-- default color ramps
			numberOfColors = 1;
			colorList = new PackedArgb[1];
			colorList[0] = PackedArgb::solidGray;
		}
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Vector const randomUnit(RandomGenerator & randomGenerator)
	{
		float const z = randomGenerator.randomReal(-1.f, 1.f);
		float const t = randomGenerator.randomReal(0.f, PI_TIMES_2);
		float const r = sqrt(1.f - sqr(z));
		return Vector(r * cos(t), r * sin(t), z);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	VertexBufferFormat const getVertexBufferFormat()
	{
		VertexBufferFormat format;
		format.setPosition();
		format.setColor0();
		return format;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	DynamicVertexBuffer * ms_dynamicVertexBuffer;
}

using namespace StarAppearanceNamespace;

// ======================================================================
// StarAppearance::LocalShaderPrimitive
// ======================================================================

class StarAppearance::LocalShaderPrimitive : public ShaderPrimitive
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	LocalShaderPrimitive(StarAppearance const & owner, int numberOfVertices, uint64 sectionMask);
	virtual ~LocalShaderPrimitive();

	virtual Vector const getPosition_w() const;
	virtual float getDepthSquaredSortKey() const;
	virtual int getVertexBufferSortKey() const;
	virtual StaticShader const & prepareToView() const;
	virtual void prepareToDraw() const;
	virtual void draw() const;

private:

	LocalShaderPrimitive();
	LocalShaderPrimitive(LocalShaderPrimitive const &);
	LocalShaderPrimitive & operator=(LocalShaderPrimitive const &);

private:

	StarAppearance const & m_owner;	
	int const m_numberOfVertices;
	uint64 const m_sectionMask;
};

// ======================================================================
// STATIC PUBLIC StarAppearance::LocalShaderPrimitive
// ======================================================================

void StarAppearance::install()
{
	LocalShaderPrimitive::install();

	DebugFlags::registerFlag(ms_noRenderStars, "ClientTerrain", "noRenderStars");
	DebugFlags::registerFlag(ms_debugReport, "ClientTerrain", "reportStars");
	ExitChain::add(StarAppearanceNamespace::remove, "StarAppearanceNamespace::remove");

	ms_dynamicVertexBuffer = new DynamicVertexBuffer(getVertexBufferFormat());
}

// ----------------------------------------------------------------------

void StarAppearanceNamespace::remove()
{
	DebugFlags::unregisterFlag(ms_noRenderStars);
	DebugFlags::unregisterFlag(ms_debugReport);
	delete ms_dynamicVertexBuffer;
	ms_dynamicVertexBuffer = NULL;
}

// ======================================================================
// PUBLIC StarAppearance::LocalShaderPrimitive
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(StarAppearance::LocalShaderPrimitive, true, 0, 0, 0);

// ======================================================================

StarAppearance::LocalShaderPrimitive::LocalShaderPrimitive(StarAppearance const & owner, int const numberOfVertices, uint64 const sectionMask) : 
	ShaderPrimitive(),
	m_owner(owner),
	m_numberOfVertices(numberOfVertices),
	m_sectionMask(sectionMask)
{
}

// ----------------------------------------------------------------------

StarAppearance::LocalShaderPrimitive::~LocalShaderPrimitive()
{
}

// ----------------------------------------------------------------------

const Vector StarAppearance::LocalShaderPrimitive::getPosition_w() const
{
	return Vector::zero;
}
	
//-------------------------------------------------------------------

float StarAppearance::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return 0.f;
}

// ----------------------------------------------------------------------

int StarAppearance::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return 0;
}

// ----------------------------------------------------------------------

const StaticShader &StarAppearance::LocalShaderPrimitive::prepareToView() const
{
	return m_owner.m_shader->prepareToView();
}

// ----------------------------------------------------------------------

void StarAppearance::LocalShaderPrimitive::prepareToDraw() const
{
	Transform transform = m_owner.getTransform_w();
	transform.setPosition_p(ShaderPrimitiveSorter::getCurrentCameraPosition());

	if (!m_owner.m_allowRotate)
		transform.resetRotate_l2p();

	ms_dynamicVertexBuffer->lock(m_numberOfVertices);

	VertexBufferWriteIterator v = ms_dynamicVertexBuffer->begin();
	uint64 sectionMask = 1;

	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			for (int k = 0; k < 4; ++k)
			{
				if (m_sectionMask & sectionMask)
				{
					Section const * section = m_owner.m_sections[i][j][k];
					SystemVertexBuffer const * const sectionVertexBuffer = section->second;

					v.copy(sectionVertexBuffer->beginReadOnly(), sectionVertexBuffer->getNumberOfVertices());
					v += sectionVertexBuffer->getNumberOfVertices();
				}

				sectionMask <<= 1;
			}

	ms_dynamicVertexBuffer->unlock();

	Graphics::setObjectToWorldTransformAndScale(transform, Vector::xyz111);
	Graphics::setVertexBuffer(*ms_dynamicVertexBuffer);
}

// ----------------------------------------------------------------------

void StarAppearance::LocalShaderPrimitive::draw() const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("StarAppearance::PrimitiveNode::draw");

	const float pointSize = Graphics::getPointSize();
	Graphics::setPointSize(2.f);

	const bool pointSpriteEnable = Graphics::getPointSpriteEnable();
	Graphics::setPointSpriteEnable(true);

	bool fogEnabled;
	float fogDensity;
	PackedArgb fogColor;

	Graphics::getFog(fogEnabled, fogDensity, fogColor);
	Graphics::setFog(false, fogDensity, fogColor);

	Graphics::drawPointList();

	Graphics::setFog(fogEnabled, fogDensity, fogColor);

	Graphics::setPointSize(pointSize);
	Graphics::setPointSpriteEnable(pointSpriteEnable);
}

// ======================================================================
// PUBLIC StarAppearance
// ======================================================================

StarAppearance::StarAppearance(char const * const starColorRampFileName, int const numberOfStars, bool const allowRotate, float const /*initialAlpha*/) : 
	Appearance(0),
	m_dpvsObject(NULL),
	m_shader(ShaderTemplateList::fetchModifiableShader("shader/stars.sht")),
	m_localShaderPrimitiveList(new LocalShaderPrimitiveList),
	m_allowRotate(allowRotate),
	m_lastColor(0)
{
	m_dpvsObject = RenderWorld::createUnboundedObject(this);

	typedef std::pair<Vector, uint32> Point;
	typedef std::vector<Point> PointList;
	PointList sectionPointList[4][4][4];
	{
		PackedArgb * colorList = 0;
		int numberOfColors;
		loadColorRamp(starColorRampFileName, colorList, numberOfColors);

		RandomGenerator random(numberOfStars);

		for (int i = 0; i < numberOfStars; ++i)
		{
			VectorArgb color(colorList[i % numberOfColors]);
			color.a *= random.randomReal(1.f);

			PackedArgb const color2(color);

			Vector const position = randomUnit(random) * 10.f;

			int const x = (position.x < -5.f) ? 0 : (position.x < 0.f ? 1 : (position.x < 5.f ? 2 : 3));
			int const y = (position.y < -5.f) ? 0 : (position.y < 0.f ? 1 : (position.y < 5.f ? 2 : 3));
			int const z = (position.z < -5.f) ? 0 : (position.z < 0.f ? 1 : (position.z < 5.f ? 2 : 3));
			sectionPointList[x][y][z].push_back(std::make_pair(position, color2.getArgb()));
		}

		delete[] colorList;
		colorList = 0;
	}

	{
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				for (int k = 0; k < 4; ++k)
				{
					m_sections[i][j][k] = 0;

					BoxExtent extent;
					extent.setMin(Vector::maxXYZ);
					extent.setMax(Vector::negativeMaxXYZ);

					PointList const & pointList = sectionPointList[i][j][k];
					if (pointList.size())
					{
						Section * const section = new Section;
						m_sections[i][j][k] = section;

						SystemVertexBuffer * const systemVertexBuffer = new SystemVertexBuffer(getVertexBufferFormat(), pointList.size());
						DEBUG_REPORT_LOG(ms_debugReport, ("%i %i %i number=%i size=%i\n", i, j, k, pointList.size(), pointList.size() * systemVertexBuffer->getVertexSize()));
						section->second = systemVertexBuffer;

						VertexBufferWriteIterator v = systemVertexBuffer->begin();

						for (size_t m = 0; m < pointList.size(); ++m, ++v)
						{
							Point const & point = pointList[m];
							extent.updateMinAndMax(point.first);

							v.setPosition(point.first);
							v.setColor0(point.second);
						}

						extent.calculateCenterAndRadius();
						section->first = extent.getSphere();
					}
				}
	}

	DEBUG_WARNING(!m_shader->getStaticShader()->hasTextureFactor(TAG_MAIN),("StarAppearance: %s is missing texture factor tag MAIN.  Please open and save this shader template in the ShaderBuilder.", m_shader->getName()));
}

// ----------------------------------------------------------------------

StarAppearance::~StarAppearance()
{
	m_dpvsObject->release();

	m_shader->release();

	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			for (int k = 0; k < 4; ++k)
			{
				if (m_sections[i][j][k])
				{
					delete m_sections[i][j][k]->second;
					delete m_sections[i][j][k];
				}
			}

	std::for_each(m_localShaderPrimitiveList->begin(), m_localShaderPrimitiveList->end(), PointerDeleter());
	m_localShaderPrimitiveList->clear();
	delete m_localShaderPrimitiveList;
}

// ----------------------------------------------------------------------

DPVS::Object * StarAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

// ----------------------------------------------------------------------

void StarAppearance::setAlpha(bool const /* opaqueEnabled */, float const /* opaqueAlpha */, bool const /* alphaEnabled */, const float alphaAlpha)
{
	PackedArgb color(PackedArgb::solidWhite);
	color.setA(static_cast<uint8>(alphaAlpha * 255));

	if (color.getArgb() != m_lastColor && m_shader->getStaticShader()->hasTextureFactor(TAG_MAIN))
	{
		m_lastColor = color.getArgb();
		m_shader->getStaticShader()->setTextureFactor(TAG_MAIN, color.getArgb());
	}
}

// ----------------------------------------------------------------------

void StarAppearance::render() const
{
	if (!m_localShaderPrimitiveList->empty())
	{
		std::for_each(m_localShaderPrimitiveList->begin(), m_localShaderPrimitiveList->end(), PointerDeleter());
		m_localShaderPrimitiveList->clear();
	}

	if (ms_noRenderStars)
		return;

	Camera const & camera = ShaderPrimitiveSorter::getCurrentCamera();

#ifdef _DEBUG
	int totalNumberOfStars = 0;
	int numberOfStars = 0;
#endif
	int numberOfVertices = 0;
	uint64 sectionMask = 0;
	uint64 mask = 1;

	Transform const & transform = m_allowRotate ? getTransform_w() : Transform::identity;

	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			for (int k = 0; k < 4; ++k)
			{
				Section const * const section = m_sections[i][j][k];
				if (section)
				{
					SystemVertexBuffer const * const systemVertexBuffer = section->second;

#ifdef _DEBUG
					totalNumberOfStars += systemVertexBuffer->getNumberOfVertices();
#endif

					if (camera.testVisibility_c(Sphere(camera.rotate_w2o(transform.rotate_l2p(section->first.getCenter())), section->first.getRadius())))
					{
						if ((numberOfVertices + systemVertexBuffer->getNumberOfVertices()) * systemVertexBuffer->getVertexSize() > cms_maximumVertexBufferSize)
						{
							DEBUG_REPORT_PRINT(ms_debugReport, ("vertices=%i\n", numberOfVertices));
							LocalShaderPrimitive * const localShaderPrimitive = new LocalShaderPrimitive(*this, numberOfVertices, sectionMask);
							m_localShaderPrimitiveList->push_back(localShaderPrimitive);
							ShaderPrimitiveSorter::add(*localShaderPrimitive);

#ifdef _DEBUG
							numberOfStars += numberOfVertices;
#endif
							numberOfVertices = 0;
							sectionMask = 0;
						}

						numberOfVertices += systemVertexBuffer->getNumberOfVertices();
						sectionMask |= mask;
					}
				}

				mask <<= 1;
			}

	if (numberOfVertices)
	{
		DEBUG_REPORT_PRINT(ms_debugReport, ("vertices=%i\n", numberOfVertices));
		LocalShaderPrimitive * const localShaderPrimitive = new LocalShaderPrimitive(*this, numberOfVertices, sectionMask);
		m_localShaderPrimitiveList->push_back(localShaderPrimitive);
		ShaderPrimitiveSorter::add(*localShaderPrimitive);

#ifdef _DEBUG
		numberOfStars += numberOfVertices;
#endif
	}

	DEBUG_REPORT_PRINT(ms_debugReport, ("stars=%i/%i\n", numberOfStars, totalNumberOfStars));
}

// ======================================================================

