// ======================================================================
//
// DrawTextureCommandElement.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "DrawTextureCommandElement.h"

#include "BlueprintWriter.h"
#include "BoolElement.h"
#include "DialogEnterName.h"
#include "ElementTypeIndex.h"
#include "Exporter.h"
#include "HueElement.h"
#include "HueModeElement.h"
#include "ImageSlotElement.h"
#include "BasicRangedIntVariableElement.h"
#include "PaletteColorVariableElement.h"
#include "RegionElement.h"
#include "TextureSourceModeElement.h"
#include "TextureWriteModeElement.h"
#include "VariableDefinitionElement.h"

#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Material.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/SystemVertexBuffer.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientGraphics/VertexBuffer.h"
#include "clientObject/ObjectListCamera.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/PaletteArgb.h"
#include "sharedMath/PaletteArgbList.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/MemoryBlockManagedObject.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"

#include <algorithm>
#include <vector>
#include <string>

// ======================================================================

namespace DrawTextureCommandElementNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	std::string const  cs_disableElementText("disable drawing");
	std::string const  cs_disableElementTrue("disabled");
	std::string const  cs_disableElementFalse("enabled");

	const char *const  ms_copySourceShaderName     = "shader/texren_copy_c1a1.sht";
	const char *const  ms_alphaBlendShaderName     = "shader/texren_alphablend_c1a1.sht";
	const char *const  ms_blend2OverwriteShaderName = "shader/texren_blend2-overwrite_c1a1_c2a2.sht";

	const float        ms_defaultNormalizedLength = 1.0f;

	const float        ms_defaultDepth            = 3.0f;
	const float        ms_defaultCameraNearPlane  = 0.1f;
	const float        ms_defaultCameraFarPlane   = 1000.0f;

	const int          ms_imageSlotsPerSTM[] =
		{
			1, // STM_texture1,
			2  // STM_textureBlend2	
		};
	const int          ms_imageSlotsPerSTMCount = sizeof(ms_imageSlotsPerSTM) / sizeof(ms_imageSlotsPerSTM[0]);

	enum VariableTypeId
	{
		VTID_blendFactor
	};

	const char *const  ms_variableDefinitionDescriptions[] =
		{
			"Blend Factor" // VTID_blendFactor
		};

	const float ms_uValues[][4] =
		{
			// normal
			{
				0.0f,
				1.0f,
				1.0f,
				0.0f
			},
			// reversed
			{
				1.0f,
				0.0f,
				0.0f,
				1.0f
			}
		};

	const float ms_vValues[][4] =
		{
			// normal
			{
				0.0f,
				0.0f,
				1.0f,
				1.0f
			},
			// reversed
			{
				1.0f,
				1.0f,
				0.0f,
				0.0f
			}
		};

	const Tag  TAG_BLN0 = TAG(B,L,N,0);
	const Tag  TAG_BLN1 = TAG(B,L,N,1);
	const Tag  TAG_BLND = TAG(B,L,N,D);
	const Tag  TAG_ECDT = TAG(E,C,D,T);
	const Tag  TAG_MAIN = TAG(M,A,I,N);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool                      ms_installed;

	StaticShader             *ms_copySourceShader;
	StaticShader             *ms_alphaBlendShader;
	StaticShader             *ms_blend2OverwriteShader;

	DynamicVertexBuffer      *ms_dynamicVbOneTcSet;
	DynamicVertexBuffer      *ms_dynamicVbTwoTcSet;

	const Texture            *ms_defaultTexture;

	ObjectListCamera         *ms_camera;
}

using namespace DrawTextureCommandElementNamespace;

// ======================================================================
// class DrawTextureCommandElement
// ======================================================================

void DrawTextureCommandElement::install()
{
	DEBUG_FATAL(ms_installed, ("DrawTextureCommandElement already installed"));

	ExitChain::add(remove, "DrawTextureCommandElement");

	//-- load shaders
	ms_copySourceShader = dynamic_cast<StaticShader*>(ShaderTemplateList::fetchModifiableShader(ms_copySourceShaderName));
	NOT_NULL(ms_copySourceShader);

	ms_alphaBlendShader = dynamic_cast<StaticShader*>(ShaderTemplateList::fetchModifiableShader(ms_alphaBlendShaderName));
	NOT_NULL(ms_alphaBlendShader);

	ms_blend2OverwriteShader = dynamic_cast<StaticShader*>(ShaderTemplateList::fetchModifiableShader(ms_blend2OverwriteShaderName));
	NOT_NULL(ms_blend2OverwriteShader);

	//-- create vertex buffers
	VertexBufferFormat formatOneTcSet;
	formatOneTcSet.setPosition();
	formatOneTcSet.setNumberOfTextureCoordinateSets(1);
	formatOneTcSet.setTextureCoordinateSetDimension(0, 2);
	formatOneTcSet.setTextureCoordinateSetDimension(1, 2);
	ms_dynamicVbOneTcSet = new DynamicVertexBuffer(formatOneTcSet);

	VertexBufferFormat formatTwoTcSet;
	formatTwoTcSet.setPosition();
	formatTwoTcSet.setNumberOfTextureCoordinateSets(2);
	formatTwoTcSet.setTextureCoordinateSetDimension(0, 2);
	formatTwoTcSet.setTextureCoordinateSetDimension(1, 2);
	ms_dynamicVbTwoTcSet = new DynamicVertexBuffer(formatTwoTcSet);

	ms_defaultTexture = TextureList::fetchDefaultTexture();

	//-- create the camera
	ms_camera = new ObjectListCamera(1);
	ms_camera->setNearPlane(ms_defaultCameraNearPlane);
	ms_camera->setFarPlane(ms_defaultCameraFarPlane);

	ms_installed = true;
}

