// ======================================================================
//
// BlueprintWriter.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "BlueprintWriter.h"

#include "sharedFile/Iff.h"
#include "clientGraphics/SystemVertexBuffer.h"
#include "sharedFoundation/VoidBindSecond.h"
#include "sharedFoundation/VoidMemberFunction.h"

#include <algorithm>
#include <string>
#include <vector>

// ======================================================================
// lint supression
// ======================================================================

//e754: Info -- local structure member xxx not referenced)
//lint -esym(754, TriFanPrimitive::TriFanPrimitive)
//lint -esym(754, TriFanPrimitive::operator=)
//lint -esym(754, ClearFrameBufferRenderCommand::ClearFrameBufferRenderCommand)
//lint -esym(754, ClearFrameBufferRenderCommand::operator=)
//lint -esym(754, ShaderRenderCommand::ShaderRenderCommand)
//lint -esym(754, ShaderRenderCommand::operator=)
//lint -esym(754, SetConstantTextureOperation::SetConstantTextureOperation)
//lint -esym(754, SetConstantTextureOperation::operator=)
//lint -esym(754, Set1dTextureOperation::Set1dTextureOperation)
//lint -esym(754, Set1dTextureOperation::operator=)
//lint -esym(754, Set2dTextureOperation::Set2dTextureOperation)
//lint -esym(754, Set2dTextureOperation::operator=)
//lint -esym(754, SetTextureFactorAlphaOperation::SetTextureFactorAlphaOperation)
//lint -esym(754, SetTextureFactorAlphaOperation::operator=)
//lint -esym(754, ClearFrameBufferRenderCommand::write)
//lint -esym(754, ShaderRenderCommand::write)
//lint -esym(754, TriFanPrimitive::write)


// ======================================================================

