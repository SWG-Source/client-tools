// ======================================================================
//
// BlueprintTextureRendererTemplate.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "clientTextureRenderer/FirstClientTextureRenderer.h"
#include "clientTextureRenderer/BlueprintTextureRendererTemplate.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Material.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticIndexBuffer.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientTextureRenderer/BlueprintSharedTextureRenderer.h"
#include "clientTextureRenderer/BlueprintTextureRenderer.h"
#include "clientTextureRenderer/TextureRendererList.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFoundation/VoidBindSecond.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/PaletteArgb.h"
#include "sharedMath/PaletteArgbList.h"
#include "sharedObject/BasicRangedIntCustomizationVariable.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"

#include <algorithm>
#include <limits>
#include <map>
#include <string>
#include <vector>

// ======================================================================

#ifdef _DEBUG

#define CLAMP_VARIABLES 1

#else

// -TRF- this should be turned off in release mode once we have
//       available variables reporting the valid ranges for a variable.
#define CLAMP_VARIABLES 1

#endif

namespace BlueprintTextureRendererTemplateNamespace
{
	// this converts from the old texture format table to the new table
	TextureFormat const textureFormatConversionTable[] =
	{
		TF_RGB_555,     // TF_RGB_555
		TF_RGB_565,     // TF_RGB_565
		TF_ARGB_1555,   // TF_ARGB_1555
		TF_ARGB_4444,   // TF_ARGB_4444
		TF_RGB_888,     // TF_RGB_888
		TF_XRGB_8888,   // TF_RGB_888_32
		TF_ARGB_8888,   // TF_ARGB_8888
		TF_L_8,         // TF_Luminance_8
		TF_Count,       // TF_Luminance_Alpha_88
		TF_Count,       // TF_Luminance_Alpha_44
		TF_DXT1,        // TF_DXT1
		TF_DXT2,        // TF_DXT2
		TF_DXT3,        // TF_DXT3
		TF_DXT4,        // TF_DXT4
		TF_DXT5,        // TF_DXT5
		TF_A_8,         // TF_A_8
		TF_Count,       // TF_I_8
		TF_Count,       // TF_RGB_332
		TF_Count,       // TF_ARGB_8332
		TF_P_8,         // TF_RGB_P8
		TF_Count,       // TF_RG_44
		TF_Count,       // TF_RG_88
		TF_Count,       // TF_ARG_555
		TF_Count,       // TF_ARG_655
		TF_Count,       // TF_ARG_888
	};
}
using namespace BlueprintTextureRendererTemplateNamespace;

// ======================================================================
// lint supression
// ======================================================================
//lint -esym(754, BlueprintTextureRendererCamera::BlueprintTextureRendererCamera)
//lint -esym(754, BlueprintTextureRendererCamera::operator=)
//lint -esym(754, ClearFrameBufferRenderCommand::ClearFrameBufferRenderCommand)
//lint -esym(754, ClearFrameBufferRenderCommand::operator=)
//lint -esym(754, ParallelProjectionCameraSetup::ParallelProjectionCameraSetup)
//lint -esym(754, ParallelProjectionCameraSetup::operator=)
//lint -esym(754, PrepareCommand::PrepareCommand);
//lint -esym(754, PrepareCommand::operator=);
//lint -esym(754, RenderCommand::RenderCommand)
//lint -esym(754, RenderCommand::operator=)
//lint -esym(754, ShaderRenderSetCommand::ShaderRenderSetCommand)
//lint -esym(754, ShaderRenderSetCommand::operator=)
//lint -esym(754, TriListIndexedRenderCommand::TriListIndexedRenderCommand)
//lint -esym(754, TriListIndexedRenderCommand::operator=)
//lint -esym(754, TriFanRenderCommand::TriFanRenderCommand)
//lint -esym(754, TriFanRenderCommand::operator=)

//lint -e66 // bad type, having problem compiling for_each(ShaderTemplateContainer)

// ======================================================================

const Tag TAG_AND  = TAG3(A,N,D);
const Tag TAG_BTRT = TAG(B,T,R,T);
const Tag TAG_COND = TAG(C,O,N,D);
const Tag TAG_CVTP = TAG(C,V,T,P);
const Tag TAG_DEST = TAG(D,E,S,T);
const Tag TAG_FALS = TAG(F,A,L,S);
const Tag TAG_FOPS = TAG(F,O,P,S);
const Tag TAG_NONE = TAG(N,O,N,E);
const Tag TAG_NOP  = TAG3(N,O,P);
const Tag TAG_OR   = TAG2(O,R);
const Tag TAG_PCMD = TAG(P,C,M,D);
const Tag TAG_PCMS = TAG(P,C,M,S);
const Tag TAG_RTFC = TAG(R,T,F,C);
const Tag TAG_RTLI = TAG(R,T,L,I);
const Tag TAG_SST1 = TAG(S,S,T,1);
const Tag TAG_SST2 = TAG(S,S,T,2);
const Tag TAG_SSTC = TAG(S,S,T,C);
const Tag TAG_STF  = TAG3(S,T,F);
const Tag TAG_STFA = TAG(S,T,F,A);
const Tag TAG_STFP = TAG(S,T,F,P);
const Tag TAG_TOPS = TAG(T,O,P,S);
const Tag TAG_TRUE = TAG(T,R,U,E);


// ======================================================================
// embedded entity declarations
// ======================================================================

namespace
{
	class Condition;
	class PrepareOperation;
}

// ======================================================================

class BlueprintTextureRendererTemplate::PrepareCommand
{
public:

	PrepareCommand();
	~PrepareCommand();

	void load_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate);

	void execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const;

private:

	typedef std::vector<PrepareOperation*> PrepareOperationContainer;

private:

	Condition                 *m_condition;
	PrepareOperationContainer *m_trueOperations;
	PrepareOperationContainer *m_falseOperations;

private:
	// disabled
	PrepareCommand(const PrepareCommand&);
	PrepareCommand &operator =(const PrepareCommand&);
};

// ======================================================================

class BlueprintTextureRendererTemplate::RenderCommand
{
public:

	static RenderCommand *createRenderCommand_0001(Iff *iff);
	static RenderCommand *createRenderPrimitive_0001(Iff *iff);

public:

	RenderCommand() {}
	virtual ~RenderCommand() {}
	
	virtual void execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const = 0;

private:
	// disabled
	RenderCommand(const RenderCommand&);
	RenderCommand &operator =(const RenderCommand&);
};

// ======================================================================

class BlueprintTextureRendererTemplate::CameraSetup
{
public:

	static CameraSetup *create_0001(Iff *iff);

public:

	CameraSetup() {}
	virtual ~CameraSetup() {}

	virtual const Camera *getCamera(const Texture &destTexture, const IntVector &intValues, int mipmapLevel) const = 0;
	
};

// ======================================================================

class BlueprintTextureRendererTemplate::VariableFactory
{
public:

	virtual ~VariableFactory();

	virtual CustomizationVariable *createCustomizationVariable() const = 0;

	const std::string &getName() const;
	bool               isPrivate() const;

protected:

	explicit VariableFactory(const std::string &name, bool isPrivate);

private:

	// disabled
	VariableFactory();

private:

	std::string  m_name;
	bool         m_isPrivate;

};

// ----------------------------------------------------------------------

class BlueprintTextureRendererTemplate::BasicRangedIntVariableFactory: public BlueprintTextureRendererTemplate::VariableFactory
{
public:

	BasicRangedIntVariableFactory(const std::string &name, bool isPrivate, int minValueInclusive, int maxValueExclusive);

	virtual CustomizationVariable *createCustomizationVariable() const;

private:

	// disabled
	BasicRangedIntVariableFactory();
	BasicRangedIntVariableFactory(const BasicRangedIntVariableFactory&);             //lint -esym(754, BasicRangedIntVariableFactory::BasicRangedIntVariableFactory) // not referenced // defensive disabling
	BasicRangedIntVariableFactory &operator =(const BasicRangedIntVariableFactory&); //lint -esym(754, BasicRangedIntVariableFactory::operator=) // not referenced // defensive disabling

private:

	int  m_minValueInclusive;
	int  m_maxValueExclusive;

};

// ----------------------------------------------------------------------

class BlueprintTextureRendererTemplate::PaletteColorVariableFactory: public BlueprintTextureRendererTemplate::VariableFactory
{
public:

	PaletteColorVariableFactory(const std::string &name, bool isPrivate, const PaletteArgb *palette);
	virtual ~PaletteColorVariableFactory();

	virtual CustomizationVariable *createCustomizationVariable() const;

private:

	// disabled
	PaletteColorVariableFactory();
	PaletteColorVariableFactory(const PaletteColorVariableFactory&);             //lint -esym(754, PaletteColorVariableFactory::PaletteColorVariableFactory) // not referenced // defensive disabling
	PaletteColorVariableFactory &operator =(const PaletteColorVariableFactory&); //lint -esym(754, PaletteColorVariableFactory::operator=) // not referenced // must be disabled
	
private:

	const PaletteArgb *const m_palette;

};

// ======================================================================

namespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef stdvector<int>::fwd      IntVector;
	typedef stdvector<Shader*>::fwd  ShaderVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class ClearFrameBufferRenderCommand: public BlueprintTextureRendererTemplate::RenderCommand
	{
	public:

		ClearFrameBufferRenderCommand();

		virtual void execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const;
		void         load_0001(Iff *iff);

	private:

		bool    m_clearColor;
		uint32  m_colorValue;

		bool    m_clearDepth;
		real    m_depthValue;

		bool    m_clearStencil;
		uint32  m_stencilValue;

	private:
		// disabled
		ClearFrameBufferRenderCommand(const ClearFrameBufferRenderCommand&);
		ClearFrameBufferRenderCommand &operator =(const ClearFrameBufferRenderCommand&);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class ShaderRenderSetCommand: public BlueprintTextureRendererTemplate::RenderCommand
	{
	public:

		ShaderRenderSetCommand();
		virtual ~ShaderRenderSetCommand();

		virtual void execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const;
		void         load_0001(Iff *iff);

	private:

		typedef std::vector<BlueprintTextureRendererTemplate::RenderCommand*> RenderCommandContainer;
		typedef RenderCommandContainer::const_iterator                        RCConstIterator;

	private:

		int                     m_shaderIndex;
		RenderCommandContainer  m_renderCommands;

	private:
		// disabled
		ShaderRenderSetCommand(const ShaderRenderSetCommand&);
		ShaderRenderSetCommand &operator =(const ShaderRenderSetCommand&);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class TriListIndexedRenderCommand: public BlueprintTextureRendererTemplate::RenderCommand
	{
	public:

		TriListIndexedRenderCommand();

		virtual void execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const;
		void         load_0001(Iff *iff);

	private:

		int  m_vertexBufferIndex;
		int  m_indexBufferIndex;
		int  m_vertexBufferMinimumIndex;
		int  m_vertexBufferNumberOfVertices;
		int  m_indexBufferStartIndex;
		int  m_primitiveCount;

	private:
		// disabled
		TriListIndexedRenderCommand(const TriListIndexedRenderCommand&);
		TriListIndexedRenderCommand &operator =(const TriListIndexedRenderCommand&);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class TriFanRenderCommand: public BlueprintTextureRendererTemplate::RenderCommand
	{
	public:

		TriFanRenderCommand();

		virtual void execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const;
		void         load_0001(Iff *iff);

	private:

		int  m_vertexBufferIndex;

	private:
		// disabled
		TriFanRenderCommand(const TriFanRenderCommand&);
		TriFanRenderCommand &operator =(const TriFanRenderCommand&);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Condition
	{
	public:

		static Condition *create_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate);

	public:

		Condition() {}
		virtual ~Condition() {}

		virtual bool evaluate(const IntVector &intValues) const = 0;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class TrueAlwaysCondition: public Condition
	{
	public:

		TrueAlwaysCondition();

		void         load_0001(Iff *iff) const;
		virtual bool evaluate(const IntVector &intValues) const;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class FalseAlwaysCondition: public Condition
	{
	public:

		FalseAlwaysCondition();

		void         load_0001(Iff *iff) const;
		virtual bool evaluate(const IntVector &intValues) const;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class OrCondition: public Condition
	{
	public:

		OrCondition();
		virtual ~OrCondition();

		void         load_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate);
		virtual bool evaluate(const IntVector &intValues) const;

	private:

		typedef std::vector<Condition*> ConditionContainer;

	private:

		ConditionContainer  m_conditions;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class AndCondition: public Condition
	{
	public:

		AndCondition();
		virtual ~AndCondition();

		void         load_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate);
		virtual bool evaluate(const IntVector &intValues) const;

	private:

		typedef std::vector<Condition*> ConditionContainer;

	private:

		ConditionContainer  m_conditions;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class PrepareOperation
	{
	public:

		static PrepareOperation *create_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate);

	public:

		PrepareOperation() {}
		virtual ~PrepareOperation() {}

		virtual void execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const = 0;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class NoOperation: public PrepareOperation
	{
	public:

		NoOperation();

		void         load_0001(Iff *iff) const;

		virtual void execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class SetShaderConstantTextureOperation: public PrepareOperation
	{
	public:

		SetShaderConstantTextureOperation();
		virtual ~SetShaderConstantTextureOperation();

		void         load_0001(Iff *iff);

		virtual void execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const;

	private:

		int  m_shaderIndex;
		Tag  m_textureTag;
		int  m_textureIndex;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class SetShaderTexture1dOperation: public PrepareOperation
	{
	public:

		SetShaderTexture1dOperation();
		virtual ~SetShaderTexture1dOperation();

		void         load_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate);

		virtual void execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const;

	private:

		int  m_shaderIndex;
		Tag  m_textureTag;
		int  m_baseTextureIndex;

		int  m_textureIndexVariableId;
		int  m_entryCountSubscript0;
	
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class SetShaderTexture2dOperation: public PrepareOperation
	{
	public:

		SetShaderTexture2dOperation();
		virtual ~SetShaderTexture2dOperation();

		void         load_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate);

		virtual void execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const;

	private:

		int  m_shaderIndex;
		Tag  m_textureTag;
		int  m_baseTextureIndex;

		int  m_subscript0VariableId;
		int  m_entryCountSubscript0;

		int  m_subscript1VariableId;
		int  m_entryCountSubscript1;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class SetShaderTextureFactorOperation: public PrepareOperation
	{
	public:

		SetShaderTextureFactorOperation();
		virtual ~SetShaderTextureFactorOperation();

		void         load_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate);

		virtual void execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const;

	private:

		int         m_shaderIndex;
		Tag         m_textureFactorTag;
		int         m_textureFactorVariableId;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class SetShaderTextureFactorAlphaOperation: public PrepareOperation
	{
	public:

		SetShaderTextureFactorAlphaOperation();
		virtual ~SetShaderTextureFactorAlphaOperation();

		void         load_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate);

		virtual void execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const;

	private:

		int         m_shaderIndex;
		Tag         m_textureFactorTag;
		int         m_alphaVariableId;
		PackedArgb  m_constantColor;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class SetShaderTextureFactorFromPaletteOperation: public PrepareOperation
	{
	public:

		SetShaderTextureFactorFromPaletteOperation();
		virtual ~SetShaderTextureFactorFromPaletteOperation();

		void         load_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate);

		virtual void execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const;

	private:

		int                m_shaderIndex;
		Tag                m_textureFactorNameTag;
		const PaletteArgb *m_palette;
		int                m_variableIndex; // index into intValues for the palette entry variable (the variable has the value, this is not the value).
		// bool               m_variableIsPrivate;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class ParallelProjectionCameraSetup: public BlueprintTextureRendererTemplate::CameraSetup
	{
	public:

		ParallelProjectionCameraSetup();

		virtual const Camera *getCamera(const Texture &destTexture, const IntVector &intValues, int mipmapLevel) const;
		void                  load_0001(Iff *iff);

	private:

		real m_normalizedLength;

	private:
		// disabled
		ParallelProjectionCameraSetup(const ParallelProjectionCameraSetup&);
		ParallelProjectionCameraSetup &operator =(const ParallelProjectionCameraSetup&);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	/**
	 * Defines the BlueprintTextureRendererTemplate camera.
	 *
	 * This is here solely to satisfy the Camera interface forcing definition
	 * of the Camera::drawScene() function.  BlueprintTextureRendererTemplate drives
	 * all rendering through a different mechanism than the traditional
	 * "add objects to camera" approach, so this camera doesn't do
	 * anything different than Camera.
	 */

	class BlueprintTextureRendererCamera: public Camera
	{
	public:

		BlueprintTextureRendererCamera();

	protected:

		virtual void drawScene(void) const;

	private:
		// disabled
		BlueprintTextureRendererCamera(const BlueprintTextureRendererCamera&);
		const BlueprintTextureRendererCamera &operator =(const BlueprintTextureRendererCamera&);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool                            ms_installed;
	BlueprintTextureRendererCamera *ms_camera;

}

// ======================================================================
// class BlueprintTextureRendererCamera
// ======================================================================

BlueprintTextureRendererCamera::BlueprintTextureRendererCamera()
:
	Camera()
{
}

// ----------------------------------------------------------------------

void BlueprintTextureRendererCamera::drawScene(void) const
{
	// do nothing
}

// ======================================================================
// class ClearFrameBufferRenderCommand
// ======================================================================

ClearFrameBufferRenderCommand::ClearFrameBufferRenderCommand()
:
	RenderCommand(),
	m_clearColor(true),
	m_colorValue(PackedArgb::solidBlack.getArgb()),
	m_clearDepth(true),
	m_depthValue(CONST_REAL(1)),
	m_clearStencil(true),
	m_stencilValue(0)
{
}

// ----------------------------------------------------------------------

void ClearFrameBufferRenderCommand::execute(const BlueprintTextureRendererTemplate &, const IntVector &, ShaderVector &) const
{
	Graphics::clearViewport(m_clearColor, m_colorValue, m_clearDepth, m_depthValue, m_clearStencil, m_stencilValue);
}

// ----------------------------------------------------------------------

void ClearFrameBufferRenderCommand::load_0001(Iff *iff)
{
	NOT_NULL(iff);

	iff->enterChunk(TAG(C,F,B,C));

		m_clearColor   = (iff->read_int32() != 0);
		m_colorValue   = iff->read_uint32();

		m_clearDepth   = (iff->read_int32() != 0);
		m_depthValue   = iff->read_float();

		m_clearStencil = (iff->read_int32() != 0);
		m_stencilValue = iff->read_uint32();

	iff->exitChunk(TAG(C,F,B,C));
}

// ======================================================================
// class ShaderRenderSetCommand
// ======================================================================

ShaderRenderSetCommand::ShaderRenderSetCommand()
: RenderCommand(),
	m_shaderIndex(0),
	m_renderCommands()
{
}

// ----------------------------------------------------------------------

ShaderRenderSetCommand::~ShaderRenderSetCommand()
{
	std::for_each(m_renderCommands.begin(), m_renderCommands.end(), PointerDeleter());
}

// ----------------------------------------------------------------------

void ShaderRenderSetCommand::execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const
{
	UNREF(trTemplate);

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_shaderIndex, static_cast<int>(shaders.size()));
	const StaticShader &shader = NON_NULL(shaders[static_cast<size_t>(m_shaderIndex)])->prepareToView();

	const int passCount = shader.getNumberOfPasses();
	for (int i = 0; i < passCount; ++i)
	{
		Graphics::setStaticShader(shader, i);
		RCConstIterator itEnd = m_renderCommands.end();
		for (RCConstIterator it = m_renderCommands.begin(); it != itEnd; ++it)
			(*it)->execute(trTemplate, intValues, shaders);
	}
}

// ----------------------------------------------------------------------

void ShaderRenderSetCommand::load_0001(Iff *iff)
{
	NOT_NULL(iff);

	iff->enterForm(TAG(S,R,S,C));

		iff->enterChunk(TAG_INFO);
		
			m_shaderIndex                = iff->read_int32();
			const int renderCommandCount = iff->read_int32();

			m_renderCommands.reserve(static_cast<size_t>(renderCommandCount));

		iff->exitChunk(TAG_INFO);

		for (int i = 0; i < renderCommandCount; ++i)
		{
			RenderCommandContainer::value_type renderCommand(BlueprintTextureRendererTemplate::RenderCommand::createRenderPrimitive_0001(iff));
			m_renderCommands.push_back(renderCommand);
		}

	iff->exitForm(TAG(S,R,S,C));
}

// ======================================================================
// class TriListIndexedRenderCommand
// ======================================================================

TriListIndexedRenderCommand::TriListIndexedRenderCommand() :
	RenderCommand(),
	m_vertexBufferIndex(0),
	m_indexBufferIndex(0),
	m_vertexBufferMinimumIndex(0),
	m_vertexBufferNumberOfVertices(0),
	m_indexBufferStartIndex(0),
	m_primitiveCount(0)
{
}

// ----------------------------------------------------------------------

void TriListIndexedRenderCommand::execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &, ShaderVector &) const
{
	const StaticVertexBuffer &vb = trTemplate.getVertexBuffer(m_vertexBufferIndex);
	const StaticIndexBuffer  &ib = trTemplate.getIndexBuffer(m_indexBufferIndex);

	Graphics::setVertexBuffer(vb);
	Graphics::setIndexBuffer(ib);

	Graphics::drawIndexedTriangleList(0, m_vertexBufferMinimumIndex, m_vertexBufferNumberOfVertices, m_indexBufferStartIndex, m_primitiveCount);

#if 0
	ib.lockReadOnly();
	vb.lockReadOnly();

		const int indexCount   = ib.getNumberOfIndices();
		const Index *indexData = ib.getIndexData();

		DEBUG_REPORT_LOG(true, ("RenderTriList (%d indices)\n", indexCount));

		for (int i = 0; i < indexCount; ++i)
		{
			//-- get position of the vert in object space
			const int    index      = static_cast<int>(indexData[i]);
			const Vector position_o = vb.getXYZ(index);

			//-- grab u,v
			real u;
			real v;

			vb.getTextureCoordinates(index, 0, u, v);

			//-- get position of the vert in screen space assuming use of ms_camera
			real x_s;
			real y_s;
			real z_s;
			
			const bool inFrustum = ms_camera->projectInWorldSpace(position_o, &x_s, &y_s, &z_s, true);
			DEBUG_REPORT_LOG(true, ("--(%7.2f,%7.2f,%7.2f) => (%7.2f,%7.2f,%7.2f) (%5.2f, %5.2f) [%s]\n", position_o.x, position_o.y, position_o.z, x_s, y_s, z_s, u, v, inFrustum ? "inside frustum" : "outside frustum"));
		}

	vb.unlock();
	ib.unlock();
#endif
}

// ----------------------------------------------------------------------

void TriListIndexedRenderCommand::load_0001(Iff *iff)
{
	iff->enterChunk(TAG_RTLI);

		m_vertexBufferIndex            = iff->read_int32();
		m_indexBufferIndex             = iff->read_int32();
		m_vertexBufferMinimumIndex     = iff->read_int32();
		m_vertexBufferNumberOfVertices = iff->read_int32();
		m_indexBufferStartIndex        = iff->read_int32();
		m_primitiveCount               = iff->read_int32();

	iff->exitChunk(TAG_RTLI);
}

// ======================================================================
// class TriFanRenderCommand
// ======================================================================

TriFanRenderCommand::TriFanRenderCommand()
:
	RenderCommand(),
	m_vertexBufferIndex(0)
{
}

// ----------------------------------------------------------------------

void TriFanRenderCommand::execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &, ShaderVector &) const
{
	const StaticVertexBuffer &vb = trTemplate.getVertexBuffer(m_vertexBufferIndex);

	Graphics::setVertexBuffer(vb);
	Graphics::drawTriangleFan();
}

// ----------------------------------------------------------------------

void TriFanRenderCommand::load_0001(Iff *iff)
{
	iff->enterChunk(TAG_RTFC);

		m_vertexBufferIndex = static_cast<int>(iff->read_int32());

	iff->exitChunk(TAG_RTFC);
}

// ======================================================================
// class BlueprintTextureRendererTemplate::RenderCommand
// ======================================================================

BlueprintTextureRendererTemplate::RenderCommand *BlueprintTextureRendererTemplate::RenderCommand::createRenderCommand_0001(Iff *iff)
{
	NOT_NULL(iff);

	switch (iff->getCurrentName())
	{
		case TAG(C,F,B,C):
			{
				ClearFrameBufferRenderCommand *const renderCommand = NON_NULL(new ClearFrameBufferRenderCommand());
				renderCommand->load_0001(iff);
				return renderCommand;
			}
		case TAG(S,R,S,C):
			{
				ShaderRenderSetCommand *const renderCommand = NON_NULL(new ShaderRenderSetCommand());
				renderCommand->load_0001(iff);
				return renderCommand;
			}
		default:
			{
				char name[256];
				iff->formatLocation(name, sizeof(name));
				FATAL(true, ("unsupported BlueprintTextureRenderer render command [%s]", name));
				return 0; //lint !e527 // unreachable // yes, joyful MSVC
			}
	}
}

// ----------------------------------------------------------------------

BlueprintTextureRendererTemplate::RenderCommand *BlueprintTextureRendererTemplate::RenderCommand::createRenderPrimitive_0001(Iff *iff)
{
	NOT_NULL(iff);

	switch (iff->getCurrentName())
	{
		case TAG_RTLI:
			{
				TriListIndexedRenderCommand *const renderCommand = NON_NULL(new TriListIndexedRenderCommand());
				renderCommand->load_0001(iff);
				return renderCommand;
			}
		case TAG_RTFC:
			{
				TriFanRenderCommand *const renderCommand = NON_NULL(new TriFanRenderCommand());
				renderCommand->load_0001(iff);
				return renderCommand;
			}
		default:
			{
				char name[256];
				iff->formatLocation(name, sizeof(name));
				FATAL(true, ("unsupported BlueprintTextureRenderer render command [%s]", name));
				return 0; //lint !e527 // unreachable // yes, joyful MSVC
			}
	}
}

// ======================================================================
// class BlueprintTextureRendererTemplate::PrepareCommand
// ======================================================================

BlueprintTextureRendererTemplate::PrepareCommand::PrepareCommand()
:	m_condition(0),
	m_trueOperations(0),
	m_falseOperations(0)
{
}

// ----------------------------------------------------------------------

void BlueprintTextureRendererTemplate::PrepareCommand::load_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate)
{
	NOT_NULL(iff);

	iff->enterForm(TAG_PCMD);
	{
		//-- load condition.  if not present, then the condition is always true (i.e. true commands always executed).
		if (iff->getCurrentName() == TAG_COND)
		{
			iff->enterForm(TAG_COND);
				m_condition = Condition::create_0001(iff, trTemplate);
			iff->exitForm(TAG_COND);
		}

		//-- load true operations to perform
		if (iff->enterForm(TAG_TOPS, true))
		{
			// get # ops
			iff->enterChunk(TAG_INFO);
				const int opCount = iff->read_int32();
			iff->exitChunk(TAG_INFO);

			// load ops
			m_trueOperations = new PrepareOperationContainer;
			m_trueOperations->reserve(static_cast<size_t>(opCount));
			for (int i = 0; i < opCount; ++i)
				m_trueOperations->push_back(PrepareOperation::create_0001(iff, trTemplate));

			iff->exitForm(TAG_TOPS);
		}

		//-- load false operations
		if (iff->enterForm(TAG_FOPS, true))
		{
			// get # ops
			iff->enterChunk(TAG_INFO);
				const int opCount = iff->read_int32();
			iff->exitChunk(TAG_INFO);

			// load ops
			m_falseOperations = new PrepareOperationContainer;
			m_falseOperations->reserve(static_cast<size_t>(opCount));
			for (int i = 0; i < opCount; ++i)
				m_falseOperations->push_back(PrepareOperation::create_0001(iff, trTemplate));

			iff->exitForm(TAG_FOPS);
		}
	}
	iff->exitForm(TAG_PCMD);
}

// ----------------------------------------------------------------------

BlueprintTextureRendererTemplate::PrepareCommand::~PrepareCommand()
{
	delete m_condition;
	
	if (m_trueOperations)
	{
		std::for_each(m_trueOperations->begin(), m_trueOperations->end(), PointerDeleter());
		delete m_trueOperations;
	}

	if (m_falseOperations)
	{
		std::for_each(m_falseOperations->begin(), m_falseOperations->end(), PointerDeleter());
		delete m_falseOperations;
	}
}

// ----------------------------------------------------------------------

void BlueprintTextureRendererTemplate::PrepareCommand::execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const
{
	if (!m_condition || m_condition->evaluate(intValues))
	{
		// run the tru operations
		if (m_trueOperations)
		{
			PrepareOperationContainer::const_iterator itEnd = m_trueOperations->end();
			for (PrepareOperationContainer::const_iterator it = m_trueOperations->begin(); it != itEnd; ++it)
				(*it)->execute(trTemplate, intValues, shaders);
		}
	}
	else
	{
		// run the false operations
		if (m_falseOperations)
		{
			PrepareOperationContainer::const_iterator itEnd = m_falseOperations->end();
			for (PrepareOperationContainer::const_iterator it = m_falseOperations->begin(); it != itEnd; ++it)
				(*it)->execute(trTemplate, intValues, shaders);
		}
	}
}

// ======================================================================
// class ParallelProjectionCameraSetup
// ======================================================================

ParallelProjectionCameraSetup::ParallelProjectionCameraSetup()
:
	CameraSetup(),
	m_normalizedLength(CONST_REAL(1))
{
}

// ----------------------------------------------------------------------

const Camera *ParallelProjectionCameraSetup::getCamera(const Texture &destTexture, const IntVector &intValues, int mipmapLevel) const
{
	UNREF(intValues);

	NOT_NULL(ms_camera);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, mipmapLevel, destTexture.getMipmapLevelCount());

	const uint  uMipmapLevel   = static_cast<uint>(mipmapLevel);
	const uint  uTextureWidth  = static_cast<uint>(destTexture.getWidth());
	const uint  uTextureHeight = static_cast<uint>(destTexture.getHeight());

	const int viewportWidth  = std::max(1, static_cast<int>(uTextureWidth >> uMipmapLevel));
	const int viewportHeight = std::max(1, static_cast<int>(uTextureHeight >> uMipmapLevel));
	ms_camera->setViewport(0, 0, viewportWidth, viewportHeight);

	ms_camera->setParallelProjection(CONST_REAL(0), CONST_REAL(0), m_normalizedLength, m_normalizedLength);

	return ms_camera;
}

// ----------------------------------------------------------------------

void ParallelProjectionCameraSetup::load_0001(Iff *iff)
{
	NOT_NULL(iff);

	iff->enterChunk(TAG(P,C,A,M));
	{
		m_normalizedLength = iff->read_float();
	}
	iff->exitChunk(TAG(P,C,A,M));
}

// ======================================================================
// class CameraSetup
// ======================================================================

BlueprintTextureRendererTemplate::CameraSetup *BlueprintTextureRendererTemplate::CameraSetup::create_0001(Iff *iff)
{
	NOT_NULL(iff);

	switch (iff->getCurrentName())
	{
		case TAG(P,C,A,M):
			{
				ParallelProjectionCameraSetup *const cameraSetup = NON_NULL(new ParallelProjectionCameraSetup());
				cameraSetup->load_0001(iff);
				return cameraSetup;
			}
		case TAG(U,C,A,M):
			{
				FATAL(true, ("add support"));  // -TRF-
				return 0; //lint !e527 // unreachable
			}
		default:
			{
				char name[256];
				iff->formatLocation(name, sizeof(name));
				FATAL(true, ("unsupported BlueprintTextureRendererTemplate camera [%s]", name));
				return 0; //lint !e527 // unreachable
			}
	}
}

// ======================================================================
// class BlueprintTextureRendererTemplate::VariableFactory
// ======================================================================

BlueprintTextureRendererTemplate::VariableFactory::~VariableFactory()
{
}

// ----------------------------------------------------------------------

inline const std::string &BlueprintTextureRendererTemplate::VariableFactory::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

inline bool BlueprintTextureRendererTemplate::VariableFactory::isPrivate() const
{
	return m_isPrivate;
}

// ======================================================================

BlueprintTextureRendererTemplate::VariableFactory::VariableFactory(const std::string &name, bool newIsPrivate) :
	m_name(name),
	m_isPrivate(newIsPrivate)
{
}

// ======================================================================
// class BlueprintTextureRendererTemplate::BasicRangedIntVariableFactory
// ======================================================================

BlueprintTextureRendererTemplate::BasicRangedIntVariableFactory::BasicRangedIntVariableFactory(const std::string &name, bool newIsPrivate, int minValueInclusive, int maxValueExclusive) :
	VariableFactory(name, newIsPrivate),
	m_minValueInclusive(minValueInclusive),
	m_maxValueExclusive(maxValueExclusive)
{
}

// ----------------------------------------------------------------------

CustomizationVariable *BlueprintTextureRendererTemplate::BasicRangedIntVariableFactory::createCustomizationVariable() const
{
	// create the customization variable.  
	// note: we set the default value to the min value.
	return new BasicRangedIntCustomizationVariable(m_minValueInclusive, m_minValueInclusive, m_maxValueExclusive);
}

// ======================================================================
// class BlueprintTextureRendererTemplate::PaletteColorVariableFactory
// ======================================================================

BlueprintTextureRendererTemplate::PaletteColorVariableFactory::PaletteColorVariableFactory(const std::string &name, bool newIsPrivate, const PaletteArgb *palette) :
	VariableFactory(name, newIsPrivate),
	m_palette(palette)
{
	//-- fetch local reference to palette
	NOT_NULL(m_palette);
	m_palette->fetch();
}

// ----------------------------------------------------------------------

BlueprintTextureRendererTemplate::PaletteColorVariableFactory::~PaletteColorVariableFactory()
{
	//-- release local reference
	//lint -esym(1540, PaletteColorVariableFactory::m_palette) // neither freed nor zero'ed // we're releasing, equivalent
	m_palette->release();
}

// ----------------------------------------------------------------------

CustomizationVariable *BlueprintTextureRendererTemplate::PaletteColorVariableFactory::createCustomizationVariable() const
{
	return new PaletteColorCustomizationVariable(m_palette);
}

// ======================================================================
// class TrueAlwaysCondition
// ======================================================================

TrueAlwaysCondition::TrueAlwaysCondition()
:	Condition()
{
}

// ----------------------------------------------------------------------


void TrueAlwaysCondition::load_0001(Iff *iff) const
{
	NOT_NULL(iff);

	iff->enterChunk(TAG_TRUE);
	iff->exitChunk(TAG_TRUE);
}

// ----------------------------------------------------------------------

bool TrueAlwaysCondition::evaluate(const IntVector&) const
{
	return true;
}

// ======================================================================
// class FalseAlwaysCondition
// ======================================================================

FalseAlwaysCondition::FalseAlwaysCondition()
:	Condition()
{
}

// ----------------------------------------------------------------------


void FalseAlwaysCondition::load_0001(Iff *iff) const
{
	NOT_NULL(iff);

	iff->enterChunk(TAG_FALS);
	iff->exitChunk(TAG_FALS);
}

// ----------------------------------------------------------------------

bool FalseAlwaysCondition::evaluate(const IntVector&) const
{
	return false;
}

// ======================================================================
// class OrCondition
// ======================================================================

OrCondition::OrCondition()
:	Condition(),
	m_conditions()
{
}

// ----------------------------------------------------------------------

OrCondition::~OrCondition()
{
	IGNORE_RETURN(std::for_each(m_conditions.begin(), m_conditions.end(), PointerDeleter()));
}

// ----------------------------------------------------------------------

void OrCondition::load_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate)
{
	NOT_NULL(iff);

	iff->enterForm(TAG_OR);

		iff->enterChunk(TAG_INFO);
			const size_t conditionCount = static_cast<size_t>(iff->read_uint32());
		iff->exitChunk(TAG_INFO);

		m_conditions.reserve(conditionCount);
		for (size_t i = 0; i < conditionCount; ++i)
			m_conditions.push_back(Condition::create_0001(iff, trTemplate));

	iff->exitForm(TAG_OR);
}

// ----------------------------------------------------------------------

bool OrCondition::evaluate(const IntVector &intValues) const
{
	//-- return true if any of the conditions are true
	const ConditionContainer::const_iterator itEnd = m_conditions.end();
	for (ConditionContainer::const_iterator it = m_conditions.begin(); it != itEnd; ++it)
		if ((*it)->evaluate(intValues))
			return true;

	//-- if we get here, nothing was true, so return false
	return false;
}

// ======================================================================
// class AndCondition
// ======================================================================

AndCondition::AndCondition()
:	Condition(),
	m_conditions()
{
}

// ----------------------------------------------------------------------

AndCondition::~AndCondition()
{
	IGNORE_RETURN(std::for_each(m_conditions.begin(), m_conditions.end(), PointerDeleter()));
}

// ----------------------------------------------------------------------

void AndCondition::load_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate)
{
	NOT_NULL(iff);

	iff->enterForm(TAG_AND);

		iff->enterChunk(TAG_INFO);
			const size_t conditionCount = static_cast<size_t>(iff->read_uint32());
		iff->exitChunk(TAG_INFO);

		m_conditions.reserve(conditionCount);
		for (size_t i = 0; i < conditionCount; ++i)
			m_conditions.push_back(Condition::create_0001(iff, trTemplate));

	iff->exitForm(TAG_AND);
}

// ----------------------------------------------------------------------

bool AndCondition::evaluate(const IntVector &intValues) const
{
	//-- return false if any of the conditions are false
	const ConditionContainer::const_iterator itEnd = m_conditions.end();
	for (ConditionContainer::const_iterator it = m_conditions.begin(); it != itEnd; ++it)
		if (! ((*it)->evaluate(intValues)) )
			return false;

	//-- if we get here, nothing was false, so return true
	return true;
}

// ======================================================================
// class Condition
// ======================================================================

Condition *Condition::create_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate)
{
	NOT_NULL(iff);

	switch (iff->getCurrentName())
	{
		case TAG_TRUE:
			{
				TrueAlwaysCondition *const condition = new TrueAlwaysCondition;
				condition->load_0001(iff);
				return condition;
			}
		case TAG_FALS:
			{
				FalseAlwaysCondition *const condition = new FalseAlwaysCondition;
				condition->load_0001(iff);
				return condition;
			}
		case TAG_CVTP:
			{
				// depracated (, should no longer be generated.  For now will always generate true
				TrueAlwaysCondition *const condition = new TrueAlwaysCondition;
				condition->load_0001(iff);
				return condition;
			}
		case TAG_OR:
			{
				OrCondition *const condition = new OrCondition;
				condition->load_0001(iff, trTemplate);
				return condition;
			}
		case TAG_AND:
			{
				AndCondition *const condition = new AndCondition;
				condition->load_0001(iff, trTemplate);
				return condition;
			}
		default:
			{
				char name[5];
				ConvertTagToString(iff->getCurrentName(), name);
				FATAL(true, ("unsupported condition type [%s]", name));
				return 0; //lint !e527 // unreachable // right, for MSVC
			}
	}
}

// ======================================================================
// class NoOperation
// ======================================================================

NoOperation::NoOperation()
:	PrepareOperation()
{
}

// ----------------------------------------------------------------------

void NoOperation::load_0001(Iff *iff) const
{
	NOT_NULL(iff);

	iff->enterChunk(TAG_NOP);
	iff->exitChunk(TAG_NOP);
}

// ----------------------------------------------------------------------

void NoOperation::execute(const BlueprintTextureRendererTemplate &, const IntVector &, ShaderVector &) const
{
	// as name implies, do nothing.
}

// ======================================================================
// class SetShaderConstantTextureOperation
// ======================================================================

SetShaderConstantTextureOperation::SetShaderConstantTextureOperation()
:	PrepareOperation(),
	m_shaderIndex(-1),
	m_textureTag(TAG(N,O,N,E)),
	m_textureIndex(-1)
{
}

// ----------------------------------------------------------------------

SetShaderConstantTextureOperation::~SetShaderConstantTextureOperation()
{
}

// ----------------------------------------------------------------------

void SetShaderConstantTextureOperation::load_0001(Iff *iff)
{
	NOT_NULL(iff);

	iff->enterChunk(TAG_SSTC);

		m_shaderIndex  = iff->read_int32();
		m_textureTag   = static_cast<Tag>(iff->read_uint32());
		m_textureIndex = iff->read_int32();
		
	iff->exitChunk(TAG_SSTC);
}

// ----------------------------------------------------------------------

void SetShaderConstantTextureOperation::execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const
{
	UNREF(intValues);

	//-- get the shader
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_shaderIndex, static_cast<int>(shaders.size()));

	StaticShader *const shader = safe_cast<StaticShader*>(shaders[static_cast<size_t>(m_shaderIndex)]);
	NOT_NULL(shader);

	//-- get the texture
	const Texture *const texture = trTemplate.fetchTexture(m_textureIndex);
	NOT_NULL(texture);

	//-- set the texture into the shader
	shader->setTexture(m_textureTag, *texture);

	//-- release the texture
	texture->release();
}

