//======================================================================
//
// CuiTurntableAdapter.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiTurntableAdapter_H
#define INCLUDED_CuiTurntableAdapter_H

//======================================================================

#include "UINotification.h"

class CuiWidget3dObjectListViewer;
class Light;
class Object;

//----------------------------------------------------------------------

class CuiTurntableAdapter
{
public:

	                      CuiTurntableAdapter   (CuiWidget3dObjectListViewer & viewer);
	                     ~CuiTurntableAdapter   ();

	void                  arrange               ();
	void                  turnTo                (int index, bool playSound);
	bool                  isTurning             () const;
	void                  setBaseObjectRotation (float radians);
	void                  setAutoRotateObjects  (bool b);

	Light &               getSpotLight          ();

	int                   getCurrentIndex       () const;
	Object *              getCurrentObject      ();

	void                  setSpacingModifier    (float f);
	void                  setTurnToFace         (bool b);
	void                  update                (float deltaTimeSecs);

private:

	CuiTurntableAdapter & operator=             (const CuiTurntableAdapter &);
	                      CuiTurntableAdapter   (const CuiTurntableAdapter &);

	typedef stdvector<Object *>::fwd ObjectVector;
	typedef stdvector<float>::fwd    AngleVector;

	void                  getObjectVector       (ObjectVector & ov);
	void                  updateObjectRotations ();

	CuiWidget3dObjectListViewer * m_viewer;
	AngleVector *                 m_angles;

	bool                          m_isTurning;
	float                         m_baseObjectRotation;
	bool                          m_autoRotateObjects;

	int                           m_currentIndex;
	float                         m_previousAngle;

	Light *                       m_spotLight;

	float                         m_arrangementRadius;
	float                         m_appearanceRadius;

	float                         m_spacingModifier;

	bool                          m_turnToFace;
};

//----------------------------------------------------------------------

inline bool CuiTurntableAdapter::isTurning () const
{
	return m_isTurning;
}

//----------------------------------------------------------------------

inline Light & CuiTurntableAdapter::getSpotLight ()
{
	return *NON_NULL (m_spotLight);
}

//----------------------------------------------------------------------

inline int CuiTurntableAdapter::getCurrentIndex       () const
{
	return m_currentIndex;
}

//======================================================================

#endif