// ----------------------------------------------------------------------

void DrawTextureCommandElement::remove()
{
	DEBUG_FATAL(!ms_installed, ("DrawTextureCommandElement not installed"));

	delete ms_camera;
	ms_camera = 0;

	ms_defaultTexture->release();
	ms_defaultTexture = 0;

	delete ms_dynamicVbOneTcSet;
	ms_dynamicVbOneTcSet = 0;

	delete ms_dynamicVbTwoTcSet;
	ms_dynamicVbTwoTcSet = 0;

	ms_blend2OverwriteShader->release();
	ms_blend2OverwriteShader = 0;

	ms_alphaBlendShader->release();
	ms_alphaBlendShader = 0;

	ms_copySourceShader->release();
	ms_copySourceShader = 0;

	ms_installed = false;
}

// ----------------------------------------------------------------------

bool DrawTextureCommandElement::isPersistedNext(const Iff &iff)
{
	return (iff.isCurrentForm() && (iff.getCurrentName() == TAG_ECDT));
}

// ======================================================================
// class DrawTextureCommandElement: public member functions
// ======================================================================

DrawTextureCommandElement::DrawTextureCommandElement() :
	CommandElement(),
	m_hueMode(HM_none),
	m_hueElementCount(0),
	m_sourceTextureMode(STM_texture1),
	m_writeTextureMode(WTM_overwrite),
	m_imageSlotCount(1),
	m_commandDescription(new std::string()),
	m_disableRender(false),
	m_shader(NULL)
{
	DEBUG_FATAL(!ms_installed, ("DrawTextureCommandElement not installed"));

	//-- add children elements
	ElementVector &children = getChildren();

	children.push_back(new BoolElement(m_disableRender, cs_disableElementText, cs_disableElementTrue, cs_disableElementFalse));
	children.push_back(new HueModeElement(*this));
	children.push_back(new TextureSourceModeElement(*this));
	children.push_back(new TextureWriteModeElement(*this));
	children.push_back(new RegionElement("Destion Region", 0.0f, 0.0f, 1.0f, 1.0f));
	children.push_back(new ImageSlotElement());

	createShader();
}

// ----------------------------------------------------------------------

DrawTextureCommandElement::DrawTextureCommandElement(Iff &iff, ElementVector &sourceTextures) :
	CommandElement(),
	m_hueMode(HM_none),
	m_hueElementCount(0),
	m_sourceTextureMode(STM_texture1),
	m_writeTextureMode(WTM_overwrite),
	m_imageSlotCount(1),
	m_commandDescription(new std::string()),
	m_disableRender(false),
	m_shader(NULL)
{
	//-- add children elements
	ElementVector &children = getChildren();

	children.push_back(new BoolElement(m_disableRender, cs_disableElementText, cs_disableElementTrue, cs_disableElementFalse));
	children.push_back(new HueModeElement(*this));
	children.push_back(new TextureSourceModeElement(*this));
	children.push_back(new TextureWriteModeElement(*this));

	//-- load from iff
	iff.enterForm(TAG_ECDT);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				load_0000(iff, sourceTextures);
				break;

			case TAG_0001:
				load_0001(iff, sourceTextures);
				break;

			default:
				{
					char version[5];
					ConvertTagToString(iff.getCurrentName(), version);
					FATAL(true, ("unsupported version [%s]", version));
				}
		}

	iff.exitForm(TAG_ECDT);

	createShader();
}

// ----------------------------------------------------------------------

DrawTextureCommandElement::~DrawTextureCommandElement()
{
	delete m_commandDescription;
	if (m_shader)
	{
		m_shader->release();
		m_shader = NULL;
	}
}

// ----------------------------------------------------------------------

