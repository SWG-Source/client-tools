// ======================================================================
//
// MayaUtility.cpp
// Todd Fiala
//
// copyright 1999, Bootprint Entertainment
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "MayaUtility.h"

#include "AtiDXTC.h"
#include "ATI_Compress.h"
#include "clientGraphics/Dds.h"
#include "sharedFile/Iff.h"
#include "sharedImage/Image.h"
#include "sharedImage/ImageFormatList.h"
#include "sharedImage/ImageManipulation.h"
#include "sharedStatusWindow/LogWindow.h"
#include "MayaCompoundString.h"
#include "MayaMeshWeighting.h"
#include "MayaShaderReader.h"
#include "MayaMisc.h"
#include "Messenger.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Transform.h"
#include "sharedFile/TreeFile.h"

#include <algorithm>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <vector>

#include "maya/MAnimControl.h"
#include "maya/MDagPath.h"
#include "maya/MFn.h"
#include "maya/MFnAttribute.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MIntArray.h"
#include "maya/MItDependencyGraph.h"
#include "maya/MItMeshPolygon.h"
#include "maya/MFileIO.h"
#include "maya/MFnLambertShader.h"
#include "maya/MFnPhongShader.h"
#include "maya/MFnMesh.h"
#include "maya/MFnSet.h"
#include "maya/MFnStringData.h"
#include "maya/MGlobal.h"
#include "maya/MMatrix.h"
#include "maya/MObject.h"
#include "maya/MObjectArray.h"
#include "maya/MSelectionList.h"
#include "maya/MString.h"
#include "maya/MStringArray.h"
#include "maya/MTime.h"

// ======================================================================

const MString MayaUtility::cms_layeredTextureInputArrayAttributeName("inputs");

// ======================================================================
// static storage
// ======================================================================

const std::string  cs_masterName("master");

const char         cs_dagPathSeparatorChar = '|';

const char *const  ms_commandDisableAllNodes = "doEnableNodeItems false all;";
const MString      ms_commandDgDirty("dgdirty -a;");
const char *const  ms_commandGoToBindPose    = "dagPose -r -g -bp;";
const char *const  ms_commandEnableAllNodes  = "doEnableNodeItems true all;";

/// the special prefix on shader group attributes we assume means that the MayaShaderBuilder can process them
const std::string  ms_attributePrefix = "soe_";

/// this version number is used to sync up with the MayaShaderBuilder MEL script version
const std::string  ms_mayaShaderBuilderVersion = "1.0";

const int          ms_crcFileIffSize = 32 * 1024;

static Messenger *messenger;

// ======================================================================
// class MayaUtility
// ======================================================================

bool MayaUtility::install (const char *logWindowName, Messenger *newMessenger)
{
	messenger = newMessenger;
	LogWindow::install (logWindowName);
	return true;
}

// ----------------------------------------------------------------------

void MayaUtility::remove (void)
{
	messenger = 0;
}

// ----------------------------------------------------------------------

const std::string &MayaUtility::getAttributePrefix()
{
	return ms_attributePrefix;
}

// ----------------------------------------------------------------------

const std::string &MayaUtility::getMayaShaderBuilderVersion()
{
	return ms_mayaShaderBuilderVersion;
}

// ----------------------------------------------------------------------

#if 0

void MayaUtility::mayaLogIndent (int indentLevel, const char *format, ...)
{
	char buffer [1024];
	const int SPACES_PER_INDENT_LEVEL = 2;

	const int spaceCount = SPACES_PER_INDENT_LEVEL * indentLevel;
	memset (buffer, ' ', spaceCount);
	strcpy (buffer + spaceCount, format);

	va_list argList;
	va_start (argList, format);
	LogWindow::printArgList (buffer, argList);
	va_end (argList);
}

#endif

// ----------------------------------------------------------------------
/**
 * strip the base name (path name minus any directories minus the last extension).
 * 
 * Examples:
 * 
 * stripBaseName ("hello\\todd.trf", buffer, sizeof (buffer)) -> "todd"
 * stripBaseName ("d:\\otherdir\\todd.trf.abc", buffer, sizeof (buffer)) -> "todd.trf"
 */

bool MayaUtility::stripBaseName (const char *pathName, char *baseName, int baseNameSize, char pathSeparator)
{
	MESSENGER_INDENT;

	MESSENGER_REJECT (!pathName, ("stripBaseName (): null pathName arg\n"));
	MESSENGER_REJECT (!baseName, ("stripBaseName (): null baseName arg\n"));
	MESSENGER_REJECT (baseNameSize < 1, ("invalid baseNameSize arg %d\n", baseNameSize));

	// search for start of base + ext name
	const char *baseStart = strrchr (pathName, pathSeparator);
	if (baseStart)
		++baseStart;
	else
		baseStart = pathName;

	// copy base + ext into buffer
	memset (baseName, 0, static_cast<size_t>(baseNameSize));
	strncpy (baseName, baseStart, static_cast<size_t> (baseNameSize - 1));

	// search for extension, terminate at it
	char *ext = strrchr (baseName, '.');
	if (ext)
		*ext = 0;

	return true;
}

// ----------------------------------------------------------------------
/**
 * Given a Maya DagPath full or partial name, which may contain '|' path
 * separator characters, return the short node name.
 *
 * This function removes the optional DAG directory preceding the short
 * node name.
 *
 * e.g. stripDagPathDirectory("some_node|l0") returns "l0".
 *      stripDagPathDirectory("my_mesh")      returns "my_mesh".
 *
 * @param dagPathName  the name of the partial or full DAG path name, optionally
 *                     containing multiple parent levels separated by '|'.
 * @param dagNodeName  the short DAG node name is returned in this parameter.
 */

void MayaUtility::stripDagPathDirectory(const std::string &dagPathName, std::string &dagNodeName)
{
	//-- Find position of last separator character.
	const std::string::size_type lastDirSeparatorPos = dagPathName.rfind(cs_dagPathSeparatorChar);
	if (static_cast<int>(lastDirSeparatorPos) != static_cast<int>(std::string::npos))
	{
		IGNORE_RETURN(dagNodeName.assign(dagPathName.begin() + (lastDirSeparatorPos + 1), dagPathName.end()));
	}
	else
	{
		//-- There is no separator character, copy the whole thing.
		dagNodeName = dagPathName;
	}
}

// ----------------------------------------------------------------------
/**
 * build local to parent transform, performing rotation first, then
 * translation.
 * 
 * I wanted to localize this so I can support alternate rotation orders
 * as allowed by Maya.
 */

bool MayaUtility::buildLocalToParentTransform (real rotateX, real rotateY, real rotateZ, real translateX, real translateY, real translateZ, Transform *transform)
{
	MESSENGER_INDENT;

	if (!transform)
	{
		MESSENGER_LOG(("buildLocalToParentTransform () given null transform arg\n"));
		return false;
	}

	// make identity
	*transform = Transform::identity;

	// rotate in x,y,z order
	// -TRF- add xyz order so we preform them correctly
	transform->roll_l (rotateZ);
	transform->yaw_l (rotateY);
	transform->pitch_l (rotateX);

	// translate in parent space
	transform->setPosition_p (translateX, translateY, translateZ);

	return true;
}

// ----------------------------------------------------------------------
/**
 * build a quaternion that performs rotation in x,y,z order.
 */

bool MayaUtility::buildQuaternion (real rotateX, real rotateY, real rotateZ, Quaternion *quaternion)
{
	MESSENGER_INDENT;

	if (!quaternion)
	{
		MESSENGER_LOG (("buildQuaternion () null quaternion arg\n"));
		return false;
	}

	// build component quaternions for each of x,y,z
	Quaternion  qx (rotateX, Vector::unitX);
	Quaternion  qy (rotateY, Vector::unitY);
	Quaternion  qz (rotateZ, Vector::unitZ);

	// perform rotation in this order: first x, then y, finally z (these are Maya semantics for joint orientation, doesn't appear to be a way to change order)
	// note: quaternions build with first operation on left, last operation on right, opposite from transforms
	*quaternion = qz * (qy * qx);

	return true;
}

// ----------------------------------------------------------------------
/**
 * return the first non-intermediate child node that is a mesh.
 */

bool MayaUtility::getFirstChildMesh (const MFnDagNode &fnDagNode, MObject *meshObject)
{
	MESSENGER_INDENT;

	MESSENGER_REJECT (!meshObject, ("null meshObject arg\n"));

	MStatus     status;
	MFnDagNode  fnChildDag;

	const unsigned int childCount = fnDagNode.childCount ();
	for (unsigned int childIndex = 0; childIndex < childCount; ++childIndex)
	{
		*meshObject = fnDagNode.child (childIndex, &status);
		MESSENGER_REJECT (!status, ("failed to get child %u of %u, error = \"%s\"\n", childIndex+1, childCount, status.errorString ().asChar ()));

		if (meshObject->apiType() != MFn::kMesh)
			continue;

		// make sure we don't pick up an intermediate object
		status = fnChildDag.setObject (*meshObject);
		MESSENGER_REJECT (!status, ("failed to set dag object for child %u of %u, error = \"%s\"\n", childIndex+1, childCount, status.errorString ().asChar ()));

		bool isIntermediate = fnChildDag.isIntermediateObject (&status);
		MESSENGER_REJECT (!status, ("failed to get intermediate object status for child %u of %u, error = \"%s\"\n", childIndex+1, childCount, status.errorString ().asChar ()));

		// if the object isn't an intermediate, then we've found our mesh child node
		if (!isIntermediate)
			return true;
	}

	// didn't find a non-intermediate child mesh object
	return false;
}

// ----------------------------------------------------------------------
/**
 * find all upstream objects containing plugs sending information to the
 * specified node's attribute.
 * 
 * This function returns the objects that output data (via plugs) to
 * the specified attribute of the specified node.
 * 
 * -TRF- perhaps findUpstreamObjects () is a better name?
 */

bool MayaUtility::findSourceObjects (const MFnDependencyNode &node, const MString &attributeName, MObjectArray *objectArray, bool optional)
{
	MESSENGER_INDENT;
	
	MESSENGER_REJECT (!objectArray, ("null objectArray arg"));

	MStatus      status;
	const MPlug  destPlug = node.findPlug (attributeName, &status);
	if (!status)
	{
		if (!optional)
		{
			MESSENGER_REJECT_WARNING (true, ("failed to get plug for attribute %s, error = \"%s\"\n", attributeName.asChar (), status.errorString ().asChar ())); //lint !e506 !e774 // constant value boolean // @todo add a macro for constant warnings.
		}
		else
			return false;
	}

	MPlugArray sourcePlugArray;
	IGNORE_RETURN(destPlug.connectedTo (sourcePlugArray, true, false, &status));

	if (!status)
	{
		if (!optional)
		{
			MESSENGER_REJECT_WARNING (!status, ("failed to get plugs connected to %s plug, error = \"%s\"\n", attributeName.asChar (), status.errorString ().asChar()));
		}
		else
			return false;
	}

	// clear out the object array
	IGNORE_RETURN(objectArray->clear ());

	const unsigned int plugCount = sourcePlugArray.length ();
	for (unsigned int plugIndex = 0; plugIndex < plugCount; ++plugIndex)
	{
		status = objectArray->append (sourcePlugArray [plugIndex].node ());
		if (!status)
		{
			if (!optional)
			{
				MESSENGER_REJECT (!status, ("object list append failed, \"%s\"\n", status.errorString ().asChar ()));
			}
			else
				return false;
		}
	}
	return true;
}

// ----------------------------------------------------------------------

bool MayaUtility::getNodePlug (const MFnDependencyNode &depNode, const MString &attributeName, MPlug *plug, bool optional)
{
	MESSENGER_INDENT;

	if (!plug)
	{
		if (!optional)
		{
			MESSENGER_LOG_WARNING (("null plug arg\n"));
		}
		return false;
	}

	MStatus  status;
	*plug = depNode.findPlug (attributeName, &status);
	if (!status)
	{
		if (!optional)
		{
			MESSENGER_LOG_WARNING (("failed to find plug for %s, error=\"%s\"\n", attributeName.asChar (), status.errorString ().asChar()));
		}
		return false;
	}
	
	return true;
}

// ----------------------------------------------------------------------

bool MayaUtility::getNodeFloatValue (const MFnDependencyNode &depNode, const MString &attributeName, real *realValue)
{
	MESSENGER_INDENT;

	if (!realValue)
	{
		MESSENGER_LOG_WARNING (("null realValue arg\n"));
		return false;
	}

	// get the attribute plug
	MPlug  plug;
	if (!getNodePlug (depNode, attributeName, &plug))
	{
		MESSENGER_LOG_WARNING (("getNodeFloatValue () failed to get plug\n"));
		return false;
	}
	
	// get the attribute value
	MStatus  status;
	float    plugValue;

	status = plug.getValue (plugValue);
	if (!status)
	{
		MESSENGER_LOG_WARNING (("getNodeFloatValue () failed to get float value, error=\"%s\"\n", status.errorString ().asChar ()));
		return false;
	}

	// return value to caller
	*realValue = static_cast<real>(plugValue);
	return true;
}

// ----------------------------------------------------------------------

