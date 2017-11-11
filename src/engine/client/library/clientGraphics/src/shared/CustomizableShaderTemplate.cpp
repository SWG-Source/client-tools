// ======================================================================
//
// CustomizableShaderTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/CustomizableShaderTemplate.h"

#include "clientGraphics/CustomizableShader.h"
#include "clientGraphics/Material.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "sharedDebug/DataLint.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/PaletteArgb.h"
#include "sharedMath/PaletteArgbList.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/BasicRangedIntCustomizationVariable.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"

#include <algorithm>
#include <string>
#include <vector>

// ======================================================================

const Tag TAG_AMCL = TAG(A,M,C,L);
const Tag TAG_CSHD = TAG(C,S,H,D);
const Tag TAG_CUST = TAG(C,U,S,T);
const Tag TAG_DFCL = TAG(D,F,C,L);
const Tag TAG_EMCL = TAG(E,M,C,L);
// const Tag TAG_MAIN = TAG(M,A,I,N);
const Tag TAG_MATR = TAG(M,A,T,R);
const Tag TAG_NONE = TAG(N,O,N,E);
const Tag TAG_PAL  = TAG3(P,A,L);
const Tag TAG_TFAC = TAG(T,F,A,C);
const Tag TAG_TX1D = TAG(T,X,1,D);
const Tag TAG_TXTR = TAG(T,X,T,R);

// ======================================================================

bool                              CustomizableShaderTemplate::ms_installed;
MemoryBlockManager *CustomizableShaderTemplate::ms_memoryBlockManager;

bool                              CustomizableShaderTemplate::ms_debugLogChanges;

// ======================================================================

#ifdef _DEBUG

std::string TagToStdString(Tag tag)
{
	char buffer[5];

	ConvertTagToString(tag, buffer);
	return buffer;
}

#endif

// ======================================================================

class CustomizableShaderTemplate::MaterialIntOperation
{
public:

	MaterialIntOperation();
	virtual ~MaterialIntOperation();

	virtual bool execute(const IntVariableFactoryVector &intVariableFactoryVector, const IntVector &intValues, Material &material) const = 0;

};

// ----------------------------------------------------------------------

class CustomizableShaderTemplate::AmbientMaterialIntOperation: public CustomizableShaderTemplate::MaterialIntOperation
{
public:

	AmbientMaterialIntOperation();

	virtual bool execute(const IntVariableFactoryVector &intVariableFactoryVector, const IntVector &intValues, Material &material) const;
	void         load_0000(Iff &iff, CustomizableShaderTemplate &csTemplate);
	void         load_0001(Iff &iff, CustomizableShaderTemplate &csTemplate);

private:

	int  m_variableIndex;
};

// ----------------------------------------------------------------------

class CustomizableShaderTemplate::DiffuseMaterialIntOperation: public CustomizableShaderTemplate::MaterialIntOperation
{
public:

	DiffuseMaterialIntOperation();

	virtual bool execute(const IntVariableFactoryVector &intVariableFactoryVector, const IntVector &intValues, Material &material) const;
	void         load_0000(Iff &iff, CustomizableShaderTemplate &csTemplate);
	void         load_0001(Iff &iff, CustomizableShaderTemplate &csTemplate);

private:

	int  m_variableIndex;
};

// ----------------------------------------------------------------------

class CustomizableShaderTemplate::EmissiveMaterialIntOperation: public CustomizableShaderTemplate::MaterialIntOperation
{
public:

	EmissiveMaterialIntOperation();

	virtual bool execute(const IntVariableFactoryVector &intVariableFactoryVector, const IntVector &intValues, Material &material) const;
	void         load_0000(Iff &iff, CustomizableShaderTemplate &csTemplate);
	void         load_0001(Iff &iff, CustomizableShaderTemplate &csTemplate);

private:

	int  m_variableIndex;
};

// ======================================================================

class CustomizableShaderTemplate::CustomizedMaterial
{
public:

	static MaterialIntOperation *createIntOperation_0000(Iff &iff, CustomizableShaderTemplate &csTemplate);
	static MaterialIntOperation *createIntOperation_0001(Iff &iff, CustomizableShaderTemplate &csTemplate);

public:

	CustomizedMaterial();
	~CustomizedMaterial();

	bool applyMaterial(const IntVariableFactoryVector &intVariableFactoryVector, const IntVector &intValues, StaticShader &shader) const;

	void load_0000(Iff &iff, CustomizableShaderTemplate &csTemplate);
	void load_0001(Iff &iff, CustomizableShaderTemplate &csTemplate);

private:

	typedef std::vector<MaterialIntOperation*>  IntOperationVector;

private:

	Tag                 m_materialTag;
	IntOperationVector  m_intOperationVector;

};

// ======================================================================
/**
 * Handles changing one of the Shader's tagged textures to one of 
 * a 1-d array of textures.
 *
 * Implementation note: later, if we support more than 1-d texture
 * selection, this should become a virtual base class (see the Material
 * handling).
 */

class CustomizableShaderTemplate::TextureIntOperation
{
public:

	TextureIntOperation();

	void load_0000(Iff &iff, CustomizableShaderTemplate &csTemplate);
	void load_0001(Iff &iff, CustomizableShaderTemplate &csTemplate);
	bool applyCustomization(const CustomizableShaderTemplate &csTemplate, const IntVector &intValues, StaticShader &shader) const;

private:

	Tag  m_textureTag;
	int  m_baseTextureIndex;
	int  m_textureCount;

	int  m_variableIndex;

};

// ======================================================================

class CustomizableShaderTemplate::TextureFactorIntOperation
{
public:

	static TextureFactorIntOperation *create_0001(Iff &iff, CustomizableShaderTemplate &csTemplate);

public:

	bool applyCustomization(const IntVariableFactoryVector &intVariableFactoryVector, const IntVector &intValues, StaticShader &shader) const;

private:

	TextureFactorIntOperation();
	void load_0001(Iff &iff, CustomizableShaderTemplate &csTemplate);

private:
	
	Tag  m_tfactorTag;
	int  m_variableIndex;

};

// ======================================================================

class CustomizableShaderTemplate::IntVariableFactory
{
public:

	virtual ~IntVariableFactory();

	virtual CustomizationVariable *createCustomizationVariable() const = 0;

	const std::string             &getVariableName() const;
	bool                           isVariablePrivate() const;

protected:

	IntVariableFactory(const std::string &variableName, bool variableIsPrivate);

private:

	// disabled
	IntVariableFactory();
	IntVariableFactory(const IntVariableFactory&);             //lint -esym(754, IntVariableFactory::IntVariableFactory) // unreferenced // defensive hiding
	IntVariableFactory &operator =(const IntVariableFactory&); //lint -esym(754, IntVariableFactory::operator=) // unreferenced // required suppression

private:

	const std::string  m_variableName;
	const bool         m_variableIsPrivate;

};