void DrawTextureCommandElement::load_0000(Iff &iff, ElementVector &sourceTextures)
{
	ElementVector &children = getChildren();

	iff.enterForm(TAG_0000);

		//-- read basic info
		iff.enterChunk(TAG_INFO);
		{
			m_sourceTextureMode = static_cast<SourceTextureMode>(iff.read_int32());
			m_writeTextureMode  = static_cast<WriteTextureMode>(iff.read_int32());

			m_imageSlotCount    = static_cast<int>(iff.read_int32());
		}
		iff.exitChunk(TAG_INFO);

		//-- read region element
		children.push_back(new RegionElement(iff));

		//-- read image slots
		for (int i = 0; i < m_imageSlotCount; ++i)
		{
			children.push_back(new ImageSlotElement(iff, sourceTextures));
		}

		VariableDefinitionElement::loadVariableDefinitions(iff, getChildren());
	
		//-- read command description
		iff.enterChunk(TAG_NAME);
		{
			char buffer[1024];

			iff.read_string(buffer, sizeof(buffer)-1);
			IGNORE_RETURN(m_commandDescription->assign(buffer));

		}
		iff.exitChunk(TAG_NAME);

	iff.exitForm(TAG_0000);

#if 1
	// -TRF- hack for some bogus data handling going on elsewhere.
	//       somehow I'm deleting one of the necessary image slots.
	//       add one if we need more.
	const int requiredImageSlotCount = ms_imageSlotsPerSTM[static_cast<uint>(m_sourceTextureMode)];
	if (requiredImageSlotCount != m_imageSlotCount)
	{
		WARNING(true, ("required image slots = %d, image slots on disk = %d, adjusting [command: %s]\n", requiredImageSlotCount, m_imageSlotCount, m_commandDescription->c_str()));
		FATAL(requiredImageSlotCount < m_imageSlotCount, ("we don't handle deleting extra image slots right now"));

		for (int j = m_imageSlotCount; j < requiredImageSlotCount; ++j)
		{
			children.push_back(new ImageSlotElement());
		}

		m_imageSlotCount = requiredImageSlotCount;
	}
#endif
}

// ----------------------------------------------------------------------

void DrawTextureCommandElement::load_0001(Iff &iff, ElementVector &sourceTextures)
{
	ElementVector &children = getChildren();

	iff.enterForm(TAG_0001);

		//-- read basic info
		iff.enterChunk(TAG_INFO);
		{
			m_hueMode           = static_cast<HueMode>(iff.read_int32());
			m_sourceTextureMode = static_cast<SourceTextureMode>(iff.read_int32());
			m_writeTextureMode  = static_cast<WriteTextureMode>(iff.read_int32());

			m_imageSlotCount    = static_cast<int>(iff.read_int16());
			m_hueElementCount   = static_cast<int>(iff.read_int16());
		}
		iff.exitChunk(TAG_INFO);

		//-- read region element
		children.push_back(new RegionElement(iff));

		//-- Read image slots.
		{
			for (int i = 0; i < m_imageSlotCount; ++i)
				children.push_back(new ImageSlotElement(iff, sourceTextures));
		}

		VariableDefinitionElement::loadVariableDefinitions(iff, getChildren());
	
		//-- Read hue elements.
		{
			for (int i = 0; i < m_hueElementCount; ++i)
				children.push_back(new HueElement(iff));
		}

		//-- read command description
		iff.enterChunk(TAG_NAME);
		{
			char buffer[1024];

			iff.read_string(buffer, sizeof(buffer)-1);
			IGNORE_RETURN(m_commandDescription->assign(buffer));

		}
		iff.exitChunk(TAG_NAME);

	iff.exitForm(TAG_0001);

	// @todo figure out if this code is still needed.  It was needed back in version 0000 days at one point,
	//       but the problem may have been resolved, rendering this code useless.
#if 1
	// -TRF- hack for some bogus data handling going on elsewhere.
	//       somehow I'm deleting one of the necessary image slots.
	//       add one if we need more.
	const int requiredImageSlotCount = ms_imageSlotsPerSTM[static_cast<uint>(m_sourceTextureMode)];
	if (requiredImageSlotCount != m_imageSlotCount)
	{
		WARNING(true, ("required image slots = %d, image slots on disk = %d, adjusting [command: %s]\n", requiredImageSlotCount, m_imageSlotCount, m_commandDescription->c_str()));
		FATAL(requiredImageSlotCount < m_imageSlotCount, ("we don't handle deleting extra image slots right now"));

		for (int j = m_imageSlotCount; j < requiredImageSlotCount; ++j)
		{
			children.push_back(new ImageSlotElement());
		}

		m_imageSlotCount = requiredImageSlotCount;
	}
#endif
}

// ----------------------------------------------------------------------

bool DrawTextureCommandElement::writeForWorkspace(Iff &iff) const
{
	iff.insertForm(TAG_ECDT);	
		iff.insertForm(TAG_0001);

			//-- write basic info
			iff.insertChunk(TAG_INFO);
			{
				iff.insertChunkData(static_cast<int32>(m_hueMode));
				iff.insertChunkData(static_cast<int32>(m_sourceTextureMode));
				iff.insertChunkData(static_cast<int32>(m_writeTextureMode));
				iff.insertChunkData(static_cast<int16>(m_imageSlotCount));
				iff.insertChunkData(static_cast<int16>(m_hueElementCount));
			}
			iff.exitChunk(TAG_INFO);

			//-- write region element
			const RegionElement &regionElement = getRegionElement();
			const bool reSuccess = regionElement.writeForWorkspace(iff);
			if (!reSuccess)
				return false;

			//-- write image slots
			{
				for (int i = 0; i < m_imageSlotCount; ++i)
				{
					const ImageSlotElement &imageSlotElement = *NON_NULL(getImageSlotElement(i));
					const bool iseSuccess = imageSlotElement.writeForWorkspace(iff);
					if (!iseSuccess)
						return false;
				}
			}

			//-- write variable definitions
			if (!VariableDefinitionElement::writeVariableDefinitions(iff, getChildren()))
				return false;

			//-- Write HueElement data.
			{
				for (int i = 0; i < m_hueElementCount; ++i)
				{
					const HueElement *const hueElement = getHueElement(i);
					NOT_NULL(hueElement);

					const bool success = hueElement->writeForWorkspace(iff);
					if (!success)
					{
						DEBUG_WARNING(true, ("Failed to write HueElement [index = %d].", i));
						return false;
					}
				}			
			}

		//-- write command description
		iff.insertChunk(TAG_NAME);
			iff.insertChunkString(m_commandDescription->c_str());
		iff.exitChunk(TAG_NAME);

		iff.exitForm(TAG_0001);
	iff.exitForm(TAG_ECDT);	

	return true;
}

