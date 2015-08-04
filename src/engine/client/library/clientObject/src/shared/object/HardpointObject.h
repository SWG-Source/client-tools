// ======================================================================
//
// HardpointObject.h
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_HardpointObject_H
#define INCLUDED_HardpointObject_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedObject/Object.h"

class MemoryBlockManager;

// ======================================================================

class HardpointObject : public Object
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	HardpointObject ();
	explicit HardpointObject (const CrcString& hardpointName);
	virtual ~HardpointObject ();

	virtual float alter (float elapsedTime);

	void setHardpointName (const CrcString& hardpointName);
	CrcString const &getHardpointName () const;

	void snapToPosition ();

private:

	HardpointObject (const HardpointObject&);
	HardpointObject& operator= (const HardpointObject&);

private:

	PersistentCrcString m_hardpointName;
#ifdef _DEBUG
	bool           m_missingHardpointWarning;
#endif
};

//----------------------------------------------------------------------

inline const CrcString & HardpointObject::getHardpointName () const
{
	return m_hardpointName;
}

// ======================================================================

#endif
