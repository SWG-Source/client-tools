// ======================================================================
//
// SkyBox6SidedAppearance.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/SkyBox6SidedAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/Tag.h"

#include "dpvsObject.hpp"

#include <algorithm>
#include <vector>

// ======================================================================
// SkyBoxAppearanceNamespace
// ======================================================================

namespace SkyBoxAppearanceNamespace
{
	Tag const TAG_MAIN = TAG(M,A,I,N);
}

using namespace SkyBoxAppearanceNamespace;

// ======================================================================
// SkyBox6SidedAppearance::LocalShaderPrimitive
// ======================================================================

class SkyBox6SidedAppearance::LocalShaderPrimitive : public ShaderPrimitive
{
public:

	static VertexBufferFormat getVertexBufferFormat();

public:

	LocalShaderPrimitive(SkyBox6SidedAppearance const & owner, Texture const * texture, Vector const * vertexFan);
	virtual ~LocalShaderPrimitive();

	virtual const Vector getPosition_w() const;
	virtual float getDepthSquaredSortKey() const;
	virtual int getVertexBufferSortKey() const;
	virtual const StaticShader& prepareToView() const;
	virtual void prepareToDraw() const;
	virtual void draw() const;

private:

	void init(Vector const * vertexFan);

private:

	SkyBox6SidedAppearance const & m_owner;	
	Shader * const m_shader;
	Texture const * const m_texture;
	StaticVertexBuffer m_vertexBuffer;

private:

	LocalShaderPrimitive();
	LocalShaderPrimitive(const LocalShaderPrimitive&);
	LocalShaderPrimitive& operator=(const LocalShaderPrimitive&);
};

// ======================================================================
// STATIC PUBLIC SkyBox6SidedAppearance::LocalShaderPrimitive
// ======================================================================

VertexBufferFormat SkyBox6SidedAppearance::LocalShaderPrimitive::getVertexBufferFormat()
{
	VertexBufferFormat format;
	format.setPosition();
	format.setNumberOfTextureCoordinateSets(1);
	format.setTextureCoordinateSetDimension(0, 2);
	return format;
}

// ======================================================================
// PUBLIC SkyBox6SidedAppearance::LocalShaderPrimitive
// ======================================================================

SkyBox6SidedAppearance::LocalShaderPrimitive::LocalShaderPrimitive(SkyBox6SidedAppearance const & owner, Texture const * const texture, Vector const * const vertexFan) : 
	ShaderPrimitive(),
	m_owner(owner),
	m_shader(ShaderTemplateList::fetchModifiableShader("shader/skybox_6sided.sht")),
	m_texture(texture),
	m_vertexBuffer(getVertexBufferFormat(), 4)
{
	init(vertexFan);

	NOT_NULL(m_texture);
	m_texture->fetch();
	m_shader->getStaticShader()->setTexture(TAG_MAIN, *texture);
}

// ----------------------------------------------------------------------

SkyBox6SidedAppearance::LocalShaderPrimitive::~LocalShaderPrimitive()
{
	m_texture->release();
	m_shader->release();
}

// ----------------------------------------------------------------------

Vector const SkyBox6SidedAppearance::LocalShaderPrimitive::getPosition_w() const
{
	return Vector::zero;
}
	
//-------------------------------------------------------------------

float SkyBox6SidedAppearance::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return 0.f;
}

// ----------------------------------------------------------------------

int SkyBox6SidedAppearance::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return m_vertexBuffer.getSortKey();
}

// ----------------------------------------------------------------------

StaticShader const & SkyBox6SidedAppearance::LocalShaderPrimitive::prepareToView() const
{
	return m_shader->prepareToView();
}

// ----------------------------------------------------------------------

void SkyBox6SidedAppearance::LocalShaderPrimitive::prepareToDraw() const
{
	Transform transform = m_owner.getTransform_w();
	transform.setPosition_p(ShaderPrimitiveSorter::getCurrentCameraPosition());
	Graphics::setObjectToWorldTransformAndScale(transform, Vector::xyz111);
	Graphics::setVertexBuffer(m_vertexBuffer);
}

// ----------------------------------------------------------------------

void SkyBox6SidedAppearance::LocalShaderPrimitive::draw() const
{
	Graphics::drawTriangleFan();
}

// ======================================================================
// PRIVATE SkyBox6SidedAppearance::LocalShaderPrimitive
// ======================================================================

