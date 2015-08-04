//======================================================================
//
// CuiTransition.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiTransition_H
#define INCLUDED_CuiTransition_H

//======================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"

class UIWidget;

// ======================================================================

/**
* CuiTransition
*/

class CuiTransition :
public CuiMediator
{
public:

	enum State
	{
		S_pre,
		S_wait,
		S_post
	};

	enum Type
	{
		T_horizontal
	};

	struct DoorData
	{
		UIWidget * widget;
		UIPoint    targetStart;
		UIPoint    targetWait;
		bool       finished;
	};

	explicit                      CuiTransition (UIPage & page);

	void                          setTransitionPrevious (const std::string & mediatorName);
	void                          setTransitionNext     (const std::string & mediatorName);
	
	void                          onTransitionReady     (const std::string & mediatorName);

	static void                   signalTransitionReady (const std::string & mediatorName);
	static void                   startTransition       (const std::string & from, const std::string & to);

	void                          update                (float deltaTimeSecs);

	static bool                   isTransitioning       ();

protected:

	virtual void                  performActivate   ();
	virtual void                  performDeactivate ();

private:
	                             ~CuiTransition ();
	                              CuiTransition ();
	                              CuiTransition (const CuiTransition & rhs);
	CuiTransition &               operator=      (const CuiTransition & rhs);

	void                          reset ();

private:

	std::string                   m_mediatorNamePrevious;
	std::string                   m_mediatorNameNext;

	float                         m_elapsedTime;
	Type                          m_type;
	State                         m_state;

	UIWidget *                    m_doorLeft;
	UIWidget *                    m_doorRight;

	UIRect                        m_doorLeftMargin;
	UIRect                        m_doorRightMargin;

	float                         m_horizontalSpeed;

	typedef stdvector<DoorData>::fwd DoorDataVector;
	DoorDataVector *              m_doorDataVector;

	bool                          m_skipNextUpdate;

	float                         m_countownToWaitTransition;

	std::string                   m_soundStart;
	std::string                   m_soundLoop;
	std::string                   m_soundStopClose;
	std::string                   m_soundStopOpen;

};

//======================================================================

#endif

