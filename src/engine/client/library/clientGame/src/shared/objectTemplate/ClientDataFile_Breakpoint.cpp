//======================================================================
//
// ClientDataFile_Breakpoint.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientDataFile_Breakpoint.h"

#include "sharedFile/Iff.h"

//======================================================================

Breakpoint::Breakpoint() :
m_name(),
m_dependency(),
m_exclusion(),
m_point(),
m_pointDelta(),
m_normal(),
m_normalDelta(),
m_probability(0.0f)
{
}

//----------------------------------------------------------------------

Breakpoint::~Breakpoint()
{
}

//----------------------------------------------------------------------

void Breakpoint::load (Iff & iff)
{
	iff.enterChunk (TAG_INFO);
	{
		iff.read_string(m_name);
		iff.read_string(m_dependency);
		iff.read_string(m_exclusion);
		iff.read_string(m_hardpointName);
		m_point = iff.read_floatVector();
		m_pointDelta = iff.read_floatVector();

		//- delta must be positive
		m_pointDelta.x = fabsf(m_pointDelta.x);
		m_pointDelta.y = fabsf(m_pointDelta.y);
		m_pointDelta.z = fabsf(m_pointDelta.z);

		m_normal = iff.read_floatVector();
		if (m_hardpointName.empty() && !m_normal.normalize())
			DEBUG_FATAL(true, ("ClientDataFile_Breakpoint normal [%f,%f,%f] failed to normalize", m_normal.x, m_normal.y, m_normal.z));
		m_normalDelta = iff.read_floatVector();

		//- delta must be positive
		m_normalDelta.x = convertDegreesToRadians(fabsf(m_normalDelta.x));
		m_normalDelta.y = convertDegreesToRadians(fabsf(m_normalDelta.y));
		m_normalDelta.z = convertDegreesToRadians(fabsf(m_normalDelta.z));

		m_probability = iff.read_float();
	}
	iff.exitChunk (TAG_INFO);
}


//======================================================================
