// ======================================================================
//
// ShaderPrimitiveSet.cpp
// Copyright 2000 - 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/ShaderPrimitiveSet.h"

#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSetTemplate.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "clientGraphics/StaticIndexBuffer.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/Transform.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"

#include <vector>
#include <algorithm>

// ======================================================================

Tag const TAG_DOT3 = TAG(D,O,T,3);

// ======================================================================

class ShaderPrimitiveSet::LocalShaderPrimitive : public ShaderPrimitive
{
public:

	static void *operator new (size_t size);
	static void  operator delete (void* pointer);

	static void install();

public:

	LocalShaderPrimitive(const Appearance &owner, ShaderPrimitiveSet * pParent, const ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate &shaderPrimitiveTemplate);
	virtual ~LocalShaderPrimitive();

	virtual float               alter(float time);
	virtual const Vector        getPosition_w() const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader &prepareToView() const;
	virtual void                prepareToDraw() const;
	virtual void                draw() const;
	virtual float               getRadius() const;

	virtual void                setCustomizationData(CustomizationData *customizationData);
	virtual void                addCustomizationVariables(CustomizationData &customizationData) const;

	const Shader*               getShader () const;
	const ShaderTemplate*       getShaderTemplate () const;
	ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate const & getShaderPrimitiveTemplate() const;

	virtual void                getCostEstimate(int &numberOfVertices, int &numberOfTriangles, float &complexity) const;

	void                        setTexture(Tag tag, const Texture &texture);

	bool                        collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const;

private:

	static void remove();

private:

	static MemoryBlockManager *ms_memoryBlockManager;

private:

	const Appearance &                                                m_owner;
	const ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate &  m_template;
	Shader const *                                                    m_constShader;
	Shader *                                                          m_modifiableShader;
	ShaderPrimitiveSet *                                              m_parent;
#ifdef _DEBUG
	mutable bool                                                      m_badDrawFlag;
#endif

private:
	
	// Disabled.
	LocalShaderPrimitive();
	LocalShaderPrimitive(const LocalShaderPrimitive &); //lint -esym(754, LocalShaderPrimitive::LocalShaderPrimitive) // (Info -- local structure member 'LocalShaderPrimitive::LocalShaderPrimitive' not referenced) // Defensive hiding.
	LocalShaderPrimitive &operator =(const LocalShaderPrimitive &);
};

// ======================================================================

MemoryBlockManager *ShaderPrimitiveSet::LocalShaderPrimitive::ms_memoryBlockManager;

// ======================================================================

void ShaderPrimitiveSet::LocalShaderPrimitive::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("Already installed"));
	ms_memoryBlockManager = new MemoryBlockManager("ShaderPrimitiveSet::LocalShaderPrimitive::mbm", true, sizeof(LocalShaderPrimitive), 0, 0, 0);
	ExitChain::add(&remove, "ShaderPrimitiveSet::LocalShaderPrimitive::remove");
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSet::LocalShaderPrimitive::remove()
{
	DEBUG_FATAL(!ms_memoryBlockManager, ("Not installed"));
	delete ms_memoryBlockManager;
	ms_memoryBlockManager = NULL;
}

// ----------------------------------------------------------------------

