// ==================================================================
//
// MeshGeneratorTemplate.h
// copyright 2001 Sony Online Entertainment
// 
// ==================================================================

#ifndef INCLUDED_MeshGeneratorTemplate_H
#define INCLUDED_MeshGeneratorTemplate_H

// ==================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"

class BasicMeshGeneratorTemplate;
class CrcString;
class MeshGenerator;

// ==================================================================
/**
 * Contains the shared data associated with a MeshGenerator, and contains
 * all data necessary to construct a new MeshGenerator instance
 * based off of this template.
 *
 * This class stores the shared (i.e. static/constant) data associated
 * with a MeshGenerator.  There can be many MeshGenerator instances
 * all making use of the same loaded MeshGeneratorTemplate.
 */

class MeshGeneratorTemplate
{
friend class MeshGeneratorTemplateList;

public:

	/**
	 * Retrieve the number of detail levels associated with this
	 * MeshGeneratorTemplate instance.
	 *
	 * @return  the number of detail levels associated with this
	 *          MeshGeneratorTemplate instance.
	 */
	virtual int getDetailCount() const = 0;

	/**
	 * Fetch a reference to one detail level associated with
	 * the MeshGeneratorTemplate.
	 *
	 * Fetching increases ther reference count on the returned
	 * instance.  The caller should make certain to call release() 
	 * on the returned instance when it no longer is needed.
	 */
	virtual const BasicMeshGeneratorTemplate *fetchBasicMeshGeneratorTemplate(int detailIndex) const = 0;

	/**
	 * Do whatever is necessarily to load as much known-to-be-needed
	 * data at this point.
	 *
	 * In particular, load anything that normally would be demand-loaded.
	 */
	virtual void preloadAssets() const = 0;
	virtual void garbageCollect() const = 0;

	virtual bool hasOnlyNonCollidableShaderTemplates() const;

	const CrcString      &getName() const;

	void                  fetch() const;
	void                  release() const;
	int                   getReferenceCount() const;

protected:

	explicit MeshGeneratorTemplate(const CrcString &name);
	virtual ~MeshGeneratorTemplate();

private:

	// Disabled.
	MeshGeneratorTemplate();
	MeshGeneratorTemplate(const MeshGeneratorTemplate&);
	MeshGeneratorTemplate &operator =(const MeshGeneratorTemplate&);

private:

	PersistentCrcString  m_name;
	mutable int          m_referenceCount;

};

// ==================================================================
/**
 * Increment the reference count on this instance.
 *
 * The caller must maintain a reference count for each independent reference
 * to this instance.  Failure to do so most likely will result in a piece
 * of code referencing an already-deleted instance.  Likewise, the caller
 * must ensure release() is called whenever a reference to this instance
 * no longer is needed.
 */

inline void MeshGeneratorTemplate::fetch() const
{
	++m_referenceCount;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the reference count associated with this instance.
 *
 * @return  the reference count associated with this instance.
 */

inline int MeshGeneratorTemplate::getReferenceCount() const
{
	return m_referenceCount;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the TreeFile pathName of this MeshGeneratorTemplate instance.
 *
 * The pathName may be a zero-length string if the MeshGeneratorTemplate
 * was loaded via an embedded Iff rather than through a stand-alone
 * MeshGeneratorTemplate file.
 *
 * @return  the TreeFile pathName of this MeshGeneratorTemplate instance.
 */

inline const CrcString &MeshGeneratorTemplate::getName() const
{
	return m_name;
}

// ==================================================================

#endif
