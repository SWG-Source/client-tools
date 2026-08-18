// ======================================================================
//
// ConfigSharedFile.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ConfigSharedFile_H
#define INCLUDED_ConfigSharedFile_H

// ======================================================================

class ConfigSharedFile
{
public:
	static void install();

	static bool        getEnableAsynchronousLoader();
	static int         getAsynchronousLoaderPriority();
	static int         getAsynchronousLoaderCallbacksPerFrame();
	static int         getAsynchronousLoaderCallbackTimeBudgetMs();
	static bool        getValidateIff();
	static bool        getSearchPathNegativeCache();
	static bool        getSearchPathFileManifest();   // 2026-08-15 cold-singles arc: loose-dir file manifest (first-touch probe elimination); see TreeFile_SearchNode.h
	static int         getNumberOfTreeFilePreloads();
	static char const * getTreeFilePreload(int index);
};

// ======================================================================

#endif