void SkyBox6SidedAppearance::LocalShaderPrimitive::init(Vector const * const vertexFan)
{
	float const uvs[8] = 
	{
		0.f, 0.f, 
		1.f, 0.f, 
		1.f, 1.f, 
		0.f, 1.f
	};

	m_vertexBuffer.lock();
	{
		VertexBufferWriteIterator v = m_vertexBuffer.begin();

		int i;
		for (i = 0; i < 4; ++i, ++v)
		{
			v.setPosition(vertexFan[i]);
			v.setTextureCoordinates(0, uvs[i * 2 + 0], uvs[i * 2 + 1]);
		}
	}
	m_vertexBuffer.unlock();
}

// ======================================================================
// PUBLIC SkyBox6SidedAppearance
// ======================================================================

SkyBox6SidedAppearance::SkyBox6SidedAppearance(char const * textureNameMask) : 
	Appearance(0),
	m_localShaderPrimitiveList(new LocalShaderPrimitiveList),
	m_dpvsObject(0)
{
	Vector const vertices[6][4] =
	{
			//Vector(-1.f,  1.f,  1.f), // 0
			//Vector( 1.f,  1.f,  1.f), // 1
			//Vector( 1.f,  1.f, -1.f), // 2
			//Vector(-1.f,  1.f, -1.f), // 3
			//Vector(-1.f, -1.f,  1.f), // 4
			//Vector( 1.f, -1.f,  1.f), // 5
			//Vector( 1.f, -1.f, -1.f), // 6
			//Vector(-1.f, -1.f, -1.f)  // 7

		{
			Vector(-1.f,  1.f,  1.f), // 0
			Vector( 1.f,  1.f,  1.f), // 1
			Vector( 1.f, -1.f,  1.f), // 5
			Vector(-1.f, -1.f,  1.f)  // 4
		},

		{
			Vector( 1.f,  1.f,  1.f), // 1
			Vector( 1.f,  1.f, -1.f), // 2
			Vector( 1.f, -1.f, -1.f), // 6
			Vector( 1.f, -1.f,  1.f)  // 5
		},

		{
			Vector( 1.f,  1.f, -1.f), // 2
			Vector(-1.f,  1.f, -1.f), // 3
			Vector(-1.f, -1.f, -1.f), // 7
			Vector( 1.f, -1.f, -1.f)  // 6
		},

		{
			Vector(-1.f,  1.f, -1.f), // 3
			Vector(-1.f,  1.f,  1.f), // 0
			Vector(-1.f, -1.f,  1.f), // 4
			Vector(-1.f, -1.f, -1.f)  // 7
		},

		{
			Vector(-1.f,  1.f, -1.f), // 3
			Vector( 1.f,  1.f, -1.f), // 2
			Vector( 1.f,  1.f,  1.f), // 1
			Vector(-1.f,  1.f,  1.f)  // 0
		},

		{
			Vector(-1.f, -1.f,  1.f), // 4
			Vector( 1.f, -1.f,  1.f), // 5
			Vector( 1.f, -1.f, -1.f), // 6
			Vector(-1.f, -1.f, -1.f)  // 7
		}
	};

	char const * const sideNames[6] =
	{
		"front",
		"right",
		"back",
		"left",
		"top",
		"bottom"
	};

	for (int i = 0; i < 6; ++i)
	{
		char buffer[MAX_PATH];
		IGNORE_RETURN(snprintf(buffer, sizeof(buffer) - 1, "texture/%s_%s.dds", textureNameMask, sideNames[i]));
		Texture const * const texture = TextureList::fetch(buffer);
		m_localShaderPrimitiveList->push_back(new LocalShaderPrimitive(*this, texture, &vertices[i][0]));
		texture->release();
	}

	m_dpvsObject = RenderWorld::createUnboundedObject(this);
}

// ----------------------------------------------------------------------

SkyBox6SidedAppearance::~SkyBox6SidedAppearance()
{
	std::for_each(m_localShaderPrimitiveList->begin(), m_localShaderPrimitiveList->end(), PointerDeleter());
	delete m_localShaderPrimitiveList;
	IGNORE_RETURN(m_dpvsObject->release());
	m_dpvsObject = 0;
}

// ----------------------------------------------------------------------

DPVS::Object * SkyBox6SidedAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

// ----------------------------------------------------------------------

void SkyBox6SidedAppearance::render() const
{
	for (LocalShaderPrimitiveList::iterator iter = m_localShaderPrimitiveList->begin(); iter != m_localShaderPrimitiveList->end(); ++iter)
		ShaderPrimitiveSorter::add(**iter);
}

// ======================================================================

