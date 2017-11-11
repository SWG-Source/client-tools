//===================================================================
//
// ComponentAppearance.h
// copyright 2000-2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
//===================================================================

#ifndef INCLUDED_ComponentAppearance_H
#define INCLUDED_ComponentAppearance_H

//===================================================================

#include "sharedObject/Appearance.h"

class ComponentAppearanceTemplate;
class ObjectList;

namespace DPVS
{
	class Model;
};

//===================================================================

class ComponentAppearance : public Appearance
{
public:

	explicit ComponentAppearance(const ComponentAppearanceTemplate* componentAppearanceTemplate);
	virtual ~ComponentAppearance();

	virtual bool              isLoaded() const;
	virtual bool collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const;
	virtual bool implementsCollide() const;
	virtual const Extent     *getExtent() const;
	virtual AxialBox const getTangibleExtent() const;
	virtual float             alter(float time);
	virtual void              setAlpha(bool opaqueEnabled, float opaqueAlpha, bool alphaEnabled, float alphaAlpha);

	virtual void              setOwner(Object *newOwner);

	virtual DPVS::Object *    getDpvsObject() const;
	virtual void              addToWorld();
	virtual void              removeFromWorld();
	virtual void              render() const;
	virtual void              objectListCameraRender() const;

	virtual void              setTexture(Tag tag, const Texture &texture);

	virtual const IndexedTriangleList* getRadarShape () const;

	virtual ComponentAppearance *       asComponentAppearance();
	virtual ComponentAppearance const * asComponentAppearance() const;

	virtual void setShadowBlobAllowed();

	ObjectList const & getObjectList() const;

#ifdef _DEBUG
	virtual int  getPolygonCount () const;
	virtual void debugDump(std::string &result, int indentLevel) const;
#endif

private:

	ComponentAppearance();
	ComponentAppearance(const ComponentAppearance&);
	ComponentAppearance& operator=(const ComponentAppearance&);

	const ComponentAppearanceTemplate *getComponentAppearanceTemplate() const;
	BoxExtent*                         createExtent();
	void                               calculateExtent();

	DPVS::Model*                       fetchDpvsTestModel(const ComponentAppearanceTemplate* componentAppearanceTemplate);

private:

	bool                 m_needExtentCalculated;
	DPVS::Object *       m_dpvsObject;
	mutable ObjectList * m_objectList;
};

//===================================================================

#endif
