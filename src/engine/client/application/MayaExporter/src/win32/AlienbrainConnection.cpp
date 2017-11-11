// ======================================================================
//
// AlienbrainConnection.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"

// JU_TODO: alienbrain def out
#if 0

#include "AlienbrainConnection.h"
#include <NxNConstants.h>

#include "ExporterLog.h"
#include "Messenger.h"

#pragma warning (disable: 4505)

//////////////////////////////////////////////////////////////////////////////////

CNxNSmartIntegrator* AlienbrainConnection::ms_SmartIntegrator;
CNxNIntegrator*      AlienbrainConnection::ms_Integrator;
CNxNWorkspace*       AlienbrainConnection::ms_Workspace;
CNxNProject*         AlienbrainConnection::ms_Project;
bool                 AlienbrainConnection::ms_installed;
bool                 AlienbrainConnection::ms_connected;

Messenger*           AlienbrainConnection::messenger;

//////////////////////////////////////////////////////////////////////////////////

/**
 * Install the module.
 */
void AlienbrainConnection::install(Messenger* newMessenger)
{
	messenger = newMessenger;
	ms_SmartIntegrator = NULL;
	ms_Integrator = NULL;
	ms_Workspace = NULL;
	ms_Project = NULL;
	ms_installed = true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Remove the module, closing down any data structures.
 */
void AlienbrainConnection::remove()
{
	if(ms_SmartIntegrator)
	{
		delete ms_SmartIntegrator;
		ms_SmartIntegrator = NULL;
	}
	ms_Integrator = NULL;
	ms_Workspace = NULL;
	ms_Project = NULL;

	ms_installed = false;
}

//////////////////////////////////////////////////////////////////////////////////
//begin Alienbrain wrapper functions

/**
 * Opens an Alienbrain connection.  Serves as a wrapper around the Alienbrain XDK
 */
bool AlienbrainConnection::initConnection(const CNxNString& sProjectName)
{
	FATAL(!ms_installed, ("AlienbrainConnection not installed"));

	if(ms_SmartIntegrator)
	{
		delete ms_SmartIntegrator;
		ms_SmartIntegrator = NULL;
	}

	ms_SmartIntegrator = new CNxNSmartIntegrator();
	bool result = false;
	if (!ms_SmartIntegrator || !ms_SmartIntegrator->InitInstance(NXN_CONNECT_HIDE_DIALOG_AND_CONNECT))
	{
		delete ms_SmartIntegrator;
		ms_SmartIntegrator = NULL;
		return false;
	};

	ms_Integrator = ms_SmartIntegrator->GetIntegrator();

	ms_Workspace = ms_Integrator->CreateWorkspace(_STR("C:\\test.nwk")); // Filename is unimporant, its ignored
	if (!ms_Workspace)
	{
		result = ms_SmartIntegrator->ExitInstance();
		return false;
	};

	ms_Project = ms_Workspace->LoadProject(sProjectName);
	if (!ms_Project) 
	{
		result = ms_Integrator->CloseWorkspace(ms_Workspace);
		result = ms_SmartIntegrator->ExitInstance();
		delete ms_SmartIntegrator;
		return false;
	};
	ms_connected = true;
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Closes an Alienbrain connection.  Serves as a wrapper around the Alienbrain XDK
 */
void AlienbrainConnection::shutdownConnection()
{
	FATAL(!ms_installed, ("AlienbrainConnection not installed"));
	bool result = ms_Workspace->UnloadProject(ms_Project);
	
	result = ms_SmartIntegrator->ExitInstance();

	delete ms_SmartIntegrator;
	ms_SmartIntegrator = NULL;
	ms_connected = false;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Store an object to the Alienbrain database.  Imports or checks out/in as needed.
  * Serves as a wrapper around the Alienbrain XDK
 */
bool AlienbrainConnection::storeObject(const CNxNPath& Filename, const CNxNPath& NamespaceFolder, const CNxNString& Comment, const CNxNString& Keywords)
{
	FATAL(!ms_installed, ("AlienbrainConnection not installed"));
	MESSENGER_REJECT(!ms_connected, ("AlienbrainConnection not connected"));
	CNxNPath NamespacePath(NamespaceFolder);
	NamespacePath.StandardizeAsFolder();
	NamespacePath += Filename.GetName();

	if (objectExists(NamespacePath))
	{
		return updateObject(Filename, NamespacePath, Comment);
	}
	return importFile(Filename, NamespaceFolder, Comment, Keywords);
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Set a property on an existing Alienbrain database file.  Serves as a wrapper around the Alienbrain XDK
 */
bool AlienbrainConnection::setProperty(const CNxNString& NamespacePath, const CNxNString& PropertyName, const CNxNString& PropertyValue)
{
	FATAL(!ms_installed, ("AlienbrainConnection not installed"));
	MESSENGER_REJECT(!ms_connected, ("AlienbrainConnection not connected"));
	CNxNPath path(NamespacePath);
	path.StandardizeAsFolder();

	if (!objectExists(path))
		return false;

	CNxNProperty Property(PropertyName, PropertyValue, 0);
	CNxNNode* pNode = ms_Project->FindNode(path);

	if (!pNode || !pNode->IsValid())
		return false;

	return pNode->SetProperty(Property);
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Get a property from an existing Alienbrain database file.  This function also retrieves properties
 * that are inherited from parent objects.  Serves as a wrapper around the Alienbrain XDK
 */
CNxNString AlienbrainConnection::getProperty(const CNxNString& NamespacePath, const CNxNString& PropertyName)
{
	FATAL(!ms_installed, ("AlienbrainConnection not installed"));

	if (!ms_connected)
	{
		MESSENGER_MESSAGE_BOX(NULL, "AlienbrainConnection not connected", "Error", MB_OK);
		return _STR("");
	}

	CNxNPath path(NamespacePath);
	path.StandardizeAsFolder();

	if (!objectExists(path))
		return _STR("");

	CNxNProperty Property(PropertyName, NXNCONST_GETPROP_INHERIT);

	CNxNNode* pNode = ms_Project->FindNode(path);
	if (!pNode || !pNode->IsValid())
		return _STR("");

	if (!pNode->GetProperty(Property))
		return _STR("");

	CNxNString result = Property.GetString();
	if(result.IsNull())
		return _STR("");

	return result;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Tests for object existance in the Alienbrain database.  Serves as a wrapper around the Alienbrain XDK
 */
bool AlienbrainConnection::objectExists(const CNxNString& sNamespacePath)
{
	FATAL(!ms_installed, ("AlienbrainConnection not installed"));
	MESSENGER_REJECT(!ms_connected, ("AlienbrainConnection not connected"));
	CNxNPath path(sNamespacePath);
	path.StandardizeAsFolder();
	if (!ms_Project->FindNode(path))
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Tests for object existance in the Alienbrain database.  Serves as a wrapper around the Alienbrain XDK
 */
bool AlienbrainConnection::objectExistsInDB(const CNxNString& sNamespacePath)
{
	FATAL(!ms_installed, ("AlienbrainConnection not installed"));
	MESSENGER_REJECT(!ms_connected, ("AlienbrainConnection not connected"));

	if(!ms_SmartIntegrator->FileExistsInDatabase(sNamespacePath)) 
	{
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

bool AlienbrainConnection::NewerFileOnServer(const CNxNString& sNamespacePath)
{
	FATAL(!ms_installed, ("AlienbrainConnection not installed"));
	MESSENGER_REJECT(!ms_connected, ("AlienbrainConnection not connected"));

	return ms_SmartIntegrator->NewerFileOnServer(sNamespacePath);
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Checks a file out, updates it with new local data, and checks it back in.
 * Serves as a wrapper around the Alienbrain XDK
 */
bool AlienbrainConnection::updateObject(const CNxNPath& Filename, const CNxNPath& NamespacePath, const CNxNString& Comment)
{
	FATAL(!ms_installed, ("AlienbrainConnection not installed"));
	MESSENGER_REJECT(!ms_connected, ("AlienbrainConnection not connected"));
	if (!checkOutFile(NamespacePath, Comment, true))
		return false;

	CNxNString sLocalPath = getProperty(NamespacePath, "LocalPath");
	if (sLocalPath.IsEmpty())
		return false;

	BOOL bStatus = CopyFile(Filename, sLocalPath, FALSE);

	if (!checkInFile(NamespacePath, Comment))
		return false;

	if (!bStatus)
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Create a folder on the Alienbrain server.  Serves as a wrapper around the Alienbrain XDK
 */
bool AlienbrainConnection::createFolder(const CNxNPath& NamespacePath)
{
	FATAL(!ms_installed, ("AlienbrainConnection not installed"));
	MESSENGER_REJECT(!ms_connected, ("AlienbrainConnection not connected"));
	if (NamespacePath == _STR(""))
		return false;

	CNxNPath ParentPath = NamespacePath.GetParent();
	ParentPath.StandardizeForNamespace();

	if (!ParentPath)
		return false;

	if (!objectExists(ParentPath))
	{
		if (!createFolder(ParentPath))
			return false; 
	}

	CNxNNode* pParent = ms_Project->FindNode(ParentPath); // get the parent node

	if (!pParent || !pParent->IsValid())
		return false;

	CNxNCommand cmdNewFolder(_STR(NXN_COMMAND_NEW_FOLDER), ParentPath, 1, 0);

	bool result = cmdNewFolder.SetAt(0, NamespacePath.GetName());

	if (!pParent->RunCommand(cmdNewFolder)) 
		return false;
	
	UNREF(result);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Import a new file into the Alienbrain database.  Serves as a wrapper around the Alienbrain XDK
 */
bool AlienbrainConnection::importFile(const CNxNString& Filename, const CNxNPath& NamespaceFolder, const CNxNString& Comment, const CNxNString& Keywords) 
{
	FATAL(!ms_installed, ("AlienbrainConnection not installed"));
	MESSENGER_REJECT(!ms_connected, ("AlienbrainConnection not connected"));

	CNxNPath path(NamespaceFolder);
	path.StandardizeAsFolder();

	if (!objectExists(path))
	{
		if (!createFolder(path))
			return false;
	}

	if(!ms_SmartIntegrator->FileExistsOnLocalHardDisk(Filename)) 
	{
		MESSENGER_LOG_ERROR(("%s does not exist on the local drive, can't import\n", static_cast<LPCSTR>(Filename)));
		return false;
	}

	CNxNDbNode* pParent = static_cast<CNxNDbNode*>(ms_Project->FindNode(path));

	if (!pParent || !pParent->IsValid())
	{
		if(pParent)
		{
			MESSENGER_LOG_ERROR(("%s is not a valid Alienbrain node, can't import into this directory\n", static_cast<LPCSTR>(pParent->GetDbPath())));
		}
		else
		{
			MESSENGER_LOG_ERROR(("couldn't get parent directory for %s, can't import\n", static_cast<LPCSTR>(path)));
		}
		return false; 
	}

	if (!pParent->ImportDbNode(Filename, false, Comment, Keywords))
	{
		MESSENGER_LOG_ERROR(("Failed to import %s into the Alienbrain folder %s\n", static_cast<LPCSTR>(Filename), static_cast<LPCSTR>(pParent->GetDbPath())));
		return false; 
	}

	MESSENGER_LOG(("%s imported into Alienbrain successfully\n", static_cast<LPCSTR>(Filename)));
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Add a dependency between two Alienbrain files.  Serves as a wrapper around the Alienbrain XDK
 */
bool AlienbrainConnection::addDependency(const CNxNString& dependsOnFilePath, const CNxNString& isDependedOnFilePath)
{
	FATAL(!ms_installed, ("AlienbrainConnection not installed"));
	MESSENGER_REJECT(!ms_connected, ("AlienbrainConnection not connected"));
	UNREF(dependsOnFilePath);
	UNREF(isDependedOnFilePath);
/*
	CNxNDbNode* nodeA = (CNxNDbNode*) ms_Project->findNode(dependsOnFilePath);

	if (!nodeA || !nodeA->IsValid())
		return false;		

	CNxNDbNode* nodeB = (CNxNDbNode*) ms_Project->findNode(isDependedOnFilePath);

	if (!nodeB || !nodeB->IsValid())
		return false;		
	
	if (!nodeA->DependencyAdd(*nodeB))
		return false;

*/
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Tests if an Alienbrain file is locked by someone else.  Serves as a wrapper around the Alienbrain XDK
 */
bool AlienbrainConnection::nodeIsLockedByAnotherUser(const CNxNNode& Node)
{
	FATAL(!ms_installed, ("AlienbrainConnection not installed"));
	MESSENGER_REJECT(!ms_connected, ("AlienbrainConnection not connected"));
	CNxNProperty Username(_STR(NXN_PROPERTY_USER_NAME), 0);
	CNxNProperty LockedBy(_STR(NXN_PROPERTY_LOCKED_BY), 0);

	bool result = Node.GetProperty(Username);
	if(!result)
		return false;
	
	result = Node.GetProperty(LockedBy);
	if(!result)
		return false;

	return ((LockedBy.GetString() != _STR("")) && (Username.GetString() != LockedBy.GetString()));
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Checks a file out from the Alienbrain database.  Serves as a wrapper around the Alienbrain XDK
 */
bool AlienbrainConnection::checkOutFile(const CNxNPath& NamespacePath, const CNxNString& Comment, const bool bDontGetLocalCopy)
{
	FATAL(!ms_installed, ("AlienbrainConnection not installed"));
	MESSENGER_REJECT(!ms_connected, ("AlienbrainConnection not connected"));

	CNxNPath path(NamespacePath);
	path.StandardizeAsFolder();

	if (!objectExists(path))
		return false;

	CNxNDbNode* pNode = static_cast<CNxNDbNode*>(ms_Project->FindNode(path));

	if (!pNode || !pNode->IsValid())
	{
		if(pNode)
		{
			MESSENGER_LOG_ERROR(("%s is not a valid Alienbrain file node, check-out failed\n", static_cast<LPCSTR>(pNode->GetDbPath())));
		}
		else
		{
			MESSENGER_LOG_ERROR(("couldn't get Alienbrain file node for %s, check-out failed\n", static_cast<LPCSTR>(path)));
		}
		return false;
	}

	if (nodeIsLockedByAnotherUser(*pNode))
	{
		MESSENGER_LOG_ERROR(("%s is already checked out from Alienbrain by someone else, check-out failed\n", static_cast<LPCSTR>(pNode->GetDbPath())));
		return false;
	}

	if (!pNode->CheckOut(false, Comment, bDontGetLocalCopy, false, false, _STR(""), false/*, (EAS removed) false */))
	{
		MESSENGER_LOG_ERROR(("%s failed to check-out from Alienbrain\n", static_cast<LPCSTR>(pNode->GetDbPath())));
		return false;
	}

	MESSENGER_LOG(("%s checked out from Alienbrain successfully\n", static_cast<LPCSTR>(pNode->GetDbPath())));
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Checks a file into the Alienbrain database.  Serves as a wrapper around the Alienbrain XDK
 */
bool AlienbrainConnection::checkInFile(const CNxNPath& NamespacePath, const CNxNString& Comment)
{
	FATAL(!ms_installed, ("AlienbrainConnection not installed"));
	MESSENGER_REJECT(!ms_connected, ("AlienbrainConnection not connected"));
	if (!objectExists(NamespacePath))
		return false;

	CNxNPath path(NamespacePath);
	path.StandardizeAsFolder();

	CNxNDbNode* pNode = static_cast<CNxNDbNode*>(ms_Project->FindNode(path));

	if (!pNode || !pNode->IsValid())
	{
		if(pNode)
		{
			MESSENGER_LOG_ERROR(("%s is not a valid Alienbrain file node, check-in failed\n", static_cast<LPCSTR>(pNode->GetDbPath())));
		}
		else
		{
			MESSENGER_LOG_ERROR(("couldn't get Alienbrain file node for %s, check-in failed\n", static_cast<LPCSTR>(path)));
		}
		return false;
	}

	if (nodeIsLockedByAnotherUser(*pNode))
	{
		MESSENGER_LOG_ERROR(("%s is already checked out from Alienbrain by someone else, check-in failed\n", static_cast<LPCSTR>(pNode->GetDbPath())));
		return false;
	}

	if (!pNode->CheckIn(false, _STR(""), Comment))
	{
		MESSENGER_LOG_ERROR(("%s failed to check-in to Alienbrain\n", static_cast<LPCSTR>(pNode->GetDbPath())));
		return false;
	}

	MESSENGER_LOG(("%s checked in to Alienbrain successfully\n", static_cast<LPCSTR>(pNode->GetDbPath())));
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Build an Alienbrain path from a base Alienbrain directory and a filename.  Serves as a wrapper around the Alienbrain XDK
 */
CNxNString AlienbrainConnection::makeNamespacePath(const CNxNPath& Filename, const CNxNPath& NamespaceFolder)
{
	FATAL(!ms_installed, ("AlienbrainConnection not installed"));
	if (!ms_connected)
	{
		MESSENGER_MESSAGE_BOX(NULL, "AlienbrainConnection not connected", "Error", MB_OK);
		return _STR("");
	}

	CNxNPath NamespacePath(NamespaceFolder);
	NamespacePath.StandardizeAsFolder();
	NamespacePath += Filename.GetName();
	return NamespacePath;
}

//////////////////////////////////////////////////////////////////////////////////

CNxNNode *AlienbrainConnection::findNode(const CNxNPath &NamespacePath)
{
	FATAL(!ms_installed, ("AlienbrainConnection not installed"));
	if (!ms_connected)
	{
		MESSENGER_MESSAGE_BOX(NULL, "AlienbrainConnection not connected", "Error", MB_OK);
		return NULL;
	}
	return ms_Project->FindNode(NamespacePath);
}

#endif
// JU_TODO: end alienbrain def out
//////////////////////////////////////////////////////////////////////////////////