// ----------------------------------------------------------------------

class CustomizableShaderTemplate::PaletteColorVariableFactory: public CustomizableShaderTemplate::IntVariableFactory
{
public:

	PaletteColorVariableFactory(const std::string &variableName, bool variableIsPrivate, const CrcString &paletteArgbPathName, int defaultPaletteEntry);
	virtual ~PaletteColorVariableFactory();

	virtual CustomizationVariable *createCustomizationVariable() const;

	const PaletteArgb             *fetchPalette() const;

private:

	// disabled
	PaletteColorVariableFactory();
	PaletteColorVariableFactory(const PaletteColorVariableFactory&);             //lint -esym(754, PaletteColorVariableFactory::PaletteColorVariableFactory) // unreferenced // defensive hiding
	PaletteColorVariableFactory &operator =(const PaletteColorVariableFactory&); //lint -esym(754, PaletteColorVariableFactory::operator=) // unreferenced // required suppression

private:

	const PaletteArgb *const m_palette;
	const int                m_defaultPaletteEntryIndex;
};

// ----------------------------------------------------------------------

class CustomizableShaderTemplate::RangedIntVariableFactory: public CustomizableShaderTemplate::IntVariableFactory
{
public:

	RangedIntVariableFactory(const std::string &variableName, bool variableIsPrivate, int minValueInclusive, int defaultValue, int maxValueExclusive);

	virtual CustomizationVariable *createCustomizationVariable() const;

	// Disabled.
	RangedIntVariableFactory();
	RangedIntVariableFactory(const RangedIntVariableFactory&);             //lint -esym(754, RangedIntVariableFactory::RangedIntVariableFactory) // unreferenced // defensive hiding
	RangedIntVariableFactory &operator =(const RangedIntVariableFactory&); //lint -esym(754, RangedIntVariableFactory::operator=)                // unreferenced // defensive hiding

private:

	int  m_minValueInclusive;
	int  m_defaultValue;
	int  m_maxValueExclusive;

};

// ======================================================================

class CustomizableShaderTemplate::CachedTexture
{
public:

	explicit CachedTexture(const char *texturePathName);
	~CachedTexture();

	const Texture *fetchTexture() const;

private:

	// Disabled.
	CachedTexture();
	CachedTexture(CachedTexture const &);             //lint -esym(754, CachedTexture::CachedTexture) // (Info -- local structure member 'CachedTexture::CachedTexture(const CachedTexture &)' not referenced) // Defensive hiding.
	CachedTexture &operator =(CachedTexture const &);

private:

	PersistentCrcString     m_texturePathName;
	mutable const Texture  *m_texture;
};

// ======================================================================

#ifdef _DEBUG

static std::string TagToString(Tag tag)
{
	char buffer[5];
	
	ConvertTagToString(tag, buffer);
	return buffer;
}

#endif

// ======================================================================
// class CustomizableShaderTemplate::IntVariableFactory
// ======================================================================

CustomizableShaderTemplate::IntVariableFactory::IntVariableFactory(const std::string &variableName, bool variableIsPrivate) :
	m_variableName(variableName),
	m_variableIsPrivate(variableIsPrivate)
{
}

// ----------------------------------------------------------------------

CustomizableShaderTemplate::IntVariableFactory::~IntVariableFactory()
{
}

// ----------------------------------------------------------------------

inline const std::string &CustomizableShaderTemplate::IntVariableFactory::getVariableName() const
{
	return m_variableName;
};

// ----------------------------------------------------------------------

inline bool CustomizableShaderTemplate::IntVariableFactory::isVariablePrivate() const
{
	return m_variableIsPrivate;
};

// ======================================================================
// class CustomizableShaderTemplate::PaletteColorVariableFactory
// ======================================================================

CustomizableShaderTemplate::PaletteColorVariableFactory::PaletteColorVariableFactory(const std::string &variableName, bool variableIsPrivate, const CrcString &paletteArgbPathName, int defaultPaletteEntryIndex) :
	IntVariableFactory(variableName, variableIsPrivate),
	m_palette(PaletteArgbList::fetch(paletteArgbPathName)),
	m_defaultPaletteEntryIndex(defaultPaletteEntryIndex)
{
}

// ----------------------------------------------------------------------

CustomizableShaderTemplate::PaletteColorVariableFactory::~PaletteColorVariableFactory()
{
	//lint -esym(1540, PaletteColorVariableFactory::m_palette) // not freed or zero'ed // it's okay, it's reference counted and released
	m_palette->release();
}

// ----------------------------------------------------------------------

CustomizationVariable *CustomizableShaderTemplate::PaletteColorVariableFactory::createCustomizationVariable() const
{
	return new PaletteColorCustomizationVariable(m_palette, m_defaultPaletteEntryIndex);
}

// ----------------------------------------------------------------------

const PaletteArgb *CustomizableShaderTemplate::PaletteColorVariableFactory::fetchPalette() const
{
	m_palette->fetch();
	return m_palette;
}

// ======================================================================
// class CustomizableShaderTemplate::RangedIntVariableFactory
// ======================================================================

CustomizableShaderTemplate::RangedIntVariableFactory::RangedIntVariableFactory(const std::string &variableName, bool variableIsPrivate, int minValueInclusive, int defaultValue, int maxValueExclusive) :
	IntVariableFactory(variableName, variableIsPrivate),
	m_minValueInclusive(minValueInclusive),
	m_defaultValue(defaultValue),
	m_maxValueExclusive(maxValueExclusive)
{
	WARNING_STRICT_FATAL(m_minValueInclusive > m_maxValueExclusive, ("min value [%d] > max value [%d].", m_minValueInclusive, m_maxValueExclusive));
	WARNING_STRICT_FATAL(!WithinRangeExclusiveExclusive(m_minValueInclusive - 1, defaultValue, maxValueExclusive), ("default [%d] out of valid range [%d..%d)", m_defaultValue, m_minValueInclusive, m_maxValueExclusive));
}

// ----------------------------------------------------------------------

CustomizationVariable *CustomizableShaderTemplate::RangedIntVariableFactory::createCustomizationVariable() const
{
	return new BasicRangedIntCustomizationVariable(m_minValueInclusive, m_defaultValue, m_maxValueExclusive);
}

// ======================================================================
// class CustomizableShaderTemplate::MaterialIntOperation
// ======================================================================

CustomizableShaderTemplate::MaterialIntOperation::MaterialIntOperation()
{
}

// ----------------------------------------------------------------------

CustomizableShaderTemplate::MaterialIntOperation::~MaterialIntOperation()
{
}

// ======================================================================
// class CustomizableShaderTemplate::AmbientMaterialIntOperation
// ======================================================================

CustomizableShaderTemplate::AmbientMaterialIntOperation::AmbientMaterialIntOperation() :
	MaterialIntOperation(),
	m_variableIndex(-1)
{
}

