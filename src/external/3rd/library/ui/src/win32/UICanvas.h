#ifndef __UICANVAS_H__
#define __UICANVAS_H__

#include "UIBaseObject.h"
#include "UITypes.h"
#include "UIDeformer.h"
#include "UIWatcher.h"

#include <vector>

class UICanvasGenerator;
class UIDeformer;


struct UICanvasState
{
public:

	UIPoint Translation;
	float   TransformationMatrix[3][3];		// [Row][Column]

	UIRect	ClippingRect;
	UIColor	Color;
	float	Opacity;

	bool    TranslationOnly : 1;
	bool    EnableFiltering : 1;
	bool    DepthOverride : 1;

	UIFloatPoint mShear;

	typedef std::vector<UIDeformer*> DeformerVector;
	DeformerVector mDeformers;

	float mDepth;
};

typedef ui_stdvector<UICanvasState>::fwd                     UICanvasStateVector;
typedef ui_stdstack<UICanvasState, UICanvasStateVector>::fwd UICanvasStateStack;

class UICanvas : public UIBaseObject
{
public:
	
	static const char            *TypeName; //lint !e1516 // data m ember hides inherited member
	
	explicit                     UICanvas             ( const UISize & );
	virtual                     ~UICanvas             ();
	
	virtual bool                 IsA                  ( const UITypeID Type ) const;
	virtual const char          *GetTypeName          () const;
	
	virtual bool                 Generate             () const;
			void                 SetGenerator         ( UICanvasGenerator * );
			void                 GetGenerator         ( UICanvasGenerator *& ) const;
	
	virtual bool                 BeginRendering       ();
	virtual void                 EndRendering         ();
	
	virtual void                 SetSize              ( const UISize & );
	virtual void                 GetSize              ( UISize & ) const;
	virtual long                 GetWidth             () const;
	virtual long                 GetHeight            () const;
	
	virtual void                 BltFrom              ( const UICanvas * const src, const UIPoint &Source, const UIPoint &Destination, const UISize &Size );
	virtual void                 BltFromNoScaleOrRotate( const UICanvas * const src, const UIPoint &Source, const UIPoint &Destination, const UISize &Size );
	virtual void                 ClearTo              ( const UIColor &Color, const UIRect &);

	        void                 RenderLine           (const UILine & line);
	virtual void                 RenderLines          (const UICanvas * const src, int numLines,   const UILine * lines, const UILine * uvs);
	virtual void                 RenderLines          (const UICanvas * const , int , const UILine * , const UIFloatPoint * , const UIColor *);
	
	virtual void                 RenderLineStrip      (const UICanvas * const str, int PointCount, const UIFloatPoint *Points);

	virtual void                 RenderTriangles      (const UICanvas * const src, int TriangleCount, const UITriangle * tris, const UITriangle * uvs);
	
	virtual void                 Flush                ();
	virtual void                 EnableFiltering      ( bool );
	virtual void                 Refresh              ();
	
	        void                 PushState            ();
	        void                 RestoreState         ();           // Restores the last pushed state, leaves pushed state on the stack
	        void                 RestoreClip          ();            // Restores only the last pushed clipping rectangle, leaves pushed state on the stack
	        void                 RestoreTransform     ();       // Restores only the last pushed transformation, leaves pushed state on the stack
	        void                 RestoreOpacity       ();         // Restores only the last pushed opacity value, leaves pushed state on the stack
	        void                 PopState             ();               // Restores the last pushed state, removes the pushed state from the stack
	
	        void                 Translate            ( const UIPoint & );
	        void                 Translate            ( UIScalar x, UIScalar y );
	  const UIPoint &            GetTranslation       () const;
	
	        void                 Rotate               ( float Rotation );   // Rotation is in radians
	        void                 Scale                ( float HorizontalScale, float VerticalScale );
	
	// returns false if argument is completely clipped
	        bool                 Clip                 ( const UIRect & );
	        bool                 Clip                 ( UIScalar left, UIScalar top, UIScalar right, UIScalar bottom );
	        bool                 ClipLeft             ( UIScalar Left );
	        bool                 ClipTop              ( UIScalar Top );
	        bool                 ClipBottom           ( UIScalar Bottom );
	        bool                 ClipRight            ( UIScalar Right );
	
	        void                 GetClip              ( UIRect & ) const;
	        void                 SetClip              ( const UIRect & );

	        void                 SetColor             ( const UIColor & );  // Does not change opacity
	const UIColor &              GetColor             () const;
	        void                 ModifyColor          (const UIColor &);

