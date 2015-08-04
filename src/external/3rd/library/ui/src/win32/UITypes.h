// ======================================================================
//
// UITypes.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef __UITYPES_H__
#define __UITYPES_H__

//-----------------------------------------------------------------

typedef long                    UIScalar;
typedef unsigned long           UITime;


//======================================================================================
struct UIColor
{
public:
	
	              UIColor();
	              UIColor( int nr, int ng, int nb );
	              UIColor( unsigned char nr, unsigned char ng, unsigned char nb );
	              UIColor( unsigned char nr, unsigned char ng, unsigned char nb, unsigned char na );
	
	unsigned long  FormatRGBA( void ) const;
	
	void           Set( const UIColor &NewColor );
	void           Set( unsigned char nr, unsigned char ng, unsigned char nb );
	void           Set( unsigned char nr, unsigned char ng, unsigned char nb, unsigned char na );	
	void           Set( unsigned long c );

	static UIColor lerp (const UIColor & start, const UIColor & end, float f);

	bool           operator == (const UIColor & rhs) const;
	bool           operator != (const UIColor & rhs) const;

	//-----------------------------------------------------------------

	unsigned char r, g, b, a;

	static const UIColor white;
	static const UIColor black;
	static const UIColor red;
	static const UIColor yellow;
	static const UIColor green;
	static const UIColor cyan;
	static const UIColor blue;
	static const UIColor violet;
};

//----------------------------------------------------------------------

inline bool UIColor::operator == (const UIColor & rhs) const
{
	return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
}

//----------------------------------------------------------------------

inline bool UIColor::operator != (const UIColor & rhs) const
{
	return !(*this == rhs);
}

//======================================================================================
struct UIFloatPoint;

struct UIPoint
{
public:

	UIPoint();
	UIPoint( UIScalar nx, UIScalar ny );
	explicit UIPoint(UIFloatPoint const & floatPoint);

	void		 Set( UIScalar nx, UIScalar ny );
	
	UIPoint &operator += (const UIPoint &rhs);
	UIPoint &operator -= (const UIPoint &rhs);
	UIPoint &operator /= (const UIScalar rhs);
	UIPoint &operator *= (const UIScalar rhs);
	UIPoint &operator *= (const float rhs);
	UIPoint &operator *= (const UIFloatPoint & rhs);

	bool     operator != (const UIPoint &rhs) const;
	bool     operator == (const UIPoint &rhs) const;

	float    Magnitude       () const;
	float    DistanceSquared (const UIPoint & rhs) const;

	static UIPoint lerp (const UIPoint & start, const UIPoint & end, float t);

	UIScalar x, y;

	static const UIPoint zero;
	static const UIPoint one;
};

typedef UIPoint UISize;

inline UIPoint operator - (const UIPoint &rhs)                      { return UIPoint(-rhs.x,          -rhs.y); }
inline UIPoint operator + (const UIPoint &lhs, const UIPoint &rhs)  { return UIPoint( lhs.x + rhs.x,  lhs.y + rhs.y ); }
inline UIPoint operator - (const UIPoint &lhs, const UIPoint &rhs)  { return UIPoint( lhs.x - rhs.x,  lhs.y - rhs.y ); }
inline UIPoint operator / (const UIPoint &lhs, const UIScalar rhs)  { return UIPoint( lhs.x / rhs,    lhs.y / rhs ); }
inline UIPoint operator * (const UIPoint &lhs, const UIScalar rhs)  { return UIPoint( lhs.x * rhs,    lhs.y * rhs ); }
inline UIPoint operator / (const UIPoint &lhs, const UIPoint & rhs) { return UIPoint( lhs.x / rhs.x,  lhs.y / rhs.y ); }
inline UIPoint operator * (const UIPoint &lhs, const UIPoint & rhs) { return UIPoint( lhs.x * rhs.x,  lhs.y * rhs.y ); }
inline UIPoint operator * (const UIPoint &lhs, const float rhs)     { return UIPoint( static_cast<long>(lhs.x * rhs),  static_cast<long>(lhs.y * rhs) ); }

//======================================================================================
struct UIFloatPoint
{
public:

	UIFloatPoint() : x (0.0f), y (0.0f) {};
	UIFloatPoint( float nx, float ny ) : x (nx), y (ny) {}
	explicit UIFloatPoint (const UIPoint & pt) : x (static_cast<float>(pt.x)), y (static_cast<float>(pt.y)) {}

	void          Set( float nx, float ny ){ x = nx; y = ny; };
	
	UIFloatPoint &operator += (const UIFloatPoint &rhs)      { x += rhs.x; y += rhs.y; return *this; };
	UIFloatPoint &operator -= (const UIFloatPoint &rhs)      { x -= rhs.x; y -= rhs.y; return *this; };
	UIFloatPoint &operator *= (const UIFloatPoint &rhs)      { x *= rhs.x; y *= rhs.y; return *this; };
	UIFloatPoint &operator /= (const float rhs)              { x /= rhs; y /= rhs; return *this; };
	UIFloatPoint &operator *= (const float rhs)              { x *= rhs; y *= rhs; return *this; };

