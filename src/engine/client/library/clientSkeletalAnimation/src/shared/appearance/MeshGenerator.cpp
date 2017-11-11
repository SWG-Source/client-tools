// ======================================================================
//
// MeshGenerator.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/MeshGenerator.h"

#include "clientSkeletalAnimation/MeshGeneratorTemplate.h"

#include <algorithm>
#include <vector>

// ======================================================================
/**
 * Associates a ModificationHandler and context.
 */

struct MeshGenerator::ModificationHandlerInfo
{
public:

	ModificationHandlerInfo(ModificationHandler modificationHandler, const void *context);

	bool operator ==(const ModificationHandlerInfo &rhs) const;

public:

	ModificationHandler  m_modificationHandler;
	const void          *m_context;

private:

	// disabled
	ModificationHandlerInfo();

};

// ======================================================================

inline MeshGenerator::ModificationHandlerInfo::ModificationHandlerInfo(ModificationHandler modificationHandler, const void *context) :
	m_modificationHandler(modificationHandler),
	m_context(context)
{
}

// ----------------------------------------------------------------------

inline bool MeshGenerator::ModificationHandlerInfo::operator ==(const ModificationHandlerInfo &rhs) const
{
	return (rhs.m_modificationHandler == m_modificationHandler) && (rhs.m_context == m_context);
}

// ======================================================================

bool MeshGenerator::isReadyForUse() const
{
	// Default: everything is ready.
	return true;
}

// ----------------------------------------------------------------------
/**
 * Decrement the reference count associated with this instance.
 *
 * If the reference count falls below one, the instance will be
 * destroyed.
 *
 * The caller should maintain a separate reference count for each
 * logically independent reference to the MeshGenerator instance
 * through a call to fetch(). When a reference is needed no longer, 
 * release() should be called.
 */

void MeshGenerator::release() const
{
	--m_referenceCount;

	if (m_referenceCount < 1)
	{
		DEBUG_WARNING(m_referenceCount < 0, ("bad reference count handling: deleting MeshGenerator [0x%08x] with ref count [%d] < 0\n", this, m_referenceCount));
		delete const_cast<MeshGenerator*>(this);
	}
}

// ----------------------------------------------------------------------
/**
 * Add the ModificationHandler/context pair to be invoked whenever the
 * MeshGenerator is modified.
 *
 * The caller will need to deregister the handler/context when it no longer
 * wishes to receive modification callbacks.
 *
 * @param handler  callback to invoke when this MeshGenerator instance
 *                 is modified.
 * @param context  the context to be passed to the callback when it is
 *                 modified.
 *
 * @see deregisterModificationHandler()
 */

void MeshGenerator::registerModificationHandler(ModificationHandler handler, const void *context)
{
	//-- ensure the container has been created
	if (!m_modificationHandlerInfoVector)
		m_modificationHandlerInfoVector = new ModificationHandlerInfoVector();

	const ModificationHandlerInfo  mhInfo(handler, context);

#ifdef _DEBUG
	//-- check for duplicate entries
	const ModificationHandlerInfoVector::iterator findIt = std::find(m_modificationHandlerInfoVector->begin(), m_modificationHandlerInfoVector->end(), mhInfo);
	DEBUG_FATAL(findIt != m_modificationHandlerInfoVector->end(), ("tried to add duplicate handler/context info"));
#endif

	//-- add new modification handler to container
	m_modificationHandlerInfoVector->push_back(mhInfo);
}

// ----------------------------------------------------------------------
/**
 * Remove the ModificationHandler/context so they no longer are invoked whenever 
 * the MeshGenerator is modified.
 *
 * The caller must have previously called registerModificationHandler() with
 * the handler/context combination.
 *
 * @param handler  callback to invoke when this MeshGenerator instance
 *                 is modified.
 * @param context  the context to be passed to the callback when it is
 *                 modified.
 *
 * @see registerModificationHandler()
 */

void MeshGenerator::deregisterModificationHandler(ModificationHandler handler, const void *context)
{
	//-- ensure we have a modification handler container
	if (!m_modificationHandlerInfoVector)
	{
		DEBUG_WARNING(true, ("no ModificationHandler entries have been made.\n"));
		return;
	}

	//-- find the entry
	const ModificationHandlerInfoVector::iterator findIt = std::find(m_modificationHandlerInfoVector->begin(), m_modificationHandlerInfoVector->end(), ModificationHandlerInfo(handler, context));
	if (findIt == m_modificationHandlerInfoVector->end())
	{
		DEBUG_WARNING(true, ("tried to remove non-existant handler/context info\n"));
		return;
	}

	//-- erase modification handler from container
	IGNORE_RETURN(m_modificationHandlerInfoVector->erase(findIt));
}

// ======================================================================
/**
 * Construct a MeshGenerator instance.
 *
 * The class will retain a logical reference to the given 
 * MeshGeneratorTemplate instance.
 *
 * @param meshGeneratorTemplate  the MeshGeneratorTemplate instance from
 *                               which this MeshGenerator instance is created,
 *                               must not be NULL.
 */

MeshGenerator::MeshGenerator(const MeshGeneratorTemplate *meshGeneratorTemplate) :
	m_meshGeneratorTemplate(meshGeneratorTemplate),
	m_referenceCount(0),
	m_modificationHandlerInfoVector(0)
{
	//-- keep a reference to the MeshGeneratorTemplate instance
	NOT_NULL(m_meshGeneratorTemplate);
	m_meshGeneratorTemplate->fetch();
}

// ----------------------------------------------------------------------
/**
 * Destroy the MeshGenerator instance.
 */

MeshGenerator::~MeshGenerator(void)
{
	//-- release reference to MeshGeneratorTemplate 
	//lint -esym(1740, MeshGenerator::m_meshGeneratorTemplate) // niether freed nor zero'ed // it's okay, we're releasing
	m_meshGeneratorTemplate->release();

	delete m_modificationHandlerInfoVector;
}

// ----------------------------------------------------------------------
/**
 * Fire off a callback to each interested entity that the MeshGenerator
 * has been modified.
 *
 * @see registerModificationHandler()
 * @see deregisterModificationHandler()
 */

void MeshGenerator::signalModified()
{
	if (!m_modificationHandlerInfoVector)
		return;

	const ModificationHandlerInfoVector::iterator endIt = m_modificationHandlerInfoVector->end();
	for (ModificationHandlerInfoVector::iterator it = m_modificationHandlerInfoVector->begin(); it != endIt; ++it)
		(*(it->m_modificationHandler))(it->m_context, *this);
}

// ======================================================================
