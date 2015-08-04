#include "_precompile.h"
#include "UITextStyleManager.h"

#include "UICanvas.h"
#include "UILoader.h"
#include "UILowerString.h"
#include "UIManager.h"
#include "UINamespace.h"
#include "UIPage.h"
#include "UIPropertyDescriptor.h"
#include "UITextStyle.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"
#include <cassert>
#include <map>
#include <list>
#include <vector>

const char * const UITextStyleManager::TypeName = "TextStyleManager";

//----------------------------------------------------------------------

const UILowerString UITextStyleManager::PropertyName::FontFaces         = UILowerString ("FontFaces");
const UILowerString UITextStyleManager::PropertyName::FontLocale        = UILowerString ("FontLocale");

//----------------------------------------------------------------------


UITextStyleManager * UITextStyleManager::s_theManager = 0;
Unicode::String UITextStyleManager::s_fontLocale = Unicode::emptyString;

#define MIN_POINT_SIZE 11
#define MAX_POINT_SIZE 120

//======================================================================================
#define _TYPENAME UITextStyleManager

namespace UITextStyleManagerNamespace
{
	int DefaultFontSize = 14;
	int SmallestJapaneseFontSize = 12;
}
using namespace UITextStyleManagerNamespace;
//======================================================================================

//----------------------------------------------------------------------

void UITextStyleManager::install(const char * fontLocale)
{
	assert(fontLocale); //lint !e1776 // John sez assert is done busted
	s_fontLocale = Unicode::String(Unicode::narrowToWide(fontLocale));
}

//----------------------------------------------------------------------

UITextStyleManager::UITextStyleManager () :
UIBaseObject         (),
m_logicalToFixedFaceMap(new LogicalFontFaceToFontFaceMap),
m_logicalFontFaces(new LogicalFontFaces),
m_initialized(false)
{
	if(s_theManager != 0)
	{
		assert(false); //lint !e1776 // John sez assert is done busted
	}
	s_theManager = this;		
}


//----------------------------------------------------------------------

UITextStyleManager::~UITextStyleManager ()
{
	delete m_logicalToFixedFaceMap;
	m_logicalToFixedFaceMap = 0;
	delete m_logicalFontFaces;
	m_logicalFontFaces = 0;
	if (this == s_theManager)
		s_theManager = NULL;
}

//----------------------------------------------------------------------

