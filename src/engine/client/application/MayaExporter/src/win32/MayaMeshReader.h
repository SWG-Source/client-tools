// ======================================================================
//
// MayaMeshReader.h
// Todd Fiala
//
// copyright 1999, Bootprint Entertainment
//
// ======================================================================

#ifndef MAYA_MESH_READER_H
#define MAYA_MESH_READER_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/ArrayList.h"
#include "sharedMath/Vector.h"

#include "maya/MDagPath.h"
#include "maya/MFloatArray.h"
#include "maya/MFloatPointArray.h"
#include "maya/MFloatVectorArray.h"
#include "maya/MIntArray.h"
#include "maya/MObjectArray.h"
#include "maya/MStringArray.h"

#include <string>
#include <vector>

class MeshBuilder;
class Messenger;

class MSelectionList;

// ======================================================================

class MayaMeshReader
{
private:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct MayaMeshData
	{
	private:

		enum
		{
			MAX_UV_COUNT = 8
		};

	public:
		// -TRF- fixed up color array, normal array, position array, etc
		//       poly iterator

		bool              *hasVertexAlphaArray;
		bool               isValid;
		MDagPath           dagPath;
		MFloatPointArray   pointArray;
		MFloatVectorArray  normalArray;
		MObjectArray       shaderGroupArray;
		MIntArray          polyShaderGroupMap;
		MStringArray       uvSetNameArray;
		ArrayList<int>     shaderGroupPolyCount;

	private:
	
		// disabled
		MayaMeshData(void);
		MayaMeshData(const MayaMeshData&);
		MayaMeshData &operator =(const MayaMeshData&);

	private:

		bool  _buildHasVertexAlphaArray(void);

	public:

		explicit MayaMeshData(const MDagPath &meshDagPath);
		~MayaMeshData(void);

		bool  addFaces(int shaderGroupIndex, MeshBuilder *builder) const;
		bool  usesShader(int shaderGroupIndex) const;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct ShaderMeshNode
	{
	
	public:

		const MayaMeshData *mayaMeshData;
		int                 shaderGroupIndex;

		ShaderMeshNode(const ShaderMeshNode&o);
		ShaderMeshNode &operator =(const ShaderMeshNode&o);
		ShaderMeshNode(const MayaMeshData *newMayaMeshData, int newShaderGroupIndex);
		~ShaderMeshNode(void);


	private:

		// disabled
		ShaderMeshNode(void);

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct PerShaderData
	{

	public:

		std::string     name;
		MObject         shaderGroupObject;
		std::vector<ShaderMeshNode> shaderMeshNodes;
		bool                        dot3;
		int                         normalMapTextureCoordinateIndex;

	private:

		// disabled
		PerShaderData(void);
		PerShaderData(const PerShaderData&);
		PerShaderData &operator =(const PerShaderData&);

	public:

		explicit PerShaderData(const char *newName, bool dot3, int normalMapTextureCoordinateIndex);
		~PerShaderData(void);

		bool  hasVertexAlpha(void) const;

		bool  addMayaMeshData(const MayaMeshData *mayaMeshData, int shaderGroupIndex);		
		bool  construct(MeshBuilder *builder, const char *shaderTemplateReferenceDir, const char *textureReferenceDir) const;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	bool                          isValidFlag;
	std::vector<MayaMeshData *>   mayaMeshData;
	std::vector<PerShaderData *>  perShaderData;

private:

	// disabled
	MayaMeshReader(void);
	MayaMeshReader(const MayaMeshReader&);
	MayaMeshReader &operator =(const MayaMeshReader&);

private:

	bool           buildMayaMeshDataList(const MSelectionList &transformList);
	bool           buildMayaMeshDataList(const MDagPath &meshObject);
	PerShaderData *getPerShaderData(const char *shaderTemplateBaseName, bool dot3, int normalMapTexCoordIndex);
	bool           buildPerShaderDataList(void);

public:

	static void install(Messenger *newMessenger);
	static void remove(void);

	static const Vector globallyMergeVertexPosition(const Vector& position);
	static void         clearGlobalVertexIndexer();

	explicit MayaMeshReader(const MSelectionList &transformList);
	explicit MayaMeshReader(const MDagPath &meshObject);
	~MayaMeshReader(void);

	bool isValid(void) const;

	bool generateShaderTemplateData(const MeshBuilder &builder, const char *textureReferenceDir, const char *effectReferenceDir, const char *shaderTemplateReferenceDir, const char *shaderTemplateWriteDir, const char *textureWriteDir) const;
	bool buildMesh(MeshBuilder *builder, const char *shaderTemplateReferenceDir, const char *textureReferenceDir) const;

	const char* getFirstShaderTemplateName (void) const;
};

// ======================================================================
// retrieve whether the MayaMeshReader object is valid
//
// Return Value:
//
//   true if the MayaMeshReader object was constructed successfully,
//   false otherwise.
//
// Remarks:
//
//   The client should discard the MayaMeshReader if this function
//   returns false after construction of the object.  A return value
//   of false indicates that an error occurred during construction of
//   the MayaMeshReader.

inline bool MayaMeshReader::isValid(void) const
{
	return isValidFlag;
}

// ======================================================================

inline MayaMeshReader::ShaderMeshNode::ShaderMeshNode(const ShaderMeshNode &o)
:	mayaMeshData(o.mayaMeshData),
	shaderGroupIndex(o.shaderGroupIndex)
{ 
}

// ----------------------------------------------------------------------

inline MayaMeshReader::ShaderMeshNode &MayaMeshReader::ShaderMeshNode::operator =(const ShaderMeshNode &o)
{
	if (&o == this)
		return *this;

	mayaMeshData     = o.mayaMeshData; 
	shaderGroupIndex = o.shaderGroupIndex; 

	return *this;
}

// ======================================================================

#endif
