// ======================================================================
//
// AlienbrainConnection.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef ALIENBRAINCONNECTION_H
#define ALIENBRAINCONNECTION_H

// JU_TODO: alienbrain def out
#if 0
#include <stdio.h>

#pragma warning(push, 3) //NxN imports have some issues at warning level 4, so give them a break
#include <NxNIntegratorSDK.h>
#include <NxNSmartIntegrator.h>
#pragma warning(pop)    //restore our iron-fisted warning level

// ======================================================================

class Messenger;

// ======================================================================


/**
 * This class wraps the Alienbrain API, making calls into it simpler and cleaner.
 */
class AlienbrainConnection
{
	public:
		static void install(Messenger* newMessenger);
		static void remove();

		static bool       initConnection(const CNxNString& sProjectName);
		static void       shutdownConnection();
		static bool       storeObject(const CNxNPath& Filename, const CNxNPath& NamespaceFolder, const CNxNString& Comment, const CNxNString& Keywords);
		static bool       setProperty(const CNxNString& NamespacePath, const CNxNString& PropertyName, const CNxNString& PropertyValue);
		static CNxNString getProperty(const CNxNString& NamespacePath, const CNxNString& PropertyName);
		static CNxNString makeNamespacePath(const CNxNPath& Filename, const CNxNPath& NamespaceFolder);
		static bool       addDependency(const CNxNString& dependsOnFilePath, const CNxNString& isDependedOnFilePath);
		static bool       objectExists(const CNxNString& sNamespacePath);
		static bool       updateObject(const CNxNPath& Filename, const CNxNPath& NamespacePath, const CNxNString& Comment);
		static bool       createFolder(const CNxNPath& NamespacePath);
		static bool       importFile(const CNxNString& Filename, const CNxNPath& NamespaceFolder, const CNxNString& Comment, const CNxNString& Keywords);
		static bool       nodeIsLockedByAnotherUser( const CNxNNode& Node);
		static bool       checkOutFile(const CNxNPath& NamespacePath, const CNxNString& Comment = _STR( "" ), const bool bDontGetLocalCopy = false);
		static bool       checkInFile(const CNxNPath& NamespacePath, const CNxNString& Comment = _STR( "" ));
		static CNxNNode*  findNode(const CNxNPath& NamespacePath);
		static bool       objectExistsInDB(const CNxNString& sNamespacePath);
		static bool 		  NewerFileOnServer(const CNxNString& sNamespacePath);

	private:
		//Importer data
		static CNxNSmartIntegrator* ms_SmartIntegrator;
		static CNxNIntegrator*      ms_Integrator;
		static CNxNWorkspace*       ms_Workspace;
		static CNxNProject*         ms_Project;
		static bool                 ms_installed;
		static bool                 ms_connected;


		///MUST be named messenger for #define's, can't be called ms_messenger (see Messenger.h)
		static Messenger* messenger;
};

// ======================================================================
#endif
// JU_TODO: end alienbrain def out

#endif //ALIENBRAINCONNECTION_H
