// ======================================================================
//
// ExportStaticMesh.h
// Todd Fiala
//
// copyright 1999, Bootprint Entertainment
//
// ======================================================================

#ifndef EXPORT_STATIC_MESH_H
#define EXPORT_STATIC_MESH_H

// ======================================================================

#include "maya/mDagPath.h"
#include "maya/MPxCommand.h"
#include "maya/MStatus.h"

class Messenger;

class MArgList;

// ======================================================================

class ExportStaticMesh: public MPxCommand
{

public:

	static void *creator (void);

public:

	static void install(Messenger *messenger);
	static void remove(void);
	static bool processArguments(
		const MArgList &args, 
		MDagPath *targetDagPath, 
		bool &interactive, 
		bool &commitToAlienbrain, 
		bool &commitToCurrent, 
		bool &lock, 
		bool &unlock, 
		bool &showViewerAfterExport, 
		MString &branch,
		bool &fixPobCrc
	);

	MStatus doIt(const MArgList &args);

};

// ======================================================================

#endif
