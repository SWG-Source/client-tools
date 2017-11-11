// PRIVATE

// ======================================================================
//
// ClientDataFile_Wearable.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ClientDataFile_Wearable_H
#define INCLUDED_ClientDataFile_Wearable_H

// ======================================================================

#include "clientGame/ClientDataFile.h"
#include <vector>

class PersistentCrcString;
class SkeletalAppearanceTemplate;
class SharedObjectTemplate;

// ======================================================================

/**
 * Support baked wearables via the client data file.
 */

class ClientDataFile::Wearable
{
public:

	static Wearable *load_0000(Iff &iff);
	static void      concludeApply(Object *wearerObject);

public:

	~Wearable();

	bool apply(Object *wearerObject) const;
	void preloadAssets() const;

private:

	class VariableSetting;
	typedef stdvector<VariableSetting*>::fwd  VariableSettingVector;

private:

	Wearable(Iff &iff);

	// Disabled.
	Wearable();
	Wearable(const Wearable&);
	Wearable &operator =(const Wearable&);

private:

	SharedObjectTemplate const * m_objectTemplate;
	int m_objectWearableArrangement;
	SkeletalAppearanceTemplate *m_appearanceTemplate;
	VariableSettingVector       m_variableSettingVector;

};

// ======================================================================

#endif
