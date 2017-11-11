// ======================================================================
//
// CreateTransformMask.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CreateTransformMask_H
#define INCLUDED_CreateTransformMask_H

// ======================================================================

#include "maya/MPxCommand.h"

class CrcLowerString;
class MArgList;
class Messenger;
class MStatus;

// ======================================================================

class CreateTransformMask: public MPxCommand
{
public:

	static void  install(Messenger *newMessenger);
	static void  remove();

	static void *creator();

public:

	MStatus      doIt(const MArgList &argList);

private:

	struct Arguments;

	typedef stdset<CrcLowerString>::fwd  StringSet;

private:

	static bool  processArguments(const MArgList &argList, Arguments &collectedArgs);
	static bool  collectTransformNames(StringSet &transformNames);
	static bool  getOutputPathName(CrcLowerString &outputPathName);
	static bool  writeTransformMask(const StringSet &transformNames, const CrcLowerString &pathName);

private:

	static const char *const cms_iffFilter;
	static const int         cms_iffSize;

	static Messenger        *messenger;

};

// ======================================================================

#endif
