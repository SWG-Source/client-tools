#include "FirstUiBuilder.h"
#include "UIDirect3DPrimaryCanvas.h"

#include "UIDirect3DTextureCanvas.h"
#include "UIUtils.h"

#include <cmath>

extern LPDIRECTDRAW7 gDirectDraw;
extern LPDIRECT3D7   gDirect3D;
LPDIRECTDRAWSURFACE7 gPrimarySurface = 0;

namespace UIDirect3DPrimaryCanvasNamespace
{
	void updateRenderState (LPDIRECT3DDEVICE7 device, const D3DRENDERSTATETYPE state, const DWORD newValue)
	{
		DWORD oldValue;
		if (device->GetRenderState (state, &oldValue) == D3D_OK && oldValue != newValue)
			device->SetRenderState (state, newValue);
	}

	void updateTextureStageState (LPDIRECT3DDEVICE7 device, const DWORD stage, const D3DTEXTURESTAGESTATETYPE type, const DWORD newValue)
	{
		DWORD oldValue;
		if (device->GetTextureStageState (stage, type, &oldValue) == D3D_OK && oldValue != newValue)
			device->SetTextureStageState (stage, type, newValue);
	}
}

using namespace UIDirect3DPrimaryCanvasNamespace;

UIDirect3DPrimaryCanvas::UIDirect3DPrimaryCanvas( const UISize &size, HWND hwnd, bool fullscreen ) : 
UICanvas( size ),
mQuads (),
mShaderQuads (),
mShowShaders (true)
{
	mWindow						 = hwnd;
	mIsFullscreen			 = fullscreen;

	mPrimarySurface		 = 0;
	mBackBufferSurface = 0;
	mRenderDevice			 = 0;
	mClipper					 = 0;
	mCurrentTexture		 = 0;
	mShowTriangles		 = false;

	memset( &mPreferedTexturePixelFormat, 0, sizeof( mPreferedTexturePixelFormat ) );
}

UIDirect3DPrimaryCanvas::~UIDirect3DPrimaryCanvas()
{
	DestroyAll();
}

bool UIDirect3DPrimaryCanvas::IsA( const UITypeID Type ) const
{
	return (Type == TUIDirect3DPrimaryCanvas) || UICanvas::IsA( Type );
}

UIBaseObject *UIDirect3DPrimaryCanvas::Clone( void ) const
{
	return 0;
}

void UIDirect3DPrimaryCanvas::Attach( UIBaseObject *o )
{
	UICanvas::Attach( o );

	if( o && o->IsA( TUIDirect3DTextureCanvas ) )
		mAttachedTextures.insert( reinterpret_cast<UIDirect3DTextureCanvas *>(o) );
}

void UIDirect3DPrimaryCanvas::Detach( UIBaseObject *o )
{
	if( o && o->IsA( TUIDirect3DTextureCanvas ) )
	{
		assert( mAttachedTextures.find( reinterpret_cast<UIDirect3DTextureCanvas *>(o) ) != mAttachedTextures.end() );
		mAttachedTextures.erase( mAttachedTextures.find( reinterpret_cast<UIDirect3DTextureCanvas *>(o) ) );
	}

	UICanvas::Detach( o );
}

void UIDirect3DPrimaryCanvas::DestroyAll( void )
{
	if( mClipper )
	{
		mClipper->Release();
		mClipper = 0;
	}

	if( mPrimarySurface )
	{
		mPrimarySurface->Release();
		mPrimarySurface = 0;
	}

	if( mBackBufferSurface )
	{
		mBackBufferSurface->Release();
		mBackBufferSurface = 0;
	}

	if( mRenderDevice )
	{
		mRenderDevice->Release();
		mRenderDevice = 0;
	}
}

void UIDirect3DPrimaryCanvas::SetSize( const UISize &NewSize )
{
	UICanvas::SetSize( NewSize );
	DestroyAll();
}

//----------------------------------------------------------------------

void UIDirect3DPrimaryCanvas::ShowTriangles( bool ShowTriangles )
{
	mShowTriangles = ShowTriangles;
}

//----------------------------------------------------------------------

void UIDirect3DPrimaryCanvas::ShowShaders (bool b)
{
	mShowShaders = b;
}

//----------------------------------------------------------------------

long UIDirect3DPrimaryCanvas::GetTriangleCount( void ) const
{
	return mTriangleCount;
}

