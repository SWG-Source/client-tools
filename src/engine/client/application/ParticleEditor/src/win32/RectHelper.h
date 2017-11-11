// ============================================================================
//
// RectHelper.h
// copyright 2001 Sony Online Entertainment
//
// ============================================================================

#ifndef RECTHELPER_H
#define RECTHELPER_H

//-----------------------------------------------------------------------------
class RectHelper
{
public:

	static QPoint mapToRect(QRect const &rect, QPoint const &point);
	static float  getPercentageInRectX(QRect const &rect, QPoint const &point);
	static float  getPercentageInRectY(QRect const &rect, QPoint const &point);
	static QPoint getPointInRect(QRect const &rect, float const xPercent, float const yPercent);
	static QPoint getPointInRect(QRect const &rect, float const xPercent, float const y, float const minValueY, float const maxValueY);
};

// ============================================================================

#endif // RECTHELPER_H