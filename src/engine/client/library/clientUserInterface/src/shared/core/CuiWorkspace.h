//======================================================================
//
// CuiWorkspace.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiWorkspace_H
#define INCLUDED_CuiWorkspace_H

//======================================================================

#include "sharedMessageDispatch/Receiver.h"
#include "UITypes.h"
#include "UIEventCallback.h"
#include "UINotification.h"

class type_info;

class CuiMediator;
class CuiWorkspaceGlassPane;
class CuiWorkspaceIcon;
class UIComposite;
class UIOpacityEffector;
class UIPage;

//----------------------------------------------------------------------

/**
* The CuiWorkspace provides management of movable/resizable/minimizable
* CuiMediators on a given page.  The workspace also manages icons and
* their transisions to and from full CuiMediators.
*
*/

class CuiWorkspace : 
public MessageDispatch::Receiver, 
public UINotification, 
public UIEventCallback
{
public:

	                      CuiWorkspace         (UIPage & page, bool toggleMediatorsEnabled);
	                     ~CuiWorkspace         ();

	void                  addMediator          (CuiMediator & mediator);
	void                  removeMediator       (CuiMediator & mediator);

	virtual void          receiveMessage       (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	void                  Notify               (UINotificationServer *notifyingObject, UIBaseObject *contextObject, Code notificationCode );

	bool                  iconify              (CuiMediator & mediator);
	void                  moveIcon             (CuiWorkspaceIcon & icon, const UIPoint & pos);

	void                  restoreFromIcon      (UIWidget * icon, CuiMediator & mediator);
	virtual bool          OnMessage            (UIWidget * context, const UIMessage & msg );
	virtual void          OnWidgetRectChanging (UIWidget * context, UIRect & targetRect);
	virtual void          OnWidgetRectChanged  (UIWidget * context);


	const UIRect          findClosestWidgetEdges  (const UIWidget * context,  const UIRect & targetRect) const;
	const UIRect          pushRectToWindowEdges   (const UIRect & targetRect, const UIRect & closestEdges) const;
	const UIRect          autoPositionMovingRect  (UIWidget * context,  const UIRect & targetRect, bool squash) const;

	void                  restore              (CuiMediator & mediator);
	void                  maximize             (CuiMediator & mediator);
	void                  close                (CuiMediator & mediator);

	void                  focusMediator        (CuiMediator & mediator, bool b);

	void                  setEnabled           (bool b);
	bool                  getEnabled           () const;

	const UIPage &        getPage              () const;
	UIPage &              getPage              ();

	const UIComposite &   getComposite         () const;
	UIComposite &         getComposite         ();

	static void           setGameWorkspace     (CuiWorkspace * workspace);
	static CuiWorkspace * getGameWorkspace     ();

	bool                  hasMediator          (const CuiMediator & mediator) const;

	CuiMediator *         findMediatorByType   (const type_info & info);
	CuiMediator *         findMediatorByPage   (const UIPage & page);
	CuiMediator *         focusTopMediator     (const CuiMediator * ignoreMediator);

	void                  updateMediatorEnabledStates (bool force);

	void                  positionMediator     (CuiMediator & mediator, const UIPoint & pt, bool squash = false);
	void                  positionMediator     (CuiMediator & mediator);

	void                  updateOpacities      (float opacityEnabled, float opacityDisabled);

	void                  setEffectors         (UIOpacityEffector * enableEffector, UIOpacityEffector * disableEffector);

	bool                  canAddOrRemoveMediator () const;

	float                 getOpacityEnabled    () const;
	float                 getOpacityDisabled   () const;

	void                  updateGlow           ();
	void                  updateGlowRect       () const;
	void                  update               (float deltaTimeSecs);

	void                  cancelFocusForMediator (const CuiMediator & mediator);

	void                  resetAllToDefaultSizeAndLocation ();

	void                  saveAllSettings() const;

	CuiMediator * getFocusMediator() const;
	void setShowGlow(bool glow);
	bool getShowGlow() const;
	void focusGlowMediatorByPage(UIPage & page);

private:
	CuiWorkspace &        operator=            (const CuiWorkspace & rhs);
	                      CuiWorkspace         (const CuiWorkspace & rhs);

private:

	typedef stdset<CuiMediator *>::fwd MediatorSet;
	struct IconAnimation
	{
		UIRect curRect;
		UIRect endRect;
		IconAnimation (const UIRect & start, const UIRect & end) : curRect (start), endRect (end) {}
	};

	typedef stdvector<IconAnimation>::fwd IconAnimationVector;
	typedef stdmap<UIWidget *, CuiMediator *>::fwd   IconMediatorMap;
	typedef stdmap<UIWidget *, CuiMediator *>::fwd   WidgetMediatorMap;

	bool                     m_enabled;

	UIPage &                 m_page;

	MediatorSet *            m_mediators;

	CuiWorkspaceGlassPane *  m_glassPane;

	IconAnimationVector *    m_iconAnimations;

	IconMediatorMap *        m_iconMediatorMap;

	bool                     m_toggleMediatorsEnabled;

	static CuiWorkspace *    ms_gameWorkspace;

	CuiMediator *            m_raiseMediator;

	float                    m_defaultOpacityEnabled;
	float                    m_defaultOpacityDisabled;

	UIOpacityEffector *      m_opacityEffectorEnabled;
	UIOpacityEffector *      m_opacityEffectorDisabled;

	bool                     m_iteratingEnabledStates;

	UIWidget *               m_glowFocused;
	CuiMediator *            m_mediatorFocused;

	bool                     m_showGlow;
};

//----------------------------------------------------------------------

inline bool CuiWorkspace::getEnabled () const
{
	return m_enabled;
}

//----------------------------------------------------------------------

inline const UIPage & CuiWorkspace::getPage () const
{
	return m_page;
}

//----------------------------------------------------------------------

inline UIPage & CuiWorkspace::getPage ()
{
	return m_page;
}

//----------------------------------------------------------------------

inline bool CuiWorkspace::canAddOrRemoveMediator () const
{
	return !m_iteratingEnabledStates;
}

//----------------------------------------------------------------------

inline float CuiWorkspace::getOpacityEnabled    () const
{
	return m_defaultOpacityEnabled;
}

//----------------------------------------------------------------------

inline float CuiWorkspace::getOpacityDisabled   () const
{
	return m_defaultOpacityDisabled;
}

//======================================================================

#endif
