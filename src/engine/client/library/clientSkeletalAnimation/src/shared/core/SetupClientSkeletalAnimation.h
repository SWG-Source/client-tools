// ======================================================================
//
// SetupClientSkeletalAnimation.h
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SetupClientSkeletalAnimation_H
#define INCLUDED_SetupClientSkeletalAnimation_H

// ======================================================================

class SetupClientSkeletalAnimation
{
public:

	struct Data
	{
		Data();

		bool  allowLod0Skipping;
		bool  stitchedSkinInheritsFromSelf;
	};

public:

	static void setupGameData(Data &data);
	static void setupToolData(Data &data);
	static void setupViewerData(Data &data);

	static void install(Data const &data);

private:

	// Disabled.
	SetupClientSkeletalAnimation (void);
	SetupClientSkeletalAnimation(const SetupClientSkeletalAnimation&);
	SetupClientSkeletalAnimation& operator= (const SetupClientSkeletalAnimation&);

private:

	static const char *const cms_defaultPriorityMapFileName;

};

// ======================================================================

#endif
