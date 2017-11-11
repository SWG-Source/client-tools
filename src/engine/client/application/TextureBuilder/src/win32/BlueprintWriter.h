// ======================================================================
//
// BlueprintWriter.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_BlueprintWriter_h
#define INCLUDED_BlueprintWriter_h

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

enum TextureFormat;

class Iff;
class Vector;
class VertexBuffer;
class SystemVertexBuffer;

// ======================================================================
/**
 * This class is responsible for preparing and writing 
 * BlueprintTextureRendererTemplate data files from input it is given.
 *
 * @see BlueprintTextureRendererTemplate
 * @see BlueprintTextureRenderer
 * @see TextureRenderer
 */

class BlueprintWriter
{
public:

	typedef stdvector<TextureFormat>::fwd   TextureFormatVector;

public:

	BlueprintWriter();
	~BlueprintWriter();

	// general setup
	void  setVariableOwnership(uint variableOwnership);

	void  setDestTextureAttributes(int width, int height, const TextureFormatVector &runtimeTextureFormats);
	void  setCamera(float projectionLength);

	// asset definitions
	int   addShaderTemplate(const std::string &shaderTemplateName);
	int   addTexture(const std::string &textureName);
	int   addVertexBuffer(const SystemVertexBuffer &vertexBuffer);

	// render commands
	int   beginNewShaderRenderCommand(int shaderIndex);
	void  addShaderRenderTriFanPrimitive(int vertexBufferIndex);

	void  endShaderRenderCommand();

	int   addClearFrameBufferCommand(bool clearColor, uint32 colorValue, bool clearDepth, float depthValue, bool clearStencil, uint32 stencilValue);

	// condition definitions

	// prepare commands
	void  beginNewPrepareCommand();

	void  beginNewPrepareOperations(bool whichOperations);
	void  endPrepareOperations();

	// prepare operations that can be added between beginCondition*Operations()/endCondition*Operations()
	void  addSetShaderConstantTextureOperation(int shaderIndex, Tag textureTag, int textureIndex);
	void  addSetShaderTexture1dOperation(int shaderIndex, Tag textureTag, int baseTextureIndex, const std::string &indexVariableName, int entryCountSubscript0);
	void  addSetShaderTexture2dOperation(int shaderIndex, Tag textureTag, int baseTextureIndex, const std::string &subscript0IndexVariableName, int entryCountSubscript0, const std::string &subscript1IndexVariableName, int entryCountSubscript1);

	void  addSetShaderTextureFactorFromPalette(int shaderIndex, Tag textureFactorTag, const std::string &variableName, bool variableIsPrivate, const std::string &palettePathName);
	void  addSetShaderTextureFactorAlpha(int shaderIndex, Tag textureFactorTag, uint8 red, uint8 green, uint8 blue, const std::string &textureFactorAlphaVariableName);

	void  endPrepareCommand();

	//-- write interface
	void  write(Iff &iff) const;

private:

	class RenderCommand;
	class ClearFrameBufferRenderCommand;
	class ShaderRenderCommand;

	class PrepareCommand;

	typedef stdvector<boost::shared_ptr<std::string> >::fwd             StringVector;
	typedef stdvector<boost::shared_ptr<SystemVertexBuffer> >::fwd      VertexBufferVector;
	typedef stdvector<boost::shared_ptr<RenderCommand> >::fwd           RenderCommandVector;
	typedef stdvector<boost::shared_ptr<PrepareCommand> >::fwd          PrepareCommandVector;

private:

	uint                                m_variableOwnership;

	int                                 m_destTextureWidth;
	int                                 m_destTextureHeight;
	std::auto_ptr<TextureFormatVector>  m_runtimeFormats;

	float                               m_cameraProjectionLength;

	std::auto_ptr<StringVector>         m_shaderTemplateNames;
	std::auto_ptr<StringVector>         m_textureNames;
	std::auto_ptr<VertexBufferVector>   m_vertexBuffers;

	std::auto_ptr<RenderCommandVector>  m_renderCommands;
	ShaderRenderCommand                *m_currentShaderRenderCommand;

	std::auto_ptr<PrepareCommandVector> m_prepareCommands;	
	PrepareCommand                     *m_currentPrepareCommand;
	int                                 m_currentPrepareOperationType;

private:
	// disabled
	BlueprintWriter(const BlueprintWriter&);
	BlueprintWriter &operator =(const BlueprintWriter&);
};

// ======================================================================
/**
 * Set the AvaiableVariables::Ownership type for all the variables
 * exposed by the exported BlueprintTextureRendererTemplate.
 *
 * @see AvailableVariables::Ownership
 */

inline void BlueprintWriter::setVariableOwnership(uint variableOwnership)
{
	m_variableOwnership = variableOwnership;
}

// ======================================================================

#endif
