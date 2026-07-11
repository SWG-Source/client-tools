#ifndef INCLUDED_SurveyMessageArchive_H
#define INCLUDED_SurveyMessageArchive_H

#include "sharedMathArchive/VectorArchive.h"
#include "SurveyMessageTypes.h"

namespace Archive
{
	inline void put(ByteStream& target, const Survey_DataItem& data)
	{
		put(target, data.m_location);
		put(target, data.m_efficiency);
	}

	inline void get(ReadIterator& source, Survey_DataItem& data)
	{
		get(source, data.m_location);
		get(source, data.m_efficiency);
	}
}

#endif