// ======================================================================
// class SetShaderTexture1dOperation
// ======================================================================

SetShaderTexture1dOperation::SetShaderTexture1dOperation() :
	PrepareOperation(),
	m_shaderIndex(-1),
	m_textureTag(TAG(N,O,N,E)),
	m_baseTextureIndex(-1),
	m_textureIndexVariableId(-1),
	m_entryCountSubscript0(-1)
{
}

// ----------------------------------------------------------------------

SetShaderTexture1dOperation::~SetShaderTexture1dOperation()
{
}

// ----------------------------------------------------------------------

void SetShaderTexture1dOperation::load_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate)
{
	NOT_NULL(iff);

	iff->enterChunk(TAG_SST1);

		m_shaderIndex = static_cast<int>(iff->read_int32());
		DEBUG_FATAL(m_shaderIndex < 0, ("bad m_shaderIndex %d", m_shaderIndex));

		m_textureTag       = static_cast<Tag>(iff->read_uint32());

		m_baseTextureIndex = static_cast<int>(iff->read_int32());
		DEBUG_FATAL(m_baseTextureIndex < 0, ("bad m_baseTextureIndex %d", m_shaderIndex));

		//-- get texture variable name and range info
		char name[MAX_PATH];
		iff->read_string(name, MAX_PATH-1);

		m_entryCountSubscript0 = static_cast<int>(iff->read_int32());
		DEBUG_FATAL(m_entryCountSubscript0 < 1, ("bad m_entryCountSubscript0 %d", m_entryCountSubscript0));

		//-- tell the template about the variable
		m_textureIndexVariableId = trTemplate.submitRangedIntVariable(name, 0, m_entryCountSubscript0);

	iff->exitChunk(TAG_SST1);
}

