#ifndef __UIELLIPSE_H__
#define __UIELLIPSE_H__

#ifndef __UIIMAGE_H__
#include "UIImage.h"
#endif // __UIIMAGE_H__

#include <vector>

class UIEllipse : public UIImage
{
public:

	static const char * TypeName;

	class PropertyName
	{
	public:
		static UILowerString ArcBegin;
		static UILowerString ArcEnd;
		static UILowerString Thickness;
		static UILowerString OuterThickness;
		static UILowerString InnerRadiusColor;
		static UILowerString InnerRadiusOpacity;        
		static UILowerString CircleSegments;
		static UILowerString LineSegments;
	};

                            UIEllipse();
	virtual                ~UIEllipse();
                                            
	virtual bool            IsA( const UITypeID ) const;
	virtual const char     *GetTypeName() const;
	virtual UIBaseObject   *Clone() const;

	virtual void            GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void            GetPropertyNames(UIPropertyNameVector &, bool forCopy) const;

	virtual bool            SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool            GetProperty( const UILowerString & Name, UIString &Value ) const;

	virtual bool            CanSelect() const;
	virtual bool            WantsMessage( const UIMessage & ) const;

	virtual bool            ProcessMessage( const UIMessage & );
	virtual void            Render( UICanvas & ) const;

			float           GetArcBegin() const;                   // Units are 0 - 1, 12 o'clock = 0
			void            SetArcBegin( const float );

			float           GetArcEnd() const;                     // Units are 0 - 1, 12 o'clock = 0
			void            SetArcEnd( const float );

	float GetArcLength() const;

			float           GetThickness() const;
			void            SetThickness( const float );

	float GetOuterThickness() const;
	void SetOuterThickness(const float);

			const UIColor   &GetInnerRadiusColor() const;
			void            SetInnerRadiusColor( const UIColor & );

			float           GetInnerRadiusOpacity() const;
			void            SetInnerRadiusOpacity( const float );

			void            SetAllColors( const UIColor &NewColor );
			void            SetAllOpacity( float Opacity );

			void            SetPercent( float Percent ); // This is purely dynamic.
			float           GetPercent() const;

			void            SetLineSegments(int mode);
			int             GetLineSegments() const;
			

	virtual void OnSizeChanged(UISize const & newSize, UISize const & oldSize);
	virtual void OnLocationChanged(UIPoint const & newLocation, UIPoint const & oldLocation);

private:
							UIEllipse( UIEllipse & );
	UIEllipse              &operator = ( UIEllipse & );

private:

	typedef std::vector< UIFloatPoint > UIFloatPointVector;
	typedef std::vector< UILine > UILineVector;

private:
	float                   mArcBegin;                  // Units are 0 - 1, 12 o'clock = 0
	float                   mArcEnd;                        // Units are 0 - 1, 12 o'clock = 0
	float                   mThickness;             // <= 0.0 means use a line
	float mOuterThickness;
	bool                    mHasInnerRadiusColor;
	UIColor                 mInnerRadiusColor;  
	float                   mInnerRadiusOpacity;
	float                   mPercent;
	int                     mLineSegments;
	int                     mCircleSegments;
  
	mutable bool            mPointsDirty;
	mutable UIFloatPointVector mPoints;    
	mutable UILineVector mLines;
};

//=============================================================================
inline float UIEllipse::GetArcBegin() const
{
	return mArcBegin;
}

//=============================================================================
inline void UIEllipse::SetArcBegin( const float ArcBegin )
{
	mArcBegin = ArcBegin;
	mPointsDirty = true;
}

//=============================================================================
inline float UIEllipse::GetArcEnd() const
{
	return mArcEnd;
}

//=============================================================================
inline void UIEllipse::SetArcEnd( const float ArcEnd )
{
	mArcEnd = ArcEnd;
	mPointsDirty = true;
}

//=============================================================================
inline float UIEllipse::GetThickness() const
{
	return mThickness;
}

//=============================================================================
inline void UIEllipse::SetThickness( const float Thickness )
{
	mThickness = Thickness;
	mPointsDirty = true;
}

//=============================================================================
inline const UIColor &UIEllipse::GetInnerRadiusColor() const
{
	if( mHasInnerRadiusColor )
	{
		return mInnerRadiusColor;
	}

	return mInnerRadiusColor;
}

//=============================================================================
inline void UIEllipse::SetInnerRadiusColor( const UIColor &InnerRadiusColor )
{
	mInnerRadiusColor = InnerRadiusColor;
	mHasInnerRadiusColor = true;
}

//=============================================================================
inline float UIEllipse::GetInnerRadiusOpacity() const
{
	return mInnerRadiusOpacity;
}

//=============================================================================
inline void UIEllipse::SetInnerRadiusOpacity( const float InnerRadiusOpacity )
{
	mInnerRadiusOpacity = InnerRadiusOpacity;
}

//=============================================================================

inline float UIEllipse::GetOuterThickness() const
{
	return mOuterThickness;
}

//=============================================================================

inline void UIEllipse::SetOuterThickness(float const thickness)
{
	mOuterThickness = thickness;
}


#endif // __UIELLIPSE_H__