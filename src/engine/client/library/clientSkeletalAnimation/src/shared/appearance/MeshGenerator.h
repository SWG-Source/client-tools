// ======================================================================
//
// MeshGenerator.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_MeshGenerator_H
#define INCLUDED_MeshGenerator_H

// ======================================================================

class Appearance;
class CrcLowerString;
class CustomizationData;
class MeshConstructionHelper;
class MeshGeneratorTemplate;
class OcclusionZoneSet;
class ShaderPrimitive;
class Skeleton;
class TransformNameMap;

// ======================================================================

class MeshGenerator
{
public:

	/**
	 * Callback signature for entities interested in receiving a callback
	 * when the MeshGenerator is modified.
	 *
	 * This callback will be made after the MeshGenerator instance has been
	 * modified.
	 *
	 * @param context        contains the context pointer registered with the
	 *                       handler.
	 * @param meshGenerator  specifies the MeshGenerator instance that was just
	 *                       modified. 
	 */
	typedef void (*ModificationHandler)(const void *context, MeshGenerator &meshGenerator);

	typedef stdvector<ShaderPrimitive *>::fwd  ShaderPrimitiveVector;

public:

	/**
	 * Retrieve the layer at which this mesh output should reside relative to other mesh output.
	 *
	 * Smaller values are closer to the center of the mesh than larger values.  Items at the
	 * same layer cannot occlude each other, but can occlude items in layers with smaller layer
	 * values.
	 */
	virtual int  getOcclusionLayer() const = 0;

	/**
	 * Apply any modifications to the Skeleton associated with this MeshGenerator instance.
	 *
	 * Any modifications to the skeleton required by the MeshGenerator must be performed
	 * via this call.  The reason for this is that all Skeleton modifications must be made
	 * prior to associating any vertex data to a Skeleton transform index.  Modifications to
	 * the Skeleton may change Skeleton transform indices.
	 *
	 * For example usage, the SkeletalMeshGenerator attaches hardpoints to the Skeleton via
	 * this interface.
	 *
	 * This function assumes it is invoked between a Skeleton::beginSkeletonModification(),
	 * Skeleton::endSkeletonModification() function pair.
	 *
	 * @param skeleton         the skeleton that will be modified.
	 */
	virtual void  applySkeletonModifications(Skeleton &skeleton) const = 0;

	/**
	 *
	 * Generate ShaderPrimitive objects.
	 *
	 * Generated mesh data is added as ShaderPrimitive objects.  The generated mesh 
	 * data takes into account any recognized parameters specified in variableSet.
	 * Any transforms applied should be mapped to the indices specified in
	 * transformNameMap.  The MeshGenerator becomes authoratitive and
	 * provides the data for a weld zone if it has not yet been specified;
	 * otherwise, it uses the authoritative weld data that exists.  The named 
	 * mesh area zones which are currently fully occluded are specified in
	 * zonesCurrentlyOccluded.  If this mesh contains any of the fully
	 * occluded zones, those zones are not included in data added to outputMesh.
	 * Any zones fully occluded by this mesh are added to zonesThisOccludes.
	 */
	virtual void addShaderPrimitives(
		Appearance             &appearance, 
		int                     lodIndex, 
		CustomizationData      *customizationData,
		const TransformNameMap &transformNameMap, 
		const OcclusionZoneSet &zonesCurrentlyOccluded, 
		OcclusionZoneSet       &zonesOccludedByThisLayer, 
		ShaderPrimitiveVector  &shaderPrimitives
		) const = 0;

	/**
	 * Set the CustomizationData instance to be associated with this
	 * MeshGenerator instance.
	 *
	 * @param customizationData  the new CustomizationData instance to be
	 *                           associated with this MeshGenerator instance.
	 *                           This value may be NULL.
	 */
	virtual void setCustomizationData(CustomizationData *customizationData) = 0;

	/**
	 * Create a stand-alone appearance suitable for viewing this MeshGenerator
	 * object.
	 *
	 * This functionality is intended to be used solely for non-production
	 * tools- or debug-style code.  If you want to create a real appearance
	 * making use of a MeshGenerator, you need to create a SkeletalAppearanceTemplate.
	 *
	 * @return  an Appearance instance for viewing solely this MeshGenerator.
	 */
	virtual Appearance *createAppearance() const = 0;

	/**
	 * Add all CustomizationData variables influencing this MeshGenerator instance
	 * to the given CustomizationData instance.
	 *
	 * This is primarily useful as a mechanism for tools.  The game should already
	 * know which customization variables it has enabled for Object instances via the
	 * ObjectTemplate system.
	 *
	 * Derived classes do not need to chain down to this function.
	 *
	 * @param customizationData  the CustomizationData instance to which new
	 *                           variables will be added.
	 */
	virtual void addCustomizationVariables(CustomizationData &customizationData) const = 0;

	/** 
	 * Indicate if this instance is ready to be used.
	 *
	 * This function could return false if the underlying asset is preloading or baking
	 * or doing some other startup activity.  The caller can assume that once this value
	 * returns true, it will never return false again (i.e. once ready for use, it will
	 * always be ready for use as long as the instance exists).
	 *
	 * @return  true if the asset is ready for use at this time and here forward during
	 *          this instance's lifetime; false if the asset is not yet ready for use.
	 */
	virtual bool isReadyForUse() const;

	void  fetch() const;
	void  release() const;
	int   getReferenceCount() const;

	const MeshGeneratorTemplate &getMeshGeneratorTemplate() const;

	void  registerModificationHandler(ModificationHandler handler, const void *context);
	void  deregisterModificationHandler(ModificationHandler handler, const void *context);

protected:

	explicit MeshGenerator(const MeshGeneratorTemplate *meshGeneratorTemplate);
	virtual ~MeshGenerator();

	void  signalModified();

private:

	struct ModificationHandlerInfo;

	typedef stdvector<ModificationHandlerInfo>::fwd  ModificationHandlerInfoVector;

private:

	const MeshGeneratorTemplate *const  m_meshGeneratorTemplate;
	mutable int                         m_referenceCount;

	ModificationHandlerInfoVector      *m_modificationHandlerInfoVector;

private:

	// disabled
	MeshGenerator();
	MeshGenerator(const MeshGenerator&);
	MeshGenerator &operator =(const MeshGenerator&);

};


// ======================================================================
/**
 * Retrieve the reference count associated with this MeshGenerator
 * instance.
 *
 * @return  the reference count associated with this MeshGenerator
 *          instance.
 */

inline int MeshGenerator::getReferenceCount() const
{
	return m_referenceCount;
}

// ----------------------------------------------------------------------
/**
 * Increment the reference count associated with this instance.
 *
 * The caller should maintain a separate reference count for each
 * logically independent reference to the MeshGenerator instance
 * through a call to fetch(). When a reference is needed no longer, 
 * release() should be called.
 */

inline void MeshGenerator::fetch() const
{
	++m_referenceCount;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the MeshGeneratorTemplate instance associated with this
 * MeshGenerator instance.
 *
 * @return  the MeshGeneratorTemplate instance associated with this
 *          MeshGenerator instance.
 */

inline const MeshGeneratorTemplate &MeshGenerator::getMeshGeneratorTemplate() const
{
	return *NON_NULL(m_meshGeneratorTemplate);
}

// ======================================================================

#endif