// ----------------------------------------------------------------------

void SetShaderTexture1dOperation::execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const
{
	//-- get the shader
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_shaderIndex, static_cast<int>(shaders.size()));

	StaticShader *const shader = safe_cast<StaticShader*>(shaders[static_cast<size_t>(m_shaderIndex)]);
	NOT_NULL(shader);

	//-- get value for texture index 0
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_textureIndexVariableId, static_cast<int>(intValues.size()));
	int variableIndex = intValues[static_cast<size_t>(m_textureIndexVariableId)];

#if CLAMP_VARIABLES
	//-- clamp the range to the valid set of values
	variableIndex = clamp(0, variableIndex, m_entryCountSubscript0 - 1);
#endif

	const int textureIndex = m_baseTextureIndex + variableIndex;

	//-- get the texture
	const Texture *const texture = trTemplate.fetchTexture(textureIndex);
	NOT_NULL(texture);

	//-- set the texture into the shader
	shader->setTexture(m_textureTag, *texture);

	//-- release the texture
	texture->release();
}

// ======================================================================
// class SetShaderTexture2dOperation
// ======================================================================

SetShaderTexture2dOperation::SetShaderTexture2dOperation() :
	PrepareOperation(),
	m_shaderIndex(-1),
	m_textureTag(TAG(N,O,N,E)),
	m_baseTextureIndex(-1),
	m_subscript0VariableId(-1),
	m_entryCountSubscript0(-1),
	m_subscript1VariableId(-1),
	m_entryCountSubscript1(-1)
{
}

