// ======================================================================
//
// UIWatcher.h
// copyright 1998    Bootprint Entertainment
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// The UIWatcher system allows pointers to objects to be automatically
// reset to NULL when the object watching them is destroyed.
// 
// For something <T> to be watchable, it must provide a routine of the form:
//
//   	UIWatchedByList &T::getWatchedByList() const;
//
// This allows things to be watchable without having to derive from a common
// base class.  The UIWatchedByList returned by the object must have exactly
// the same lifespan as the object itself.  The easiest way to accomplish
// that is to make the UIWatchedByList a member of the object.  Since it is
// possible to watch const objects, it's very likely that the member variable
// will need to be made mutable.
//
// ======================================================================

#ifndef INCLUDED_UIWatcher_H
#define INCLUDED_UIWatcher_H

// ======================================================================

class UIWatchedByList;

#include <set>

// ======================================================================

// Base class for UIWatcher and ConstUIWatcher.  This should never be used 
// directly by the application.  
class BaseUIWatcher
{
	friend class UIWatchedByList;

protected:

	explicit BaseUIWatcher(void *newValue);
	virtual ~BaseUIWatcher();

	void reset();
	void addTo(UIWatchedByList &watchedByList);
	void removeFrom(UIWatchedByList &watchedByList);

private:

	/// Disabled
	BaseUIWatcher();

	/// Disabled
	BaseUIWatcher(const BaseUIWatcher &);

protected:

	void *m_data;
};

// UIWatcher to a non-const object.
template <typename T>
class UIWatcher : public BaseUIWatcher
{
public:

	explicit UIWatcher(T *data=NULL);
	UIWatcher(const UIWatcher &newValue);
	~UIWatcher();  //lint !e1509 // Warning -- base class destructor for class is not virtual

	UIWatcher<T> & operator = (T *newValue);
	UIWatcher<T> & operator = (const UIWatcher<T> &);

	// Equality and less operators for UIWatchers. Without these operators,
	// std::find will coerce the pointer type to a UIWatcher and the resulting constructor
	// call will add an element to the watched object's watchedBy list, potentially
	// causing a reallocation. Then, when the find is done, the temporary object is
	// destroyed and the UIWatcher is removed from the watchedBy list.

	bool operator == ( T const * const rhs ) const;
	bool operator <  ( T const * const rhs ) const;

	bool operator == ( UIWatcher<T> const & rhs ) const;
	bool operator <  ( UIWatcher<T> const & rhs ) const;
	
	T *getPointer() const;
	operator T*() const;
	T *operator ->() const;
};

// UIWatcher to a const object.
template <typename T>
class ConstUIWatcher : public BaseUIWatcher
{
public:

	explicit ConstUIWatcher(const T *data=NULL);
	ConstUIWatcher(const ConstUIWatcher &newValue);
	~ConstUIWatcher();  //lint !e1509 // Warning -- base class destructor for class is not virtual

	ConstUIWatcher<T> &operator =(const T *newValue);
	ConstUIWatcher<T> &operator =(const ConstUIWatcher<T> &);
	
	// Equality and less operators for UIWatchers. Without these operators,
	// std::find will coerce the pointer type to a UIWatcher and the resulting constructor
	// call will add an element to the watched object's watchedBy list, potentially
	// causing a reallocation. Then, when the find is done, the temporary object is
	// destroyed and the UIWatcher is removed from the watchedBy list.

	bool operator == ( T const * const rhs ) const;
	bool operator <  ( T const * const rhs ) const;

	bool operator == ( ConstUIWatcher<T> const & rhs ) const;
	bool operator <  ( ConstUIWatcher<T> const & rhs ) const;
	
	const T *getPointer() const;
	operator const T*() const;
	const T *operator ->() const;
};

// Keeps track of whom is watching it.
class UIWatchedByList
{
	friend class BaseUIWatcher;

public:

	UIWatchedByList();
	~UIWatchedByList();

protected:

	void add(BaseUIWatcher &baseUIWatcher);
	void remove(BaseUIWatcher &baseUIWatcher);

private:

	/// Disabled
	UIWatchedByList(const UIWatchedByList &);

	/// Disabled
	UIWatchedByList &operator &(const UIWatchedByList &);

private:

	typedef std::set<BaseUIWatcher *> List;

private:

	List m_list;
};

