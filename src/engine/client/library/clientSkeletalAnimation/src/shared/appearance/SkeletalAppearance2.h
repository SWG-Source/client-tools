// ==================================================================
//
// SkeletalAppearance2.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ==================================================================

#ifndef Included_SkeletalAppearance_H
#define Included_SkeletalAppearance_H

// ==================================================================

#include "sharedCollision/BoxExtent.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "sharedObject/Appearance.h"
#include <vector>

class AnimationEnvironment;
class BoxExtent;
class CompositeMesh;
class CrcLowerString;
class CrcString;
class CustomizationData;
class MeshGenerator;
class MeshGeneratorTemplate;
class Object;
class Quaternion;
class ShaderPrimitive;
class MemoryBlockManager;
class ShaderTemplate;
class SkeletalAppearanceInstanceData;
class SkeletalAppearanceTemplate;
class Skeleton;
class Sphere;
class TransformAnimationController;
class TransformAnimationResolver;
class TransformModifier;
class Vector;

namespace DPVS
{
	class SphereModel;
}

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/CrcLowerString.h"

// ==================================================================

class SkeletalAppearance2: public Appearance
{
	friend class FullGeometrySkeletalAppearanceBatchRenderer;

	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	typedef stdvector<MeshGenerator*>::fwd       MeshGeneratorVector;
	typedef stdvector<MeshGeneratorVector>::fwd  MeshGeneratorVectorVector;
	typedef stdvector<Skeleton*>::fwd            SkeletonVector;
	typedef stdvector<int>::fwd                  IntVector;

	typedef void (*ContainsDestroyedAttachmentWearableCallback)(Object &object);

	class AttachedAppearance;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	static void install();

	static void   setDetailLevelBias(float lodBias);
	static float  getDetailLevelBias();

	static bool   getShowSkeleton();
	static void   setShowSkeleton(bool showIt);

	static void   setContainsDestroyedAttachmentWearableCallback(ContainsDestroyedAttachmentWearableCallback callback);

	static void   getMaximumDesiredDetailLevel(bool &enabled, int &lodIndex);
	static void   setMaximumDesiredDetailLevel(bool enabled, int lodIndex);

	static void   getBatchRenderScreenFraction(bool &enabled, float &screenFraction);
	static void   setBatchRenderScreenFraction(bool enabled,  float screenFraction);

	static void   setUiContextEnabled(bool enabled);
	static bool   getUiContextEnabled();


#ifdef _DEBUG
	static bool  getRenderTargetDirection();
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	explicit SkeletalAppearance2(const SkeletalAppearanceTemplate *appearanceTemplate);
	virtual ~SkeletalAppearance2(void);

	virtual SkeletalAppearance2 *       asSkeletalAppearance2();
	virtual SkeletalAppearance2 const * asSkeletalAppearance2() const;

