// ======================================================================
//
// PlaybackScriptTemplateList.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_PlaybackScriptTemplateList_H
#define INCLUDED_PlaybackScriptTemplateList_H

// ======================================================================

class CrcLowerString;
class LessPointerComparator;
class Object;
class PlaybackScript;
class PlaybackScriptTemplate;

// ======================================================================

class PlaybackScriptTemplateList
{
friend class PlaybackScriptTemplate;

public:

	typedef stdvector<Object*>::fwd  ObjectVector;

public:

	static void                          install();

	static const PlaybackScriptTemplate *fetch(const CrcLowerString &filename);
	static PlaybackScript               *createPlaybackScript(const CrcLowerString &templateName, int priority, const ObjectVector &actors);

private:

	typedef stdmap<const CrcLowerString*, PlaybackScriptTemplate*, LessPointerComparator>::fwd  TemplateMap;

private:

	static void                          remove();
	static void                          stopTracking(const PlaybackScriptTemplate *playbackScriptTemplate);

private:

	// disabled
	PlaybackScriptTemplateList();
	PlaybackScriptTemplateList(const PlaybackScriptTemplateList&);
	PlaybackScriptTemplateList &operator =(const PlaybackScriptTemplateList&);

private:

	static bool         ms_installed;
	static TemplateMap *ms_templates;

};

// ======================================================================

#endif