// ======================================================================
/**
 * Construct a BaseUIWatcher
 */

inline BaseUIWatcher::BaseUIWatcher(void *data)
: m_data(data)
{
}

// ----------------------------------------------------------------------
/**
 * Reset a BaseUIWatcher.
 *
 * This routine is only called when the object being pointed to is being destroyed.
 */

inline void BaseUIWatcher::reset()
{
	m_data = NULL;
}

// ----------------------------------------------------------------------
/**
 * Destroy a BaseUIWatcher
 *
 * This routine will walk the singly linked list of UIWatchers and remove
 * this UIWatcher from the list.
 */

inline BaseUIWatcher::~BaseUIWatcher()
{
	m_data = NULL;
}

// ----------------------------------------------------------------------
/**
 * Add this BaseUIWatcher to the UIWatchedByList.
 * 
 * This routine exists so we don't need make UIWatcher<T> a friend of UIWatchedByList.
 * @internal
 */

inline void BaseUIWatcher::addTo(UIWatchedByList &watchedByList)
{
	watchedByList.add(*this);
}

// ----------------------------------------------------------------------
/**
 * Remove this BaseUIWatcher from the UIWatchedByList.
 * 
 * This routine exists so we don't need make UIWatcher<T> a friend of UIWatchedByList.
 * @internal
 */

inline void BaseUIWatcher::removeFrom(UIWatchedByList &watchedByList)
{
	watchedByList.remove(*this);
}

// ======================================================================
/**
 * Convert the data pointer to the object pointer type.
 * 
 * @return Pointer to the object the UIWatcher points to
 */

template <typename T>
inline T *UIWatcher<T>::getPointer() const
{
	return reinterpret_cast<T *>(m_data);
}

// ----------------------------------------------------------------------
/**
 * Construct a UIWatcher.
 * 
 * @param data Initial value for the data pointer.
 */

template <typename T>
inline UIWatcher<T>::UIWatcher(T *data)
: BaseUIWatcher(data)
{
	if (m_data)
		addTo(getPointer()->getWatchedByList());
}

// ----------------------------------------------------------------------
/**
 * Copy-construct a UIWatcher.
 * 
 * @param rhs UIWatcher to copy the data pointer from.
 */

template <typename T>
inline UIWatcher<T>::UIWatcher(const UIWatcher &rhs)
: BaseUIWatcher(rhs.m_data)
{
	if (m_data)
		addTo(getPointer()->getWatchedByList());
}

// ----------------------------------------------------------------------
/**
 * Destroy a UIWatcher.
 */

template <typename T>
inline UIWatcher<T>::~UIWatcher()
{
	if (m_data)
		removeFrom(getPointer()->getWatchedByList());
}

// ----------------------------------------------------------------------
/**
 * Set the value of UIWatcher from a pointer.
 * 
 * @param newValue  Pointer to copy the data from.
 * @return The UIWatcher itself.
 */

template <typename T>
inline UIWatcher<T> & UIWatcher<T>::operator = (T *data)
{
	if (data != m_data)
	{
		if (m_data)
			removeFrom(getPointer()->getWatchedByList());
			
		m_data = data;

		if (m_data)
			addTo(getPointer()->getWatchedByList());
	}

	return *this;
}

// ----------------------------------------------------------------------
/**
 * Assign one UIWatcher to another, effectively making the assignee watch
 * the same object as the assigner.
 * 
 * @return The object itself
 */

template<typename T>
inline UIWatcher<T> & UIWatcher<T>::operator = (const UIWatcher<T> & rhs)
{
	return operator = (rhs.getPointer());
}

// ----------------------------------------------------------------------
/**
 * Equality operator for UIWatchers, which allows us to compare a UIWatcher
 * and a pointer without coercing the pointer to a UIWatcher also. 
 *
 * A UIWatcher is considered to be equal to a pointer P if it's watching 
 * the object pointed to by P.
 * 
 * @return True if the UIWatcher is watching the object pointed to by rhs.
 */

template<typename T>
inline bool UIWatcher<T>::operator == ( T const * const rhs ) const
{
	return getPointer() == rhs;
}

// ----------------------------------------------------------------------
/**
 * Less operator for UIWatchers, which allows us to compare a UIWatcher
 * and a pointer without coercing the pointer to a UIWatcher also.
 *
 * Asking if a UIWatcher is less than a pointer doesn't make much sense,
 * but it's required by the STL associative containers.
 * 
 * @return True if the UIWatcher's pointer is numerically less than rhs
 */

