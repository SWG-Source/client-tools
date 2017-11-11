//======================================================================
//
// ShipComponentEditorServerTemplate.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentEditorServerTemplate_H
#define INCLUDED_ShipComponentEditorServerTemplate_H

//======================================================================

class ShipComponentEditorServerTemplate
{
public:
	std::string name; //lint !e1925 //public data member
	std::string fullPath; //lint !e1925 //public data member
	std::string fullPathSharedTemplate; //lint !e1925 //public data member
	std::string sharedTemplateName; //lint !e1925 //public data member
	std::string baseServerTemplate; //lint !e1925 //public data member
	std::string chassisType; //lint !e1925 //public data member

	bool load(std::string const & otName, bool isShip);

	bool operator<(ShipComponentEditorServerTemplate const & rhs) const;
	bool operator==(ShipComponentEditorServerTemplate const & rhs) const;
	bool operator!=(ShipComponentEditorServerTemplate const & rhs) const;

private:
	bool findValueForKey(std::string const & bufferStr, std::string const & key, std::string & result);
	void loadFromString(std::string const & bufferStr, bool isShip);

};

//----------------------------------------------------------------------

inline bool ShipComponentEditorServerTemplate::operator!=(ShipComponentEditorServerTemplate const & rhs) const
{
	return !(*this == rhs);
}

//----------------------------------------------------------------------

inline bool ShipComponentEditorServerTemplate::operator==(ShipComponentEditorServerTemplate const & rhs) const
{
	return name == rhs.name;
}

//----------------------------------------------------------------------

inline bool ShipComponentEditorServerTemplate::operator<(ShipComponentEditorServerTemplate const & rhs) const
{
	return name < rhs.name;
}

//======================================================================

#endif
