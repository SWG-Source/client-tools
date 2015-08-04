// ======================================================================
//
// ExportArgs.h
// copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef EXPORT_ARGS_H
#define EXPORT_ARGS_H

#include "maya/MString.h"

// ======================================================================

class ExportArgs
{
public:
	static const MString      cs_allArgName;
	static const MString      cs_animationStateGraphArgName;
	static const MString      cs_authorArgName;
	static const MString      cs_filenameArgName;
	static const MString      cs_frameArgName;
	static const MString      cs_ignoreBlendTargetsArgName;
	static const MString      cs_ignoreShadersArgName;
	static const MString      cs_ignoreTexturesArgName;
	static const MString      cs_interactiveArgName;
	static const MString      cs_lockArgName;
	static const MString      cs_meshGeneratorArgName;
	static const MString      cs_nameArgName;
	static const MString      cs_nodeArgName;
	static const MString      cs_noRevertOnFailArgName;
	static const MString      cs_outputDirArgName;
	static const MString      cs_outputFileNameArgName;
	static const MString      cs_partOfOtherExportArgName;
	static const MString      cs_skeletonArgName;
	static const MString      cs_submitArgName;
	static const MString      cs_unlockArgName;
	static const MString      cs_warningsArgName;
	static const MString      cs_branchArgName;
	static const MString      cs_silentArgName;
	static const MString      cs_showViewerAfterExport;
	static const std::string  cs_currentBranch;
	static const std::string  cs_x1Branch;
	static const std::string  cs_x2Branch;
	static const std::string  cs_noneBranch;
	static const MString      cs_createNewChangelistArgName;
	static const MString      cs_disableCompression;
	static const MString      cs_fixPobCrc;
};

// ======================================================================

#endif