	        void                 SetOpacity           ( float );    // Force opacity to argument
	        void                 ModifyOpacity        ( float ); // Multiply opacity by argument
	        float                GetOpacity           () const;
	        bool                 IsTransparent        () const;
	        bool                 IsNotTransparent     () const;

	        long                 GetFlushCount        () const;

	        void                 SetSourceScale       (float x, float y);

	//-- shearing currently only work in limited circumstances.
	//-- shearing of children in sheared parents is broken
	//-- clipping is broken
	        void                 SetShear             (const UIFloatPoint & shear);
	        void                 Shear                (const UIFloatPoint & shear);
	        const UIFloatPoint  & GetShear            () const;
	        UIPoint              Transform            ( const UIPoint &In ) const;
			UIFloatPoint         Transform            ( const UIFloatPoint &In ) const;
	        UIPoint              Transform            ( UIScalar x, UIScalar y ) const;
	        UIFloatPoint         TransformFP          ( const float x, const float y ) const;
	        void                 Transform            ( float &x, float &y ) const;
			void                 Transform            ( UILine & dst, const UILine & src ) const;
	static void                  PerformShearing      (float & x, float & y, const UIFloatPoint & shear);
	static void                  PerformShearing      (long & x,  long & y, const UIFloatPoint & shear);

	const UICanvasState &        GetCurrentState      () const;
	void                         SetCurrentState      (const UICanvasState & state);

	virtual void                 RenderQuad           ( const UICanvas * const src, const UIFloatPoint Vertices[4], const UIFloatPoint UVs[4], const UIColor Colors[4] );
	virtual void                 RenderQuad           ( const UICanvas * const src, const UIFloatPoint Vertices[4], const UIFloatPoint UVs[4] );

	void QueueDeformer(UIDeformer & deformer);
	void DequeueDeformer(UIDeformer const & deformer);
	UICanvasState::DeformerVector const & GetDeformers() const;
	void Deform(UIFloatPoint const * in, UIFloatPoint * out, size_t count);
	void GetDeformScale(UIFloatPoint & DeformScale);
	bool IsDeforming() const;

	bool IsFiltering() const;
	
	virtual void Reload(UINarrowString const & newTextureName);

private:
	                             UICanvas             ();
	UICanvas & operator=  (const UICanvas &);
	           UICanvas   (const UICanvas &);


protected:

	UISize                       mSize;
	UIPoint                      mOrigin;

	UIFloatPoint                 mSrcScale;

	UICanvasGenerator           *mGenerator;

	UICanvasState                mState;
	UICanvasStateStack *         mStateStack;

	long                         mFlushCount;
};


//----------------------------------------------------------------------

inline const UICanvasState & UICanvas::GetCurrentState      () const
{
	return mState;
}

//----------------------------------------------------------------------

inline void UICanvas::SetCurrentState      (const UICanvasState & state)
{
	mState = state;
}

//-----------------------------------------------------------------

inline void UICanvas::SetSourceScale (float x, float y)
{
	mSrcScale.x = x;
	mSrcScale.y = y;
}

//-----------------------------------------------------------------

extern bool InitializeCanvasSystem( void *arg );
extern void ShutdownCanvasSystem( void *arg );

//----------------------------------------------------------------------

inline void	UICanvas::ClearTo( const UIColor &Color, const UIRect & ClearRect )
{
	UIColor OldColor = mState.Color;
	
	mState.Color = Color;
	BltFrom( 0, UIPoint(0,0), UIPoint(ClearRect.left, ClearRect.top), UISize( ClearRect.Width(), ClearRect.Height() ) );
	mState.Color = OldColor;
	
}

//----------------------------------------------------------------------

// Canvas state operations inlined
inline void UICanvas::EnableFiltering( bool NewValue )
{
	mState.EnableFiltering = NewValue;
}

//----------------------------------------------------------------------

inline void UICanvas::Translate( const UIPoint &Translation )
{
	Translate (Translation.x, Translation.y);
}

//----------------------------------------------------------------------

inline void UICanvas::Translate( UIScalar x, UIScalar y )
{
	PerformShearing (x, y, mState.mShear);

	mState.TransformationMatrix[0][2] = mState.TransformationMatrix[0][0] * x + mState.TransformationMatrix[0][1] * y + mState.TransformationMatrix[0][2];
	mState.TransformationMatrix[1][2] = mState.TransformationMatrix[1][0] * x + mState.TransformationMatrix[1][1] * y + mState.TransformationMatrix[1][2];
	mState.TransformationMatrix[2][2] = mState.TransformationMatrix[2][0] * x + mState.TransformationMatrix[2][1] * y + mState.TransformationMatrix[2][2];

	mState.Translation.x += x;
	mState.Translation.y += y;
}

