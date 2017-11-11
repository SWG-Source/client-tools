// ======================================================================
//
// SwgCuiCraftExperiment_Attrib.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCraftExperiment_Attrib_H
#define INCLUDED_SwgCuiCraftExperiment_Attrib_H

// ======================================================================

#include "swgClientUserInterface/SwgCuiCraftExperiment.h"

class UIText;
class UIPage;
class UIOpacityEffector;
class StringId;

//-----------------------------------------------------------------

class SwgCuiCraftExperiment::Attrib :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                    Attrib            (UIPage & page, SwgCuiCraftExperiment & mediatorExperiment);
	                           ~Attrib            ();
	virtual void                performActivate   ();
	virtual void                performDeactivate ();

	virtual bool                OnMessage         (UIWidget *context, const UIMessage & msg );	

	void                        setAttrib         (int index, const StringId & name, float min, float max, float cur, float resourceMax);
	void                        setExp            (int exp);

	int                         getExp            () const;

	int                         getIndex          () const;
	const StringId &            getName           () const;

	void                        setExpPointsAvailable (int points);

private:
	Attrib (const Attrib & rhs);
	Attrib & operator= (const Attrib & rhs);

	int                         findExpIndex      (UIWidget * widget);
	void                        updateExpWidgets  ();
	void                        updateExpBoxes    ();
	void                        setTextLabel      (const StringId & name);

	enum { NumPercents = 10 };

	UIPage *                    m_percentPages   [NumPercents];
	UIButton *                  m_percentButtons [NumPercents];

	UIPage *                    m_pageInner;
	UIPage *                    m_pageCur;
	UIPage *                    m_pageExp;
	UIPage *                    m_pageMax;
	UIPage *                    m_pageResourceMax;

	UIText *                    m_textLabel;
	UIText *                    m_textValue;

	int                         m_index;
	float                       m_min;
	float                       m_max;
	float                       m_cur;
	float                       m_resourceMax;

	int                         m_exp;

	StringId *                  m_name;
	SwgCuiCraftExperiment &     m_mediatorExperiment;

	int                         m_expBegin;
	int                         m_expEnd;

	int                         m_expPointsAvailable;

	bool                        m_mouseDown;
};

//-----------------------------------------------------------------

inline int SwgCuiCraftExperiment::Attrib::getExp () const
{
	return m_exp;
}

//-----------------------------------------------------------------

inline int SwgCuiCraftExperiment::Attrib::getIndex() const
{
	return m_index;
}

// ======================================================================

#endif