// ----------------------------------------------------------------------

std::string DrawTextureCommandElement::getLabel() const
{
	std::string  result("Draw Texture Command");

	if (m_commandDescription->length())
	{
		result += ": ";
		result += *m_commandDescription;
	}
	else
		result += ": <no description>";
		
	return result;
}

// ----------------------------------------------------------------------

unsigned int DrawTextureCommandElement::getTypeIndex() const
{
	return ETI_commandDrawTexture;
}

// ----------------------------------------------------------------------

bool DrawTextureCommandElement::userCanDelete() const
{
	return true;
}

// ----------------------------------------------------------------------

HueElement *DrawTextureCommandElement::getHueElement(int index)
{
	//-- Find the index'th element of type HueElement.
	ElementVector &children   = getChildren();
	int            foundCount = 0;

	const ElementVector::iterator itEnd = children.end();
	for (ElementVector::iterator it = children.begin(); it != itEnd; ++it)
	{
		HueElement *const hueElement = dynamic_cast<HueElement*>(*it);
		if (hueElement)
		{
			++foundCount;
			if (index == (foundCount - 1))
				return hueElement;
		}
	}

	//-- Didn't find it.
	return 0;
}

// ----------------------------------------------------------------------

const HueElement *DrawTextureCommandElement::getHueElement(int index) const
{
	//-- Find the index'th element of type HueElement.
	const ElementVector &children   = getChildren();
	int                  foundCount = 0;

	const ElementVector::const_iterator itEnd = children.end();
	for (ElementVector::const_iterator it = children.begin(); it != itEnd; ++it)
	{
		const HueElement *const hueElement = dynamic_cast<const HueElement*>(*it);
		if (hueElement)
		{
			++foundCount;
			if (index == (foundCount - 1))
				return hueElement;
		}
	}

	//-- Didn't find it.
	return 0;
}

// ----------------------------------------------------------------------

ImageSlotElement *DrawTextureCommandElement::getImageSlotElement(int index)
{
	ElementVector &children   = getChildren();
	int            foundCount = 0;

	const ElementVector::iterator itEnd = children.end();
	for (ElementVector::iterator it = children.begin(); it != itEnd; ++it)
	{
		ImageSlotElement *const imageSlotElement = dynamic_cast<ImageSlotElement*>(*it);
		if (imageSlotElement)
		{
			++foundCount;
			if (index == foundCount - 1)
				return imageSlotElement;
		}
	}

	// didn't find it
	return 0;
}

// ----------------------------------------------------------------------

const ImageSlotElement *DrawTextureCommandElement::getImageSlotElement(int index) const
{
	const ElementVector &children   = getChildren();
	int                  foundCount = 0;

	const ElementVector::const_iterator itEnd = children.end();
	for (ElementVector::const_iterator it = children.begin(); it != itEnd; ++it)
	{
		const ImageSlotElement *const imageSlotElement = dynamic_cast<const ImageSlotElement*>(*it);
		if (imageSlotElement)
		{
			++foundCount;
			if (index == foundCount - 1)
				return imageSlotElement;
		}
	}

	// didn't find it
	return 0;
}

// ----------------------------------------------------------------------

PackedArgb DrawTextureCommandElement::getHueColor(int index, CustomizationData const &variables) const
{
	//-- Retrieve the HueElement instance.
	const HueElement *const hueElement = getHueElement(index);
	NOT_NULL(hueElement);

	//-- Retrieve the customization variable name.
	const std::string &variableName = hueElement->getVariableName();

	//-- Retrieve variable value.
	PaletteColorCustomizationVariable const *const variable = dynamic_cast<PaletteColorCustomizationVariable const*>(variables.findConstVariable(variableName));
	if (variable)
		return variable->getValueAsColor();
	else
	{
		DEBUG_WARNING(true, ("DrawTextureCommandElement::getHueColor(): hue variable [%s] does not exist in CustomizationData, using blue for returned color.", variableName.c_str()));
#ifdef _DEBUG
		variables.debugDump();
#endif
		return PackedArgb::solidBlue;
	}
}

