//======================================================================
//
// CuiTransition.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiTransition.h"

#include "UIData.h"
#include "UIPage.h"
#include "clientAudio/Audio.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMediatorTypes.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedMath/Vector.h"

#include <vector>

//----------------------------------------------------------------------

namespace
{
	bool s_debugFlagsRegistered = false;
	CuiTransition * s_transition = 0;

	SoundId     m_sidStart;
	SoundId     m_sidLoop;
	SoundId     m_sidStopClose;
	SoundId     m_sidStopOpen;

	bool s_enabled = false;
}

//======================================================================

CuiTransition::CuiTransition (UIPage & page) :
CuiMediator                ("CuiTransition", page),
m_mediatorNamePrevious     (),
m_mediatorNameNext         (),
m_elapsedTime              (0.0f),
m_type                     (T_horizontal),
m_state                    (S_pre),
m_doorLeft                 (0),
m_doorRight                (0),
m_doorLeftMargin           (),
m_doorRightMargin          (),
m_horizontalSpeed          (1000.0f),
m_doorDataVector           (new DoorDataVector),
m_skipNextUpdate           (false),
m_countownToWaitTransition (0.0f),
m_soundStart               (),
m_soundLoop                (),
m_soundStopClose           (),
m_soundStopOpen            ()
{

	if (!s_debugFlagsRegistered)
	{
		DebugFlags::registerFlag(s_enabled,       "ClientUserInterface", "transitions");
		s_debugFlagsRegistered = true;
	}

	getCodeDataObject (TUIWidget,       m_doorLeft,        "doorLeft");
	getCodeDataObject (TUIWidget,       m_doorRight,       "doorRight");

	const UIData * const codeData = getCodeData ();
	NOT_NULL (codeData);

	if (codeData)
	{
		static const UILowerString prop_doorLeftMargin  = UILowerString ("DoorLeftMargin");
		static const UILowerString prop_doorRightMargin = UILowerString ("DoorRightMargin");
		static const UILowerString prop_horizontalSpeed = UILowerString ("HorizontalSpeed");
		static const UILowerString prop_soundStart      = UILowerString ("soundStart");
		static const UILowerString prop_soundLoop       = UILowerString ("soundLoop");
		static const UILowerString prop_soundStopClose  = UILowerString ("soundStopClose");
		static const UILowerString prop_soundStopOpen   = UILowerString ("soundStopOpen");

		codeData->GetPropertyRect   (prop_doorLeftMargin,  m_doorLeftMargin);
		codeData->GetPropertyRect   (prop_doorRightMargin, m_doorRightMargin);
		codeData->GetPropertyFloat  (prop_horizontalSpeed, m_horizontalSpeed);

		codeData->GetPropertyNarrow (prop_soundStart,     m_soundStart);
		codeData->GetPropertyNarrow (prop_soundLoop,      m_soundLoop);
		codeData->GetPropertyNarrow (prop_soundStopClose, m_soundStopClose);
		codeData->GetPropertyNarrow (prop_soundStopOpen,  m_soundStopOpen);

		if (m_soundStart == ".")
			m_soundStart.clear ();
		if (m_soundLoop == ".")
			m_soundLoop.clear ();
		if (m_soundStopClose == ".")
			m_soundStopClose.clear ();
		if (m_soundStopOpen == ".")
			m_soundStopOpen.clear ();
	}

	if (!s_transition)
		s_transition = this;
}

//----------------------------------------------------------------------

CuiTransition::~CuiTransition ()
{
	delete m_doorDataVector;
	m_doorDataVector = 0;

	if (s_transition == this)
		s_transition = 0;
}

//----------------------------------------------------------------------

void CuiTransition::performActivate   ()
{
	setIsUpdating (true);

	reset ();
}

//----------------------------------------------------------------------

void CuiTransition::performDeactivate ()
{
	setIsUpdating (false);
	m_mediatorNamePrevious.clear ();
	m_mediatorNameNext.clear     ();
}

//----------------------------------------------------------------------

void CuiTransition::setTransitionPrevious (const std::string & mediatorName)
{
	m_mediatorNamePrevious = mediatorName;
}