	bool     operator != (const UIFloatPoint &rhs) const
	{
		return (x != rhs.x) || (y != rhs.y); //lint !e777 // testing floats for equality
	};

	bool     operator == (const UIFloatPoint &rhs) const
	{
		return (x == rhs.x) && (y == rhs.y); //lint !e777 // testing floats for equality
	};

	float    Magnitude       () const;
	float    DistanceSquared (const UIFloatPoint & rhs) const;
		
	static UIFloatPoint lerp (const UIFloatPoint & start, const UIFloatPoint & end, float t);

	float x, y;

	static const UIFloatPoint zero;
	static const UIFloatPoint one;
};

inline UIFloatPoint operator - (const UIFloatPoint &rhs) { return UIFloatPoint(-rhs.x, -rhs.y); };
inline UIFloatPoint operator + (const UIFloatPoint &lhs, const UIFloatPoint &rhs){ return UIFloatPoint( lhs.x + rhs.x, lhs.y + rhs.y ); };
inline UIFloatPoint operator - (const UIFloatPoint &lhs, const UIFloatPoint &rhs){ return UIFloatPoint( lhs.x - rhs.x, lhs.y - rhs.y ); };
inline UIFloatPoint operator / (const UIFloatPoint &lhs, const float rhs){ return UIFloatPoint( lhs.x / rhs, lhs.y / rhs ); };
inline UIFloatPoint operator * (const UIFloatPoint &lhs, const float rhs){ return UIFloatPoint( lhs.x * rhs, lhs.y * rhs ); };

//======================================================================================

struct UILine
{
public:
	UIFloatPoint p1, p2;

	UILine (const UIFloatPoint & _p1, const UIFloatPoint & _p2);
	UILine ();
};

//----------------------------------------------------------------------

struct UITriangle
{
public:
	UIFloatPoint p1, p2, p3;

	UITriangle (const UIFloatPoint & _p1, const UIFloatPoint & p2, const UIFloatPoint & p3);
	UITriangle ();
};

//======================================================================================

struct UIRect
{
public:

	UIRect() : left (0), top(0), right (0), bottom (0) {};
	UIRect( UIScalar nleft, UIScalar ntop, UIScalar nright, UIScalar nbottom ) : left (nleft), top (ntop), right (nright), bottom (nbottom) {}
	UIRect( const UIRect &r ) : left (r.left), top (r.top), right (r.right), bottom (r.bottom) {}
	explicit UIRect( const UISize &s ) : left (0), top (0), right (s.x), bottom (s.y) {}
	explicit UIRect( const UIPoint & pt, const UISize &s ) : left (pt.x), top (pt.y), right (pt.x + s.x), bottom (pt.y + s.y) {}

	void           Set( UIScalar nleft, UIScalar ntop, UIScalar nright, UIScalar nbottom );
	void           Set( const UIPoint & pt, const UISize & size);
	UIScalar       Width( void ) const;
	UIScalar       Height( void ) const;
	const UISize   Size( void ) const;
	const UIPoint  Location (void) const;
	UIPoint GetCenter() const { return UIPoint((left + right) / 2, (top + bottom) / 2); }

	bool     ContainsPoint( const UIPoint &p ) const { return (p.x >= left) && (p.x <= right) && (p.y >= top) && (p.y <= bottom); };

	bool     operator != (const UIRect &rhs) const { return (left != rhs.left) || (top != rhs.top) || (right != rhs.right) || (bottom != rhs.bottom); };
	bool     operator == (const UIRect &rhs) const { return (left == rhs.left) && (top == rhs.top) && (right == rhs.right) && (bottom == rhs.bottom); };
	
	void     operator += (const UIPoint &rhs) { left += rhs.x; right += rhs.x; top += rhs.y; bottom += rhs.y; };
	void     operator -= (const UIPoint &rhs) { left -= rhs.x; right -= rhs.x; top -= rhs.y; bottom -= rhs.y; };

	void Extend(UIRect const & rect);

	UIScalar left, top;
	UIScalar right, bottom;

	static const UIRect zero;
	static const UIRect empty;
};

//-----------------------------------------------------------------

inline void           UIRect::Set      ( UIScalar nleft, UIScalar ntop, UIScalar nright, UIScalar nbottom )
{
	left = nleft; top = ntop; right = nright; bottom = nbottom;
}

inline void           UIRect::Set      ( const UIPoint & pt, const UISize & size)
{
	Set (pt.x, pt.y, pt.x + size.x, pt.y + size.y);
}

inline UIScalar       UIRect::Width    () const { return right - left; };
inline UIScalar       UIRect::Height   () const { return bottom - top; };
inline const UISize   UIRect::Size     () const { return UISize( right - left, bottom - top); };
inline const UIPoint  UIRect::Location () const { return UIPoint (left, top); }

//======================================================================================
struct UIQuad
{
public:

	UIQuad( UIScalar x1, UIScalar y1, UIScalar x2, UIScalar y2, UIScalar x3, UIScalar y3, UIScalar x4, UIScalar y4 );
	UIQuad ();

	UIPoint  p1, p2, p3, p4;
};

