//
// BeamAppearanceance.cpp
// asommers 6-22-99
//
// copyright 1999, bootprint entertainment
//
// @todo Should consider a beam manager to batch these, but there are sorting implications
// ----------------------------------------------------------------------

#include "clientObject/FirstClientObject.h"
#include "clientObject/BeamAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Transform.h"
#include "sharedMath/VectorArgb.h"
#include "sharedMath/Sphere.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/Object.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include "dpvsModel.hpp"
#include "dpvsObject.hpp"

#include <vector>
#include <string>

// ======================================================================

// @todo memory block manage this guy
namespace BeamAppearanceNamespace
{
	std::string const              ms_alternateShaderDataTable = "datatables/appearance/alternate_lightsaber_shaders.iff";
	std::vector<Shader*>           ms_alternateShaders;

	void LoadAlternateShaders();
}

using namespace BeamAppearanceNamespace;

class BeamAppearance::LocalShaderPrimitive : public ShaderPrimitive
{
public:

	static VertexBufferFormat getVertexBufferFormat();

public:

	LocalShaderPrimitive(const BeamAppearance &owner, float newLength, float newWidth, Shader* newShader, const VectorArgb& newColor);
	virtual ~LocalShaderPrimitive();

	virtual float               alter(float time);
	virtual const Vector        getPosition_w() const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader &prepareToView() const;
	virtual void                prepareToDraw() const;
	virtual void                draw() const;

	const Sphere               &getSphere() const;
	void                        setCustomizationData(CustomizationData *customizationData);
	void                        addCustomizationVariables(CustomizationData &customizationData) const;

	void                        setLength(float length);
	void                        setWidth(float width);
	void                        setAlpha(uint8 alpha);
	void						useBaseShader();
	void						useAlternateShader(int index);

	float                       getWidth() const;
	float                       getLength() const;

private:

	const BeamAppearance        &m_owner;
	Shader                      *m_shader;
	Shader						*m_baseShader;
	mutable DynamicVertexBuffer  m_vertexBuffer;
	float                        m_length;
	float                        m_width;
	Sphere                       m_sphere;
	PackedArgb                   m_color;

private:

	LocalShaderPrimitive();
	LocalShaderPrimitive(const LocalShaderPrimitive &);             //lint -esym(754, LocalShaderPrimitive::LocalShaderPrimitive) // not referenced // defensive hiding
	LocalShaderPrimitive &operator =(const LocalShaderPrimitive &);
};

//===================================================================

VertexBufferFormat BeamAppearance::LocalShaderPrimitive::getVertexBufferFormat()
{
	VertexBufferFormat format;
	format.setPosition();
	format.setColor0();
	format.setNumberOfTextureCoordinateSets(1);
	format.setTextureCoordinateSetDimension(0, 2);
	return format;
}

//===================================================================

BeamAppearance::LocalShaderPrimitive::LocalShaderPrimitive(const BeamAppearance &owner, float newLength, float newWidth, Shader* newShader, const VectorArgb& newColor)
: ShaderPrimitive(),
	m_owner(owner),
	m_shader(NULL),
	m_baseShader(newShader),
	m_vertexBuffer(getVertexBufferFormat()),
	m_length(newLength),
	m_width(newWidth),
	m_sphere(),
	m_color(newColor)
{
	m_sphere.setCenter(0.0f, 0.0f, m_length * 0.5f);
	m_sphere.setRadius(sqrt(sqr(m_length * 0.5f) + sqr(m_width)));


	m_shader = m_baseShader;
}

// ----------------------------------------------------------------------

BeamAppearance::LocalShaderPrimitive::~LocalShaderPrimitive()
{
	//m_shader->release();
	m_shader = 0;

	m_baseShader->release();
	m_baseShader = 0;

}

// ----------------------------------------------------------------------

float BeamAppearance::LocalShaderPrimitive::alter(float time)
{
	return m_shader->alter(time);
}

// ----------------------------------------------------------------------

const Vector BeamAppearance::LocalShaderPrimitive::getPosition_w() const
{
	return m_owner.getTransform_w().getPosition_p();
}
	
//-------------------------------------------------------------------

float BeamAppearance::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return m_owner.getTransform_w().getPosition_p().magnitudeBetweenSquared(ShaderPrimitiveSorter::getCurrentCameraPosition());
}

// ----------------------------------------------------------------------

int BeamAppearance::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return m_vertexBuffer.getSortKey();
}

// ----------------------------------------------------------------------

const StaticShader &BeamAppearance::LocalShaderPrimitive::prepareToView() const
{
	return m_shader->prepareToView();
}

// ----------------------------------------------------------------------

