#ifndef __UITEXTSTYLEMANAGER_H__
#define __UITEXTSTYLEMANAGER_H__

#include "UiMemoryBlockManagerMacros.h"
#include "UIWidget.h"
#include "UIStandardLoader.h"

class UiMemoryBlockManager;
class UIPage;
class UITextStyle;
class UILoader;

//----------------------------------------------------------------------


class UITextStyleManager : public UIBaseObject
{

public:

	class PropertyName 
	{ //lint !e578 // hides
	public:
		static const UILowerString FontFaces;
		static const UILowerString FontLocale;
	};

	class TextStyleManagerLoader : public UIStandardLoader<UITextStyleManager>
	{
		public:
			UIBaseObject *          Create      () const { return UITextStyleManager::GetInstance(); };
	};


	static const char     * const TypeName;

	                       UITextStyleManager           ();
	virtual               ~UITextStyleManager           ();

	virtual bool           IsA                (const UITypeID ) const;
	virtual const char    *GetTypeName        () const;
	virtual UIBaseObject  *Clone              () const;

	virtual void           GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void           GetPropertyNames   (UIPropertyNameVector &, bool forCopy ) const;


	virtual bool           SetProperty        (const UILowerString & Name, const UIString &Value );
	virtual bool           GetProperty        (const UILowerString & Name, UIString &Value ) const;

	static UITextStyleManager *       GetInstance             ();

	UITextStyle *          GetFontForLogicalFont (const Unicode::String &logicalFontName);
	UITextStyle *          GetFontForLogicalFont (const std::string &logicalFontName);
	UITextStyle *          GetFontForLogicalFont (const UILowerString &logicalFontName);
	//Unicode::String const &GetLogicalFontForFont (UITextStyle *font);

	static void            install            (const char * fontLocale);

	void                   Initialize         (UIPage *rootPage, UILoader &loader);

private:

	typedef ui_stdmap<UILowerString, UILowerString>::fwd LogicalFontFaceToFontFaceMap;
	typedef ui_stdvector<UILowerString>::fwd             LogicalFontFaces;	

	                       UITextStyleManager               (const UITextStyleManager &);
	UITextStyleManager &             operator =             (const UITextStyleManager &);


	static Unicode::String           s_fontLocale;	
	static UITextStyleManager *      s_theManager;

	LogicalFontFaceToFontFaceMap *   m_logicalToFixedFaceMap;
	LogicalFontFaces             *   m_logicalFontFaces;


	bool                   m_initialized;
};

//======================================================================================

#endif // __UITEXTSTYLEMANAGER_H__