// ----------------------------------------------------------------------

bool CustomizableShaderTemplate::AmbientMaterialIntOperation::execute(const IntVariableFactoryVector &intVariableFactoryVector, const IntVector &intValues, Material &material) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_variableIndex, static_cast<int>(intValues.size()));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_variableIndex, static_cast<int>(intVariableFactoryVector.size()));

	//-- fetch the palette
	const PaletteColorVariableFactory *const pcvFactory = safe_cast<const PaletteColorVariableFactory*>(intVariableFactoryVector[static_cast<size_t>(m_variableIndex)]);
	NOT_NULL(pcvFactory);

	const PaletteArgb *const palette = pcvFactory->fetchPalette();
	NOT_NULL(palette);

	//-- set the palette color
	const int         paletteEntryIndex = intValues[static_cast<size_t>(m_variableIndex)];

	bool error = false;
	const VectorArgb  color(palette->getEntry(paletteEntryIndex, error));

	WARNING(error, ("CustomizableShaderTemplate::AmbientMaterialIntOperation::execute error"));

	DEBUG_REPORT_LOG(ms_debugLogChanges, ("|- setting ambient (r=%g,g=%g,b=%g,a=%g) [%d]\n", color.r, color.g, color.b, color.a, paletteEntryIndex));

	material.setAmbientColor(color);

	//-- release the palette
	palette->release();

	return !error;
}

// ----------------------------------------------------------------------

void CustomizableShaderTemplate::AmbientMaterialIntOperation::load_0000(Iff &iff, CustomizableShaderTemplate &csTemplate)
{
	//-- load the data
	iff.enterChunk(TAG_AMCL);

		char variableName[512];
		char paletteFileName[512];

		iff.read_string(variableName, sizeof(variableName) - 1);
		iff.read_string(paletteFileName, sizeof(paletteFileName) - 1);

		const int defaultPaletteIndex = static_cast<int>(iff.read_int32());

	iff.exitChunk(TAG_AMCL);

	//-- register the customization variable
	m_variableIndex = csTemplate.submitPaletteColorVariable(variableName, true, paletteFileName, defaultPaletteIndex);
}

// ----------------------------------------------------------------------

void CustomizableShaderTemplate::AmbientMaterialIntOperation::load_0001(Iff &iff, CustomizableShaderTemplate &csTemplate)
{
	//-- load the data
	iff.enterChunk(TAG_AMCL);

		char variableName[512];
		char paletteFileName[512];

		iff.read_string(variableName, sizeof(variableName) - 1);

		const bool variableIsPrivate = (iff.read_int8() != 0) ? true : false;

		iff.read_string(paletteFileName, sizeof(paletteFileName) - 1);

		const int defaultPaletteIndex = static_cast<int>(iff.read_int32());

	iff.exitChunk(TAG_AMCL);

	//-- register the customization variable
	m_variableIndex = csTemplate.submitPaletteColorVariable(variableName, variableIsPrivate, paletteFileName, defaultPaletteIndex);
}

// ======================================================================
// class CustomizableShaderTemplate::DiffuseMaterialIntOperation
// ======================================================================

CustomizableShaderTemplate::DiffuseMaterialIntOperation::DiffuseMaterialIntOperation() :
	MaterialIntOperation(),
	m_variableIndex(-1)
{
}

// ----------------------------------------------------------------------

bool CustomizableShaderTemplate::DiffuseMaterialIntOperation::execute(const IntVariableFactoryVector &intVariableFactoryVector, const IntVector &intValues, Material &material) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_variableIndex, static_cast<int>(intValues.size()));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_variableIndex, static_cast<int>(intVariableFactoryVector.size()));

	//-- fetch the palette
	const PaletteColorVariableFactory *const pcvFactory = safe_cast<const PaletteColorVariableFactory*>(intVariableFactoryVector[static_cast<size_t>(m_variableIndex)]);
	NOT_NULL(pcvFactory);

	const PaletteArgb *const palette = pcvFactory->fetchPalette();
	NOT_NULL(palette);

	//-- set the palette color
	const int         paletteEntryIndex = intValues[static_cast<size_t>(m_variableIndex)];
	bool error = false;
	const VectorArgb  color(palette->getEntry(paletteEntryIndex, error));
	WARNING(error, ("CustomizableShaderTemplate::DiffuseMaterialIntOperation::execute error"));

	DEBUG_REPORT_LOG(ms_debugLogChanges, ("|- setting diffuse (r=%g,g=%g,b=%g,a=%g) [%d]\n", color.r, color.g, color.b, color.a, paletteEntryIndex));
	material.setDiffuseColor(color);

	//-- release the palette
	palette->release();

	return !error;
}

// ----------------------------------------------------------------------

void CustomizableShaderTemplate::DiffuseMaterialIntOperation::load_0000(Iff &iff, CustomizableShaderTemplate &csTemplate)
{
	//-- load the data
	iff.enterChunk(TAG_DFCL);

		char variableName[512];
		char paletteFileName[512];

		iff.read_string(variableName, sizeof(variableName) - 1);
		iff.read_string(paletteFileName, sizeof(paletteFileName) - 1);

		const int defaultPaletteIndex = static_cast<int>(iff.read_int32());

	iff.exitChunk(TAG_DFCL);

	//-- register the customization variable
	m_variableIndex = csTemplate.submitPaletteColorVariable(variableName, true, paletteFileName, defaultPaletteIndex);
}

// ----------------------------------------------------------------------

void CustomizableShaderTemplate::DiffuseMaterialIntOperation::load_0001(Iff &iff, CustomizableShaderTemplate &csTemplate)
{
	//-- load the data
	iff.enterChunk(TAG_DFCL);

		char variableName[512];
		char paletteFileName[512];

		iff.read_string(variableName, sizeof(variableName) - 1);

		const bool variableIsPrivate = (iff.read_int8() != 0) ? true : false;

		iff.read_string(paletteFileName, sizeof(paletteFileName) - 1);

		const int defaultPaletteIndex = static_cast<int>(iff.read_int32());

	iff.exitChunk(TAG_DFCL);

	//-- register the customization variable
	m_variableIndex = csTemplate.submitPaletteColorVariable(variableName, variableIsPrivate, paletteFileName, defaultPaletteIndex);
}

// ======================================================================
// class CustomizableShaderTemplate::EmissiveMaterialIntOperation
// ======================================================================

CustomizableShaderTemplate::EmissiveMaterialIntOperation::EmissiveMaterialIntOperation() :
	MaterialIntOperation(),
	m_variableIndex(-1)
{
}

// ----------------------------------------------------------------------