// ----------------------------------------------------------------------

void DrawTextureCommandElement::createVariableElements(VariableElementVector &variables) const
{
	const ElementVector                 &children = getChildren();
	const ElementVector::const_iterator  itEnd    = children.end();
	for (ElementVector::const_iterator it = children.begin(); it != itEnd; ++it)
	{
		//-- Handle direct children variables (e.g. blend factor).
		const VariableDefinitionElement *const vdElement = dynamic_cast<const VariableDefinitionElement*>(*it);
		if (vdElement && vdElement->getVariableName().length())
		{
			switch (vdElement->getTypeId())
			{
				case VTID_blendFactor:
					variables.push_back(new BasicRangedIntVariableElement(vdElement->getFullyQualifiedVariableName("/private").c_str(), 0, 0, 255));
					break;

				default:
					FATAL(true, ("unknown variable type id [%d]", vdElement->getTypeId()));
			}
		}

		//-- Handle HueElement variables.
		const HueElement *const hueElement = dynamic_cast<const HueElement*>(*it);
		if (hueElement)
			variables.push_back(new PaletteColorVariableElement(hueElement->getFullyQualifiedVariableName("/shared_owner").c_str(), hueElement->getPalettePathName().c_str(), hueElement->getDefaultPaletteIndex()));
	}
}

// ----------------------------------------------------------------------

const RegionElement &DrawTextureCommandElement::getRegionElement() const
{
	//-- find the destination region
	const RegionElement *region = 0;
	{
		const ElementVector &children             = getChildren();
		const ElementVector::const_iterator itEnd = children.end();
		for (ElementVector::const_iterator it = children.begin(); (it != itEnd) && !region; ++it)
			region = dynamic_cast<const RegionElement*>(*it);
	}
	NOT_NULL(region);

	return *region;
}

// ----------------------------------------------------------------------

void DrawTextureCommandElement::fillVertexBuffer(VertexBufferWriteIterator it) const
{
	const RegionElement &region = getRegionElement();

	//-- initialize flip state for each u,v
	int flipUState[2];
	int flipVState[2];

#ifdef _DEBUG
	// for lint
	{
		for (int i = 0; i < 2; ++i)
		{
			flipUState[i] = 0;
			flipVState[i] = 0;
		}
	}
#endif

	{
		for (int i = 0; i < m_imageSlotCount; ++i)
		{
			const ImageSlotElement *ise = getImageSlotElement(i);
			NOT_NULL(ise);

			flipUState[i] = ise->getFlipU() ? 1 : 0;
			flipVState[i] = ise->getFlipV() ? 1 : 0;
		}
	}

	//-- upper left
	it.setPosition(region.getX0(), region.getY0(), ms_defaultDepth);
	it.setTextureCoordinates(0, ms_uValues[flipUState[0]][0], ms_vValues[flipVState[0]][0]);
	if (m_imageSlotCount >= 2)
		it.setTextureCoordinates(1, ms_uValues[flipUState[1]][0], ms_vValues[flipVState[1]][0]);
	++it;

	//-- upper right
	it.setPosition(region.getX1(), region.getY0(), ms_defaultDepth);
	it.setTextureCoordinates(0, ms_uValues[flipUState[0]][1], ms_vValues[flipVState[0]][1]);
	if (m_imageSlotCount >= 2)
		it.setTextureCoordinates(1, ms_uValues[flipUState[1]][1], ms_vValues[flipVState[1]][1]);
	++it;

	//-- lower right
	it.setPosition(region.getX1(), region.getY1(), ms_defaultDepth);
	it.setTextureCoordinates(0, ms_uValues[flipUState[0]][2], ms_vValues[flipVState[0]][2]);
	if (m_imageSlotCount >= 2)
		it.setTextureCoordinates(1, ms_uValues[flipUState[1]][2], ms_vValues[flipVState[1]][2]);
	++it;

	//-- lower left
	it.setPosition(region.getX0(), region.getY1(), ms_defaultDepth);
	it.setTextureCoordinates(0, ms_uValues[flipUState[0]][3], ms_vValues[flipVState[0]][3]);
	if (m_imageSlotCount >= 2)
		it.setTextureCoordinates(1, ms_uValues[flipUState[1]][3], ms_vValues[flipVState[1]][3]);
	++it;
}

// ----------------------------------------------------------------------

const Texture &DrawTextureCommandElement::getImageSlotTexture(int index, CustomizationData const &customizationData) const
{
	const ImageSlotElement *const imageSlotElement = getImageSlotElement(index);
	NOT_NULL(imageSlotElement);

	const Texture *texture = 0;

	if (imageSlotElement->isSlotFilled())
		texture = &imageSlotElement->getTexture(customizationData);
	else
		texture = ms_defaultTexture;

	NOT_NULL(texture);
	return *texture;
}

// ----------------------------------------------------------------------

uint32 DrawTextureCommandElement::getTextureFactorAlpha(int variableTypeId, CustomizationData const &customizationData) const
{
	return (static_cast<uint32>(getIntVariableDefinitionValue(variableTypeId, 0, customizationData)) << 24) | 0x00ffffff;
}

