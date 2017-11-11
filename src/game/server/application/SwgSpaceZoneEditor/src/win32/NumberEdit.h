//
// NumberEdit.h
// asommers 8-25-2001
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_NumberEdit_H
#define INCLUDED_NumberEdit_H

//-------------------------------------------------------------------

class NumberEdit : public CEdit
{
public:

	explicit NumberEdit (bool allowNegative, bool restrictToIntegers=false);
	virtual ~NumberEdit();

public:

	operator float () const;  //lint !e1930
	NumberEdit& operator= (float f); 

	//{{AFX_VIRTUAL(NumberEdit)
	//}}AFX_VIRTUAL

protected:

	NumberEdit ();
	NumberEdit& operator= (const NumberEdit&);

	void trimTrailingZeros (CString& s) const;

protected:

	//{{AFX_MSG(NumberEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:

	void  SetNumber (float f);
	float GetNumber () const;

	void  SetNumberFloat (float f);
	float GetNumberFloat () const;

	void  SetNumberInt (int f);
	int   GetNumberInt () const;

private:

	bool m_allowNegative;
	bool m_restrictToIntegers;
};  //lint !e1721

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 

