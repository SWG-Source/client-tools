// ======================================================================
//
// SkeletonTemplate.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_SkeletonTemplate_H
#define INCLUDED_SkeletonTemplate_H

// ==================================================================

#include "clientSkeletalAnimation/SkeletonTemplateDef.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"

class BasicSkeletonTemplate;

// ==================================================================

class SkeletonTemplate
{
friend class SkeletonTemplateList;

public:

	/**
	 * Retrieve the number of detail levels associated with this
	 * SkeletonTemplate instance.
	 *
	 * @return  the number of detail levels associated with this
	 *          SkeletonTemplate instance.
	 */
	virtual int getDetailCount() const = 0;

	/**
	 * Fetch a reference to one detail level associated with
	 * the SkeletonTemplate.
	 *
	 * Fetching increases ther reference count on the returned
	 * instance.  The caller should make certain to call release() 
	 * on the returned instance when it no longer is needed.
	 */
	virtual const BasicSkeletonTemplate *fetchBasicSkeletonTemplate(int detailIndex) const = 0;

	void                      fetch() const;
	void                      release() const;
	int                       getReferenceCount() const;

	CrcString const          &getName() const;

protected:

	explicit SkeletonTemplate(CrcString const &name);
	virtual ~SkeletonTemplate();

private:

	// disabled
	SkeletonTemplate();
	SkeletonTemplate(const SkeletonTemplate&);
	SkeletonTemplate &operator =(const SkeletonTemplate&);

private:

	mutable int               m_referenceCount;
	PersistentCrcString const m_name;	

};

// ==================================================================

inline CrcString const &SkeletonTemplate::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

inline int SkeletonTemplate::getReferenceCount() const
{
	return m_referenceCount;
}

// ==================================================================

#endif