bool CustomizableShaderTemplate::EmissiveMaterialIntOperation::execute(const IntVariableFactoryVector &intVariableFactoryVector, const IntVector &intValues, Material &material) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_variableIndex, static_cast<int>(intValues.size()));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_variableIndex, static_cast<int>(intVariableFactoryVector.size()));

	//-- fetch the palette
	const PaletteColorVariableFactory *const pcvFactory = safe_cast<const PaletteColorVariableFactory*>(intVariableFactoryVector[static_cast<size_t>(m_variableIndex)]);
	NOT_NULL(pcvFactory);

	const PaletteArgb *const palette = pcvFactory->fetchPalette();
	NOT_NULL(palette);

	//-- set the palette color
	const int paletteEntryIndex = intValues[static_cast<size_t>(m_variableIndex)];
	bool error = false;
	const     VectorArgb color(palette->getEntry(paletteEntryIndex, error));

	WARNING(error, ("CustomizableShaderTemplate::EmissiveMaterialIntOperation::execute error"));

	DEBUG_REPORT_LOG(ms_debugLogChanges, ("|- setting emissive (r=%g,g=%g,b=%g,a=%g) [%d]\n", color.r, color.g, color.b, color.a, paletteEntryIndex));
	material.setEmissiveColor(color);

	//-- release the palette
	palette->release();

	return !error;
}

// ----------------------------------------------------------------------

void CustomizableShaderTemplate::EmissiveMaterialIntOperation::load_0000(Iff &iff, CustomizableShaderTemplate &csTemplate)
{
	//-- load the data
	iff.enterChunk(TAG_DFCL);

		char variableName[512];
		char paletteFileName[512];

		iff.read_string(variableName, sizeof(variableName) - 1);
		iff.read_string(paletteFileName, sizeof(paletteFileName) - 1);

		const int defaultPaletteIndex = static_cast<int>(iff.read_int32());

	iff.exitChunk(TAG_DFCL);

	//-- register the customization variable
	m_variableIndex = csTemplate.submitPaletteColorVariable(variableName, true, paletteFileName, defaultPaletteIndex);
}

// ----------------------------------------------------------------------

void CustomizableShaderTemplate::EmissiveMaterialIntOperation::load_0001(Iff &iff, CustomizableShaderTemplate &csTemplate)
{
	//-- load the data
	iff.enterChunk(TAG_DFCL);

		char variableName[512];
		char paletteFileName[512];

		iff.read_string(variableName, sizeof(variableName) - 1);

		const bool variableIsPrivate = (iff.read_int8() != 0) ? true : false;

		iff.read_string(paletteFileName, sizeof(paletteFileName) - 1);

		const int defaultPaletteIndex = static_cast<int>(iff.read_int32());

	iff.exitChunk(TAG_DFCL);

	//-- register the customization variable
	m_variableIndex = csTemplate.submitPaletteColorVariable(variableName, variableIsPrivate, paletteFileName, defaultPaletteIndex);
}

// ======================================================================
// class CustomizableShaderTemplate::CustomizedMaterial
// ======================================================================

CustomizableShaderTemplate::MaterialIntOperation *CustomizableShaderTemplate::CustomizedMaterial::createIntOperation_0000(Iff &iff, CustomizableShaderTemplate &csTemplate)
{
	const Tag version = iff.getCurrentName();
	switch (version)
	{
		case TAG_AMCL:
			{
				AmbientMaterialIntOperation *operation = new AmbientMaterialIntOperation;
				operation->load_0000(iff, csTemplate);

				return operation;
			}

		case TAG_DFCL:
			{
				DiffuseMaterialIntOperation *operation = new DiffuseMaterialIntOperation;
				operation->load_0000(iff, csTemplate);

				return operation;
			}

		case TAG_EMCL:
			{
				EmissiveMaterialIntOperation *operation = new EmissiveMaterialIntOperation;
				operation->load_0000(iff, csTemplate);

				return operation;
			}

		default:
			DEBUG_FATAL(true, ("unsupported MaterialIntOperation [%s]", TagToString(version).c_str()));
			return 0; //lint !e527 // Unreachable // Reachable in release.
	}
}

// ----------------------------------------------------------------------

CustomizableShaderTemplate::MaterialIntOperation *CustomizableShaderTemplate::CustomizedMaterial::createIntOperation_0001(Iff &iff, CustomizableShaderTemplate &csTemplate)
{
	const Tag version = iff.getCurrentName();
	switch (version)
	{
		case TAG_AMCL:
			{
				AmbientMaterialIntOperation *operation = new AmbientMaterialIntOperation;
				operation->load_0001(iff, csTemplate);

				return operation;
			}

		case TAG_DFCL:
			{
				DiffuseMaterialIntOperation *operation = new DiffuseMaterialIntOperation;
				operation->load_0001(iff, csTemplate);

				return operation;
			}

		case TAG_EMCL:
			{
				EmissiveMaterialIntOperation *operation = new EmissiveMaterialIntOperation;
				operation->load_0001(iff, csTemplate);

				return operation;
			}

		default:
			DEBUG_FATAL(true, ("unsupported MaterialIntOperation [%s]", TagToString(version).c_str()));
			return 0; //lint !e527 // Unreachable // Reachable in release.
	}
}

// ======================================================================

CustomizableShaderTemplate::CustomizedMaterial::CustomizedMaterial() :
	m_materialTag(TAG_NONE),
	m_intOperationVector()
{
}

// ----------------------------------------------------------------------

CustomizableShaderTemplate::CustomizedMaterial::~CustomizedMaterial()
{
	std::for_each(m_intOperationVector.begin(), m_intOperationVector.end(), PointerDeleter());
}

// ----------------------------------------------------------------------

bool CustomizableShaderTemplate::CustomizedMaterial::applyMaterial(const IntVariableFactoryVector &intVariableFactoryVector, const IntVector &intValues, StaticShader &shader) const
{
	//-- Ignore this customization if this material is not supported.
	if (!shader.hasMaterial(m_materialTag))
		return false;

	//-- initialize the material with current value
	Material  material;

	const bool getMaterialResult = shader.getMaterial(m_materialTag, material);
	DEBUG_FATAL(!getMaterialResult, ("failed to get material [%s]", TagToString(m_materialTag).c_str()));
	UNREF(getMaterialResult);

	DEBUG_REPORT_LOG(ms_debugLogChanges, ("\nmodifying material [%s]:\n", TagToStdString(m_materialTag).c_str()));

	bool ok = true;

	//-- apply each int operation
	{
		const IntOperationVector::const_iterator endIt = m_intOperationVector.end();
		for (IntOperationVector::const_iterator it = m_intOperationVector.begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);
			ok = (*it)->execute(intVariableFactoryVector, intValues, material) && ok;
		}
	}

#if 1
	//-- set the material into the shader
	shader.setMaterial(m_materialTag, material);
#else
	// @todo get rid of this.
	//-- Take the diffuse color from the material, set the shader's texture factor with it.
	shader.setTextureFactor(TAG_MAIN, material.getDiffuseColor().convertToUint32());
