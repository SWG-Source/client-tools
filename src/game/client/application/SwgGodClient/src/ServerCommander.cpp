// ======================================================================
//
// ServerCommander.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "ServerCommander.h"

#include "BuildoutAreaSupport.h"
#include "GodClientData.h"
#include "Modification.h"
#include "ModificationHistory.h"
#include "ServerCommander.moc"
#include "ServerObjectData.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientController.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/GroundScene.h"
#include "clientGame/RemoteCreatureController.h"
#include "clientGraphics/RenderWorld.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Vector.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "sharedNetworkMessages/SceneChannelMessages.h"
#include "sharedObject/CellProperty.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/PortalProperty.h"

#include <qmessagebox.h>

// =====================================================================

namespace
{
	//nested class definitions
	class TransformModification : public Modification
	{
	public:
		virtual bool undo()
		{
			ClientObject* const obj = dynamic_cast<ClientObject*>(m_obj.findInClientWorld());
			
			if(!obj)
				return false;
			
			ServerCommander::getInstance().setObjectTransform(obj, m_obj.transform, false);
			return true;
		}
		
		virtual bool redo()
		{
			ClientObject* const obj = dynamic_cast<ClientObject*>(m_obj.findInClientWorld());
			
			if(!obj)
				return false;
			
			ServerCommander::getInstance().setObjectTransform(obj, m_newTransform, false);
			return true;
		}
		
		virtual ~TransformModification()
		{
		}
		
		TransformModification(const Object& obj, const Transform& newTransform) :
		Modification(),
		m_obj(obj),
			m_newTransform(newTransform)
		{
		}
		
	private:
		//disabled
		TransformModification();                                            //lint !e754 unreferenced function declaration, that's because we don't want it defined
		TransformModification(const TransformModification& rhs);            //lint !e754 unreferenced function declaration, that's because we don't want it defined
		TransformModification& operator=(const TransformModification& rhs); //lint !e754 unreferenced function declaration, that's because we don't want it defined
	private:
		GodClientData::ClipboardObject m_obj;
		Transform                      m_newTransform;
		
	};

	//another class definition
	class DeleteModification : public Modification
	{
	public:
		virtual bool undo()
		{
			ClientObject* const obj = dynamic_cast<ClientObject*>(m_obj.findInClientWorld());
			
			if(obj)
				return false;
			
			std::string templateName = Game::getSinglePlayer() ?
				m_obj.sharedObjectTemplateName : m_obj.serverObjectTemplateName;

			IGNORE_RETURN(ServerCommander::getInstance().createObject("", templateName, CellProperty::getWorldCellProperty(), m_obj.transform, false));
			return true;
		}
		
		virtual bool redo()
		{
			ClientObject* const obj = dynamic_cast<ClientObject*>(m_obj.findInClientWorld());
			
			if(!obj)
				return false;

			IGNORE_RETURN(ServerCommander::getInstance().deleteObject(obj, false));
			return true;
		}
		
		virtual ~DeleteModification()
		{
		}
		
		explicit DeleteModification(const Object& obj) :
		Modification(),
		m_obj(obj)
		{
		}
		
	private:
		//disabled
		DeleteModification();                                         //lint !e754 unreferenced function declaration, that's because we don't want it defined
		DeleteModification(const DeleteModification& rhs);            //lint !e754 unreferenced function declaration, that's because we don't want it defined
		DeleteModification& operator=(const DeleteModification& rhs); //lint !e754 unreferenced function declaration, that's because we don't want it defined

	private:
		GodClientData::ClipboardObject m_obj;		
	};

	//even another class definition
	class CreateModification : public Modification
	{
	public:
		virtual bool undo()
		{
			ClientObject* const obj = dynamic_cast<ClientObject*>(m_obj.findInClientWorld());
			
			if(!obj)
				return false;
			
			IGNORE_RETURN(ServerCommander::getInstance().deleteObject(obj, false));
			return true;
		}
		
		virtual bool redo()
		{
			ClientObject* const obj = dynamic_cast<ClientObject*>(m_obj.findInClientWorld());
			
			if(obj)
				return false;

			std::string templateName = Game::getSinglePlayer() ?
				m_obj.sharedObjectTemplateName : m_obj.serverObjectTemplateName;

			IGNORE_RETURN(ServerCommander::getInstance().createObject("", templateName, CellProperty::getWorldCellProperty(), m_obj.transform, false));
			return true;
		}
		
