//======================================================================
//
// CuiTransfer.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiTransfer_H
#define INCLUDED_CuiTransfer_H

//======================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"

class UIButton;
class UIPage;
class UISliderbar;
class UIText;
class UITextbox;

// ======================================================================

/**
* CuiTransfer
*/

class CuiTransfer : 
public CuiMediator, 
public UIEventCallback
{
public:
	explicit                      CuiTransfer (UIPage & page);

	virtual void                  OnSliderbarChanged(UIWidget *context);
	virtual void                  OnButtonPressed(UIWidget * context);
	virtual void                  OnTextboxChanged(UIWidget * context);
	
protected:
	virtual void                  performActivate        ();
	virtual void                  handleMediatorPropertiesChanged ();

private:
	void                          matchValuesToSlider();
	void                          matchSliderToValues();
	void                          setCurrentFromAndTo(int currentFrom, int currentTo);
	int                           getMaxFrom () const;
	int                           getMaxTo   () const;
	int                           calculateTotal(int from, int to) const;
	bool                          checkTotal(int from, int to) const;
	void                          displayFrom(int from);
	void                          displayTo(int to);

private:
	//disabled
	                              CuiTransfer ();
	                              CuiTransfer (const CuiTransfer & rhs);
	CuiTransfer &                 operator=   (const CuiTransfer & rhs);

private:
	void                          revert ();

private:
	UIText *                      m_startingFromText;
	UIText *                      m_startingToText;
	UITextbox *                   m_currentFromText;
	UITextbox *                   m_currentToText;
	UISliderbar *                 m_slider;
	UIButton *                    m_buttonCancel;
	UIButton *                    m_buttonRevert;
	UIButton *                    m_buttonOk;
	UIPage *                      m_transaction;
	int                           m_currentFrom;
	int                           m_currentTo;
	int                           m_startingFrom;
	int                           m_startingTo;
	int                           m_conversionRatioFrom;
	int                           m_conversionRatioTo;
	int                           m_startingTotal;
	int                           m_modulusFrom;
	int                           m_modulusTo;
};

//======================================================================

inline int CuiTransfer::getMaxFrom() const
{
	if(m_conversionRatioTo == 0)
		return 0;
	return m_startingTotal / m_conversionRatioTo;
}

//----------------------------------------------------------------------

inline int CuiTransfer::getMaxTo() const
{
	if(m_conversionRatioFrom == 0)
		return 0;
	return m_startingTotal / m_conversionRatioFrom;
}

//======================================================================

#endif

