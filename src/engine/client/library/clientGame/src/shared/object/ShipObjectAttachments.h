//======================================================================
//
// ShipObjectAttachments.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipObjectAttachments_H
#define INCLUDED_ShipObjectAttachments_H

//======================================================================

#include "sharedMath/Vector.h"

template <typename T> class Watcher;
class ShipObject;
class Transform;
class Object;
class Vector;

//----------------------------------------------------------------------


class ShipObjectAttachments
{
public:
	explicit ShipObjectAttachments(ShipObject & ship);
	~ShipObjectAttachments();

	typedef Watcher<Object> ObjectWatcher;
	typedef stdvector<ObjectWatcher>::fwd   WatcherVector;
	typedef stdvector<Transform>::fwd TransformList;
	typedef stdmap<int, WatcherVector>::fwd WatcherVectorMap;

	typedef std::pair<Transform, Object *> TransformObjectPair;
	typedef stdvector<TransformObjectPair>::fwd TransformObjectList;

	WatcherVector const * getComponentAttachments(int chassisSlot) const;
	WatcherVectorMap const * getAllComponentAttachments() const;
	void updateComponentStatesForSlot(int chassisSlot);
	Object *getFirstAttachedObjectForWeapon(int weaponIndex, Transform &hardpointTransform_o2p);
	bool getFirstComponentPosition_w(int const chassisSlot, Vector &componentPosition_w) const;
	bool getFirstComponentPosition_o(int const chassisSlot, Vector &componentPosition_o) const;
	Object const *getFirstAttachedObjectForWeapon(int weaponIndex, Transform &hardpointTransform_o2p) const;
	void handleComponentDestruction(int chassisSlot, float severity);
	void removeComponentAttachments (int chassisSlot);
	bool findAttachmentHardpoints_o (int chassisSlot, std::string const & hardpointPrefixString, Transform const & parentTransform_w, TransformObjectList & transformObjectList) const;
	void updateComponentAttachment (int chassisSlot);
	Vector const & getAverageProjectileMuzzlePosition_p() const;
	float getAverageProjectileRange() const;
	void recalculateAverageProjectileData() const;

private:

	ShipObjectAttachments(ShipObjectAttachments const & rhs);
	ShipObjectAttachments & operator=(ShipObjectAttachments const & rhs);

private:

	ShipObject * m_ship;
	WatcherVectorMap * m_componentAttachments;
	WatcherVectorMap * m_componentDestroyedParticles;

	mutable Vector m_averageProjectileMuzzlePosition_p;
	mutable float  m_averageProjectileRange;
	mutable bool m_averageProjectileDataDirty;
};

//======================================================================

#endif
