// ======================================================================
//
// DetailAppearance.h
// Portions Copyright 1999 Bootprint Entertainment
// Portions Copyright 2000-2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DetailAppearance_H
#define INCLUDED_DetailAppearance_H

// ======================================================================

class DetailAppearanceTemplate;
class IndexedTriangleListShaderPrimitive;

namespace DPVS
{
	class Model;
}

#include "sharedObject/Appearance.h"

// ======================================================================

class DetailAppearance: public Appearance
{
public:

	static void  install();
	static void  setForceLowDetailLevels (bool forceLowDetailLevels);
	static bool  getForceLowDetailLevels ();
	static void  setForceHighDetailLevels (bool forceHighDetailLevels);
	static bool  getForceHighDetailLevels ();
	static void  setDetailLevelBias (float detailLevelBias);
	static float getDetailLevelBias ();
	static void  setDrawRadarShape (bool drawRadarShape);
	static bool  getDrawRadarShape ();
	static void  setDrawTestShape (bool drawTestShape);
	static bool  getDrawTestShape ();
	static void  setDrawWriteShape (bool drawWriteShape);
	static bool  getDrawWriteShape ();

	static bool  getFadeInEnabled();
	static void  setFadeInEnabled(bool enabled);

	static bool  getGlobalCrossFadeEnabled();
	static void  setGlobalCrossFadeEnabled(bool enabled);

public:

	explicit DetailAppearance(const DetailAppearanceTemplate *newDetailAppearanceTemplate);
	virtual  ~DetailAppearance();

	virtual const Extent *getExtent() const;
	virtual AxialBox const getTangibleExtent() const;

	virtual bool  isLoaded() const;
	virtual bool collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const;
	virtual bool implementsCollide() const;

	virtual float alter(float time);
	virtual void  setOwner(Object *newOwner);

	virtual void  setCustomizationData (CustomizationData *customizationData);
	virtual void  addCustomizationVariables (CustomizationData &customizationData) const;

	virtual void  addToWorld();
	virtual void  removeFromWorld();
	virtual void  preRender() const;
	virtual void  objectListCameraRender() const;

	//------------------------------------------------
	// User-controlled feature methods.
	// When UserControlled is set, the current LOD
	// is specified by the user and no distance-based
	// selection is performed.
	void setUserControlled(bool newUserControlled);
	bool getUserControlled() const;
	void toggleUserControlled();
	void incrementDetailLevel();
	void decrementDetailLevel();
	//------------------------------------------------

	int  getDetailLevelCount() const;
	int  getCurrentDetailLevel() const;

	bool getCrossFadeEnabled()                const { return m_crossFadeEnabled; }
	void setCrossFadeEnabled(bool enabled)          { m_crossFadeEnabled=enabled; }

	virtual void drawDebugShapes (DebugShapeRenderer *renderer) const;

	virtual void setTexture(Tag tag, const Texture &texture);

	const Appearance*               getAppearance (int detailLevel) const;

#ifdef _DEBUG
	virtual int  getPolygonCount () const;
	virtual void debugDump(std::string &result, int indentLevel) const;
#endif

	virtual const IndexedTriangleList* getRadarShape () const;

	virtual DetailAppearance       * asDetailAppearance();
	virtual DetailAppearance const * asDetailAppearance() const;

	virtual void setShadowBlobAllowed();

protected:

	virtual void onScaleModified(Vector const &oldScale, Vector const &newScale);

private:

	enum
	{
		AppearanceListCapacity = 8
	};

private:

	virtual DPVS::Object           *getDpvsObject() const;

	const DetailAppearanceTemplate* getDetailAppearanceTemplate() const;
	Appearance*                     _getAppearance (int detailLevel);

	
	int   _chooseDetailLevel() const; // returns LOD index or -1 on error.

	void  _alterFadeIn(float deltaTime);
	void  _alterCrossFade(float deltaTime);
	void  _alterShowLodZero();
	void  _alterBeginFade();
	void  _alterFinishFade();
	void  _endAppearanceFade(Appearance *appearance, bool enabled);
	void  _updateLodSettings(int lodIndex);
	void  _setDpvsWriteShape();

	bool  _canCrossFade() const;

	void  dataLint() const;

#ifdef _DEBUG
	void _checkState();
#endif

	DPVS::Model* fetchDpvsTestShape (DetailAppearanceTemplate const *appearanceTemplate);

private:

	// disabled
	DetailAppearance();
	DetailAppearance(const DetailAppearance&);
	DetailAppearance &operator =(const DetailAppearance&);

private:

	int                                  m_appearanceListSize;
	Appearance*                          m_appearanceList [AppearanceListCapacity];
	mutable bool                         m_setTextureList [AppearanceListCapacity];
	Tag                                  m_setTextureTag;
	const Texture*                       m_setTextureTexture;

	bool mutable                         m_setShadowBlobAllowedList[AppearanceListCapacity];
	bool                                 m_setShadowBlobAllowed;
	bool                                 m_crossFadeEnabled;

	mutable DPVS::Object*                m_dpvsObject;

	bool                                 m_userControlled;
	int8                                 m_alters;
	mutable int                          m_currentDetailLevel;
	mutable int                          m_nextDetailLevel;

	CustomizationData                   *m_customizationData;

	mutable float                        m_timer;
};

// ======================================================================
inline void DetailAppearance::setUserControlled(bool newUserControlled) { m_userControlled = newUserControlled; }
//-------------------------------------------------------------------
inline void DetailAppearance::toggleUserControlled()                    { m_userControlled = !m_userControlled; }
//-------------------------------------------------------------------
inline int  DetailAppearance::getCurrentDetailLevel()             const { return m_currentDetailLevel; }
//-------------------------------------------------------------------
inline bool DetailAppearance::getUserControlled()                 const { return m_userControlled; }
// ======================================================================

#endif
