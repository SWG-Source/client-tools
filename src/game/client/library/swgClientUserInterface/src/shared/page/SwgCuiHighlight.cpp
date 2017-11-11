//======================================================================
//
// SwgCuiHighlight.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiHighlight.h"

#include "UIManager.h"
#include "UIText.h"
#include "UIPage.h"
#include "UIData.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "sharedNetworkMessages/NewbieTutorialHighlightUIElement.h"
#include "swgSharedUtility/States.h"


//======================================================================

namespace SwgCuiHighlightNamespace
{
	const int HIGHLIGHT_INCREASED_SIZE = 60;
}

using namespace SwgCuiHighlightNamespace;

SwgCuiHighlight::SwgCuiHighlight (UIPage & page, int sceneType) :
CuiMediator           ("SwgCuiHighlight", page),
m_sampleHighlightPage (0),
m_highlightPages (),
m_sceneType(sceneType)
{
	getCodeDataObject (TUIPage, m_sampleHighlightPage,      "sample", true);	
	
	connectToMessage (NewbieTutorialHighlightUIElement::cms_name);
}

//----------------------------------------------------------------------

SwgCuiHighlight::~SwgCuiHighlight ()
{	
	removeAllHighlights();
}

//----------------------------------------------------------------------

void SwgCuiHighlight::performActivate   ()
{
	setIsUpdating (true);
	UIManager::gUIManager ().AddToRenderLastList(&getPage());
}

//----------------------------------------------------------------------

void SwgCuiHighlight::performDeactivate ()
{
	setIsUpdating (false);
	UIManager::gUIManager ().RemoveFromRenderLastList(&getPage());
}

//----------------------------------------------------------------------

void SwgCuiHighlight::update   (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	std::map<UIWidget *, HighlightRecord>::iterator i = m_highlightPages.begin();
	while(i != m_highlightPages.end())
	{
		HighlightRecord &hr = i->second;
		hr.currentTime += deltaTimeSecs;
		if( ((hr.maxTime > 0.0f) && (hr.currentTime >= hr.maxTime)) || !(hr.target->IsVisible()) )
		{
			hr.highlightPage->SetVisible(false);
			getPage().RemoveChild(hr.highlightPage);
			m_highlightPages.erase(i++);
		}
		else
		{
			UIPoint const &targetLoc = hr.target->GetWorldLocation();
			UISize const &targetSize = hr.target->GetSize();			
			int maxSize = std::max(targetSize.x, targetSize.y);
			UIPoint mySize(maxSize + HIGHLIGHT_INCREASED_SIZE, maxSize + HIGHLIGHT_INCREASED_SIZE);
			UIPoint myLoc(targetLoc.x + targetSize.x / 2 - maxSize / 2 - HIGHLIGHT_INCREASED_SIZE / 2, 
				targetLoc.y + targetSize.y / 2 - maxSize / 2 - HIGHLIGHT_INCREASED_SIZE / 2);
			hr.highlightPage->SetLocation(myLoc);
			hr.highlightPage->SetSize(mySize);
			++i;
		}
	}

}

//----------------------------------------------------------------------

void SwgCuiHighlight::addHighlight(UIWidget *target, float time)
{
	HighlightRecord hr;
	hr.currentTime = 0.0f;
	hr.maxTime = time;
	hr.target = target;
	UIPage *newHighlightPage = NON_NULL (static_cast<UIPage *>(m_sampleHighlightPage->DuplicateObject ())); //lint !e1774 //stfu noob
	getPage().AddChild(newHighlightPage);
	newHighlightPage->SetName(UINarrowString("new"));
	newHighlightPage->Link ();
	newHighlightPage->SetVisible (true);
	hr.highlightPage = newHighlightPage;
	m_highlightPages.insert(std::make_pair(target, hr));
}

//-----------------------------------------------------------------

void SwgCuiHighlight::receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
{
	//----------------------------------------------------------------------
	
	if(Game::getHudSceneType() != static_cast<int>(m_sceneType))
		return;

	if (message.isType (NewbieTutorialHighlightUIElement::cms_name))
	{
		//-- what type of request is it?
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage*> (&message))->getByteStream ().begin ();
		const NewbieTutorialHighlightUIElement newbieTutorialHighlightUIElement (ri);

		float time = newbieTutorialHighlightUIElement.getTime();
		const std::string & widgetPath = newbieTutorialHighlightUIElement.getWidgetPath();
		
		UIWidget * const target = safe_cast<UIWidget *>(getPage().GetObjectFromPath (widgetPath.c_str(), TUIWidget));
		if(target)
		{
			if(time >= 0.0f)					
				addHighlight(target, time);			
			else
				removeHighlights(target);
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiHighlight::removeHighlights(UIWidget *target)
{
	std::map<UIWidget *, HighlightRecord>::iterator i = m_highlightPages.begin();
	while(i != m_highlightPages.end())
	{
		HighlightRecord const & hr = i->second;
		if(hr.target == target)
		{		
			hr.highlightPage->SetVisible(false);
			getPage().RemoveChild(hr.highlightPage);
			m_highlightPages.erase(i++);		
		}
		else
			++i;
	}
}

//-----------------------------------------------------------------

void SwgCuiHighlight::removeAllHighlights()
{
	std::map<UIWidget *, HighlightRecord>::iterator i = m_highlightPages.begin();
	while(!m_highlightPages.empty())
	{
		HighlightRecord const & hr = i->second;
		hr.highlightPage->SetVisible(false);
		getPage().RemoveChild(hr.highlightPage);
		m_highlightPages.erase(i);		
		i = m_highlightPages.begin();
	}
}

//======================================================================