		virtual ~CreateModification()
		{
		}
		
		explicit CreateModification(const Object& obj) :
		Modification(),
		m_obj(obj)
		{
		}
		
	private:
		//disabled
		CreateModification();                                         //lint !e754 unreferenced function declaration, that's because we don't want it defined
		CreateModification(const CreateModification& rhs);            //lint !e754 unreferenced function declaration, that's because we don't want it defined
		CreateModification& operator=(const CreateModification& rhs); //lint !e754 unreferenced function declaration, that's because we don't want it defined
		
	private:
		GodClientData::ClipboardObject m_obj;
	};
}

//-----------------------------------------------------------------

ServerCommander::ServerCommander()
: QObject(),
  Singleton<ServerCommander>(),
  MessageDispatch::Emitter()
{
}

//-----------------------------------------------------------------

/**
 * Build a command to ask the server to create an instance of an object template
 * @pre templateName must be fully qualified (i.e. object/apron.iff)
 */
Object* ServerCommander::createObject(const std::string& theName, const std::string& templateName, const CellProperty* cellProperty, const Transform& transform, bool pushUndo)
{
	UNREF(pushUndo);
	
	if (Game::getSinglePlayer())
	{
		if (templateName.find("/shared_") == std::string::npos)
		{
			// Single player mode, creating an object using a server template, which really means we're modifying an area buildout file
			Object* obj = BuildoutAreaSupport::createNewObject(ConstCharCrcString(templateName.c_str()), cellProperty, transform);
			return obj;
		}
		else
		{		
			// create the equipped object
			ClientObject * const obj = safe_cast<ClientObject *>(ObjectTemplate::createObject (templateName.c_str()));
			if (!obj)
			{
				const std::string msg = "Unable to load template: " + templateName;
				IGNORE_RETURN(QMessageBox::warning(0, "Warning", msg.c_str()));
				return 0;
			}
			
			obj->setDebugName(theName.c_str());
			obj->setObjectName(Unicode::narrowToWide(theName));

			obj->setParentCell (const_cast<CellProperty*> (cellProperty));
			CellProperty::setPortalTransitionsEnabled (false);
				obj->setTransform_o2p (transform);
			CellProperty::setPortalTransitionsEnabled (true);

			RenderWorld::addObjectNotifications (*obj);

			PortalProperty* const property = obj->getPortalProperty ();
			if (property)
				property->clientSinglePlayerInitializeFirstTimeObject ();

			obj->endBaselines ();
			obj->addToWorld();

			if(pushUndo)
				ModificationHistory::getInstance().addModification(new CreateModification(*obj));

			return obj;
		}
	}
	else
	{
		char buf[1024];
		const Vector& pos = transform.getPosition_p();
		Quaternion q(transform);

		if (cellProperty == CellProperty::getWorldCellProperty ())
			IGNORE_RETURN(_snprintf(buf, 1024, "object createTranslateRotate %s %f %f %f %f %f %f %f", templateName.c_str(), pos.x, pos.y, pos.z, q.w, q.x, q.y, q.z));
		else
			IGNORE_RETURN(_snprintf(buf, 1024, "object cellCreateTranslateRotate %s %s %f %f %f %f %f %f %f", templateName.c_str(), cellProperty->getOwner ().getNetworkId ().getValueString().c_str(), pos.x, pos.y, pos.z, q.w, q.x, q.y, q.z));

		IGNORE_RETURN(issueCommand(buf));
		return NULL;
	}

	return NULL;
}

//-----------------------------------------------------------------

/**
 * Build a command to ask the server to delete the given object
 *
 */
unsigned int ServerCommander::deleteObject(ClientObject* obj, bool pushUndo)
{
	//-- zero networkid (!isValid()) indicates a client-only object such as an ILF object

	if(Game::getSinglePlayer() || !obj->getNetworkId().isValid())
	{
		GroundScene* const gs = dynamic_cast<GroundScene*>(Game::getScene());
		
		// @todo: do not delete ANY player objects?
		if(gs && gs->getPlayer() == obj)
			return 0;

		if(pushUndo)
			ModificationHistory::getInstance().addModification(new DeleteModification(*obj));

		if (!(obj->getNetworkId() < NetworkId::cms_invalid) || BuildoutAreaSupport::deleteBuildoutObject(*obj))
		{
			obj->kill();

			//-- Ensure the ghost gets an alter call this upcoming frame so it gets deleted properly.
			obj->scheduleForAlter();
		}
	}
	else
	{
		char buf[1024];
		IGNORE_RETURN(_snprintf(buf, 1024, "object destroy %s", obj->getNetworkId().getValueString().c_str() ));
		return issueCommand(buf);
	}
	return 0;
}

