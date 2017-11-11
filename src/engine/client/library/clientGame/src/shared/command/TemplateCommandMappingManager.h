//======================================================================
//
// TemplateCommandMappingManager.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_TemplateCommandMappingManager_H
#define INCLUDED_TemplateCommandMappingManager_H

//======================================================================

class CreatureObject;

//----------------------------------------------------------------------

class TemplateCommandMappingManager
{
public:
	static void install();
	static void remove();

	static uint32 getCommandCrcForTemplateCrc(uint32 templateCrc);
	static const std::string& getCommandForTemplateCrc(uint32 templateCrc);
	static uint32 getFakeCommandGroupForTemplateCrc(uint32 templateCrc);
	static bool getExecuteFromToolbarForTemplateCrc(uint32 templateCrc);
	
protected:

};


//======================================================================

#endif
