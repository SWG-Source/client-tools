//======================================================================
//
// CuiPopupHelpData.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiPopupHelpData_H
#define INCLUDED_CuiPopupHelpData_H

//======================================================================

class UIData;

//----------------------------------------------------------------------

class CuiPopupHelpData
{
public:

	enum DesiredPosition
	{
		DP_nw,
		DP_ne,
		DP_sw,
		DP_se,
		DP_count
	};

	struct DesiredPositionNames
	{
		static const std::string nw;
		static const std::string ne;
		static const std::string sw;
		static const std::string se;
	};

	static DesiredPosition findPositionByName (const std::string & name);

	std::string     widgetPath;
	Unicode::String localText;
	DesiredPosition desiredPosition;
	Unicode::String scriptPre;
	Unicode::String scriptPost;

	CuiPopupHelpData ();
	explicit CuiPopupHelpData (const UIData & data);

	void     loadFromData (const UIData & data);
};

//======================================================================

#endif