void BeamAppearance::LocalShaderPrimitive::prepareToDraw() const
{
	//-- use objectToWorld as a hint to know how much to roll
	Transform transform = m_owner.getTransform_w();

	Vector camPos    = ShaderPrimitiveSorter::getCurrentCameraPosition();
	Vector objPos    = transform.rotateTranslate_l2p(m_sphere.getCenter());

	camPos   -= objPos;
	camPos    = transform.rotate_p2l(camPos);
	camPos.z -= m_length * 0.5f;
	camPos.z  = camPos.y;

	transform.roll_l(-camPos.theta());

	//-- setup vertexarray for drawing individual polygons
	m_vertexBuffer.lock(4);

		VertexBufferWriteIterator v = m_vertexBuffer.begin();

		v.setPosition(m_width, 0.0f, m_length);
		v.setTextureCoordinates(0, 0.0f, 0.0f);
		v.setColor0(m_color);
		++v;

		v.setPosition(m_width, 0.0f, 0.0f);
		v.setTextureCoordinates(0, 1.0f , 0.0f);
		v.setColor0(m_color);
		++v;

		v.setPosition(-m_width, 0.0f, 0.0f);
		v.setTextureCoordinates(0, 1.0f, 1.0f);
		v.setColor0(m_color);
		++v;

		v.setPosition(-m_width, 0.0f, m_length);
		v.setTextureCoordinates(0, 0.0f, 1.0f);
		v.setColor0(m_color);

	m_vertexBuffer.unlock();

	Graphics::setObjectToWorldTransformAndScale(transform, Vector::xyz111);
	Graphics::setVertexBuffer(m_vertexBuffer);
}

// ----------------------------------------------------------------------

void BeamAppearance::LocalShaderPrimitive::draw() const
{
	Graphics::drawTriangleFan();

	// @todo do a distance check here?
	// draw left length-wise edge as a line just in case the poly is really far away --- fixes slivered poly rasterization ugliness
	Graphics::drawLineList(0, 1);
}

// ----------------------------------------------------------------------

inline void BeamAppearance::LocalShaderPrimitive::setLength(float newLength)
{
	m_length = newLength;
}

// ----------------------------------------------------------------------

inline void BeamAppearance::LocalShaderPrimitive::setWidth(float newWidth)
{
	m_width = newWidth;
}

// ----------------------------------------------------------------------

inline float BeamAppearance::LocalShaderPrimitive::getWidth() const
{
	return m_width;
}

// ----------------------------------------------------------------------

inline float BeamAppearance::LocalShaderPrimitive::getLength() const
{
	return m_length;
}

// ----------------------------------------------------------------------

inline void BeamAppearance::LocalShaderPrimitive::setAlpha(uint8 alpha)
{
	m_color.setA(alpha);
}

// ----------------------------------------------------------------------

inline void BeamAppearance::LocalShaderPrimitive::useBaseShader()
{
	m_shader = m_baseShader;
}

inline void BeamAppearance::LocalShaderPrimitive::useAlternateShader(int index)
{
	if(ms_alternateShaders.empty() || ms_alternateShaders.size() < static_cast<std::vector<std::string>::size_type>(index) )
	{
		DEBUG_WARNING(true, ("Tried to use alternate Lightsaber shader but index was out of valid range. Index = %d. Reverting to base shader.", index));
		useBaseShader();
	}
	// Our index is base 1, we need it to be base 0 for our vector.
	if(ms_alternateShaders[index-1])
		m_shader = ms_alternateShaders[index-1];
	else
		useBaseShader();
}

// ----------------------------------------------------------------------

inline const Sphere &BeamAppearance::LocalShaderPrimitive::getSphere() const
{
	return m_sphere;
}

// ----------------------------------------------------------------------

inline void BeamAppearance::LocalShaderPrimitive::setCustomizationData(CustomizationData *customizationData)
{
	if (m_shader)
		m_shader->setCustomizationData(customizationData);
}

// ----------------------------------------------------------------------

inline void BeamAppearance::LocalShaderPrimitive::addCustomizationVariables(CustomizationData &customizationData) const
{
	if (m_shader)
		m_shader->addCustomizationVariables(customizationData);
}

// ======================================================================

MemoryBlockManager* BeamAppearance::ms_memoryBlockManager;

// ======================================================================

void BeamAppearance::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("BeamAppearance already installed"));
	ms_memoryBlockManager = new MemoryBlockManager("BeamAppearance::memoryBlockManager", true, sizeof(BeamAppearance), 0, 0, 0);

	LoadAlternateShaders();

	ExitChain::add (remove, "BeamAppearance::remove");
}

// ----------------------------------------------------------------------