// ----------------------------------------------------------------------

SetShaderTexture2dOperation::~SetShaderTexture2dOperation()
{
}

// ----------------------------------------------------------------------

void SetShaderTexture2dOperation::load_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate)
{
	NOT_NULL(iff);

	iff->enterChunk(TAG_SST2);

		m_shaderIndex      = static_cast<int>(iff->read_int32());
		m_textureTag       = static_cast<Tag>(iff->read_uint32());
		m_baseTextureIndex = static_cast<int>(iff->read_int32());

		// construct the subscript variable ids from the given name.
		// assume it must be a VT_int

		char name[1024];

		//-- handle variable subscript zero
		iff->read_string(name, sizeof(name)-1);

		m_entryCountSubscript0 = static_cast<int>(iff->read_int32());
		DEBUG_FATAL(m_entryCountSubscript0 < 1, ("bad m_entryCountSubscript0 %d", m_entryCountSubscript0));

		m_subscript0VariableId = trTemplate.submitRangedIntVariable(name, 0, m_entryCountSubscript0);

		//-- handle variable subscript one
		iff->read_string(name, sizeof(name)-1);

		m_entryCountSubscript1 = static_cast<int>(iff->read_int32());
		DEBUG_FATAL(m_entryCountSubscript1 < 1, ("bad m_entryCountSubscript1 %d", m_entryCountSubscript1));

		m_subscript1VariableId = trTemplate.submitRangedIntVariable(name, 0, m_entryCountSubscript1);
		
	iff->exitChunk(TAG_SST2);
}

// ----------------------------------------------------------------------

void SetShaderTexture2dOperation::execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const
{
	//-- get the shader
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_shaderIndex, static_cast<int>(shaders.size()));

	StaticShader *const shader = safe_cast<StaticShader*>(shaders[static_cast<size_t>(m_shaderIndex)]);
	NOT_NULL(shader);

	//-- get value for texture subscript 0
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_subscript0VariableId, static_cast<int>(intValues.size()));
	int subscript0Value = intValues[static_cast<size_t>(m_subscript0VariableId)];

#if CLAMP_VARIABLES
	//-- clamp the range to the valid set of values
	subscript0Value = clamp(0, subscript0Value, m_entryCountSubscript0 - 1);
#endif

	//-- get value for texture subscript 1
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_subscript1VariableId, static_cast<int>(intValues.size()));
	int subscript1Value = intValues[static_cast<size_t>(m_subscript1VariableId)];

#if CLAMP_VARIABLES
	//-- clamp the range to the valid set of values
	subscript1Value = clamp(0, subscript1Value, m_entryCountSubscript1 - 1);
#endif

	const int textureIndex = m_baseTextureIndex + subscript0Value * m_entryCountSubscript1 + subscript1Value;

	//-- get the texture
	const Texture *const texture = trTemplate.fetchTexture(textureIndex);
	NOT_NULL(texture);

	//-- set the texture into the shader
	shader->setTexture(m_textureTag, *texture);

	//-- release the texture
	texture->release();
}

// ======================================================================
// class SetShaderTextureFactorOperation
// ======================================================================

SetShaderTextureFactorOperation::SetShaderTextureFactorOperation() :
	PrepareOperation(),
	m_shaderIndex(-1),
	m_textureFactorTag(TAG(N,O,N,E)),
	m_textureFactorVariableId(-1)
{
}

// ----------------------------------------------------------------------

SetShaderTextureFactorOperation::~SetShaderTextureFactorOperation()
{
}

// ----------------------------------------------------------------------

void SetShaderTextureFactorOperation::load_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate)
{
	NOT_NULL(iff);

	iff->enterChunk(TAG_STF);
	
		m_shaderIndex      = iff->read_int32();
		m_textureFactorTag = static_cast<Tag>(iff->read_uint32());
		
		// construct the texture variable id from the given name.
		// assume it must be a VT_int
		char name[MAX_PATH];
		iff->read_string(name, MAX_PATH-1);

		// @todo: this is wrong, as it does not let us assign the max value
		m_textureFactorVariableId = trTemplate.submitRangedIntVariable(name, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

	iff->exitChunk(TAG_STF);
}

// ----------------------------------------------------------------------

void SetShaderTextureFactorOperation::execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const
{
	UNREF(trTemplate);

	//-- get the texture factor value
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_textureFactorVariableId, static_cast<int>(intValues.size()));
	int textureFactor = intValues[static_cast<size_t>(m_textureFactorVariableId)];
	
	//-- get the shader
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_shaderIndex, static_cast<int>(shaders.size()));

	StaticShader *const shader = safe_cast<StaticShader*>(shaders[static_cast<size_t>(m_shaderIndex)]);
	NOT_NULL(shader);

	shader->setTextureFactor(m_textureFactorTag, static_cast<uint32>(textureFactor));
}

// ======================================================================
// class SetShaderTextureFactorAlphaOperation
// ======================================================================

SetShaderTextureFactorAlphaOperation::SetShaderTextureFactorAlphaOperation() :
	PrepareOperation(),
	m_shaderIndex(-1),
	m_textureFactorTag(TAG_NONE),
	m_alphaVariableId(-1),
	m_constantColor()
{
}

// ----------------------------------------------------------------------

SetShaderTextureFactorAlphaOperation::~SetShaderTextureFactorAlphaOperation()
{
}

// ----------------------------------------------------------------------

void SetShaderTextureFactorAlphaOperation::load_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate)
{
	NOT_NULL(iff);

	iff->enterChunk(TAG_STFA);

		m_shaderIndex      = iff->read_int32();
		m_textureFactorTag = static_cast<Tag>(iff->read_uint32());
	
		m_constantColor.setR(iff->read_uint8());
		m_constantColor.setG(iff->read_uint8());
		m_constantColor.setB(iff->read_uint8());

		// construct the texture variable id from the given name.
		// assume it must be a VT_int
		char name[MAX_PATH];
		iff->read_string(name, MAX_PATH-1);

		m_alphaVariableId = trTemplate.submitRangedIntVariable(name, 0, 256);

	iff->exitChunk(TAG_STFA);
}

// ----------------------------------------------------------------------

void SetShaderTextureFactorAlphaOperation::execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const
{
	UNREF(trTemplate);

	//-- get the texture factor value
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_alphaVariableId, static_cast<int>(intValues.size()));
	int alpha = intValues[static_cast<size_t>(m_alphaVariableId)];

	//-- create PackedArgb color
	PackedArgb newTextureFactor(m_constantColor);
	newTextureFactor.setA(static_cast<uint8>(alpha));

	//-- get the shader
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_shaderIndex, static_cast<int>(shaders.size()));

	StaticShader *const shader = safe_cast<StaticShader*>(shaders[static_cast<size_t>(m_shaderIndex)]);
	NOT_NULL(shader);

	shader->setTextureFactor(m_textureFactorTag, newTextureFactor.getArgb());
}

// ======================================================================
// class SetShaderTextureFactorFromPaletteOperation
// ======================================================================

SetShaderTextureFactorFromPaletteOperation::SetShaderTextureFactorFromPaletteOperation() :
	PrepareOperation(),
	m_shaderIndex(-1),
	m_textureFactorNameTag(TAG_NONE),
	m_palette(0),
	m_variableIndex(-1)
{
}

// ----------------------------------------------------------------------

SetShaderTextureFactorFromPaletteOperation::~SetShaderTextureFactorFromPaletteOperation()
{
	if (m_palette)
	{
		m_palette->release();
		m_palette = 0;
	}
}

// ----------------------------------------------------------------------

void SetShaderTextureFactorFromPaletteOperation::load_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate)
{
	NOT_NULL(iff);

	iff->enterChunk(TAG_STFP);

		m_shaderIndex          = static_cast<int>(iff->read_int16());
		m_textureFactorNameTag = static_cast<Tag>(iff->read_uint32());

		//-- Load the palette.
		char palettePath[4 * MAX_PATH];

		iff->read_string(palettePath, sizeof(palettePath) - 1);
		m_palette = PaletteArgbList::fetch(TemporaryCrcString(palettePath, true));

		//-- Load the customization variable name and private status.
		char variableName[MAX_PATH];

		iff->read_string(variableName, sizeof(variableName) - 1);
		const bool variableIsPrivate = (iff->read_int8() != 0);

		//-- Indicate the need for this customization variable.
		// @todo fix this.
		m_variableIndex = trTemplate.submitPaletteColorVariable(variableName, palettePath, variableIsPrivate);

	iff->exitChunk(TAG_STFP);
}

// ----------------------------------------------------------------------