//-----------------------------------------------------------------

void ServerCommander::setObjectTransform(ClientObject* obj, const Transform& transform, bool pushUndo)
{
	UNREF(pushUndo);

	NOT_NULL(obj);

	if(Game::getSinglePlayer())
	{
		if(pushUndo)
			ModificationHistory::getInstance().addModification(new TransformModification(*obj, transform));

		if (!(obj->getNetworkId() < NetworkId::cms_invalid) || BuildoutAreaSupport::setObjectTransform(*obj, transform))
		{
			obj->setTransform_o2p(transform);
			Controller * const controller = obj->getController();
			if (controller)
			{
				RemoteCreatureController * const remoteCreatureController = dynamic_cast<RemoteCreatureController *>(controller);
				if (remoteCreatureController)
					remoteCreatureController->updateDeadReckoningModel(obj->getParentCell(), transform, 0.f);
			}
		}
	}
	else
	{
		if(pushUndo)
		{

			const ServerObjectData::ObjectInfo* serverObjectData = ServerObjectData::getInstance().getObjectInfo(obj->getNetworkId(), false);
			if(!serverObjectData)
			{
				QApplication::beep();
				return;
			}
			ModificationHistory::getInstance().addModification(new TransformModification(*obj, transform));
		}
		ClientController * controller = dynamic_cast<ClientController*>(obj->getController());
		controller->sendTransform(transform);
	}
}

//-----------------------------------------------------------------

/**
 * Build a command to ask the server to reload a given script from disk
 *
 */
unsigned int ServerCommander::scriptReload(const std::string& scriptName)
{
	if(!Game::getSinglePlayer())
	{
		char buf[1024];
		IGNORE_RETURN(_snprintf(buf, 1024, "script reload %s", scriptName.c_str()));
		return issueCommand(buf);
	}
	return 0;
}

//-----------------------------------------------------------------

/**
 * Build a command to ask the server to attach a script from the given object
 *
 */
unsigned int ServerCommander::scriptAttach(const ClientObject& obj, const std::string& scriptName)
{
	if (Game::getSinglePlayer())
	{
		if (obj.getNetworkId() < NetworkId::cms_invalid)
			BuildoutAreaSupport::attachScript(obj, scriptName);
		return 0;
	}

	char buf[1024];
	IGNORE_RETURN(_snprintf(buf, 1024, "script attach %s %s", scriptName.c_str(), obj.getNetworkId().getValueString().c_str() ));
	return issueCommand(buf);
}

//-----------------------------------------------------------------

/**
 * Build a command to ask the server to detach a script from the given object
 *
 */
unsigned int ServerCommander::scriptDetach(const ClientObject& obj, const std::string& scriptName)
{
	if (Game::getSinglePlayer())
	{
		if (obj.getNetworkId() < NetworkId::cms_invalid)
			BuildoutAreaSupport::detachScript(obj, scriptName);
		return 0;
	}

	char buf[1024];
	IGNORE_RETURN(_snprintf(buf, 1024, "script detach %s %s", scriptName.c_str(), obj.getNetworkId().getValueString().c_str()));
	return issueCommand(buf);
}

//----------------------------------------------------------------------

unsigned int ServerCommander::objvarSet(const ClientObject& obj, const std::string& objvarName, int value)
{
	if (Game::getSinglePlayer())
	{
		if (obj.getNetworkId() < NetworkId::cms_invalid)
			BuildoutAreaSupport::setObjvar(obj, objvarName, value);
		return 0;
	}

	char buf[1024];
	IGNORE_RETURN(_snprintf(buf, 1024, "objvar set %s %s %d", obj.getNetworkId().getValueString().c_str(), objvarName.c_str(), value));
	return issueCommand(buf);
}

//----------------------------------------------------------------------