//----------------------------------------------------------------------

void CuiTransition::setTransitionNext     (const std::string & mediatorName)
{
	m_mediatorNameNext = mediatorName;
}

//----------------------------------------------------------------------

void CuiTransition::onTransitionReady     (const std::string & mediatorName)
{
	if (!s_enabled)
		return;

	if (mediatorName != m_mediatorNameNext)
	{
		WARNING (!m_mediatorNameNext.empty (), ("CuiTransition bad transition ready attempt for [%s], expected [%s]", mediatorName.c_str (), m_mediatorNameNext.c_str ()));
		return;
	}

	if (!m_soundStart.empty ())
		m_sidStart = Audio::playSound (m_soundStart.c_str (), Vector::zero, NULL);

	if (!m_soundLoop.empty ())
		m_sidLoop  = Audio::playSound (m_soundLoop.c_str (),  Vector::zero, NULL);

	m_state          = S_post;
	m_elapsedTime    = 0.0f;
	m_skipNextUpdate = true;

	for (DoorDataVector::iterator it = m_doorDataVector->begin (); it != m_doorDataVector->end (); ++it)
	{
		DoorData & dd = *it;
		dd.finished = false;
	}
}

//----------------------------------------------------------------------

void CuiTransition::signalTransitionReady (const std::string & mediatorName)
{
	if (s_transition)
	{
		s_transition->onTransitionReady (mediatorName);
	}
}

//----------------------------------------------------------------------

void CuiTransition::update                (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	if (m_skipNextUpdate)
	{
		m_skipNextUpdate = false;
		return;
	}

	if (m_state == S_wait)
	{
		if (m_countownToWaitTransition > 0.0f)
		{
			m_countownToWaitTransition -= deltaTimeSecs;

			if (m_countownToWaitTransition <= 0)
			{
				CuiMediatorFactory::deactivate (m_mediatorNamePrevious.c_str ());
				CuiMediatorFactory::activate   (m_mediatorNameNext.c_str     ());
			}
		}

		return;
	}

	m_elapsedTime += deltaTimeSecs;

	long deltaPixels = static_cast<long>(m_elapsedTime * m_horizontalSpeed);

	if (deltaPixels <= 0)
		return;

	m_elapsedTime -= static_cast<float>(deltaPixels) / m_horizontalSpeed;
	
	bool aDoorFinished    = false;
	bool allDoorsFinished = true;
	
	for (DoorDataVector::iterator it = m_doorDataVector->begin (); it != m_doorDataVector->end (); ++it)
	{
		DoorData & dd = *it;
		
		NOT_NULL (dd.widget);
		
		if (dd.finished)
			continue;
		
		allDoorsFinished = false;

		UIPoint pos = dd.widget->GetLocation ();
		UIPoint target;
		
		if (m_state == S_pre)
		{
			target = dd.targetWait;
		}
		// (m_state == S_post)
		else 
		{
			target = dd.targetStart;
		}
		
		bool finished_x = true;
		bool finished_y = true;
		
		if (pos.x < target.x)
			pos.x = std::min (pos.x + deltaPixels, target.x);
		else if (pos.x > target.x)
			pos.x = std::max (pos.x - deltaPixels, target.x);
		
		if (pos.y < target.y)
			pos.y = std::min (pos.y + deltaPixels, target.y);
		else if (pos.y > target.x)
			pos.y = std::max (pos.y - deltaPixels, target.y);
		
		dd.widget->SetLocation (pos);
		
		if (finished_x && pos.x != target.x)
			finished_x = false;
		
		if (finished_y && pos.y != target.y)
			finished_y = false;
		
		dd.finished = finished_x && finished_y;
		
		aDoorFinished = aDoorFinished || dd.finished;
	}

	if (aDoorFinished)
	{
		// play sound
	}

	if (allDoorsFinished)
	{
		if (m_state == S_pre)
		{
			m_state = S_wait;

			Audio::stopSound (m_sidLoop, 0.0f);
			m_sidLoop.invalidate ();
			if (!m_soundStopClose.empty ())
				m_sidStopClose = Audio::playSound (m_soundStopClose.c_str (), Vector::zero, NULL);

			m_countownToWaitTransition = 0.1f;
			m_skipNextUpdate = true;
		}
		// (m_state == S_post)
		else 
		{
			Audio::stopSound (m_sidLoop, 0.5f);
			m_sidLoop.invalidate ();
			if (!m_soundStopOpen.empty ())
				m_sidStopOpen = Audio::playSound (m_soundStopOpen.c_str (), Vector::zero, NULL);

			deactivate ();
		}
	}
}

