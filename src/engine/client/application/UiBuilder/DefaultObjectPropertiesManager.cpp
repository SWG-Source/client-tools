#include "FirstUiBuilder.h"
#include "DefaultObjectPropertiesManager.h"

//#include "AddPropertyDialogBox.h"
#include "UnicodeUtils.h"
#include "UIWidget.h"
#include "resource.h"

#include <commctrl.h>
#include <cstdio>

#undef min

DefaultObjectPropertiesManager::DefaultObjectPropertiesManager()
{
	struct
	{
		UITypeID		TypeID;
		const char     *TypeName;
		const char     *SizeStr;
	}
	InitialDefaults[] =
	{	
		{ TUIButton,           "Button",           "64,32" },
		{ TUICheckbox,         "Checkbox",         "64,32" },
		{ TUIComboBox,         "ComboBox",         "300,16"},
		{ TUIComposite,        "Composite",        "480,320" },
		{ TUICursorSet,        "CursorSet",        0 },
		{ TUIData,             "Data",             0 },
		{ TUIDataSource,       "DataSource",       0 },
		{ TUIDataSourceContainer,"DataSourceContainer",0 },
		{ TUIDeformerHUD,      "DeformerHud",      0 },
		{ TUIDeformerRotate,   "DeformerRotate",   0 },
		{ TUIDeformerWave,     "DeformerWave",     0 },
		{ TUIDropdownbox,      "Dropdownbox",      "128,32" },
		{ TUIEllipse,          "Ellipse",          "128,128" },
		{ TUIFontCharacter,    "FontCharacter",    "16,16" },
		{ TUIGridStyle,        "GridStyle",        0 },
		{ TUIImage,            "Image",            "128,128" },
		{ TUIImageStyle,       "ImageStyle",       0 },
		{ TUIList,             "List",             "128,160" },
		{ TUIListStyle,        "ListStyle",        0 },
		{ TUIListbox,          "Listbox",          "128,160" },
		{ TUIListboxStyle,     "ListboxStyle",     0 },
		{ TUINamespace,        "Namespace",        0 },
		{ TUIPage,             "Page",             "320,240" },
		{ TUIPalette,          "Palette",          0 },
		{ TUIPie,              "Pie",              "128,128" },
		{ TUIPieStyle,         "PieStyle",         0 },
		{ TUIPopupMenu,        "PopupMenu",       "32,128" },
		{ TUIPopupMenuStyle,   "PopupMenuStyle",   0 },
		{ TUIProgressbar,      "Progressbar",      "128,32" },
		{ TUIProgressbarStyle, "ProgressbarStyle", 0 },
		{ TUIRadialMenu,       "RadialMenu",      "32,128" },
		{ TUIRunner,           "Runner",           "128,256" },
		{ TUIScrollbar,        "Scrollbar",        "32,128" },
		{ TUIScrollbarStyle,   "ScrollbarStyle",   0 },
		{ TUISliderbar,        "Sliderbar",        "128,32" },
		{ TUISliderbarStyle,   "SliderbarStyle",   0 },
		{ TUISliderbarStyle,   "SliderbarStyle",   0 },
		{ TUISliderplane,      "Sliderplane",      "128,128" },
		{ TUITabSet,           "TabSet",           "128,64" },
		{ TUITabSetStyle,      "TabSetStyle",      0 },
		{ TUITabbedPane,       "TabbedPane",       "256,32" },
		{ TUITabbedPaneStyle,  "TabbedPaneStyle",  0 },
		{ TUITable,            "Table",            "300,400" },
		{ TUITableHeader,      "TableHeader",      "300,32" },
		{ TUITableModelDefault,"TableModelDefault",0 },
		{ TUITemplate,         "Template",         0 },
		{ TUIText,             "Text",             "128,64" },
		{ TUITextbox,          "Textbox",          "128,32" },
		{ TUITextboxStyle,     "TextboxStyle",     0 },
		{ TUITextStyleManager, "TextStyleManager", "en" },
		{ TUITreeView,         "TreeView",         "300,400"},
		{ TUIVolumePage,       "VolumePage",       "128,128" },
	};

	for (int i = 0; i < sizeof(InitialDefaults) / sizeof(InitialDefaults[0]); ++i )
	{
		DefaultObjectProperties DefaultProperty;

		DefaultProperty.TypeID   = InitialDefaults[i].TypeID;
		DefaultProperty.TypeName = InitialDefaults[i].TypeName;

		UIString NameProperty (UI_UNICODE_T("New "));
		NameProperty += UIUnicode::narrowToWide (InitialDefaults[i].TypeName);
		DefaultProperty.Properties["Name"] = NameProperty;

		
		if (DefaultProperty.TypeID == TUITextStyleManager)
			DefaultProperty.Properties["FontLocale"] = Unicode::narrowToWide (InitialDefaults[i].SizeStr);
		else if (InitialDefaults [i].SizeStr)
			DefaultProperty.Properties["Size"] = Unicode::narrowToWide (InitialDefaults [i].SizeStr);

		m_defaultObjectProperties.push_back(DefaultProperty);
	}

	m_selectedPropertyMap = 0;
}