#endif

	return ok;
}

// ----------------------------------------------------------------------

void CustomizableShaderTemplate::CustomizedMaterial::load_0000(Iff &iff, CustomizableShaderTemplate &csTemplate)
{
	//-- process one material
	iff.enterForm(TAG_ENTR);

		//-- find out which material this is for
		iff.enterChunk(TAG_INFO);
			m_materialTag = static_cast<Tag>(iff.read_uint32());
		iff.exitChunk(TAG_INFO);

		//-- load the material's int customizations
		while (!iff.atEndOfForm())
			m_intOperationVector.push_back(createIntOperation_0000(iff, csTemplate));

	iff.exitForm(TAG_ENTR);
}

// ----------------------------------------------------------------------

void CustomizableShaderTemplate::CustomizedMaterial::load_0001(Iff &iff, CustomizableShaderTemplate &csTemplate)
{
	//-- process one material
	iff.enterForm(TAG_ENTR);

		//-- find out which material this is for
		iff.enterChunk(TAG_INFO);
			m_materialTag = static_cast<Tag>(iff.read_uint32());
		iff.exitChunk(TAG_INFO);

		//-- load the material's int customizations
		while (!iff.atEndOfForm())
			m_intOperationVector.push_back(createIntOperation_0001(iff, csTemplate));

	iff.exitForm(TAG_ENTR);
}

// ======================================================================
// class CustomizableShaderTemplate::TextureIntOperation
// ======================================================================

CustomizableShaderTemplate::TextureIntOperation::TextureIntOperation() :
	m_textureTag(TAG_NONE),
	m_baseTextureIndex(-1),
	m_textureCount(0),
	m_variableIndex(-1)
{
}

// ----------------------------------------------------------------------

void CustomizableShaderTemplate::TextureIntOperation::load_0000(Iff &iff, CustomizableShaderTemplate &csTemplate)
{
	//-- Load data.
	iff.enterChunk(TAG_TX1D);

		m_textureTag       = static_cast<Tag>(iff.read_uint32());
		m_baseTextureIndex = static_cast<int>(iff.read_int16());
		m_textureCount     = static_cast<int>(iff.read_int16());

		char shortVariableName[MAX_PATH];
		iff.read_string(shortVariableName, sizeof(shortVariableName) - 1);

		const bool variableIsPrivate    = (iff.read_int8() != 0);

		const int  defaultVariableIndex = static_cast<int>(iff.read_int16());

	iff.exitChunk(TAG_TX1D);

	//-- Get variable index.
	m_variableIndex = csTemplate.submitRangedIntVariable(shortVariableName, variableIsPrivate, 0, defaultVariableIndex, m_textureCount);
}

// ----------------------------------------------------------------------

inline void CustomizableShaderTemplate::TextureIntOperation::load_0001(Iff &iff, CustomizableShaderTemplate &csTemplate)
{
	//-- Use version 0000 code, no change.
	load_0000(iff, csTemplate);
}

// ----------------------------------------------------------------------

bool CustomizableShaderTemplate::TextureIntOperation::applyCustomization(const CustomizableShaderTemplate &csTemplate, const IntVector &intValues, StaticShader &shader) const
{
	//-- Check whether this shader implementation makes use of this texture.
	if (!shader.hasTexture(m_textureTag))
		return false;
	else if (m_textureCount < 1)
	{
		// @todo add warning here.
		return false;
	}

	//-- Get local texture array index.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_variableIndex, static_cast<int>(intValues.size()));
	int localArrayIndex = intValues[static_cast<IntVector::size_type>(m_variableIndex)];

	//-- Compute template global texture index.

	bool ok = true;
	// Ensure value is within valid range.  Clamp and warn if not.
	if ((localArrayIndex < 0) || (localArrayIndex >= m_textureCount))
	{
		DEBUG_WARNING(true, ("customizable shader [%s]: tried to set texture customization [/%s/%s] to value [%d], valid range is [0 .. %d], inclusive. Clamping to [%d].", 
			csTemplate.getName().getString(),
			csTemplate.isIntVariablePrivate(m_variableIndex) ? "private" : "shared_owner",
			csTemplate.getIntVariableName(m_variableIndex).c_str(),
			localArrayIndex,
			m_textureCount - 1,
			clamp(0, localArrayIndex, m_textureCount - 1)));

		localArrayIndex = clamp(0, localArrayIndex, m_textureCount - 1);
		ok = false;
	}

	const int globalArrayIndex = m_baseTextureIndex + localArrayIndex;

	//-- Fetch the texture.
	const Texture *const texture = csTemplate.fetchTexture(globalArrayIndex);

	if (texture)
	{
		//-- Set the texture.
		shader.setTexture(m_textureTag, *texture);

		//-- Release the local texture reference.
		texture->release();
	}

	return ok;
}

// ======================================================================
// class CustomizableShaderTemplate::TextureFactorIntOperation
// ======================================================================

CustomizableShaderTemplate::TextureFactorIntOperation *CustomizableShaderTemplate::TextureFactorIntOperation::create_0001(Iff &iff, CustomizableShaderTemplate &csTemplate)
{
	//-- Create the only supported texture factor operation.
	TextureFactorIntOperation *const operation = new TextureFactorIntOperation();
	operation->load_0001(iff, csTemplate);

	return operation;
}

// ======================================================================

bool CustomizableShaderTemplate::TextureFactorIntOperation::applyCustomization(const IntVariableFactoryVector &intVariableFactoryVector, const IntVector &intValues, StaticShader &shader) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_variableIndex, static_cast<int>(intValues.size()));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_variableIndex, static_cast<int>(intVariableFactoryVector.size()));

	//-- Check whether this shader implementation makes use of this texture factor.
	if (!shader.hasTextureFactor(m_tfactorTag))
		return false;

	//-- fetch the palette
	const PaletteColorVariableFactory *const pcvFactory = safe_cast<const PaletteColorVariableFactory*>(intVariableFactoryVector[static_cast<size_t>(m_variableIndex)]);
	NOT_NULL(pcvFactory);

	const PaletteArgb *const palette = pcvFactory->fetchPalette();
	NOT_NULL(palette);

	//-- set the palette color
	const int         paletteEntryIndex = intValues[static_cast<size_t>(m_variableIndex)];
	bool error = false;
	const PackedArgb &color             = palette->getEntry(paletteEntryIndex, error);

	WARNING(error, ("CustomizableShaderTemplate::TextureFactorIntOperation::execute error"));

	DEBUG_REPORT_LOG(ms_debugLogChanges, ("|- setting tfactor (r=%u,g=%u,b=%u,a=%u) [%d]\n", color.getR(), color.getG(), color.getB(), color.getA(), paletteEntryIndex));
	shader.setTextureFactor(m_tfactorTag, color.getArgb());

	//-- release the palette
	palette->release();

	return !error;
}

