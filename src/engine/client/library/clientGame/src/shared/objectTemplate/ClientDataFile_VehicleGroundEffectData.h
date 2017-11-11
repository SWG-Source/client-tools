//======================================================================
//
// ClientDataFile_VehicleGroundEffectData.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientDataFile_VehicleGroundEffectData_H
#define INCLUDED_ClientDataFile_VehicleGroundEffectData_H

//======================================================================

class Iff;
class AppearanceTemplate;

//----------------------------------------------------------------------

class VehicleGroundEffectData
{
public:
	
	static const VehicleGroundEffectData cms_invalid;
	
	//----------------------------------------------------------------------

	void load (Iff & iff);
	VehicleGroundEffectData ();
	~VehicleGroundEffectData ();
	
public:
	
	std::string                m_soundTemplateName;
	bool                       m_isWaterEffect;
	std::string                m_hardpointName;
	std::string                m_appearanceTemplateName;
	const AppearanceTemplate * m_appearanceTemplate;

private:
	VehicleGroundEffectData (const VehicleGroundEffectData & rhs);
	VehicleGroundEffectData& operator = (const VehicleGroundEffectData & rhs);
};

//======================================================================

#endif
