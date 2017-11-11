// ==================================================================
//
// BasicMeshGeneratorTemplate.h
// copyright 2001 Sony Online Entertainment
// 
// ==================================================================

#ifndef INCLUDED_BasicMeshGeneratorTemplate_H
#define INCLUDED_BasicMeshGeneratorTemplate_H

// ==================================================================

#include "clientSkeletalAnimation/MeshGeneratorTemplate.h"

// ==================================================================
/**
 * Contains the shared data associated with a MeshGenerator, and contains
 * all data necessary to construct a new MeshGenerator instance
 * based off of this template.
 *
 * This class stores the shared (i.e. static/constant) data associated
 * with a MeshGenerator.  There can be many MeshGenerator instances
 * all making use of the same loaded BasicMeshGeneratorTemplate.
 */

class BasicMeshGeneratorTemplate: public MeshGeneratorTemplate
{
public:

	virtual int                               getDetailCount() const;
	virtual const BasicMeshGeneratorTemplate *fetchBasicMeshGeneratorTemplate(int detailIndex) const;

	/**
	 * Create a MeshGenerator instance associated with this template
	 * instance.
	 *
	 * The BasicMeshGeneratorTemplate instance stores the data for the
	 * MeshGenerator that can be shared across all instances of the
	 * mesh.  MeshGenerator stores the per-instance data, such as
	 * customization modifications.
	 *
	 * Derived classes overriding this function do not need to chain
	 * down to it.
	 *
	 * @return  a (new) MeshGenerator associated with this BasicMeshGeneratorTemplate,
	 *          with the MeshGenerator reference count increased by one.
	 */
	virtual MeshGenerator *createMeshGenerator() const = 0;

protected:

	explicit BasicMeshGeneratorTemplate(CrcString const &name);

private:

	// Disabled.
	BasicMeshGeneratorTemplate();
	BasicMeshGeneratorTemplate(const BasicMeshGeneratorTemplate&);
	BasicMeshGeneratorTemplate &operator =(const BasicMeshGeneratorTemplate&);

};

// ==================================================================

#endif