DefaultObjectPropertiesManager::~DefaultObjectPropertiesManager()
{
}

void DefaultObjectPropertiesManager::ApplyDefaultPropertiesToObject( UIBaseObject *TargetObject ) const
{
	for( DefaultObjectPropertiesList::const_iterator i = m_defaultObjectProperties.begin(); i != m_defaultObjectProperties.end(); ++i )
	{
		if(_stricmp(TargetObject->GetTypeName(), i->TypeName.c_str()) == 0)
		{
			for( StringMap::const_iterator j = i->Properties.begin(); j != i->Properties.end(); ++j )
				TargetObject->SetProperty( UILowerString (j->first), j->second );

			break;
		}
	}

	if (TargetObject->IsA (TUIWidget))
	{
		UIWidget * const wid    = static_cast<UIWidget *>(TargetObject);	
		const UIWidget * const parent = wid->GetParentWidget ();

		if (parent)
		{
			UISize size = wid->GetSize ();
			const UISize & parentSize = parent->GetSize ();
			size.x = std::min (size.x, parentSize.x);
			size.y = std::min (size.y, parentSize.y);
			wid->SetSize (size);
		}
	}
}

void DefaultObjectPropertiesManager::SaveTo( FILE *fp )
{
	fprintf( fp, "<DefaultObjectPropertiesManager>\n" );

	for( DefaultObjectPropertiesList::iterator i = m_defaultObjectProperties.begin();
	     i != m_defaultObjectProperties.end(); ++i )
	{
		fprintf( fp, "<%s>\n", i->TypeName );

		for( StringMap::iterator j = i->Properties.begin(); j != i->Properties.end(); ++j )
		{
			const UINarrowString & name = (*j).first;
			const UINarrowString value = Unicode::wideToNarrow ((*j).second);

			fprintf( fp, "'%s' = '%s'\n", name.c_str (), value.c_str ());
		}

		fprintf( fp, "</%s>\n", i->TypeName );
	}

	fprintf( fp, "</DefaultObjectPropertiesManager>\n" );
}

void DefaultObjectPropertiesManager::LoadFrom( FILE *fp )
{
	char LineBuffer[1024];

	while( fgets( LineBuffer, sizeof( LineBuffer ), fp ) )
	{
		char *pNewLine = strchr( LineBuffer, '\n' );

		if( pNewLine )
			*pNewLine = '\0';

		if( !_stricmp( LineBuffer, "<DefaultObjectPropertiesManager>" ) )
			continue;
		else if( !_stricmp( LineBuffer, "</DefaultObjectPropertiesManager>" ) )
			break;
		else if( *LineBuffer == '<' )
		{
			m_selectedPropertyMap = 0;

			if( *(LineBuffer + 1) == '/' )
				continue;

			if( strchr( LineBuffer + 1, '>' ) )
				*strchr( LineBuffer + 1, '>' ) = '\0';


			for( DefaultObjectPropertiesList::iterator i = m_defaultObjectProperties.begin(); i != m_defaultObjectProperties.end(); ++i )
			{
				if( !_stricmp( LineBuffer + 1, i->TypeName.c_str() ) )
				{
					m_selectedPropertyMap = &(i->Properties);
					break;
				}
			}
		}
		else
		{
			UINarrowString Name;
			UIString Value;
			char    *p = LineBuffer;
			bool     bInQuote = false;
			bool     bInValue = false;

			while( *p )
			{
				if( *p == '\'' )
				{
					if( bInQuote )
						bInQuote = false;
					else
						bInQuote = true;
				}
				else if( bInQuote )
				{
					if( bInValue )
						Value += *p;
					else
						Name += *p;
				}
				else if( *p == '=' )
					bInValue = true;

				p++;
			}

			if (m_selectedPropertyMap)
			{
				(*m_selectedPropertyMap)[Name] = Value;
			}
		}
	}	
}

