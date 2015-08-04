//===================================================================
//
// GdiHelper.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_GdiHelper_H
#define INCLUDED_GdiHelper_H

//===================================================================

class GdiHelper
{
private:

	DWORD m_black;
	DWORD m_red;  
	DWORD m_green;
	DWORD m_blue; 

	CDC*  m_pDC;
	CPen  m_penBlack1;
	CPen  m_penBlack2;
	CPen  m_penBlack3;
	CPen  m_penRed1;  
	CPen  m_penRed2;  
	CPen  m_penRed3;
	CPen  m_penGreen1;
	CPen  m_penGreen2;
	CPen  m_penGreen3;
	CPen  m_penBlue1; 
	CPen  m_penBlue2; 
	CPen  m_penBlue3;
	CPen* m_oldPen;

public:

	GdiHelper (CDC* pDC) :
		m_pDC       (pDC),
		m_black     (RGB (0, 0, 0)),
		m_red       (RGB (255, 0, 0)),
		m_green     (RGB (0, 255, 0)),
		m_blue      (RGB (0, 0, 255)),
		m_penBlack1 (PS_SOLID, 1, m_black),
		m_penBlack2 (PS_SOLID, 2, m_black),
		m_penBlack3 (PS_SOLID, 3, m_black),
		m_penRed1   (PS_SOLID, 1, m_red),
		m_penRed2   (PS_SOLID, 2, m_red),
		m_penRed3   (PS_SOLID, 3, m_red),
		m_penGreen1 (PS_SOLID, 1, m_green),
		m_penGreen2 (PS_SOLID, 2, m_green),
		m_penGreen3 (PS_SOLID, 3, m_green),
		m_penBlue1  (PS_SOLID, 1, m_blue),
		m_penBlue2  (PS_SOLID, 2, m_blue),
		m_penBlue3  (PS_SOLID, 3, m_blue),
		m_oldPen    (m_pDC->SelectObject (&m_penBlack1))
	{
	}

	~GdiHelper ()
	{
		if (m_oldPen)
			m_pDC->SelectObject (m_oldPen);
	}
	
	void black1 () { m_pDC->SelectObject (&m_penBlack1); m_pDC->SetTextColor (m_black); }
	void black2 () { m_pDC->SelectObject (&m_penBlack2); m_pDC->SetTextColor (m_black); }
	void black3 () { m_pDC->SelectObject (&m_penBlack3); m_pDC->SetTextColor (m_black); }
	void red1 ()   { m_pDC->SelectObject (&m_penRed1);   m_pDC->SetTextColor (m_red);   }
	void red2 ()   { m_pDC->SelectObject (&m_penRed2);   m_pDC->SetTextColor (m_red);   }
	void red3 ()   { m_pDC->SelectObject (&m_penRed3);   m_pDC->SetTextColor (m_red);   }
	void green1 () { m_pDC->SelectObject (&m_penGreen1); m_pDC->SetTextColor (m_green); }
	void green2 () { m_pDC->SelectObject (&m_penGreen2); m_pDC->SetTextColor (m_green); }
	void green3 () { m_pDC->SelectObject (&m_penGreen3); m_pDC->SetTextColor (m_green); }
	void blue1 ()  { m_pDC->SelectObject (&m_penBlue1);  m_pDC->SetTextColor (m_blue);  }
	void blue2 ()  { m_pDC->SelectObject (&m_penBlue2);  m_pDC->SetTextColor (m_blue);  }
	void blue3 ()  { m_pDC->SelectObject (&m_penBlue3);  m_pDC->SetTextColor (m_blue);  }
	void line (int x0, int y0, int x1, int y1) { m_pDC->MoveTo (x0, y0); m_pDC->LineTo (x1, y1); }
	void line (const CPoint& p0, const CPoint& p1) { line (p0.x, p0.y, p1.x, p1.y); }
	void circle (int x, int y, int r) { m_pDC->Ellipse (x - r, y - r, x + r, y + r); }
	void circle (const CPoint& p, int r) { circle (p.x, p.y, r); }
};

//===================================================================

#endif
