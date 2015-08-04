#ifndef __UIPROGRESSBAR_H__
#define __UIPROGRESSBAR_H__

#include "UIWidget.h"
#include "UIProgressbarStyle.h"

class UIProgressbar : public UIWidget
{
public:

	static const char					 *TypeName;

	class PropertyName
	{
	public:
		static const UILowerString Progress;
		static const UILowerString Style;
	};

															UIProgressbar();
	virtual										 ~UIProgressbar();

	virtual bool								IsA( const UITypeID ) const;
	virtual const char				 *GetTypeName( void ) const;
	virtual UIBaseObject			 *Clone( void ) const;

	virtual void                        GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void								GetPropertyNames( UIPropertyNameVector &, bool forCopy  ) const;
	virtual void                        GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool								SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool								GetProperty( const UILowerString & Name, UIString &Value ) const;

	virtual bool								ProcessMessage( const UIMessage & );
	virtual void								Render( UICanvas & ) const;

					void								SetStyle( UIProgressbarStyle * );
					UIProgressbarStyle *GetProgressbarStyle( void ) const { return mStyle; };
	virtual	UIStyle						 *GetStyle( void ) const;

					void								SetProgress( const float );
					float								GetProgress( void ) const;

private:

															UIProgressbar( UIProgressbar & );
	UIProgressbar							 &operator = ( UIProgressbar & );


	UIProgressbarStyle				 *mStyle;
	float												mProgress;
};

#endif // __UIPROGRESSBAR_H__