	virtual bool collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo& result) const;
	virtual bool implementsCollide() const;
	virtual bool                        isLoaded() const;
	virtual const Extent               *getExtent(void) const;
	virtual AxialBox const getTangibleExtent() const;
	virtual const Sphere               &getSphere(void) const;

	bool                                collideForceGeometryUpdate(const Vector &start_o, const Vector &end_o, CollisionInfo& result) const;

	virtual float                       alter(float time);

	virtual void                        render() const;

	virtual void                        setCustomizationData(CustomizationData *customizationData);
	virtual void                        addCustomizationVariables(CustomizationData &customizationData) const;

	virtual bool                        findHardpoint(CrcString const &hardpointName, Transform &hardpointTransform) const;

	virtual void                        addToWorld();
	virtual void                        removeFromWorld();

	int                                 getSkeletonLodCount() const;
	const Skeleton                     &getSkeleton(int lodIndex) const;
	Skeleton                           &getSkeleton(int lodIndex);

	SkeletonVector                     &getSkeletons(void);
	const SkeletonVector               &getSkeletons(void) const;

	const Skeleton                     *getDisplayLodSkeleton() const;
	Skeleton                           *getDisplayLodSkeleton();

	int                                 getDetailLevelCount() const;
	bool                                isDetailLevelAvailable(int lodIndex) const;

	bool                                rebuildIfDirtyAndAvailable();
	void                                rebuildMesh(int lodIndex);

	void                                setShowMesh(bool showIt);
	void                                setShowAttachments(bool enabled);
	void                                setShowExactMeshExtent(bool showIt);

	bool                                getShowMesh() const;
	bool                                getShowAttachments() const;
	bool                                getShowExactMeshExtent() const;

	TransformAnimationController       *getAnimationController();
	const TransformAnimationController *getAnimationController() const;

	const TransformAnimationResolver   &getAnimationResolver() const;
	TransformAnimationResolver         &getAnimationResolver();

	void                                attach(Object *object, const CrcString &transformName);
	void                                detach(const Object *object);

	int                                 getAttachedAppearanceCount() const;
	Appearance                         *getAttachedAppearance(int index);
	const Appearance                   *getAttachedAppearance(int index) const;
	Object                             *getAttachedObject(int index);
	const Object                       *getAttachedObject(int index) const;
	const CrcString                    &getAttachedAppearanceTransformName(int index) const;

	void                                hideAttachment(int index, bool hide);
	bool                                isAttachmentHidden(int index) const;

	int                                 findAttachmentIndexByHardpoint(const CrcString &hardpointName) const;
	Object                             *findAttachedObject(const CrcString &hardpointName);

	void                                wear(Object *object);
	void                                stopWearing(const Object *object);

	void                                lockWearables();
	void                                unlockWearables();
	bool                                areWearablesLocked() const;

	int                                 getWearableCount() const;
	Object*								getWearableObject(int index);
	const Object                       *getWearableObject(int index) const;
	const SkeletalAppearance2          *getWearableAppearance(int index) const;
	SkeletalAppearance2                *getWearableAppearance(int index);

	const BoxExtent                    &getApproximateMeshExtent();
	const BoxExtent                    &getExactMeshExtent();

	void                                overrideSkeletonNextRender(const SkeletonVector &overridingSkeletons);

	CustomizationData                  *fetchCustomizationData();

	void                                setDesiredVelocity(const Vector &velocityRelativeToSelf);
	Vector const                       &getDesiredVelocity() const;

	void                                getObjectLocomotion(Quaternion &rotation, Vector &translation, float elapsedTime) const;

	void                                setTargetObject(const Object *object);
	void                                setTargetObjectAndHardpoint(const Object *targetObject, CrcLowerString const &hardpointName);
	Object const                       *getTargetObject() const;

	void                                setYawDirection(int direction);

	AnimationEnvironment               &getAnimationEnvironment();

	// these should go away as soon as Ash is the only way to go.
	bool                                hasAsgController() const;
	bool                                hasAshController() const;

	float                               getUniformScale() const;

	void                                setUserControlledDetailLevel(bool userControlled);
	bool                                getUserControlledDetailLevel() const;

	void                                incrementDetailLevel();
	void                                decrementDetailLevel();
	void                                setDetailLevel(int index);
	int                                 getDisplayLodIndex() const;

	void                                clearAllTransformModifiers();
	void                                addTransformModifierTakeOwnership(CrcString const &transformName, TransformModifier *modifier);

	void                                setPlannedLodIndex(int plannedLodIndex);
	int                                 getPlannedLodIndex() const;

	void                                setEveryOtherFrameSkinningEnabled(bool enabled);
	bool                                getEveryOtherFrameSkinningEnabled() const;

	void                                setForceHardSkinningEnabled(bool enabled);
	bool                                getForceHardSkinningEnabled() const;

	bool                                isTargetPositionValid () const;
	const Vector &                      getTargetPosition_w () const;
	void                                setTargetPosition_w (const Vector & v);
	const Vector &                      getTargetPosition_o () const;

	// This is in object space.
	Vector const & getDirectionToTarget() const;

	void                                setExtentDelegateObject (const Object * obj);
	const BoxExtent &                   getDelegateBoxExtent (bool exactMeshExtents) const;

	enum FadeState
	{
		FS_uninitialized,
		FS_waitingForLoadCompletion,
		FS_fadingIn,
		FS_notFading,
		FS_fadeToHold,
		FS_hold,
		FS_fadeOutToRemove,
		FS_removed
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	// Note: The hold parameter is currently used in states that require the hold value.
	// all other states ignore the hold value to preserve functionality.
	void setFadeState(FadeState const fadeState, float const fadeFractionStart, float const fadeHold = 0.0f);
	FadeState getFadeState() const;
	float getFadeFraction() const;

	void setIsBlueGlowie(bool b);
	void setIsHolonet(bool b);

	bool getIsBlueGlowie() const;
	bool getIsHolonet() const;

	void markAsDirty();

	

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum HologramType
	{
		HT_none = -1,
		HT_type1_quality1, // Blue Green
		HT_type1_quality2,
		HT_type1_quality3,
		HT_type1_quality4,

		HT_type2_quality1, // Purple
		HT_type2_quality2,
		HT_type2_quality3,
		HT_type2_quality4,

		HT_type3_quality1, // Orange
		HT_type3_quality2,
		HT_type3_quality3,
		HT_type3_quality4,

		HT_num_types
	};
	void setHologramType(HologramType holoType);
	HologramType getHologramType() const;
    
    static const int m_maxShaderSize = 9;
	bool isWearing(Object const * object);

protected:

	virtual void                        onScaleModified(Vector const &oldScale, Vector const &newScale);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	class AttachedTransformModifier;
	class SkeletonSegmentDescriptor;

	typedef stdvector<AttachedAppearance*>::fwd           AttachedAppearanceVector;
	typedef stdvector<AttachedTransformModifier*>::fwd    AttachedTransformModifierVector;
	typedef stdvector<bool>::fwd                          BoolVector;
	typedef stdvector<const MeshGeneratorTemplate*>::fwd  MeshGeneratorTemplateVector;
	typedef stdvector<ShaderPrimitive*>::fwd              ShaderPrimitiveVector;
	typedef stdvector<ShaderPrimitiveVector>::fwd         ShaderPrimitiveVectorVector;
	typedef Watcher<Object> ObjectWatcher;
	typedef stdvector<ObjectWatcher>::fwd                 WatcherObjectVector;
	typedef std::pair<ObjectWatcher, WatcherObjectVector> ObjectWatcherVectorPair;
	typedef stdvector<ObjectWatcherVectorPair>::fwd WatcherObjectVectorVector;
	typedef stdvector<SkeletonSegmentDescriptor>::fwd     SkeletonSegmentDescriptorVector;
	typedef stdset<SkeletonSegmentDescriptor>::fwd        SkeletonSegmentDescriptorSet;
	typedef stdvector<Vector>::fwd                        VectorVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	static void           remove();

	static void           getAppearanceTemplateSkeletons(const SkeletalAppearanceTemplate &appearanceTemplate, SkeletonSegmentDescriptorSet &descriptorSet, SkeletonSegmentDescriptorVector &descriptorVector);
	static void           meshGeneratorModifiedCallback(const void *context, MeshGenerator &meshGenerator);

	static CompositeMesh &getCompositeMesh();
	virtual DPVS::Object *getDpvsObject() const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	void                              lookupAttachmentTransformIndices(int detailLevel);

	MeshGeneratorVector const        &getOwnedMeshGeneratorsForDetailLevel(int lodIndex) const;
	void                              fetchOwnedMeshGeneratorsForDetailLevel(const SkeletalAppearanceTemplate &appearanceTemplate, int lodIndex, MeshGeneratorVector &meshGenerators);
	bool                              areAllMeshGeneratorsReadyForDetailLevel(int lodIndex) const;
	bool                              areShaderPrimitivesReadyForDetailLevel(int lodIndex) const;

	void                              buildCompositeMesh(CompositeMesh &compositeMesh, int lodIndex) const;
	void                              addSkeletonSegments(Skeleton &skeleton, int lodIndex);

	void                              notifyMeshGeneratorModified();

	const ShaderPrimitiveVector      &getDisplayLodShaderPrimitives() const;
	ShaderPrimitiveVector            &getDisplayLodShaderPrimitives();

	const SkeletalAppearanceTemplate &getSkeletalAppearanceTemplate() const;

	int                               calculateDisplayLodIndex(float diameterScreenFraction, int previousLodIndex) const;

	void                              removeDeletedWearables();


	void                              calculateExtentDeltas(int lodIndex, Vector &minDelta, Vector &maxDelta) const;
	void                              getNoAttachmentExtent(BoxExtent &extent, int lodIndex) const;
	void                              updateExtentWithLod(BoxExtent &extent, int lodIndex) const;

	bool                              rebuildIfDirtyAndAvailable(int lodIndex);
	bool                              rebuildOrAdjustDisplayLodIndex();
	void                              unloadUnusedResources();

	void                              handleFade(float elapsedTime);

	void                              updateDpvsTestObjectWithExtents() const;

	void                              updateTargetPosition();


	// Disabled.
	SkeletalAppearance2(void);
	SkeletalAppearance2(const SkeletalAppearance2&);
	SkeletalAppearance2 &operator =(const SkeletalAppearance2&);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	static bool                               ms_installed;
	static float                              ms_lodBias;

	static CompositeMesh                     *ms_compositeMesh;

#ifdef _DEBUG
	static bool                               ms_renderTargetDirection;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	AnimationEnvironment                     *m_animationEnvironment;

	/// Each entry in the vector is a Skeleton instance for a given LOD level.
	SkeletonVector                     *const m_skeletons;

	/// Each entry in the vector is a vector of MeshGenerator instances for a given LOD level.
	MeshGeneratorVectorVector          *const m_ownedMeshGenerators;

	CustomizationData                        *m_customizationData;

	bool                                      m_showMesh;
	bool                                      m_showAttachments;

	TransformAnimationResolver               *m_animationResolver;
	mutable const TransformAnimationResolver *m_overridingAnimationResolver;

	ShaderPrimitiveVectorVector              *m_perLodShaderPrimitives;
	mutable IntVector                         m_perLodMruFrameVector;

	AttachedAppearanceVector                 *m_attachedAppearances;
	WatcherObjectVectorVector                *m_wornAppearanceObjects;
	int                                      m_wearablesLockedFrame;

	/// Each entry in the vector represents whether the ShaderPrimitive data for that LOD index needs to be rebuilt.
	BoolVector																m_appearanceDirty;
	VectorVector                              m_perLodMeshExtentMinDelta;
	VectorVector                              m_perLodMeshExtentMaxDelta;	

	mutable BoxExtent                        *m_meshExtent;
	bool                                      m_showMeshExtent;

	mutable int                               m_displayLodIndex;
	mutable bool                              m_lodIsReady;
	bool                                      m_userControlledDetailLevel;
	int                                       m_maxAvailableDetailLevelIndex;

	Vector                                   &m_animationLocomotionVelocity;
	Vector                                   &m_directionToTarget;
	ConstWatcher<Object>                      m_targetObject;
	mutable bool                              m_targetPositionValid;
	Vector                                    m_targetPosition_w;
	Vector                                    m_targetPosition_o;

	mutable BoxExtent                         m_extent;

	float                                    &m_scale;
	int                                      &m_yawDirection;

	DPVS::Object mutable                     *m_dpvsObject;

	AttachedTransformModifierVector          *m_attachedTransformModifiers;

	mutable FadeState                         m_fadeState;
	mutable float                             m_fadeFraction;
	float                                     m_fadeHold;

	mutable bool                              m_mostRecentRenderUsedBatch;

	int                                       m_plannedLodIndex;
	int                                       m_plannedLodSetFrameNumber;
	bool                                      m_everyOtherFrameSkinningEnabled;
	bool                                      m_forceHardSkinningEnabled;

	ConstWatcher<Object>                      m_extentDelegateObject;
	mutable BoxExtent                         m_extentDelegateTransformed;

	Timer m_unloadUnusedResourcesTimer;

	CrcLowerString                            m_targetHardpoint;

	bool                                      m_isBlueGlowie;
	bool                                      m_isHolonet;
	HologramType							  m_hologramType;
	int32									  m_blackHologramFrame;

	mutable ShaderTemplate const *            m_blueGlowieShaderTemplate;
	mutable ShaderTemplate const *            m_blueGlowieBumpShaderTemplate;

	mutable ShaderTemplate const *			  m_holoShaderTemplate[m_maxShaderSize];	
    mutable ShaderTemplate const *            m_holonetShaderTemplate;
	mutable ShaderTemplate const *            m_holonetBumpShaderTemplate;
};

