// ======================================================================
//
// ExportSkeletalAppearanceTemplate.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_ExportSkeletalAppearanceTemplate_H
#define INCLUDED_ExportSkeletalAppearanceTemplate_H

// ======================================================================

#include "maya/MPxCommand.h"

class MArgList;
class MayaMeshWeighting ;
class MDagPath;
class Messenger;
class MStatus;

// ======================================================================

class ExportSkeletalAppearanceTemplate: public MPxCommand
{
public:

	static void install(Messenger *newMessenger);
	static void remove(void);

	static void *creator(void);

public:

	virtual MStatus doIt(const MArgList &args);

private:

	typedef stdvector<MDagPath>::fwd     DagPathVector;
	typedef stdvector<std::string>::fwd  StringVector;
	typedef stdset<std::string>::fwd     StringSet;

private:

	static bool  createFromArgList(const MArgList &argList);
	static bool  createInteractively(bool commitToSourceControl, bool createNewChangelist, bool lock, bool unlock, bool showViewerAfterExport,const std::string & branch);

	static bool  createSatFile(const std::string &outputFileShortName, const StringVector &skeletonReferenceNames, const StringVector &meshGeneratorReferenceNames, const std::string &animationStateGraphReferenceName);
	static bool  getMeshGeneratorReferenceNames(const DagPathVector &shapeDagPaths, StringVector &meshGeneratorReferenceNames);
	static bool  getReferencedSkeletonSegments(const DagPathVector &meshShapeDagPaths, StringVector &skeletonTemplateNames);
	static bool  addRequiredSkeletonTemplateReferenceNames(const MayaMeshWeighting &meshWeighting, StringSet &skeletonTemplateNameSet, StringVector &skeletonTemplateNameVector);
#if 0
	static bool  getUserData(std::string &outputFileShortName, std::string &animationStateGraphTemplateName);
	static int CALLBACK exportSatDialogProc(HWND dialogHandle, UINT message, WPARAM wParam, LPARAM lParam);
#endif
	static bool  exportMeshGeneratorTemplates(const StringVector &meshReferencePathNames, bool commitToSourceControl, bool createNewChangelist, bool isInteractive, bool lock, bool unlock, const std::string & branch);
	static void  getNodeNameFromTemplatePathName(const std::string &templatePathName, std::string &nodeName);

private:

	ExportSkeletalAppearanceTemplate();
	virtual ~ExportSkeletalAppearanceTemplate();

private:

	static Messenger   *messenger;

	static std::string  ms_dialogSatShortName;
	static std::string  ms_dialogAsgTemplateReferenceName;
	static std::string  ms_dialogLastSatPathSelected;
	static bool         ms_useSourceControl;
};

// ======================================================================

#endif