template<typename T>
inline bool UIWatcher<T>::operator <  ( T const * const rhs ) const
{
	return getPointer() < rhs;
}

// ----------------------------------------------------------------------
/**
 * Equality operator for UIWatchers. 
 *
 * Two UIWatchers are considered to be equal if they watch the same object.
 * 
 * @return True if the two UIWatchers are watching the same object
 */

template<typename T>
inline bool UIWatcher<T>::operator == ( UIWatcher<T> const & rhs ) const
{
	return getPointer() == rhs.getPointer();
}

// ----------------------------------------------------------------------
/**
 * Less operator for UIWatchers.
 *
 * Asking if a UIWatcher is less than another UIWatcher doesn't make much 
 * sense, but it's required by the STL associative containers.
 * 
 * @return True if the UIWatcher's pointer is numerically less than rhs
 */

template<typename T>
inline bool UIWatcher<T>::operator < ( UIWatcher<T> const & rhs ) const
{
	return getPointer() < rhs.getPointer();
}

// ----------------------------------------------------------------------
/**
 * Cast the UIWatcher to the object pointer type.
 * 
 * @return Pointer to the object the UIWatcher points to.
 */

template <typename T>
inline UIWatcher<T>::operator T *() const
{
	return reinterpret_cast<T *>(m_data);
}

// ----------------------------------------------------------------------
/**
 * Dereference the UIWatcher to the object.
 * 
 * @return Pointer to the object the UIWatcher points to.
 */

template <typename T>
inline T *UIWatcher<T>::operator ->() const
{
	return reinterpret_cast<T *>(m_data);
}

// ======================================================================
/**
 * Convert the data pointer to the object pointer type.
 * 
 * @return Pointer to the object the ConstUIWatcher points to
 */

template <typename T>
inline const T *ConstUIWatcher<T>::getPointer() const
{
	return reinterpret_cast<const T *>(m_data);
}

// ----------------------------------------------------------------------
/**
 * Construct a UIWatcher.
 * 
 * @param data Initial value for the data pointer.
 */

template <typename T>
inline ConstUIWatcher<T>::ConstUIWatcher(const T *data)
: BaseUIWatcher(const_cast<void *>(reinterpret_cast<const void *>(data)))
{
	if (m_data)
		addTo(getPointer()->getWatchedByList());
}

// ----------------------------------------------------------------------
/**
 * Copy-construct a ConstUIWatcher.
 * 
 * @param rhs ConstUIWatcher to copy the data pointer from.
 */

template <typename T>
inline ConstUIWatcher<T>::ConstUIWatcher(const ConstUIWatcher &rhs)
: BaseUIWatcher(rhs.m_data)
{
	if (m_data)
		addTo(getPointer()->getWatchedByList());
}

// ----------------------------------------------------------------------
/**
 * Destroy a ConstUIWatcher.
 */

template <typename T>
inline ConstUIWatcher<T>::~ConstUIWatcher()
{
	if (m_data)
		removeFrom(getPointer()->getWatchedByList());
}

// ----------------------------------------------------------------------
/**
 * Set the value of ConstUIWatcher from a pointer.
 * 
 * @param newValue  Pointer to copy the data from.
 * @return The ConstUIWatcher itself.
 */

template <typename T>
inline ConstUIWatcher<T> &ConstUIWatcher<T>::operator =(const T *data)
{
	if (m_data != static_cast<const void *>(data))
	{
		if (m_data)
			removeFrom(getPointer()->getWatchedByList());
			
		m_data = const_cast<void *>(reinterpret_cast<const void *>(data));

		if (m_data)
			addTo(getPointer()->getWatchedByList());
	}

	return *this;
}

// ----------------------------------------------------------------------
/**
 * Assign one ConstUIWatcher to another, effectively making the assignee watch
 * the same object as the assigner.
 * 
 * @return The object itself
 */

template<typename T>
inline ConstUIWatcher<T> & ConstUIWatcher<T>::operator = (const ConstUIWatcher<T> & rhs)
{
	return operator = (rhs.getPointer());
}

