//======================================================================
//
// SwgCuiChatWindow_Tab.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiChatWindow_Tab_H
#define INCLUDED_SwgCuiChatWindow_Tab_H

//======================================================================

#include "swgClientUserInterface/SwgCuiChatWindow_ChannelId.h"

#include "UINotification.h"

//----------------------------------------------------------------------

class SwgCuiChatWindow::Tab :
public UINotificationServer
{
public:

	//----------------------------------------------------------------------

	typedef stdvector<ChannelId>::fwd ChannelVector;

	Tab ();
	~Tab ();

	bool                    hasChannel        (const ChannelId & id) const;
	const ChannelId &       getDefaultChannel () const;
	void                    parse             (const Unicode::String & str);

	void                    addChannel        (const ChannelId & id);
	void                    removeChannel     (const ChannelId & id);
	void                    setDefaultChannel (const ChannelId & id);
	void                    getChannels       (ChannelVector & cv) const;

	void                    appendText         (const ChannelId & id, const Unicode::String & str);
	void                    getAppendableText  (const Unicode::String & current, Unicode::String & str, bool appendMode = false) const;
	int                     getLineCount       () const;
	int                     removeLeadingLines (int num);

	const Unicode::String & getText            () const;
	const Unicode::String & getName            () const;
	const Unicode::String & getAppendText      () const;

	void					resetAppendText	   ();
	void					setDoAppend		   (bool b);
	bool					getDoAppend		   () const;


	                         Tab               (const SwgCuiChatWindow::Tab &);
	Tab &                    operator=         (const SwgCuiChatWindow::Tab &);

	Tab &                    copy              (const SwgCuiChatWindow::Tab &, bool copyText);

	void                     mergeTab          (const SwgCuiChatWindow::Tab & rhs);

	void                     setModified       (const ChannelId & id, bool b, bool notify = true);
	void                     setUnmodified     (bool notify = true);
	bool                     isModified        (const ChannelId & id) const;

	bool                     updateModifiedDifference (const Tab & rhs, bool notify = true);

	bool                     channelsEmpty     () const;

	int                      getTabId          () const;
	void                     setTabId          (int id);

	static void              setLastTabId      (int id);

	int                      getCharactersCut   () const;
	void                     clearCharactersCut ();

	const Unicode::String &  getUserAssignedName () const;
	void                     setUserAssignedName (const Unicode::String & str);

	void                     saveTabSettings     (const std::string & mediatorName, int index) const;
	void                     loadTabSettings     (const std::string & mediatorName, int index);
	void                     saveChatHistory     (const std::string & mediatorName, int index) const;
	void                     loadChatHistory     (const std::string & mediatorName, int index);

private:

	void updateName                (bool notify);

	ChannelId                      m_defaultChannel;
	typedef stdset<ChannelId>::fwd ChannelSet;
	ChannelSet *                   m_channels;

	ChannelSet *                   m_modifiedChannels;

	Unicode::String                m_text;
	Unicode::String                m_appendText;
	Unicode::String                m_name;
	Unicode::String                m_userAssignedName;

	bool                           m_modified;
	bool						   m_doAppend;

	int                            m_tabId;

	int                            m_charactersCut;
};

//----------------------------------------------------------------------

inline const SwgCuiChatWindow::ChannelId & SwgCuiChatWindow::Tab::getDefaultChannel () const
{
	return m_defaultChannel;
}

//----------------------------------------------------------------------

inline const Unicode::String & SwgCuiChatWindow::Tab::getText            () const
{
	return m_text;
}

//----------------------------------------------------------------------

inline const Unicode::String & SwgCuiChatWindow::Tab::getAppendText		 () const
{
	return m_appendText;
}


//----------------------------------------------------------------------

inline const Unicode::String & SwgCuiChatWindow::Tab::getName            () const
{
	return m_name;
}

//----------------------------------------------------------------------

inline int SwgCuiChatWindow::Tab::getTabId          () const
{
	return m_tabId;
}

//----------------------------------------------------------------------

inline int SwgCuiChatWindow::Tab::getCharactersCut   () const
{
	return m_charactersCut;
}

//----------------------------------------------------------------------

inline const Unicode::String & SwgCuiChatWindow::Tab::getUserAssignedName () const
{
	return m_userAssignedName;
}

inline void SwgCuiChatWindow::Tab::setDoAppend(bool b)
{
	m_doAppend = b;
}

inline bool SwgCuiChatWindow::Tab::getDoAppend() const
{
	return m_doAppend;
}

//======================================================================

#endif