void BeamAppearance::remove()
{
	DEBUG_FATAL(!ms_memoryBlockManager, ("BeamAppearance is not installed"));

	std::vector<Shader*>::size_type i = 0;
	for(; i < ms_alternateShaders.size(); ++i )
	{
		if(ms_alternateShaders[i])
			ms_alternateShaders[i]->release();
	}
	ms_alternateShaders.clear();

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

void *BeamAppearance::operator new(size_t size)
{
	UNREF (size);
	NOT_NULL(ms_memoryBlockManager);
	DEBUG_FATAL(size != sizeof(BeamAppearance), ("bad size"));
	DEBUG_FATAL(size != static_cast<size_t>(ms_memoryBlockManager->getElementSize()), ("installed with bad size"));
	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void BeamAppearance::operator delete (void *pointer)
{
	NOT_NULL(ms_memoryBlockManager);
	ms_memoryBlockManager->free(pointer);
}

// ======================================================================

BeamAppearance::BeamAppearance(float newLength, float newWidth, Shader *newShader, const VectorArgb &newColor)
: Appearance(0),
	m_localShaderPrimitive(NULL),
	m_dpvsObject(NULL)
{
	m_localShaderPrimitive = new LocalShaderPrimitive(*this, newLength, newWidth, newShader, newColor);
	updateDpvsTestModel();
	NOT_NULL(m_dpvsObject);
}

// ----------------------------------------------------------------------

BeamAppearance::~BeamAppearance()
{
	delete m_localShaderPrimitive;
	IGNORE_RETURN(m_dpvsObject->release());
}

// ----------------------------------------------------------------------

const Sphere &BeamAppearance::getSphere() const
{
	return m_localShaderPrimitive->getSphere();
}

// ----------------------------------------------------------------------

void BeamAppearance::setLength(float newLength)
{
	m_localShaderPrimitive->setLength(newLength);
	updateDpvsTestModel();
}

// ----------------------------------------------------------------------

void BeamAppearance::setWidth(float newWidth)
{
	m_localShaderPrimitive->setWidth(newWidth);
}

// ----------------------------------------------------------------------

void BeamAppearance::updateDpvsTestModel()
{
	const float width = m_localShaderPrimitive->getWidth();
	const float length = m_localShaderPrimitive->getLength();

	AxialBox box;
	box.setMin(Vector(-width, -width, 0.0f));
	box.setMax(Vector( width,  width, length));

	if (m_dpvsObject)
	{
		DPVS::OBBModel *const model = RenderWorld::fetchBoxModel(box);
		m_dpvsObject->setTestModel(model);
		IGNORE_RETURN(model->release());
	}
	else
		m_dpvsObject = RenderWorld::createObject(this, box);
}

// ----------------------------------------------------------------------

DPVS::Object * BeamAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

// ----------------------------------------------------------------------

float BeamAppearance::alter(float time)
{
	return m_localShaderPrimitive->alter(time);
}

// ----------------------------------------------------------------------

void BeamAppearance::setCustomizationData(CustomizationData *customizationData)
{
	m_localShaderPrimitive->setCustomizationData(customizationData);
}

// ----------------------------------------------------------------------

void BeamAppearance::addCustomizationVariables(CustomizationData &customizationData) const
{
	m_localShaderPrimitive->addCustomizationVariables(customizationData);
}

// ----------------------------------------------------------------------

void BeamAppearance::render() const
{
	ShaderPrimitiveSorter::add(*m_localShaderPrimitive);
}

// ----------------------------------------------------------------------

void BeamAppearance::setAlpha(bool const /* opaqueEnabled */, float const /* opaqueAlpha */, bool const /* alphaEnabled */, float const alphaAlpha)
{
	m_localShaderPrimitive->setAlpha(static_cast<uint8>(alphaAlpha * 255.0f)); 
}

// ----------------------------------------------------------------------

void BeamAppearance::useBaseShader()
{
	m_localShaderPrimitive->useBaseShader();
}

// ----------------------------------------------------------------------

void BeamAppearance::useAlternateShader(int index)
{
	m_localShaderPrimitive->useAlternateShader(index);
}

// ----------------------------------------------------------------------

void BeamAppearanceNamespace::LoadAlternateShaders()
{
	DataTable * altShaders = DataTableManager::getTable(ms_alternateShaderDataTable, true);

	for( int i = 0; i < altShaders->getNumRows(); ++i)
	{
		std::string newShaderString = altShaders->getStringValue(0, i);

		if(newShaderString.empty())
		{
			DEBUG_WARNING(true,("Empty string specified for alternate Lightsaber shader on row [%d]", i));
			ms_alternateShaders.push_back(NULL);
		}

		const ShaderTemplate *shaderTemplate = 0;
		shaderTemplate = ShaderTemplateList::fetch(newShaderString.c_str());
		if (shaderTemplate)
		{
			Shader * newShader = shaderTemplate->fetchModifiableShader();
			ms_alternateShaders.push_back(newShader);
			shaderTemplate->release();
		}
		else
		{
			DEBUG_WARNING(true, ("Failed to find alternate Lightsaber shader [%s]", newShaderString.c_str()));
		}

	}

}

// ======================================================================