// ==================================================================

#ifdef _DEBUG

inline bool SkeletalAppearance2::getRenderTargetDirection()
{
	return ms_renderTargetDirection;
}

#endif

// ======================================================================

inline SkeletalAppearance2::SkeletonVector &SkeletalAppearance2::getSkeletons(void)
{
	return *m_skeletons;
}

// ------------------------------------------------------------------

inline const SkeletalAppearance2::SkeletonVector &SkeletalAppearance2::getSkeletons(void) const
{
	return *m_skeletons;
}

// ----------------------------------------------------------------------

inline void SkeletalAppearance2::setShowMesh(bool showIt)
{
	m_showMesh = showIt;
}

// ----------------------------------------------------------------------

inline void SkeletalAppearance2::setShowAttachments(bool enable)
{
	m_showAttachments = enable;
}

// ----------------------------------------------------------------------

inline void SkeletalAppearance2::setShowExactMeshExtent(bool showIt)
{
	m_showMeshExtent = showIt;
}

// ----------------------------------------------------------------------

inline bool SkeletalAppearance2::getShowMesh() const
{
	return m_showMesh;
}

// ----------------------------------------------------------------------

inline bool SkeletalAppearance2::getShowAttachments() const
{
	return m_showAttachments;
}

// ----------------------------------------------------------------------

