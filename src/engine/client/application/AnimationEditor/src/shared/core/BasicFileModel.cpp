// ======================================================================
//
// BasicFileModel.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/BasicFileModel.h"

#include "sharedFoundation/PointerDeleter.h"

#include <algorithm>
#include <vector>

// ======================================================================

class BasicFileModel::ListenerData
{
public:

	ListenerData(ModificationListener callback, const void *context);
	~ListenerData();

	void  signalListener(BasicFileModel &fileModel, bool oldModifiedState, bool newModifiedState) const;

	ModificationListener  getModificationListener() const;
	const void           *getContext() const;

private:

	ListenerData();
	ListenerData(const ListenerData&);             //lint -esym(754, ListenerData::ListenerData) // not referenced // defensive hiding
	ListenerData &operator =(const ListenerData&); //lint -esym(754, ListenerData::operator=)    // not referenced // invariant class cannot be assigned

private:

	const ModificationListener  m_callback;
	const void *const           m_context;

};

// ======================================================================
// class BasicFileModel::ListenerData
// ======================================================================

BasicFileModel::ListenerData::ListenerData(ModificationListener callback, const void *context) :
	m_callback(callback),
	m_context(context)
{
	NOT_NULL(m_callback);
}

// ----------------------------------------------------------------------

BasicFileModel::ListenerData::~ListenerData()
{
	//lint -esym(1540, ListenerData::m_context) // not freed nor zerod // would be zeroed but invariant class
}

// ----------------------------------------------------------------------

void BasicFileModel::ListenerData::signalListener(BasicFileModel &fileModel, bool oldModifiedState, bool newModifiedState) const
{
	if (m_callback)
		(*m_callback)(fileModel, oldModifiedState, newModifiedState, m_context);
}

// ----------------------------------------------------------------------

inline FileModel::ModificationListener BasicFileModel::ListenerData::getModificationListener() const
{
	return m_callback;
} //lint !e1763 // const function indirectly modifies class // it's okay

// ----------------------------------------------------------------------

inline const void *BasicFileModel::ListenerData::getContext() const
{
	return m_context;
}

// ======================================================================

BasicFileModel::BasicFileModel() :
	m_isModified(false),
	m_listenerDataVector(new ListenerDataVector())
{
}

// ----------------------------------------------------------------------

BasicFileModel::~BasicFileModel()
{
	std::for_each(m_listenerDataVector->begin(), m_listenerDataVector->end(), PointerDeleter());
	delete m_listenerDataVector;
}

// ----------------------------------------------------------------------

bool BasicFileModel::isModified() const
{
	return m_isModified;
}

// ----------------------------------------------------------------------

void BasicFileModel::setModifiedState(bool newModifiedState)
{
	//-- save old modified state
	const bool oldModifiedState = m_isModified;

	//-- set new modified state
	m_isModified = newModifiedState;

	//-- report state change to all listeners
	const ListenerDataVector::iterator endIt = m_listenerDataVector->end();
	for (ListenerDataVector::iterator it = m_listenerDataVector->begin(); it != endIt; ++it)
	{
		NOT_NULL(*it);
		if (*it)
			(*it)->signalListener(*this, oldModifiedState, newModifiedState);
	}
}

// ----------------------------------------------------------------------

void BasicFileModel::addModificationListener(ModificationListener listener, const void *context)
{
	m_listenerDataVector->push_back(new ListenerData(listener, context));
}

// ----------------------------------------------------------------------

void BasicFileModel::removeModificationListener(ModificationListener listener, const void *context)
{
	//-- find matching ListenerData
	const ListenerDataVector::iterator endIt = m_listenerDataVector->end();
	for (ListenerDataVector::iterator it = m_listenerDataVector->begin(); it != endIt; ++it)
	{
		// get listener data
		ListenerData *const ld = *it;
		if (!ld)
			continue;

		// check for a match
		if ((ld->getModificationListener() == listener) && (ld->getContext() == context))
		{
			// delete the matching entry
			IGNORE_RETURN(m_listenerDataVector->erase(it));
			return;
		}
	}

	//-- match not found
	WARNING(true, ("modification listener not found for removal: listener [0x%08x] context [0x%08x].", reinterpret_cast<int>(listener), reinterpret_cast<int>(context)));
}

// ======================================================================
