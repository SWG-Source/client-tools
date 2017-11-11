#ifndef __DIRECT3DPRIMARYCANVAS_H__
#define __DIRECT3DPRIMARYCANVAS_H__

#include "UICanvas.h"

#include <d3d.h>
#include <list>
#include <set>

class UIDirect3DTextureCanvas;
typedef std::set<UIDirect3DTextureCanvas *> UIDirect3DTextureCanvasSet;

class UIDirect3DPrimaryCanvas : public UICanvas
{
public:
	typedef std::list<UIQuad> UIQuadList;

													UIDirect3DPrimaryCanvas( const UISize &, HWND, bool );
	virtual								 ~UIDirect3DPrimaryCanvas();
													
	virtual bool						IsA( const UITypeID ) const;	
	virtual UIBaseObject	 *Clone( void ) const;
												
	virtual void						Attach( UIBaseObject * );
	virtual void						Detach( UIBaseObject * = 0 );
													
	virtual void						SetSize( const UISize & );
													
	virtual bool						Prepare( void ) const;	
	virtual bool						Generate( void );
													
	virtual	void						BltFrom( const UICanvas * const src, const UIPoint &Source, const UIPoint &Destination, const UISize &Size );
	virtual void                        BltFromNoScaleOrRotate( const UICanvas * const src, const UIPoint &Source, const UIPoint &Destination, const UISize &Size );
	virtual void                        RenderLines      (const UICanvas * const src, int numLines,   const UILine * lines, const UILine * uvs);
	virtual void                        RenderLines(const UICanvas * const , int numLines,   const UILine * lines, const UIFloatPoint * uvs, const UIColor * colors );
	virtual void                        ClearTo              (const UIColor &c, const UIRect & rc);
	void                                RenderTriangles      (const UICanvas * const src, int numTris, const UITriangle * tris, const UITriangle * uvs);

//	virtual void						ClearTo( const UIColor &, const UIRect &);

	virtual void						EnableFiltering( bool );
													
	virtual bool						BeginRendering( void );
	virtual void						EndRendering( void );
													
	virtual void						Flip( void );
	void						        ShowTriangles ( bool );
	void						        ShowShaders   ( bool );
	bool                                GetShowShaders () const;
	virtual long						GetTriangleCount( void ) const;	
													
	virtual HWND						GetWindow( void );
													
	virtual void						GetPreferedPixelFormat( LPDDPIXELFORMAT );
	virtual bool						MatchPixelFormat( LPDDPIXELFORMAT In, LPDDPIXELFORMAT Out );

	LPDIRECT3DDEVICE7 getDevice () { return mRenderDevice; }

private:


	UIDirect3DPrimaryCanvas & operator=                 (const UIDirect3DPrimaryCanvas &);
	                          UIDirect3DPrimaryCanvas   (const UIDirect3DPrimaryCanvas &);

	virtual void						RenderQuad( const UICanvas * const src, const UIFloatPoint Vertices[4], const UIFloatPoint UVs[4] );
	virtual void						RenderQuad( const UICanvas * const src, const UIFloatPoint Vertices[4], const UIFloatPoint UVs[4], const UIColor Colors[4] );


	virtual void					  DestroyAll( void );

	virtual HRESULT					D3DEnumPixelFormats( LPDDPIXELFORMAT );
	static HRESULT CALLBACK D3DEnumPixelFormatsStaticCallback( LPDDPIXELFORMAT, LPVOID );

	virtual HRESULT					D3DEnumPixelFormatsForBestMatch( LPDDPIXELFORMAT );
	static HRESULT CALLBACK D3DEnumPixelFormatsForBestMatchStaticCallback( LPDDPIXELFORMAT, LPVOID );

	DDPIXELFORMAT								mPreferedTexturePixelFormat;
	LPDIRECTDRAWSURFACE7				mPrimarySurface;
	LPDIRECTDRAWSURFACE7				mBackBufferSurface;
	LPDIRECTDRAWSURFACE7				mCurrentTexture;
	LPDIRECT3DDEVICE7						mRenderDevice;
	LPDIRECTDRAWCLIPPER					mClipper;
	HWND												mWindow;
	bool												mIsFullscreen;	
	UIDirect3DTextureCanvasSet	mAttachedTextures;

	long												mTriangleCount;
	bool												mShowTriangles;
	bool                                                mShowShaders;
	UIQuadList									mQuads;
	UIQuadList									mShaderQuads;

	bool                        mBestMatchFound;
	DDPIXELFORMAT								mFormatToMatch;
	DDPIXELFORMAT								mBestMatchPixelFormat;
};


#endif //  __DIRECT3DPRIMARYCANVAS_H__

