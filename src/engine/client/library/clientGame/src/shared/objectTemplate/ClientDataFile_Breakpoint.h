//======================================================================
//
// ClientDataFile_Breakpoint.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientDataFile_Breakpoint_H
#define INCLUDED_ClientDataFile_Breakpoint_H

//======================================================================

#include "sharedMath/VectorArgb.h"
#include "sharedMath/Vector.h"

class ShaderTemplate;
class GlowAppearance;
class Iff;

//----------------------------------------------------------------------

class Breakpoint
{
public:
	void load (Iff & iff);
	Breakpoint ();
	~Breakpoint ();
	
public:
	
	std::string m_name;
	std::string m_dependency;
	std::string m_exclusion;
	std::string m_hardpointName;
	Vector m_point;
	Vector m_pointDelta;
	Vector m_normal;
	Vector m_normalDelta;
	float m_probability;
};

//======================================================================

#endif
