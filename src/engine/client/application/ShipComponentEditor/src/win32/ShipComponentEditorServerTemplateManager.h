//======================================================================
//
// ShipComponentEditorServerTemplateManager.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentEditorServerTemplateManager_H
#define INCLUDED_ShipComponentEditorServerTemplateManager_H

//======================================================================

class ShipComponentEditorServerTemplate;

//----------------------------------------------------------------------

class ShipComponentEditorServerTemplateManager
{
public:

	struct Messages
	{
	public:
		struct TemplateListChanged;
	};

	static void install();
	static void remove();

	static void load();
	static void clear();
	
	typedef stdvector<ShipComponentEditorServerTemplate>::fwd ServerTemplateVector;

	static void findObjectTemplatesForChassisType(std::string const & chassisType, ServerTemplateVector & result);

	static ShipComponentEditorServerTemplate const * findTemplateByName(std::string const & name); 
	static ShipComponentEditorServerTemplate const * findTemplateByCrc(uint32 crc);

	static void regenerateTemplateDb();
};

//======================================================================

#endif