void UITextStyleManager::Initialize (UIPage* rootPage, UILoader &loader)
{
	//Dynamically load and include the fonts
	UIString fontLocale;
	if(s_fontLocale.empty())
	{
		if(!GetProperty(PropertyName::FontLocale, fontLocale))
			s_fontLocale = Unicode::narrowToWide("en");
	}
	else
		fontLocale = s_fontLocale;
	char buffer[512];
	size_t const buffer_size = sizeof(buffer);
	UI_IGNORE_RETURN(_snprintf(buffer, buffer_size, "Fonts_%s", Unicode::wideToNarrow(fontLocale).c_str()));
	UILowerString const fontsProperty(buffer);
	UIString fontsValue;
	UI_IGNORE_RETURN(GetProperty(fontsProperty, fontsValue));
	size_t endpos = 0;
	Unicode::String lineToken;
	
	UIBaseObject *fontFolderObject = new UINamespace;
	fontFolderObject->SetName("Fonts");
	UI_IGNORE_RETURN(rootPage->AddChild(fontFolderObject));	
	
	std::string loaderString;
	UILoader::UIObjectList resultObjectList;
	
	while (endpos != Unicode::String::npos && Unicode::getFirstToken (fontsValue, endpos, endpos, lineToken, Unicode::endlines) ) //lint !e650 !e737 //npos wrong sign
	{
		std::string lineAsStdString = Unicode::wideToNarrow(lineToken);
		UI_IGNORE_RETURN(loaderString.append("<include>"));
		UI_IGNORE_RETURN(loaderString.append(lineAsStdString.c_str()));
		UI_IGNORE_RETURN(loaderString.append("</include>\n"));
		UI_IGNORE_RETURN(loader.LoadFromString(loaderString, resultObjectList));
		UI_IGNORE_RETURN(loaderString.erase());
	}
	
	for(UIObjectList::iterator resultObjectListIterator = resultObjectList.begin(); resultObjectListIterator != resultObjectList.end(); ++resultObjectListIterator)
	{
		UIBaseObject *newObject = *resultObjectListIterator;
		UI_IGNORE_RETURN(fontFolderObject->AddChild(newObject));	
	}
	
	//Comma-delimited list of allowable logical font faces
	UIString fontFacesString;
	UI_IGNORE_RETURN(GetProperty(PropertyName::FontFaces, fontFacesString));
	
	const Unicode::unicode_char_t comma []       = { ',', 0 };
	endpos = 0;
	while (endpos != Unicode::String::npos && Unicode::getFirstToken (fontFacesString, endpos, endpos, lineToken, comma) ) //lint !e650 !e737 //npos wrong sign
	{
		std::string lineAsStdString = Unicode::wideToNarrow(lineToken);
		UILowerString logicalFaceString(lineAsStdString);
		m_logicalFontFaces->push_back(logicalFaceString);
	}
	
	//Load the mappings for all these fonts
	LogicalFontFaces::iterator logicalIterator;
	for(logicalIterator = m_logicalFontFaces->begin(); logicalIterator != m_logicalFontFaces->end(); ++logicalIterator)
	{
		UILowerString const &logicalFontFace = *logicalIterator;		
		UI_IGNORE_RETURN(_snprintf(buffer, buffer_size, "FontFaces_%s_%s", Unicode::wideToNarrow(fontLocale).c_str(), logicalFontFace.c_str()));
		UILowerString const logicalFontProperty(buffer);
		UIString logicalFontValue;
		UI_IGNORE_RETURN(GetProperty(logicalFontProperty, logicalFontValue));
		UI_IGNORE_RETURN(m_logicalToFixedFaceMap->insert(std::make_pair(logicalFontFace, Unicode::wideToNarrow(logicalFontValue))));
	}
	
	m_initialized = true;
}

//----------------------------------------------------------------------

const char *UITextStyleManager::GetTypeName () const
{
	return TypeName;
}

//----------------------------------------------------------------------

bool UITextStyleManager::IsA (const UITypeID Type) const
{
	return (Type == TUITextStyleManager) || UIBaseObject::IsA( Type );
}

//----------------------------------------------------------------------

void UITextStyleManager::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIBaseObject::GetPropertyGroups(o_groups, category);
}

//----------------------------------------------------------------------

void UITextStyleManager::GetPropertyNames      (UIPropertyNameVector & in, bool forCopy) const
{
	UIBaseObject::GetPropertyNames (in, forCopy);
}

//----------------------------------------------------------------------

bool UITextStyleManager::SetProperty           (const UILowerString & Name, const UIString &Value )
{
	return  UIBaseObject::SetProperty (Name, Value);
}

//----------------------------------------------------------------------

UIBaseObject  * UITextStyleManager::Clone () const
{
	//Not really valid to clone a singleton
	return NULL;
}

//----------------------------------------------------------------------

bool UITextStyleManager::GetProperty           (const UILowerString & Name, UIString &Value ) const
{
	return UIBaseObject::GetProperty (Name, Value);
}

//----------------------------------------------------------------------

UITextStyleManager * UITextStyleManager::GetInstance           ()
{
	if(!s_theManager)
		s_theManager = new UITextStyleManager;
	return s_theManager;
}

//----------------------------------------------------------------------

UITextStyle * UITextStyleManager::GetFontForLogicalFont (const std::string &logicalFontName)
{
	return GetFontForLogicalFont(UILowerString(logicalFontName));
}

//----------------------------------------------------------------------

UITextStyle * UITextStyleManager::GetFontForLogicalFont (const Unicode::String &logicalFontName)
{
	return GetFontForLogicalFont(UILowerString(Unicode::wideToNarrow(logicalFontName)));
}

//----------------------------------------------------------------------

