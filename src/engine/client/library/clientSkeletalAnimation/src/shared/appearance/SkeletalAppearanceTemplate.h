// ======================================================================
//
// SkeletalAppearanceTemplate.h
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SkeletalAppearanceTemplate_H
#define INCLUDED_SkeletalAppearanceTemplate_H

// ======================================================================

#include "sharedObject/AppearanceTemplate.h"
#include <vector>

// ----------------------------------------------------------------------

class AnimationEnvironment;
class AnimationStatePath;
class Appearance;
class CrcLowerString;
class CrcString;
class Iff;
class LodDistanceTable;
class LogicalAnimationTableTemplate;
class MeshGeneratorTemplate;
class PersistentCrcString;
class SkeletalAppearance2;
class Skeleton;
class SkeletonTemplate;
class TransformAnimationController;
class TransformAnimationResolver;
class TransformNameMap;

// ======================================================================

class SkeletalAppearanceTemplate: public AppearanceTemplate
{
public:

	struct SkeletonTemplateInfo;

	typedef stdvector<Skeleton*>::fwd  SkeletonVector;

public:

	static void install();
	static void remove();

	static AppearanceTemplate *load(const char *name, Iff *iff);

public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// engine run-time interface
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	int                           getMeshGeneratorCount() const;
	const CrcLowerString         &getMeshGeneratorName(int index) const;

	int                           getSkeletonTemplateCount() const;
	const SkeletonTemplateInfo   &getSkeletonTemplateInfo(int index) const;

	const CrcLowerString         &getSkeletonTemplateName(const SkeletonTemplateInfo &skeletonTemplateInfo) const;
	bool                          skeletonTemplateHasAttachmentTransform(const SkeletonTemplateInfo &skeletonTemplateInfo) const;
	const CrcLowerString         &getSkeletonTemplateAttachmentTransformName(const SkeletonTemplateInfo &skeletonTemplateInfo) const;

	int                           getRootSkeletonTemplateDetailCount() const;
	void                          createSkeletonLods(SkeletonVector &skeletons, TransformAnimationResolver &animationResolver) const;

	const CrcLowerString         &lookupLatForSkeleton(CrcString const &skeletonPathName) const;
	const LodDistanceTable       *getLodDistanceTable() const;

	bool                          shouldCreateAnimationController() const;
	TransformAnimationController *createAnimationController(CrcString const &skeletonTemplateName, const TransformNameMap *layoutTransformNameMap, AnimationEnvironment &animationEnvironment, Appearance *ownerAppearance, int channel, AnimationStatePath const &initialPath) const;

	//-- this form used when creating an appearance not part of a skeletal LOD
	virtual Appearance           *createAppearance() const;

	virtual void                  preloadAssets() const;
	virtual void                  preloadAssetsLight() const;
	virtual void                  garbageCollect() const;

	// these should go away as soon as Ash is the only way to go.
	bool                          hasAsgController() const;
	bool                          hasAshController() const;

	bool                          mustUseSoftSkinning() const;
	bool                          getAlwaysPlayActionGeneratorAnimations() const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// tools construction interface
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	SkeletalAppearanceTemplate();
	virtual ~SkeletalAppearanceTemplate(); // -TRF- don't like this being public, the engine runtime interface is to release the pointer, not delete it

	int                         addMeshGenerator(const char *meshGeneratorName);
	void                        removeMeshGenerator(int index);

	int                         addSkeletonTemplate(const char *skeletonTemplateName, const char *attachmentTransformName);
	void                        removeSkeletonTemplate(int index);

	void                        setCreateAnimationController(bool createController);
	void                        setSktToLatMapping(const CrcLowerString &sktReferenceName, const CrcLowerString &latReferenceName);

#if 1
	// deprecated.  will go away soon.
	void                        setAnimationStateGraphTemplateName(const CrcLowerString &pathName);
	const CrcLowerString       &getAnimationStateGraphTemplateName() const;
#endif

	void                        write(Iff &iff) const;

#ifdef _DEBUG
	bool                        renameMgnToLmg();
#endif

private:

	typedef stdvector<CrcLowerString>::fwd                        CrcLowerStringVector;
	typedef stdvector<const MeshGeneratorTemplate*>::fwd          MeshGeneratorTemplateVector;
	typedef stdmap<PersistentCrcString, CrcLowerString>::fwd      NameMap;
	typedef stdvector<SkeletonTemplateInfo>::fwd                  SkeletonTemplateInfoVector;
	typedef stdvector<const SkeletonTemplate*>::fwd               SkeletonTemplateVector;
	typedef stdvector<const LogicalAnimationTableTemplate*>::fwd  LatVector;

private:

	SkeletalAppearanceTemplate(const char *name, Iff &iff);

	void  load_0001(Iff &iff);
	void  load_0002(Iff &iff);
	void  load_0003(Iff &iff);

	void  write_0002(Iff &iff) const;
	void  write_0003(Iff &iff) const;

	bool  hasMissingAssets() const;

	// Disabled.
	SkeletalAppearanceTemplate(const SkeletalAppearanceTemplate&);
	SkeletalAppearanceTemplate &operator =(const SkeletalAppearanceTemplate&);

private:

	static bool ms_installed;

private:

	CrcLowerStringVector        *m_meshGeneratorNames;
	SkeletonTemplateInfoVector  *m_skeletonTemplateInfo;

	bool                         m_createAnimationController;
	CrcLowerString              *m_animationStateGraphTemplateName;
	NameMap                     *m_sktToLatMap;

	LodDistanceTable            *m_lodDistanceTable;

	MeshGeneratorTemplateVector mutable  m_meshGenerators;
	SkeletonTemplateVector      mutable  m_skeletonTemplates;
	LatVector                   mutable  m_latVector;

	bool                         m_missingAssets;
	bool                         m_mustUseSoftSkinning;
	bool                         m_alwaysPlayActionGeneratorAnimations;

};

// ======================================================================

inline const LodDistanceTable *SkeletalAppearanceTemplate::getLodDistanceTable() const
{
	return m_lodDistanceTable;
}

// ======================================================================

#endif
