//======================================================================
//
// SwgCuiFaction.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiFaction.h"

#include "sharedNetworkMessages/FactionResponseMessage.h"

#include "Unicode.h"

#include "UIData.h"
#include "UIPage.h"
#include "UIText.h"

//======================================================================

namespace SwgCuiFactionNamespace
{
	const UILowerString color("Color");
	const UIString      red(Unicode::narrowToWide("#FF0000"));
	const UIString      green(Unicode::narrowToWide("#00FF00"));

	const char*         rebelPage    = "FactionMeterRebel";
	const char*         imperialPage = "FactionMeterImperial";
	const char*         criminalPage = "FactionMeterCriminal";

	const char*         factionWidgetTopPiece       = "top";
	const char*         factionWidgetPercentageText = "top.percentage";
	const char*         factionWidgetCenterPiece    = "center";

	const int           maxFactionValue        = 1000;
	const int           factionFullyClosedYLoc = 3;
	const int           factionFullyOpenedYLoc = -33;
	const int           factionUIRange         = abs(factionFullyOpenedYLoc - factionFullyClosedYLoc);
}

//======================================================================

SwgCuiFaction::SwgCuiFaction (UIPage & page)
: 
CuiMediator ("SwgCuiFaction", page),
m_pageRebelFaction(0),
m_pageImperialFaction(0),
m_pageCriminalFaction(0)
{
	getCodeDataObject (TUIPage,       m_pageRebelFaction,              SwgCuiFactionNamespace::rebelPage);
	getCodeDataObject (TUIPage,       m_pageImperialFaction,           SwgCuiFactionNamespace::imperialPage);
	getCodeDataObject (TUIPage,       m_pageCriminalFaction,           SwgCuiFactionNamespace::criminalPage);
	getCodeDataObject (TUIText,       m_alignment,                     "pvpstatus");
}

//----------------------------------------------------------------------

SwgCuiFaction::~SwgCuiFaction ()
{
	m_pageRebelFaction    = 0;
	m_pageImperialFaction = 0;
	m_pageCriminalFaction = 0;
}

//----------------------------------------------------------------------

void SwgCuiFaction::performActivate   ()
{
	//start off with factions initialized to closed
	setFaction(m_pageRebelFaction,    0);
	setFaction(m_pageImperialFaction, 0);
}

//----------------------------------------------------------------------

void SwgCuiFaction::performDeactivate ()
{
}

//----------------------------------------------------------------------

void SwgCuiFaction::update(const FactionResponseMessage& msg) const
{
	setFaction(m_pageRebelFaction, msg.getFactionRebel());
	setFaction(m_pageImperialFaction, msg.getFactionImperial());
}

//----------------------------------------------------------------------

void SwgCuiFaction::setAlignment(const UIString& alignment)
{
	m_alignment->SetLocalText(alignment);
}

//----------------------------------------------------------------------

void SwgCuiFaction::setFaction(const UIPage * page, int factionValue) const
{
	//get top, percentage, and back widgets
	UIPage * const top        = NON_NULL (GET_UI_OBJ ((*page), UIPage, SwgCuiFactionNamespace::factionWidgetTopPiece));
	UIText * const percentage = NON_NULL (GET_UI_OBJ ((*page), UIText, SwgCuiFactionNamespace::factionWidgetPercentageText));
	UIPage * const back       = NON_NULL (GET_UI_OBJ ((*page), UIPage, SwgCuiFactionNamespace::factionWidgetCenterPiece));

	//set back color (green if they like you, red if they don't)
	if(factionValue < 0)
		back->SetProperty(SwgCuiFactionNamespace::color, SwgCuiFactionNamespace::red);
	else
		back->SetProperty(SwgCuiFactionNamespace::color, SwgCuiFactionNamespace::green);

	//set percentage value
	int value = abs(factionValue / 10);
	char buf[64];
	_itoa(value, buf, 10);
	percentage->SetText(Unicode::narrowToWide(buf));

	//set top location (the open/closed-ness)
	const int relativeFactionValue = abs(factionValue);
	float ratio = static_cast<float>(relativeFactionValue) / SwgCuiFactionNamespace::maxFactionValue;
	const int newFactionYLoc = SwgCuiFactionNamespace::factionFullyClosedYLoc - static_cast<int>(ratio * SwgCuiFactionNamespace::factionUIRange);
	UIPoint loc = top->GetLocation();
	loc.y = newFactionYLoc;
	top->SetLocation(loc);
}

//======================================================================