void *ShaderPrimitiveSet::LocalShaderPrimitive::operator new (size_t size)
{
	DEBUG_FATAL (!ms_memoryBlockManager, ("ShaderPrimitiveSet::LocalShaderPrimitive is not installed"));

	// do not try to alloc a descendant class with this allocator
	UNREF (size);
	DEBUG_FATAL (size != sizeof (LocalShaderPrimitive), ("bad size"));

	return ms_memoryBlockManager->allocate ();
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSet::LocalShaderPrimitive::operator delete (void *pointer)
{
	DEBUG_FATAL (!ms_memoryBlockManager, ("ShaderPrimitiveSet::LocalShaderPrimitive is not installed"));
	ms_memoryBlockManager->free (pointer);
}

// ======================================================================

ShaderPrimitiveSet::LocalShaderPrimitive::LocalShaderPrimitive(const Appearance &owner, ShaderPrimitiveSet * parent, const ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate &shaderPrimitiveTemplate)
: ShaderPrimitive(),
	m_owner(owner),
	m_template(shaderPrimitiveTemplate),
	m_constShader(m_template.fetchShader()),
	m_modifiableShader(NULL),
	m_parent(parent)
#ifdef _DEBUG
	,
	m_badDrawFlag(false)
#endif
{
	// hack to set dot3 texture coordinate set properly for old art
	StaticShader const *staticShader = dynamic_cast<StaticShader const *>(m_constShader);
	if (staticShader && staticShader->addedDot3Placeholder())
	{
		StaticShader *modifiableShader = staticShader->fetchModifiable();
		const StaticVertexBuffer *vb = m_template.getVertexBuffer();
		NOT_NULL(vb);
		const int numberOfTextureCoordinates = vb->getNumberOfTextureCoordinateSets();
		if ((numberOfTextureCoordinates == 1 || numberOfTextureCoordinates  > 2) && vb->getTextureCoordinateSetDimension(numberOfTextureCoordinates-1) >= 3)
			modifiableShader->setTextureCoordinateSet(TAG_DOT3, static_cast<uint8>(numberOfTextureCoordinates-1));

		m_modifiableShader = modifiableShader;
		m_constShader->release();
		m_constShader = m_modifiableShader;
	}

	m_radius = shaderPrimitiveTemplate.getApproximateRadius();
}

// ----------------------------------------------------------------------

ShaderPrimitiveSet::LocalShaderPrimitive::~LocalShaderPrimitive()
{
	m_constShader->release(); //lint -esym(1540, LocalShaderPrimitive::m_constShader) // (Warning -- Pointer member 'LocalShaderPrimitive::m_constShader' neither freed nor zero'ed by destructor -- Effective C++ #6) // It's okay, this is reference counted. It is const so it can't be set to zero.
	m_modifiableShader = 0;
	m_parent = 0;
}

// ----------------------------------------------------------------------

float ShaderPrimitiveSet::LocalShaderPrimitive::alter(float elapsedTime)
{
	NOT_NULL(m_constShader);
	return m_constShader->alter(elapsedTime);
}

// ----------------------------------------------------------------------

const Vector ShaderPrimitiveSet::LocalShaderPrimitive::getPosition_w() const
{
	return m_owner.getTransform_w().getPosition_p();
}
	
//-------------------------------------------------------------------

float ShaderPrimitiveSet::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return m_owner.getTransform_w().getPosition_p().magnitudeBetweenSquared(ShaderPrimitiveSorter::getCurrentCameraPosition());
}

// ----------------------------------------------------------------------

int ShaderPrimitiveSet::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return m_template.getVertexBufferSortKey();
}

// ----------------------------------------------------------------------

