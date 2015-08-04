// ======================================================================
//
// ExportSkeleton.h
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ExportSkeleton_H
#define INCLUDED_ExportSkeleton_H

// ======================================================================

#include "maya/MPxCommand.h"

class CrcLowerString;
class MArgList;
class MDagPath;
class Messenger;
class MStatus;
class MString;
class SkeletonTemplateWriter;
class Transform;

// ======================================================================

class ExportSkeleton: public MPxCommand
{
public:

	static void     install(Messenger *messenger);
	static void     remove();

	static void    *creator();

	static MString  getSkeletonName(const MDagPath &targetDagPath);

public:

	MStatus         doIt(const MArgList &args);

private:

	typedef stdvector<CrcLowerString>::fwd  CrcLowerStringVector;

private:

	static bool     processArguments(
		const MArgList &args, 
		MString *skeletonTemplateDirectory, 
		MString *authorName, 
		int *bindPoseFrameNumber, 
		MDagPath *targetDagPath, 
		bool &interactive, 
		bool &commitToSourceControl, 
		bool &commitToCurrent, 
		bool &lock, 
		bool &unlock, 
		bool &showViewerAfterExport, 
		MString &branch);
	static bool     collectSkeletonNames(const MDagPath &lodGroupDagPath, CrcLowerStringVector &skeletonShortNames);
	static bool     addJointSkeletonNames(const MDagPath &jointPath, CrcLowerStringVector &skeletonShortNames);

	static bool     getLodPath(const MDagPath &lodGroupPath, int lodIndex, MDagPath &lodPath);
	static bool     getSkeletonStartPath(const MDagPath &searchPath, const CrcLowerString &skeletonName, MDagPath &skeletonStartPath);
	static bool     hasDuplicateNames(const MDagPath &hierarchyStartPath);

private:

	ExportSkeleton();

	bool            performSingleSkeletonExport(int bindPoseFrameNumber, const MDagPath &targetDagPath, /*OUT*/ std::string& skeletonName);
	bool            performLodExport(int bindPoseFrameNumber, const MDagPath &targetDagPath, /*OUT*/ std::string& skeletonName);
	bool            addMayaJoint(SkeletonTemplateWriter &writer, const MDagPath &targetDagPath, int parentIndex);

private:

	static const int  cms_iffSize;

private:

	bool              m_printDiagnostics;

private:

	// disabled
	ExportSkeleton(const ExportSkeleton&);
	ExportSkeleton &operator =(const ExportSkeleton&);

};

// ======================================================================

#endif
