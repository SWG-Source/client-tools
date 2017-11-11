// ======================================================================
//
// MayaUtility.h
// Portions Copyright 1999, Bootprint Entertainment
// Portions Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_MayaUtility_H
#define INCLUDED_MayaUtility_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

#include "ExporterDefines.h"

class CrcLowerString;
class Image;
class Material;
class MayaMeshWeighting;
class Messenger;
class Quaternion;
class Transform;
class Vector;
class VertexBuffer;

class MDagPath;
class MFnDagNode;
class MFnDependencyNode;
class MFnMesh;
class MIntArray;
class MMatrix;
class MObject;
class MObjectArray;
class MPlug;
class MSelectionList;
class MStatus;
class MString;
class MStringArray;

#include <set>

// ======================================================================

typedef stdvector<std::pair<std::string, Tag> >::fwd  TextureRendererVector;

class MayaUtility
{

public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct Limit
	{
		bool  enabled;
		real  value;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef stdvector<MDagPath>::fwd                  DagPathVector;
	typedef stdmap<std::string, uint32>::fwd          FileCrcMap;
	typedef std::set<std::pair<std::string, CompressType> > TextureSet;
	typedef std::pair<std::string, Tag>               TextureRendererReference;  // texture renderer name, shader template texture tag
	typedef stdvector<TextureRendererReference>::fwd  TRRVector;
	
public:

	static bool install (const char *logWindowName, Messenger *newMessenger);
	static void remove (void);

	static bool stripBaseName (const char *pathName, char *baseName, int baseNameSize, char pathSeparator);
	static void stripDagPathDirectory(const std::string &dagPathName, std::string &dagNodeName);

	static bool buildLocalToParentTransform (real rotateX, real rotateY, real rotateZ, real translateX, real translateY, real translateZ, Transform *transform);
	static bool buildQuaternion (real rotateX, real rotateY, real rotateZ, Quaternion *quaternion);

	static bool getFirstChildMesh (const MFnDagNode &fnDagNode, MObject *meshObject);

	static bool findSourceObjects (const MFnDependencyNode &node, const MString &attributeName, MObjectArray *objectArray, bool optional = false);
	static bool getNodePlug (const MFnDependencyNode &depNode, const MString &attributeName, MPlug *plug, bool optional = false);
	static bool getNodeFloatValue (const MFnDependencyNode &depNode, const MString &attributeName, real *realValue);
	static bool getNodeBoolValue (const MFnDependencyNode &depNode, const MString &attributeName, bool *boolValue, bool optional = false);
	static bool getNodeIntValue (const MFnDependencyNode &dependencyNode, const MString &attributeName, int &value, bool optional = false);
	static bool getNodeStringValue (const MFnDependencyNode &dependencyNode, const MString &attributeName, std::string &value, bool optional = false);

	static bool getNodeName (const MObject &object, MString *nodeName);
	static bool getNodeName (const MObject &object, char *buffer, int bufferSize);

	static bool getDagPathFromName(const char *nodeName, MDagPath *dagPath);
	static bool getChildPathWithName(const MDagPath &parentPath, const CrcLowerString &childName, MDagPath &childDagPath);
	static bool getDependencyNodeFromName(const MString &nodeName, MFnDependencyNode &dependencyNode);

	static bool getNodeToParentTransform (const MFnDagNode &dagNode, Transform *transform);
	static bool getJointOrientQuaternion (const MFnDagNode &dagNode, Quaternion *quaternion);
	static bool getNodeLimit (const MFnDagNode &dagNode, const char *limitBaseName, Limit *limit);
	static bool decomposeMatrix (const MMatrix *hardpointMatrix, Quaternion *orientation, Vector *position);

	static bool isInSet(const MDagPath &path, const char * name);

	static bool getFramesWithKeys (const MObject &object, bool checkRotation, bool checkTranslation, int *frameArray, int maxEntryCount, int *entryCount);

	static bool generateShaderIndexedTriangleArray (unsigned int shaderIndex, MFnMesh &fnMesh, const MIntArray &shaderMap, VertexBuffer **vertexBuffer, MIntArray *indexArray, bool *hasAlpha);

	static bool getAttachedTextureObjects(const MFnDependencyNode &layeredTextureNode, MObjectArray &attachedTextures);
	static bool generateTextures (const MayaUtility::TextureSet *referencedTextures, const FileCrcMap *existingCrcMap, const char *textureWriteDir, FileCrcMap *newlyExportedCrcMap, bool forceWrite = false);

	static bool getMeshColorAlphaStatus (const MFnMesh &fnMesh, int mayaShaderIndex, const MIntArray &polyShaderAssignment, bool *hasVertexColorSet, bool *hasVertexAlphaSet);

	static bool checkForDuplicateShortNames (const MDagPath &rootDagPath, MStringArray *duplicateNames);
	static bool checkForDuplicateShortNames (const MDagPath &rootDagPath, MStringArray *duplicateNames, int compatibleFnTypeMask);

	static bool findAncestorWithNameComponent (const MDagPath &dagPath, int componentIndex, MString *componentValue, bool *foundIt, MString *ancestorNodeName = 0, MDagPath *ancestorDagPath = 0);

	static bool createDirectory(const char *directory);
	static bool ignoreNode(const MDagPath &dagPath);

	static void loadFileCrcMap(FileCrcMap *existingCrcMap, const TextureSet &referencedTextures);
	static void saveFileCrcMap(const FileCrcMap &existingCrcMap, const FileCrcMap &revisedCrcMap, const TextureSet &referencedTextures);

	static bool goToBindPose(int alternateBindPoseFrameNumber = -10);
	static bool enableDeformers();

	static bool getSkeletonRootDagPath(MDagPath *skeletonRootDagPath);

	static void clearMayaObjectHack(MObject *object);

	static void dumpDependencyNodeAttributes(const MObject &object);

	static bool meshHasAlpha( MFnMesh const & fnMesh );	// returns true if the mesh in any way uses alpha blending
	static bool getSelectionListMeshShapes(const MSelectionList &selectionList, DagPathVector &dagPaths);

	static bool addMeshAffectors(const MDagPath &meshShapeDagPath, MayaMeshWeighting &meshWeighting);
	static bool getAttachmentTransformName(const MDagPath &skeletonRootDagPath, bool &hasAttachmentTransform, std::string &attachmentTransformName);

	static bool doesNameMatchLodFormat(const char *name);

	static MObject getRootMayaObject(MObject const & object, MStatus * const status);

	//-----------------------------------------------------------------------

	static const std::string &getAttributePrefix();
	static const std::string &getMayaShaderBuilderVersion();

	//-----------------------------------------------------------------------

	static std::string getTagFromAttributeName(const std::string& attributeName, const std::string& prefix);
	static Tag convertStringtoTag(const std::string& str);

	static bool getChannelTextures(
		MayaUtility::TextureSet    &o_textures,
		const MObject              &shaderGroupObject, 
		const std::string          &channelName,
		MayaUtility::CompressType   compressType
	);
	static bool  getNormalMapSourceFilename(std::string &o_name, const MObject &shaderGroupObject);
	static float getShaderHeightMapScale(const MObject &shaderGroupObject);

	//-----------------------------------------------------------------------

private:

	static bool exportTexture(const Image *image, const char *writePathname, int &mipmapLevelsExported, bool compress);
	static bool exportCompressedNormalMap(const Image *image, const char *writePathname);
	static std::string  createCrcPathname();

private:

	static const MString  cms_layeredTextureInputArrayAttributeName;

};

// ======================================================================

#endif
