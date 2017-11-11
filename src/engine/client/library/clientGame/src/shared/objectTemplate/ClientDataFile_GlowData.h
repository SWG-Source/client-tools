//======================================================================
//
// ClientDataFile_GlowData.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientDataFile_GlowData_H
#define INCLUDED_ClientDataFile_GlowData_H

#include "sharedMath/VectorArgb.h"
#include "sharedMath/Vector.h"

class ShaderTemplate;
class GlowAppearance;
class Iff;

//======================================================================

class GlowData
{

public:
	
	void load (Iff & iff);
	GlowData ();
	~GlowData ();
	
public:
	
	std::string m_name;
	std::string m_hardpointName;
	std::string m_shaderTemplateNameFront;
	std::string m_shaderTemplateNameBack;
	VectorArgb  m_color;
	float       m_scaleMin;
	float       m_scaleMaxFront;
	float       m_scaleMaxBack;
	Vector      m_position;

	GlowAppearance * createAppearance() const;

private:

	ShaderTemplate const * m_shaderTemplateFront;
	ShaderTemplate const * m_shaderTemplateBack;

private:
	GlowData (const GlowData & rhs);
	GlowData & operator = (const GlowData & rhs);
};

//======================================================================

#endif