// ----------------------------------------------------------------------
/**
 * Equality operator for UIWatchers, which allows us to compare a UIWatcher
 * and a pointer without coercing the pointer to a UIWatcher also. 
 *
 * A UIWatcher is considered to be equal to a pointer P if it's watching 
 * the object pointed to by P.
 * 
 * @return True if the UIWatcher is watching the object pointed to by rhs.
 */

template<typename T>
inline bool ConstUIWatcher<T>::operator == ( T const * const rhs ) const
{
	return getPointer() == rhs;
}

// ----------------------------------------------------------------------
/**
 * Less operator for UIWatchers, which allows us to compare a UIWatcher
 * and a pointer without coercing the pointer to a UIWatcher also.
 *
 * Asking if a UIWatcher is less than a pointer doesn't make much sense,
 * but it's required by the STL associative containers.
 * 
 * @return True if the UIWatcher's pointer is numerically less than rhs
 */

template<typename T>
inline bool ConstUIWatcher<T>::operator <  ( T const * const rhs ) const
{
	return getPointer() < rhs;
}

// ----------------------------------------------------------------------
/**
 * Equality operator for UIWatchers. 
 *
 * Two UIWatchers are considered to be equal if they watch the same object.
 * 
 * @return True if the two UIWatchers are watching the same object
 */

template<typename T>
inline bool ConstUIWatcher<T>::operator == ( ConstUIWatcher<T> const & rhs ) const
{
	return getPointer() == rhs.getPointer();
}

// ----------------------------------------------------------------------
/**
 * Less operator for UIWatchers.
 *
 * Asking if a UIWatcher is less than another UIWatcher doesn't make much 
 * sense, but it's required by the STL associative containers.
 * 
 * @return True if the UIWatcher's pointer is numerically less than rhs
 */

template<typename T>
inline bool ConstUIWatcher<T>::operator < ( ConstUIWatcher<T> const & rhs ) const
{
	return getPointer() < rhs.getPointer();
}

// ----------------------------------------------------------------------
/**
 * Cast the ConstUIWatcher to the object pointer type.
 * 
 * @return Pointer to the object the ConstUIWatcher points to.
 */

template <typename T>
inline ConstUIWatcher<T>::operator const T *() const
{
	return reinterpret_cast<const T *>(m_data);
}

// ----------------------------------------------------------------------
/**
 * Dereference the ConstUIWatcher to the object.
 * 
 * @return Pointer to the object the ConstUIWatcher points to.
 */

template <typename T>
inline const T *ConstUIWatcher<T>::operator ->() const
{
	return reinterpret_cast<const T *>(m_data);
}

// ----------------------------------------------------------------------
/**
 * Construct a UIWatchedByList.
 *
 * This list of UIWatchers remains NULL until someone first watches the object.
 */

inline UIWatchedByList::UIWatchedByList()
: m_list()
{
}

// ----------------------------------------------------------------------
class UIWatchable
{
public:

	UIWatchable() {}
	virtual ~UIWatchable() {}


	UIWatchedByList & getWatchedByList() const;

private:
	UIWatchable(UIWatchable const & rhs);
	UIWatchable & operator=(UIWatchable const & rhs);
	
	mutable UIWatchedByList     m_watchedByList;
};

// ----------------------------------------------------------------------

inline UIWatchedByList & UIWatchable::getWatchedByList() const
{
	return m_watchedByList;
}


// ======================================================================
/**
 * Destroy a UIWatchedByList.
 *
 * All UIWatchers currently watching the owner of this object will be reset to NULL.
 */

inline UIWatchedByList::~UIWatchedByList()
{
	if (!m_list.empty())
	{
		const List::iterator end = m_list.end();
		for (List::iterator i = m_list.begin(); i != end; ++i)
			(*i)->reset();
	}
}

// ----------------------------------------------------------------------
/**
 * Add a UIWatcher to the UIWatchedByList.
 *
 * If the list hasn't yet been created, create it.
 *
 * @internal
 */

inline void UIWatchedByList::add(BaseUIWatcher & uiwatcher)
{
	m_list.insert(&uiwatcher);
}

// ----------------------------------------------------------------------
/**
 * Remove a UIWatcher from the UIWatchedByList.
 *
 * @internal
 * @todo Consider resizing the vector down to conserve memory.
 * @todo Cconsider deleting the vector if it becomes empty.
 */

inline void UIWatchedByList::remove(BaseUIWatcher &uiwatcher)
{
	m_list.erase(&uiwatcher);
}


// ======================================================================

#endif
