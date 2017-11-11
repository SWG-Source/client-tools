//======================================================================
//
// SwgCuiFaction.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiFaction_H
#define INCLUDED_SwgCuiFaction_H

//======================================================================

#include "UIString.h"	// can't forward-declare UIString as a "class"

#include "clientUserInterface/CuiMediator.h"

class FactionResponseMessage;
class UIPage;
class UIText;

// ======================================================================

class SwgCuiFaction : public CuiMediator
{
public:
	explicit                 SwgCuiFaction   (UIPage & thePage);
	virtual void             performActivate   ();
	virtual void             performDeactivate ();
	void                     update(const FactionResponseMessage& msg) const;
	void                     setAlignment(const UIString& alignment);

private:
	                        ~SwgCuiFaction ();
	                         SwgCuiFaction ();
	                         SwgCuiFaction (const SwgCuiFaction & rhs);
	SwgCuiFaction &          operator=     (const SwgCuiFaction & rhs);


	void                     setFaction(const UIPage * page, int factionValue) const;

private:
	UIPage *                 m_pageRebelFaction;
	UIPage *                 m_pageImperialFaction;
	UIPage *                 m_pageCriminalFaction;
	UIText *                 m_alignment;
};

//======================================================================

#endif
