// PRIVATE

// ======================================================================
//
// ClientDataFile_Banner.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ClientDataFile_Banner_H
#define INCLUDED_ClientDataFile_Banner_H

// ======================================================================

class AppearanceTemplate;

#include "clientGame/ClientDataFile.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"

// ======================================================================
/**
 * Supports attaching banners to other objects via the client data file.
 *
 * A flag has a vertical attachment structure like a pole.  A banner has
 * a horizontal attachment structure such as a beam or boom.  Each handles
 * wind differently.
 */

class ClientDataFile::Banner
{
public:

	static Banner *load_0000(Iff &iff, char const *filename);

public:

	~Banner();

	void apply(Object *parentObject) const;
	void preloadAssets() const;

private:

	Banner(Iff &iff, char const *filename);

	// Disabled.
	Banner();
	Banner(const Banner&);
	Banner &operator =(const Banner&);

private:

	PersistentCrcString               m_bannerAppearanceTemplateName;
	AppearanceTemplate const mutable *m_bannerAppearanceTemplate;

	PersistentCrcString               m_hardpointName;
	float                             m_windSpeedScale;
	float                             m_liftRadiansPerUnitSpeed;
	float                             m_maxAnimationSpeed;
};

// ======================================================================

#endif
