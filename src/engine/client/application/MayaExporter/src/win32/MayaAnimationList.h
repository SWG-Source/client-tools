// ======================================================================
//
// MayaAnimationList.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef MAYA_ANIMATION_LIST_H
#define MAYA_ANIMATION_LIST_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/CrcLowerString.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/LessPointerComparator.h"
#include <string>

class MDagPath;
class Messenger;
class MFnDependencyNode;
class MStatus;

// ======================================================================

class MayaAnimationList
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct AnimationInfo
	{
	public:

		struct LessFirstFrame
		{
			bool operator() (const AnimationInfo *lhs, const AnimationInfo *rhs) const;
			bool operator() (const AnimationInfo &lhs, const AnimationInfo &rhs) const;
		};

	public:

		explicit AnimationInfo(const char *animationName);
		AnimationInfo(const char *animationName, const char *exportNodeName, int firstFrameNumber, int lastFrameNumber);

	public:

		CrcLowerString  m_animationName;   // name of the animation

		std::string     m_exportNodeName;  // name of the node from which to export (must be the skeleton root or a node underneath it in the root's skeleton chain)
		int             m_firstFrame;      // first maya frame in the animation range
		int             m_lastFrame;       // last maya frame in the animation range

	private:
		// disabled
		AnimationInfo();
		//AnimationInfo(const AnimationInfo&);
		AnimationInfo &operator =(const AnimationInfo&);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef stdmap<const CrcLowerString*, AnimationInfo*, LessPointerComparator>::fwd  AnimationInfoMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	static void install(Messenger *newMessenger);
	static void remove();

public:

	MayaAnimationList(MDagPath &storageDagPath, MStatus *status);
	~MayaAnimationList();

	AnimationInfoMap       &getAnimationInfoMap();
	const AnimationInfoMap &getAnimationInfoMap() const;

	bool                    reloadMapFromStorage();
	bool                    saveMapToStorage();

private:

	bool                    deleteStorageEntries();
	bool                    findOrCreateAnimationInfo(const CrcLowerString &animationName, AnimationInfo **animationInfo);
	bool                    loadMapFromStorage();

private:

	AnimationInfoMap  *m_animationInfoMap;
	MFnDependencyNode *m_storageDependencyNode;

private:
	// disabled
	MayaAnimationList();
	MayaAnimationList(const MayaAnimationList&);
	MayaAnimationList &operator =(const MayaAnimationList&);
};

// ======================================================================

inline MayaAnimationList::AnimationInfoMap &MayaAnimationList::getAnimationInfoMap()
{
	return *m_animationInfoMap;
}

// ----------------------------------------------------------------------

inline const MayaAnimationList::AnimationInfoMap &MayaAnimationList::getAnimationInfoMap() const
{
	return *m_animationInfoMap;
}

// ----------------------------------------------------------------------
/**
 * Reload the map from the storage specified at construction time,
 * overwriting any contents in current map.
 *
 * This function allows the caller to overwrite the existing contents
 * of the animation info map.  The animation info is loaded from the
 * same Maya node that was specified upon construction of this object.
 */
inline bool MayaAnimationList::reloadMapFromStorage()
{
	return loadMapFromStorage();
}

// ======================================================================

inline bool MayaAnimationList::AnimationInfo::LessFirstFrame::operator() (const AnimationInfo *lhs, const AnimationInfo *rhs) const
{
	return lhs->m_firstFrame < rhs->m_firstFrame;
}

// ----------------------------------------------------------------------

inline bool MayaAnimationList::AnimationInfo::LessFirstFrame::operator() (const AnimationInfo &lhs, const AnimationInfo &rhs) const
{
	return lhs.m_firstFrame < rhs.m_firstFrame;
}

// ======================================================================

#endif
