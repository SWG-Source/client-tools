#ifndef INCLUDED_SurveyMessageTypes_H
#define INCLUDED_SurveyMessageTypes_H

#include "sharedMath/Vector.h"

struct Survey_DataItem
{
	Vector m_location;
	float  m_efficiency;
};

#endif // INCLUDED_SurveyMessageTypes_H