unsigned int ServerCommander::objvarSet(const ClientObject& obj, const std::string& objvarName, float value)
{
	if (Game::getSinglePlayer())
	{
		if (obj.getNetworkId() < NetworkId::cms_invalid)
			BuildoutAreaSupport::setObjvar(obj, objvarName, value);
		return 0;
	}

	char buf[1024];
	IGNORE_RETURN(_snprintf(buf, 1024, "objvar set %s %s %g", obj.getNetworkId().getValueString().c_str(), objvarName.c_str(), value));
	return issueCommand(buf);
}

//----------------------------------------------------------------------

unsigned int ServerCommander::objvarSet(const ClientObject& obj, const std::string& objvarName, std::string const &value)
{
	if (Game::getSinglePlayer())
	{
		if (obj.getNetworkId() < NetworkId::cms_invalid)
			BuildoutAreaSupport::setObjvar(obj, objvarName, value);
		return 0;
	}

	char buf[1024];
	IGNORE_RETURN(_snprintf(buf, 1024, "objvar set %s %s %s", obj.getNetworkId().getValueString().c_str(), objvarName.c_str(), value.c_str()));
	return issueCommand(buf);
}

//----------------------------------------------------------------------

unsigned int ServerCommander::objvarRemove(const ClientObject& obj, const std::string& objvarName)
{
	if (Game::getSinglePlayer())
	{
		if (obj.getNetworkId() < NetworkId::cms_invalid)
			BuildoutAreaSupport::removeObjvar(obj, objvarName);
		return 0;
	}

	char buf[1024];
	IGNORE_RETURN(_snprintf(buf, 1024, "objvar remove %s %s", obj.getNetworkId().getValueString().c_str(), objvarName.c_str()));
	return issueCommand(buf);
}

//----------------------------------------------------------------------

/**
 * Build a command to ask the server to return a list of all the scripts attached to an object
 *
 */
unsigned int ServerCommander::scriptRequestList(const NetworkId& networkId)
{
	if(Game::getSinglePlayer())
		return 0;

	char buf[1024];
	IGNORE_RETURN(_snprintf(buf, 1024, "script list %s", networkId.getValueString().c_str()));
	return issueCommand(buf);
}

//-----------------------------------------------------------------------

unsigned int ServerCommander::skillRequestList(const NetworkId& networkId)
{
	if(Game::getSinglePlayer())
		return 0;
	char buf[1024];
	IGNORE_RETURN(_snprintf(buf, 1024, "object getSkillList %s", networkId.getValueString().c_str()));
	return issueCommand(buf);
}

//-----------------------------------------------------------------

/**
 * Build a command to ask the server to return a list of all the objvars
 *
 */
unsigned int ServerCommander::objvarRequestList(const NetworkId& networkId)
{
	if(Game::getSinglePlayer())
		return 0;

	char buf[1024];
	IGNORE_RETURN(_snprintf(buf, 1024, "objvar list %s", networkId.getValueString().c_str()));
	return issueCommand(buf);
}

//-----------------------------------------------------------------

/**
 * Build a command to ask the server to take a given networkId and return the servertemplate name string
 *
 */
unsigned int ServerCommander::serverTemplateRequest(const NetworkId& networkId)
{
	if(Game::getSinglePlayer())
		return 0;

	char buf[1024];
	IGNORE_RETURN(_snprintf(buf, 1024, "object getServerTemplate %s", networkId.getValueString().c_str()));
	return issueCommand(buf);
}

//-----------------------------------------------------------------

unsigned int ServerCommander::saveBuildoutArea(std::string const & areaName, std::string const &areaLocation)
{
	if (Game::getSinglePlayer())
	{
		BuildoutAreaSupport::saveBuildoutArea(areaName);
		return 0;
	}

	char buf[1024];
	IGNORE_RETURN(_snprintf(buf, 1024, "server clientSaveBuildoutArea %s %s %s", Game::getSceneId().c_str(), areaName.c_str(), areaLocation.c_str()));
	return issueCommand(buf);
}

//-----------------------------------------------------------------

unsigned int ServerCommander::editBuildoutArea(const std::string& args)
{
	if(Game::getSinglePlayer())
		return 0;

	char buf[1024];
	IGNORE_RETURN(_snprintf(buf, 1024, "server editBuildoutArea %s", args.c_str()));
	return issueCommand(buf);
}

//-----------------------------------------------------------------

/**
 * Build a command to request a snapshot of the current sphere tree for the server
 *
 */
unsigned int ServerCommander::getSphereTreeSnapshot()
{
	if(Game::getSinglePlayer())
		return 0;

	char buf[1024] = "server showObjectSpheres";
	return issueCommand(buf);
}

