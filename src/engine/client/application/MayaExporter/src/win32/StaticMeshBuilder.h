// ======================================================================
//
// StaticMeshBuilder.h
// Portions copyright 1999, Bootprint Entertainment, Inc.
// Portions copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_StaticMeshBuilder_H
#define INCLUDED_StaticMeshBuilder_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/ArrayList.h"
#include "clientGraphics/Graphics.def"
#include "MeshBuilder.h"
#include <string>
#include <vector>

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/CrcLowerString.h"
#include "sharedMath/Transform.h"

class Iff;
class Sphere;
class ShaderPrimitiveSetWriter;
class VertexBuffer;
class Extent;
class Image;

// ======================================================================
namespace StaticMeshBuilderNamespace
{
	struct NormalTexelMixer;
}

class StaticMeshBuilder: public MeshBuilder
{
public:
	struct ShaderData
	{
		ShaderData *next;

		std::string                       shaderTemplateName;
		std::string                       normalsBaseFileName;
		CompressType                      m_normalsCompressType;
		std::vector<MeshBuilder::Face *>  faces;
		bool                              m_wrapU;
		bool                              m_wrapV;
		bool                              hasDot3TextureCoordinate;
		int                               dot3TextureCoordinateIndex;
		int                               normalMapTextureCoordinateIndex;
		float                             m_heightMapScale;

	private:

		// disabled
		ShaderData();
		ShaderData(const ShaderData&);
		ShaderData &operator =(const ShaderData&);

	public:

		explicit ShaderData(const char *newName, bool i_hasDot3, int i_normalMapTextureCoordinateIndex);
		~ShaderData();

		void setShaderNormalsInfo(
			const char   *i_normalsBaseFileName, 
			CompressType  i_normalsCompressType,
			bool          wrapU,
			bool          wrapV,
			float         i_heightMapScale
			);

		bool build(ShaderPrimitiveSetWriter *writer, int priority);

		void getNormalsFileNames(
			std::string *o_tangentNormalsFileName,
			std::string *o_worldNormalsFileName,
			std::string *o_scratchMapFileName
		) const;
	};

private:

	friend struct ShaderData;

	typedef StaticMeshBuilderNamespace::NormalTexelMixer NormalTexelMixer;

protected:

	// ----------------------------------------------------------------------------
	bool _transformWorldSpaceNormals(
		NormalTexelMixer *mixBuffer, 
		const Image *worldNormalsImage, 
		const Image *tangentNormalsImage, 
		const Image *scratchNormalsImage, 
		const std::vector<const ShaderData *> &shaders
	);
	bool _transformWorldSpaceNormals(const std::vector<const ShaderData *> &shaders);
	void _transformWorldSpaceNormals();
	// ----------------------------------------------------------------------------

	ShaderPrimitiveSetWriter *m_setWriter;

	int                       m_shaderTemplateCount;

	ShaderData               *m_firstShaderData;
	ShaderData               *m_lastShaderData;

	bool                      m_hasAlpha;

private:

	// disabled
	StaticMeshBuilder();
	StaticMeshBuilder(const StaticMeshBuilder&);
	StaticMeshBuilder &operator =(const StaticMeshBuilder&);

public:

	explicit StaticMeshBuilder(MObject const & rootMayaObject);
	virtual ~StaticMeshBuilder();

	virtual bool hasFrames() const;

	virtual bool setShaderTemplate(
		const char *shaderTemplateName, 
		int *shaderTemplateIndex, 
		bool dot3, 
		int normalMapTextureCoordinateIndex
		);

	void setShaderNormalsInfo(
		const char   *i_normalsBaseFileName, 
		CompressType  i_normalsCompressType,
		bool          wrapU,
		bool          wrapV,
		float         heightMapScale
	);

	virtual bool  addTriangleFace(const Face &face);

	virtual void  setHasAlpha(bool bHasAlpha);
	virtual bool  specifyNoMoreData();
	virtual void  getDot3TextureCoordinateInfo(const char *shaderTemplateName, bool &hasDot3Coordinate, int &textureCoordinateIndex) const;

	const ShaderPrimitiveSetWriter &getShaderPrimitiveSetWriter() const                                 { return *m_setWriter; }

	Sphere        getSphere() const;

	bool          write(Iff & iff, Vector &minVector, Vector &maxVector) const;
};

// ======================================================================


#endif