//-----------------------------------------------------------------
//-----------------------------------------------------------------
inline UIQuad::UIQuad( UIScalar nx1, UIScalar ny1,
                       UIScalar nx2, UIScalar ny2,
                       UIScalar nx3, UIScalar ny3,
                       UIScalar nx4, UIScalar ny4 ) :
p1 (nx1, ny1),
p2 (nx2, ny2),
p3 (nx3, ny3),
p4 (nx4, ny4)
{
};
//-----------------------------------------------------------------

inline UIQuad::UIQuad () :
p1 (0, 0),
p2 (0, 0),
p3 (0, 0),
p4 (0, 0)
{
}


//-----------------------------------------------------------------
//-----------------------------------------------------------------

inline UIColor::UIColor() :
r (0), g (0), b (0), a (0)
{
}

//-----------------------------------------------------------------

inline UIColor::UIColor( unsigned char nr, unsigned char ng, unsigned char nb ) :
r (nr), g (ng), b (nb), a (255)
{
}

//-----------------------------------------------------------------

inline UIColor::UIColor(int nr, int ng, int nb) :
r((unsigned char const)nr), g((unsigned char const)ng), b((unsigned char const)nb), a(255)
{
}

//-----------------------------------------------------------------

inline UIColor::UIColor( unsigned char nr, unsigned char ng, unsigned char nb, unsigned char na ) :
r (nr), g (ng), b (nb), a (na) 
{
}


//-----------------------------------------------------------------

inline unsigned long UIColor::FormatRGBA( void ) const
{
	return (a << 24) | (r << 16) | (g << 8) | b;
}
//-----------------------------------------------------------------

inline void   UIColor::Set( unsigned char nr, unsigned char ng, unsigned char nb, unsigned char na )
{ 
	r = nr;
	g = ng;
	b = nb;
	a = na;
}
//-----------------------------------------------------------------

inline void   UIColor::Set( const UIColor &NewColor )
{
	Set (NewColor.r, NewColor.g, NewColor.b, NewColor.a);
}
//-----------------------------------------------------------------

inline void   UIColor::Set( unsigned char nr, unsigned char ng, unsigned char nb )
{
	Set (nr, ng, nb, 255);
}

//-----------------------------------------------------------------

inline void   UIColor::Set(unsigned long c)
{
	Set (static_cast<unsigned char>((c >> 16) & 0xff),
		static_cast<unsigned char>((c>>8) & 0xff),
		static_cast<unsigned char>(c & 0xff),
		static_cast<unsigned char>(c >> 24 ));
};
//-----------------------------------------------------------------
//-----------------------------------------------------------------

inline UIPoint::UIPoint() :
x (0), y (0)
{
}
//-----------------------------------------------------------------

inline UIPoint::UIPoint( UIScalar nx, UIScalar ny ) :
x (nx),
y (ny)
{
}
//-----------------------------------------------------------------
inline void UIPoint::Set( UIScalar nx, UIScalar ny )
{ 
	x = nx;
	y = ny;
};

//-----------------------------------------------------------------

inline UIPoint & UIPoint::operator += (const UIPoint &rhs)
{
	x += rhs.x;
	y += rhs.y;
	return *this;
};
//-----------------------------------------------------------------

inline UIPoint & UIPoint::operator -= (const UIPoint &rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}
//-----------------------------------------------------------------

inline UIPoint & UIPoint::operator /= (const UIScalar rhs)
{
	x /= rhs;
	y /= rhs;
	return *this;
}
//-----------------------------------------------------------------

inline UIPoint & UIPoint::operator *= (const UIScalar rhs)
{ 
	x *= rhs;
	y *= rhs;
	return *this;
}

//----------------------------------------------------------------------

inline UIPoint & UIPoint::operator *= (const float rhs)
{
	x = static_cast<long>(static_cast<float>(x) * rhs);
	y = static_cast<long>(static_cast<float>(y) * rhs);
	return *this;
}

//-----------------------------------------------------------------

inline bool      UIPoint::operator != (const UIPoint &rhs) const
{ 
	return (x != rhs.x) || (y != rhs.y);
}
//-----------------------------------------------------------------

inline bool      UIPoint::operator == (const UIPoint &rhs) const { 
	return (x == rhs.x) && (y == rhs.y);
}

//----------------------------------------------------------------------

inline UILine::UILine (const UIFloatPoint & _p1, const UIFloatPoint & _p2) :
p1 (_p1),
p2 (_p2)
{
}

//----------------------------------------------------------------------

inline UILine::UILine () :
p1 (),
p2 ()
{
}

//----------------------------------------------------------------------

inline UITriangle::UITriangle (const UIFloatPoint & _p1, const UIFloatPoint & _p2, const UIFloatPoint & _p3) :
p1 (_p1),
p2 (_p2),
p3 (_p3)
{
}

//----------------------------------------------------------------------

inline UITriangle::UITriangle () :
p1 (),
p2 (),
p3 ()
{
}

//-----------------------------------------------------------------

template<class T>inline const T UI_CLAMP(const T &minT, const T &t, const T &maxT)
{
	return (t < minT) ? minT : ((t > maxT) ? maxT : t);
}

//-----------------------------------------------------------------

#endif