//-----------------------------------------------------------------

/**
 * Build a command to request the trigger volume information for the given object
 *
 */
unsigned int ServerCommander::getTriggerVolumes(const NetworkId& networkId)
{
	if(Game::getSinglePlayer())
		return 0;

	char buf[1024];
	IGNORE_RETURN(_snprintf(buf, 1024, "object triggerVolumes %s", networkId.getValueString().c_str()));
	return issueCommand(buf);
}

//-----------------------------------------------------------------

/**
 *  Ask the server to reload the given server template from disk
 */
unsigned int ServerCommander::serverTemplateReload(const std::string& serverTemplateName)
{
	if(!Game::getSinglePlayer())
	{
		char buf[1024];
		IGNORE_RETURN(_snprintf(buf, 1024, "object reloadServerTemplate %s", serverTemplateName.c_str()));
		return issueCommand(buf);
	}
	return 0;
}

//-----------------------------------------------------------------------

unsigned int ServerCommander::grantSkill(const CreatureObject & creature, const std::string & skillName)
{
	if(!Game::getSinglePlayer())
	{
		char buf[1024];
		IGNORE_RETURN(_snprintf(buf, 1024, "object grantSkill \"%s\" %s", skillName.c_str(), creature.getNetworkId().getValueString().c_str()));
		return issueCommand(buf);
	}
	return 0;
}

//-----------------------------------------------------------------------

unsigned int ServerCommander::revokeSkill(const CreatureObject & creature, const std::string & skillName)
{
	if(!Game::getSinglePlayer())
	{
		char buf[1024];
		IGNORE_RETURN(_snprintf(buf, 1024, "object revokeSkill \"%s\" %s", skillName.c_str(), creature.getNetworkId().getValueString().c_str()));
		return issueCommand(buf);
	}
	return 0;
}

//-----------------------------------------------------------------

unsigned int ServerCommander::watchTargetPath(const NetworkId& networkId)
{
	if(!Game::getSinglePlayer())
	{
		char buf[1024];
		IGNORE_RETURN(_snprintf(buf, 1024, "object watchAIPath %s 1", networkId.getValueString().c_str()));
		return issueCommand(buf);
	}
	return 0;
}

//-----------------------------------------------------------------

unsigned int ServerCommander::ignoreTargetPath(const NetworkId& networkId)
{
	if(!Game::getSinglePlayer())
	{
		char buf[1024];
		IGNORE_RETURN(_snprintf(buf, 1024, "object watchAIPath %s 0", networkId.getValueString().c_str()));
		return issueCommand(buf);
	}
	return 0;
}

//-----------------------------------------------------------------

unsigned int ServerCommander::watchPathMap(bool watch)
{
	if(!Game::getSinglePlayer())
	{
		char buf[1024];
		IGNORE_RETURN(_snprintf(buf, 1024, "server watchPathMap %d", watch));
		return issueCommand(buf);
	}
	return 0;
}

//-----------------------------------------------------------------

unsigned int ServerCommander::getRegionsList()
{
	if(!Game::getSinglePlayer())
	{
		char buf[1024];
		IGNORE_RETURN(_snprintf(buf, 1024, "server listRegions"));
		return issueCommand(buf);
	}
	return 0;
}

//-----------------------------------------------------------------

unsigned int ServerCommander::sendSystemMessage(const std::string& messageType, const std::string targetName, const std::string& message)
{
	if(!Game::getSinglePlayer())
	{
		char buf[1024];
		IGNORE_RETURN(_snprintf(buf, 1024, "systemMessage %s %s %s", messageType.c_str(), targetName.c_str(), message.c_str()));
		return issueCommand(buf);
	}
	return 0;
}

//-----------------------------------------------------------------

/**
 *  Send the given command to the server we're connected it
 */
unsigned int ServerCommander::issueCommand(const char* buf)
{
	static unsigned int last_command = 0;
	const ConGenericMessage m(buf, ++last_command);
	GameNetwork::send(m, true);
	emit consoleCommandIssued(buf);
	return last_command;
}

//-----------------------------------------------------------------

unsigned int ServerCommander::setGodMode()
{
	IGNORE_RETURN(ClientCommandQueue::enqueueCommand("setGodMode", NetworkId::cms_invalid, Unicode::emptyString));
	return 0;
}

// ================================================================
