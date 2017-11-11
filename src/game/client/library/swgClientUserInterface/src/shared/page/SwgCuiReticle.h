//======================================================================
//
// SwgCuiReticle.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiReticle_H
#define INCLUDED_SwgCuiReticle_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UITypes.h"

class Camera;
class TangibleObject;
class CreatureObject;
class UIText;

//----------------------------------------------------------------------

class SwgCuiReticle :
public CuiMediator
{
public:
	                    SwgCuiReticle         (UIPage & page);

	float               getRotation           () const;
	void                setRotation           (float f);

	static bool         computeTargetAccuracy (const CreatureObject & player, const TangibleObject & defender, float range, float & accuracy);

protected:
	void                performActivate   ();
	void                performDeactivate ();
//	void                Notify            (UINotificationServer *notifyingObject, UIBaseObject *contextObject, UINotification::Code notificationCode);

private:
	                   ~SwgCuiReticle ();

	                    SwgCuiReticle ();
	                    SwgCuiReticle (const SwgCuiReticle &);
	SwgCuiReticle &     operator=        (const SwgCuiReticle &);

	UIPage *            m_pageOut;
	UIPage *            m_pageIn;
	UIPoint             m_inDiff;
	UIPoint             m_pageDiff;
	UIText *            m_text;
};

//======================================================================

#endif