// ======================================================================

CustomizableShaderTemplate::TextureFactorIntOperation::TextureFactorIntOperation() :
	m_tfactorTag(TAG_NONE),
	m_variableIndex(-1)
{
}

// ----------------------------------------------------------------------

void CustomizableShaderTemplate::TextureFactorIntOperation::load_0001(Iff &iff, CustomizableShaderTemplate &csTemplate)
{
	iff.enterChunk(TAG_PAL);

		char variableName[512];
		char paletteFileName[512];

		iff.read_string(variableName, sizeof(variableName) - 1);

		const bool variableIsPrivate = (iff.read_int8() != 0) ? true : false;

		m_tfactorTag = static_cast<Tag>(iff.read_uint32());
		iff.read_string(paletteFileName, sizeof(paletteFileName) - 1);
		
		const int defaultPaletteIndex = static_cast<int>(iff.read_int32());

	iff.exitChunk(TAG_PAL);

	//-- Register the customization variable.
	m_variableIndex = csTemplate.submitPaletteColorVariable(variableName, variableIsPrivate, paletteFileName, defaultPaletteIndex);
}

// ======================================================================
// class CustomizableShaderTemplate::CachedTexture
// ======================================================================

CustomizableShaderTemplate::CachedTexture::CachedTexture(const char *texturePathName) :
	m_texturePathName(texturePathName, true),
	m_texture(0)
{
#ifdef _DEBUG
	if (DataLint::isEnabled())
	{
		//-- Fetch the texture.
		const Texture *const texture = fetchTexture();
		if (texture)
			texture->release();
	}
#endif
}

// ----------------------------------------------------------------------

CustomizableShaderTemplate::CachedTexture::~CachedTexture()
{
	if (m_texture)
	{
		m_texture->release();
		m_texture = 0;
	}
}

// ----------------------------------------------------------------------

const Texture *CustomizableShaderTemplate::CachedTexture::fetchTexture() const
{
	//-- Ensure the texture has been retrieved.
	if (!m_texture)
	{
		//-- Fetch local cached reference to the texture kept by this instance.
		m_texture = TextureList::fetch(m_texturePathName.getString());
	}

	//-- Fetch reference for caller.
	if (m_texture)
		m_texture->fetch();

	return m_texture;
}

// ======================================================================
// class CustomizableShaderTemplate: public static member functions
// ======================================================================

void CustomizableShaderTemplate::install()
{
	DEBUG_FATAL(ms_installed, ("CustomizableShaderTemplate already installed"));

	ms_memoryBlockManager = new MemoryBlockManager("CustomizableShaderTemplate", true, sizeof(CustomizableShaderTemplate), 0, 0, 0);
	ShaderTemplateList::registerShaderTemplateType(TAG_CSHD, create);

#ifdef _DEBUG
	DebugFlags::registerFlag(ms_debugLogChanges, "ClientGraphics", "customizedShaderLogChanges");
#endif

#if 0
	ms_debugLogChanges = true;
#endif

	ms_installed = true;
	ExitChain::add(remove, "CustomizableShaderTemplate");
}

// ----------------------------------------------------------------------

void *CustomizableShaderTemplate::operator new(size_t size)
{
	DEBUG_FATAL(!ms_installed, ("CustomizableShaderTemplate not installed"));
	DEBUG_FATAL(size != sizeof(CustomizableShaderTemplate), ("size mismatch, derived classes must define their own operator new"));
	UNREF(size);

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void CustomizableShaderTemplate::operator delete(void *data)
{
	DEBUG_FATAL(!ms_installed, ("CustomizableShaderTemplate not installed"));

	if (data)
		ms_memoryBlockManager->free(data);
}

// ======================================================================
// class CustomizableShaderTemplate: public member functions
// ======================================================================

int CustomizableShaderTemplate::getCustomizationVariableCount() const
{
	return static_cast<int>(m_intVariableFactoryVector->size());
}

// ----------------------------------------------------------------------

const std::string &CustomizableShaderTemplate::getCustomizationVariableName(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getCustomizationVariableCount());

	return (*m_intVariableFactoryVector)[static_cast<size_t>(index)]->getVariableName();
}

// ----------------------------------------------------------------------

bool CustomizableShaderTemplate::isCustomizationVariablePrivate(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getCustomizationVariableCount());

	return (*m_intVariableFactoryVector)[static_cast<size_t>(index)]->isVariablePrivate();
}

// ----------------------------------------------------------------------

CustomizationVariable *CustomizableShaderTemplate::createCustomizationVariable(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getCustomizationVariableCount());

	return (*m_intVariableFactoryVector)[static_cast<size_t>(index)]->createCustomizationVariable();
}

// ----------------------------------------------------------------------

bool CustomizableShaderTemplate::isOpaqueSolid() const
{
	NOT_NULL(m_baseShaderTemplate);
	return m_baseShaderTemplate->isOpaqueSolid();
}

// ----------------------------------------------------------------------

bool CustomizableShaderTemplate::isCollidable() const
{
	NOT_NULL(m_baseShaderTemplate);
	return m_baseShaderTemplate->isCollidable();
}


// ----------------------------------------------------------------------

bool CustomizableShaderTemplate::castsShadows() const
{
	NOT_NULL(m_baseShaderTemplate);
	return m_baseShaderTemplate->castsShadows();
}

// ----------------------------------------------------------------------

const Shader *CustomizableShaderTemplate::fetchShader() const
{
	Shader *shader = new CustomizableShader(*this);
	shader->fetch();

	return shader;
}

// ----------------------------------------------------------------------

Shader *CustomizableShaderTemplate::fetchModifiableShader() const
{
	Shader *shader = new CustomizableShader(*this);
	shader->fetch();

	return shader;
}

// ----------------------------------------------------------------------

int CustomizableShaderTemplate::getIntVariableCount() const
{
	return static_cast<int>(m_intVariableFactoryVector->size());
}

// ----------------------------------------------------------------------

const std::string &CustomizableShaderTemplate::getIntVariableName(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getIntVariableCount());

	IntVariableFactory *const ivf = (*m_intVariableFactoryVector)[static_cast<size_t>(index)];
	NOT_NULL(ivf);

	return ivf->getVariableName();
}

// ----------------------------------------------------------------------

bool CustomizableShaderTemplate::isIntVariablePrivate(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getIntVariableCount());

	IntVariableFactory *const ivf = (*m_intVariableFactoryVector)[static_cast<size_t>(index)];
	NOT_NULL(ivf);

	return ivf->isVariablePrivate();
}

// ----------------------------------------------------------------------

