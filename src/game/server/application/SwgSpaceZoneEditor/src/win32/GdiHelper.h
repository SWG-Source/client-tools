// ======================================================================
//
// GdiHelper.h
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_GdiHelper_H
#define INCLUDED_GdiHelper_H

// ======================================================================

class GdiHelper
{
private:

	DWORD const m_black;
	DWORD const m_red;  
	DWORD const m_green;
	DWORD const m_blue; 
	DWORD const m_gray;
	DWORD const m_white;
	DWORD const m_yellow;

	CDC * const m_pDC;
	CPen m_penBlack1;
	CPen m_penBlack2;
	CPen m_penBlack3;
	CPen m_penRed1;  
	CPen m_penRed2;  
	CPen m_penRed3;
	CPen m_penGreen1;
	CPen m_penGreen2;
	CPen m_penGreen3;
	CPen m_penBlue1; 
	CPen m_penBlue2; 
	CPen m_penBlue3;
	CPen m_penGray1; 
	CPen m_penGray2; 
	CPen m_penGray3;
	CPen m_penWhite1; 
	CPen m_penDottedWhite1; 
	CPen m_penWhite2; 
	CPen m_penWhite3;
	CPen m_penYellow1; 
	CPen m_penYellow2; 
	CPen m_penYellow3;
	CPen * m_oldPen;
	CBrush m_brushBlack;
	CBrush m_brushGray;
	CBrush m_brushWhite;
	CBrush * m_oldBrush;

public:

	GdiHelper(CDC * const pDC) :
		m_pDC(pDC),
		m_black(RGB(0, 0, 0)),
		m_red(RGB(255, 0, 0)),
		m_green(RGB(0, 255, 0)),
		m_blue(RGB(0, 0, 255)),
		m_gray(RGB(128, 128, 128)),
		m_white(RGB(255, 255, 255)),
		m_yellow(RGB(255, 255, 0)),
		m_penBlack1(PS_SOLID, 1, m_black),
		m_penBlack2(PS_SOLID, 2, m_black),
		m_penBlack3(PS_SOLID, 3, m_black),
		m_penRed1(PS_SOLID, 1, m_red),
		m_penRed2(PS_SOLID, 2, m_red),
		m_penRed3(PS_SOLID, 3, m_red),
		m_penGreen1(PS_SOLID, 1, m_green),
		m_penGreen2(PS_SOLID, 2, m_green),
		m_penGreen3(PS_SOLID, 3, m_green),
		m_penBlue1(PS_SOLID, 1, m_blue),
		m_penBlue2(PS_SOLID, 2, m_blue),
		m_penBlue3(PS_SOLID, 3, m_blue),
		m_penGray1(PS_SOLID, 1, m_gray),
		m_penGray2(PS_SOLID, 2, m_gray),
		m_penGray3(PS_SOLID, 3, m_gray),
		m_penWhite1(PS_SOLID, 1, m_white),
		m_penDottedWhite1(PS_DOT, 1, m_white),
		m_penWhite2(PS_SOLID, 2, m_white),
		m_penWhite3(PS_SOLID, 3, m_white),
		m_penYellow1(PS_SOLID, 1, m_yellow),
		m_penYellow2(PS_SOLID, 2, m_yellow),
		m_penYellow3(PS_SOLID, 3, m_yellow),
		m_oldPen(m_pDC->SelectObject(&m_penBlack1)),
		m_brushBlack(m_black),
		m_brushGray(m_gray),
		m_brushWhite(m_white),
		m_oldBrush(m_pDC->SelectObject(&m_brushBlack))
	{
		pDC->SetBkColor(RGB(0,0,0));
		pDC->SetBkMode(TRANSPARENT);
	}

	~GdiHelper()
	{
		if (m_oldPen)
			m_pDC->SelectObject(m_oldPen);

		if (m_oldBrush)
			m_pDC->SelectObject(m_oldBrush);
	}
	
	void black1()  { m_pDC->SelectObject(&m_penBlack1); m_pDC->SetTextColor(m_black); }
	void black2()  { m_pDC->SelectObject(&m_penBlack2); m_pDC->SetTextColor(m_black); }
	void black3()  { m_pDC->SelectObject(&m_penBlack3); m_pDC->SetTextColor(m_black); }
	void red1()    { m_pDC->SelectObject(&m_penRed1);   m_pDC->SetTextColor(m_red);   }
	void red2()    { m_pDC->SelectObject(&m_penRed2);   m_pDC->SetTextColor(m_red);   }
	void red3()    { m_pDC->SelectObject(&m_penRed3);   m_pDC->SetTextColor(m_red);   }
	void green1()  { m_pDC->SelectObject(&m_penGreen1); m_pDC->SetTextColor(m_green); }
	void green2()  { m_pDC->SelectObject(&m_penGreen2); m_pDC->SetTextColor(m_green); }
	void green3()  { m_pDC->SelectObject(&m_penGreen3); m_pDC->SetTextColor(m_green); }
	void blue1()   { m_pDC->SelectObject(&m_penBlue1);  m_pDC->SetTextColor(m_blue);  }
	void blue2()   { m_pDC->SelectObject(&m_penBlue2);  m_pDC->SetTextColor(m_blue);  }
	void blue3()   { m_pDC->SelectObject(&m_penBlue3);  m_pDC->SetTextColor(m_blue);  }
	void gray1()   { m_pDC->SelectObject(&m_penGray1);  m_pDC->SetTextColor(m_gray);  }
	void gray2()   { m_pDC->SelectObject(&m_penGray2);  m_pDC->SetTextColor(m_gray);  }
	void gray3()   { m_pDC->SelectObject(&m_penGray3);  m_pDC->SetTextColor(m_gray);  }
	void white1()  { m_pDC->SelectObject(&m_penWhite1);  m_pDC->SetTextColor(m_white);  }
	void dottedWhite1()  { m_pDC->SelectObject(&m_penDottedWhite1);  m_pDC->SetTextColor(m_white);  }
	void white2()  { m_pDC->SelectObject(&m_penWhite2);  m_pDC->SetTextColor(m_white);  }
	void white3()  { m_pDC->SelectObject(&m_penWhite3);  m_pDC->SetTextColor(m_white);  }
	void yellow1() { m_pDC->SelectObject(&m_penYellow1);  m_pDC->SetTextColor(m_yellow);  }
	void yellow2() { m_pDC->SelectObject(&m_penYellow2);  m_pDC->SetTextColor(m_yellow);  }
	void yellow3() { m_pDC->SelectObject(&m_penYellow3);  m_pDC->SetTextColor(m_yellow);  }
	void line(int x0, int y0, int x1, int y1) { m_pDC->MoveTo(x0, y0); m_pDC->LineTo(x1, y1); }
	void line(const CPoint& p0, const CPoint& p1) { line(p0.x, p0.y, p1.x, p1.y); }
	void circle(int x, int y, int r) { m_pDC->Ellipse(x - r, y - r, x + r, y + r); }
	void circle(const CPoint& p, int r) { circle(p.x, p.y, r); }
};

// ======================================================================

#endif
