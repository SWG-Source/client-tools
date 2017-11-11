//======================================================================
//
// ClientDataFile_InterpolatedSound.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientDataFile_InterpolatedSound.h"

#include "clientGame/GlowAppearance.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"

//======================================================================

InterpolatedSound::InterpolatedSound() :
m_hardpointName(),
m_soundTemplateName(),
m_volumeMin(0.0f),
m_volumeMax(1.0f),
m_pitchDeltaMin(0.0f),
m_pitchDeltaMax(0.0f),
m_interpolationRateUp(1.0f),
m_interpolationRateDown(1.0f)
{
}

//----------------------------------------------------------------------

InterpolatedSound::~InterpolatedSound()
{
}

//----------------------------------------------------------------------

void InterpolatedSound::load(Iff & iff)
{
	iff.enterChunk (TAG_INFO);
	{
		iff.read_string(m_hardpointName);
		iff.read_string(m_soundTemplateName);
		m_volumeMin = iff.read_float();
		m_volumeMax = iff.read_float();
		m_pitchDeltaMin = iff.read_float();
		m_pitchDeltaMax = iff.read_float();
		m_interpolationRateUp = iff.read_float();
		m_interpolationRateDown = iff.read_float();

		DEBUG_WARNING(!TreeFile::exists(m_soundTemplateName.c_str()), ("InterpolatedSound::load: [%s] specifies invalid sound template %s", iff.getFileName(), m_soundTemplateName.c_str()));
	}

	iff.exitChunk (TAG_INFO);
}

//======================================================================