// ----------------------------------------------------------------------

const StaticShader &DrawTextureCommandElement::prepareShaderForBaking(CustomizationData const &customizationData) const
{
	//-- select the shader to use based on source texture mode
	StaticShader *modifiableShader = dynamic_cast<StaticShader*>(m_shader);
	NOT_NULL(modifiableShader);

	switch (m_sourceTextureMode)
	{
		case STM_texture1:
			//-- set the texture
			modifiableShader->setTexture(TAG(M,A,I,N), getImageSlotTexture(0, customizationData));
			break;

		case STM_textureBlend2:
			modifiableShader = ms_blend2OverwriteShader;

			//-- set the textures
			modifiableShader->setTexture(TAG(B,L,N,0), getImageSlotTexture(0, customizationData));
			modifiableShader->setTexture(TAG(B,L,N,1), getImageSlotTexture(1, customizationData));
			
			//-- set the texture coordinate sets
			modifiableShader->setTextureCoordinateSet(TAG(B,L,N,0), 0);
			modifiableShader->setTextureCoordinateSet(TAG(B,L,N,1), 1);

			//-- set the blend factor
			modifiableShader->setTextureFactor(TAG(B,L,N,D), getTextureFactorAlpha(static_cast<int>(VTID_blendFactor), customizationData));
			break;

		default:

			FATAL(true, ("unknown source texture mode [%u]", m_sourceTextureMode));
	}

	//-- Setup shader for hueing.
	switch (m_hueMode)
	{
		case HM_none:
			// Do nothing.
			break;

		case HM_oneColor:
			// Set MAIN texture factor.
			{
				//-- Get the hue color.
				const PackedArgb hueColor = getHueColor(0, customizationData);

				//-- Set the hue color.
				modifiableShader->setTextureFactor(TAG_MAIN, hueColor.getArgb());
			}
			break;

		default:
			// Unknown hue mode.
			WARNING_STRICT_FATAL(true, ("Unknown hue mode [%d].", static_cast<int>(m_hueMode)));
	}

	//-- prepare the shader for use
	NOT_NULL(modifiableShader);
	return modifiableShader->prepareToView();
}

// ----------------------------------------------------------------------

void DrawTextureCommandElement::applyCommand(Texture &bakeTexture, const ElementVector &sourceTextures, CustomizationData const &customizationData) const
{
	UNREF(sourceTextures);

	//-- Skip out if we're disabling render for this command.
	if (m_disableRender)
		return;

	//-- prepare the shader
	const StaticShader &renderShader = prepareShaderForBaking(customizationData);
	const int shaderPassCount        = renderShader.getNumberOfPasses();

	//-- render the source texture into the bake texture
	const int mipmapLevelCount = bakeTexture.getMipmapLevelCount();
	for (int mipmapLevel = 0; mipmapLevel < mipmapLevelCount; ++mipmapLevel)
	{
		//-- fill dynamic vertex buffer
		DynamicVertexBuffer *const vertexBuffer = (m_imageSlotCount == 1) ? ms_dynamicVbOneTcSet : ms_dynamicVbTwoTcSet;
		vertexBuffer->lock(4);
			fillVertexBuffer(vertexBuffer->begin());
		vertexBuffer->unlock();

		//-- setup camera for this mipmap level
		const uint  uMipmapLevel   = static_cast<uint>(mipmapLevel);
		const uint  uTextureWidth  = static_cast<uint>(bakeTexture.getWidth());
		const uint  uTextureHeight = static_cast<uint>(bakeTexture.getHeight());

		const int   viewportWidth  = std::max(1, static_cast<int>(uTextureWidth >> uMipmapLevel));
		const int   viewportHeight = std::max(1, static_cast<int>(uTextureHeight >> uMipmapLevel));

		ms_camera->setViewport(0, 0, viewportWidth, viewportHeight);
		ms_camera->setParallelProjection(CONST_REAL(0), CONST_REAL(0), ms_defaultNormalizedLength, ms_defaultNormalizedLength);

		//-- render with our camera
		Graphics::beginScene();
		Graphics::setRenderTarget(&bakeTexture, CF_none, mipmapLevel);
		{
			// set the camera
			ms_camera->beginScene();

				Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
				Graphics::setCullMode(GCM_counterClockwise);

				Graphics::setVertexBuffer(*vertexBuffer);

				for (int passIndex = 0; passIndex < shaderPassCount; ++passIndex)
				{
					Graphics::setStaticShader(renderShader, passIndex);
					Graphics::drawTriangleFan();
				}

			// release the camera
			ms_camera->endScene();
		}
		IGNORE_RETURN(Graphics::copyRenderTargetToNonRenderTargetTexture());
		Graphics::endScene();
	}
}

// ----------------------------------------------------------------------

