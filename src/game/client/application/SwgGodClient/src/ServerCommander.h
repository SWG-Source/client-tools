// ======================================================================
//
// ServerCommander.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ServerCommander_H
#define INCLUDED_ServerCommander_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

#include "Singleton/Singleton.h"

class CellProperty;
class ClientObject;
class CreatureObject;
class GameNetworkMessage;
class GodClientData;
class Object;
class Transform;
class Vector;

//-----------------------------------------------------------------

/**
* The ServerCommander is the entry point for issuing server-related commands, such
* as object creation/deletion/manipulation.  The ServerCommander mimics a bridge in
* that it behaves differently depending on the state of Game::getSinglePlayer ().
*
* @todo: undo does not work properly for many serverside operations.
*/

class ServerCommander : public QObject, public Singleton<ServerCommander>, public MessageDispatch::Emitter
{
	Q_OBJECT;  //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	                      ServerCommander();
	Object*               createObject(const std::string& name, const std::string& templateName, const CellProperty* cellProperty, const Transform& transform, bool pushUndo = true);
	unsigned int          deleteObject(ClientObject* obj, bool pushUndo = true);

	//-----------------------------------------------------------------
	//-- GodClientObjectManipulator stuff

	unsigned int          grantSkill            (const CreatureObject & obj, const std::string & skillName);
	unsigned int          revokeSkill           (const CreatureObject & obj, const std::string & skillName);
	virtual void          setObjectTransform    (ClientObject* obj, const Transform& transform, bool pushUndo);

	unsigned int          scriptReload          (const std::string& scriptName);
	unsigned int          scriptAttach          (const ClientObject& obj, const std::string& scriptName);
	unsigned int          scriptDetach          (const ClientObject& obj, const std::string& scriptName);
	unsigned int          objvarSet             (const ClientObject& obj, const std::string& objvarName, int value);
	unsigned int          objvarSet             (const ClientObject& obj, const std::string& objvarName, float value);
	unsigned int          objvarSet             (const ClientObject& obj, const std::string& objvarName, std::string const &value);
	unsigned int          objvarRemove          (const ClientObject& obj, const std::string& objvarName);

	unsigned int          scriptRequestList     (const NetworkId& networkId);
	unsigned int          skillRequestList      (const NetworkId& networkId);
	unsigned int          objvarRequestList     (const NetworkId& networkId);
	unsigned int          serverTemplateRequest (const NetworkId& networkId);
	unsigned int          serverTemplateReload  (const std::string& serverTemplateName);
	unsigned int          saveBuildoutArea      (const std::string& areaName, const std::string& areaLocation);
	unsigned int          editBuildoutArea      (const std::string& args);

	unsigned int          getSphereTreeSnapshot ();
	unsigned int          getTriggerVolumes     (const NetworkId& networkId);

	unsigned int          watchTargetPath       (const NetworkId& networkId);
	unsigned int          ignoreTargetPath      (const NetworkId& networkId);
	unsigned int          watchPathMap          (bool watch);

	unsigned int          getRegionsList        ();
	unsigned int          sendSystemMessage     (const std::string& messageType, const std::string targetName, const std::string& message);
	unsigned int          setGodMode            ();

signals:
	void                  consoleCommandIssued  (const char*);

private:
	//disabled
	ServerCommander(const ServerCommander & rhs);
	ServerCommander&  operator= (const ServerCommander & rhs);

	unsigned int          issueCommand (const char*);
};

// ======================================================================

#endif
