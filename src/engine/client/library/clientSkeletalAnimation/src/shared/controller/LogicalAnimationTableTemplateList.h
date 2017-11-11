// ======================================================================
//
// LogicalAnimationTableTemplateList.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_LogicalAnimationTableTemplateList_H
#define INCLUDED_LogicalAnimationTableTemplateList_H

// ======================================================================

class CrcString;
class LessPointerComparator;
class LogicalAnimationTableTemplate;

// ======================================================================

class LogicalAnimationTableTemplateList
{
friend class LogicalAnimationTableTemplate;

public:

	static void install();

	static const LogicalAnimationTableTemplate *fetch(const CrcString &pathName);
	static void                                 garbageCollect();

private:

	static void  remove();
	static void  stopTracking(const LogicalAnimationTableTemplate &latTemplate);

	static LogicalAnimationTableTemplate *createTemplateFromIffFileName(CrcString const &pathName);
	static LogicalAnimationTableTemplate *createTemplateFromXmlFileName(CrcString const &pathName);


private:

	// disabled
	LogicalAnimationTableTemplateList();

};

// ======================================================================

#endif
