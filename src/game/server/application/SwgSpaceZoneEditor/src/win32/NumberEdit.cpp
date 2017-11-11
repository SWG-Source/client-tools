//
// NumberEdit.cpp
// asommers 8-25-2001
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstSwgSpaceZoneEditor.h"
#include "NumberEdit.h"

//-------------------------------------------------------------------

NumberEdit::NumberEdit(bool allowNegative, bool restrictToIntegers) :
	CEdit(),
	m_allowNegative(allowNegative),
	m_restrictToIntegers(restrictToIntegers)
{
}

//-------------------------------------------------------------------

NumberEdit::~NumberEdit()
{
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(NumberEdit, CEdit)
	//{{AFX_MSG_MAP(NumberEdit)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void NumberEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	const CString invalidCharacters = "`~!@#$%^&*()_+|=\\qwertyuiop[]asdfghjkl;'zxcvbnm,/QWERTYUIOP{}ASDFGHJKL:/ZXCVBNM<>?/";

	if (invalidCharacters.Find(static_cast<char>(nChar)) != -1)
		return;

	//-- length, selection info
	int start_char, end_char;
	GetSel(start_char, end_char);

	//-- current CEdit text
	CString temp_str;
	GetWindowText(temp_str);

	bool signTyped   = false;
	bool periodTyped = false;
	bool numberTyped = false;
	bool zeroTyped   = false;
	char temp_char;

	int i;
	for (i = 0; i < temp_str.GetLength(); i++) 
	{
		if ((i >= start_char) && (i < end_char))
			continue;

		temp_char = temp_str[i];

		if ((temp_char == '+') || (temp_char == '-')) 
			signTyped = true;
		else 
			if (temp_char == '.') 
				periodTyped = true;
			else
				if (isdigit(temp_char) && (temp_char != '0'))
					numberTyped = true;
				else
					if (temp_char == '0')
						zeroTyped = true;
	}

	//-- allow sign only once (first char)
	if (((nChar == '+') || (nChar == '-')) && (signTyped || (start_char != 0) || !m_allowNegative))
		return;

	//-- allow period only once
	if (nChar == '.' && (m_restrictToIntegers || periodTyped))
		return;

	if (zeroTyped && !numberTyped && (nChar == '0'))
		return;

	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

//-------------------------------------------------------------------

void NumberEdit::SetNumber(float f)
{
	SetNumberFloat(f);
}

//-------------------------------------------------------------------

float NumberEdit::GetNumber() const
{
	return GetNumberFloat();
}

//-------------------------------------------------------------------

void NumberEdit::trimTrailingZeros(CString& s) const
{
	const int p = s.ReverseFind('.');
	if (p != -1)
	{
		bool quit = false;
		while (!quit && (s.GetLength() > 1) && (s[s.GetLength() - 1] == '0' || s[s.GetLength() - 1] == '.'))
		{
			if (s[s.GetLength() - 1] == '.')
				quit = true;

			s.Delete(s.GetLength() - 1);
		}
	}
}

//-------------------------------------------------------------------

NumberEdit::operator float() const
{
	return GetNumber();
}

//-------------------------------------------------------------------

NumberEdit& NumberEdit::operator=(float f)
{
	SetNumber(f);

	return *this;
}

//-------------------------------------------------------------------

void NumberEdit::SetNumberFloat(float f)
{
	CString s;
	s.Format("%1.5f", f);
	trimTrailingZeros(s);

	SetWindowText(s);
}

//-------------------------------------------------------------------

float NumberEdit::GetNumberFloat() const
{
	CString s;
	GetWindowText(s);

	return static_cast<float>(atof(s));
}

//-------------------------------------------------------------------

void NumberEdit::SetNumberInt(int i)
{
	CString s;
	s.Format("%i", i);
	trimTrailingZeros(s);

	SetWindowText(s);
}

//-------------------------------------------------------------------

int NumberEdit::GetNumberInt() const
{
	CString s;
	GetWindowText(s);

	return static_cast<int>(atoi(s));
}

//-------------------------------------------------------------------
