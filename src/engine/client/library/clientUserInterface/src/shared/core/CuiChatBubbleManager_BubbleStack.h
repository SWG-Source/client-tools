//======================================================================
//
// CuiChatBubbleManager_BubbleStack.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiChatBubbleManager_BubbleStack_H
#define INCLUDED_CuiChatBubbleManager_BubbleStack_H

#include "CuiChatBubbleManager.h"

#include "sharedObject/CachedNetworkId.h"

#include "UITypes.h"

#include "Unicode.h"

#include <list>


//======================================================================

class CuiChatBubbleManager::BubbleStack
{
public:
	struct Element
	{
		Unicode::String   m_text;
		uint32            m_chatType;
		uint32            m_mood;
		uint16            m_volume;

		mutable long      m_lastFontSize;
		mutable long      m_lastTextSize;

		Element (const Unicode::String & text, uint32 chatType, uint32 mood, uint16 volume) :
			m_text         (text),
			m_chatType     (chatType),
			m_mood         (mood),
			m_volume       (volume),
			m_lastFontSize (0),
			m_lastTextSize (0)
		{}

		Element () : 
			m_text         (), 
			m_chatType     (0), 
			m_mood         (0),
			m_volume       (0),
			m_lastFontSize (0),
			m_lastTextSize (0)
		{}
	};

	typedef std::list <Element> ElementList;

	BubbleStack ();
	~BubbleStack ();

	void   addElement                (const Element & elem);
	const  ElementList & getElements () const;
	bool   update                    (float elapsedTime);
	float  getTopBubbleOpacity       () const;
	uint16 getMaxVolume              () const;

	void   updateMaxVolume           ();

	void setId(CachedNetworkId const & id);
	CachedNetworkId const & getId() const;

private:

	ElementList        m_elementList;
	float              m_timeout;
	float              m_opacity;
	uint16             m_maxVolume;
	CachedNetworkId    m_id;
};

//-----------------------------------------------------------------

inline const CuiChatBubbleManager::BubbleStack::ElementList & CuiChatBubbleManager::BubbleStack::getElements () const
{
	return m_elementList;
}

//-----------------------------------------------------------------

inline float CuiChatBubbleManager::BubbleStack::getTopBubbleOpacity () const
{
	return m_opacity;
}

//----------------------------------------------------------------------

inline uint16 CuiChatBubbleManager::BubbleStack::getMaxVolume () const
{
	return m_maxVolume;
}

//----------------------------------------------------------------------

inline void CuiChatBubbleManager::BubbleStack::setId(CachedNetworkId const & id)
{
	m_id = id;
}

//----------------------------------------------------------------------

inline CachedNetworkId const & CuiChatBubbleManager::BubbleStack::getId() const
{
	return m_id;
}

//======================================================================

#endif