void SetShaderTextureFactorFromPaletteOperation::execute(const BlueprintTextureRendererTemplate &trTemplate, const IntVector &intValues, ShaderVector &shaders) const
{
	UNREF(trTemplate);

	//-- Get the palette entry index.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_variableIndex, static_cast<int>(intValues.size()));
	int paletteEntryIndex = intValues[static_cast<IntVector::size_type>(m_variableIndex)];

	//-- Get the color from the palette.
	NOT_NULL(m_palette);

	int const paletteEntryCount = m_palette->getEntryCount();
	if ((paletteEntryIndex < 0) || (paletteEntryIndex >= paletteEntryCount))
	{
		DEBUG_WARNING(true, ("palette index [%d] for variable [%s] is out of valid [0..%d], setting to [%d].", paletteEntryIndex, trTemplate.getCustomizationVariableName(m_variableIndex).c_str(), paletteEntryCount-1, paletteEntryCount-1));
		paletteEntryIndex = paletteEntryCount - 1;
	}

	bool error = false;
	const PackedArgb color = m_palette->getEntry(paletteEntryIndex, error);
	WARNING(error, ("BlueprintTextureRendererTemplate execute error"));

	//-- Get the shader.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_shaderIndex, static_cast<int>(shaders.size()));

	StaticShader *const shader = safe_cast<StaticShader*>(shaders[static_cast<size_t>(m_shaderIndex)]);
	NOT_NULL(shader);

	//-- Set the texture factor.
	shader->setTextureFactor(m_textureFactorNameTag, color.getArgb());
}

// ======================================================================
// class PrepareOperation
// ======================================================================

PrepareOperation *PrepareOperation::create_0001(Iff *iff, BlueprintTextureRendererTemplate &trTemplate)
{
	NOT_NULL(iff);

	switch (iff->getCurrentName())
	{
		case TAG_NOP:
			{
				NoOperation *const operation = new NoOperation;
				operation->load_0001(iff);
				return operation;
			}
		case TAG_SSTC:
			{
				SetShaderConstantTextureOperation *const operation = new SetShaderConstantTextureOperation();
				operation->load_0001(iff);
				return operation;
			}
		case TAG_SST1:
			{
				SetShaderTexture1dOperation *const operation = new SetShaderTexture1dOperation();
				operation->load_0001(iff, trTemplate);
				return operation;
			}
		case TAG_SST2:
			{
				SetShaderTexture2dOperation *const operation = new SetShaderTexture2dOperation();
				operation->load_0001(iff, trTemplate);
				return operation;
			}
		case TAG_STF:
			{
				SetShaderTextureFactorOperation *const operation = new SetShaderTextureFactorOperation();
				operation->load_0001(iff, trTemplate);
				return operation;
			}
		case TAG_STFA:
			{
				SetShaderTextureFactorAlphaOperation *const operation = new SetShaderTextureFactorAlphaOperation();
				operation->load_0001(iff, trTemplate);
				return operation;
			}
		case TAG_STFP:
			{
				SetShaderTextureFactorFromPaletteOperation *const operation = new SetShaderTextureFactorFromPaletteOperation();
				operation->load_0001(iff, trTemplate);
				return operation;
			}
		default:
			{
				char name[5];
				ConvertTagToString(iff->getCurrentName(), name);
				FATAL(true, ("unsupported operation type [%s]", name));
				return 0; //lint !e527 // unreachable // right, for MSVC
			}
	}
}

// ======================================================================
// class BlueprintTextureRendererTemplate
// ======================================================================

void BlueprintTextureRendererTemplate::install()
{
	DEBUG_FATAL(ms_installed, ("BlueprintTextureRendererTemplate already installed"));

	ExitChain::add(remove, "BlueprintTextureRendererTemplate");
	ms_camera = new BlueprintTextureRendererCamera();

	TextureRendererList::registerTemplateLoader(TAG_BTRT, create);

	ms_installed = true;
}

// ----------------------------------------------------------------------

void BlueprintTextureRendererTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("BlueprintTextureRendererTemplate not installed"));

	delete ms_camera;
	ms_camera = 0;

	ms_installed = false;
}

// ======================================================================
// BlueprintTextureRendererTemplate: public member functions
// ======================================================================

int BlueprintTextureRendererTemplate::getCustomizationVariableCount() const
{
	return static_cast<int>(m_variableFactories->size());
}

// ----------------------------------------------------------------------

const std::string &BlueprintTextureRendererTemplate::getCustomizationVariableName(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getCustomizationVariableCount());
	return (*m_variableFactories)[static_cast<size_t>(index)]->getName();
}

// ----------------------------------------------------------------------

CustomizationVariable *BlueprintTextureRendererTemplate::createCustomizationVariable(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getCustomizationVariableCount());
	return (*m_variableFactories)[static_cast<size_t>(index)]->createCustomizationVariable();
}

// ----------------------------------------------------------------------

bool BlueprintTextureRendererTemplate::isCustomizationVariablePrivate(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getCustomizationVariableCount());
	return (*m_variableFactories)[static_cast<size_t>(index)]->isPrivate();
}

// ----------------------------------------------------------------------
/**
 * Add a new ranged int variable referenced by this 
 * BlueprintTextureRendererTemplate instance.
 *
 * This function will find and return the variable offset for the given
 * variable name if the variable already has been submitted.  If the variable
 * has never been submitted, the function will create a new variable.
 *
 * This function will DEBUG_FATAL if the given variable name attempts to
 * be registered as multiple differing data types (e.g. basic ranged int
 * and palette color variable).
 *
 * @param variableName       the name of the customization variable.
 * @param minValueInclusive  the minimum acceptable value for the ranged int.
 * @param maxValueExclusive  one above the maximum acceptable value for the
 *                           ranged int.
 *
 * @return  the index of the variable added or found.  The index is the
 *          position within the VariableFactory list, which also is the
 *          position within the BlueprintTextureRenderer's vector of int
 *          values passed into the template.
 */

int BlueprintTextureRendererTemplate::submitRangedIntVariable(const std::string &variableName, int minValueInclusive, int maxValueExclusive, bool variableIsPrivate)
{
	//-- check if any variable matches this name
	VariableFactory *variableFactory = 0;
	int              factoryIndex    = -1;

	if (findVariableFactory(variableName, variableIsPrivate, variableFactory, factoryIndex))
	{
		// A VariableFactory already exists with this name.  Ensure it is the right type.
		DEBUG_FATAL(!dynamic_cast<BasicRangedIntVariableFactory*>(variableFactory), ("template tried to submit customization variable [%s] with different types", variableName.c_str()));
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, factoryIndex, getCustomizationVariableCount());

		return factoryIndex;
	}
	else
	{
		// The VariableFactory doesn't exist.  Create it.
		m_variableFactories->push_back(new BasicRangedIntVariableFactory(variableName, variableIsPrivate, minValueInclusive, maxValueExclusive));
		return static_cast<int>(m_variableFactories->size()) - 1;
	}
}

// ----------------------------------------------------------------------

int BlueprintTextureRendererTemplate::submitPaletteColorVariable(const std::string &variableName, const char *palettePathName, bool variableIsPrivate)
{
	//-- check if any variable matches this name
	VariableFactory *variableFactory = 0;
	int              factoryIndex    = -1;

	if (findVariableFactory(variableName, variableIsPrivate, variableFactory, factoryIndex))
	{
		// A VariableFactory already exists with this name.  Ensure it is the right type.
		DEBUG_FATAL(!dynamic_cast<PaletteColorVariableFactory*>(variableFactory), ("template tried to submit customization variable [%s] with different types", variableName.c_str()));
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, factoryIndex, getCustomizationVariableCount());

		return factoryIndex;
	}
	else
	{
		// The VariableFactory doesn't exist.  Create it.

		//-- fetch the palette
		const PaletteArgb *const palette = PaletteArgbList::fetch(TemporaryCrcString(palettePathName, true));
		NOT_NULL(palette);

		//-- create the variable factory
		m_variableFactories->push_back(new PaletteColorVariableFactory(variableName, variableIsPrivate, palette));

		//-- release local reference
		palette->release();

		return static_cast<int>(m_variableFactories->size()) - 1;
	}
}

// ----------------------------------------------------------------------
/**
 * Retrieve a BlueprintSharedTextureRenderer instance baked for the
 * given customization variable values.
 *
 * This function will return a single BlueprintSharedTextureRenderer,
 * baked only once, for a given set of customization values, regardless
 * of the number of BlueprintTextureRenderer instances making use of it.
 * This is the mechanism by which we reduce the cost of having multiple
 * customized textures with the same value.
 *
 * NOTE: the caller should not use the release() call on the
 * returned TextureRenderer.  Instead, use releaseSharedTextureRendere().
 * This is because the implementation of the list requires this 
 * template instance to know when the shared texture no longer is used.
 * @todo consider fixing this so the caller cannot misuse it.
 *
 * @param intValues  the values for the customization variables for which
 *                   a BlueprintSharedTextureRenderer is requested.
 *
 * @return  a blueprintSharedTextureRenderer with a Texture baked to the
 *          given customization attributes.
 */

const BlueprintSharedTextureRenderer *BlueprintTextureRendererTemplate::fetchSharedTextureRenderer(const IntVector &intValues) const
{
	//-- check if we already have a shared texture for the given customization values
	BlueprintSharedTextureRenderer *tr = 0;

	SharedTextureRendererMap::iterator lowerBoundResult = m_sharedTextureRendererMap->lower_bound(&intValues);
	if ((lowerBoundResult != m_sharedTextureRendererMap->end()) && !m_sharedTextureRendererMap->key_comp()(&intValues, lowerBoundResult->first))
	{
		//-- we've already created a shared texture for this set of customization values
		tr = lowerBoundResult->second;
	}
	else
	{
		//-- we need to create a new shared texture for the given customization values
		tr = new BlueprintSharedTextureRenderer(*this, intValues);

		//-- add to map
		IGNORE_RETURN(m_sharedTextureRendererMap->insert(lowerBoundResult, SharedTextureRendererMap::value_type(&tr->getIntValues(), tr)));
	}

	//-- fetch reference for caller
	NOT_NULL(tr);
	tr->fetch();

	return tr;
}

// ----------------------------------------------------------------------
/**
 * Notify the template that a shared TextureRenderer managed by it is
 * getting destroyed.
 *
 * This function should be called solely by the
 * BlueprintSharedTextureRenderer destructor.
 *
 * @param sharedTextureRenderer  the shared TextureRenderer instance
 *                               getting destroyed.
 */

void BlueprintTextureRendererTemplate::notifySharedTextureDestruction(const BlueprintSharedTextureRenderer *sharedTextureRenderer) const
{
	NOT_NULL(sharedTextureRenderer);

	//-- search all map entries for the specified shared texture renderer
	const SharedTextureRendererMap::iterator endIt = m_sharedTextureRendererMap->end();
	for (SharedTextureRendererMap::iterator it = m_sharedTextureRendererMap->begin(); it != endIt; ++it)
	{
		BlueprintSharedTextureRenderer *const tr = it->second;

		if (tr == sharedTextureRenderer)
		{
			m_sharedTextureRendererMap->erase(it);
			return;
		}
	}
}

// ======================================================================
// BlueprintTextureRendererTemplate: private static member functions
// ======================================================================

TextureRendererTemplate *BlueprintTextureRendererTemplate::create(Iff *iff, const char *name)
{
	return new BlueprintTextureRendererTemplate(iff, name);
}

// ======================================================================