//----------------------------------------------------------------------

inline const UIPoint & UICanvas::GetTranslation () const
{
	return mState.Translation;
}

//----------------------------------------------------------------------

inline void UICanvas::Scale( float HorizontalScale, float VerticalScale )
{
	mState.TransformationMatrix[0][0] *= HorizontalScale;
	mState.TransformationMatrix[0][1] *= HorizontalScale;	
	mState.TransformationMatrix[1][0] *= VerticalScale;
	mState.TransformationMatrix[1][1] *= VerticalScale;

	mState.TranslationOnly = false;
}

//----------------------------------------------------------------------

inline UIPoint UICanvas::Transform( const UIPoint &In ) const
{
	return Transform (In.x, In.y);
}

//----------------------------------------------------------------------

inline UIFloatPoint UICanvas::Transform( const UIFloatPoint &In ) const
{
	UIFloatPoint tmp (In);
	Transform (tmp.x, tmp.y);
	return tmp;
}

//----------------------------------------------------------------------

inline UIFloatPoint	UICanvas::TransformFP( const float x, const float y ) const
{
	UIFloatPoint tmp(x,y);
	Transform (tmp.x, tmp.y);
	return tmp;
}

//----------------------------------------------------------------------

inline UIPoint UICanvas::Transform( UIScalar x, UIScalar y ) const
{
	float fx = static_cast<float>(x);
	float fy = static_cast<float>(y);

	Transform (fx, fy);

	return UIPoint (static_cast<long>(fx), static_cast<long>(fx));
}

//----------------------------------------------------------------------

inline void UICanvas::Transform( UILine & dst, const UILine & src ) const
{
	dst.p1 = Transform(src.p1);
	dst.p2 = Transform(src.p2);
}

//-----------------------------------------------------------------

inline void UICanvas::PerformShearing (float & x, float & y, const UIFloatPoint & shear)
{
	const UIFloatPoint tmp (x + shear.x * y, y + shear.y * x);
	x = tmp.x;
	y = tmp.y;
}

//-----------------------------------------------------------------

inline void UICanvas::PerformShearing (long & x, long & y, const UIFloatPoint & shear)
{
	const UIPoint tmp (x + static_cast<long>(shear.x * static_cast<float>(y)), y + static_cast<long>(shear.y * static_cast<float>(x)));
	x = tmp.x;
	y = tmp.y;
}

//----------------------------------------------------------------------

inline void UICanvas::Transform( float &x, float &y ) const
{
	UIFloatPoint sheared (x,y);
	PerformShearing (sheared.x, sheared.y, mState.mShear);

	const float newx = static_cast<float>(sheared.x * mState.TransformationMatrix[0][0] + sheared.y * mState.TransformationMatrix[0][1] + mState.TransformationMatrix[0][2]);
	const float newy = static_cast<float>(sheared.x * mState.TransformationMatrix[1][0] + sheared.y * mState.TransformationMatrix[1][1] + mState.TransformationMatrix[1][2]);

	x = newx;
	y = newy;
}

//----------------------------------------------------------------------

inline bool UICanvas::Clip( const UIRect &In )
{	
	return Clip (In.left, In.top, In.right, In.bottom);
}

//----------------------------------------------------------------------

inline bool UICanvas::ClipLeft( UIScalar Left )
{
	Left += mState.Translation.x;

	if( mState.ClippingRect.left < Left )
	{
		if( mState.ClippingRect.right < Left )
		{
			mState.ClippingRect.left = mState.ClippingRect.right;
			return false;
		}
			
		mState.ClippingRect.left = Left;
	}

	return true;
}

//----------------------------------------------------------------------

inline bool UICanvas::ClipTop( UIScalar TheTop )
{
	TheTop += mState.Translation.y;

	if( mState.ClippingRect.top < TheTop )
	{
		if( mState.ClippingRect.bottom < TheTop )
		{
			mState.ClippingRect.top = mState.ClippingRect.bottom;
			return false;
		}
			
		mState.ClippingRect.top = TheTop;
	}

	return true;
}

//----------------------------------------------------------------------

inline bool UICanvas::ClipBottom( UIScalar TheBottom )
{
	TheBottom += mState.Translation.y;

	if( mState.ClippingRect.bottom > TheBottom )
	{
		if( mState.ClippingRect.top > TheBottom )
		{
			mState.ClippingRect.bottom = mState.ClippingRect.top;
			return false;
		}
			
		mState.ClippingRect.bottom = TheBottom;
	}

	return true;
}