bool UIDirect3DPrimaryCanvas::Prepare( void ) const
{
	return const_cast<UIDirect3DPrimaryCanvas *>( this )->Generate();
}

bool UIDirect3DPrimaryCanvas::Generate( void )
{
	if( mPrimarySurface )
	{
		if( mPrimarySurface && mPrimarySurface->IsLost() )
		{
			HRESULT hr = mPrimarySurface->Restore();

			if( FAILED( hr ) )
				return false;
		}

		if( mBackBufferSurface && mBackBufferSurface->IsLost() )
		{
			HRESULT hr = mBackBufferSurface->Restore();

			if( FAILED( hr ) )
				return false;
		}
		return true;
	}

	try
	{
		DDSURFACEDESC2	ddsd = { sizeof( ddsd ) };
		HRESULT					hr;

		if( mIsFullscreen )
		{
			ddsd.dwFlags						= DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
			ddsd.ddsCaps.dwCaps			= DDSCAPS_3DDEVICE | DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_PRIMARYSURFACE;
			ddsd.dwBackBufferCount	= 1;
			
			hr = gDirectDraw->CreateSurface( &ddsd, &mPrimarySurface, 0 );

			if( FAILED( hr ) )
				throw hr;
		}
		else
		{
			// Primary surface
			ddsd.dwFlags				= DDSD_CAPS;
			ddsd.ddsCaps.dwCaps	= DDSCAPS_PRIMARYSURFACE;
			
			hr = gDirectDraw->CreateSurface( &ddsd, &mPrimarySurface, 0 );
			
			if( FAILED( hr ) )
			{
				if( hr == DDERR_PRIMARYSURFACEALREADYEXISTS  )
				{
					mPrimarySurface = gPrimarySurface;
					mPrimarySurface->AddRef();
				}
				else
					throw hr;
			}
			else
			{
				gPrimarySurface = mPrimarySurface;
			}

			hr = gDirectDraw->CreateClipper( 0, &mClipper, 0 );		

			if( FAILED( hr ) )
				throw hr;

			hr = mClipper->SetHWnd( 0, mWindow );

			if( FAILED( hr ) )
				throw hr;

			// Back buffer
			ddsd.dwFlags				 = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
			ddsd.dwWidth				 = GetWidth();
			ddsd.dwHeight				 = GetHeight();
			ddsd.ddsCaps.dwCaps	 = DDSCAPS_3DDEVICE | DDSCAPS_OFFSCREENPLAIN;
			ddsd.ddsCaps.dwCaps2 = 0;

			hr = gDirectDraw->CreateSurface( &ddsd, &mBackBufferSurface, 0 );

			if( FAILED( hr ) )
				throw hr;
		}

		static const GUID *DevicePreference[] =
		{
			&IID_IDirect3DTnLHalDevice,
			&IID_IDirect3DHALDevice,
			&IID_IDirect3DRGBDevice,
		};

		for( int i = 0; i < (sizeof( DevicePreference ) / sizeof( *DevicePreference )); ++i )
		{
			DDPIXELFORMAT OldPreferedTexturePixelFormat;

			hr = gDirect3D->CreateDevice( *DevicePreference[i], mIsFullscreen ? mPrimarySurface : mBackBufferSurface, &mRenderDevice );

			if( FAILED(hr) )
				continue;

			D3DVIEWPORT7 vp;

			vp.dwX      = 0;
			vp.dwY		  = 0;
			vp.dwWidth  = GetWidth();
			vp.dwHeight = GetHeight();
			vp.dvMinZ   = 0.0f;
			vp.dvMaxZ   = 1.0f;

			hr = mRenderDevice->SetViewport( &vp );

			if( FAILED(hr) )
				throw hr;

			updateRenderState( mRenderDevice, D3DRENDERSTATE_LIGHTING, FALSE );
			updateRenderState( mRenderDevice, D3DRENDERSTATE_CLIPPING, FALSE );
			updateRenderState( mRenderDevice, D3DRENDERSTATE_CULLMODE, D3DCULL_NONE );
			updateRenderState( mRenderDevice, D3DRENDERSTATE_COLORVERTEX, FALSE );
			updateRenderState( mRenderDevice, D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS );
			updateRenderState( mRenderDevice, D3DRENDERSTATE_LOCALVIEWER, FALSE );
			updateRenderState( mRenderDevice, D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			updateRenderState( mRenderDevice, D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );
			updateRenderState( mRenderDevice, D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );

			updateTextureStageState( mRenderDevice, 0, D3DTSS_MAGFILTER, D3DTFG_POINT );
			updateTextureStageState( mRenderDevice, 0, D3DTSS_MINFILTER, D3DTFG_POINT );

			updateTextureStageState( mRenderDevice, 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
			updateTextureStageState( mRenderDevice, 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			updateTextureStageState( mRenderDevice, 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
			updateTextureStageState( mRenderDevice, 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
			updateTextureStageState( mRenderDevice, 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			updateTextureStageState( mRenderDevice, 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

			updateTextureStageState( mRenderDevice, 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP );
			updateTextureStageState( mRenderDevice, 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP );

			OldPreferedTexturePixelFormat = mPreferedTexturePixelFormat;
			ZeroMemory( &mPreferedTexturePixelFormat, sizeof(mPreferedTexturePixelFormat) );
			hr = mRenderDevice->EnumTextureFormats( D3DEnumPixelFormatsStaticCallback, this );

			if( FAILED(hr) )
				throw hr;

			if( memcmp( &OldPreferedTexturePixelFormat, &mPreferedTexturePixelFormat, sizeof( mPreferedTexturePixelFormat ) ) )
			{
				UIDirect3DTextureCanvasSet::iterator i;

				for( i = mAttachedTextures.begin(); i != mAttachedTextures.end(); ++i )
					(*i)->NotifyRenderCanvasChanged();
			}

			return true;
		}		

		throw hr;
	}
	catch( HRESULT hr )
	{
		hr = hr;

		if( mClipper )
		{
			mClipper->Release();
			mClipper = 0;
		}

		if( mPrimarySurface )
		{
			mPrimarySurface->Release();
			mPrimarySurface = 0;
		}

		if( mBackBufferSurface )
		{
			mBackBufferSurface->Release();
			mBackBufferSurface = 0;
		}

		if( mRenderDevice )
		{
			mRenderDevice->Release();
			mRenderDevice = 0;
		}
	}

	return false;

}
void UIDirect3DPrimaryCanvas::ClearTo( const UIColor &c, const UIRect & rc)
{
	//-----------------------------------------------------------------
	//-- don't count triangles for clearing

	mTriangleCount -= 2;

	UICanvas::ClearTo (c, rc);
}

void UIDirect3DPrimaryCanvas::EnableFiltering( bool NewValue )
{
	if( NewValue )
	{
		updateTextureStageState( mRenderDevice, 0, D3DTSS_MAGFILTER, D3DTFG_LINEAR );
		updateTextureStageState( mRenderDevice, 0, D3DTSS_MINFILTER, D3DTFG_LINEAR );		
	}
	else
	{
		updateTextureStageState( mRenderDevice, 0, D3DTSS_MAGFILTER, D3DTFG_POINT );
		updateTextureStageState( mRenderDevice, 0, D3DTSS_MINFILTER, D3DTFG_POINT );
	}
}

//----------------------------------------------------------------------
//
// Verts are passed in in this order:
// 0-----1
// |    /|
// |   / |
// |  /  |
// | /   |
// |/    |
// 2-----3

void UIDirect3DPrimaryCanvas::RenderQuad( const UICanvas * const src, const UIFloatPoint VerticesIn[4], const UIFloatPoint UVs[4] )
{
	static D3DTLVERTEX D3DVertices[4] =
	{
		{ 0, 0, 0, 1, 0, 0, 0, 0 },
		{ 0, 0, 0, 1, 0, 0, 0, 0 },
		{ 0, 0, 0, 1, 0, 0, 0, 0 },
		{ 0, 0, 0, 1, 0, 0, 0, 0 },
	};

	const UIDirect3DTextureCanvas * SourceCanvas = 0;

	if( src )
	{
		assert( src->IsA( TUIDirect3DTextureCanvas ) );

		SourceCanvas = static_cast<const UIDirect3DTextureCanvas *>(src);

//		if( mCurrentTexture != SourceCanvas->GetSurface() )
		{
			mCurrentTexture = SourceCanvas->GetSurface();
			mRenderDevice->SetTexture( 0, mCurrentTexture );
		}		
	}
	else
		mRenderDevice->SetTexture( 0, 0 );
	
	DWORD dwVertexColor = mState.Color.FormatRGBA();

	UIFloatPoint const * Vertices = VerticesIn;
	if (IsDeforming())
	{
		UIFloatPoint NewVertices[4];
		Deform(Vertices, NewVertices, 4);
		Vertices = NewVertices;
	}

	// Copy data to vertex buffer, offseting geometry
	// so texture centers lie on pixel centers	
	D3DVertices[0].sx			= Vertices[0].x - 0.5f;
	D3DVertices[0].sy			= Vertices[0].y - 0.5f;
	D3DVertices[0].tu			= UVs[0].x;
	D3DVertices[0].tv			= UVs[0].y;
	D3DVertices[0].color	= dwVertexColor;
	D3DVertices[1].sx			= Vertices[1].x - 0.5f;
	D3DVertices[1].sy			= Vertices[1].y - 0.5f;
	D3DVertices[1].tu			= UVs[1].x;
	D3DVertices[1].tv			= UVs[1].y;
	D3DVertices[1].color	= dwVertexColor;
	D3DVertices[2].sx			= Vertices[2].x - 0.5f;
	D3DVertices[2].sy			= Vertices[2].y - 0.5f;
	D3DVertices[2].tu			= UVs[2].x;
	D3DVertices[2].tv			= UVs[2].y;
	D3DVertices[2].color	= dwVertexColor;
	D3DVertices[3].sx			= Vertices[3].x - 0.5f;
	D3DVertices[3].sy			= Vertices[3].y - 0.5f;
	D3DVertices[3].tu			= UVs[3].x;
	D3DVertices[3].tv			= UVs[3].y;
	D3DVertices[3].color	= dwVertexColor;

	mTriangleCount += 2;

	UI_IGNORE_RETURN (mRenderDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, &D3DVertices, 4, 0 ));

	if (mShowTriangles)
	{		
		mQuads.push_back( 
			UIQuad(
			static_cast<long>(Vertices [0].x), static_cast<long>(Vertices[0].y),
			static_cast<long>(Vertices [1].x), static_cast<long>(Vertices[1].y),
			static_cast<long>(Vertices [2].x), static_cast<long>(Vertices[2].y),
			static_cast<long>(Vertices [3].x), static_cast<long>(Vertices[3].y)));
	}

	if (mShowShaders && SourceCanvas && SourceCanvas->GetHasShader ())
	{
		mShaderQuads.push_back( 
			UIQuad(
			static_cast<long>(Vertices [0].x), static_cast<long>(Vertices[0].y),
			static_cast<long>(Vertices [1].x), static_cast<long>(Vertices[1].y),
			static_cast<long>(Vertices [2].x), static_cast<long>(Vertices[2].y),
			static_cast<long>(Vertices [3].x), static_cast<long>(Vertices[3].y)));
	}
}


//----------------------------------------------------------------------
//
// Verts are passed in in this order:
// 0-----1
// |    /|
// |   / |
// |  /  |
// | /   |
// |/    |
// 2-----3

void UIDirect3DPrimaryCanvas::RenderQuad( const UICanvas * const src, const UIFloatPoint VerticesIn[4], const UIFloatPoint UVs[4], const UIColor Colors[4] )
{
	static D3DTLVERTEX D3DVertices[4] =
	{
		{ 0, 0, 0, 1, 0, 0, 0, 0 },
		{ 0, 0, 0, 1, 0, 0, 0, 0 },
		{ 0, 0, 0, 1, 0, 0, 0, 0 },
		{ 0, 0, 0, 1, 0, 0, 0, 0 },
	};
	
	const UIDirect3DTextureCanvas * SourceCanvas = 0;
	
	if( src )
	{
		assert( src->IsA( TUIDirect3DTextureCanvas ) );
		
		SourceCanvas = static_cast<const UIDirect3DTextureCanvas *>(src);
		
		//		if( mCurrentTexture != SourceCanvas->GetSurface() )
		{
			mCurrentTexture = SourceCanvas->GetSurface();
			mRenderDevice->SetTexture( 0, mCurrentTexture );
		}		
	}
	else
		mRenderDevice->SetTexture( 0, 0 );

	UIFloatPoint const * Vertices = VerticesIn;
	if (IsDeforming())
	{
		UIFloatPoint NewVertices[4];
		Deform(Vertices, NewVertices, 4);
		Vertices = NewVertices;
	}
	
	// Copy data to vertex buffer, offseting geometry
	// so texture centers lie on pixel centers	
	D3DVertices[0].sx			= Vertices[0].x - 0.5f;
	D3DVertices[0].sy			= Vertices[0].y - 0.5f;
	D3DVertices[0].tu			= UVs[0].x;
	D3DVertices[0].tv			= UVs[0].y;
	D3DVertices[0].color	= Colors[0].FormatRGBA();
	D3DVertices[1].sx			= Vertices[1].x - 0.5f;
	D3DVertices[1].sy			= Vertices[1].y - 0.5f;
	D3DVertices[1].tu			= UVs[1].x;
	D3DVertices[1].tv			= UVs[1].y;
	D3DVertices[1].color	= Colors[1].FormatRGBA();
	D3DVertices[2].sx			= Vertices[2].x - 0.5f;
	D3DVertices[2].sy			= Vertices[2].y - 0.5f;
	D3DVertices[2].tu			= UVs[2].x;
	D3DVertices[2].tv			= UVs[2].y;
	D3DVertices[2].color	= Colors[2].FormatRGBA();
	D3DVertices[3].sx			= Vertices[3].x - 0.5f;
	D3DVertices[3].sy			= Vertices[3].y - 0.5f;
	D3DVertices[3].tu			= UVs[3].x;
	D3DVertices[3].tv			= UVs[3].y;
	D3DVertices[3].color	= Colors[3].FormatRGBA();
	
	mTriangleCount += 2;
	
	UI_IGNORE_RETURN (mRenderDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, &D3DVertices, 4, 0 ));
	
	if (mShowTriangles)
	{		
		mQuads.push_back( 
			UIQuad(
			static_cast<long>(Vertices [0].x), static_cast<long>(Vertices[0].y),
			static_cast<long>(Vertices [1].x), static_cast<long>(Vertices[1].y),
			static_cast<long>(Vertices [2].x), static_cast<long>(Vertices[2].y),
			static_cast<long>(Vertices [3].x), static_cast<long>(Vertices[3].y)));
	}
	
	if (mShowShaders && SourceCanvas && SourceCanvas->GetHasShader ())
	{
		mShaderQuads.push_back( 
			UIQuad(
			static_cast<long>(Vertices [0].x), static_cast<long>(Vertices[0].y),
			static_cast<long>(Vertices [1].x), static_cast<long>(Vertices[1].y),
			static_cast<long>(Vertices [2].x), static_cast<long>(Vertices[2].y),
			static_cast<long>(Vertices [3].x), static_cast<long>(Vertices[3].y)));
	}
}

//----------------------------------------------------------------------

void UIDirect3DPrimaryCanvas::BltFrom( const UICanvas * const src, const UIPoint &Source, const UIPoint &Destination, const UISize &Size )
{
	if( src )
		static_cast<const UIDirect3DTextureCanvas *>( src )->Prepare();

	Prepare();
	
	UICanvas::BltFrom( src, Source, Destination, Size );
}

//----------------------------------------------------------------------

void UIDirect3DPrimaryCanvas::BltFromNoScaleOrRotate(  const UICanvas * const srcarg, const UIPoint &Source, const UIPoint &Destination, const UISize &Size )
{
	BltFrom( srcarg, Source, Destination, Size );
}


//----------------------------------------------------------------------

void UIDirect3DPrimaryCanvas::RenderLines      (const UICanvas * const , int numLines,   const UILine * lines, const UILine * )
{
	const DWORD dwVertexColor = mState.Color.FormatRGBA();
	
	mRenderDevice->SetTexture( 0, 0 );
	mCurrentTexture = 0;
	
	for (int i = 0; i < numLines; ++i)
	{
		const UILine & theLine = lines [i];
		
		static D3DTLVERTEX Vertices[2] =
		{
			{ 0, 0, 0, 1, 0, 0, 0, 0 },
			{ 0, 0, 0, 1, 0, 0, 0, 0 },
		};
		
		UIFloatPoint p;
		
		p = Transform( theLine.p1 );
		Vertices[0].sx		= p.x;
		Vertices[0].sy		= p.y;
		Vertices[0].color = dwVertexColor;
		
		p = Transform( theLine.p2 );
		Vertices[1].sx		= p.x;
		Vertices[1].sy		= p.y;
		Vertices[1].color = dwVertexColor;	
		
		mTriangleCount += 1;
		
		UI_IGNORE_RETURN (mRenderDevice->DrawPrimitive( D3DPT_LINELIST, D3DFVF_TLVERTEX, &Vertices, 2, 0 ));
	}
}

//----------------------------------------------------------------------

void UIDirect3DPrimaryCanvas::RenderLines(const UICanvas * const , int numLines, const UILine * lines, const UIFloatPoint * uvs, const UIColor * colors)
{
	mRenderDevice->SetTexture( 0, 0 );
	mCurrentTexture = 0;

	static D3DTLVERTEX Vertices[2];
	static UIFloatPoint p;
	
	for (int i = 0; i < numLines; ++i)
	{
		const UILine & theLine = lines [i];
		int index = i * 2;
		
		Vertices[0].sx		= theLine.p1.x;
		Vertices[0].sy		= theLine.p1.y;
		Vertices[0].dvTU	= (uvs + index)->x;
		Vertices[0].dvTV	= (uvs + index)->y;
		Vertices[0].color = (colors + index)->FormatRGBA();
		
		Vertices[1].sx		= theLine.p2.x;
		Vertices[1].sy		= theLine.p2.y;
		Vertices[1].dvTU	= (uvs + index + 1)->x;
		Vertices[1].dvTV	= (uvs + index + 1)->y;
		Vertices[1].color = (colors + index + 1)->FormatRGBA();
		
		mTriangleCount += 1;
		
		UI_IGNORE_RETURN (mRenderDevice->DrawPrimitive( D3DPT_LINELIST, D3DFVF_TLVERTEX, &Vertices, 2, 0 ));
	}
}

//----------------------------------------------------------------------

void UIDirect3DPrimaryCanvas::RenderTriangles      (const UICanvas * const src, int numTris, const UITriangle * tris, const UITriangle * uvs)
{
	const DWORD dwVertexColor = mState.Color.FormatRGBA();

	if( src )
	{
		assert( src->IsA( TUIDirect3DTextureCanvas ) );

		const UIDirect3DTextureCanvas * const SourceCanvas = static_cast<const UIDirect3DTextureCanvas *>(src);
		mCurrentTexture = SourceCanvas->GetSurface();
		mRenderDevice->SetTexture( 0, mCurrentTexture );
	}
	else
		mRenderDevice->SetTexture( 0, 0 );

	for (int i = 0; i < numTris; ++i)
	{
		const UITriangle & theTri = tris [i];
		const UITriangle & theUvs = uvs  [i];
			
		static D3DTLVERTEX Vertices[3] =
		{
			{ 0, 0, 0, 1, 0, 0, 0, 0 },
			{ 0, 0, 0, 1, 0, 0, 0, 0 },
			{ 0, 0, 0, 1, 0, 0, 0, 0 }
		};
		
		UIFloatPoint p;
		UIFloatPoint uv;
		
		p  = theTri.p1;
		uv = theUvs.p1;
		Vertices[0].sx		= p.x;
		Vertices[0].sy		= p.y;
		Vertices[0].tu		= uv.x;
		Vertices[0].tv		= uv.y;
		Vertices[0].color   = dwVertexColor;

		p = theTri.p2;
		uv = theUvs.p2;
		Vertices[1].sx		= p.x;
		Vertices[1].sy		= p.y;
		Vertices[1].tu		= uv.x;
		Vertices[1].tv		= uv.y;
		Vertices[1].color   = dwVertexColor;

		p = theTri.p3;
		uv = theUvs.p3;
		Vertices[2].sx		= p.x;
		Vertices[2].sy		= p.y;
		Vertices[2].tu		= uv.x;
		Vertices[2].tv		= uv.y;
		Vertices[2].color   = dwVertexColor;
				
		++mTriangleCount;
		
		if (mShowTriangles)
		{		
			mQuads.push_back( 
				UIQuad(
				static_cast<long>(Vertices [0].sx), static_cast<long>(Vertices[0].sy),
				static_cast<long>(Vertices [1].sx), static_cast<long>(Vertices[1].sy),
				static_cast<long>(Vertices [2].sx), static_cast<long>(Vertices[2].sy),
				static_cast<long>(Vertices [0].sx), static_cast<long>(Vertices[0].sy)));
		}
		
		UI_IGNORE_RETURN (mRenderDevice->DrawPrimitive( D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, &Vertices, 3, 0 ));
	}
}

//----------------------------------------------------------------------

bool UIDirect3DPrimaryCanvas::BeginRendering( void )
{
	UICanvas::BeginRendering();

	HRESULT hr;

	mCurrentTexture = 0;

	if( !Prepare() || !mRenderDevice )
		return false;
	
	hr = mPrimarySurface->SetClipper( mClipper );
	
	if( FAILED( hr ) )
		return false;

	hr = mRenderDevice->BeginScene();

	if( FAILED(hr) )
		return false;

	mTriangleCount = 0;

	return true;
}

//----------------------------------------------------------------------

void UIDirect3DPrimaryCanvas::EndRendering( void )
{
	UICanvas::EndRendering();
	
	assert( mRenderDevice );
	
	mRenderDevice->SetTexture( 0, 0 );
	DWORD				dwOldState;
	mRenderDevice->GetRenderState( D3DRENDERSTATE_FILLMODE, &dwOldState );

	if( mShowTriangles && !mQuads.empty ())
	{
		D3DTLVERTEX Vertices[4];
		
		updateRenderState( mRenderDevice, D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME );
		
		for( int i = 0; i < 4; ++i )
		{
			Vertices[i].sz		= 0.0;
			Vertices[i].rhw		= 1.0;
			Vertices[i].color = 0xFFFFFFFF;
		}
		
		for( UIQuadList::iterator CurrentQuad = mQuads.begin(); CurrentQuad != mQuads.end(); ++CurrentQuad )
		{
			const UIQuad & q = *CurrentQuad;
			
			// Top Left
			Vertices[0].sx		= static_cast<float>(q.p1.x);
			Vertices[0].sy		= static_cast<float>(q.p1.y);
			
			// Top Right
			Vertices[1].sx		= static_cast<float>(q.p2.x);
			Vertices[1].sy		= static_cast<float>(q.p2.y);
			
			// Bottom Left	
			Vertices[2].sx		= static_cast<float>(q.p3.x);
			Vertices[2].sy		= static_cast<float>(q.p3.y);
			
			// Bottom Right
			Vertices[3].sx		= static_cast<float>(q.p4.x);
			Vertices[3].sy		= static_cast<float>(q.p4.y);
			
			mRenderDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, &Vertices, 4, 0 );
		}
		mQuads.clear();	
	}

	//----------------------------------------------------------------------

	if(!mShaderQuads.empty ())
	{
		D3DTLVERTEX Vertices[5];
		
		updateRenderState( mRenderDevice, D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME );
		
		for( int i = 0; i < 4; ++i )
		{
			Vertices[i].sz		= 0.0;
			Vertices[i].rhw		= 1.0;
			Vertices[i].color = 0xaaFFaa00;
		}
		
		for( UIQuadList::iterator CurrentQuad = mShaderQuads.begin(); CurrentQuad != mShaderQuads.end(); ++CurrentQuad )
		{
			const UIQuad & q = *CurrentQuad;
			
			// Top Left
			Vertices[0].sx		= static_cast<float>(q.p1.x);
			Vertices[0].sy		= static_cast<float>(q.p1.y);
			
			// Top Right
			Vertices[1].sx		= static_cast<float>(q.p2.x);
			Vertices[1].sy		= static_cast<float>(q.p2.y);
						
			// Bottom Right
			Vertices[2].sx		= static_cast<float>(q.p4.x);
			Vertices[2].sy		= static_cast<float>(q.p4.y);

			// Bottom Left	
			Vertices[3].sx		= static_cast<float>(q.p3.x);
			Vertices[3].sy		= static_cast<float>(q.p3.y);

			// Top Left	
			Vertices[4]		= Vertices [0];
			
			mRenderDevice->DrawPrimitive( D3DPT_LINESTRIP, D3DFVF_TLVERTEX, &Vertices, 5, 0 );
		}
		mShaderQuads.clear();	
	}

	updateRenderState( mRenderDevice, D3DRENDERSTATE_FILLMODE, dwOldState );

	mRenderDevice->EndScene();
}

//----------------------------------------------------------------------

void UIDirect3DPrimaryCanvas::Flip( void )
{
	HRESULT hr;

	assert( mPrimarySurface );
	assert( mIsFullscreen || mBackBufferSurface );

	if( mIsFullscreen )
		hr = mPrimarySurface->Flip( 0, DDFLIP_WAIT );
	else
	{
		POINT theOrigin;
		RECT  rcDest;
		RECT  rcSrc;

		theOrigin.x = 0;
		theOrigin.y = 0;
		
		ClientToScreen( mWindow, &theOrigin );

		rcDest.left   = theOrigin.x;
		rcDest.top    = theOrigin.y;
		rcDest.right  = theOrigin.x + GetWidth();
		rcDest.bottom = theOrigin.y + GetHeight();

		rcSrc.left   = 0;
		rcSrc.top    = 0;
		rcSrc.right  = GetWidth();
		rcSrc.bottom = GetHeight();

		hr = mPrimarySurface->Blt( &rcDest, mBackBufferSurface, &rcSrc, DDBLT_WAIT, 0 );
	}
}

void UIDirect3DPrimaryCanvas::GetPreferedPixelFormat( LPDDPIXELFORMAT lpddpf )
{
	*lpddpf = mPreferedTexturePixelFormat;
}

HRESULT UIDirect3DPrimaryCanvas::D3DEnumPixelFormats( LPDDPIXELFORMAT lppf )
{
	// We need surfaces with both RGB and alpha components
	if( !(lppf->dwFlags & DDPF_RGB) || !(lppf->dwFlags & DDPF_ALPHAPIXELS) )
		return D3DENUMRET_OK;

	// Alpha must be at least 8 bits
	if( lppf->dwAlphaBitDepth < 8 )
		return D3DENUMRET_OK;

	// RGB must be at least 16 bits
	if( lppf->dwRGBBitCount < 16 )
		return D3DENUMRET_OK;

	// If this is the first pixel format that meets these criteria use it for now
	if( mPreferedTexturePixelFormat.dwSize == 0 )
	{
		mPreferedTexturePixelFormat = *lppf;
		return D3DENUMRET_OK;
	}

	if( lppf->dwRGBBitCount >= mPreferedTexturePixelFormat.dwRGBBitCount  )
		mPreferedTexturePixelFormat = *lppf;
	
	return D3DENUMRET_OK;
}

HRESULT CALLBACK UIDirect3DPrimaryCanvas::D3DEnumPixelFormatsStaticCallback( LPDDPIXELFORMAT lppf, LPVOID arg )
{
	return reinterpret_cast<UIDirect3DPrimaryCanvas *>(arg)->D3DEnumPixelFormats( lppf );
}

HWND UIDirect3DPrimaryCanvas::GetWindow( void )
{
	return mWindow;
}

bool UIDirect3DPrimaryCanvas::MatchPixelFormat( LPDDPIXELFORMAT FormatToMatch, LPDDPIXELFORMAT BestMatch )
{
	HRESULT hr;

	mBestMatchFound = false;
	mFormatToMatch  = *FormatToMatch;

	hr = mRenderDevice->EnumTextureFormats( D3DEnumPixelFormatsForBestMatchStaticCallback, this );

	if( FAILED( hr ) || !mBestMatchFound )
		return false;

	*BestMatch = mBestMatchPixelFormat;
	return true;
}

HRESULT UIDirect3DPrimaryCanvas::D3DEnumPixelFormatsForBestMatch( LPDDPIXELFORMAT lppf )
{
	bool UseThisFormat = false;

	if( mFormatToMatch.dwFlags & DDPF_FOURCC )
	{
		if( ( lppf->dwFlags & DDPF_FOURCC ) && (mFormatToMatch.dwFourCC == lppf->dwFourCC) )
			UseThisFormat = true;
	}
	else
	{
		if( (mFormatToMatch.dwFlags & DDPF_ALPHAPIXELS) == (lppf->dwFlags & DDPF_ALPHAPIXELS) )
		{
			if( mFormatToMatch.dwRGBBitCount == lppf->dwRGBBitCount )
				UseThisFormat = true;
		}
	}

	if( UseThisFormat )
	{
		mBestMatchPixelFormat = *lppf;
		mBestMatchFound = true;
		return D3DENUMRET_CANCEL;
	}
	else
		return D3DENUMRET_OK;
}

HRESULT CALLBACK UIDirect3DPrimaryCanvas::D3DEnumPixelFormatsForBestMatchStaticCallback( LPDDPIXELFORMAT lppf, LPVOID arg )
{
	return reinterpret_cast<UIDirect3DPrimaryCanvas *>(arg)->D3DEnumPixelFormatsForBestMatch( lppf );
}

//----------------------------------------------------------------------

bool UIDirect3DPrimaryCanvas::GetShowShaders () const
{
	return mShowShaders;
}