BlueprintTextureRendererTemplate::BlueprintTextureRendererTemplate(Iff *iff, const char *name)
:	TextureRendererTemplate(name),
	m_cameraSetup(0),
	m_prepareCommands(new PrepareCommandContainer()),
	m_renderCommands(new RenderCommandContainer()),
	m_shaderTemplates(new ShaderTemplateContainer()),
	m_textureNames(new StringContainer()),
	m_textures(new TextureContainer()),
	m_vertexBuffers(new VertexBufferContainer()),
	m_indexBuffers(new IndexBufferContainer()),
	m_variableFactories(new VariableFactoryVector()),
	m_sharedTextureRendererMap(new SharedTextureRendererMap())
{
	DEBUG_FATAL(!ms_installed, ("BlueprintTextureRendererTemplate not installed"));

	iff->enterForm(TAG_BTRT);
	{
		switch (iff->getCurrentName())
		{
			case TAG_0001:
				load_0001(iff);
				break;
			case TAG_0002:
				load_0002(iff);
				break;
			default:
				{
					char version[5];
					ConvertTagToString(iff->getCurrentName(), version);
					FATAL(true, ("render blueprint template version [%s] not supported", version));
				}
		}
	}
	iff->exitForm(TAG_BTRT);
}

// ----------------------------------------------------------------------

BlueprintTextureRendererTemplate::~BlueprintTextureRendererTemplate()
{
	//-- release all shared texture renderers
	{
		const SharedTextureRendererMap::iterator endIt = m_sharedTextureRendererMap->end();
		for (SharedTextureRendererMap::iterator it = m_sharedTextureRendererMap->begin(); it != endIt; ++it)
		{
			//-- release our reference to the shared texture renderer.  This should be the final reference
			//   to any of these.
			NOT_NULL(it->second);
			it->second->release();
		}
	}
	delete m_sharedTextureRendererMap;

	std::for_each(m_variableFactories->begin(), m_variableFactories->end(), PointerDeleter());
	delete m_variableFactories;

	std::for_each(m_indexBuffers->begin(), m_indexBuffers->end(), PointerDeleter());
	delete m_indexBuffers;

	std::for_each(m_vertexBuffers->begin(), m_vertexBuffers->end(), PointerDeleter());
	delete m_vertexBuffers;

	{
		// can't use for_each without creating another functor doing the check-for-null object
		TextureContainer::iterator itEnd = m_textures->end();
		for (TextureContainer::iterator it = m_textures->begin(); it != itEnd; ++it)
		{
			const Texture *const texture = *it;
			if (texture)
				texture->release();
		}
		delete m_textures;
	}

	delete m_textureNames;

	IGNORE_RETURN( std::for_each(m_shaderTemplates->begin(), m_shaderTemplates->end(), VoidMemberFunction(&ShaderTemplate::release)) );
	delete m_shaderTemplates;

	IGNORE_RETURN(std::for_each(m_renderCommands->begin(), m_renderCommands->end(), PointerDeleter()));
	delete m_renderCommands;

	IGNORE_RETURN(std::for_each(m_prepareCommands->begin(), m_prepareCommands->end(), PointerDeleter()));
	delete m_prepareCommands;

	delete m_cameraSetup;
}

// ----------------------------------------------------------------------