namespace
{
	const Tag TAG_BTRT = TAG(B,T,R,T);
	const Tag TAG_CAM  = TAG3(C,A,M);
	const Tag TAG_CFBC = TAG(C,F,B,C);
	const Tag TAG_DEST = TAG(D,E,S,T);
	const Tag TAG_FOPS = TAG(F,O,P,S);
	const Tag TAG_IBS  = TAG3(I,B,S);
	const Tag TAG_PCAM = TAG(P,C,A,M);
	const Tag TAG_PCMD = TAG(P,C,M,D);
	const Tag TAG_PCMS = TAG(P,C,M,S);
	const Tag TAG_RCMS = TAG(R,C,M,S);
	const Tag TAG_RTFC = TAG(R,T,F,C);
	const Tag TAG_SHTM = TAG(S,H,T,M);
	const Tag TAG_SRSC = TAG(S,R,S,C);
	const Tag TAG_SST1 = TAG(S,S,T,1);
	const Tag TAG_SST2 = TAG(S,S,T,2);
	const Tag TAG_SSTC = TAG(S,S,T,C);
	const Tag TAG_STFA = TAG(S,T,F,A);
	const Tag TAG_STFP = TAG(S,T,F,P);
	const Tag TAG_TOPS = TAG(T,O,P,S);
	const Tag TAG_TXTS = TAG(T,X,T,S);
	const Tag TAG_VBS  = TAG3(V,B,S);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class PrepareOperation
	{
	public:

		virtual ~PrepareOperation() {}

		virtual void write(Iff *iff) const = 0;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	class SetConstantTextureOperation: public PrepareOperation
	{
	public:

		SetConstantTextureOperation(int shaderIndex, Tag textureTag, int textureIndex);

		virtual void write(Iff *iff) const;

	private:

		int  m_shaderIndex;
		Tag  m_textureTag;
		int  m_textureIndex;

	private:
		// disabled
		SetConstantTextureOperation();
		SetConstantTextureOperation(const SetConstantTextureOperation&);
		SetConstantTextureOperation &operator =(const SetConstantTextureOperation&);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Set1dTextureOperation: public PrepareOperation
	{
	public:

		Set1dTextureOperation(int shaderIndex, Tag textureTag, int baseTextureIndex, const std::string &indexVariableName, int entryCountSubscript0);
		virtual ~Set1dTextureOperation();

		virtual void write(Iff *iff) const;

	private:

		int          m_shaderIndex;
		Tag          m_textureTag;
		int          m_baseTextureIndex;

		std::string  m_indexVariableName;
		int          m_entryCountSubscript0;

	private:
		// disabled
		Set1dTextureOperation();
		Set1dTextureOperation(const Set1dTextureOperation&);
		Set1dTextureOperation &operator =(const Set1dTextureOperation&);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Set2dTextureOperation: public PrepareOperation
	{
	public:

		Set2dTextureOperation(int shaderIndex, Tag textureTag, int baseTextureIndex, const std::string &subscript0IndexVariableName, int entryCountSubscript0, const std::string &subscript1IndexVariableName, int entryCountSubscript1);
		virtual ~Set2dTextureOperation();

		virtual void write(Iff *iff) const;

	private:

		int          m_shaderIndex;
		Tag          m_textureTag;
		int          m_baseTextureIndex;

		std::string  m_subscript0IndexVariableName;
		int          m_entryCountSubscript0;

		std::string  m_subscript1IndexVariableName;
		int          m_entryCountSubscript1;

	private:
		// disabled
		Set2dTextureOperation();
		Set2dTextureOperation(const Set2dTextureOperation&);
		Set2dTextureOperation &operator =(const Set2dTextureOperation&);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	class SetTextureFactorAlphaOperation: public PrepareOperation
	{
	public:

		SetTextureFactorAlphaOperation(int shaderIndex, Tag textureFactorTag, uint8 red, uint8 green, uint8 blue, const std::string &textureFactorAlphaVariableName);
		virtual ~SetTextureFactorAlphaOperation();

		virtual void write(Iff *iff) const;

	private:

		int          m_shaderIndex;
		Tag          m_textureFactorTag;
		uint8        m_red;
		uint8        m_green;
		uint8        m_blue;
		std::string  m_textureFactorAlphaVariableName;

	private:
		// disabled
		SetTextureFactorAlphaOperation();
		SetTextureFactorAlphaOperation(const SetTextureFactorAlphaOperation&);
		SetTextureFactorAlphaOperation &operator =(const SetTextureFactorAlphaOperation&);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class SetTextureFactorFromPaletteOperation: public PrepareOperation
	{
	public:

		SetTextureFactorFromPaletteOperation(int shaderIndex, Tag textureFactorTag, const std::string &variableName, bool variableIsPrivate, const std::string &palettePathName);
		virtual ~SetTextureFactorFromPaletteOperation();

		virtual void write(Iff *iff) const;

	private:

		int          m_shaderIndex;
		Tag          m_textureFactorTag;
		std::string  m_variableName;
		bool         m_variableIsPrivate;
		std::string  m_palettePathName;

	private:
		// disabled
		SetTextureFactorFromPaletteOperation();
		SetTextureFactorFromPaletteOperation(const SetTextureFactorFromPaletteOperation&);
		SetTextureFactorFromPaletteOperation &operator =(const SetTextureFactorFromPaletteOperation&);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Primitive
	{
	public:

		virtual ~Primitive() {}

		virtual void write(Iff *iff) const = 0;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class TriFanPrimitive: public Primitive
	{
	public:

		explicit TriFanPrimitive(int vertexBufferIndex);
		virtual ~TriFanPrimitive();

		virtual void write(Iff *iff) const;

	private:

		int  m_vertexBufferIndex;

	private:
		// disabled
		TriFanPrimitive();
		TriFanPrimitive(const TriFanPrimitive&);
		TriFanPrimitive &operator =(const TriFanPrimitive&);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<boost::shared_ptr<PrepareOperation> >  PrepareOperationVector;

}

// ======================================================================
// full embedded class declarations
// ======================================================================

class BlueprintWriter::RenderCommand
{
public:

	virtual ~RenderCommand() {}

	virtual void write(Iff *iff) const = 0;

};

// ----------------------------------------------------------------------

class BlueprintWriter::ClearFrameBufferRenderCommand: public BlueprintWriter::RenderCommand
{
public:

	ClearFrameBufferRenderCommand(bool clearColor, uint32 colorValue, bool clearDepth, float depthValue, bool clearStencil, uint32 stencilValue);

	virtual void write(Iff *iff) const;

private:

	bool   m_clearColor;
	uint   m_colorValue;

	bool   m_clearDepth;
	float  m_depthValue;

	bool   m_clearStencil;
	uint   m_stencilValue;

private:
	// disabled
	ClearFrameBufferRenderCommand();
	ClearFrameBufferRenderCommand(const ClearFrameBufferRenderCommand&);
	ClearFrameBufferRenderCommand &operator =(const ClearFrameBufferRenderCommand&);
};

// ----------------------------------------------------------------------

class BlueprintWriter::ShaderRenderCommand: public BlueprintWriter::RenderCommand
{
public:

	explicit ShaderRenderCommand(int shaderIndex);
	virtual ~ShaderRenderCommand();

	void addPrimitive(const boost::shared_ptr<Primitive> &primitive);

	virtual void write(Iff *iff) const;

private:

	typedef std::vector<boost::shared_ptr<Primitive> >  PrimitiveVector;

private:

	int              m_shaderIndex;
	PrimitiveVector  m_primitives;

private:

	ShaderRenderCommand();
	ShaderRenderCommand(const ShaderRenderCommand&);
	ShaderRenderCommand &operator =(const ShaderRenderCommand&);
};

// ----------------------------------------------------------------------

class BlueprintWriter::PrepareCommand
{
public:

	PrepareCommand();

	PrepareOperationVector &getOperations(bool whichOperations);

	void                    write(Iff *iff) const;

private:

	PrepareOperationVector  m_conditionTruePrepareOperations;
	PrepareOperationVector  m_conditionFalsePrepareOperations;

};

// ======================================================================
// class SetConstantTextureOperation
// ======================================================================

SetConstantTextureOperation::SetConstantTextureOperation(int shaderIndex, Tag textureTag, int textureIndex)
:	PrepareOperation(),
	m_shaderIndex(shaderIndex),
	m_textureTag(textureTag),
	m_textureIndex(textureIndex)
{
}

// ----------------------------------------------------------------------

void SetConstantTextureOperation::write(Iff *iff) const
{
	NOT_NULL(iff);

	iff->insertChunk(TAG_SSTC);

		iff->insertChunkData(static_cast<int32>(m_shaderIndex));
		iff->insertChunkData(static_cast<uint32>(m_textureTag));
		iff->insertChunkData(static_cast<int32>(m_textureIndex));

	iff->exitChunk(TAG_SSTC);
}

// ======================================================================
// class Set1dTextureOperation
// ======================================================================

Set1dTextureOperation::Set1dTextureOperation(int shaderIndex, Tag textureTag, int baseTextureIndex, const std::string &indexVariableName, int entryCountSubscript0)
:	PrepareOperation(),
	m_shaderIndex(shaderIndex),
	m_textureTag(textureTag),
	m_baseTextureIndex(baseTextureIndex),
	m_indexVariableName(indexVariableName),
	m_entryCountSubscript0(entryCountSubscript0)
{
}

// ----------------------------------------------------------------------

Set1dTextureOperation::~Set1dTextureOperation()
{
}

// ----------------------------------------------------------------------

void Set1dTextureOperation::write(Iff *iff) const
{
	NOT_NULL(iff);

	iff->insertChunk(TAG_SST1);
		
		iff->insertChunkData(static_cast<int32>(m_shaderIndex));
		iff->insertChunkData(static_cast<uint32>(m_textureTag));
		iff->insertChunkData(static_cast<int32>(m_baseTextureIndex));

		iff->insertChunkString(m_indexVariableName.c_str());
		iff->insertChunkData(static_cast<int32>(m_entryCountSubscript0));
	
	iff->exitChunk(TAG_SST1);
}

// ======================================================================
// class Set2dTextureOperation
// ======================================================================

Set2dTextureOperation::Set2dTextureOperation(int shaderIndex, Tag textureTag, int baseTextureIndex, const std::string &subscript0IndexVariableName, int entryCountSubscript0, const std::string &subscript1IndexVariableName, int entryCountSubscript1)
:	PrepareOperation(),
	m_shaderIndex(shaderIndex),
	m_textureTag(textureTag),
	m_baseTextureIndex(baseTextureIndex),
	m_subscript0IndexVariableName(subscript0IndexVariableName),
	m_entryCountSubscript0(entryCountSubscript0),
	m_subscript1IndexVariableName(subscript1IndexVariableName),
	m_entryCountSubscript1(entryCountSubscript1)
{
}

// ----------------------------------------------------------------------

Set2dTextureOperation::~Set2dTextureOperation()
{
}

// ----------------------------------------------------------------------

void Set2dTextureOperation::write(Iff *iff) const
{
	NOT_NULL(iff);

	iff->insertChunk(TAG_SST2);
		
		iff->insertChunkData(static_cast<int32>(m_shaderIndex));
		iff->insertChunkData(static_cast<uint32>(m_textureTag));
		iff->insertChunkData(static_cast<int32>(m_baseTextureIndex));

		iff->insertChunkString(m_subscript0IndexVariableName.c_str());
		iff->insertChunkData(static_cast<int32>(m_entryCountSubscript0));

		iff->insertChunkString(m_subscript1IndexVariableName.c_str());
		iff->insertChunkData(static_cast<int32>(m_entryCountSubscript1));
	
	iff->exitChunk(TAG_SST2);
}

// ======================================================================
// class SetTextureFactorAlphaOperation
// ======================================================================

SetTextureFactorAlphaOperation::SetTextureFactorAlphaOperation(int shaderIndex, Tag textureFactorTag, uint8 red, uint8 green, uint8 blue, const std::string &textureFactorAlphaVariableName)
:	PrepareOperation(),
	m_shaderIndex(shaderIndex),
	m_textureFactorTag(textureFactorTag),
	m_red(red),
	m_green(green),
	m_blue(blue),
	m_textureFactorAlphaVariableName(textureFactorAlphaVariableName)
{
}

// ----------------------------------------------------------------------

SetTextureFactorAlphaOperation::~SetTextureFactorAlphaOperation()
{
}

// ----------------------------------------------------------------------

void SetTextureFactorAlphaOperation::write(Iff *iff) const
{
	NOT_NULL(iff);

	iff->insertChunk(TAG_STFA);

		iff->insertChunkData(static_cast<int32>(m_shaderIndex));
		iff->insertChunkData(static_cast<uint32>(m_textureFactorTag));
		iff->insertChunkData(static_cast<uint8>(m_red));
		iff->insertChunkData(static_cast<uint8>(m_green));
		iff->insertChunkData(static_cast<uint8>(m_blue));
		iff->insertChunkString(m_textureFactorAlphaVariableName.c_str());

	iff->exitChunk(TAG_STFA);
}

// ======================================================================
// class SetTextureFactorFromPaletteOperation
// ======================================================================

SetTextureFactorFromPaletteOperation::SetTextureFactorFromPaletteOperation(int shaderIndex, Tag textureFactorTag, const std::string &variableName, bool variableIsPrivate, const std::string &palettePathName) :
	PrepareOperation(),
	m_shaderIndex(shaderIndex),
	m_textureFactorTag(textureFactorTag),
	m_variableName(variableName),
	m_variableIsPrivate(variableIsPrivate),
	m_palettePathName(palettePathName)
{
}

// ----------------------------------------------------------------------

SetTextureFactorFromPaletteOperation::~SetTextureFactorFromPaletteOperation()
{
}

// ----------------------------------------------------------------------

void SetTextureFactorFromPaletteOperation::write(Iff *iff) const
{
	NOT_NULL(iff);

	iff->insertChunk(TAG_STFP);

		iff->insertChunkData(static_cast<int16>(m_shaderIndex));
		iff->insertChunkData(static_cast<uint32>(m_textureFactorTag));
		iff->insertChunkString(m_palettePathName.c_str());
		iff->insertChunkString(m_variableName.c_str());
		iff->insertChunkData(static_cast<int8>(m_variableIsPrivate ? 1 : 0));

	iff->exitChunk(TAG_STFP);
}

// ======================================================================
// class TriFanPrimitive
// ======================================================================

TriFanPrimitive::TriFanPrimitive(int vertexBufferIndex)
:	Primitive(),
	m_vertexBufferIndex(vertexBufferIndex)
{
}

// ----------------------------------------------------------------------

TriFanPrimitive::~TriFanPrimitive()
{
}

// ----------------------------------------------------------------------

void TriFanPrimitive::write(Iff *iff) const
{
	NOT_NULL(iff);

	iff->insertChunk(TAG_RTFC);
		iff->insertChunkData(static_cast<int32>(m_vertexBufferIndex));
	iff->exitChunk(TAG_RTFC);
}

// ======================================================================
// class BlueprintWriter::ClearFrameBufferRenderCommand
// ======================================================================

BlueprintWriter::ClearFrameBufferRenderCommand::ClearFrameBufferRenderCommand(bool clearColor, uint32 colorValue, bool clearDepth, float depthValue, bool clearStencil, uint32 stencilValue)
:	RenderCommand(),
	m_clearColor(clearColor),
	m_colorValue(colorValue),
	m_clearDepth(clearDepth),
	m_depthValue(depthValue),
	m_clearStencil(clearStencil),
	m_stencilValue(stencilValue)
{
}

// ----------------------------------------------------------------------

void BlueprintWriter::ClearFrameBufferRenderCommand::write(Iff *iff) const
{
	NOT_NULL(iff);

	iff->insertChunk(TAG_CFBC);

		iff->insertChunkData(static_cast<int32>(m_clearColor));
		iff->insertChunkData(static_cast<uint32>(m_colorValue));

		iff->insertChunkData(static_cast<int32>(m_clearDepth));
		iff->insertChunkData(static_cast<float>(m_depthValue));

		iff->insertChunkData(static_cast<int32>(m_clearStencil));
		iff->insertChunkData(static_cast<uint32>(m_stencilValue));

	iff->exitChunk(TAG_CFBC);
}

// ======================================================================
// class BlueprintWriter::ShaderRenderCommand
// ======================================================================

BlueprintWriter::ShaderRenderCommand::ShaderRenderCommand(int shaderIndex)
:	RenderCommand(),
	m_shaderIndex(shaderIndex),
	m_primitives()
{
}

// ----------------------------------------------------------------------

BlueprintWriter::ShaderRenderCommand::~ShaderRenderCommand()
{
}

// ----------------------------------------------------------------------

void BlueprintWriter::ShaderRenderCommand::addPrimitive(const boost::shared_ptr<Primitive> &primitive)
{
	m_primitives.push_back(primitive);
}

// ----------------------------------------------------------------------

void BlueprintWriter::ShaderRenderCommand::write(Iff *iff) const
{
	NOT_NULL(iff);

	iff->insertForm(TAG_SRSC);

		iff->insertChunk(TAG_INFO);

			iff->insertChunkData(static_cast<int32>(m_shaderIndex));
			iff->insertChunkData(static_cast<int32>(m_primitives.size()));

		iff->exitChunk(TAG_INFO);

		std::for_each(m_primitives.begin(), m_primitives.end(), VoidBindSecond(VoidMemberFunction(&Primitive::write), iff));

	iff->exitForm(TAG_SRSC);
}

// ======================================================================
// class BlueprintWriter::PrepareCommand
// ======================================================================

BlueprintWriter::PrepareCommand::PrepareCommand()
:	m_conditionTruePrepareOperations(),
	m_conditionFalsePrepareOperations()
{
}

// ----------------------------------------------------------------------

PrepareOperationVector &BlueprintWriter::PrepareCommand::getOperations(bool whichOperations)
{
	if (whichOperations)
		return m_conditionTruePrepareOperations; //lint !e1536 // exposing low access member
	else
		return m_conditionFalsePrepareOperations; //lint !e1536 // exposing low access member
}

// ----------------------------------------------------------------------

void BlueprintWriter::PrepareCommand::write(Iff *iff) const
{
	NOT_NULL(iff);

	iff->insertForm(TAG_PCMD);

		if (!m_conditionTruePrepareOperations.empty())
		{
			iff->insertForm(TAG_TOPS);

				iff->insertChunk(TAG_INFO);
					iff->insertChunkData(static_cast<int32>(m_conditionTruePrepareOperations.size()));
				iff->exitChunk(TAG_INFO);

				std::for_each(m_conditionTruePrepareOperations.begin(), m_conditionTruePrepareOperations.end(), VoidBindSecond(VoidMemberFunction(&PrepareOperation::write), iff));

			iff->exitForm(TAG_TOPS);
		}

		if (!m_conditionFalsePrepareOperations.empty())
		{
			iff->insertForm(TAG_FOPS);

				iff->insertChunk(TAG_INFO);
					iff->insertChunkData(static_cast<int32>(m_conditionFalsePrepareOperations.size()));
				iff->exitChunk(TAG_INFO);

				std::for_each(m_conditionFalsePrepareOperations.begin(), m_conditionFalsePrepareOperations.end(), VoidBindSecond(VoidMemberFunction(&PrepareOperation::write), iff));

			iff->exitForm(TAG_FOPS);
		}

	iff->exitForm(TAG_PCMD);
}

// ======================================================================
// class BlueprintWriter
// ======================================================================

BlueprintWriter::BlueprintWriter()
:	m_variableOwnership(0),
	m_destTextureWidth(256),
	m_destTextureHeight(256),
	m_runtimeFormats(new TextureFormatVector()),
	m_cameraProjectionLength(0.0f),
	m_shaderTemplateNames(new StringVector()),
	m_textureNames(new StringVector()),
	m_vertexBuffers(new VertexBufferVector()),
	m_renderCommands(new RenderCommandVector()),
	m_currentShaderRenderCommand(NULL),
	m_prepareCommands(new PrepareCommandVector()),
	m_currentPrepareCommand(NULL),
	m_currentPrepareOperationType(-1)
{
}

// ----------------------------------------------------------------------

BlueprintWriter::~BlueprintWriter()
{
	m_currentPrepareCommand      = NULL;
	m_currentShaderRenderCommand = NULL;
}

// ----------------------------------------------------------------------

void BlueprintWriter::setDestTextureAttributes(int width, int height, const TextureFormatVector &runtimeTextureFormats)
{
	m_destTextureWidth  = width;
	m_destTextureHeight = height;
	*m_runtimeFormats   = runtimeTextureFormats;
}

// ----------------------------------------------------------------------

void BlueprintWriter::setCamera(float projectionLength)
{
	m_cameraProjectionLength = projectionLength;
}

// ----------------------------------------------------------------------

int BlueprintWriter::addShaderTemplate(const std::string &shaderTemplateName)
{
	m_shaderTemplateNames->push_back(boost::shared_ptr<std::string>(new std::string(shaderTemplateName)));
	return static_cast<int>(m_shaderTemplateNames->size() - 1);
}

// ----------------------------------------------------------------------

int BlueprintWriter::addTexture(const std::string &textureName)
{
	m_textureNames->push_back(boost::shared_ptr<std::string>(new std::string(textureName)));
	return static_cast<int>(m_textureNames->size() - 1);
}

// ----------------------------------------------------------------------

int BlueprintWriter::addVertexBuffer(const SystemVertexBuffer &vertexBuffer)
{
	const int vertexCount = vertexBuffer.getNumberOfVertices();

	boost::shared_ptr<SystemVertexBuffer> newVertexBuffer(new SystemVertexBuffer(vertexBuffer.getFormat(), vertexCount));
	newVertexBuffer->begin().copy(vertexBuffer.beginReadOnly(), vertexCount);

	m_vertexBuffers->push_back(newVertexBuffer);
	return static_cast<int>(m_vertexBuffers->size() - 1);
}

// ----------------------------------------------------------------------

int BlueprintWriter::beginNewShaderRenderCommand(int shaderIndex)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, shaderIndex, static_cast<int>(m_shaderTemplateNames->size()));

	boost::shared_ptr<ShaderRenderCommand> newRenderCommand(new ShaderRenderCommand(shaderIndex));
	m_renderCommands->push_back(newRenderCommand);

	m_currentShaderRenderCommand = newRenderCommand.get();
	return static_cast<int>(m_renderCommands->size() - 1);
}

// ----------------------------------------------------------------------

void BlueprintWriter::addShaderRenderTriFanPrimitive(int vertexBufferIndex)
{
	DEBUG_FATAL(!m_currentShaderRenderCommand, ("not within a beginNewShaderRenderCommand()/endShaderRenderCommand() block"));

	m_currentShaderRenderCommand->addPrimitive(boost::shared_ptr<Primitive>(new TriFanPrimitive(vertexBufferIndex)));
}

// ----------------------------------------------------------------------

void BlueprintWriter::endShaderRenderCommand()
{
	DEBUG_FATAL(!m_currentShaderRenderCommand, ("not within a beginNewShaderRenderCommand()/endShaderRenderCommand() block"));

	m_currentShaderRenderCommand = NULL;
}

// ----------------------------------------------------------------------

int BlueprintWriter::addClearFrameBufferCommand(bool clearColor, uint32 colorValue, bool clearDepth, float depthValue, bool clearStencil, uint32 stencilValue)
{
	m_renderCommands->push_back(boost::shared_ptr<ClearFrameBufferRenderCommand>(new ClearFrameBufferRenderCommand(clearColor, colorValue, clearDepth, depthValue, clearStencil, stencilValue)));
	return static_cast<int>(m_renderCommands->size() - 1);
}

// ----------------------------------------------------------------------

void BlueprintWriter::beginNewPrepareCommand()
{
	DEBUG_FATAL(m_currentPrepareCommand, ("must not call beginNewPrepareCommand() while within a beginNewPrepareCommand()/endPrepareCommand() block"));

	boost::shared_ptr<PrepareCommand> newPrepareCommand(new PrepareCommand);
	m_prepareCommands->push_back(newPrepareCommand);

	m_currentPrepareCommand = newPrepareCommand.get();
}

// ----------------------------------------------------------------------

void BlueprintWriter::beginNewPrepareOperations(bool whichOperations)
{
	DEBUG_FATAL(!m_currentPrepareCommand, ("must call beginNewPrepareOperations() while within a beginNewPrepareCommand()/endPrepareCommand() block"));
	DEBUG_FATAL(m_currentPrepareOperationType != -1, ("must not start a new beginNewPrepareOperations() block while still in a previous one, call endPrepareOperations() first"));

	if (whichOperations)
		m_currentPrepareOperationType = 1;
	else
		m_currentPrepareOperationType = 0;
}

// ----------------------------------------------------------------------

void BlueprintWriter::endPrepareOperations()
{
	DEBUG_FATAL(m_currentPrepareOperationType == -1, ("not within a beginNewPrepareOperations()/endPrepareOperations() block"));
	m_currentPrepareOperationType = -1;
}

// ----------------------------------------------------------------------

void BlueprintWriter::addSetShaderConstantTextureOperation(int shaderIndex, Tag textureTag, int textureIndex)
{
	DEBUG_FATAL(!m_currentPrepareCommand, ("must call prepare operation additions while within a beginNewPrepareOperations()/endPrepareOperations() block"));
	DEBUG_FATAL(m_currentPrepareOperationType == -1, ("not within a beginNewPrepareOperations()/endPrepareOperations() block"));

	PrepareOperationVector &prepareOperations = m_currentPrepareCommand->getOperations(m_currentPrepareOperationType == 1);
	prepareOperations.push_back(boost::shared_ptr<PrepareOperation>(new SetConstantTextureOperation(shaderIndex, textureTag, textureIndex)));
}

// ----------------------------------------------------------------------

void BlueprintWriter::addSetShaderTexture1dOperation(int shaderIndex, Tag textureTag, int baseTextureIndex, const std::string &indexVariableName, int entryCountSubscript0)
{
	DEBUG_FATAL(!m_currentPrepareCommand, ("must call prepare operation additions while within a beginNewPrepareOperations()/endPrepareOperations() block"));
	DEBUG_FATAL(m_currentPrepareOperationType == -1, ("not within a beginNewPrepareOperations()/endPrepareOperations() block"));

	PrepareOperationVector &prepareOperations = m_currentPrepareCommand->getOperations(m_currentPrepareOperationType == 1);
	prepareOperations.push_back(boost::shared_ptr<PrepareOperation>(new Set1dTextureOperation(shaderIndex, textureTag, baseTextureIndex, indexVariableName, entryCountSubscript0)));
}

// ----------------------------------------------------------------------

void BlueprintWriter::addSetShaderTexture2dOperation(int shaderIndex, Tag textureTag, int baseTextureIndex, const std::string &subscript0IndexVariableName, int entryCountSubscript0, const std::string &subscript1IndexVariableName, int entryCountSubscript1)
{
	DEBUG_FATAL(!m_currentPrepareCommand, ("must call prepare operation additions while within a beginNewPrepareOperations()/endPrepareOperations() block"));
	DEBUG_FATAL(m_currentPrepareOperationType == -1, ("not within a beginNewPrepareOperations()/endPrepareOperations() block"));

	PrepareOperationVector &prepareOperations = m_currentPrepareCommand->getOperations(m_currentPrepareOperationType == 1);
	prepareOperations.push_back(boost::shared_ptr<PrepareOperation>(new Set2dTextureOperation(shaderIndex, textureTag, baseTextureIndex, subscript0IndexVariableName, entryCountSubscript0, subscript1IndexVariableName, entryCountSubscript1)));
}

// ----------------------------------------------------------------------

void BlueprintWriter::addSetShaderTextureFactorAlpha(int shaderIndex, Tag textureTag, uint8 red, uint8 green, uint8 blue, const std::string &textureFactorAlphaVariableName)
{
	DEBUG_FATAL(!m_currentPrepareCommand, ("must call prepare operation additions while within a beginNewPrepareOperations()/endPrepareOperations() block"));
	DEBUG_FATAL(m_currentPrepareOperationType == -1, ("not within a beginNewPrepareOperations()/endPrepareOperations() block"));

	PrepareOperationVector &prepareOperations = m_currentPrepareCommand->getOperations(m_currentPrepareOperationType == 1);
	prepareOperations.push_back(boost::shared_ptr<PrepareOperation>(new SetTextureFactorAlphaOperation(shaderIndex, textureTag, red, green, blue, textureFactorAlphaVariableName)));
}

// ----------------------------------------------------------------------

void BlueprintWriter::addSetShaderTextureFactorFromPalette(int shaderIndex, Tag textureFactorTag, const std::string &variableName, bool variableIsPrivate, const std::string &palettePathName)
{
	DEBUG_FATAL(!m_currentPrepareCommand, ("must call prepare operation additions while within a beginNewPrepareOperations()/endPrepareOperations() block"));
	DEBUG_FATAL(m_currentPrepareOperationType == -1, ("not within a beginNewPrepareOperations()/endPrepareOperations() block"));

	PrepareOperationVector &prepareOperations = m_currentPrepareCommand->getOperations(m_currentPrepareOperationType == 1);
	prepareOperations.push_back(boost::shared_ptr<PrepareOperation>(new SetTextureFactorFromPaletteOperation(shaderIndex, textureFactorTag, variableName, variableIsPrivate, palettePathName)));
}

// ----------------------------------------------------------------------

void BlueprintWriter::endPrepareCommand()
{
	DEBUG_FATAL(!m_currentPrepareCommand, ("must call endPrepareCommand() while within a beginNewPrepareCommand()/endPrepareCommand() block"));

	m_currentPrepareCommand = NULL;
}

// ----------------------------------------------------------------------

void BlueprintWriter::write(Iff &iff) const
{
	iff.insertForm(TAG_BTRT);
		iff.insertForm(TAG_0001);

			//-- write general info
			iff.insertChunk(TAG_INFO);
			{
				iff.insertChunkData(static_cast<uint32>(m_variableOwnership));
			}
			iff.exitChunk(TAG_INFO);

			//-- write destination texture info
			iff.insertChunk(TAG_DEST);
			{
				iff.insertChunkData(static_cast<int32>(m_destTextureWidth));
				iff.insertChunkData(static_cast<int32>(m_destTextureHeight));
				iff.insertChunkData(static_cast<uint32>(m_runtimeFormats->size()));

				const TextureFormatVector::const_iterator itEnd = m_runtimeFormats->end();
				for (TextureFormatVector::const_iterator it = m_runtimeFormats->begin(); it != itEnd; ++it)
					iff.insertChunkData(static_cast<uint32>(*it));
			}
			iff.exitChunk(TAG_DEST);

			//-- write shader templates
			iff.insertForm(TAG_SHTM);
			{
				iff.insertChunk(TAG_INFO);
					iff.insertChunkData(static_cast<int32>(m_shaderTemplateNames->size()));
				iff.exitChunk(TAG_INFO);

				const StringVector::const_iterator itEnd = m_shaderTemplateNames->end();
				for (StringVector::const_iterator it = m_shaderTemplateNames->begin(); it != itEnd; ++it)
				{
					iff.insertChunk(TAG_NAME);
						iff.insertChunkString((*it)->c_str());
					iff.exitChunk(TAG_NAME);
				}
			}
			iff.exitForm(TAG_SHTM);

			//-- write textures
			iff.insertForm(TAG_TXTS);
				iff.insertChunk(TAG_INFO);
				{
					iff.insertChunkData(static_cast<int32>(m_textureNames->size()));

					const StringVector::const_iterator itEnd = m_textureNames->end();
					for (StringVector::const_iterator it = m_textureNames->begin(); it != itEnd; ++it)
						iff.insertChunkString((*it)->c_str());
				}
				iff.exitChunk(TAG_INFO);
			iff.exitForm(TAG_TXTS);

			//-- write vertex buffers
			iff.insertForm(TAG_VBS);
			{
				iff.insertChunk(TAG_INFO);
					iff.insertChunkData(static_cast<int32>(m_vertexBuffers->size()));
				iff.exitChunk(TAG_INFO);

				const VertexBufferVector::const_iterator itEnd = m_vertexBuffers->end();
				for (VertexBufferVector::const_iterator it = m_vertexBuffers->begin(); it != itEnd; ++it)
				{
					const SystemVertexBuffer *const vb = it->get();
					NOT_NULL(vb);
					vb->write(iff);
				}
			}
			iff.exitForm(TAG_VBS);

			//-- write index buffers
			iff.insertForm(TAG_IBS);
				iff.insertChunk(TAG_INFO);
					iff.insertChunkData(static_cast<int32>(0));
				iff.exitChunk(TAG_INFO);
			iff.exitForm(TAG_IBS);

			//-- write camera info
			iff.insertForm(TAG_CAM);
				iff.insertChunk(TAG_PCAM);
					iff.insertChunkData(m_cameraProjectionLength);
				iff.exitChunk(TAG_PCAM);
			iff.exitForm(TAG_CAM);

			//-- write render commands
			iff.insertForm(TAG_RCMS);
			{
				iff.insertChunk(TAG_INFO);
					iff.insertChunkData(static_cast<int32>(m_renderCommands->size()));
				iff.exitChunk(TAG_INFO);

				std::for_each(m_renderCommands->begin(), m_renderCommands->end(), VoidBindSecond(VoidMemberFunction(&RenderCommand::write), &iff));
			}
			iff.exitForm(TAG_RCMS);

			//-- write prepare commands
			iff.insertForm(TAG_PCMS);
			{
				iff.insertChunk(TAG_INFO);
					iff.insertChunkData(static_cast<int32>(m_prepareCommands->size()));
				iff.exitChunk(TAG_INFO);

				std::for_each(m_prepareCommands->begin(), m_prepareCommands->end(), VoidBindSecond(VoidMemberFunction(&PrepareCommand::write), &iff));
			}
			iff.exitForm(TAG_PCMS);

		iff.exitForm(TAG_0001);
	iff.exitForm(TAG_BTRT);
}

// ======================================================================
