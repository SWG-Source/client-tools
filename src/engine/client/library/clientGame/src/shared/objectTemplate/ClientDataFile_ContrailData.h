//======================================================================
//
// ClientDataFile_ContrailData.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientDataFile_ContrailData_H
#define INCLUDED_ClientDataFile_ContrailData_H

//======================================================================

class Appearance;
class Iff;
class SwooshAppearance;
class SwooshAppearanceTemplate;

//----------------------------------------------------------------------

class ContrailData
{
public:
	
	void load (Iff & iff);
	ContrailData ();
	~ContrailData ();
	
public:
	
	std::string m_name;
	std::string m_hardpointName0;
	std::string m_hardpointName1;
	std::string m_swooshAppearanceName;
	std::string m_overrideName;
	float m_width;

	SwooshAppearance * createAppearance(Appearance const & appearance) const;

private:

	SwooshAppearanceTemplate const * m_swooshAppearanceTemplate;
	void load_0000(Iff & iff);

private:
	ContrailData (const ContrailData & rhs);
	ContrailData& operator = (const ContrailData & rhs);
};

//======================================================================

#endif
