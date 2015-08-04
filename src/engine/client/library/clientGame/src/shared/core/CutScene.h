// ======================================================================
//
// CutScene.h
// Copyright 2005 Sony Online Entertainment Inc.
// All Rights Reserved
// ======================================================================

#ifndef INCLUDED_CutScene_H
#define INCLUDED_CutScene_H

// ======================================================================

class CutScene
{
public:
	static bool install();

	static const char *lookupCutScene(const char *i_areaName);

	static bool getCutSceneSeen(const char *i_videoName, const char *i_playerFileBaseName);
	static void setCutSceneSeen(const char *i_videoName, const char *i_playerFileBaseName, bool hasSeenIt);
	static void clearAllSeen(const char *i_playerFileBaseName);

	static bool start(const char *i_videoName, bool isPremier=false);
	static void stop();
	static bool isRunning();        // returns true if a cut-scene is playing.
	static bool isRunningPremier(); // returns true if a cut-scene is playing for the first time.
	static bool isFullScreen();
	static void update();
};

// ======================================================================

#endif
