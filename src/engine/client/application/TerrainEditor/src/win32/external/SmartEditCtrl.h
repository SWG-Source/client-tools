//
//
//
//
//
//

//-------------------------------------------------------------------

#ifndef INCLUDED_SmartEditCtrl_H
#define INCLUDED_SmartEditCtrl_H

//-------------------------------------------------------------------

#define SES_NUMBERS		0x0001	// no negative sign allowed
#define SES_LETTERS		0x0002	// also upper and lower case
#define SES_FLOATINGPT	0x0004	// allows '-', '.', and 'E'
#define SES_OTHERCHARS	0x0008	// allows punctuation
#define SES_SIGNED		0x0010	// allows negative sign also
#define SES_UNDERSCORE	0x0020	// allows the underscore
#define	SES_HEXADECIMAL 0x0040	// allows hex chars (0-9,A-F,X)
#define SES_ALL			0xFFFF	// anything goes

//-------------------------------------------------------------------

class PropertyView;
class SmartSliderCtrl;

//-------------------------------------------------------------------

class SmartEditCtrl : public CEdit
{
private:

	PropertyView*  m_propertyView;

	WORD	       m_wParseType;

protected:

	virtual void OnBadInput();

public:

	explicit SmartEditCtrl (PropertyView* propertyView);
	virtual ~SmartEditCtrl();

	void	LinkSmartSliderCtrl (SmartSliderCtrl* smartSliderCtrl);
	void    SetParseType (WORD type);
	void    SetParams (int imin, int imax, int ticks);
	void    SetParams (float fmin, float fmax, int ticks, const char* formatString);
	bool    SetValue (int value);
	bool    SetValue (float value);
	void	SetFormatString (const char* formatString);

	void    UpdateEdit (int position);

	int     GetValueInt () const;
	float   GetValueFloat () const;

private :

	void    InitSlider();

private:

	SmartSliderCtrl* m_pSlider;

	char* m_pFormat;
	int   m_iTextId;
	int   m_iSliderId;
	int   m_iBands;
	BOOL  m_bUseFp;
	int   m_iMin, m_iMax, m_iValue, m_iRange;
	float m_fMin, m_fMax, m_fValue, m_fRange;

protected:

	//{{AFX_MSG(SmartEditCtrl)
	afx_msg void OnChar(UINT, UINT, UINT);
	afx_msg void OnUpdate();	
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

#endif
