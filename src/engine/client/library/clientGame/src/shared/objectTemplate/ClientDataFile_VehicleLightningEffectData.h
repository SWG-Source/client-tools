//======================================================================
//
// ClientDataFile_VehicleLightningffectData.h
// copyright (c) 2008 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientDataFile_VehicleLightningEffectData_H
#define INCLUDED_ClientDataFile_VehicleLightningEffectData_H

//======================================================================

class Iff;
class AppearanceTemplate;

//----------------------------------------------------------------------

class VehicleLightningEffectData
{
public:
	
	static const VehicleLightningEffectData cms_invalid;
	
	//----------------------------------------------------------------------

	void load (Iff & iff);
	VehicleLightningEffectData ();
	~VehicleLightningEffectData ();
	
public:
	
	std::string m_startHardpointName;
	std::string m_endHardpointName;
	std::string m_appearanceTemplateName;

private:
	VehicleLightningEffectData (const VehicleLightningEffectData & rhs);
	VehicleLightningEffectData& operator = (const VehicleLightningEffectData & rhs);
};

//======================================================================

#endif