UITextStyle * UITextStyleManager::GetFontForLogicalFont (const UILowerString &logicalFontName)
{
	if(!m_initialized)
		return 0;
	
	// Used here because we're not guaranteed UIManager is instantiated in a globally static call
	const bool useJapanese = UIManager::gUIManager().isLocaleJapanese();
	const int fontSizeToMoveTowards = (useJapanese ? SmallestJapaneseFontSize : DefaultFontSize);

	//decompose the font name into the font face and the point size
	char buffer[512];
	size_t const buffer_size = sizeof(buffer);	
	std::string const & logicalFontNameAsString = logicalFontName.get();
	size_t oldPos = 0;
	size_t newPos = logicalFontNameAsString.find('_', oldPos);
	while(newPos != logicalFontNameAsString.npos) //lint !e650 !e737 //npos wrong sign
	{
		oldPos = newPos + 1;
		newPos = logicalFontNameAsString.find('_', oldPos);
	}
	std::string const & logicalFontFace = logicalFontNameAsString.substr(0, oldPos - 1);
	std::string const & logicalFontPoint = logicalFontNameAsString.substr(oldPos, logicalFontNameAsString.length());
	int const logicalPointSize = atoi(logicalFontPoint.c_str());
	
	
	if(logicalFontName.startsWith('/'))
	{
		//It's already decomposed into a fixed font
		int tryPointSize = logicalPointSize;
		int dir = 0;
		if (tryPointSize < fontSizeToMoveTowards)
			dir = 1;
		else
			dir = -1;
		while((tryPointSize >= MIN_POINT_SIZE) && (tryPointSize <= MAX_POINT_SIZE))
		{
			UI_IGNORE_RETURN(_snprintf(buffer, buffer_size, "%s_%d", logicalFontFace.c_str(), tryPointSize));
			UITextStyle *  textStyle = reinterpret_cast<UITextStyle *>(GetObjectFromPath(buffer, TUITextStyle)); //lint !e740 have to cast
			if(textStyle)
			{				
				textStyle->SetLogicalName(Unicode::narrowToWide(logicalFontName.c_str()));
				return textStyle;
			}
			tryPointSize += dir;
		}

	}
	
	//turn the font face into a fixed font face
	LogicalFontFaceToFontFaceMap::iterator logicalIterator = m_logicalToFixedFaceMap->find(UILowerString(logicalFontFace));
	if(logicalIterator == m_logicalToFixedFaceMap->end())
	{
		logicalIterator = m_logicalToFixedFaceMap->find(UILowerString("default"));
		if(logicalIterator == m_logicalToFixedFaceMap->end())
			logicalIterator = m_logicalToFixedFaceMap->begin();
		if(logicalIterator == m_logicalToFixedFaceMap->end())
		{
			//Well, now we're really screwed
			UITextStyle *  textStyle = static_cast<UITextStyle *>(GetObjectFromPath("/Fonts.aurabesh_12", TUITextStyle)); //lint !e740 have to cast
			if(textStyle)
			{
				UI_REPORT_LOG_PRINT(true, ("Returned bogus aurabesh_12 font because there was no default font"));
				textStyle->SetLogicalName(Unicode::narrowToWide(logicalFontName.c_str()));
				return textStyle;
			}
			assert(logicalIterator != m_logicalToFixedFaceMap->end());	//This error means there's no fonts at all
		}
	}
	
	UILowerString const & fixedFontFace = logicalIterator->second;
	
	//look for the widget start at the suggested point and head towards 14 (with 14 going down)
	//except Japanese, which head towards the smallest font available
	int tryPointSize = logicalPointSize;
	int dir = 0;
	if (tryPointSize < fontSizeToMoveTowards)
		dir = 1;
	else
		dir = -1;
	while((tryPointSize >= MIN_POINT_SIZE) && (tryPointSize <= MAX_POINT_SIZE))
	{
		//put the font name back together and try to get the UI object
		UI_IGNORE_RETURN(_snprintf(buffer, buffer_size, "/Fonts.%s_%d", fixedFontFace.c_str(), tryPointSize));
		UIString fontWidgetPath(Unicode::narrowToWide(buffer));
		UITextStyle *  textStyle = reinterpret_cast<UITextStyle *>(GetObjectFromPath(fontWidgetPath, TUITextStyle)); //lint !e740 have to cast
		if(textStyle)
		{
			textStyle->SetLogicalName(Unicode::narrowToWide(logicalFontName.c_str()));
			return textStyle;
		}
		tryPointSize += dir;
	}
	return 0;	
}

//----------------------------------------------------------------------

