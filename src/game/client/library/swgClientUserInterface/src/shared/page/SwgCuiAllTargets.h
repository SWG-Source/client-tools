//======================================================================
//
// SwgCuiAllTargets.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAllTargets_H
#define INCLUDED_SwgCuiAllTargets_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"

#include "sharedObject/Object.h"
#include "UITypes.h"

class Appearance;
class CachedNetworkId;
class Camera;
class CreatureObject;
class SwgCuiStatusGround;
class SwgCuiReticle;
class TangibleObject;
class UIOpacityEffector;
class UIPage;
class UISizeEffector;
class UIText;
class Vector;

namespace SwgCuiAllTargetsNamespace
{
	class MyRenderer;
}

//----------------------------------------------------------------------

class SwgCuiAllTargets :
public CuiMediator
{
public:
	static bool isTargetInPlayerWeaponRange(Object const & target);

public:
	virtual void        updateOnRender();
	void                displayDamageText (const UIPoint & pt, bool leftRight, int h, int a, int m);
	
protected:
	                    SwgCuiAllTargets  (char const * const mediatorDebugName, UIPage & page);
						~SwgCuiAllTargets ();

	virtual void        performActivate   ();
	virtual void        performDeactivate ();
	virtual void        update            (float deltaTimeSecs);

private:
	                    SwgCuiAllTargets  ();
	                    SwgCuiAllTargets  (const SwgCuiAllTargets &);
	SwgCuiAllTargets &  operator=         (const SwgCuiAllTargets &);

	SwgCuiStatusGround * addStatus         (TangibleObject & tangible);
	void                updateStatusOpacity(CachedNetworkId const & id);
	void                update            (const Camera & camera);
	void                raiseReticle      (SwgCuiReticle & reticle);
	void                updateDamageText  ();
	virtual bool        isWaypointVisible (Vector & effectiveWaypointPosition_o, Camera const & camera, Appearance const * appearance) const =0;

	void removeUnusedStatusPages(bool const forceUpdate);
private:
	typedef stdvector<UISmartPointer<UIText> >::fwd         TextVector;
	typedef stdvector<UIPage *>::fwd                        PageVector;
	typedef stdmap<CachedNetworkId, UISmartPointer<SwgCuiStatusGround> >::fwd StatusMap;

	StatusMap *         m_statii;

	UIPage *            m_sampleStatus;

	UISize              m_sampleMinSize;
	UISize              m_sampleSizeDelta;

	UIPage *            m_sampleReticle;

	SwgCuiReticle *     m_reticleLookAt;
	SwgCuiReticle *     m_reticleCombat;

	SwgCuiReticle *     m_topReticle;

	UIPage *            m_sampleArrow;
	UIPage *            m_sampleWaypointArrow;
	UIPage *            m_arrowTarget;
	UIPage *            m_arrowCombat;

	TextVector *        m_textDamageVector;
	UIText *            m_textDamageSample;
	UIOpacityEffector * m_effectorDamageOpacity;
	UISizeEffector *    m_effectorDamageSize;

	SwgCuiAllTargetsNamespace::MyRenderer * m_myRenderer;

	int m_sceneType; // Game::SceneType
	float m_updateTimeDelta;
};

//======================================================================

#endif