const StaticShader &ShaderPrimitiveSet::LocalShaderPrimitive::prepareToView() const
{
	return m_constShader->prepareToView();
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSet::LocalShaderPrimitive::prepareToDraw() const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ShaderPrimitiveSet::LSP::prepareToDraw");

	Graphics::setObjectToWorldTransformAndScale(m_owner.getTransform_w(), m_owner.getScale());
	m_template.prepareToDraw(m_owner.getTransform_w());
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSet::LocalShaderPrimitive::draw() const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ShaderPrimitiveSet::LSP::draw");

#ifdef _DEBUG
	Graphics::setBadVertexBufferVertexShaderCombination(&m_badDrawFlag, m_owner.getAppearanceTemplate()->getName());
#endif

	m_template.draw();

#ifdef _DEBUG
	Graphics::setBadVertexBufferVertexShaderCombination(NULL, NULL);
#endif
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSet::LocalShaderPrimitive::setCustomizationData(CustomizationData *customizationData)
{
	if (m_constShader->obeysCustomizationData())
	{
		if (!m_modifiableShader)
		{
			m_modifiableShader = m_constShader->convertToModifiableShader();
			m_constShader = m_modifiableShader;
		}

		m_modifiableShader->setCustomizationData(customizationData);
	}
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSet::LocalShaderPrimitive::addCustomizationVariables(CustomizationData &customizationData) const
{
	m_constShader->addCustomizationVariables(customizationData);
}

// ----------------------------------------------------------------------

const Shader * ShaderPrimitiveSet::LocalShaderPrimitive::getShader () const
{
	return m_constShader;
}

// ----------------------------------------------------------------------

const ShaderTemplate* ShaderPrimitiveSet::LocalShaderPrimitive::getShaderTemplate () const
{
	NOT_NULL (m_constShader);
	return &m_constShader->getShaderTemplate ();
}

// ----------------------------------------------------------------------

ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate const & ShaderPrimitiveSet::LocalShaderPrimitive::getShaderPrimitiveTemplate() const
{
	return m_template;
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSet::LocalShaderPrimitive::getCostEstimate(int &numberOfVertices, int &numberOfTriangles, float &complexity) const
{
	m_template.getCostEstimate(numberOfVertices, numberOfTriangles, complexity);
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSet::LocalShaderPrimitive::setTexture(const Tag tag, const Texture &texture)
{
	StaticShader const * const staticShader = m_constShader->getStaticShader();

	if (staticShader)
	{
		if (staticShader->hasTexture(tag))
		{
			if (!m_modifiableShader)
			{
				m_modifiableShader = m_constShader->convertToModifiableShader();
				m_constShader = m_modifiableShader;
			}

			m_modifiableShader->getStaticShader()->setTexture(tag, texture);
		}
		else
		{
#ifdef _DEBUG
			char tagString[5];
			ConvertTagToString(tag, tagString);
			DEBUG_WARNING(true, ("Texture tag %s not found for shader %s", tagString, staticShader->getShaderTemplate().getName().getString ()));
#endif
		}
	}
}

// ----------------------------------------------------------------------

bool ShaderPrimitiveSet::LocalShaderPrimitive::collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const
{
	return m_template.collide(start_o, end_o, collideParameters, result);
}

//----------------------------------------------------------------------

float ShaderPrimitiveSet::LocalShaderPrimitive::getRadius() const
{
	return m_radius * m_owner.getScale().x;
}

// ======================================================================

void ShaderPrimitiveSet::install()
{
	LocalShaderPrimitive::install();
}

// ======================================================================

ShaderPrimitiveSet::ShaderPrimitiveSet(const Appearance &owner, const ShaderPrimitiveSetTemplate &shaderPrimitiveSetTemplate) :
	m_primitives(new Primitives),
	m_localCameraPos()
{
	m_primitives->reserve(shaderPrimitiveSetTemplate.m_localShaderPrimitiveTemplateList->size());

	const ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplateList::iterator end = shaderPrimitiveSetTemplate.m_localShaderPrimitiveTemplateList->end();
	for (ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplateList::iterator i = shaderPrimitiveSetTemplate.m_localShaderPrimitiveTemplateList->begin(); i != end; ++i)
		m_primitives->push_back(new LocalShaderPrimitive(owner, this, **i));
}

// ----------------------------------------------------------------------

ShaderPrimitiveSet::~ShaderPrimitiveSet()
{
	std::for_each(m_primitives->begin(), m_primitives->end(), PointerDeleter());
	delete m_primitives;
}

// ----------------------------------------------------------------------

float ShaderPrimitiveSet::alter(float elapsedTime)
{
	float alterResult = AlterResult::cms_kill;

	const Primitives::iterator end = m_primitives->end();
	for (Primitives::iterator i = m_primitives->begin(); i != end; ++i)
	{
		float const primitiveAlterResult = (*i)->alter(elapsedTime);
		alterResult = std::min(primitiveAlterResult, alterResult);
	}

	return alterResult;
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSet::add()
{
	const Primitives::iterator end = m_primitives->end();
	for (Primitives::iterator i = m_primitives->begin(); i != end; ++i)
		ShaderPrimitiveSorter::add(**i);
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSet::addWithAlphaFadeOpacity(bool const opaqueEnabled, float const opaqueAlpha, bool const alphaEnabled, float const alphaAlpha)
{
	const Primitives::iterator end = m_primitives->end();
	for (Primitives::iterator i = m_primitives->begin(); i != end; ++i)
		ShaderPrimitiveSorter::addWithAlphaFadeOpacity(**i, opaqueEnabled, opaqueAlpha, alphaEnabled, alphaAlpha);
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSet::setCustomizationData(CustomizationData *customizationData)
{
	const Primitives::iterator end = m_primitives->end();
	for (Primitives::iterator i = m_primitives->begin(); i != end; ++i)
	{
		NOT_NULL(*i);
		(*i)->setCustomizationData(customizationData);
	}
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSet::addCustomizationVariables(CustomizationData &customizationData)
{
	const Primitives::iterator end = m_primitives->end();
	for (Primitives::iterator i = m_primitives->begin(); i != end; ++i)
	{
		NOT_NULL(*i);
		(*i)->addCustomizationVariables(customizationData);
	}
}

// ----------------------------------------------------------------------

int ShaderPrimitiveSet::getNumberOfShaders () const
{
	NOT_NULL (m_primitives);
	return static_cast<int> (m_primitives->size ());
}

// ----------------------------------------------------------------------

const Shader* ShaderPrimitiveSet::getShader (int shaderIndex) const
{
	NOT_NULL (m_primitives);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, shaderIndex, getNumberOfShaders ());

	return (*m_primitives) [static_cast<uint> (shaderIndex)]->getShader ();
}

// ----------------------------------------------------------------------

const ShaderTemplate* ShaderPrimitiveSet::getShaderTemplate (int shaderIndex) const
{
	NOT_NULL (m_primitives);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, shaderIndex, getNumberOfShaders ());

	return (*m_primitives) [static_cast<uint> (shaderIndex)]->getShaderTemplate ();
}

// ----------------------------------------------------------------------

const StaticVertexBuffer* ShaderPrimitiveSet::getVertexBuffer (int shaderIndex) const
{
	NOT_NULL (m_primitives);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, shaderIndex, getNumberOfShaders ());

	LocalShaderPrimitive const * const localShaderprimitive = (*m_primitives)[static_cast<size_t>(shaderIndex)];
	return localShaderprimitive->getShaderPrimitiveTemplate().getVertexBuffer();
}

// ----------------------------------------------------------------------

const StaticIndexBuffer* ShaderPrimitiveSet::getIndexBuffer (int shaderIndex) const
{
	NOT_NULL (m_primitives);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, shaderIndex, getNumberOfShaders ());

	LocalShaderPrimitive const * const localShaderprimitive = (*m_primitives)[static_cast<size_t>(shaderIndex)];
	return localShaderprimitive->getShaderPrimitiveTemplate().getIndexBuffer();
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSet::getCostEstimate(int &numberOfVertices, int &numberOfTriangles, float &complexity) const
{
	const Primitives::iterator end = m_primitives->end();
	for (Primitives::iterator i = m_primitives->begin(); i != end; ++i)
		(*i)->getCostEstimate(numberOfVertices, numberOfTriangles, complexity);
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSet::setTexture(const Tag tag, const Texture &texture)
{
	const Primitives::iterator end = m_primitives->end();
	for (Primitives::iterator i = m_primitives->begin(); i != end; ++i)
		(*i)->setTexture(tag, texture);
}

// ----------------------------------------------------------------------

bool ShaderPrimitiveSet::collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const
{
	bool collided = false;

	result.setPoint(end_o);

	Primitives::iterator const end = m_primitives->end();
	for (Primitives::iterator iter = m_primitives->begin(); iter != end; ++iter)
		if ((*iter)->collide(start_o, result.getPoint (), collideParameters, result))
			collided = true;
		
	return collided;
}

// ======================================================================
