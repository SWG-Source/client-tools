#ifndef __UIDIRECTDRAWCANVAS_H__
#define __UIDIRECTDRAWCANVAS_H__

#include <ddraw.h>

#include "UICanvas.h"

extern	LPDIRECTDRAW7				 gDirectDraw;

class UIDirectDrawCanvas : public UICanvas
{
public:

	static const char    *TypeName;

										    UIDirectDrawCanvas( UISize );
	virtual						   ~UIDirectDrawCanvas();
										
	virtual bool			    IsA( const UITypeID ) const;
	virtual UIBaseObject *Clone( void ) const;
										
	virtual bool			    Generate( void ) const = 0;
										
	virtual void			    SetSize( const UISize & );
										
					bool			    Prepare( void ) const;
	virtual void			    BltFrom( const UICanvas * const src, const UIPoint &Source, const UIPoint &Destination, const UISize &Size );

	// Used by generator functions
	virtual LPDIRECTDRAWSURFACE7 GetSurface( void ) const;
	virtual void			    SetSurface( LPDIRECTDRAWSURFACE7 );

	virtual void          Refresh( void );
										
				 DWORD					FormatUIColorForDirectDrawSurface( const UIColor & ) const;
				 DWORD					FormatRGBForDirectDrawSurface( const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a = 0xFF ) const;
												
				  void					SetDirectDrawSurface16( unsigned char *p, const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a = 0xFF ) const;
				  void					SetDirectDrawSurface24( unsigned char *p, const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a = 0xFF ) const;
					void					SetDirectDrawSurface32( unsigned char *p, const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a = 0xFF ) const;

private:
	

	UIDirectDrawCanvas & operator=           (const UIDirectDrawCanvas &);
	                     UIDirectDrawCanvas  (const UIDirectDrawCanvas &);

protected:

	virtual void          DestoryAll( void );

	LPDIRECTDRAWSURFACE7  mSurface;	

	DWORD									mRedScale;
	DWORD									mRedShift;
	DWORD									mRedMask;
												
	DWORD									mGreenScale;
	DWORD									mGreenShift;
	DWORD									mGreenMask;
												
	DWORD									mBlueScale;
	DWORD									mBlueShift;
	DWORD									mBlueMask;
												
	DWORD									mAlphaScale;
	DWORD									mAlphaShift;
	DWORD									mAlphaMask;
};

inline DWORD UIDirectDrawCanvas::FormatUIColorForDirectDrawSurface( const UIColor &color ) const
{
	return (((DWORD)(color.r >> mRedScale) << mRedShift) & mRedMask) | 
		     (((DWORD)(color.g >> mGreenScale) << mGreenShift) & mGreenMask) | 
				 (((DWORD)(color.b >> mBlueScale) << mBlueShift) & mBlueMask) |
				 (((DWORD)(color.a >> mAlphaScale) << mAlphaShift) & mAlphaMask);
}

inline DWORD UIDirectDrawCanvas::FormatRGBForDirectDrawSurface( const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a ) const
{
	return (((DWORD)(r >> mRedScale  ) << mRedShift  ) & mRedMask  ) |
		     (((DWORD)(g >> mGreenScale) << mGreenShift) & mGreenMask) |
				 (((DWORD)(b >> mBlueScale ) << mBlueShift ) & mBlueMask ) |
				 (((DWORD)(a >> mAlphaScale) << mAlphaShift) & mAlphaMask);
}

inline void UIDirectDrawCanvas::SetDirectDrawSurface16( unsigned char *p, const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a ) const
{
	*(WORD *)(p) = static_cast<unsigned short> ((((WORD)((r >> mRedScale  ) << (WORD)mRedShift  ) & (WORD)mRedMask  )) |
		             (((WORD)((g >> mGreenScale) << (WORD)mGreenShift) & (WORD)mGreenMask)) |
								 (((WORD)((b >> mBlueScale ) << (WORD)mBlueShift ) & (WORD)mBlueMask )) |
								 (((WORD)((a >> mAlphaScale) << (WORD)mAlphaShift) & (WORD)mAlphaMask)));
}

inline void UIDirectDrawCanvas::SetDirectDrawSurface24( unsigned char *p, const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char ) const
{
	*p       = r;
	*(p + 1) = g;
	*(p + 2) = b;
}

inline void UIDirectDrawCanvas::SetDirectDrawSurface32( unsigned char *p, const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a ) const
{
	*(DWORD *)(p) = (((DWORD)(r >> mRedScale  ) << mRedShift  ) & mRedMask  ) |
		              (((DWORD)(g >> mGreenScale) << mGreenShift) & mGreenMask) |
									(((DWORD)(b >> mBlueScale ) << mBlueShift ) & mBlueMask ) |
								  (((DWORD)(a >> mAlphaScale) << mAlphaShift) & mAlphaMask);
}

typedef void (UIDirectDrawCanvas::*DDCSetPixelFunc)( unsigned char *, const unsigned char, const unsigned char, const unsigned char, const unsigned char ) const;

#endif // __UIDIRECTDRAWCANVAS_H__