bool MayaUtility::getNodeBoolValue (const MFnDependencyNode &depNode, const MString &attributeName, bool *boolValue, bool optional)
{
	MESSENGER_INDENT;

	if (!boolValue)
	{
		MESSENGER_LOG_WARNING (("getNodeBoolValue  () passed null boolValue arg\n"));
		return false;
	}

	// get the attribute plug
	MPlug  plug;
	if (!getNodePlug (depNode, attributeName, &plug, optional))
	{
		if (!optional)
			MESSENGER_LOG_WARNING (("getNodeBoolValue () failed to get plug\n"));

		return false;
	}
	
	// get the attribute value
	MStatus  status;

	status = plug.getValue (*boolValue);
	if (!status)
	{
		if (!optional)
			MESSENGER_LOG_WARNING (("getNodeBoolValue () failed to get bool value, error=\"%s\"\n", status.errorString ().asChar ()));

		return false;
	}

	//-- Succeeded in retrieving the value.
	return true;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the int value associated with a given attribute name on a given dependency node.
 *
 * @param dependencyNode  the function set for the dependency node containing the attribute
 *                        to query.
 * @parma attributeName   the name of the attribute for which the value will be retrieved.
 * @param value           the value of the attribute will be stored in the value on successful return.
 * @param optional        if false, emits error messages when internal failures occur; otherwise no messages are printed.
 *
 * @return  true if the attribute exists and the value is retrieved; false otherwise.
 */

bool MayaUtility::getNodeIntValue (const MFnDependencyNode &dependencyNode, const MString &attributeName, int &value, bool optional)
{
	MESSENGER_INDENT;

	//-- get the attribute plug
	MPlug  plug;
	if (!getNodePlug (dependencyNode, attributeName, &plug, optional))
	{
		if (!optional)
		{
			MESSENGER_LOG_WARNING (("getNodeIntValue () failed to get plug\n"));
		}
		return false;
	}
	
	//-- get the attribute value
	MStatus  status;

	status = plug.getValue (value);
	if (!status)
	{
		if (!optional)
		{
			MESSENGER_LOG_WARNING (("getNodeIntValue () failed to get int value, error [%s].\n", status.errorString ().asChar ()));
		}
		return false;
	}

	//-- return success
	return true;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the string value associated with a given attribute name on a given dependency node.
 *
 * @param dependencyNode  the function set for the dependency node containing the attribute
 *                        to query.
 * @parma attributeName   the name of the attribute for which the value will be retrieved.
 * @param value           the value of the attribute will be stored in the value on successful return.
 *
 * @return  true if the attribute exists and the value is retrieved; false otherwise.
 */

bool MayaUtility::getNodeStringValue (const MFnDependencyNode &dependencyNode, const MString &attributeName, std::string &value, bool optional)
{
	MESSENGER_INDENT;

	//-- get the attribute plug
	MPlug  plug;
	if (!getNodePlug (dependencyNode, attributeName, &plug, optional))
	{
		if (!optional)
			MESSENGER_LOG_WARNING (("getNodeStringValue () failed to get plug\n"));

		return false;
	}
	
	//-- get the attribute value
	MStatus  status;
	MString  mstringValue;

	status = plug.getValue (mstringValue);
	if (!status)
	{
		if (!optional)
			MESSENGER_LOG_WARNING (("getNodeStringValue () failed to get string value, error [%s].\n", status.errorString ().asChar ()));

		return false;
	}

	value = mstringValue.asChar();

	//-- return success
	return true;
}

// ----------------------------------------------------------------------
/**
 * retrieve the name of the given dependency node.
 * 
 * @param object  [IN]  dependency node object for which the name will be retrieve
 * @param nodeName  [OUT] string containing name of dependency node
 * @return true:  dependency node name retrieved
 * false: failure to retrieve name
 */

bool MayaUtility::getNodeName (const MObject &object, MString *nodeName)
{
	MESSENGER_INDENT;

	MESSENGER_REJECT (!nodeName, ("null nodeName arg"));

	// make object a dependency node
	MStatus            status;
	MFnDependencyNode  fnDependencyNode (object, &status);
	MESSENGER_REJECT (!status, ("getNodeName() failed to set object for MFnDependencyNode, error = \"%s\"\n", status.errorString ().asChar ()));

	// return dependency node name
	*nodeName = fnDependencyNode.name ();
	return true;
}

// ----------------------------------------------------------------------
/**
 * retrieve the name of the given dependency node.
 * 
 * @param object  [IN]  dependency node object for which the name will be retrieve
 * @param buffer  [OUT] string containing name of dependency node
 * @param bufferSize  [IN]  byte size of buffer
 * @return true:  dependency node name retrieved
 * false: failure to retrieve name
 */

bool MayaUtility::getNodeName (const MObject &object, char *buffer, int bufferSize)
{
	MESSENGER_INDENT;

	MString nameString;

	const bool result = getNodeName (object, &nameString);
	MESSENGER_REJECT (!result, ("failed to get dependency node name\n"));

	const unsigned nameLength = nameString.length ();
	MESSENGER_REJECT (nameLength >= static_cast<unsigned>(bufferSize), ("getNodeName () given buffer too small, requires %u bytes, specified size is %d bytes\n", nameLength+1, bufferSize));

	strcpy(buffer, nameString.asChar ());
	return true;
}

// ----------------------------------------------------------------------

bool MayaUtility::getDagPathFromName(const char *nodeName, MDagPath *dagPath)
{
	NOT_NULL(dagPath);
	MStatus status;

	MSelectionList  nodeList;
	status = MGlobal::getSelectionListByName(MString(nodeName), nodeList);
	MESSENGER_REJECT(!status, ("MGlobal::getSelectionListByName failed to to find %s in the selection\n", nodeName));

	MESSENGER_REJECT(nodeList.length() < 1, ("no scene nodes match specified node [%s]\n", nodeName));
	MESSENGER_REJECT(nodeList.length() > 1, ("multiple nodes match specified node [%s]\n", nodeName));

	status = nodeList.getDagPath(0, *dagPath);
	MESSENGER_REJECT(!status, ("failed to get dag path for export node [%s]\n", nodeName));

	//-- success
	return true;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the MDagPath for a direct descendant of the parent MDagPath.
 *
 * This function will lower-case the child names prior to doing a name
 * comparison, so the case of child node names is irrelevant.
 *
 * @param parentPath    the MDagPath to the parent.
 * @param childName     the lower-case name of the child to search for.
 * @param childDagPath  if found, the dag path to the child with the given name is returned here.
 *
 * @return  true if the child with the given name was found; false if the child
 *          was not found or an error occurred.
 */

bool MayaUtility::getChildPathWithName(const MDagPath &parentPath, const CrcLowerString &childName, MDagPath &childDagPath)
{
	MStatus status;

	//-- Check each child for a name match with the given childName.
	const unsigned childCount = parentPath.childCount(&status);
	STATUS_REJECT(status, "parentPath.childCount()");

	MFnDagNode  childDagNode;

	for (unsigned i = 0; i < childCount; ++i)
	{
		// Get the child object.
		MObject childObject = parentPath.child(i, &status);
		STATUS_REJECT(status, "parentPath.child()");

		// Get MFnDependencyNode for childObject.
		status = childDagNode.setObject(childObject);
		STATUS_REJECT(status, "childDagNode.setObject()");

		// Test child's name against target name.
		if (CrcLowerString(childDagNode.name().asChar()) == childName)
		{
			// Child matched: return the MDagPath to the child.
			status = childDagNode.getPath(childDagPath);
			STATUS_REJECT(status, "childDagNode.getPath()");

			return true;
		}
	}

	//-- Child path not found.
	return false;
}

// ----------------------------------------------------------------------
/**
 * Retrieve a dependency node function set for a node of a given name.
 *
 * If multiple nodes match the given name, the first node will be
 * initialized into the function set.
 *
 * @param nodeName        the name of the dependency node for which a function
 *                        set will be retrieved.  By necessity this must be a
 *                        short name since we are cannot assume the node is a
 *                        dag node.
 * @param dependencyNode  if the dependency node exists, this function set will
 *                        be initialized to the proper node upon return.
 *
 * @return  true if the dependency node exists and is set into the function set;
 *          false otherwise.
 */

bool MayaUtility::getDependencyNodeFromName(const MString &nodeName, MFnDependencyNode &dependencyNode)
{
	// NOTE: I haven't found a simple way to retrieve a Maya dependency node by name.  A dag node
	//       can be retrieved by name.
	MStatus  status;

	//-- save the active selection list
	MSelectionList  savedActiveList;
	IGNORE_RETURN(MGlobal::getActiveSelectionList(savedActiveList));

	//-- select nodes by the specified name
	status = MGlobal::selectByName(nodeName, MGlobal::kReplaceList);
	if (!status)
		return false;

	//-- get matching names
	MSelectionList  matchList;
	IGNORE_RETURN(MGlobal::getActiveSelectionList(matchList));

	//-- restore selection list
	IGNORE_RETURN(MGlobal::setActiveSelectionList(savedActiveList));

	//-- select the first returned node into the dependency node
	if (matchList.length() < 1)
		return false;

	MObject  dependencyNodeObject;
	status = matchList.getDependNode(0, dependencyNodeObject);
	if (!status)
		return false;

	status = dependencyNode.setObject(dependencyNodeObject);
	if (!status)
		return false;
	
	//-- return success
	return true;
}

// ----------------------------------------------------------------------

bool MayaUtility::getNodeToParentTransform (const MFnDagNode &dagNode, Transform *transform)
{
	MESSENGER_INDENT;
	MESSENGER_REJECT (!transform, ("null transform object specified\n"));

	// we're going to build this ourselves so we know exactly what we're getting.
	// maya has a matrix already built for us, but who knows exactly what we'll get.

	bool       result;

	real       rotateX;
	real       rotateY;
	real       rotateZ;
	real       translateX;
	real       translateY;
	real       translateZ;

	// get the rotation.
	result = getNodeFloatValue (dagNode, "jointOrientX", &rotateX);
	MESSENGER_REJECT (!result, ("getNodeToParentTransform () failed to get rotateX\n"));

	result = getNodeFloatValue (dagNode, "jointOrientY", &rotateY);
	MESSENGER_REJECT (!result, ("getNodeToParentTransform () failed to get rotateY\n"));

	result = getNodeFloatValue (dagNode, "jointOrientZ", &rotateZ);
	MESSENGER_REJECT (!result, ("getNodeToParentTransform () failed to get rotateZ\n"));

	// get the translation
	result = getNodeFloatValue (dagNode, "translateX", &translateX);
	MESSENGER_REJECT (!result, ("getNodeToParentTransform () failed to get translateX\n"));

	result = getNodeFloatValue (dagNode, "translateY", &translateY);
	MESSENGER_REJECT (!result, ("getNodeToParentTransform () failed to get translateY\n"));

	result = getNodeFloatValue (dagNode, "translateZ", &translateZ);
	MESSENGER_REJECT (!result, ("getNodeToParentTransform () failed to get translateZ\n"));

	// engine rotate x = maya rotate x
	// engine rotate y = -maya rotate y
	// engine rotate z = -maya rotate z

	// engine translate x = - maya translate x
	// engine translate y = maya translate y
	// engine translate z = maya translate z

	// -TRF- for now, assume the artists are using xyz rotation order.  later we probably
	// want to check the value of rotationOrder (in what format is rotationOrder?)
	result = buildLocalToParentTransform (rotateX, -rotateY, -rotateZ, -translateX, translateY, translateZ, transform);
	MESSENGER_REJECT (!result, ("buildLocalToParentTransform failed\n"));

	return true;
}

// ----------------------------------------------------------------------

bool MayaUtility::getJointOrientQuaternion (const MFnDagNode &dagNode, Quaternion *quaternion)
{
	MESSENGER_INDENT;

	if (!quaternion)
	{
		MESSENGER_LOG_WARNING (("getJointOrientQuaternion () null quaternion arg\n"));
		return false;
	}

	// get maya joint orient attributes
	real  orientX;
	real  orientY;
	real  orientZ;

	if (!getNodeFloatValue (dagNode, "rotateX", &orientX))
	{
		MESSENGER_LOG_WARNING (("getJointOrientQuaternion () failed to get jointOrientX attribute\n"));
		return false;
	}
	if (!getNodeFloatValue (dagNode, "rotateY", &orientY))
	{
		MESSENGER_LOG_WARNING (("getJointOrientQuaternion () failed to get jointOrientY attribute\n"));
		return false;
	}
	if (!getNodeFloatValue (dagNode, "rotateZ", &orientZ))
	{
		MESSENGER_LOG_WARNING (("getJointOrientQuaternion () failed to get jointOrientZ attribute\n"));
		return false;
	}

	// engine rotate x = maya rotate x
	// engine rotate y = -maya rotate y
	// engine rotate z = -maya rotate z
	const bool result = buildQuaternion (orientX, -orientY, -orientZ, quaternion);
	if (!result)
	{
		MESSENGER_LOG_WARNING (("getJointOrientQuaternion () failed to build quaternion\n"));
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------
/**
 * retrieve the limit info for the specified node's attribute.
 * 
 * Each limit has an enabled state, indicating whether the limit
 * should take place, and a value that indicates the limit.  The
 * attribute for the limit enabled state attribute is <limitBaseName> + "LimitEnable".
 * The attribute name for the limit value attribute is
 * <limitBaseName> + "Limit".
 */

bool MayaUtility::getNodeLimit (const MFnDagNode &dagNode, const char *limitBaseName, Limit *limit)
{
	MESSENGER_INDENT;

	if (!limit)
	{
		MESSENGER_LOG_WARNING (("getNodeLimit () given null limit arg\n"));
		return false;
	}
	if (!limitBaseName)
	{
		MESSENGER_LOG_WARNING (("getNodeLimit () given null limitBaseName arg\n"));
		return false;
	}

	char attributeName [128];

	// -TRF- I'm doing the enable first, forcing less efficient code, because I'm not sure
	// I can ask for the limit if its not enabled without causing an error.
	sprintf(attributeName, "%sLimitEnable", limitBaseName);
	if (!getNodeBoolValue (dagNode, attributeName, &(limit->enabled)))
	{
		MESSENGER_LOG_WARNING (("getNodeLimit () failed to get attribute named \"%s\"\n", attributeName));
		return false;
	}
	if (!limit->enabled)
	{
		// we can bail
		limit->value = CONST_REAL (0.0);
		return true;
	}

	sprintf(attributeName, "%sLimit", limitBaseName);
	if (!getNodeFloatValue (dagNode, attributeName, &(limit->value)))
	{
		MESSENGER_LOG_WARNING (("getNodeLimit () failed to get attribute named \"%s\"\n", attributeName));
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------
/**
 * decompose a maya matrix (right-handed) into the appropriate quaternion and vector
 * for our engine's left-handed use.
 * 
 * It appears Maya transposes the order of their matrices: i.e.
 * element[3][0] = x translate  (rather than our [0][3])
 * element[3][1] = y translate  (rather than our [1][3])
 * element[3][2] = z translate  (rather than our [2][3])
 */

bool MayaUtility::decomposeMatrix (const MMatrix *hardpointMatrix, Quaternion *orientation, Vector *position)
{
	MESSENGER_INDENT;

	MESSENGER_REJECT(!hardpointMatrix, ("null hardpointMatrix arg\n"));
	MESSENGER_REJECT(!orientation, ("null orientation arg\n"));
	MESSENGER_REJECT(!position, ("null position arg\n"));

	float    elements[4][4];
	MStatus  status;
	MMatrix  homMatrix = hardpointMatrix->homogenize();

#if 0
	float nonhomElements[4][4];
	hardpointMatrix->get(nonhomElements);
#endif

	status = homMatrix.get(elements);
	MESSENGER_REJECT(!status, ("get element failed,\"%s\"\n", status.errorString().asChar()));

	// flip maya matrix so as not to confuse myself
	int s;
	int t;
	for (s = 0; s < 3; ++s)
		for (t = s+1; t < 4; ++t)
		{
			float temp     = elements[s][t];
			elements[s][t] = elements[t][s];
			elements[t][s] = temp;
		}

	// save position (local to parent translation), flip x
	position->x = -elements[0][3];
	position->y =  elements[1][3];
	position->z =  elements[2][3];

	// reset position
	elements[0][3] = 0.0f;
	elements[1][3] = 0.0f;
	elements[2][3] = 0.0f;

	Transform  xform;
#if 1
	// I think multiplying x column by -1 will handle rhr -> lhr conversion
	elements[0][1] = -elements[0][1];
	elements[0][2] = -elements[0][2];
	elements[1][0] = -elements[1][0];
	elements[2][0] = -elements[2][0];

	// -TRF- totally ignore scale

	// setup transform
	int i;
	int j;
	for (i = 0; i < 3; ++i)
		for (j = 0; j < 4; ++j)
			xform.matrix[i][j] = elements[i][j];
#else
	// for now, set rotation to nothing
	memset(xform.matrix, 0, sizeof(xform.matrix));
	xform.matrix[0][0] = CONST_REAL(1.0);
	xform.matrix[1][1] = CONST_REAL(1.0);
	xform.matrix[2][2] = CONST_REAL(1.0);
#endif

	// initialize quaternion with transform
	*orientation = Quaternion(xform);

	return true;
}

// ----------------------------------------------------------------------

bool MayaUtility::getFramesWithKeys (const MObject &object, bool checkRotation, bool checkTranslation, int *frameArray, int maxEntryCount, int *entryCount)
{
	MESSENGER_INDENT;
	
	// look at rotateZ, rotateY, rotateX attributes for animCurveTA animation curves plugged in
	// look at translateZ, translateY, translateX attributes for any animCurveTL animation curves plugged in

	UNREF (object);
	UNREF (checkRotation);
	UNREF (checkTranslation);
	UNREF (frameArray);
	UNREF (maxEntryCount);
	UNREF (entryCount);

	// -TRF- to do

	return false;
}

// ----------------------------------------------------------------------

static bool WriteDdsHeader(FILE *file, int pixelWidth, int pixelHeight, uint ddsDataFormat, int mipmapLevelCount)
{
	NOT_NULL(file);
	MESSENGER_REJECT((pixelWidth < 1) || (pixelHeight < 1), ("bad image dimensions: (%d/%d)\n", pixelWidth, pixelHeight));

	//-- write the magic word ("DDS ")
	size_t actualWriteSize;

	uint32 magicWord = DDS::MakeFourCC('D', 'D', 'S', ' ');
	actualWriteSize  = fwrite(&magicWord, 1, sizeof(magicWord), file);
	MESSENGER_REJECT(actualWriteSize != sizeof(magicWord), ("WriteDdsHeader(): only %u out of %u bytes were written\n", actualWriteSize, sizeof(magicWord)));

	//-- build the header
	DDS::DDS_HEADER  ddsHeader;
	memset(&ddsHeader, 0, sizeof(ddsHeader));

	ddsHeader.dwSize              = sizeof(ddsHeader);
	ddsHeader.dwHeaderFlags       = DDS::DDS_HEADER_FLAGS_TEXTURE | DDS::DDS_HEADER_FLAGS_MIPMAP | DDS::DDS_HEADER_FLAGS_LINEARSIZE;
	ddsHeader.dwWidth             = static_cast<DWORD>(pixelWidth);
	ddsHeader.dwHeight            = static_cast<DWORD>(pixelHeight);
	ddsHeader.dwSurfaceFlags      = DDS::DDS_SURFACE_FLAGS_TEXTURE | DDS::DDS_SURFACE_FLAGS_MIPMAP;
	ddsHeader.dwMipMapCount       = static_cast<DWORD>(mipmapLevelCount);
	ddsHeader.dwPitchOrLinearSize = static_cast<DWORD>(pixelWidth);

	switch(ddsDataFormat)
	{
		case SU_DXT1:
			ddsHeader.ddspf = DDS::DDSPF_DXT1;
			break;

		case SU_DXT3:
			ddsHeader.ddspf = DDS::DDSPF_DXT3;
			break;

		case SU_DXT5:
			ddsHeader.ddspf = DDS::DDSPF_DXT5;
			break;

		case SU_RGBA:
			ddsHeader.ddspf = DDS::DDSPF_A8R8G8B8;
			break;

		default:
			MESSENGER_LOG_ERROR(("unexpected dds data format [%d]", ddsDataFormat));
			return false;
	}

	//write the DDS header
	actualWriteSize = fwrite(&ddsHeader, 1, sizeof(ddsHeader), file);
	MESSENGER_REJECT(actualWriteSize != sizeof(ddsHeader), ("WriteDdsHeader(): only %u out of %u bytes were written\n", actualWriteSize, sizeof(ddsHeader)));

	//-- no errors
	return true;
}

// ----------------------------------------------------------------------

bool MayaUtility::exportTexture(const Image *image, const char *writePathname, int &mipmapLevelsExported, bool compress)
{
	NOT_NULL(image);
	NOT_NULL(writePathname);

	MESSENGER_INDENT;

	//-- make sure we're 4 bytes per pixel
	const int bytesPerPixel = image->getBytesPerPixel();
	MESSENGER_REJECT((bytesPerPixel != 3) && (bytesPerPixel != 4), ("MayaUtility::exportTexture() only supports source images with 4 bytes per pixel, current=%d\n", bytesPerPixel));

	//-- setup formats used by dds compressor
	const Image::PixelFormat pixelFormat = image->getPixelFormat();
	uint                     ddsColorFormat;
	uint                     ddsDataFormat;

	switch (pixelFormat)
	{
		case Image::PF_bgra_8888:
			ddsColorFormat = SU_BGRA;
			if(!compress)
				ddsDataFormat = SU_RGBA;
			else
				ddsDataFormat = SU_DXT5;
			break;

		case Image::PF_bgr_888:
			ddsColorFormat = SU_BGR;
			if(!compress)
				ddsDataFormat = SU_RGB;
			else
				ddsDataFormat = SU_DXT1;
			break;

		case Image::PF_rgba_8888:
			ddsColorFormat = SU_RGBA;
			if(!compress)
				ddsDataFormat = SU_RGBA;
			else
				ddsDataFormat = SU_DXT5;
			break;

		case Image::PF_rgb_888:
			ddsColorFormat = SU_RGB;
			if(!compress)
				ddsDataFormat = SU_RGB;
			else
				ddsDataFormat = SU_DXT1;
			break;

		case Image::PF_abgr_8888:
		case Image::PF_argb_8888:
		case Image::PF_nonStandard:
		default:
			// -TRF- in these cases, could convert image to a supported format
			MESSENGER_LOG_ERROR(("image format not supported by dds code\n"));
			return false;
	}

	//-- compress it

	// create scratch buffer for image data
	const int compressBufferSize = image->getBufferSize() * 4;
	std::vector<uint8> compressedData(static_cast<size_t>(compressBufferSize));

	uint8 *currentCompressBuffer   = &compressedData[0];
	int    totalCompressedDataSize = 0;
	int    mipmapLevelCount        = 0;

	std::vector<Image*>  imagesToDelete;
	const Image         *conversionImage = image;

	do
	{
		const uint8 *const sourceImageData = conversionImage->lockReadOnly();

		int levelCompressedSize = 0;
		if(compress)
		{
			//-- convert image data to dds format
			levelCompressedSize = AtiDXTCCompressRGB4(const_cast<uint8*>(sourceImageData), conversionImage->getWidth(), conversionImage->getHeight(), ddsColorFormat, ddsDataFormat, reinterpret_cast<uint16*>(currentCompressBuffer)); //lint !e826 // suspicious pointer-to-pointer conversion (area too small) // @todo look into this more
			MESSENGER_REJECT(levelCompressedSize < 0, ("AtiDXTCCompressRGB4() failed\n"));
		}
		else
		{
			int memToCopy = conversionImage->getWidth() * conversionImage->getHeight() * 4;
			memcpy(currentCompressBuffer, sourceImageData, static_cast<size_t>(memToCopy));
			levelCompressedSize = memToCopy;
		}

		conversionImage->unlock();

		totalCompressedDataSize += levelCompressedSize;
		currentCompressBuffer   += levelCompressedSize;

		++mipmapLevelCount;

		//-- build next mipmap
		int newWidth  = conversionImage->getWidth() / 2;
		int newHeight = conversionImage->getHeight() / 2;

		if ((newWidth < 1) && (newHeight < 1))
		{
			// we're done
			conversionImage = 0;
		}
		else
		{
			// Make sure width and height are at least 1.
			newWidth  = std::max(1, newWidth);
			newHeight = std::max(1, newHeight);

			//-- create the new image, add to deletion list
			Image *newImage = new Image();
			imagesToDelete.push_back(newImage);

			newImage->setDimensions(newWidth, newHeight, conversionImage->getBitsPerPixel(), conversionImage->getBytesPerPixel());
			newImage->setPixelInformation(conversionImage->getRedMask(), conversionImage->getGreenMask(), conversionImage->getBlueMask(), conversionImage->getAlphaMask());

			//-- create the mipmap level
			ImageManipulation::generateNextSmallerMipmap(*conversionImage, *newImage);

			//-- set conversion image to this new image for next loop
			conversionImage = newImage;
		}
	} 
	while (conversionImage);

	// delete mipmap level images
	std::for_each(imagesToDelete.begin(), imagesToDelete.end(), PointerDeleter());

	// open the file
	FILE *f = fopen(writePathname, "wb");
	MESSENGER_REJECT(!f, ("failed to open file [%s] for writing\n", writePathname));

	// write the data
	const bool headerWriteSuccess = WriteDdsHeader(f, image->getWidth(), image->getHeight(), ddsDataFormat, mipmapLevelCount);
	const size_t writeResult      = fwrite(&(compressedData[0]), 1, static_cast<size_t>(totalCompressedDataSize), f);

	// wrap up
	fclose(f);
	MESSENGER_REJECT(!headerWriteSuccess, ("failed to write dds header for file [%s]\n", writePathname));
	MESSENGER_REJECT(writeResult != static_cast<size_t>(totalCompressedDataSize), ("failed to write texture [%s], only [%u of %d] bytes written\n", writePathname, writeResult, totalCompressedDataSize));

	//-- no errors
	mipmapLevelsExported = mipmapLevelCount;

	return true;
}


bool MayaUtility::exportCompressedNormalMap(const Image *image, const char *writePathname)
{
	NOT_NULL(image);
	NOT_NULL(writePathname);

	MESSENGER_INDENT;

	//-- make sure we're 4 bytes per pixel
	const int bytesPerPixel = image->getBytesPerPixel();
	MESSENGER_REJECT((bytesPerPixel != 4), ("MayaUtility::exportCompressedNormalMap() only supports source images with 4 bytes per pixel, current=%d\n", bytesPerPixel));


	//-- compress it

	// create scratch buffer for image data
	const int compressBufferSize = image->getBufferSize() * 4;
	std::vector<uint8> compressedData(static_cast<size_t>(compressBufferSize));

	uint8 *currentCompressBuffer   = &compressedData[0];	
	int totalCompressedDataSize = 0;
	int mipmapLevelCount = 0;


	std::vector<Image*>  imagesToDelete;
	const Image *conversionImage = image;

	do
	{
		const uint8 *const sourceImageData = conversionImage->lockReadOnly();

		// source texture
		ATI_TC_Texture srcTexture;
		srcTexture.dwSize = sizeof(srcTexture);
		srcTexture.dwWidth = conversionImage->getWidth();
		srcTexture.dwHeight = conversionImage->getHeight();
		srcTexture.dwPitch = conversionImage->getWidth() * 4;
		srcTexture.format = ATI_TC_FORMAT_ARGB_8888;
		srcTexture.dwDataSize = ATI_TC_CalculateBufferSize(&srcTexture);
		srcTexture.pData = (ATI_TC_BYTE*) sourceImageData;

		// dest texture
		ATI_TC_Texture destTexture;
		destTexture.dwSize = sizeof(destTexture);
		destTexture.dwWidth = conversionImage->getWidth();
		destTexture.dwHeight = conversionImage->getHeight();
		destTexture.dwPitch = 0;
		destTexture.format = ATI_TC_FORMAT_ATI2N_DXT5;
		destTexture.dwDataSize = ATI_TC_CalculateBufferSize(&destTexture);
		destTexture.pData = (ATI_TC_BYTE*) currentCompressBuffer;

		// compress
		ATI_TC_ERROR res = ATI_TC_ConvertTexture(&srcTexture, &destTexture, NULL, NULL, NULL, NULL);

		conversionImage->unlock();

		switch(res)
		{
		case ATI_TC_OK:
			//MESSENGER_MESSAGE_BOX(NULL,"ATI_TC_ConvertTexture returned : ATI_TC_OK","debug",MB_OK);
			break;
		case ATI_TC_ABORTED:
			MESSENGER_MESSAGE_BOX(NULL,"ATI_TC_ConvertTexture returned : ATI_TC_ABORTED","nmap compress error",MB_OK);
			return false;
		case ATI_TC_ERR_INVALID_SOURCE_TEXTURE:
			MESSENGER_MESSAGE_BOX(NULL,"ATI_TC_ConvertTexture returned : ATI_TC_ERR_INVALID_SOURCE_TEXTURE","nmap compress error",MB_OK);
			return false;
		case ATI_TC_ERR_INVALID_DEST_TEXTURE:
			MESSENGER_MESSAGE_BOX(NULL,"ATI_TC_ConvertTexture returned : ATI_TC_ERR_INVALID_DEST_TEXTURE","nmap compress error",MB_OK);
			return false;
		case ATI_TC_ERR_UNSUPPORTED_SOURCE_FORMAT:
			MESSENGER_MESSAGE_BOX(NULL,"ATI_TC_ConvertTexture returned : ATI_TC_ERR_UNSUPPORTED_SOURCE_FORMAT","nmap compress error",MB_OK);
			return false;
		case ATI_TC_ERR_UNSUPPORTED_DEST_FORMAT:
			MESSENGER_MESSAGE_BOX(NULL,"ATI_TC_ConvertTexture returned : ATI_TC_ERR_UNSUPPORTED_DEST_FORMAT","nmap compress error",MB_OK);
			return false;
		case ATI_TC_ERR_SIZE_MISMATCH:
			MESSENGER_MESSAGE_BOX(NULL,"ATI_TC_ConvertTexture returned : ATI_TC_ERR_SIZE_MISMATCH","nmap compress error",MB_OK);
			return false;
		case ATI_TC_ERR_UNABLE_TO_INIT_CODEC:
			MESSENGER_MESSAGE_BOX(NULL,"ATI_TC_ConvertTexture returned : ATI_TC_ERR_UNABLE_TO_INIT_CODEC","nmap compress error",MB_OK);
			return false;
		case ATI_TC_ERR_GENERIC:
			MESSENGER_MESSAGE_BOX(NULL,"ATI_TC_ConvertTexture returned : ATI_TC_ERR_GENERIC","nmap compress error",MB_OK);
			return false;
		default:
			MESSENGER_MESSAGE_BOX(NULL,"ATI_TC_ConvertTexture returned : UNKNOWN","nmap compress error",MB_OK);
			return false;
		}

		totalCompressedDataSize += destTexture.dwDataSize;
		currentCompressBuffer += destTexture.dwDataSize;
		++mipmapLevelCount;

		// build the next mapmap
		int newWidth  = conversionImage->getWidth() / 2;
		int newHeight = conversionImage->getHeight() / 2;

		if ((newWidth < 1) && (newHeight < 1))
		{
			// we're done
			conversionImage = 0;
		}
		else
		{
			// Make sure width and height are at least 1.
			newWidth  = std::max(1, newWidth);
			newHeight = std::max(1, newHeight);

			//-- create the new image, add to deletion list
			Image *newImage = new Image();
			imagesToDelete.push_back(newImage);

			newImage->setDimensions(newWidth, newHeight, conversionImage->getBitsPerPixel(), conversionImage->getBytesPerPixel());
			newImage->setPixelInformation(conversionImage->getRedMask(), conversionImage->getGreenMask(), conversionImage->getBlueMask(), conversionImage->getAlphaMask());

			//-- create the mipmap level
			ImageManipulation::generateNextSmallerMipmap(*conversionImage, *newImage);

			//-- set conversion image to this new image for next loop
			conversionImage = newImage;
		}
	} 
	while (conversionImage);

	// delete mipmap level images
	std::for_each(imagesToDelete.begin(), imagesToDelete.end(), PointerDeleter());

	// open the file
	FILE *f = fopen(writePathname, "wb");
	MESSENGER_REJECT(!f, ("failed to open file [%s] for writing\n", writePathname));

	// write the data
	uint ddsDataFormat = SU_DXT5;
	const bool headerWriteSuccess = WriteDdsHeader(f, image->getWidth(), image->getHeight(), ddsDataFormat, mipmapLevelCount);
	const size_t writeResult      = fwrite(&(compressedData[0]), 1, static_cast<size_t>(totalCompressedDataSize), f);

	// wrap up
	fclose(f);
	MESSENGER_REJECT(!headerWriteSuccess, ("failed to write dds header for file [%s]\n", writePathname));
	MESSENGER_REJECT(writeResult != static_cast<size_t>(totalCompressedDataSize), ("failed to write texture [%s], only [%u of %d] bytes written\n", writePathname, writeResult, totalCompressedDataSize));

	return true;
}


// ----------------------------------------------------------------------
/**
 * Return the Maya objects, usually textures, attached to the input
 * color attributes of a Maya layered texture node.
 *
 * It is valid to return successfully and have no attached textures.
 * This can happen if the user hasn't attached any nodes to the inputs
 * attribute.
 *
 * @param layeredTextureNode  an MFnDependencyNode instance associated with
 *                            the layered texture node under consideration.
 * @param attachedTextures    returns the MObjects for nodes attached to
 *                            the color attribute of the layered texture 
 *                            inputs attribute.
 *
 * @return  true if the function completed successfully; false if an error occurred.
 */

bool MayaUtility::getAttachedTextureObjects(const MFnDependencyNode &layeredTextureNode, MObjectArray &attachedTextures)
{
	MStatus status;

	//-- Get the inputs attribute node as an array plug.
	MPlug inputArrayPlug = layeredTextureNode.findPlug(cms_layeredTextureInputArrayAttributeName, &status);
	STATUS_REJECT(status, "layeredTextureNode.findPlug()");

	//-- Iterate over all array elements.  Each element is a compound plug containing a color compound value and an alpha value.
	//   This code will only look at the color compound value.
	const unsigned inputElementCount = inputArrayPlug.evaluateNumElements(&status);
	STATUS_REJECT(status, "inputArrayPlug.evaluateNumElements()");

	MPlug       inputElement;
	MPlug       colorPlug;
	MPlugArray  texturePlugArray;

	for (unsigned elementIndex = 0; elementIndex < inputElementCount; ++elementIndex)
	{
		//-- Get the plug for the array element.
		inputElement = inputArrayPlug.elementByPhysicalIndex(elementIndex, &status);
		STATUS_REJECT(status, "inputArrayPlug.elementByPhysicalIndex()");

		//-- Get the first child of the element (a compound plug with children as color compound and alpha value).
		colorPlug = inputElement.child(0, &status);
		STATUS_REJECT(status, "inputElement.child()");

		//-- Get the source plugs that plug into the input color attribute.
		const bool isConnected = colorPlug.connectedTo(texturePlugArray, true, false, &status);
		STATUS_REJECT(status, "colorPlug.connectedTo()");

		if (isConnected)
		{
			const unsigned plugArrayLength = texturePlugArray.length();
			MESSENGER_REJECT(plugArrayLength != 1, ("layered texture [%s], attribute [inputs] physical index [%u] has invalid attached object count [%u].\n", layeredTextureNode.name().asChar(), elementIndex, plugArrayLength));

			const MPlug &texturePlug = texturePlugArray[0];
			if (texturePlug.isNetworked())
			{
				//-- Add MObject instance for attached node.
				IGNORE_RETURN(attachedTextures.append(texturePlug.node()));
			}
		}
	}

	//-- Return no errors.  It is possible that there are no attached textures.
	return true;
}

// ----------------------------------------------------------------------

bool MayaUtility::generateTextures(
	const TextureSet *referencedTextures, 
	const FileCrcMap *existingCrcMap, 
	const char       *textureWriteDir, 
	FileCrcMap       *newlyExportedCrcMap, 
	bool              forceWrite
	)
{
	NOT_NULL(referencedTextures);
	NOT_NULL(textureWriteDir);

	MESSENGER_LOG(("- - - - - [TEXTURE GENERATION] - - - - -\n"));
	MESSENGER_INDENT;

	bool allTexturesGenerated = true;

	//-- walk through each texture
	const TextureSet::const_iterator itEnd = referencedTextures->end();
	for (TextureSet::const_iterator it = referencedTextures->begin(); it != itEnd; ++it)
	{
		const std::string &sourceTexturePathname = it->first;

		CompressType compressType = it->second;

		if (!TreeFile::exists(sourceTexturePathname.c_str()))
		{
			MESSENGER_LOG_ERROR(("source texture %s does not exist, skipping\n", sourceTexturePathname.c_str()));
			allTexturesGenerated = false;
			continue;
		}

		//-- create destination texture file name
		// strip off directory and extension of texture
		char baseName[MAX_PATH];
		const bool stripResult = stripBaseName(sourceTexturePathname.c_str(), baseName, sizeof(baseName), '/');
		MESSENGER_REJECT(!stripResult, ("stripBaseName() failed on [%s]\n", sourceTexturePathname.c_str()));

		// construct destination texture filename
		std::string destinationTexturePathname = textureWriteDir;

		const size_t destDirLength = destinationTexturePathname.length();
		if (destDirLength && destinationTexturePathname[destDirLength-1] != '\\')
			destinationTexturePathname.push_back('\\');

		destinationTexturePathname += baseName;
		destinationTexturePathname += ".dds";

		//-- check if dest texture filename exists
		const bool destTextureExists = TreeFile::exists(destinationTexturePathname.c_str());

		//-- calculate crc for source image
		MESSENGER_LOG(("processing source texture [%s]\n", sourceTexturePathname.c_str()));
		MESSENGER_INDENT;

		MESSENGER_LOG(("loading..."));

		std::auto_ptr<Image> image(ImageFormatList::loadImage(sourceTexturePathname.c_str()));
		if (image.get())
		{
			MESSENGER_LOG(("--success\n"));
		}
		else
		{
			MESSENGER_LOG(("--failed, skipping file\n"));
			allTexturesGenerated = false;
			continue;
		}

		//-- verify source texture is a power-of-two texture in each dimension and does not exceed 1024 in any dimension
		MESSENGER_LOG(("verifying source texture dimensions..."));
		const int sourceWidth  = image->getWidth();
		const int sourceHeight = image->getHeight();
		const int maxDimension = 1024;

		if (!IsPowerOfTwo(sourceWidth) || !IsPowerOfTwo(sourceHeight))
		{
			MESSENGER_LOG_ERROR(("--failed, dimensions are not a power of two (%d x %d)\n", sourceWidth, sourceHeight));
			allTexturesGenerated = false;
			continue;
		}

		if (std::max(sourceWidth, sourceHeight) > maxDimension)
		{
			MESSENGER_LOG_ERROR(("--failed, dimensions (%d x %d) larger than max dimension %d\n", sourceWidth, sourceHeight, maxDimension));
			allTexturesGenerated = false;
			continue;
		}

		MESSENGER_LOG(("--success (%d x %d)\n", sourceWidth, sourceHeight));

		//-- calculate source crc
		MESSENGER_LOG(("calculating crc..."));
		const uint32 currentSourceCrc = image->calculateCrc();
		MESSENGER_LOG(("--done.\n"));

		//-- figure out whether we should export the texture
		bool doTextureExport;

		MESSENGER_LOG(("checking if export necessary..."));
		if (!destTextureExists)
		{
			// definitely have to write as dest texture doesn't exist
			doTextureExport = true;
			MESSENGER_LOG(("--yes, dest texture [%s] doesn't exist.\n", destinationTexturePathname.c_str()));
		}
		else
		{
			// dest file exists
			if (forceWrite)
			{
				// caller is requesting we always export
				doTextureExport = true;
				MESSENGER_LOG(("--yes, caller is forcing a write\n"));
			}
			else
			{
				if (!existingCrcMap)
				{
					// no way to check whether source image has changed since last export.  safest thing to do is export.
					doTextureExport = true;
					MESSENGER_LOG(("--yes, no crc data available\n"));
				}
				else
				{
					// get the crc from last export of texture
					const FileCrcMap::const_iterator crcIt = existingCrcMap->find(sourceTexturePathname);
					if (crcIt == existingCrcMap->end())
					{
						// we don't have crc data on the image, safest thing to do is export it
						doTextureExport = true;
						MESSENGER_LOG(("--yes, crc data is available but not for this file\n"));
					}
					else
					{
						// check current source image Crc against stored crc.
						const uint32 exportedSourceCrc = (*crcIt).second;
						doTextureExport = (exportedSourceCrc != currentSourceCrc);
						if (doTextureExport)
						{
							MESSENGER_LOG(("--yes, crc check indicates source texture changed\n"));
						}
						else
						{
							MESSENGER_LOG(("--no, crc check indicates source image hasn't changed\n"));
						}
					}
				}
			}
		}

		//-- export the texture
		if (!doTextureExport)
		{
			MESSENGER_LOG(("texture doesn't require export.\n"));
		}
		else
		{
			//-- export the texture
			int mipmapLevelCount = 0;

			MESSENGER_LOG(("exporting to [%s]...", destinationTexturePathname.c_str()));


			bool exportSuccess;
			if (compressType == CT_normalMap) // this is going to be a compressed normal map
			{
				exportSuccess = exportCompressedNormalMap(image.get(), destinationTexturePathname.c_str());
			}
			else // this is going to be an uncompressed normal map/texture or a compressed texture
			{
				bool compress = (compressType == CT_texture);
				exportSuccess = exportTexture(image.get(), destinationTexturePathname.c_str(), mipmapLevelCount, compress);
			}
			

			if (exportSuccess)
			{
				MESSENGER_LOG(("success (%d mipmap levels).\n", mipmapLevelCount));
			}
			else
			{
				MESSENGER_LOG(("failed.\n"));
				return false;
			}

			//-- save the new crc
			if (newlyExportedCrcMap)
				IGNORE_RETURN( newlyExportedCrcMap->insert(FileCrcMap::value_type(sourceTexturePathname, currentSourceCrc)) );
		}
	}

	return allTexturesGenerated;
}

// ----------------------------------------------------------------------
/**
 * Pull the Tag name from a given attribute string name (ex. soe_materialAmbient_ENVM)
 */

std::string MayaUtility::getTagFromAttributeName(const std::string& attributeName, const std::string& prefix)
{
	std::string::size_type pos = attributeName.find(prefix);
	MESSENGER_REJECT(pos == std::string::npos,("bad string format in textureName_<TAG> attribute"));
	return attributeName.substr(pos + prefix.size());
}

// ----------------------------------------------------------------------
/**
 * Convert a 4 character string into a Tag
 */
Tag MayaUtility::convertStringtoTag(const std::string& str)
{
	DEBUG_FATAL(str.size() < 4,("tag string not long enough"));
	return (str[0] << 24) |(str[1] << 16) |(str[2] << 8) |(str[3] << 0);
}

bool MayaUtility::getChannelTextures(
	MayaUtility::TextureSet    &o_textures,
	const MObject              &shaderGroupObject, 
	const std::string          &channelName,
	CompressType                compressType
	)
{
	MESSENGER_REJECT (shaderGroupObject.apiType () != MFn::kShadingEngine, ("buildTexture () expecting object of type MFn::kShaderEngine, found type %s\n", shaderGroupObject.apiTypeStr ()));

	//get the shader node attached to the shader group
	bool               result;
	MStatus            status;
	MObjectArray       objectArray;
	MFnDependencyNode  fnDepNode (shaderGroupObject, &status);
	MESSENGER_REJECT (!status, ("failed to set SG object for fnDepNode,\"%s\"\n", status.errorString ().asChar ()));

	result = MayaUtility::findSourceObjects (fnDepNode, "surfaceShader", &objectArray);
	MESSENGER_REJECT (!result, ("failed to get shader connected to shader group \"%s\"\n", fnDepNode.name ().asChar ()));
	MESSENGER_REJECT (objectArray.length () != 1, ("expecting surfaceShader to have one upstream connection, found %d\n", objectArray.length ()));

	//find texture attached to shader
	status = fnDepNode.setObject (objectArray [0]);
	MESSENGER_REJECT (!status, ("failed to set shader object for fnDepNode,\"%s\"\n", status.errorString ().asChar ()));

	result = MayaUtility::findSourceObjects (fnDepNode, channelName.c_str(), &objectArray);
	MESSENGER_REJECT (!result, ("failed to get texture files connected to shader\"%s\"\n", fnDepNode.name ().asChar ()));
	if (objectArray.length () < 1)
	{
		//no textures
		return false;
	}
	MESSENGER_REJECT (objectArray.length () > 1, ("expecting surfaceShader to have one upstream connection, found %d\n", objectArray.length ()));

	//set dependency node to the texture node
	status = fnDepNode.setObject (objectArray [0]);
	MESSENGER_REJECT (!status, ("failed to set file texture for fnDepNode,\"%s\"\n", status.errorString ().asChar ()));

	// fnDepNode now points to a "file" node.

	//track textures that are intended to be rendered via TextureRenderers.  these texture DG nodes end in _TR
	MString mayaNodeName = fnDepNode.name(&status);
	MESSENGER_REJECT(!status, ("failed to get node name for texture node\n"));
	IGNORE_RETURN(mayaNodeName.toLowerCase());
	const std::string  fullNodeName = mayaNodeName.asChar();
	const size_t       stringLength = fullNodeName.length();
	const bool isTextureRenderer    = (stringLength > 3) && (fullNodeName[stringLength-3] == '_') && (fullNodeName[stringLength-2] == 't') && (fullNodeName[stringLength-1] == 'r');

	if (objectArray [0].apiType() == MFn::kFileTexture)
	{
		// ----------

		bool wrapU;
		bool wrapV;
		
		result = MayaUtility::getNodeBoolValue(fnDepNode,"wrapU",&wrapU);
		MESSENGER_REJECT (!result, ("Failed to get texture wrap U"));
		result = MayaUtility::getNodeBoolValue(fnDepNode,"wrapV",&wrapV);
		MESSENGER_REJECT (!result, ("Failed to get texture wrap V"));

		// ----------

		//get texture name
		MObject      fileNameAttr = fnDepNode.attribute ("fileTextureName");
		const MPlug  plugToFileName (objectArray [0], fileNameAttr); 
		MObject      valueObject;

		status = plugToFileName.getValue (valueObject);
		MESSENGER_REJECT (!status, ("failed to get value object for fileTextureName,\"%s\"\n", status.errorString ().asChar ()));

		MFnStringData fnStringData (valueObject, &status);
		MESSENGER_REJECT (!status, ("failed to set filename string object for fnStringData,\"%s\"\n", status.errorString ().asChar ()));

		MString mFilename = fnStringData.string();
		IGNORE_RETURN(mFilename.toLowerCase());
		std::string filename = mFilename.asChar();

		//-- Remove Maya project '//' from path name.
		const std::string::size_type projectStartPos = filename.find("//");
		if (static_cast<int>(projectStartPos) != static_cast<int>(std::string::npos))
		{
			// Remove the first '/'.
			filename.erase(projectStartPos, 1);
			MESSENGER_LOG_WARNING(("texture file [%s] contained a Maya project-relative '//', please fix.\n", filename.c_str()));
		}

		//convert the texture from a fully-pathed file to texture\<filename>.dds
		/*
		char textureBaseName[128];
		bool result;
		result = MayaUtility::stripBaseName(filename.c_str(), textureBaseName, sizeof (textureBaseName), '/');
		MESSENGER_REJECT (!result, ("failed to strip base name from shader group texture filename \"%s\"\n", filename.c_str()));
		std::string textureRelativeName =  textureReferenceDirectory;
			         textureRelativeName += textureBaseName;
			         textureRelativeName += ".dds";
		*/

		// only add the texture if it's not a texture renderer (a texture reference defined in maya, but the file is built later)
		if (isTextureRenderer)
		{
			MESSENGER_LOG (("%s is a texture renderer, skipping...\n", fullNodeName.c_str()));
		}
		else
		{
			o_textures.insert(std::make_pair(filename, compressType));
		}
	}

	return !o_textures.empty();
}

bool MayaUtility::getNormalMapSourceFilename(
	std::string         &o_name, 
	const MObject       &shaderGroupObject
	)
{
	MESSENGER_REJECT(shaderGroupObject.apiType() != MFn::kShadingEngine,("getShaderGroupMaterial() expecting object of type MFn::kShaderEngine, found type %s\n", shaderGroupObject.apiTypeStr()));

	MStatus           status;
	MFnAttribute      attribute;
	MObject           attributeObject;
	MPlug             plug;
	MObjectArray      objectArray;

	MFnDependencyNode shaderDependencyNode(shaderGroupObject, &status);	MESSENGER_REJECT(!status,("failed to set SG object for shaderDependencyNode,\"%s\"\n", status.errorString().asChar()));

	bool result = MayaUtility::findSourceObjects(shaderDependencyNode, "surfaceShader", &objectArray);	MESSENGER_REJECT(!result,("failed to get shader connected to shader group \"%s\"\n", shaderDependencyNode.name().asChar()));
																																		MESSENGER_REJECT(objectArray.length() != 1,("expecting surfaceShader to have one upstream connection, found %d\n", objectArray.length()));
	MFnLambertShader lambert(objectArray[0], &status);							MESSENGER_REJECT(!status,("failed to set lambert shader object for shaderDependencyNode,\"%s\"\n", status.errorString().asChar()));

	//get all the attributes, and for each one, build the appropriate part of the shader template
	const unsigned attributeCount = lambert.attributeCount(&status);		MESSENGER_REJECT(!status,("failed to get attribute count for shader attributes\n"));
	for(unsigned i = 0; i < attributeCount; ++i)
	{
		//get the attribute
		attributeObject = lambert.attribute(i, &status);						MESSENGER_REJECT(!status,("failed to get attribute [%u]\n", i));
		status = attribute.setObject(attributeObject);							MESSENGER_REJECT(!status,("failed to set object into MFnAttribute\n"));
		std::string attributeName =(attribute.name(&status)).asChar();		MESSENGER_REJECT(!status,("failed to get attribute name\n"));

		//is it one of our attributes?
		if (attributeName.find(ms_attributePrefix) != std::string::npos)
		{
			MFnPhongShader phong(objectArray[0], &status);							MESSENGER_REJECT(!status,("failed to set phong shader object for shaderDependencyNode,\"%s\"\n", status.errorString().asChar()));
			const MObject phongObject = phong.object();
			MFnDependencyNode materialDependencyNode(phongObject, &status);	MESSENGER_REJECT(!status,("failed to set material object for shaderDependencyNode,\"%s\"\n", status.errorString().asChar()));
			// get the value for this attribute
			plug = materialDependencyNode.findPlug(attributeObject, &status);	MESSENGER_REJECT_WARNING(!status,("failed to find plug for attribute [%s]\n", attributeName.c_str()));

			if (attributeName.find("textureName") != std::string::npos)
			{
				const std::string prefix = ms_attributePrefix + "textureName_";
				const std::string tagStr = getTagFromAttributeName(attributeName, prefix);
				Tag tag = convertStringtoTag(tagStr);
				MString value;
				status = plug.getValue(value);

				CompressType compressType = CT_texture;

				bool isNormalMap=false;
				//if we have a normal map (tag NRML) then do NOT compress the texture
				if (tag == TAG(N,R,M,L))
				{
					compressType = CT_none;
					isNormalMap=true;
				}
				else if (tag == TAG(C,N,R,M)) // do compress normal maps w/tag CNMR
				{
					compressType = CT_normalMap;
					isNormalMap=true;
				}

				if (!status) // no plug
				{
					if (isNormalMap)
					{
						TextureSet textures;

						if (getChannelTextures(textures, shaderGroupObject, attributeName, compressType))
						{
							if (!textures.empty())
							{
								TextureSet::iterator tsi = textures.begin();
								o_name = tsi->first;

								return true;
							}
						}
					}
				}
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------

float MayaUtility::getShaderHeightMapScale(const MObject &shaderGroupObject)
{
	MayaShaderReader sr(shaderGroupObject);
	float ret_val;
	if (sr.getFloatAttribute(ret_val, "soe_heightMapScale"))
	{
		return ret_val;
	}
	else
	{
		return 1;
	}
}

// ----------------------------------------------------------------------
/**
 *
 * Determine if the specified mesh has color and alpha data set.
 *
 * The color check will test if there is any non-zero color data
 * set for the mesh.  It will independently check if there is non-one
 * alpha data set.  Thus, it is possible for an artist to color the alpha
 * for transparency effect and not touch the vertex colors.  In this case,
 * retrieving the Maya color will give the artist-specified alpha combined
 * with a bogus zero color.
 *
 * @arg fnMesh             the Maya mesh to check
 * @arg mayaShaderIndex    set to the mesh shader index for which we'll test polygons.  if -1, test all polygons.
 * @arg hasVertexColorSet  this routine sets this to true if there was any set vertex color data that was non-zero
 * @arg hasVertexAlphaSet  this routine sets this to true if there was any set vertex alpha data that was non-one
 */

bool MayaUtility::getMeshColorAlphaStatus(const MFnMesh &fnMesh, int mayaShaderIndex, const MIntArray &polyShaderAssignment, bool *hasVertexColorSet, bool *hasVertexAlphaSet)
{
	NOT_NULL(hasVertexColorSet);
	NOT_NULL(hasVertexAlphaSet);

	MESSENGER_INDENT;
	MStatus status;

	*hasVertexColorSet = false;
	*hasVertexAlphaSet = false;

	MObject meshObject = fnMesh.object(&status);
	MESSENGER_REJECT(!status, ("MFnMesh::object() failed\n"));

	MItMeshPolygon itPoly(meshObject, &status);
	MESSENGER_REJECT(!status, ("failed to create MItMeshPolygon for shape\n"));

	bool isDone = itPoly.isDone(&status);
	MESSENGER_REJECT(!status, ("itPoly.isDone() failed\n"));

	int     polygonIndex = 0;
	MColor  mayaColor;

	const float colorEpsilon = 1.0f/257.0f;
	const float alphaEpsilon = colorEpsilon;

	while (!isDone)
	{
		// check if this polygon belongs to the target Maya shader.  If not, skip
		if ((mayaShaderIndex == -1) || (mayaShaderIndex == polyShaderAssignment[static_cast<unsigned>(polygonIndex)]))
		{

			//-- process the polygon
			const long polyVertexCount = itPoly.polygonVertexCount(&status);
			MESSENGER_REJECT(!status, ("itPoly.polygonVertexCount() failed\n"));

			for (int vertexIndex = 0; vertexIndex < static_cast<int>(polyVertexCount); ++vertexIndex)
			{
				//-- check vertex color
				const bool hasColor = itPoly.hasColor(vertexIndex, &status);
				MESSENGER_REJECT(!status, ("MItMeshPolygon::hasColor() failed\n"));

				if (hasColor)
				{
					// get the color
					status = itPoly.getColor(mayaColor, vertexIndex);
					MESSENGER_REJECT(!status, ("MItMeshPolygon::getColor() failed\n"));

					// examine color values.  only check if we don't yet know whether we have vertex color.
					if (!*hasVertexColorSet)
					{
						// assume color values don't go negative.
						// we're looking for any non-zero color.
						if ((mayaColor.r >= colorEpsilon) || (mayaColor.g >= colorEpsilon) || (mayaColor.b >= colorEpsilon))
						{
							// this vertex has non-zero user-set color
							*hasVertexColorSet = true;
							if (*hasVertexAlphaSet)
							{
								// we're done, they're both set somewhere
								return true;
							}
						}
					}

					// examine alpha values
					if (!*hasVertexAlphaSet)
					{
						// we're looking for any non-one alpha
						const real da = 1.0f - mayaColor.a;
						if ((da > alphaEpsilon) || (da < -alphaEpsilon))
						{
							// this vertex has non-one user-set alpha
							*hasVertexAlphaSet = true;
							if (*hasVertexColorSet)
							{
								// we're done, they're both set somewhere
								return true;
							}
						}
					}
				}
			}
		}

		//-- increment the loop
		status = itPoly.next();
		MESSENGER_REJECT(!status, ("itPoly.next() failed\n"));

		isDone = itPoly.isDone(&status);
		MESSENGER_REJECT(!status, ("itPoly.isDone() failed\n"));

		++polygonIndex;
	}

	// we're done
	return true;
}

// ----------------------------------------------------------------------

static bool DuplicateNameCheckHelper(const MDagPath &checkNodeDagPath, MStringArray *duplicateNames, std::set<std::string> *nameSet, bool checkTypeMask, MFn::Type compatibleFnTypeMask)
{
	NOT_NULL(duplicateNames);
	NOT_NULL(nameSet);

	MStatus status;

	if (!MayaUtility::ignoreNode(checkNodeDagPath))
	{
		//-- get dag path node
		MObject checkNode = checkNodeDagPath.node(&status);
		MESSENGER_REJECT(!status, ("MDagPath::node() failed\n"));

		//-- check if object is compatible with mask
		if (checkTypeMask)
		{
			if (!checkNode.hasFn(compatibleFnTypeMask))
			{
				// ignore this and its children
				return true;
			}
		}

		//-- get name of dag node
		MFnDagNode fnDagNode(checkNode, &status);
		MESSENGER_REJECT(!status, ("failed to set MFnDagNode for [%s]\n", checkNodeDagPath.partialPathName().asChar()));

		const MString mayaNodeName = fnDagNode.name(&status);
		MESSENGER_REJECT(!status, ("failed to get dag node name\n"));

		const MayaCompoundString mayaNodeNameCompound(mayaNodeName);
		MESSENGER_REJECT(mayaNodeNameCompound.getComponentCount() < 1, ("yikes, no components for DAG node [%s]\n", checkNodeDagPath.partialPathName().asChar()));

		MString gameNodeName = mayaNodeNameCompound.getComponentString(0);
		IGNORE_RETURN(gameNodeName.toLowerCase());

		//-- Skip character system mesh-relative hardpoint names.
		if ((_stricmp(gameNodeName.asChar(), "hp") == 0) && (mayaNodeNameCompound.getComponentCount() > 1))
			return true;

		//-- check if short name is a duplicate
		std::pair<std::set<std::string>::iterator, bool> insertResult = nameSet->insert(std::string(gameNodeName.asChar()));
		if (!insertResult.second)
		{
			//-- failed to add string to set; therefore, there's another short name that is a duplicate of this one
			IGNORE_RETURN(duplicateNames->append(gameNodeName));
		}
	}

	//-- handle node's children
	const unsigned childCount = checkNodeDagPath.childCount(&status);
	MESSENGER_REJECT(!status, ("failed to get child count for DAG node\n"));

	for (unsigned i = 0; i < childCount; ++i)
	{
		// get the object
		MObject childObject = checkNodeDagPath.child(i, &status);
		MESSENGER_REJECT(!status, ("failed to get child index [%u]\n", i));

		// turn it into a dag node so we can get a dag path to it
		MFnDagNode  fnChildDagNode(childObject, &status);
		MESSENGER_REJECT(!status, ("failed to set MFnDagNode for [%s]'s child node, it's a [%s]\n", checkNodeDagPath.partialPathName().asChar(), childObject.apiTypeStr()));

		MDagPath childDagPath;
		status = fnChildDagNode.getPath(childDagPath);
		MESSENGER_REJECT(!status, ("failed to get a dag path to child object\n"));

		const bool childSuccess = DuplicateNameCheckHelper(childDagPath, duplicateNames, nameSet, checkTypeMask, compatibleFnTypeMask);
		MESSENGER_REJECT(!childSuccess, ("failed to add child joint\n"));
	}

	//-- no errors
	return true;
}

// ----------------------------------------------------------------------
/**
 *
 * Check for duplicate DAG node short names including all nodes in the specified hierarchy.
 */

bool MayaUtility::checkForDuplicateShortNames(const MDagPath &rootDagPath, MStringArray *duplicateNames)
{
	NOT_NULL(duplicateNames);

	MESSENGER_INDENT;

	std::set<std::string> nameSet;

	const bool dnchSuccess = DuplicateNameCheckHelper(rootDagPath, duplicateNames, &nameSet, false, MFn::kInvalid);
	return dnchSuccess;
}

// ----------------------------------------------------------------------
/**
 *
 * Check for duplicate DAG node short names in the specified hierarchy for nodes of a given
 * Maya function set compatibility.
 *
 *  NOTE: -TRF- why is this an int rather than a proper MFn::Type?  My MSVC compiler would internal error (VC6,SP5,P3/P4 asm extensions added)
 *  with inclusion of "maya/MFn.h" in header.
 *
 *  If a given DAG node is not compatible with the specified function set, the search ignores that DAG node and all of its children.
 *
 * @arg  rootDagPath           Root of the DAG hierarchy to check for duplicate names.
 * @arg  duplicateNames        The list of short names in duplicate are added to this list.  will be empty if no duplicate names.
 * @arg  compatibleFnTypeMask  MFn::Type of compatible function set for the node types we're interested in checking, converted to int.
 *
 * @return  true on successful operation, false if an error occured
 */

bool MayaUtility::checkForDuplicateShortNames(const MDagPath &rootDagPath, MStringArray *duplicateNames, int compatibleFnTypeMask)
{
	MESSENGER_INDENT;

	std::set<std::string> nameSet;

	const bool dnchSuccess = DuplicateNameCheckHelper(rootDagPath, duplicateNames, &nameSet, true, static_cast<MFn::Type>(compatibleFnTypeMask));
	return dnchSuccess;
}

// ----------------------------------------------------------------------
/**
 *
 * Search up the DAG for an ancestor of the specified DAG node with a MayaCompoundString node name containing
 * a value for given component index.
 *
 * The DAG is searched, starting from the specified dag path.  The search may need to go no further than
 * the specified dag node to find a node with a node name containing the specified component.
 *
 * One use of this is to find the skeleton template name associated with a given joint in the joint hierarchy.
 *
 * @param ancestorObject  the object for the anscestor (or dagPath) that matched the search criteria.
 */

bool MayaUtility::findAncestorWithNameComponent(const MDagPath &dagPath, int componentIndex, MString *componentValue, bool *foundIt, MString *ancestorNodeName, MDagPath *ancestorDagPath)
{
	NOT_NULL(foundIt);

	MESSENGER_INDENT;
	MStatus status;

	MESSENGER_REJECT(componentIndex < 0, ("invalid component index [%d]\n", componentIndex));

	bool     localFoundIt = false;
	bool     done         = false;
	MDagPath testDagPath  = dagPath;

	do
	{
		if (!ignoreNode(testDagPath))
		{
			//-- get name for dag node
			MObject object = testDagPath.node(&status);
			MESSENGER_REJECT(!status, ("failed to get dag node\n"));

			MFnDagNode fnDagNode(object, &status);
			MESSENGER_REJECT(!status, ("failed to set MFnDagNode\n"));

			MayaCompoundString compoundName(fnDagNode.name(&status));
			MESSENGER_REJECT(!status, ("failed to get name for dag node\n"));

			//-- check if compound name has specified component
			const int componentCount = compoundName.getComponentCount();
			if (componentCount > componentIndex)
			{
				// we've got it
				NOT_NULL(componentValue);
				*componentValue   = compoundName.getComponentString(componentIndex);
				localFoundIt      = true;
				done              = true;

				if (ancestorNodeName)
					*ancestorNodeName = compoundName.getCompoundString();

				if (ancestorDagPath)
					*ancestorDagPath = testDagPath;
			}
		}

		if (!done)
		{
			// didn't find it, check node's parent
			const unsigned int popsLeft = testDagPath.length(&status);
			MESSENGER_REJECT(!status, ("MDagPath::length() failed\n"));

			if (!popsLeft)
			{
				// no parents left to check.  we did not find a node with specified name component.
				done = true;
			}
			else
			{
				// move up the dag one node (to parent)
				status = testDagPath.pop(1);
				MESSENGER_REJECT(!status, ("MDagPath::pop() failed [%s]\n", status.errorString().asChar()));
			}
		}
	} while (!done);

	*foundIt = localFoundIt;

	// no errors
	return true;
}

// ----------------------------------------------------------------------
/**
 *
 * creates a directory, creating any required parent directories as necessary.
 */

bool MayaUtility::createDirectory(const char *directory)
{
	//-- construct list of subdirectories all the way down to root
	std::stack<std::string> directoryStack;

	std::string currentDirectory = directory;

	// build the stack
	while (!currentDirectory.empty())
	{
		// remove trailing backslash
		if (currentDirectory[currentDirectory.size()-1] == '\\')
			IGNORE_RETURN(currentDirectory.erase(currentDirectory.size()-1));

		if (currentDirectory[currentDirectory.size()-1] == ':')
		{
			// we've hit something like c:
			break;
		}

		if (!currentDirectory.empty())
			directoryStack.push(currentDirectory);

		// now strip off current directory
		size_t previousDirIndex = currentDirectory.rfind('\\');
		if (static_cast<int>(previousDirIndex) == currentDirectory.npos)
			break;
		else
			IGNORE_RETURN(currentDirectory.erase(previousDirIndex));
	}

	//-- build all directories specified by the initial directory
	while (!directoryStack.empty())
	{
		// get the directory
		currentDirectory = directoryStack.top();
		directoryStack.pop();

		// try to create it (don't pass any security attributes)
		const BOOL result = CreateDirectory(currentDirectory.c_str(), NULL);
		if (result)
			MESSENGER_LOG(("created directory [%s]\n", currentDirectory.c_str()));
	}

	return true;
}

// ----------------------------------------------------------------------
/**
 * Returns whether the specified node should be ignored.
 *
 * If the specified node has an "ignore" string as a component of the
 * name, the node will be ignored.  If there is any error processing
 * the node, the node will be set as if it specified "ignore".
 *
 * e.g. rhand_locator__ignore <= this node will be ignored.
 */

bool MayaUtility::ignoreNode(const MDagPath &dagPath)
{
	//-- get name for dag node
	MStatus status;
	MObject object = dagPath.node(&status);
	if (!status)
		return true;

	MFnDagNode fnDagNode(object, &status);
	if (!status)
		return true;

	MayaCompoundString compoundName(fnDagNode.name(&status));
	if (!status)
		return true;

	//-- check if compound name has specified component
	const int componentCount = compoundName.getComponentCount();
	for (int i = 1; i < componentCount; ++i)
	{
		MString component = compoundName.getComponentString(i);
		if (_stricmp(component.asChar(), "ignore") == 0)
		{
			// we should ignore
			return true;
		}
	}

	// do not ignore it
	return false;
}

// ----------------------------------------------------------------------

std::string MayaUtility::createCrcPathname()
{
	std::string crcPathname;

	//-- get directory name of current maya file
	// grab maya path name
	const MString mayaCurrentPathName = MFileIO::currentFile();
	std::string   currentPathName(mayaCurrentPathName.asChar());

	// convert forward slash to back slash
	{
		const size_t nameLength = currentPathName.length();
		for (size_t i = 0; i < nameLength; ++i)
		if (currentPathName[i] == '/')
			currentPathName[i] = '\\';
	}

	// strip down to directory name
	const size_t endOfDir = currentPathName.rfind('\\');
	if (static_cast<int>(endOfDir) == currentPathName.npos)
		currentPathName = "";
	else
		IGNORE_RETURN(currentPathName.erase(endOfDir + 1));

	//-- add filename
	currentPathName += "MayaExporterTextureCrcs.iff";
	return currentPathName;
}

// ----------------------------------------------------------------------

void MayaUtility::loadFileCrcMap(FileCrcMap *existingCrcMap, const TextureSet &referencedTextures)
{
	NOT_NULL(existingCrcMap);
	MESSENGER_INDENT;

	if (referencedTextures.size() < 1)
	{
		// nothing to do here
		return;
	}

	//-- construct the pathname for the crc file
	// source texture used to sample a reasonable directory comes from the first referenced texture
	const std::string crcPathname = createCrcPathname();

	//-- open the iff
	if (!TreeFile::exists(crcPathname.c_str()))
	{
		MESSENGER_LOG(("could not find crc texture file [%s]\n", crcPathname.c_str()));
		return;
	}

	//-- load the data
	Iff iff(crcPathname.c_str());

	iff.enterForm(TAG(F,C,R,C));
		iff.enterForm(TAG(0,0,0,1));
			iff.enterChunk(TAG(D,A,T,A));
			{
				while (iff.getChunkLengthLeft())
				{
					// load pathname
					char texturePathname[MAX_PATH];
					iff.read_string(texturePathname, sizeof(texturePathname)-1);

					// load crc
					const uint32 textureCrc = iff.read_uint32();

					// add to map
					(*existingCrcMap)[texturePathname] = textureCrc;
				}
			}
			iff.exitChunk(TAG(D,A,T,A));
		iff.exitForm(TAG(0,0,0,1));
	iff.exitForm(TAG(F,C,R,C));

	MESSENGER_LOG(("loaded texture CRCs from [%s]\n", crcPathname.c_str()));
}

// ----------------------------------------------------------------------

void MayaUtility::saveFileCrcMap(const FileCrcMap &existingCrcMap, const FileCrcMap &revisedCrcMap, const TextureSet &referencedTextures)
{
	MESSENGER_INDENT;

	if (referencedTextures.size() < 1)
	{
		// nothing to do here
		return;
	}

	//-- construct the merged crc map.
	// start with existing crc map, then add
	MayaUtility::FileCrcMap writeMap(existingCrcMap);

	{
		// add in revised crc map entries, replacing existing entries as necessary
		MayaUtility::FileCrcMap::const_iterator       sourceIt    = revisedCrcMap.begin();
		const MayaUtility::FileCrcMap::const_iterator sourceItEnd = revisedCrcMap.end();
		for (; sourceIt != sourceItEnd; ++sourceIt)
		{
			const std::string &texturePathname = (*sourceIt).first;
			const uint32       textureCrc      = (*sourceIt).second;

			writeMap[texturePathname] = textureCrc;
		}
	}

	//-- construct the pathname for the crc file
	// source texture used to sample a reasonable directory comes from the first referenced texture
	const std::string crcPathname = createCrcPathname();

	//-- save the data
	Iff iff(ms_crcFileIffSize);

	iff.insertForm(TAG(F,C,R,C));
		iff.insertForm(TAG(0,0,0,1));
			iff.insertChunk(TAG(D,A,T,A));
			{
				MayaUtility::FileCrcMap::const_iterator       writeIt    = writeMap.begin();
				const MayaUtility::FileCrcMap::const_iterator writeItEnd = writeMap.end();
				for (; writeIt != writeItEnd; ++writeIt)
				{
					const std::string &texturePathname = (*writeIt).first;
					const uint32       textureCrc      = (*writeIt).second;

					iff.insertChunkString(texturePathname.c_str());
					iff.insertChunkData(textureCrc);
				}
			}
			iff.exitChunk(TAG(D,A,T,A));
		iff.exitForm(TAG(0,0,0,1));
	iff.exitForm(TAG(F,C,R,C));

	//-- write to a file
	const bool writeSuccess = iff.write(crcPathname.c_str(), true);
	if (!writeSuccess)
	{
		MESSENGER_LOG(("failed to write crc texture file [%s]\n", crcPathname.c_str()));
		return;
	}

	MESSENGER_LOG(("saved texture CRCs to [%s]\n", crcPathname.c_str()));
}

// ----------------------------------------------------------------------
/**
 * Go to the bind pose through the Maya bind-pose node (if present) or
 * through the specified bind pose frame number.
 */

bool MayaUtility::goToBindPose(int alternateBindPoseFrameNumber)
{
	//-- always go to the alternate bind pose frame number first.
	//   Note: this fixes undesirable Maya behavior where the
	//   sequence:
	//     (1) disable all nodes, 
	//     (2) go to bind pose
	//     (3) re-enable all nodes
	//   would cause some bones not to go back to the proper
	//   location.

	//-- go to the frame where the artist set the skeleton to the bind pose
	// set the maya frame number to the bind pose frame
	MStatus  status;
	MTime    mayaFrameTime;

	status = mayaFrameTime.setValue(static_cast<double>(alternateBindPoseFrameNumber));
	MESSENGER_REJECT(!status, ("mayaFrameTime.setValue(%.2f) failed\n", static_cast<double>(alternateBindPoseFrameNumber)));

	status = MAnimControl::setCurrentTime(mayaFrameTime);
	MESSENGER_REJECT(!status, ("MAnimControl.setCurrentTime() failed\n"));

	//-- run the "disable all nodes" command
	status = MGlobal::executeCommand(MString(ms_commandDisableAllNodes), true, true);
	if (status)
	{
		//-- run the "go to bind pose" command
		status = MGlobal::executeCommand(MString(ms_commandGoToBindPose), true, true);
		if (!status)
		{
			MESSENGER_LOG(("MEL command to go to bind pose failed (see Maya display)\n"));

			// enable deformers: we disabled them to go to the real bind pose, but the real
			// bind pose data is not present.  We want to fall back to the frame -10 non-authoratative 
			// bind pose, so re-enable deformers which may have been used for the pose.
			IGNORE_RETURN(enableDeformers());
		}
	}
	else
	{
		MESSENGER_LOG(("MEL command to disable all nodes failed (see Maya display)\n"));
	}

	return true;
}

// ----------------------------------------------------------------------
/**
 * Enable all deformers within the scene.
 *
 * When going to bind pose, all deformers must be disabled.  This
 * function re-enables the deformers so proper animation playback
 * can occur.
 */

bool MayaUtility::enableDeformers()
{
	//-- Run the "enable all nodes" command
	MStatus status = MGlobal::executeCommand(MString(ms_commandEnableAllNodes), true, true);
	MESSENGER_REJECT(!status, ("MEL command to enable all nodes failed (see Maya display)\n"));

	//-- Run the "dgdirty -a" command to fix the Maya bug where constrained connections are not hooked up properly.
	status = MGlobal::executeCommand(ms_commandDgDirty, true, true);
	MESSENGER_REJECT(!status, ("MEL command [%s] failed (see Maya display)\n", ms_commandDgDirty.asChar()));

	return true;
}

// ----------------------------------------------------------------------

bool MayaUtility::getSkeletonRootDagPath(MDagPath *skeletonRootDagPath)
{
	NOT_NULL(skeletonRootDagPath);

	//-- get the active selection list.
	MSelectionList  activeSelectionList;
	MStatus status = MGlobal::getActiveSelectionList(activeSelectionList);
	MESSENGER_REJECT(!status, ("MGlobal::getActiveSelectionList() failed\n"));

	//-- this function only works if a single object is selected.
	MESSENGER_REJECT(activeSelectionList.length() != 1, ("error: just one object should be selected, currently %u are selected\n", activeSelectionList.length()));

	//-- get dag path for selected item
	MDagPath selectedDagPath;
	status = activeSelectionList.getDagPath(0, selectedDagPath);
	MESSENGER_REJECT(!status, ("Failed to get dag path for selected item.  Is it a dag node?\n"));

	//-- find parent with skeleton name specified
	MString  skeletonTemplateName;
	bool     foundIt = false;

	const bool faResult = findAncestorWithNameComponent(selectedDagPath, 1, &skeletonTemplateName, &foundIt, 0, skeletonRootDagPath);
	MESSENGER_REJECT(!faResult, ("findAncestorWithNameComponent failed\n"));
	MESSENGER_REJECT(!foundIt, ("failed to find skeleton name for skeleton containing selected node [%s]\n", selectedDagPath.partialPathName().asChar()));

	//-- no errors
	return true;
}

// ----------------------------------------------------------------------

bool MayaUtility::isInSet(const MDagPath &path, const char * name)
{
	MObjectArray sets;

	MSelectionList object;
	IGNORE_RETURN(object.add(path));

	MStatus status = MGlobal::getAssociatedSets(object, sets);
	if (status.error()) return false;

	int x, l;
	l = static_cast<int>(sets.length());
	for (x=0; x<l; ++x)
	{
		const MFnSet set(sets[static_cast<unsigned int>(x)], &status);
		if (!status.error())
		{
			MString setName = set.name();
			//messenger->logMessage("Set named %s\n", setName.asChar());
			if (setName == name) return true;
		}
	}
	return false;
}

// ----------------------------------------------------------------------
/**
 * Use this function to clear out an MObject variable for which the Maya API
 * somehow fails to keep a proper reference count.
 *
 * @param object  the Maya object that is causing the application to crash
 *                when destructed.  this appears to be a sign of Maya failing
 *                to track reference counts to objects properly.
 */

void MayaUtility::clearMayaObjectHack(MObject *object)
{
	if (object)
	{
		// clear out the pointer to the maya object implementation (PIMPL)
		*reinterpret_cast<unsigned long*>(object) = 0;
	}
}

// ----------------------------------------------------------------------

void MayaUtility::dumpDependencyNodeAttributes(const MObject &object)
{
	MESSENGER_INDENT;
	MStatus  status;

	MFnDependencyNode  fnDependencyNode(object, &status);
	if (!status)
	{
		MESSENGER_LOG_ERROR(("failed to create dependency node object [%s]\n", status.errorString().asChar()));
		return;
	}

	const unsigned attributeCount = fnDependencyNode.attributeCount(&status);
	if (!status)
	{
		MESSENGER_LOG_ERROR(("failed to get attribute count for dep node [%s]\n", status.errorString().asChar()));
		return;
	}

	REPORT_LOG(true, ("Attributes for node [%s]:\n", fnDependencyNode.name().asChar()));

	MFnAttribute  fnAttribute;
	for (unsigned i = 0; i < attributeCount; ++i)
	{
		MObject attributeObject = fnDependencyNode.attribute(i, &status);
		if (!status)
		{
			MESSENGER_LOG_ERROR(("failed to get attribute object [%s]\n", status.errorString().asChar()));
			return;
		}

		status = fnAttribute.setObject(attributeObject);
		if (!status)
		{
			MESSENGER_LOG_ERROR(("failed to set MFnAttribute [%s]\n", status.errorString().asChar()));
			return;
		}

		//-- print attribute name and type
		const MString attributeName = fnAttribute.name(&status);
		if (!status)
		{
			MESSENGER_LOG_ERROR(("failed to get attribute name [%s]\n", status.errorString().asChar()));
			return;
		}

		REPORT_LOG(true, ("-attribute name [%s], type [%s]\n", attributeName.asChar(), attributeObject.apiTypeStr()));
	}
}

// ----------------------------------------------------------------------

bool shaderHasTextureAlpha ( const MObject & shaderObject )
{
	bool hasAlpha = false;

	// check if shader group's shader has anything attached to its transparency attribute.  If so, assume we have texture alpha.
	MStatus            status;

	// get the shader node
	MFnDependencyNode  fnDepNode (shaderObject, &status);
	MESSENGER_REJECT (!status, ("failed to assign shader object to fnDepNode,\"%s\"\n", status.errorString ().asChar ()));

	// check if it has transparency attached
	const MPlug transparencyPlug = fnDepNode.findPlug ("transparency", &status);
	MESSENGER_REJECT (!status, ("failed to find shader transparency plug,\"%s\"\n", status.errorString ().asChar ()));

	// effect has alpha (texture alpha) if transparency plug is attached (i.e. is set to some value)
	hasAlpha = transparencyPlug.isNetworked (&status);
	MESSENGER_REJECT (!status, ("failed to determine if transparency plug is networked,\"%s\"\n", status.errorString ().asChar ()));

	return hasAlpha;
}

bool shaderGroupHasTextureAlpha( const MObject &shaderGroupObject )
{
	// check if shader group's shader has anything attached to its transparency attribute.  If so, assume we have texture alpha.
	MStatus            status;
	MObjectArray       objectArray;

	// get the shader node
	MFnDependencyNode  fnDepNode (shaderGroupObject, &status);
	MESSENGER_REJECT (!status, ("failed to assign shader group object to fnDepNode,\"%s\"\n", status.errorString ().asChar ()));

	bool result = MayaUtility::findSourceObjects (fnDepNode, "surfaceShader", &objectArray);
	MESSENGER_REJECT (!result, ("failed to find upstream object plugged into shader group's surfaceShader attribute.\n"));
	MESSENGER_REJECT (objectArray.length () != 1, ("expecting to find one surface attached to SG surfaceShader, found %d instead.\n", objectArray.length ()));

	const MObject & shaderObject = objectArray[0];

	return shaderHasTextureAlpha(shaderObject);
}

bool shaderHasEffectAlpha( const MObject & shaderObject )
{
	MStatus           status;

	MFnLambertShader fnLambert(shaderObject, &status);
	MESSENGER_REJECT(!status,("failed to set lambert shader object for shaderDependencyNode,\"%s\"\n", status.errorString().asChar()));

	//get all the attributes, and for each one, build the appropriate part of the shader template
	const unsigned attributeCount = fnLambert.attributeCount(&status);
	MESSENGER_REJECT(!status,("failed to get attribute count for shader attributes\n"));

	for(unsigned i = 0; i < attributeCount; ++i)
	{
		// ----------
		// If the attribute name does not contain "effectName", skip it

		MObject attributeObject = fnLambert.attribute(i, &status);
		MESSENGER_REJECT(!status,("failed to get attribute [%u]\n", i));
	
		MFnAttribute fnAttribute(attributeObject,&status);
		MESSENGER_REJECT(!status,("failed to set object into MFnAttribute\n"));

		std::string attributeName =(fnAttribute.name(&status)).asChar();
		MESSENGER_REJECT(!status,("failed to get attribute name\n"));

		if(attributeName.find("effectName") == std::string::npos) //lint !e737 !e650 std::string::npos isn't same signage as std::string's find(), sigh
		{
			continue;
		}

		// ----------
		// The attribute name contains "effectName", so get the plug for the attribute and see
		// if its value contains "alpha"

		MFnDependencyNode fnDepNode(shaderObject, &status);
		MESSENGER_REJECT(!status,("failed to set dependency node,\"%s\"\n", status.errorString().asChar()));

		MPlug plug = fnDepNode.findPlug(attributeObject, &status);
		MESSENGER_REJECT(!status,("failed to find plug for attribute [%s]\n", attributeName.c_str()));

		MString plugValue;
		status = plug.getValue(plugValue);
		MESSENGER_REJECT(!status,("failed to get effect namefor attribute [%s]\n", attributeName.c_str()));

		// ----------

		std::string effectName = plugValue.asChar();

		if(effectName.find("alpha") != std::string::npos) //lint !e737 !e650 std::string::npos isn't same signage as std::string's find(), sigh
		{
			// Found an effect that uses alpha
			return true;
		}
	}

	return false;
}


bool shaderGroupHasEffectAlpha( const MObject & shaderGroupObject )
{
	MESSENGER_REJECT(shaderGroupObject.apiType() != MFn::kShadingEngine,("getShaderGroupMaterial() expecting object of type MFn::kShaderEngine, found type %s\n", shaderGroupObject.apiTypeStr()));

	// ----------	

	MStatus           status;

	MFnDependencyNode shaderDependencyNode(shaderGroupObject, &status);
	MESSENGER_REJECT(!status,("failed to set SG object for shaderDependencyNode,\"%s\"\n", status.errorString().asChar()));

	MObjectArray objectArray;
	bool result = MayaUtility::findSourceObjects(shaderDependencyNode, "surfaceShader", &objectArray);
	MESSENGER_REJECT(!result,("failed to get shader connected to shader group \"%s\"\n", shaderDependencyNode.name().asChar()));
	MESSENGER_REJECT(objectArray.length() != 1,("expecting surfaceShader to have one upstream connection, found %d\n", objectArray.length()));

	// ----------

	MObject shaderObject = objectArray[0];

	return shaderHasEffectAlpha(shaderObject);
}






const float alphaEpsilon = 1.0f/257.0f;

bool epsilonEqual( float value, float target, float epsilon )
{
	float delta = value - target;

	if(delta > epsilon) return false;
	if(delta < -epsilon) return false;

	return true;
}


bool meshHasVertexAlpha(const MFnMesh &fnMesh)
{
	MStatus status;

	MObject meshObject = fnMesh.object(&status);
	MESSENGER_REJECT(!status, ("MFnMesh::object() failed\n"));

	MItMeshPolygon itPoly(meshObject, &status);
	MESSENGER_REJECT(!status, ("failed to create MItMeshPolygon for shape\n"));

	bool isDone = itPoly.isDone(&status);
	MESSENGER_REJECT(!status, ("itPoly.isDone() failed\n"));

	while (!isDone)
	{
		//-- process the polygon
		const int polyVertexCount = static_cast<int>(itPoly.polygonVertexCount(&status));
		MESSENGER_REJECT(!status, ("itPoly.polygonVertexCount() failed\n"));

		for (int i = 0; i < polyVertexCount; ++i)
		{
			//-- check vertex color
			const bool hasColor = itPoly.hasColor(i, &status);
			MESSENGER_REJECT(!status, ("MItMeshPolygon::hasColor() failed\n"));

			if (!hasColor) continue;

			// ----------
			// get the color

			MColor  mayaColor;
			status = itPoly.getColor(mayaColor, i);
			MESSENGER_REJECT(!status, ("MItMeshPolygon::getColor() failed\n"));

			if(!epsilonEqual( mayaColor.a, 1.0f, alphaEpsilon ))
			{
				return true;
			}
		}

		//-- increment the loop
		status = itPoly.next();
		MESSENGER_REJECT(!status, ("itPoly.next() failed\n"));

		isDone = itPoly.isDone(&status);
		MESSENGER_REJECT(!status, ("itPoly.isDone() failed\n"));

	}

	// we're done
	return false;
}


bool MayaUtility::meshHasAlpha ( MFnMesh const & fnMesh )
{
	if( meshHasVertexAlpha(fnMesh) ) return true;

	// ----------
	
	MStatus            status;
	MObjectArray       shaderGroupArray;
	MIntArray          shaderGroupMap;

	status = fnMesh.getConnectedShaders(0, shaderGroupArray, shaderGroupMap);
	MESSENGER_REJECT( !status, ("failed to get shaders connected to the mesh\n"));

	int nShaderGroups = static_cast<int>(shaderGroupArray.length());

	for(int i = 0; i < nShaderGroups; i++)
	{
		MObject & shaderGroup = shaderGroupArray[static_cast<unsigned int>(i)];

		if( shaderGroupHasTextureAlpha(shaderGroup) ) return true;

		if( shaderGroupHasEffectAlpha(shaderGroup) ) return true;
	}

	// ----------

	return false;
}

// ----------------------------------------------------------------------
/**
 * Retrieve dag paths for the shape nodes associated with any Mesh nodes
 * in the given selection list.
 *
 * This function will check each node in selectionList.  If the node is a
 * Mesh node (NOT a shape node), the first child shape node associated
 * with Mesh parent will be added.  If the node in the selectionList is
 * already a shape node (unlikely but possible), the shape node will be
 * added.  If the item is neither a Mesh node or a Shape node, the selection
 * list item is ignored.
 *
 * It is not an error for there to be no shape nodes returned, nor is it an
 * error for there to be no mesh/shape nodes in the given selectionList.
 *
 * This function has been modified to work with transform nodes that are
 * LOD grouping nodes.  If a node is an LOD grouping node, the shape DagPath
 * returned is the shape of the first LOD detail level, assumed to be named
 * l0.  Other detail level shapes are ignored.
 *
 * @param selectionList  the list of nodes to check for mesh shape nodes.
 * @param dagPaths       the dag paths of all shape nodes are returned here.
 *
 * @return  true if the operation succeeded without failure, false otherwise.
 */

bool MayaUtility::getSelectionListMeshShapes(const MSelectionList &selectionList, DagPathVector &dagPaths)
{
	MStatus  status;
	MDagPath sourceDagPath;
	MDagPath meshDagPath;

	//-- setup return vector
	const unsigned int sourceDagNodeCount = selectionList.length(&status);
	STATUS_REJECT(status, "selectionList.length()");

	dagPaths.clear();
	dagPaths.reserve(sourceDagNodeCount);

	for (unsigned int sourceIndex = 0; sourceIndex < sourceDagNodeCount; ++sourceIndex)
	{
		//-- Get the dag node under consideration.
		status = selectionList.getDagPath(sourceIndex, sourceDagPath);
		if (!status)
		{
			// skip this node, not a dag path
			continue;
		}

		//-- Handle an LOD group node.
		if (sourceDagPath.apiType() == MFn::kLodGroup)
		{
			// Set sourceDagPath to be the dag path of the first child, which should be the l0 transform node.

			//-- Get # detail levels.
			const unsigned int childCount = sourceDagPath.childCount(&status);
			STATUS_REJECT(status, "sourceDagPath.childCount()");

			if (childCount < 1)
			{
				//-- LOD node doesn't have any children, skip it.
				continue;
			}

			//-- Set sourceDagPath to dag path of first child.  This should be the l0 transform node.
			status = sourceDagPath.push(sourceDagPath.child(0));
			STATUS_REJECT(status, "sourceDagPath.push()");
		}

		//-- Skip if this node and its child are not compatible with kMesh.
		const bool isSourceOrChildMesh = sourceDagPath.hasFn(MFn::kMesh, &status);
		STATUS_REJECT(status, "sourceDagPath.hasFn()");

		if (!isSourceOrChildMesh)
			continue;
		
		//-- if this node is a mesh, add its dag path to the return vector
		const MFn::Type sourceApiType = sourceDagPath.apiType(&status);
		STATUS_REJECT(status, "sourceDagPath.apiType()");

		if (sourceApiType == MFn::kMesh)
			dagPaths.push_back(sourceDagPath);
		else
		{
			//-- source node isn't a mesh, but at least one of its children is.
			const unsigned int childCount = sourceDagPath.childCount(&status);
			STATUS_REJECT(status, "sourceDagPath.childCount()");

			//find the *first* mesh child (and break out of for loop)
			for (unsigned int childIndex = 0; childIndex < childCount; ++childIndex)
			{
				//-- get the child object
				MObject childObject = sourceDagPath.child(childIndex, &status);
				STATUS_REJECT(status, "sourceDagPath.child()");

				//-- check if it is a mesh
				const MFn::Type childType = childObject.apiType();
				if (childType == MFn::kMesh)
				{
					// turn it into a dag node so we can get a dag path to it
					MFnDagNode fnDagNode(childObject, &status);
					STATUS_REJECT(status, "fnDagNode constructor()");

					status = fnDagNode.getPath(meshDagPath);
					STATUS_REJECT(status, "fnDagNode.getPath()");

					//-- add child mesh shape dag path to return vector
					dagPaths.push_back(meshDagPath);

					break;
				}
			}
		}
	}

	//-- return success
	return true;
}

// ----------------------------------------------------------------------
/**
 * Assign the affectors associated with the given mesh to the MayMeshWeighting
 * helper class.
 *
 * @param meshShapeDagPath  the mesh for which the affector will be added.
 * @param meshWeighting     the MayaMeshWeighting helper class instance to which
 *                          the affector data will be added.
 *
 * @return  true if operation succeeded, false otherwise.
 */

bool MayaUtility::addMeshAffectors(const MDagPath &meshShapeDagPath, MayaMeshWeighting &meshWeighting)
{
	MESSENGER_INDENT;

	MObjectArray skinClusters;
	MStatus      status;

	//-- retrieve skin clusters affecting target mesh
	const bool getScSuccess = MayaMisc::getSceneDeformersAffectingMesh(meshShapeDagPath, MFn::kSkinClusterFilter, &skinClusters);
	MESSENGER_REJECT(!getScSuccess, ("failed to get skin clusters affecting export mesh\n"));

	//-- apply skin clusters to the mesh weighting helper class
	const unsigned skinClusterCount = skinClusters.length();
	if (skinClusterCount == 0)
	{
		// the selected mesh is affected by a single transform

		//-- get transform dag path
		MDagPath parentDagPath = meshShapeDagPath;

		const unsigned int popsLeft = parentDagPath.length(&status);
		MESSENGER_REJECT(!status, ("MDagPath::length() failed\n"));
		MESSENGER_REJECT(popsLeft < 1, ("no parents left for [%s]\n", parentDagPath.partialPathName().asChar()));

		status = parentDagPath.pop(1);
		MESSENGER_REJECT(!status, ("MDagPath::pop() failed\n"));

		//-- announce what we're doing
		MFnDagNode fnDagNode(parentDagPath, &status);
		MESSENGER_REJECT(!status, ("failed to set MFnDagNode for [%s]\n", parentDagPath.partialPathName().asChar()));
		MESSENGER_LOG(("adding full weighting to [%s]\n", fnDagNode.name().asChar()));

		//-- add parent transform to the mesh weighting object
		const bool asaSuccess = meshWeighting.addSingleAffector(parentDagPath);
		MESSENGER_REJECT(!asaSuccess, ("MeshWeighting::addSingleAffector() failed\n"));
	}
	else
	{
		MESSENGER_LOG(("skin clusters affecting the selected mesh [%u]:\n", skinClusterCount));
		for (unsigned i = 0; i < skinClusterCount; ++i)
		{
			MESSENGER_INDENT;

			const MObject     &skinClusterObject = skinClusters[i];
			MFnDependencyNode  dependencyNode(skinClusterObject, &status);

			MESSENGER_LOG(("skin cluster: [%s]\n", dependencyNode.name().asChar()));

			const bool ascSuccess = meshWeighting.addSkinClusterAffector(skinClusterObject);
			MESSENGER_REJECT(!ascSuccess, ("failed to add skin cluster to mesh weighting helper object\n"));
		}
	}

	//-- success
	return true;
}

// ----------------------------------------------------------------------
/**
 * Determine if a given SkeletonTemplate root node is attached to
 * another SkeletonTemplate, and if so, return the name of the transform
 * to which it is attached.
 *
 * If the given transform node has a parent, and if the parent's name is
 * anything other than 'master', the node is considered attached.  The
 * name of the parent then becomes the attachment transform name.
 *
 * @param skeletonRootDagPath      the dag path to the Maya node that is a
 *                                 SkeletonTemplate root node.
 * @param hasAttachmentTransform   upon return, will contain true if
 *                                 the skeleton is attached to another,
 *                                 false otherwise.
 * @param attachmentTransformName  upon return, will contain the name
 *                                 of the Maya node to which the Skeleton
 *                                 Template is attached.
 *
 * @return  true if the operation succeeded, false otherwise.
 */

bool MayaUtility::getAttachmentTransformName(const MDagPath &skeletonRootDagPath, bool &hasAttachmentTransform, std::string &attachmentTransformName)
{
	MStatus  status;

	//-- Check if root has a parent node.
	const unsigned int rootDagLength = skeletonRootDagPath.length(&status);
	STATUS_REJECT(status, "rootDagPath.length()");

	if (rootDagLength < 1)
	{
		// no parents, so not attached
		hasAttachmentTransform = false;
		return true;
	}

	//-- Get parent node name.
	MDagPath parentDagPath = skeletonRootDagPath;
	status = parentDagPath.pop();
	STATUS_REJECT(status, "parentDagPath.pop()");

	MString parentPathName = parentDagPath.partialPathName();
	IGNORE_RETURN(parentPathName.toLowerCase());

	std::string parentNodeName;
	stripDagPathDirectory(std::string(parentPathName.asChar()), parentNodeName);

	//-- Determine if the parent node name matches master or LOD name format.
	if ((parentNodeName == cs_masterName) || doesNameMatchLodFormat(parentNodeName.c_str()))
	{
		// This node is the root of a skeleton hierarchy.
		hasAttachmentTransform = false;
	}
	else
	{
		// This node is not the root of a skeleton hierarchy.
		attachmentTransformName = parentNodeName;
		hasAttachmentTransform  = true;
	}

	//-- return success
	return true;
}

// ----------------------------------------------------------------------
/**
 * Determine if a given name matches the LOD naming convention.
 *
 * This function checks if the lower-case form of name fully matches
 * 'l<number>' (e.g. l0, l1, l2 ... l305, etc.).
 *
 * @param name  the name to consider matching against the LOD format.
 *              This name does not need to be lower case.
 *
 * @return  true if the name matches the LOD naming convention; false otherwise.
 */

bool MayaUtility::doesNameMatchLodFormat(const char *name)
{
	// @todo replace with a regex.
	const size_t length = strlen(name);

	if (length < 2)
	{
		// Cannot be an LOD name.
		return false;
	}

	//-- Ensure first character is an 'l'.
	if (tolower(name[0]) != 'l')
	{
		// Cannot be an LOD node.
		return false;
	}

	//-- Ensure all subsequent characters are numeric.
	for (size_t i = 1; i < length; ++i)
	{
		if (!isdigit(name[i]))
		{
			// Found a non-digit, cannot be an LOD node name.
			return false;
		}
	}

	//-- Must match, return success.
	return true;

}

// ----------------------------------------------------------------------

MObject MayaUtility::getRootMayaObject(MObject const & object, MStatus * const status)
{
	MObject possibleParent = object;
	MObject parent = object;

	for (;;)
	{
		MFnDagNode dagNode(possibleParent, status);
		if (!status)
			return object;

		possibleParent = dagNode.parent(0, status);
		if (!status)
			return object;
		
		if (possibleParent.apiType() != MFn::kWorld)
			parent = possibleParent;
		else
			break;
	}

	return parent;
}

// ======================================================================