void DrawTextureCommandElement::setHueMode(HueMode hueMode)
{
	//-- Ignore setting to the same value.
	if (hueMode == m_hueMode)
	{
		// nothing to do
		return;
	}

	//-- Modify this object based on current mode setting.
	const int currentHueElementCount = m_hueElementCount;
	const int newHueElementCount     = static_cast<int>(hueMode);

	if (newHueElementCount < currentHueElementCount)
	{
		//-- Remove unnecessary HueElement instances, starting from the end.
		ElementVector &children = getChildren();

		for (int i = currentHueElementCount - 1; i >= newHueElementCount; --i)
		{
			//-- delete this image slot element
			HueElement *const        hueElement = getHueElement(i);
			ElementVector::iterator  it         = std::find(children.begin(), children.end(), static_cast<Element*>(hueElement));
			DEBUG_FATAL(it == children.end(), ("Found HueElement [%d, 0x%08x], but not in children list.", i, hueElement));

			IGNORE_RETURN(children.erase(it));
			delete hueElement;
		}
	}
	else if (newHueElementCount > currentHueElementCount)
	{
		ElementVector &children = getChildren();
		char           description[64];

		for (int i = currentHueElementCount; i < newHueElementCount; ++i)
		{
			sprintf(description, "hue %d", i + 1);
			children.push_back(new HueElement(description));
		}
	}

	//-- Save new HueElement count.
	m_hueElementCount = newHueElementCount;

	//-- Set new HueMode.
	m_hueMode = hueMode;
}

// ----------------------------------------------------------------------

void DrawTextureCommandElement::setSourceTextureMode(SourceTextureMode sourceTextureMode)
{
	if (sourceTextureMode == m_sourceTextureMode)
	{
		// nothing to do
		return;
	}

	//-- modify this object based on current mode setting
	const int currentImageSlotCount = m_imageSlotCount;

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(sourceTextureMode), ms_imageSlotsPerSTMCount);
	const int newImageSlotCount = ms_imageSlotsPerSTM[static_cast<size_t>(sourceTextureMode)];

	if (newImageSlotCount < currentImageSlotCount)
	{
		ElementVector &children = getChildren();

		for (int i = currentImageSlotCount - 1; i >= newImageSlotCount; --i)
		{
			//-- delete this image slot element
			ImageSlotElement *const imageSlotElement = getImageSlotElement(i);
			ElementVector::iterator it               = std::find(children.begin(), children.end(), static_cast<Element*>(imageSlotElement));
			DEBUG_FATAL(it == children.end(), ("found image slot [%d, 0x%08x], but not in children list", i, imageSlotElement));

			IGNORE_RETURN(children.erase(it));
			delete imageSlotElement;
		}
	}
	else if (newImageSlotCount > currentImageSlotCount)
	{
		ElementVector &children = getChildren();

		for (int i = currentImageSlotCount; i < newImageSlotCount; ++i)
			children.push_back(new ImageSlotElement());
	}

	//-- save new image slot count
	m_imageSlotCount = newImageSlotCount;

	removeAllVariableDefinitionElements();

	switch (sourceTextureMode)
	{
		case STM_texture1:
			break;
		case STM_textureBlend2:
			{
				ElementVector &children = getChildren();
				children.push_back(new VariableDefinitionElement(ms_variableDefinitionDescriptions[static_cast<int>(VTID_blendFactor)], static_cast<int>(VTID_blendFactor)));
			}
			break;
		default:
			FATAL(true, ("unknown source texture mode [%u]", m_sourceTextureMode));
	}

	//-- set new mode
	m_sourceTextureMode = sourceTextureMode;
}

// ----------------------------------------------------------------------

bool DrawTextureCommandElement::hasLeftDoubleClickAction() const
{
	return true;
}

// ----------------------------------------------------------------------

bool DrawTextureCommandElement::doLeftDoubleClickAction(TextureBuilderDoc&)
{
	DialogEnterName dlg(*m_commandDescription);

	if (dlg.DoModal() == IDOK)
	{
		//-- name was entered
		IGNORE_RETURN(m_commandDescription->assign(dlg.m_name));

		//-- indicate we're modified
		return true;
	}
	else
	{
		// indicate we did not modify this element
		return false;
	}
}

// ----------------------------------------------------------------------

void DrawTextureCommandElement::getFirstFilledImageSlotDimensions(int &imageWidth, int &imageHeight) const
{
	Object *const object = new MemoryBlockManagedObject;
	CustomizationData *const customizationData = new CustomizationData(*object);
	customizationData->fetch();

	for (int i = 0; i < m_imageSlotCount; ++i)
	{
		// -TRF- inefficient
		const ImageSlotElement &imageSlot = *NON_NULL(getImageSlotElement(i));
		if (imageSlot.isSlotFilled())
		{
			const Texture &texture = imageSlot.getTexture(*customizationData);
			imageWidth             = texture.getWidth();
			imageHeight            = texture.getHeight();
		}
	}

	customizationData->release();
	delete object;
}

// ----------------------------------------------------------------------

