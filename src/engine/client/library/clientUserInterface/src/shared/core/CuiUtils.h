//======================================================================
//
// CuiUtils.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiUtils_H
#define INCLUDED_CuiUtils_H

//======================================================================

class PackedArgb;
class Vector2d;
class Vector;
class VectorArgb;
struct UIColor;

//----------------------------------------------------------------------

namespace CuiUtils
{
	bool					ParseVector2d    (const Unicode::String &, Vector2d & );
	bool					ParseVector      (const Unicode::String &, Vector & );
	bool					ParseVectorArgb  (const Unicode::String &, VectorArgb & );

	bool                    FormatVector2d   (Unicode::String &, const Vector2d & );
	bool					FormatVector     (Unicode::String &, const Vector & );
	bool					FormatVectorArgb (Unicode::String &, const VectorArgb & );

	time_t                  GetSystemSeconds  (); // Pass this into the following formatting functions
	time_t                  GetTimeZoneDifferenceFromUTC();

	bool                    FormatDate        (Unicode::String &, time_t secs);

	bool                    FormatTimeDuration (Unicode::String &, time_t secs, bool showDays, bool showHours, bool showMins, bool showSecs, bool shortHand = false);

	const UIColor           convertColor     (const PackedArgb & pargb);
	const PackedArgb        convertColor     (const UIColor & color);

	const VectorArgb        convertToVectorArgb (const UIColor & color);
	const UIColor           convertColor     (const VectorArgb & vargb);
}


//======================================================================

#endif
