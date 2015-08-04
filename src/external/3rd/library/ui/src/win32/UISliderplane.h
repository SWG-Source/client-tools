#ifndef __UISLIDERPLANE_H__
#define __UISLIDERPLANE_H__

#include "UIEventCallback.h"
#include "UIWidget.h"

class UISliderplaneStyle;
class UIButton;

class UISliderplane :
public UIWidget,
public UIEventCallback
{
public:

	static const char          *TypeName;

	class PropertyName
	{
	public:
		static const UILowerString LowerLimit;
		static const UILowerString OnChange;
		static const UILowerString Style;
		static const UILowerString UpperLimit;
		static const UILowerString Value;
		static const UILowerString ValueUpdateContinuous;
	};

	UISliderplane();
	virtual                        ~UISliderplane();

	virtual bool                    IsA( const UITypeID ) const;
	virtual const char             *GetTypeName( void ) const;
	virtual UIBaseObject           *Clone( void ) const;

	virtual void                    GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void                    GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual void                    GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool                    SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool                    GetProperty( const UILowerString & Name, UIString &Value ) const;

	virtual bool                    OnMessage( UIWidget *Context, const UIMessage & msg );
	virtual bool                    ProcessMessage( const UIMessage & );

	virtual void                    Render( UICanvas & ) const;

	void                            SetStyle( UISliderplaneStyle * );
	        UISliderplaneStyle     *GetSliderplaneStyle( void ) const;
	virtual UIStyle                *GetStyle( void ) const;

	void                            SetValue( UIPoint, bool notify );
	const UIPoint &                 GetValue( void ) const;

	const UIPoint &                 GetUpperLimit( void ) const;
	void                            SetUpperLimit( const UIPoint &);

	const UIPoint &                 GetLowerLimit( void ) const;
	void                            SetLowerLimit( const UIPoint & );

	//-----------------------------------------------------------------

	virtual bool                    AddChild( UIBaseObject * );
	virtual bool                    RemoveChild( UIBaseObject * );
	virtual void                    SelectChild( UIBaseObject * );
	virtual UIBaseObject           *GetChild( const char * ObjectName ) const;
	virtual void                    GetChildren( UIObjectList & ) const;
	virtual unsigned long           GetChildCount( void ) const;

	virtual void                    Link( void );

	virtual UIWidget               *GetWidgetFromPoint( const UIPoint &, bool mustGetInput ) const;

	//-----------------------------------------------------------------

private:

	                                UISliderplane( UISliderplane & );
	UISliderplane                  &operator = ( UISliderplane & );

	const UIPoint                   GetSliderPosition( void ) const;

	const UIPoint                   CalculateSliderLocationFromValue () const;
	const UIPoint                   CalculateValueFromSliderLocation () const;
	void                            updateValueFromPoint (const UIPoint & point, bool notify);

	UISliderplaneStyle             *mStyle;

	UIPoint                         mLowerLimit;
	UIPoint                         mUpperLimit;
	UIPoint                         mValue;

	bool                            mDraggingSlider;
	long                            mDragMouseOrigin;
	UIPoint                         mDragValueOrigin;

	UIButton *                      mSliderButton;

	bool                            mValueUpdateContinuous;

};

//-----------------------------------------------------------------
inline const UIPoint & UISliderplane::GetValue( void ) const
{
	return mValue;
};
//-----------------------------------------------------------------
inline const UIPoint & UISliderplane::GetUpperLimit( void ) const
{
	return mUpperLimit;
};
//-----------------------------------------------------------------
inline const UIPoint & UISliderplane::GetLowerLimit( void ) const
{
	return mLowerLimit;
};
//-----------------------------------------------------------------
#endif // __UISLIDERPLANE_H__