bool CustomizableShaderTemplate::applyShaderSettings(const IntVector &intValues, StaticShader &shader) const
{
	NOT_NULL(m_intVariableFactoryVector);

	bool ok = true;

	//-- Handle material customizations.
	if (m_customizedMaterialVector)
	{
		const CustomizedMaterialVector::const_iterator endIt = m_customizedMaterialVector->end();
		for (CustomizedMaterialVector::const_iterator it = m_customizedMaterialVector->begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);
			ok = (*it)->applyMaterial(*m_intVariableFactoryVector, intValues, shader) && ok;
		}
	}

	//-- Handle texture customizations.
	if (m_textureIntOperationVector)
	{
		const TextureIntOperationVector::const_iterator endIt = m_textureIntOperationVector->end();
		for (TextureIntOperationVector::const_iterator it = m_textureIntOperationVector->begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);
			ok = (*it)->applyCustomization(*this, intValues, shader) && ok;
		}
	}

	//-- Handle texture factor customizations.
	if (m_textureFactorIntOperationVector)
	{
		const TextureFactorIntOperationVector::const_iterator endIt = m_textureFactorIntOperationVector->end();
		for (TextureFactorIntOperationVector::const_iterator it = m_textureFactorIntOperationVector->begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);
			ok = (*it)->applyCustomization(*m_intVariableFactoryVector, intValues, shader) && ok;
		}
	}

	return ok;
}

// ----------------------------------------------------------------------
/**
 * Fetch the templateTextureIndex'th texture associated with this template.
 *
 * @param templateTextureIndex  the index into the global table of textures
 *                              that are associated with this template.
 *
 * @return  a Texture instance with its reference count incremented for the
 *          caller.
 */

const Texture *CustomizableShaderTemplate::fetchTexture(int templateTextureIndex) const
{
	NOT_NULL(m_cachedTextureVector);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, templateTextureIndex, static_cast<int>(m_cachedTextureVector->size()));

	return (*m_cachedTextureVector)[static_cast<CachedTextureVector::size_type>(templateTextureIndex)]->fetchTexture();
}

// ----------------------------------------------------------------------

bool CustomizableShaderTemplate::getHueInfo(IntVector const &intValues, StringVector &variableNames, PackedArgbVector &colors) const
{
	//-- Initialize return data.
	variableNames.clear();
	colors.clear();

	int index = 0;

	IntVariableFactoryVector::const_iterator const endIt = m_intVariableFactoryVector->end();
	for (IntVariableFactoryVector::const_iterator it = m_intVariableFactoryVector->begin(); it != endIt; ++it, ++index)
	{
		//-- Find all palette color variables.
		PaletteColorVariableFactory const *const factory = dynamic_cast<PaletteColorVariableFactory const*>(*it);
		if (factory)
		{
			//-- Get the color.
			PaletteArgb const *palette = factory->fetchPalette();
			if (palette)
			{
				//-- Lookup color.
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(intValues.size()));
				int const paletteIndex = intValues[static_cast<IntVector::size_type>(index)];

				bool error = false;
				colors.push_back(palette->getEntry(paletteIndex, error));

				WARNING(error, ("CustomizableShaderTemplate::getHueInfo error"));

				//-- Keep track of variable name.
				variableNames.push_back(&factory->getVariableName());

				//-- Release local references.
				palette->release();
			}
		}
	}

	return !variableNames.empty();
}

// ======================================================================
// class CustomizableShaderTemplate: private static member functions
// ======================================================================

void CustomizableShaderTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("CustomizableShaderTemplate not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;

	ShaderTemplateList::deregisterShaderTemplateType(TAG_CSHD);

	ms_installed = false;
}

// ----------------------------------------------------------------------

ShaderTemplate *CustomizableShaderTemplate::create(const CrcString &name, Iff &iff)
{
	return new CustomizableShaderTemplate(name, iff);
}

// ======================================================================
// class CustomizableShaderTemplate: private member functions
// ======================================================================

CustomizableShaderTemplate::CustomizableShaderTemplate(const CrcString &name, Iff &iff) :
	ShaderTemplate(name),
	m_baseShaderTemplate(0),
	m_intVariableFactoryVector(new IntVariableFactoryVector()),
	m_customizedMaterialVector(0),
	m_textureIntOperationVector(0),
	m_textureFactorIntOperationVector(0),
	m_cachedTextureVector(0)
{
	iff.enterForm(TAG_CSHD);

		const Tag version = iff.getCurrentName();
		switch(version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			case TAG_0001:
				load_0001(iff);
				break;

			default:
				DEBUG_FATAL(true, ("unsupported CustomizableShaderTemplate version [%s]", TagToString(version).c_str()));
		}

	iff.exitForm(TAG_CSHD);

#ifdef _DEBUG
	if (m_baseShaderTemplate)
	{
		//-- Set the debug name for the base shader template so we get appropriate warning info from problems with it.
		m_baseShaderTemplate->setDebugName(&getName());
	}
#endif
}

// ----------------------------------------------------------------------

CustomizableShaderTemplate::~CustomizableShaderTemplate()
{
	if (m_cachedTextureVector)
	{
		std::for_each(m_cachedTextureVector->begin(), m_cachedTextureVector->end(), PointerDeleter());
		delete m_cachedTextureVector;
	}

	if (m_textureFactorIntOperationVector)
	{
		std::for_each(m_textureFactorIntOperationVector->begin(), m_textureFactorIntOperationVector->end(), PointerDeleter());
		delete m_textureFactorIntOperationVector;
	}

	if (m_textureIntOperationVector)
	{
		std::for_each(m_textureIntOperationVector->begin(), m_textureIntOperationVector->end(), PointerDeleter());
		delete m_textureIntOperationVector;
	}

	if (m_customizedMaterialVector)
	{
		std::for_each(m_customizedMaterialVector->begin(), m_customizedMaterialVector->end(), PointerDeleter());
		delete m_customizedMaterialVector;
	}

	std::for_each(m_intVariableFactoryVector->begin(), m_intVariableFactoryVector->end(), PointerDeleter());
	delete m_intVariableFactoryVector;

	if (m_baseShaderTemplate)
	{
		m_baseShaderTemplate->release();
		m_baseShaderTemplate = 0;
	}
}

// ----------------------------------------------------------------------

void CustomizableShaderTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		//-- load the base shader template
		m_baseShaderTemplate = ShaderTemplateList::fetch(iff);
		NOT_NULL(m_baseShaderTemplate);

		//-- load material-related customizations
		if (iff.enterForm(TAG_MATR, true))
		{
			m_customizedMaterialVector = new CustomizedMaterialVector();

			while (!iff.atEndOfForm())
			{
				CustomizedMaterial *const customizedMaterial = new CustomizedMaterial();
				customizedMaterial->load_0000(iff, *this);

				m_customizedMaterialVector->push_back(customizedMaterial);
			}
			iff.exitForm(TAG_MATR);
		}

		//-- load texture-related customizations
		if (iff.enterForm(TAG_TXTR, true))
		{
			//-- Load texture names.
			iff.enterChunk(TAG_DATA);
				
				const int textureCount = static_cast<int>(iff.read_int16());
				m_cachedTextureVector  = new CachedTextureVector(static_cast<CachedTextureVector::size_type>(textureCount));

				char texturePathName[4 * MAX_PATH];
				for (int i = 0; i < textureCount; ++i)
				{
					//-- Load texture path name.
					iff.read_string(texturePathName, sizeof(texturePathName) - 1);

					//-- Create cached texture entry.
					(*m_cachedTextureVector)[static_cast<CachedTextureVector::size_type>(i)] = new CachedTexture(texturePathName);
				}

			iff.exitChunk(TAG_DATA);

			//-- Load texture customization operations.
			iff.enterForm(TAG_CUST);

				m_textureIntOperationVector = new TextureIntOperationVector();

				while (!iff.atEndOfForm())
				{
					TextureIntOperation *const newOperation = new TextureIntOperation();
					newOperation->load_0000(iff, *this);

					m_textureIntOperationVector->push_back(newOperation);
				}

			iff.exitForm(TAG_CUST);

			iff.exitForm(TAG_TXTR);
		}

		//-- load tfactor-related customizations
		if (iff.enterForm(TAG_TFAC, true))
		{
			iff.exitForm(TAG_TFAC);
		}

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void CustomizableShaderTemplate::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);

		//-- load the base shader template
		m_baseShaderTemplate = ShaderTemplateList::fetch(iff);
		NOT_NULL(m_baseShaderTemplate);

		//-- load material-related customizations
		if (iff.enterForm(TAG_MATR, true))
		{
			if (!m_customizedMaterialVector)
				m_customizedMaterialVector = new CustomizedMaterialVector();

			while (!iff.atEndOfForm())
			{
				CustomizedMaterial *const customizedMaterial = new CustomizedMaterial();
				customizedMaterial->load_0001(iff, *this);

				m_customizedMaterialVector->push_back(customizedMaterial);
			}
			iff.exitForm(TAG_MATR);
		}

		//-- load texture-related customizations
		if (iff.enterForm(TAG_TXTR, true))
		{
			//-- Load texture names.
			iff.enterChunk(TAG_DATA);
				
				const int textureCount = static_cast<int>(iff.read_int16());
				if (!m_cachedTextureVector)
					m_cachedTextureVector = new CachedTextureVector(static_cast<CachedTextureVector::size_type>(textureCount));

				char texturePathName[4 * MAX_PATH];
				for (int i = 0; i < textureCount; ++i)
				{
					//-- Load texture path name.
					iff.read_string(texturePathName, sizeof(texturePathName) - 1);

					//-- Create cached texture entry.
					(*m_cachedTextureVector)[static_cast<CachedTextureVector::size_type>(i)] = new CachedTexture(texturePathName);
				}

			iff.exitChunk(TAG_DATA);

			//-- Load texture customization operations.
			iff.enterForm(TAG_CUST);

				if (!m_textureIntOperationVector)
					m_textureIntOperationVector = new TextureIntOperationVector();

				while (!iff.atEndOfForm())
				{
					TextureIntOperation *const newOperation = new TextureIntOperation();
					newOperation->load_0001(iff, *this);

					m_textureIntOperationVector->push_back(newOperation);
				}

			iff.exitForm(TAG_CUST);

			iff.exitForm(TAG_TXTR);
		}

		//-- load tfactor-related customizations
		if (iff.enterForm(TAG_TFAC, true))
		{
			// Ensure storage exists.
			if (!m_textureFactorIntOperationVector)
				m_textureFactorIntOperationVector = new TextureFactorIntOperationVector();

			// Load each tfactor int operation.
			while (!iff.atEndOfForm())
				m_textureFactorIntOperationVector->push_back(TextureFactorIntOperation::create_0001(iff, *this));

			iff.exitForm(TAG_TFAC);
		}

	iff.exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

int CustomizableShaderTemplate::submitRangedIntVariable(const std::string &variableName, bool variableIsPrivate, int minValueInclusive, int defaultValue, int maxValueExclusive)
{
	IntVariableFactory *intVariableFactory = 0;
	int                 variableIndex      = -1;

	if (!findIntVariable(variableName, variableIsPrivate, variableIndex, intVariableFactory))
	{
		// the variable doesn't exist, create it and return index
		m_intVariableFactoryVector->push_back(new RangedIntVariableFactory(variableName, variableIsPrivate, minValueInclusive, defaultValue, maxValueExclusive));
		return static_cast<int>(m_intVariableFactoryVector->size()) - 1;
	}
	else
	{
		// The variable already exists.
		//-- Ensure its the right type.
		DEBUG_FATAL(!dynamic_cast<RangedIntVariableFactory*>(intVariableFactory), ("tried to create the same variable name [%s] but with different type", variableName.c_str()));

		//-- return index of existing variable
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, variableIndex, static_cast<int>(m_intVariableFactoryVector->size()));
		return variableIndex;
	}
}

// ----------------------------------------------------------------------

int CustomizableShaderTemplate::submitPaletteColorVariable(const std::string &variableName, bool variableIsPrivate, const char *palettePathName, int defaultValue)
{
	IntVariableFactory *intVariableFactory = 0;
	int                 variableIndex      = -1;

	if (!findIntVariable(variableName, variableIsPrivate, variableIndex, intVariableFactory))
	{
		// the variable doesn't exist, create it and return index
		m_intVariableFactoryVector->push_back(new PaletteColorVariableFactory(variableName, variableIsPrivate, TemporaryCrcString(palettePathName, true), defaultValue));
		return static_cast<int>(m_intVariableFactoryVector->size()) - 1;
	}
	else
	{
		// the variable already exists
		//-- ensure its the right type
		DEBUG_FATAL(!dynamic_cast<PaletteColorVariableFactory*>(intVariableFactory), ("tried to create the same variable name [%s] but with different type", variableName.c_str()));

		//-- return index of existing variable
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, variableIndex, static_cast<int>(m_intVariableFactoryVector->size()));
		return variableIndex;
	}
}

// ----------------------------------------------------------------------

bool CustomizableShaderTemplate::findIntVariable(const std::string &variableName, bool variableIsPrivate, int &index, IntVariableFactory *&variableFactory)
{
	index = 0;

	const IntVariableFactoryVector::iterator endIt = m_intVariableFactoryVector->end();
	for (IntVariableFactoryVector::iterator it = m_intVariableFactoryVector->begin(); it != endIt; ++it, ++index)
	{
		NOT_NULL(*it);

		if ((variableName == (*it)->getVariableName()) && 
			  (variableIsPrivate == (*it)->isVariablePrivate())) //lint !e731 // boolean argument to equal/not equal // yes, this is the intended logic.
		{
			// found a match
			variableFactory = *it;
			return true;
		}
	}

	//-- not found
	index = -1;
	variableFactory = 0;
	return false;
}

// ======================================================================