//----------------------------------------------------------------------

void CuiTransition::reset ()
{
	m_state = S_pre;
	m_elapsedTime = 0.0f;
	
	m_skipNextUpdate = true;

	Audio::stopSound (m_sidLoop, 0.0f);
	m_sidLoop.invalidate       ();

	if (!m_soundStart.empty ())
		m_sidStart = Audio::playSound (m_soundStart.c_str (), Vector::zero, NULL);

	if (!m_soundLoop.empty ())
		m_sidLoop  = Audio::playSound (m_soundLoop.c_str (),  Vector::zero, NULL);
	
	m_doorDataVector->clear ();	

	switch (m_type)
	{
	case T_horizontal:
		{
			m_doorLeft->SetVisible  (true);
			m_doorRight->SetVisible (true);
			
			DoorData dd_left;
			DoorData dd_right;
			
			dd_left.finished  = false;
			dd_right.finished = false;
			
			dd_left.widget  = m_doorLeft;
			dd_right.widget = m_doorRight;
			
			UIWidget * const parent = getPage ().GetParentWidget ();
			NOT_NULL (parent);
			const UISize & parentSize = parent->GetSize ();
			const UIPoint & parentCenter = parentSize / 2L;
			
			m_doorLeft->SetLocation (m_doorLeftMargin.left - m_doorLeft->GetWidth (),  m_doorLeft->GetLocation ().y);
			m_doorRight->SetLocation (parentSize.x - m_doorRightMargin.right,          m_doorRight->GetLocation ().y);

			dd_left.targetStart   = m_doorLeft->GetLocation ();
			dd_right.targetStart  = m_doorRight->GetLocation ();
			
			dd_left.targetWait.x  = parentCenter.x - m_doorLeftMargin.right - m_doorLeft->GetWidth ();
			dd_left.targetWait.y  = m_doorLeft->GetLocation ().y;
			
			dd_right.targetWait.x = parentCenter.x + m_doorRightMargin.left;
			dd_right.targetWait.y = m_doorRight->GetLocation ().y;

			m_doorDataVector->push_back (dd_left);
			m_doorDataVector->push_back (dd_right);
		}
		break;
	}
}

//----------------------------------------------------------------------


void CuiTransition::startTransition       (const std::string & from, const std::string & to)
{
	if (!s_debugFlagsRegistered)
	{
		DebugFlags::registerFlag(s_enabled,       "ClientUserInterface", "transitions");
		s_debugFlagsRegistered = true;
	}

	if (s_enabled)
	{
		CuiTransition * const trans = safe_cast<CuiTransition *>(CuiMediatorFactory::activate (CuiMediatorTypes::Transition));
		NOT_NULL (trans);
		trans->setTransitionPrevious (from);
		trans->setTransitionNext     (to);
		trans->reset ();
	}
	else
	{
		CuiMediatorFactory::activate   (to.c_str ());
		CuiMediatorFactory::deactivate (from.c_str ());
	}
}

//----------------------------------------------------------------------

bool CuiTransition::isTransitioning       ()
{
	if (!s_debugFlagsRegistered)
	{
		DebugFlags::registerFlag(s_enabled,       "ClientUserInterface", "transitions");
		s_debugFlagsRegistered = true;
	}

	if (s_enabled)
	{
		CuiMediator * const trans = CuiMediatorFactory::get (CuiMediatorTypes::Transition, false);
		return (trans && trans->isActive ());
	}

	return false;
}

//======================================================================
