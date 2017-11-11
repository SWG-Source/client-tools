//======================================================================
//
// CuiPopupHelpData.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiPopupHelpData.h"

#include "UIData.h"

//======================================================================

const std::string CuiPopupHelpData::DesiredPositionNames::nw = "nw";
const std::string CuiPopupHelpData::DesiredPositionNames::ne = "ne";
const std::string CuiPopupHelpData::DesiredPositionNames::sw = "sw";
const std::string CuiPopupHelpData::DesiredPositionNames::se = "se";

//----------------------------------------------------------------------

CuiPopupHelpData::CuiPopupHelpData () :
widgetPath      (),
localText       (),
desiredPosition (DP_nw),
scriptPre       (),
scriptPost      ()
{
}

//----------------------------------------------------------------------

CuiPopupHelpData::CuiPopupHelpData (const UIData & data) :
widgetPath      (),
localText       (),
desiredPosition (DP_nw),
scriptPre       (),
scriptPost      ()
{
	loadFromData (data);
}

//----------------------------------------------------------------------

CuiPopupHelpData::DesiredPosition CuiPopupHelpData::findPositionByName (const std::string & name)
{
	if (name == DesiredPositionNames::nw)
		return DP_nw;
	if (name == DesiredPositionNames::ne)
		return DP_ne;
	if (name == DesiredPositionNames::sw)
		return DP_sw;
	if (name == DesiredPositionNames::se)
		return DP_se;

	return DP_count;
}

//----------------------------------------------------------------------

void CuiPopupHelpData::loadFromData (const UIData & data)
{
	static const UILowerString prop_widget           ("Widget");
	static const UILowerString prop_localText        ("LocalText");
	static const UILowerString prop_text             ("Text");
	static const UILowerString prop_desiredPosition  ("DesiredPosition");
	static const UILowerString prop_scriptPre        ("ScriptPre");
	static const UILowerString prop_scriptPost       ("ScriptPost");

	//-- path
	data.GetPropertyNarrow (prop_widget, widgetPath);

	//-- text
	if (!data.GetProperty (prop_localText, localText))
	{
		if (!data.GetProperty (prop_text, localText))
		{
			DEBUG_FATAL (true, ("CuiPopupHelpData no text for [%s]", data.GetFullPath ().c_str ()));
		}
	}

	//-- position
	std::string dp;
	if (data.GetPropertyNarrow (prop_desiredPosition, dp))
		desiredPosition = findPositionByName (dp);

	data.GetProperty (prop_scriptPre,  scriptPre);
	data.GetProperty (prop_scriptPost, scriptPost);
}

//======================================================================