void BlueprintTextureRendererTemplate::load_0001(Iff *iff)
{
	NOT_NULL(iff);

	int shaderTemplateCount = 0;
	int textureCount        = 0; 
	int vertexBufferCount   = 0;
	int indexBufferCount    = 0;
	int renderCommandCount  = 0;
	int prepareCommandCount = 0;

	iff->enterForm(TAG_0001);
	{
		//-- load general info
		iff->enterChunk(TAG_INFO);
		{
			//-- skip ownership info
			IGNORE_RETURN(iff->read_uint32());
		}
		iff->exitChunk(TAG_INFO);

		//-- load destination texture info
		iff->enterChunk(TAG_DEST);
		{
			const int destinationPreferredWidth  = iff->read_int32();
			setDestinationPreferredWidth(destinationPreferredWidth);

			const int destinationPreferredHeight = iff->read_int32();
			setDestinationPreferredHeight(destinationPreferredHeight);

			const size_t runtimeFormatCount = static_cast<size_t>(iff->read_uint32());

			std::vector<TextureFormat> runtimeFormats;
			runtimeFormats.reserve(runtimeFormatCount);

			for (size_t i = 0; i < runtimeFormatCount; ++i)
			{
				int const index = static_cast<int>(iff->read_uint32());
				DEBUG_FATAL(index < 0 || index >= static_cast<int>(TF_Count), ("Invalid old texture format in data file"));
				TextureFormat const newFormat = textureFormatConversionTable[index];
				DEBUG_FATAL(newFormat == TF_Count, ("Old texture format in data file is now unsupported"));
				runtimeFormats.push_back(newFormat);
			}

			setDestinationRuntimeFormats(&runtimeFormats[0], static_cast<int>(runtimeFormatCount));
		}
		iff->exitChunk(TAG_DEST);

		//-- load shader templates
		iff->enterForm(TAG(S,H,T,M));
		{
			// get # shader templates
			iff->enterChunk(TAG_INFO);
			{
				shaderTemplateCount = iff->read_int32();
			}
			iff->exitChunk(TAG_INFO);

			// load shader templates
			m_shaderTemplates->reserve(static_cast<size_t>(shaderTemplateCount));
			for (int i = 0; i < shaderTemplateCount; ++i)
			{
				bool error = false;
				const ShaderTemplate *const shaderTemplate = ShaderTemplateList::fetch(*iff, error);
				m_shaderTemplates->push_back(shaderTemplate);
			}
		}
		iff->exitForm(TAG(S,H,T,M));

		//-- load texture names
		iff->enterForm(TAG(T,X,T,S));
		{
			iff->enterChunk(TAG_INFO);
			{
				// get # textures
				textureCount = iff->read_int32();
	
				// load texture names
				m_textureNames->reserve(static_cast<size_t>(textureCount));
				m_textures->reserve(static_cast<size_t>(textureCount));

				for (int i = 0; i < textureCount; ++i)
				{
					// get the texture name, but...
					char textureName[MAX_PATH];
					iff->read_string(textureName, sizeof(textureName)-1);
					m_textureNames->push_back(std::string(textureName));

					// don't fetch the texture until we need it.
					m_textures->push_back(0);
				}
			}
			iff->exitChunk(TAG_INFO);
		}
		iff->exitForm(TAG(T,X,T,S));

		//-- load vertex buffers
		iff->enterForm(TAG3(V,B,S));
		{
			// get # vertex buffers
			iff->enterChunk(TAG_INFO);
			{
				vertexBufferCount = iff->read_int32();
			}
			iff->exitChunk(TAG_INFO);

			// load vbs
			m_vertexBuffers->reserve(static_cast<size_t>(vertexBufferCount));
			for (int i = 0; i < vertexBufferCount; ++i)
			{
				StaticVertexBuffer *const vb = new StaticVertexBuffer(*iff);
				m_vertexBuffers->push_back(vb);
			}
		}
		iff->exitForm(TAG3(V,B,S));

		//-- load index buffers
		iff->enterForm(TAG3(I,B,S));
		{
			// get # index buffers
			iff->enterChunk(TAG_INFO);
			{
				indexBufferCount = iff->read_int32();
			}
			iff->exitChunk(TAG_INFO);

			// load index buffer data
			m_indexBuffers->reserve(static_cast<size_t>(indexBufferCount));
			for (int i = 0; i < indexBufferCount; ++i)
			{
				iff->enterChunk(TAG(I,D,A,T));
				{
					// get # indices in this buffer
					const int indexCount = iff->read_int32();

					// create the index buffer
					StaticIndexBuffer *const ib = new StaticIndexBuffer(indexCount);
					m_indexBuffers->push_back(ib);

					ib->lock();
					{
						Index *index = ib->begin();
						iff->read_uint16(indexCount, index);
					}
					ib->unlock();

				}
				iff->exitChunk(TAG(I,D,A,T));
			}
		}
		iff->exitForm(TAG3(I,B,S));

		//-- load camera setup
		iff->enterForm(TAG3(C,A,M));
			m_cameraSetup = CameraSetup::create_0001(iff);
		iff->exitForm(TAG3(C,A,M));

		//-- load render commands
		iff->enterForm(TAG(R,C,M,S));
		{
			// get # render commands
			iff->enterChunk(TAG_INFO);
			{
				renderCommandCount = iff->read_int32();
			}
			iff->exitChunk(TAG_INFO);

			// load render commands
			m_renderCommands->reserve(static_cast<size_t>(renderCommandCount));
			for (int i = 0; i < renderCommandCount; ++i)
				m_renderCommands->push_back(RenderCommand::createRenderCommand_0001(iff));
		}
		iff->exitForm(TAG(R,C,M,S));

		//-- load prepare commands
		iff->enterForm(TAG_PCMS);
		{
			iff->enterChunk(TAG_INFO);
			{
				prepareCommandCount = iff->read_int32();
			}
			iff->exitChunk(TAG_INFO);

			m_prepareCommands->reserve(static_cast<size_t>(prepareCommandCount));
			for (int i = 0; i < prepareCommandCount; ++i)
			{
				PrepareCommand *const prepareCommand = new PrepareCommand;
				prepareCommand->load_0001(iff, *this);
				m_prepareCommands->push_back(prepareCommand);
			} //lint !e429 // prepareCommand neither freed nor returned // right, it's in a list
		}
		iff->exitForm(TAG_PCMS);
	}
	iff->exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

void BlueprintTextureRendererTemplate::load_0002(Iff *iff)
{
	NOT_NULL(iff);

	int shaderTemplateCount = 0;
	int textureCount        = 0; 
	int vertexBufferCount   = 0;
	int indexBufferCount    = 0;
	int renderCommandCount  = 0;
	int prepareCommandCount = 0;

	iff->enterForm(TAG_0001);
	{
		//-- load general info
		iff->enterChunk(TAG_INFO);
		{
			//-- skip ownership info
			IGNORE_RETURN(iff->read_uint32());
		}
		iff->exitChunk(TAG_INFO);

		//-- load destination texture info
		iff->enterChunk(TAG_DEST);
		{
			const int destinationPreferredWidth  = iff->read_int32();
			setDestinationPreferredWidth(destinationPreferredWidth);

			const int destinationPreferredHeight = iff->read_int32();
			setDestinationPreferredHeight(destinationPreferredHeight);

			const size_t runtimeFormatCount = static_cast<size_t>(iff->read_uint32());

			std::vector<TextureFormat> runtimeFormats;
			runtimeFormats.reserve(runtimeFormatCount);

			for (size_t i = 0; i < runtimeFormatCount; ++i)
				runtimeFormats.push_back(static_cast<TextureFormat>(iff->read_uint32()));

			setDestinationRuntimeFormats(&runtimeFormats[0], static_cast<int>(runtimeFormatCount));
		}
		iff->exitChunk(TAG_DEST);

		//-- load shader templates
		iff->enterForm(TAG(S,H,T,M));
		{
			// get # shader templates
			iff->enterChunk(TAG_INFO);
			{
				shaderTemplateCount = iff->read_int32();
			}
			iff->exitChunk(TAG_INFO);

			// load shader templates
			m_shaderTemplates->reserve(static_cast<size_t>(shaderTemplateCount));
			for (int i = 0; i < shaderTemplateCount; ++i)
			{
				bool error = false;
				const ShaderTemplate *const shaderTemplate = ShaderTemplateList::fetch(*iff, error);
				m_shaderTemplates->push_back(shaderTemplate);
			}
		}
		iff->exitForm(TAG(S,H,T,M));

		//-- load texture names
		iff->enterForm(TAG(T,X,T,S));
		{
			iff->enterChunk(TAG_INFO);
			{
				// get # textures
				textureCount = iff->read_int32();
	
				// load texture names
				m_textureNames->reserve(static_cast<size_t>(textureCount));
				m_textures->reserve(static_cast<size_t>(textureCount));

				for (int i = 0; i < textureCount; ++i)
				{
					// get the texture name, but...
					char textureName[MAX_PATH];
					iff->read_string(textureName, sizeof(textureName)-1);
					m_textureNames->push_back(std::string(textureName));

					// don't fetch the texture until we need it.
					m_textures->push_back(0);
				}
			}
			iff->exitChunk(TAG_INFO);
		}
		iff->exitForm(TAG(T,X,T,S));

		//-- load vertex buffers
		iff->enterForm(TAG3(V,B,S));
		{
			// get # vertex buffers
			iff->enterChunk(TAG_INFO);
			{
				vertexBufferCount = iff->read_int32();
			}
			iff->exitChunk(TAG_INFO);

			// load vbs
			m_vertexBuffers->reserve(static_cast<size_t>(vertexBufferCount));
			for (int i = 0; i < vertexBufferCount; ++i)
			{
				StaticVertexBuffer *const vb = new StaticVertexBuffer(*iff);
				m_vertexBuffers->push_back(vb);
			}
		}
		iff->exitForm(TAG3(V,B,S));

		//-- load index buffers
		iff->enterForm(TAG3(I,B,S));
		{
			// get # index buffers
			iff->enterChunk(TAG_INFO);
			{
				indexBufferCount = iff->read_int32();
			}
			iff->exitChunk(TAG_INFO);

			// load index buffer data
			m_indexBuffers->reserve(static_cast<size_t>(indexBufferCount));
			for (int i = 0; i < indexBufferCount; ++i)
			{
				iff->enterChunk(TAG(I,D,A,T));
				{
					// get # indices in this buffer
					const int indexCount = iff->read_int32();

					// create the index buffer
					StaticIndexBuffer *const ib = new StaticIndexBuffer(indexCount);
					m_indexBuffers->push_back(ib);

					ib->lock();
					{
						Index *index = ib->begin();
						iff->read_uint16(indexCount, index);
					}
					ib->unlock();

				}
				iff->exitChunk(TAG(I,D,A,T));
			}
		}
		iff->exitForm(TAG3(I,B,S));

		//-- load camera setup
		iff->enterForm(TAG3(C,A,M));
			m_cameraSetup = CameraSetup::create_0001(iff);
		iff->exitForm(TAG3(C,A,M));

		//-- load render commands
		iff->enterForm(TAG(R,C,M,S));
		{
			// get # render commands
			iff->enterChunk(TAG_INFO);
			{
				renderCommandCount = iff->read_int32();
			}
			iff->exitChunk(TAG_INFO);

			// load render commands
			m_renderCommands->reserve(static_cast<size_t>(renderCommandCount));
			for (int i = 0; i < renderCommandCount; ++i)
				m_renderCommands->push_back(RenderCommand::createRenderCommand_0001(iff));
		}
		iff->exitForm(TAG(R,C,M,S));

		//-- load prepare commands
		iff->enterForm(TAG_PCMS);
		{
			iff->enterChunk(TAG_INFO);
			{
				prepareCommandCount = iff->read_int32();
			}
			iff->exitChunk(TAG_INFO);

			m_prepareCommands->reserve(static_cast<size_t>(prepareCommandCount));
			for (int i = 0; i < prepareCommandCount; ++i)
			{
				PrepareCommand *const prepareCommand = new PrepareCommand;
				prepareCommand->load_0001(iff, *this);
				m_prepareCommands->push_back(prepareCommand);
			} //lint !e429 // prepareCommand neither freed nor returned // right, it's in a list
		}
		iff->exitForm(TAG_PCMS);
	}
	iff->exitForm(TAG_0001);
}

// ----------------------------------------------------------------------
/**
 * Check if a VariableFactory with a given name has been registered, and
 * if so, return the VariableFactory and the index for it.
 *
 * @param variableName     name of the variable for which to check.
 * @param variableFactory  returns the VariableFactory instance if a match
 *                         is found.
 * @param factoryIndex     returns the index of the VariableFactory instance
 *                         if a match is found.  This index corresponds to
 *                         the BlueprintTextureRenderer int value index
 *                         used to retrieve values of variables.
 *
 * @return  true if a VariableFactory instance has been registered with the
 *          given name; false otherwise.
 */

bool BlueprintTextureRendererTemplate::findVariableFactory(const std::string &variableName, bool variableIsPrivate, VariableFactory *&variableFactory, int &factoryIndex)
{
	//-- do a linear search for a match on the variable name
	const int factoryCount = static_cast<int>(m_variableFactories->size());
	for (factoryIndex = 0; factoryIndex < factoryCount; ++factoryIndex)
	{
		VariableFactory *const vf = (*m_variableFactories)[static_cast<size_t>(factoryIndex)];
		NOT_NULL(vf);

		// @todo eliminate variance due to text case here --- normally we're using CrcLowerStrings
		//       but we're not here.
		if ((vf->getName() == variableName) && (vf->isPrivate() == variableIsPrivate)) //lint !e731 // Info -- Boolean argument to equal/not equal // intentional.
		{
			// found it
			variableFactory = vf;
			return true;
		}
	}

	//-- didn't find it
	variableFactory = 0;
	factoryIndex    = -1;

	return false;
}

// ----------------------------------------------------------------------

TextureRenderer *BlueprintTextureRendererTemplate::createTextureRenderer() const
{
	TextureRenderer *const tr = new BlueprintTextureRenderer(*this);
	tr->fetch();

	return tr;
}

// ----------------------------------------------------------------------

void BlueprintTextureRendererTemplate::addCustomizationVariables(CustomizationData &customizationData) const
{
	//-- create a CustomizationVariable for each influencing variable if and only if the variable doesn't already exist.
	const VariableFactoryVector::const_iterator endIt = m_variableFactories->end();
	for (VariableFactoryVector::const_iterator it = m_variableFactories->begin(); it != endIt; ++it)
	{
		// get the variable factory name
		VariableFactory *const vf = *it;
		NOT_NULL(vf);

		const std::string &variableName = vf->getName();

		// check if it already exists in the customizationData
		const CustomizationVariable *const existingCustomizationVariable = customizationData.findConstVariable(variableName);
		if (existingCustomizationVariable)
		{
			// @todo warn or FATAL if not the right type
			continue;
		}

		// create CustomizationVariable
		customizationData.addVariableTakeOwnership(variableName, vf->createCustomizationVariable());
	}
}

// ----------------------------------------------------------------------

void BlueprintTextureRendererTemplate::prepareShaders(const IntVector &intValues, ShaderVector &shaders) const
{
	const PrepareCommandContainer::const_iterator itEnd = m_prepareCommands->end();
	for (PrepareCommandContainer::const_iterator it = m_prepareCommands->begin(); it != itEnd; ++it)
		(*it)->execute(*this, intValues, shaders);
}

// ----------------------------------------------------------------------

bool BlueprintTextureRendererTemplate::render(Texture *destinationTexture, const IntVector &intValues, ShaderVector &shaders) const
{
	NOT_NULL(m_cameraSetup);
	NOT_NULL(destinationTexture);

	//-- set the camera
	// NOTE: below it looks like I'm doing all the work that the Camera::renderScene() was
	//       designed to do; this is not without reason.  I can receive a user-defined
	//       camera which would not have any idea how to render a set of our render commands.

	bool endFrameSucceeded = true;

	const int mipmapLevelCount = destinationTexture->getMipmapLevelCount();
	for (int mipmapLevel = 0; (mipmapLevel < mipmapLevelCount) && endFrameSucceeded; ++mipmapLevel)
	{
		// get camera from setup
		const Camera *const camera = m_cameraSetup->getCamera(*destinationTexture, intValues, mipmapLevel);

		//-- render with this camera
		Graphics::beginScene();
		Graphics::setRenderTarget(destinationTexture, CF_none, mipmapLevel);
		{
			// set the camera
			camera->beginScene();

				Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
				Graphics::setCullMode(GCM_counterClockwise);

				const RenderCommandContainer::const_iterator itEnd = m_renderCommands->end();
				for (RenderCommandContainer::const_iterator it = m_renderCommands->begin(); it != itEnd; ++it)
					(*it)->execute(*this, intValues, shaders);

			// release the camera
			camera->endScene();
		}
		Graphics::endScene();
		endFrameSucceeded = Graphics::copyRenderTargetToNonRenderTargetTexture();
		Graphics::setRenderTarget(NULL, CF_none, 0);

	}

	return endFrameSucceeded;
}

// ----------------------------------------------------------------------

const StaticVertexBuffer &BlueprintTextureRendererTemplate::getVertexBuffer(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_vertexBuffers->size()));
	return *NON_NULL((*m_vertexBuffers)[static_cast<size_t>(index)]);
}

// ----------------------------------------------------------------------

const StaticIndexBuffer &BlueprintTextureRendererTemplate::getIndexBuffer(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_indexBuffers->size()));
	return *NON_NULL((*m_indexBuffers)[static_cast<size_t>(index)]);
}

// ----------------------------------------------------------------------

int BlueprintTextureRendererTemplate::getTextureCount() const
{
	return static_cast<int>(m_textures->size());
}

// ----------------------------------------------------------------------

const Texture *BlueprintTextureRendererTemplate::fetchTexture(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_textures->size()));

	const size_t uIndex = static_cast<size_t>(index);
	const Texture *texture = (*m_textures)[uIndex];
	if (!texture)
	{
		// we need to fetch this one
		texture = TextureList::fetch((*m_textureNames)[uIndex].c_str());
		NOT_NULL(texture);

		// cache this.  the current ref count is for this cached copy.
		(*m_textures)[uIndex] = texture;
	}

	// bump up ref count for caller
	texture->fetch();
	return texture;
}

// ======================================================================
