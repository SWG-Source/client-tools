// ======================================================================
//
// MayaMisc.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "MayaMisc.h"

#include "maya/MDagPath.h"
#include "maya/MFnDagNode.h"
#include "maya/MFnSkinCluster.h"
#include "maya/MItDependencyNodes.h"
#include "maya/MObjectArray.h"
#include "Messenger.h"

// ======================================================================
// static object storage
// ======================================================================

Messenger *MayaMisc::messenger;
bool       MayaMisc::ms_installed;

// ======================================================================
// static non-member functions
// ======================================================================

static void hasNodeTypeInHierarchyHelper(const MDagPath &dagPath, MFn::Type nodeType, bool *foundIt)
{
	NOT_NULL(foundIt);
	MStatus status;

	//-- check api type of this node
	if (dagPath.apiType() == nodeType)
	{
		*foundIt = true;
		return;
	}

	//-- check children
	const unsigned childCount = dagPath.childCount(&status);
	for (unsigned i = 0; (i < childCount) && !(*foundIt); ++i)
	{
		// get the object
		MObject childObject = dagPath.child(i);

		// turn it into a dag node so we can get a dag path to it
		MFnDagNode  fnDagNode(childObject);

		MDagPath childDagPath;
		status = fnDagNode.getPath(childDagPath);
		if (status)
			hasNodeTypeInHierarchyHelper(childDagPath, nodeType, foundIt);
	}
}

// ======================================================================
// static member functions
// ======================================================================

void MayaMisc::install(Messenger *newMessenger)
{
	DEBUG_FATAL(ms_installed, ("MayaMisc already installed"));
	messenger = newMessenger;

	ms_installed = true;
}

// ----------------------------------------------------------------------

void MayaMisc::remove()
{
	DEBUG_FATAL(!ms_installed, ("MayaMisc not installed"));
	messenger    = 0;
	ms_installed = false;
}

// ----------------------------------------------------------------------

/**
 *
 * Check if a given node type exists somewhere within the DAG hierarchy
 * specified.
 *
 * @arg hierarchyRoot  the root DAG node of the hierachy to check
 * @arg nodeType       the Maya node type to check for
 */

bool MayaMisc::hasNodeTypeInHierarachy(const MDagPath &hierarchyRoot, MFn::Type nodeType)
{
	DEBUG_FATAL(!ms_installed, ("MayaMisc not installed"));

	bool  foundNode = false;

	hasNodeTypeInHierarchyHelper(hierarchyRoot, nodeType, &foundNode);
	return foundNode;
}

// ----------------------------------------------------------------------
/**
 *
 * Return a list of all the deformer nodes that affect a given mesh shape.
 *
 * @arg meshDagPath      Maya DAG path to the mesh shape node for which to find affecting deformers
 * @arg deformerType     The type of deformer to hunt for (e.g. MFn::kSkinClusterFilter, MFn::kBlendShape)
 * @arg deformerObjects  Returns the deformer objects affecting meshDagPath in the MObjectArray
 */

bool MayaMisc::getSceneDeformersAffectingMesh(const MDagPath &meshDagPath, MFn::Type deformerType, MObjectArray *deformerObjects)
{
	DEBUG_FATAL(!ms_installed, ("MayaMisc not installed"));
	NOT_NULL(deformerObjects);

	MESSENGER_INDENT;
	MStatus status;

	const MObject targetMeshObject = meshDagPath.node(&status);
	MESSENGER_REJECT(!status, ("failed to get node from meshDagPath\n"));

	//-- create deformer iterator
	MItDependencyNodes dnIterator(deformerType, &status);
	MESSENGER_REJECT(!status, ("failed to create dependency node iterator for deformer\n"));

	//-- loop over all blend shapes
	MObjectArray  outputGeometry;

	bool stillIterate = !dnIterator.isDone(&status);
	MESSENGER_REJECT(!status, ("dnIterator.isDone() failed\n"));
	while (stillIterate)
	{
		//-- setup the geometry filter functions
		MObject deformerObject = dnIterator.item(&status);
		MESSENGER_REJECT(!status, ("dnIterator.item() iterator function failed\n"));

		MFnGeometryFilter fnGeometryFilter(deformerObject, &status);
		MESSENGER_REJECT(!status, ("failed to initialize MFnGeometryFilter awith object\n"));

		//-- check if this deformer uses the specified mesh as its base shape
		status = fnGeometryFilter.getOutputGeometry(outputGeometry);
		MESSENGER_REJECT(!status, ("failed to get output geometry for deformer\n"));

		const unsigned affectedMeshCount = outputGeometry.length();
		for (unsigned i = 0; i < affectedMeshCount; ++i)
		{
			// get the object
			MObject &affectedObject = outputGeometry[i];

			if (affectedObject == targetMeshObject)
			{
				// found a deformer affecting our mesh
				status = deformerObjects->append(deformerObject);
				MESSENGER_REJECT(!status, ("failed to append object to deformer MObjectArray\n"));
			}
		}

		// clear for next blend shape
		status = outputGeometry.clear();
		MESSENGER_REJECT(!status, ("outputGeometry.clear() failed\n"));

		//-- increment iterator
		status = dnIterator.next();
		MESSENGER_REJECT(!status, ("failed to iterate to next blend shape\n"));

		stillIterate = !dnIterator.isDone(&status);
		MESSENGER_REJECT(!status, ("dnIterator.isDone() failed\n"));
	}

	//-- success
	return true;
}

// ======================================================================
