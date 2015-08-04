//======================================================================
//
// UIPaletteRegistrySetup.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UIPaletteRegistrySetup.h"

#include "UIButtonStyle.h"
#include "UICheckbox.h"
#include "UIListStyle.h"
#include "UIPalette.h"
#include "UIPie.h"
#include "UIRectangleStyle.h"
#include "UITable.h"
#include "UIText.h"
#include "UITextboxStyle.h"
#include "UITooltipStyle.h"
#include "UITreeViewStyle.h"
#include "UIWidget.h"
#include "UIColorEffector.h"

#include <cassert>

//======================================================================

namespace
{
	bool s_installed = false;

#define MAKE_PAL(a) const UILowerString s_pal##a = UILowerString ("Pal" #a);

	MAKE_PAL (ActivatedIcon);
	MAKE_PAL (ActivatedText);
	MAKE_PAL (Bg);
	MAKE_PAL (BgColor);
	MAKE_PAL (BgTint);
	MAKE_PAL (Carat);
	MAKE_PAL (Color);
	MAKE_PAL (DisabledIcon);
	MAKE_PAL (DisabledText);
	MAKE_PAL (Grid);
	MAKE_PAL (GridH);
	MAKE_PAL (GridV);
	MAKE_PAL (HoverIcon);
	MAKE_PAL (HoverText);
	MAKE_PAL (NormalIcon);
	MAKE_PAL (NormalText);
	MAKE_PAL (Pie);
	MAKE_PAL (SelectedIcon);
	MAKE_PAL (SelectedText);
	MAKE_PAL (SelectedTextHighlight);
	MAKE_PAL (SelectedTextLowlight);
	MAKE_PAL (Selection);
	MAKE_PAL (SelectionBg);
	MAKE_PAL (SelectionRect);
	MAKE_PAL (Target);
	MAKE_PAL (Target2);
	MAKE_PAL (Text);
	MAKE_PAL (TextHighlight);
	MAKE_PAL (TextLowlight);

#undef MAKE_PAL
}

//----------------------------------------------------------------------

void UIPaletteRegistrySetup::install ()
{
	assert (!s_installed);

#define REG_PAL(a,b,c) UIPalette::RegisterPaletteEntry (T##a, s_pal##b, ##a::PropertyName::##c)

	REG_PAL (UIWidget,    Color,               Color);
	REG_PAL (UIWidget,    BgColor,             BackgroundColor);
	REG_PAL (UIWidget,    BgTint,              BackgroundTint);

	REG_PAL (UITable,     GridH,               GridColorHorizontal);
	REG_PAL (UITable,     GridV,               GridColorVertical);
	REG_PAL (UITable,     Text,                DefaultTextColor);
	REG_PAL (UITable,     SelectionBg,         SelectionColorBackground);
	REG_PAL (UITable,     SelectionRect,       SelectionColorRect);
	REG_PAL (UITable,     SelectedText,        SelectionTextColor);

	REG_PAL (UICheckbox,  Text,                TextColor);

	REG_PAL (UIPie,       Pie,                 PieColor);

	REG_PAL (UIText,      Text,                TextColor);

	REG_PAL (UIListStyle, Bg,                  BackgroundColor);
	REG_PAL (UIListStyle, Text,                DefaultTextColor);
	REG_PAL (UIListStyle, Grid,                GridColor);
	REG_PAL (UIListStyle, SelectionBg,         SelectionColorBackground);
	REG_PAL (UIListStyle, SelectionRect,       SelectionColorRect);
	REG_PAL (UIListStyle, SelectedText,        SelectionTextColor);


	REG_PAL (UIButtonStyle,    DisabledText,   DisabledTextColor);
	REG_PAL (UIButtonStyle,    DisabledIcon,   DisabledIconColor);
	REG_PAL (UIButtonStyle,    HoverText,      MouseOverTextColor);
	REG_PAL (UIButtonStyle,    HoverIcon,      MouseOverIconColor);
	REG_PAL (UIButtonStyle,    NormalText,     NormalTextColor);
	REG_PAL (UIButtonStyle,    NormalIcon,     NormalIconColor);
	REG_PAL (UIButtonStyle,    ActivatedText,  PressedTextColor);
	REG_PAL (UIButtonStyle,    ActivatedIcon,  PressedIconColor);
	REG_PAL (UIButtonStyle,    SelectedText,   SelectedTextColor);
	REG_PAL (UIButtonStyle,    SelectedIcon,   SelectedIconColor);

	REG_PAL (UIRectangleStyle, Color,          Color);

	REG_PAL (UITooltipStyle,   Text,           TextColor);

	REG_PAL (UITreeViewStyle,  Grid,           GridColor);
	REG_PAL (UITreeViewStyle,  SelectionBg,    SelectionColorBackground);
	REG_PAL (UITreeViewStyle,  SelectionRect,  SelectionColorRect);
	REG_PAL (UITreeViewStyle,  Text,           DefaultTextColor);
	REG_PAL (UITreeViewStyle,  TextHighlight,  DefaultTextColorHighlight);
	REG_PAL (UITreeViewStyle,  TextLowlight,   DefaultTextColorLowlight);
	REG_PAL (UITreeViewStyle,  SelectedText,          SelectionTextColor);
	REG_PAL (UITreeViewStyle,  SelectedTextHighlight, SelectionTextColorHighlight);
	REG_PAL (UITreeViewStyle,  SelectedTextLowlight,  SelectionTextColorLowlight);

	REG_PAL (UIColorEffector,  Target,         TargetColor);
	REG_PAL (UIColorEffector,  Target2,        TargetColor2);

	REG_PAL (UITextboxStyle,  Carat,           CaratColor);
	REG_PAL (UITextboxStyle,  Selection,       SelectionColor);
	REG_PAL (UITextboxStyle,  Text,            TextColor);

#undef REG_PAL

}

//======================================================================