//----------------------------------------------------------------------

inline bool UICanvas::ClipRight( UIScalar Right )
{
	Right += mState.Translation.x;

	if( mState.ClippingRect.right > Right )
	{
		if( mState.ClippingRect.left > Right )
		{
			mState.ClippingRect.right = mState.ClippingRect.left;
			return false;
		}
			
		mState.ClippingRect.right = Right;
	}

	return true;
}

//----------------------------------------------------------------------

inline void UICanvas::SetClip( const UIRect & In)
{
	mState.ClippingRect.top    = In.top     + mState.Translation.y;
	mState.ClippingRect.left   = In.left    + mState.Translation.x;
	mState.ClippingRect.bottom = In.bottom  + mState.Translation.y;
	mState.ClippingRect.right  = In.right   + mState.Translation.x; 
}

//----------------------------------------------------------------------

inline void UICanvas::GetClip( UIRect &Out ) const
{
	Out.top    = mState.ClippingRect.top    - mState.Translation.y;
	Out.left   = mState.ClippingRect.left   - mState.Translation.x;
	Out.bottom = mState.ClippingRect.bottom - mState.Translation.y;
	Out.right  = mState.ClippingRect.right  - mState.Translation.x;
}

//----------------------------------------------------------------------

inline void UICanvas::SetColor( const UIColor &NewColor )
{
	mState.Color.r = NewColor.r;
	mState.Color.g = NewColor.g;
	mState.Color.b = NewColor.b;
}

//----------------------------------------------------------------------

inline void UICanvas::SetOpacity( float NewOpacity )
{
	mState.Opacity = NewOpacity;

	if( mState.Opacity >= 1.0f )
		mState.Color.a = 0xFF;
	else if( mState.Opacity <= 0.0f )
		mState.Color.a = 0x00;
	else
		mState.Color.a = static_cast<unsigned char>(mState.Opacity * 255.0f);
}

//----------------------------------------------------------------------

inline void UICanvas::ModifyOpacity( float OpacityModifier )
{
	mState.Opacity *= OpacityModifier;

	if( mState.Opacity >= 1.0f )
		mState.Color.a = 0xFF;
	else if( mState.Opacity <= 0.0f )
		mState.Color.a = 0x00;
	else
		mState.Color.a = static_cast<unsigned char>(mState.Opacity * 255.0f);
}

//----------------------------------------------------------------------

inline void UICanvas::ModifyColor (const UIColor & color)
{
	static const float oo_255 = 1.0f / 255.0f;

	mState.Color.r = static_cast<unsigned char> (static_cast<float>(mState.Color.r) * static_cast<float>(color.r) * oo_255);
	mState.Color.g = static_cast<unsigned char> (static_cast<float>(mState.Color.g) * static_cast<float>(color.g) * oo_255);
	mState.Color.b = static_cast<unsigned char> (static_cast<float>(mState.Color.b) * static_cast<float>(color.b) * oo_255);
}

//----------------------------------------------------------------------

inline const UIColor & UICanvas::GetColor () const
{
	return mState.Color;
}

//----------------------------------------------------------------------

inline float UICanvas::GetOpacity () const
{
	return mState.Opacity;
}

//----------------------------------------------------------------------

inline bool UICanvas::IsTransparent () const
{
	return mState.Color.a == 0;
}

//----------------------------------------------------------------------

inline bool UICanvas::IsNotTransparent () const
{
	return mState.Color.a != 0;
};

//----------------------------------------------------------------------

inline long UICanvas::GetFlushCount () const
{
	return mFlushCount;
}

//----------------------------------------------------------------------

inline void UICanvas::SetShear (const UIFloatPoint & shear)
{
	mState.mShear = shear;
}

//----------------------------------------------------------------------

inline void UICanvas::Shear (const UIFloatPoint & shear)
{
	mState.mShear += shear;
}

//----------------------------------------------------------------------

inline const UIFloatPoint  & UICanvas::GetShear () const
{
	return mState.mShear;
}

//----------------------------------------------------------------------

inline UICanvasState::DeformerVector const & UICanvas::GetDeformers() const
{
	return mState.mDeformers;
}

//----------------------------------------------------------------------

inline bool UICanvas::IsFiltering() const
{
	return mState.EnableFiltering;
}


//----------------------------------------------------------------------

#endif // __UICANVAS_H__
