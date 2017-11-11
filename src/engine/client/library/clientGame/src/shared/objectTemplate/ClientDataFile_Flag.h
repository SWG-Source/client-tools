// PRIVATE

// ======================================================================
//
// ClientDataFile_Flag.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ClientDataFile_Flag_H
#define INCLUDED_ClientDataFile_Flag_H

// ======================================================================

class AppearanceTemplate;

#include "clientGame/ClientDataFile.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"

// ======================================================================
/**
 * Supports attaching flags to other objects via the client data file.
 */

class ClientDataFile::Flag
{
public:

	static Flag *load_0000(Iff &iff, char const *filename);

public:

	~Flag();

	void apply(Object *parentObject) const;
	void preloadAssets() const;

private:

	Flag(Iff &iff, char const *filename);

	// Disabled.
	Flag();
	Flag(const Flag&);
	Flag &operator =(const Flag&);

private:

	PersistentCrcString               m_flagAppearanceTemplateName;
	AppearanceTemplate const mutable *m_flagAppearanceTemplate;

	PersistentCrcString               m_hardpointName;
	float                             m_windSpeedScale;
	float                             m_maxAnimationSpeed;
};

// ======================================================================

#endif
