//======================================================================
//
// CuiChatBubbleManager_BubbleStack.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiChatBubbleManager_BubbleStack.h"

#include "clientUserInterface/ConfigClientUserInterface.h"

#include <vector>

//======================================================================

namespace
{
	float computeTimeout (size_t numChars)
	{
		const float timeoutMin  =  CuiChatBubbleManager::getBubblesDurationMin ();
		const float timeoutMax  =  CuiChatBubbleManager::getBubblesDurationMax ();
		const float timeoutRate =  CuiChatBubbleManager::getBubblesDurationScale ();

		return std::max (timeoutMin, std::min (timeoutMax, static_cast<float> (numChars) * timeoutRate));
	}
}

//-----------------------------------------------------------------

CuiChatBubbleManager::BubbleStack::BubbleStack () :
m_elementList (),
m_timeout     (0.0f),
m_opacity     (1.0f),
m_maxVolume   (0),
m_id(CachedNetworkId::cms_cachedInvalid)
{
}

//-----------------------------------------------------------------

CuiChatBubbleManager::BubbleStack::~BubbleStack ()
{
}

//-----------------------------------------------------------------

void CuiChatBubbleManager::BubbleStack::addElement (const Element & elem)
{
	const bool empty = m_elementList.empty ();
	m_elementList.push_front (elem);

	const int maxStackHeight = ConfigClientUserInterface::getChatBubblesMaxStackHeight ();

	m_maxVolume = std::max (elem.m_volume, m_maxVolume);

	if (empty)
	{
		m_timeout = computeTimeout (elem.m_text.size ());
		m_opacity = 1.0f;
	}
	else if (maxStackHeight < static_cast<int>(m_elementList.size ()))
	{
		m_elementList.pop_back ();
		DEBUG_FATAL (m_elementList.empty (), ("no"));
		m_timeout = computeTimeout (m_elementList.back ().m_text.size ());
		m_opacity = 1.0f;

		updateMaxVolume ();
	}
}

//----------------------------------------------------------------------

void CuiChatBubbleManager::BubbleStack::updateMaxVolume ()
{
	m_maxVolume = 0;
	
	for (ElementList::const_iterator it = m_elementList.begin (); it != m_elementList.end (); ++it)
	{
		const Element & elem = *it;
		m_maxVolume = std::max (elem.m_volume, m_maxVolume);
	}
}

//-----------------------------------------------------------------

bool  CuiChatBubbleManager::BubbleStack::update (float elapsedTime)
{
	m_timeout -= elapsedTime;
	
	const int maxStackHeight = ConfigClientUserInterface::getChatBubblesMaxStackHeight ();

	//-- if we have too many bubbles, get rid of one
	if (maxStackHeight < static_cast<int>(m_elementList.size ()))
	{
		m_timeout = -1.0f;
		m_opacity = -1.0f;
	}

	//-- time's up, delete me
	if (m_timeout < 0.0f)
	{
		m_opacity -= elapsedTime;
		
		if (m_opacity < 0.0f)
		{
			if (m_elementList.empty ())
			{
				DEBUG_FATAL (true, ("list empty on update"));
				return false;
			}
			
			m_elementList.pop_back ();
			
			if (m_elementList.empty ())
				return false;
			
			m_timeout = computeTimeout (m_elementList.back ().m_text.size ());
			m_opacity = 1.0f;

			updateMaxVolume ();
		}
	}
	
	return true;
}

//======================================================================
