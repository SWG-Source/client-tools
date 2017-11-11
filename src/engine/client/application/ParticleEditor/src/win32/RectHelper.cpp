// ============================================================================
//
// RectHelper.cpp
// copyright 2001 Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "RectHelper.h"

//-----------------------------------------------------------------------------
QPoint RectHelper::mapToRect(QRect const &rect, QPoint const &point)
{
	return QPoint(point.x() - rect.left(), static_cast<int>(static_cast<float>(point.y() - rect.top())));
}

//-----------------------------------------------------------------------------
float RectHelper::getPercentageInRectX(QRect const &rect, QPoint const &point)
{
	QPoint mappedPoint(mapToRect(rect, point));

	return static_cast<float>(mappedPoint.x()) / static_cast<float>(rect.width());
}

//-----------------------------------------------------------------------------
float RectHelper::getPercentageInRectY(QRect const &rect, QPoint const &point)
{
	QPoint mappedPoint(mapToRect(rect, point));

	return static_cast<float>(mappedPoint.y()) / static_cast<float>(rect.height());
}

//-----------------------------------------------------------------------------
QPoint RectHelper::getPointInRect(QRect const &rect, float const xPercent, float const y, float const minValueY, float const maxValueY)
{
	float yPercent = 0.0f;

	if (maxValueY - minValueY > 0.0f)
	{
		yPercent = (maxValueY - y) / (maxValueY - minValueY);
	}
	
	return getPointInRect(rect, xPercent, yPercent);
}

//-----------------------------------------------------------------------------
QPoint RectHelper::getPointInRect(QRect const &rect, float const xPercent, float const yPercent)
{
   int const x = rect.left() + static_cast<int>(xPercent * static_cast<float>(rect.width() - 1));
   int const y = rect.top() + static_cast<int>(yPercent * static_cast<float>(rect.height() - 1));

	return QPoint(x, y);
}

// ============================================================================