inline bool SkeletalAppearance2::getShowExactMeshExtent() const
{
	return m_showMeshExtent;
	
}

// ----------------------------------------------------------------------

inline const TransformAnimationResolver &SkeletalAppearance2::getAnimationResolver() const
{
	NOT_NULL(m_animationResolver);
	return *m_animationResolver;
}

// ----------------------------------------------------------------------

inline TransformAnimationResolver &SkeletalAppearance2::getAnimationResolver()
{
	NOT_NULL(m_animationResolver);
	return *m_animationResolver;
}

// ----------------------------------------------------------------------

inline AnimationEnvironment &SkeletalAppearance2::getAnimationEnvironment()
{
	NOT_NULL(m_animationEnvironment);
	return *m_animationEnvironment;
}

// ----------------------------------------------------------------------

inline float SkeletalAppearance2::getUniformScale() const
{
	return m_scale;
}

// ----------------------------------------------------------------------

inline void SkeletalAppearance2::setUserControlledDetailLevel(bool userControlled)
{
	m_userControlledDetailLevel = userControlled;
}

// ----------------------------------------------------------------------

inline bool SkeletalAppearance2::getUserControlledDetailLevel() const
{
	return m_userControlledDetailLevel;
}

// ----------------------------------------------------------------------

inline int SkeletalAppearance2::getDisplayLodIndex() const
{
	return m_displayLodIndex;
}

//----------------------------------------------------------------------

inline bool SkeletalAppearance2::isTargetPositionValid () const
{
	return m_targetPositionValid;
}

//----------------------------------------------------------------------

inline const Vector & SkeletalAppearance2::getTargetPosition_w () const
{
	return m_targetPosition_w;
}

//----------------------------------------------------------------------

inline const Vector & SkeletalAppearance2::getTargetPosition_o () const
{
	return m_targetPosition_o;
}

//----------------------------------------------------------------------

inline Vector const & SkeletalAppearance2::getDirectionToTarget() const
{
	return m_directionToTarget;
}

//----------------------------------------------------------------------

inline bool SkeletalAppearance2::getIsBlueGlowie() const
{
	return m_isBlueGlowie;
}

//----------------------------------------------------------------------

inline bool SkeletalAppearance2::getIsHolonet() const
{
	return m_isHolonet;
}

//----------------------------------------------------------------------

inline SkeletalAppearance2::HologramType SkeletalAppearance2::getHologramType() const
{
	return m_hologramType;
}

// ==================================================================

#endif
