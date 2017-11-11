// ======================================================================
//
// MayaMisc.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef MAYA_MISC_H
#define MAYA_MISC_H

// ======================================================================
// include files

#include "maya/MFn.h"

// ----------------------------------------------------------------------
// forward declarations

class MDagPath;
class Messenger;
class MObjectArray;

// ======================================================================

class MayaMisc
{
public:

	static void install(Messenger *newMessenger);
	static void remove();

	static bool hasNodeTypeInHierarachy(const MDagPath &hierarchyRoot, MFn::Type nodeType);
	static bool getSceneDeformersAffectingMesh(const MDagPath &meshDagPath, MFn::Type deformerType, MObjectArray *deformerObjects);

private:

	static Messenger *messenger;
	static bool       ms_installed;

};

// ======================================================================

#endif
