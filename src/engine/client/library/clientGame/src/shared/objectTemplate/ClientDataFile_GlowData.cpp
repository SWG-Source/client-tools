//======================================================================
//
// ClientDataFile_GlowData.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientDataFile_GlowData.h"

#include "clientGame/GlowAppearance.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedMath/VectorArgb.h"

//======================================================================

GlowData::GlowData() :
m_name(),
m_hardpointName(),
m_shaderTemplateNameFront(),
m_shaderTemplateNameBack(),
m_color(VectorArgb::solidWhite),
m_scaleMin(0.0f),
m_scaleMaxFront(0.0f),
m_scaleMaxBack(0.0f),
m_position(),
m_shaderTemplateFront(NULL),
m_shaderTemplateBack(NULL)
{
}

//----------------------------------------------------------------------

GlowData::~GlowData()
{
	if (m_shaderTemplateFront != NULL)
	{
		m_shaderTemplateFront->release();
		m_shaderTemplateFront = 0;
	}

	if (m_shaderTemplateBack != NULL)
	{
		m_shaderTemplateBack->release();
		m_shaderTemplateBack = 0;
	}
}

//----------------------------------------------------------------------

void GlowData::load(Iff & iff)
{
	iff.enterChunk (TAG_INFO);
	{
		iff.read_string(m_name);
		iff.read_string(m_hardpointName);
		iff.read_string(m_shaderTemplateNameFront);
		iff.read_string(m_shaderTemplateNameBack);
		m_color = iff.read_floatVectorArgb();
		m_scaleMin = iff.read_float();
		m_scaleMaxFront = iff.read_float();
		m_scaleMaxBack = iff.read_float();

		DEBUG_WARNING(!TreeFile::exists(m_shaderTemplateNameFront.c_str()), ("GlowData::load: [%s] specifies invalid shader template %s", iff.getFileName(), m_shaderTemplateNameFront.c_str()));
		if (!m_shaderTemplateNameFront.empty())
			m_shaderTemplateFront = ShaderTemplateList::fetch(m_shaderTemplateNameFront.c_str());

		DEBUG_WARNING(!TreeFile::exists(m_shaderTemplateNameBack.c_str()), ("GlowData::load: [%s] specifies invalid shader template %s", iff.getFileName(), m_shaderTemplateNameBack.c_str()));
		if (!m_shaderTemplateNameBack.empty())
			m_shaderTemplateBack = ShaderTemplateList::fetch(m_shaderTemplateNameBack.c_str());
	}

	iff.exitChunk (TAG_INFO);

	static Tag const TAG_INF2 = TAG(I,N,F,2);

	if (iff.enterChunk(TAG_INF2, true))
	{
		m_position = iff.read_floatVector();
		iff.exitChunk(TAG_INF2);
	}
}

//----------------------------------------------------------------------

GlowAppearance * GlowData::createAppearance() const
{
	GlowAppearance * const glowAppearance = new GlowAppearance(m_shaderTemplateFront, m_shaderTemplateBack, m_color, m_scaleMin, m_scaleMaxFront, m_scaleMaxBack);
	return glowAppearance;
}

//======================================================================