void DrawTextureCommandElement::exportTextureRenderer(Exporter &exporter) const
{
	BlueprintWriter &writer = exporter.getWriter();

	//-- add the appropriate shader
	int shaderIndex = -1;

	switch (m_sourceTextureMode)
	{
		case STM_texture1:
			{
				//-- export the shader
				switch (m_writeTextureMode)
				{
					case WTM_overwrite:
						shaderIndex = writer.addShaderTemplate(ms_copySourceShaderName);
						break;
					case WTM_alphaBlend:
						shaderIndex = writer.addShaderTemplate(ms_alphaBlendShaderName);
						break;
					default:
						FATAL(true, ("unknown write texture mode [%u]", m_writeTextureMode));
				}
				
				//-- export prepare commands for shader
				writer.beginNewPrepareCommand();
					writer.beginNewPrepareOperations(true);

						NON_NULL(getImageSlotElement(0))->exportPrepareOperations(exporter, shaderIndex, TAG_MAIN);

					writer.endPrepareOperations();
				writer.endPrepareCommand();
			}
			break;
		case STM_textureBlend2:
			{
				//-- export the shader
				shaderIndex = writer.addShaderTemplate(ms_blend2OverwriteShaderName);

				writer.beginNewPrepareCommand();
					writer.beginNewPrepareOperations(true);

						//-- export prepare operations to setup shader textures
						NON_NULL(getImageSlotElement(0))->exportPrepareOperations(exporter, shaderIndex, TAG_BLN0);
						NON_NULL(getImageSlotElement(1))->exportPrepareOperations(exporter, shaderIndex, TAG_BLN1);

						//-- get the variable definition element that defines this variable name
						const VariableDefinitionElement *vdefElement = getVariableDefinitionElement(static_cast<int>(VTID_blendFactor), 0);
						NOT_NULL(vdefElement);
				
						//-- export prepare operations to setup the blend factor
						std::string const shortVariableName = Element::makeShortVariableName(vdefElement->getVariableName());
						writer.addSetShaderTextureFactorAlpha(shaderIndex, TAG_BLND, 255, 255, 255, shortVariableName);

					writer.endPrepareOperations();
				writer.endPrepareCommand();
			}
			break;
		default:
			FATAL(true, ("unknown source texture mode [%u]", m_sourceTextureMode));
	}

	//-- Export hue support.
	switch (m_hueMode)
	{
		case HM_oneColor:
			{
				//-- Get the HueElement instance.
				const HueElement *const hueElement = getHueElement(0);

				//-- Add prepare operation to hue via texture factor.
				writer.beginNewPrepareCommand();
					writer.beginNewPrepareOperations(true);

						std::string shortVariableName = Element::makeShortVariableName(hueElement->getVariableName());
						writer.addSetShaderTextureFactorFromPalette(shaderIndex, TAG_MAIN, shortVariableName, hueElement->isVariablePrivate(), hueElement->getPalettePathName());

					writer.endPrepareOperations();
				writer.endPrepareCommand();
			}
			break;

		case HM_none:
		default:
			// Do nothing.
			break;
	}

	//-- export the vertex buffer
	VertexBufferFormat  format;

	if (m_imageSlotCount == 1)
	{
		format.setPosition();
		format.setNumberOfTextureCoordinateSets(1);
		format.setTextureCoordinateSetDimension(0, 2);
	}
	else if (m_imageSlotCount == 2)
	{
		format.setPosition();
		format.setNumberOfTextureCoordinateSets(2);
		format.setTextureCoordinateSetDimension(0, 2);
		format.setTextureCoordinateSetDimension(1, 2);
	}
	else
		FATAL(true, ("command has unsupported image slot count %d", m_imageSlotCount));
		
	SystemVertexBuffer vb(format, 4);
	fillVertexBuffer(vb.begin());

	const int vertexBufferIndex = writer.addVertexBuffer(vb);

	//-- create the draw command
	IGNORE_RETURN(writer.beginNewShaderRenderCommand(shaderIndex));
		writer.addShaderRenderTriFanPrimitive(vertexBufferIndex);
	writer.endShaderRenderCommand();
}

// ----------------------------------------------------------------------

void DrawTextureCommandElement::createShader()
{
	//-- Release existing shader.
	if (m_shader)
	{
		m_shader->release();
		m_shader = NULL;
	}

	//-- Create shader of the appropriate type.
	switch (m_sourceTextureMode)
	{
		case STM_texture1:
			{
				//-- select the shader
				switch (m_writeTextureMode)
				{
					case WTM_overwrite:
						m_shader = ShaderTemplateList::fetchModifiableShader(ms_copySourceShaderName);
						NOT_NULL(m_shader);
						break;

					case WTM_alphaBlend:
						m_shader = ShaderTemplateList::fetchModifiableShader(ms_alphaBlendShaderName);
						NOT_NULL(m_shader);
						break;

					default:
						FATAL(true, ("unknown write texture mode [%u]", m_writeTextureMode));
				}
			}
			break;

		case STM_textureBlend2:
			{
				m_shader = ShaderTemplateList::fetchModifiableShader(ms_blend2OverwriteShaderName);
				NOT_NULL(m_shader);
			}
			break;

		default:
			FATAL(true, ("unknown texture source mode [%d].", m_sourceTextureMode));
	}

	//-- Ensure we have a shader set.
	NOT_NULL(m_shader);
}

// ======================================================================
