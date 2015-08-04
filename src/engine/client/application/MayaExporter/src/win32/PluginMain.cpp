// ======================================================================
//
// PluginMain.cpp
// Portions copyright 1998 Bootprint Entertainment.
// Portions copyright 2000-2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "PluginMain.h"
#include "maya/MEventMessage.h"
#include "maya/MSceneMessage.h"


#include "sharedCollision/BoxTree.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/RegistryKey.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedImage/SetupSharedImage.h"
#include "sharedMath/SetupSharedMath.h"
#include "sharedObject/SetupSharedObject.h"
#include "sharedPathfinding/SetupSharedPathfinding.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedStatusWindow/LogWindow.h"
#include "sharedStatusWindow/SetupSharedStatusWindow.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedCollision/SimpleExtent.h"

#include "clientGraphics/SetupClientGraphics.h"

#include "gumi_atoi.h"

#include "AddAnimationCommand.h"
#include "AlienbrainConnection.h"
#include "AlienbrainImporter.h"
#include "AnimationMessageCollector.h"
#include "CreateTransformMask.h"
#if !NO_DATABASE
#include "DatabaseImporter.h"
#endif
#include "DeleteAnimationCommand.h"
#include "ExportCommand.h"
#include "ExportKeyframeSkeletalAnimation.h"
#include "ExportSkeletalAppearanceTemplate.h"
#include "ExportSkeletalMeshGenerator.h"
#include "ExportSkeleton.h"
#include "ExportStaticMesh.h"
#include "ExporterLog.h"
#include "ExportManager.h"
#include "LogWindowMessenger.h"
#include "MayaAnimatingTextureShaderTemplateWriter.h"
#include "MayaAnimationList.h"
#include "MayaCustomizableShaderTemplateWriter.h"
#include "MayaLightMeshReader.h"
#include "MayaMeshReader.h"
#include "MayaMeshWeighting.h"
#include "MayaMisc.h"
#include "MayaOwnerProxyShaderTemplateWriter.h"
#include "MayaPerPixelLighting.h"
#include "MayaShaderTemplateBuilder.h"
#include "MayaStaticShaderTemplateWriter.h"
#include "MeshBuilder.h"
#include "OccludedFaceMapGenerator.h"
#include "PerforceImporter.h"
#include "ReexportCommand.h"
#include "SetAuthorCommand.h"
#include "SetBaseDirectory.h"
#include "SetDirectoryCommand.h"
#include "VersionFile.h"
#include "VertexIndexer.h"
#include "VisitAnimationCommand.h"

#pragma warning(push,3)
#include "maya/MFnPlugin.h"
#pragma warning(pop)

#include <time.h>

#pragma warning (disable: 4505)

// ======================================================================

namespace PluginMainNamespace
{
	static const char *const  APPEARANCE_WRITE_DIR_KEY                = "AppearanceWriteDir";
	static const char *const  DEFAULT_APPEARANCE_WRITE_DIR            = "\\exported\\appearance\\";

	static const char *const  SAT_WRITE_DIR_KEY                       = "SatWriteDir";
	static const char *const  DEFAULT_SAT_WRITE_DIR                   = "\\exported\\appearance\\";

	static const char *const  SHADER_TEMPLATE_WRITE_DIR_KEY           = "ShaderTemplateWriteDir";
	static const char *const  DEFAULT_SHADER_TEMPLATE_WRITE_DIR       = "\\exported\\shader\\";

	static const char *const  SHADER_TEMPLATE_REF_DIR_KEY             = "ShaderTemplateReferenceDir";
	static const char *const  DEFAULT_SHADER_TEMPLATE_REF_DIR         = "shader/";

	static const char *const  EFFECT_REF_DIR_KEY                      = "EffectReferenceDir";
	static const char *const  DEFAULT_EFFECT_REF_DIR                  = "effect/";

	static const char *const  TEXTURE_REF_DIR_KEY                     = "TextureReferenceDir";
	static const char *const  DEFAULT_TEXTURE_REF_DIR                 = "texture/";

	static const char *const  TEXTURE_WRITE_DIR_KEY                   = "TextureWriteDir";
	static const char *const  DEFAULT_TEXTURE_WRITE_DIR               = "\\exported\\texture\\";

	static const char *const  TEXTURE_RENDERER_REF_DIR_KEY            = "TextureRendererReferenceDir";
	static const char *const  DEFAULT_TEXTURE_RENDERER_REF_DIR        = "texturerenderer/";

	static const char *const  ANIMATION_WRITE_DIR_KEY                 = "AnimationWriteDir";
	static const char *const  DEFAULT_ANIMATION_WRITE_DIR             = "\\exported\\appearance\\animation\\";

	static const char *const  LOG_WINDOW_PLACEMENT_KEY                = "LogWindowPlacement";

	static const char *const  AUTHOR_KEY                              = "Author";
	static const char *const  DEFAULT_AUTHOR                          = "asommers";

	static const char *const  ACTIVE_DRIVE_KEY                        = "ActiveDrive";
	static const char *const  ACTIVE_DRIVE_VALUE                      = "c:\\";

	static const char *const  SKELETON_TEMPLATE_WRITE_DIR_KEY         = "SkeletonTemplateWriteDir";
	static const char *const  DEFAULT_SKELETON_TEMPLATE_WRITE_DIR     = "\\exported\\appearance\\skeleton\\";

	static const char *const  SKELETON_TEMPLATE_REFERENCE_DIR_KEY     = "SkeletonTemplateReferenceDir";
	static const char *const  DEFAULT_SKELETON_TEMPLATE_REFERENCE_DIR = "appearance/skeleton/";

	static const char *const  LOG_DIR_KEY                             = "LogDir";
	static const char *const  DEFAULT_LOG_DIR                         = "\\exported\\log\\";

	static const char *const  APPEARANCE_REFERENCE_DIR_KEY            = "AppearanceReferenceDir";
	static const char *const  DEFAULT_APPEARANCE_REFERENCE_DIR        = "appearance/";

	static const char *const  ASSET_DB_NAME_KEY                       = "AssetDatabaseName";
	static const char *const  DEFAULT_ASSET_DB_NAME                   = "swodb";

	static const char *const  ASSET_DB_LOGINNAME_KEY                  = "AssetDatabaseLogin";
	static const char *const  DEFAULT_ASSET_DB_LOGINNAME              = "asset";

	static const char *const  ASSET_DB_PASSWORD_KEY                   = "AssetDatabasePassword";
	static const char *const  DEFAULT_ASSET_DB_PASSWORD               = "asset";

	static const char *const  ASSET_DB_ACTIVATED_KEY                  = "AssetDatabaseActivated";
	static const char *const  DEFAULT_ASSET_DB_ACTIVATED              = "true";

	static const char *const  VIEWER_LOCATION_KEY                     = "Viewer";
	static const char *const  DEFAULT_VIEWER_LOCATION                 = "Enter full path to viewer.exe here (c:\\swg\\current\\exe\\win32\\viewer.exe)";

	static const char *const  PERFORCE_BRANCH_LIST_KEY                = "PerforceBranchList";
	static const char *const  DEFAULT_PERFORCE_BRANCH_LIST            = "current,x1,s0,test";

	static const char *const  ALIENBRAIN_PROJECT_NAME_KEY             = "AlienbrainProjectName";
	static const char *const  DEFAULT_ALIENBRAIN_PROJECT_NAME         = "swg";

	static const char *const  BOOTPRINT_REGISTRY_KEY                  = "Software\\Bootprint\\StaticMeshExporter";
	static const char *const  SOE_REGISTRY_KEY                        = "Software\\SOE\\MayaExporter";

	static LogWindowMessenger  theMessenger;
	static Messenger          *messenger;                //this name is necessary for the MESSENGER_* macros
	static bool                s_engineStopped;

	static bool                s_useStatusWindow = true; //NOTE: to enable/disable the status window you must also modify USE_STATUS_WINDOW in Messenger.h

	static const int           cs_defaultMemorySize = 128;
	static int                 s_allocatedMemory = cs_defaultMemorySize;
}

using namespace PluginMainNamespace;

// ======================================================================

static bool StartEngine ()
{
	//-- setup shared
	{
		SetupSharedThread::install();

		SetupSharedDebug::install(4096);

		SetupSharedFoundation::Data setupFoundationData (SetupSharedFoundation::Data::D_console);
		SetupSharedFoundation::install (setupFoundationData);

		SetupSharedCompression::install();

		SetupSharedFile::install(false);

		SetupSharedRandom::install(static_cast<uint32>(time(NULL)));
		if(s_useStatusWindow)
			SetupSharedStatusWindow::install (GetPluginInstanceHandle());

		SetupSharedMath::install();

		SetupSharedImage::Data setupImageData;
		SetupSharedImage::setupDefaultData(setupImageData);
		SetupSharedImage::install(setupImageData);

		SetupSharedObject::Data setupSharedObjectData;
		SetupSharedObject::setupDefaultConsoleData (setupSharedObjectData);
		setupSharedObjectData.ensureDefaultAppearanceExists = false;
		SetupSharedObject::install (setupSharedObjectData);
		
		SetupSharedPathfinding::install();
	}

	//-- setup client
	{
		SetupClientGraphics::Data setupGraphicsData;
		SetupClientGraphics::setupDefaultConsoleData (setupGraphicsData);
		SetupClientGraphics::install (setupGraphicsData);
	}

	TreeFile::addSearchAbsolute (0);

	if(s_useStatusWindow)
	{
		// install components
		if (!MayaUtility::install ("SOE MayaExporter", &theMessenger))
		{
			SetupSharedFoundation::remove ();
			return false;
		}
	}

	//-- setup the directories
	SetDirectoryCommand::install(&theMessenger);
	
	// -TRF-
	//-- NOTE: this is an order-dependent operation.  to fix this, all the
	//         legacy code referencing *_DIR_INDEX needs to be changed to
	//         reference an int var or a #define const char* to work
	//         without this order dependence.
	// -EAS- note that there should be one call here for each entry in the directory index enum in the .h file
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("appearance write directory") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("shader template write directory") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("shader template reference directory") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("effect reference directory") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("texture reference directory") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("texture write directory") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("texture renderer reference directory") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("animation write directory") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("author") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("skeleton template write directory") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("skeleton template reference directory") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("log write directory") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("sat write directory") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("appearance reference directory") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("assetdb name") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("assetdb login name") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("assetdb login password") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("assetdb activated state") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("viewer location") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("perforce branch list") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("alienbrain project name") );
	IGNORE_RETURN( SetDirectoryCommand::registerDirectory("active drive") );

	VertexIndexer::Vertex::install();
	MayaLightMeshReader::install(&theMessenger);	
	MayaMeshReader::install(&theMessenger);	
	ExportStaticMesh::install(&theMessenger);
	MeshBuilder::install(&theMessenger);
	ExportSkeleton::install(&theMessenger);
	MayaMeshWeighting::install(&theMessenger);
	ExportSkeletalAppearanceTemplate::install(&theMessenger);
	ExportSkeletalMeshGenerator::install(&theMessenger);
	ExportKeyframeSkeletalAnimation::install(&theMessenger);
	MayaMisc::install(&theMessenger);
// JU_TODO: alienbrain def out
#if 0
	AlienbrainImporter::install(&theMessenger);
#endif
// JU_TODO: end alienbrain def out
	SetBaseDirectory::install(&theMessenger);
	MayaAnimationList::install(&theMessenger);
	AddAnimationCommand::install(&theMessenger);
	VisitAnimationCommand::install(&theMessenger);
	ReexportCommand::install(&theMessenger);
	DeleteAnimationCommand::install(&theMessenger);
	SetAuthorCommand::install(&theMessenger);
	ExporterLog::install(&theMessenger);
// JU_TODO: alienbrain def out
#if 0
	AlienbrainConnection::install(&theMessenger);
#endif
// JU_TODO: end alienbrain def out
	PerforceImporter::install(&theMessenger);
	ExportCommand::install(&theMessenger);
	OccludedFaceMapGenerator::install(&theMessenger);
#if !NO_DATABASE
	DatabaseImporter::install(&theMessenger);
#endif
	MayaShaderTemplateBuilder::install(&theMessenger);
	MayaStaticShaderTemplateWriter::install(&theMessenger);
	MayaCustomizableShaderTemplateWriter::install(&theMessenger);
	MayaOwnerProxyShaderTemplateWriter::install(&theMessenger);
	MayaAnimatingTextureShaderTemplateWriter::install(&theMessenger);
	MayaPerPixelLighting::install(&theMessenger);
	AnimationMessageCollector::install(&theMessenger);
	CreateTransformMask::install(&theMessenger);
	BoxTree::install();
	ExportManager::install(&theMessenger);
	SimpleExtent::install();

	return true;
}

// ----------------------------------------------------------------------

void StopEngine(bool isCleanShutdown)
{
	//-- Ignore multiple calls to this function.
	if (s_engineStopped)
		return;

	if(s_useStatusWindow)
	{
		if(!isCleanShutdown)
			LogWindow::closeThreads();
	}
	else
	{
		UNREF(isCleanShutdown);
	}

	//-- remove in reverse order of install.
	// -TRF- modify to use ExitChain.

	ExportManager::remove();
	// BoxTree uses exit chain
	CreateTransformMask::remove();
	// AnimationMessageCollector uses exit chain
	MayaPerPixelLighting::remove();
	MayaAnimatingTextureShaderTemplateWriter::remove();
	MayaOwnerProxyShaderTemplateWriter::remove();
	MayaCustomizableShaderTemplateWriter::remove();
	MayaStaticShaderTemplateWriter::remove();
	MayaShaderTemplateBuilder::remove();
	
#if !NO_DATABASE
	DatabaseImporter::remove();
#endif
	//OccludedFaceMapGenerator uses exit chain
	ExportCommand::remove();
	PerforceImporter::remove();
	ExporterLog::remove();
	SetAuthorCommand::remove();
	DeleteAnimationCommand::remove();
	ReexportCommand::remove();
	
	VisitAnimationCommand::remove();
	AddAnimationCommand::remove();
	MayaAnimationList::remove();
	SetBaseDirectory::remove();
// JU_TODO: alienbrain def out
#if 0
	AlienbrainImporter::remove();
	AlienbrainConnection::remove();
#endif
// JU_TODO: end alienbrain def out
	MayaMisc::remove();
	ExportKeyframeSkeletalAnimation::remove();
	ExportSkeletalMeshGenerator::remove();
	ExportSkeletalAppearanceTemplate::remove();
	MayaMeshWeighting::remove();
	ExportSkeleton::remove();
	
	MeshBuilder::remove();
	ExportStaticMesh::remove();
	MayaMeshReader::remove();
	MayaLightMeshReader::remove();	

	SetDirectoryCommand::remove();

	MayaUtility::remove ();

	SetupSharedPathfinding::remove();

	SetupSharedFoundation::remove ();
	SetupSharedThread::remove();

	s_engineStopped = true;
}

// ----------------------------------------------------------------------

namespace
{
	void convertBackSlashesToFrontSlashes (char* const input)
	{
		char* current = input;

		while (*current)
		{
			if (*current == '\\')
				*current = '/';

			++current;
		}
	}
}

// ----------------------------------------------------------------------

static void WriteRegistrySettings ()
{
	const char * directory = NULL;

	RegistryKey * const registryKey = RegistryKey::getLocalMachineKey ()->createSubkey (SOE_REGISTRY_KEY);

	// store values for paths
	directory = SetDirectoryCommand::getDirectoryString(APPEARANCE_WRITE_DIR_INDEX);
	registryKey->setValue (APPEARANCE_WRITE_DIR_KEY, directory, strlen (directory) + 1, REG_SZ);

	directory = SetDirectoryCommand::getDirectoryString(APPEARANCE_REFERENCE_DIR_INDEX);
	registryKey->setValue (APPEARANCE_REFERENCE_DIR_KEY, directory, strlen (directory) + 1, REG_SZ);

	directory = SetDirectoryCommand::getDirectoryString(SHADER_TEMPLATE_WRITE_DIR_INDEX);
	registryKey->setValue (SHADER_TEMPLATE_WRITE_DIR_KEY, directory, strlen (directory) + 1, REG_SZ);

	directory = SetDirectoryCommand::getDirectoryString(SHADER_TEMPLATE_REFERENCE_DIR_INDEX);
	registryKey->setValue (SHADER_TEMPLATE_REF_DIR_KEY, directory, strlen (directory) + 1, REG_SZ);

	directory = SetDirectoryCommand::getDirectoryString(EFFECT_REFERENCE_DIR_INDEX);
	registryKey->setValue (EFFECT_REF_DIR_KEY, directory, strlen (directory) + 1, REG_SZ);

	directory = SetDirectoryCommand::getDirectoryString(TEXTURE_REFERENCE_DIR_INDEX);
	registryKey->setValue (TEXTURE_REF_DIR_KEY, directory, strlen (directory) + 1, REG_SZ);

	directory = SetDirectoryCommand::getDirectoryString(TEXTURE_WRITE_DIR_INDEX);
	registryKey->setValue (TEXTURE_WRITE_DIR_KEY, directory, strlen (directory) + 1, REG_SZ);

	directory = SetDirectoryCommand::getDirectoryString(TEXTURE_RENDERER_REFERENCE_DIR_INDEX);
	registryKey->setValue (TEXTURE_RENDERER_REF_DIR_KEY, directory, strlen (directory) + 1, REG_SZ);

	directory = SetDirectoryCommand::getDirectoryString(ANIMATION_WRITE_DIR_INDEX);
	registryKey->setValue (ANIMATION_WRITE_DIR_KEY, directory, strlen (directory) + 1, REG_SZ);

	directory = SetDirectoryCommand::getDirectoryString(AUTHOR_INDEX);
	registryKey->setValue (AUTHOR_KEY, directory, strlen (directory) + 1, REG_SZ);

	directory = SetDirectoryCommand::getDirectoryString(ACTIVE_DRIVE_INDEX);
	registryKey->setValue (ACTIVE_DRIVE_KEY, directory, strlen(directory) + 1, REG_SZ);

	directory = SetDirectoryCommand::getDirectoryString(SKELETON_TEMPLATE_WRITE_DIR_INDEX);
	registryKey->setValue (SKELETON_TEMPLATE_WRITE_DIR_KEY, directory, strlen (directory) + 1, REG_SZ);

	directory = SetDirectoryCommand::getDirectoryString(SKELETON_TEMPLATE_REFERENCE_DIR_INDEX);
	registryKey->setValue (SKELETON_TEMPLATE_REFERENCE_DIR_KEY, directory, strlen (directory) + 1, REG_SZ);

	directory = SetDirectoryCommand::getDirectoryString(LOG_DIR_INDEX);
	registryKey->setValue (LOG_DIR_KEY, directory, strlen (directory) + 1, REG_SZ);

	directory = SetDirectoryCommand::getDirectoryString(SAT_WRITE_DIR_INDEX);
	registryKey->setValue (SAT_WRITE_DIR_KEY, directory, strlen (directory) + 1, REG_SZ);
	
	directory = SetDirectoryCommand::getDirectoryString(ASSET_DB_NAME_DIR_INDEX);
	registryKey->setValue (ASSET_DB_NAME_KEY, directory, strlen (directory) + 1, REG_SZ);
	
	directory = SetDirectoryCommand::getDirectoryString(ASSET_DB_LOGINNAME_DIR_INDEX);
	registryKey->setValue (ASSET_DB_LOGINNAME_KEY, directory, strlen (directory) + 1, REG_SZ);

	directory = SetDirectoryCommand::getDirectoryString(ASSET_DB_PASSWORD_DIR_INDEX);
	registryKey->setValue (ASSET_DB_PASSWORD_KEY, directory, strlen (directory) + 1, REG_SZ);

	directory = SetDirectoryCommand::getDirectoryString(ASSET_DB_ACTIVATED_INDEX);
	registryKey->setValue (ASSET_DB_ACTIVATED_KEY, directory, strlen (directory) + 1, REG_SZ);

	directory = SetDirectoryCommand::getDirectoryString(VIEWER_LOCATION_INDEX);
	registryKey->setValue (VIEWER_LOCATION_KEY, directory, strlen(directory) + 1, REG_SZ);

	//version 2.194 moves the registry from a location in current_users to a location in local_machine.  There is also going to be a registry patch to that
	//  location.  If the patch happened before the user opened the exporter (and migrated their settings), then the old value would overwrite the patch.
	//  Once the dust settles with (say, version 2.196 or later) this functionality can be removed.
	char directoryBuffer[MAX_PATH];
	registryKey->getStringValue (PERFORCE_BRANCH_LIST_KEY, "", directoryBuffer, sizeof (directoryBuffer), true);
	if(strcmp(directoryBuffer, DEFAULT_PERFORCE_BRANCH_LIST) == 0)
	{
		//DO NOTHING, the current value in the new location is current as of 2.194, do NOT overwrite it with an older version from the old registry location)
	}
	else
	{
		//else save out the perforce key as normal
		directory = SetDirectoryCommand::getDirectoryString(PERFORCE_BRANCH_LIST_INDEX);
		registryKey->setValue (PERFORCE_BRANCH_LIST_KEY, directory, strlen (directory) + 1, REG_SZ);
	}

	directory = SetDirectoryCommand::getDirectoryString(ALIENBRAIN_PROJECT_NAME_INDEX);
	registryKey->setValue (ALIENBRAIN_PROJECT_NAME_KEY, directory, strlen (directory) + 1, REG_SZ);
}

// ----------------------------------------------------------------------

static bool LoadRegistrySettings ()
{
	// retrieve values for paths
	bool result;

	// get placement
	WINDOWPLACEMENT  wp;
	DWORD            placementSize;
	DWORD            placementType;
	bool             placementExist;

	// look in the old place first
	RegistryKey * const userRegistryKey = RegistryKey::getCurrentUserKey ()->createSubkey (BOOTPRINT_REGISTRY_KEY);
	if (!userRegistryKey)
	{
		REPORT_LOG (true, ("ERROR: unable to create current user registry key Software\\Bootprint\\StaticMeshExporter\n"));
		return false;
	}

	wp.length = sizeof (WINDOWPLACEMENT);
	userRegistryKey->getValueInfo(LOG_WINDOW_PLACEMENT_KEY, &placementExist, &placementSize, &placementType);
	if (placementExist && (placementType == REG_BINARY) && (placementSize == sizeof (WINDOWPLACEMENT)))
	{
		// get log window placement data
		userRegistryKey->getValue (LOG_WINDOW_PLACEMENT_KEY, &wp, sizeof (WINDOWPLACEMENT), &placementSize);
		if(s_useStatusWindow)
		{
			LogWindow::setWindowPlacement (&wp);
		}
	}

	char appearanceWriteDirectory [MAX_PATH];
	char shaderTemplateWriteDirectory [MAX_PATH];
	char shaderTemplateReferenceDirectory [MAX_PATH];
	char effectReferenceDirectory [MAX_PATH];
	char textureReferenceDirectory [MAX_PATH];
	char textureWriteDirectory [MAX_PATH];
	char textureReferenceRendererDirectory [MAX_PATH];
	char animationWriteDirectory [MAX_PATH];
	char author [MAX_PATH];
	char skeletonTemplateWriteDirectory [MAX_PATH];
	char skeletonTemplateReferenceDirectory [MAX_PATH];
	char logDirectory [MAX_PATH];
	char appearanceReferenceDirectory [MAX_PATH];
	char satWriteDirectory [MAX_PATH];
	char assetDBName [MAX_PATH];
	char assetDBLogin[MAX_PATH];
	char assetDBPassword [MAX_PATH];
	char assetDBActivated [MAX_PATH];
	char viewerLocation[MAX_PATH];
	char perforceBranchList [MAX_PATH];
	char alienbrainProjectName [MAX_PATH];
	char activeDrive [MAX_PATH];

	// appearance write dir
	result = userRegistryKey->getStringValue (APPEARANCE_WRITE_DIR_KEY, DEFAULT_APPEARANCE_WRITE_DIR, appearanceWriteDirectory, sizeof (appearanceWriteDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize appearance write directory\n"));
	SetDirectoryCommand::setDirectoryString(APPEARANCE_WRITE_DIR_INDEX, appearanceWriteDirectory);

	// shader template write dir
	result = userRegistryKey->getStringValue (SHADER_TEMPLATE_WRITE_DIR_KEY, DEFAULT_SHADER_TEMPLATE_WRITE_DIR, shaderTemplateWriteDirectory, sizeof (shaderTemplateWriteDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize shader template write directory\n"));
	SetDirectoryCommand::setDirectoryString(SHADER_TEMPLATE_WRITE_DIR_INDEX, shaderTemplateWriteDirectory);

	// shader template ref dir
	result = userRegistryKey->getStringValue (SHADER_TEMPLATE_REF_DIR_KEY, DEFAULT_SHADER_TEMPLATE_REF_DIR, shaderTemplateReferenceDirectory, sizeof (shaderTemplateReferenceDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize shader template reference directory\n"));
	convertBackSlashesToFrontSlashes (shaderTemplateReferenceDirectory);
	SetDirectoryCommand::setDirectoryString(SHADER_TEMPLATE_REFERENCE_DIR_INDEX, shaderTemplateReferenceDirectory);

	// effect ref dir
	result = userRegistryKey->getStringValue (EFFECT_REF_DIR_KEY, DEFAULT_EFFECT_REF_DIR, effectReferenceDirectory, sizeof (effectReferenceDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize effect reference directory\n"));
	convertBackSlashesToFrontSlashes (effectReferenceDirectory);
	SetDirectoryCommand::setDirectoryString(EFFECT_REFERENCE_DIR_INDEX, effectReferenceDirectory);

	// texture ref dir
	result = userRegistryKey->getStringValue (TEXTURE_REF_DIR_KEY, DEFAULT_TEXTURE_REF_DIR, textureReferenceDirectory, sizeof (textureReferenceDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize texture reference directory\n"));
	convertBackSlashesToFrontSlashes (textureReferenceDirectory);
	SetDirectoryCommand::setDirectoryString(TEXTURE_REFERENCE_DIR_INDEX, textureReferenceDirectory);

	// texture write dir
	result = userRegistryKey->getStringValue (TEXTURE_WRITE_DIR_KEY, DEFAULT_TEXTURE_WRITE_DIR, textureWriteDirectory, sizeof (textureWriteDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize texture write directory\n"));
	SetDirectoryCommand::setDirectoryString(TEXTURE_WRITE_DIR_INDEX, textureWriteDirectory);

	// texture renderer ref dir
	result = userRegistryKey->getStringValue (TEXTURE_RENDERER_REF_DIR_KEY, DEFAULT_TEXTURE_RENDERER_REF_DIR, textureReferenceRendererDirectory, sizeof (textureReferenceRendererDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize texture renderer reference directory\n"));
	convertBackSlashesToFrontSlashes (textureReferenceRendererDirectory);
	SetDirectoryCommand::setDirectoryString(TEXTURE_RENDERER_REFERENCE_DIR_INDEX, textureReferenceRendererDirectory);

	// animation write dir
	result = userRegistryKey->getStringValue (ANIMATION_WRITE_DIR_KEY, DEFAULT_ANIMATION_WRITE_DIR, animationWriteDirectory, sizeof (animationWriteDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize animation write directory\n"));
	SetDirectoryCommand::setDirectoryString(ANIMATION_WRITE_DIR_INDEX, animationWriteDirectory);

	// author
	result = userRegistryKey->getStringValue (AUTHOR_KEY, DEFAULT_AUTHOR, author, sizeof (author), true);
	MESSENGER_REJECT (!result, ("failed to initialize author\n"));
	SetDirectoryCommand::setDirectoryString(AUTHOR_INDEX, author);

	// active drive
	result = userRegistryKey->getStringValue (ACTIVE_DRIVE_KEY, ACTIVE_DRIVE_VALUE, activeDrive, sizeof (activeDrive), true);
	MESSENGER_REJECT (!result, ("failed to initialize active drive\n"));
	SetDirectoryCommand::setDirectoryString(ACTIVE_DRIVE_INDEX, activeDrive);

	// skeleton template write dir
	result = userRegistryKey->getStringValue (SKELETON_TEMPLATE_WRITE_DIR_KEY, DEFAULT_SKELETON_TEMPLATE_WRITE_DIR, skeletonTemplateWriteDirectory, sizeof (skeletonTemplateWriteDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize skeleton template write directory\n"));
	SetDirectoryCommand::setDirectoryString(SKELETON_TEMPLATE_WRITE_DIR_INDEX, skeletonTemplateWriteDirectory);

	// skeleton template ref dir
	result = userRegistryKey->getStringValue (SKELETON_TEMPLATE_REFERENCE_DIR_KEY, DEFAULT_SKELETON_TEMPLATE_REFERENCE_DIR, skeletonTemplateReferenceDirectory, sizeof (skeletonTemplateReferenceDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize skeleton template reference directory\n"));
	convertBackSlashesToFrontSlashes (skeletonTemplateReferenceDirectory);
	SetDirectoryCommand::setDirectoryString(SKELETON_TEMPLATE_REFERENCE_DIR_INDEX, skeletonTemplateReferenceDirectory);

	// log dir
	result = userRegistryKey->getStringValue (LOG_DIR_KEY, DEFAULT_LOG_DIR, logDirectory, sizeof (logDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize log directory\n"));
	SetDirectoryCommand::setDirectoryString(LOG_DIR_INDEX, logDirectory);

	// appearance reference dir
	result = userRegistryKey->getStringValue (APPEARANCE_REFERENCE_DIR_KEY, DEFAULT_APPEARANCE_REFERENCE_DIR, appearanceReferenceDirectory, sizeof (appearanceReferenceDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize log directory\n"));
	convertBackSlashesToFrontSlashes (appearanceReferenceDirectory);
	SetDirectoryCommand::setDirectoryString(APPEARANCE_REFERENCE_DIR_INDEX, appearanceReferenceDirectory);

	//-- sat file write dir
	result = userRegistryKey->getStringValue (SAT_WRITE_DIR_KEY, "", satWriteDirectory, sizeof (satWriteDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize appearance write directory\n"));
	if (!satWriteDirectory[0])
	{
		// default to appearance write directory
		strcpy(satWriteDirectory, SetDirectoryCommand::getDirectoryString(APPEARANCE_WRITE_DIR_INDEX));
	}
	SetDirectoryCommand::setDirectoryString(SAT_WRITE_DIR_INDEX, satWriteDirectory);

	// asset db name
	result = userRegistryKey->getStringValue (ASSET_DB_NAME_KEY, DEFAULT_ASSET_DB_NAME, assetDBName, sizeof (assetDBName), true);
	MESSENGER_REJECT (!result, ("failed to initialize asset database name\n"));
	SetDirectoryCommand::setDirectoryString(ASSET_DB_NAME_DIR_INDEX, assetDBName);
	
	// asset db loginname
	result = userRegistryKey->getStringValue (ASSET_DB_LOGINNAME_KEY, DEFAULT_ASSET_DB_LOGINNAME, assetDBLogin, sizeof (assetDBLogin), true);
	MESSENGER_REJECT (!result, ("failed to initialize asset database login name\n"));
	SetDirectoryCommand::setDirectoryString(ASSET_DB_LOGINNAME_DIR_INDEX, assetDBLogin);

	// asset db password
	result = userRegistryKey->getStringValue (ASSET_DB_PASSWORD_KEY, DEFAULT_ASSET_DB_PASSWORD, assetDBPassword, sizeof (assetDBPassword), true);
	MESSENGER_REJECT (!result, ("failed to initialize asset database password\n"));
	SetDirectoryCommand::setDirectoryString(ASSET_DB_PASSWORD_DIR_INDEX, assetDBPassword);

	// asset db activated state
	result = userRegistryKey->getStringValue (ASSET_DB_ACTIVATED_KEY, DEFAULT_ASSET_DB_ACTIVATED, assetDBActivated, sizeof (assetDBActivated), true);
	MESSENGER_REJECT (!result, ("failed to initialize asset database activated state\n"));
	SetDirectoryCommand::setDirectoryString(ASSET_DB_ACTIVATED_INDEX, assetDBActivated);
#if !NO_DATABASE
	if(std::string(assetDBActivated) == "false")
		DatabaseImporter::activate(false, false);
	else
		DatabaseImporter::activate(true, false);
#endif

	// p4 branch list
	result = userRegistryKey->getStringValue (PERFORCE_BRANCH_LIST_KEY, DEFAULT_PERFORCE_BRANCH_LIST, perforceBranchList, sizeof (perforceBranchList), true);
	MESSENGER_REJECT (!result, ("failed to initialize perforce branch list\n"));
	SetDirectoryCommand::setDirectoryString(PERFORCE_BRANCH_LIST_INDEX, perforceBranchList);
	ExportManager::setValidBranchesPacked(perforceBranchList);

	//alienbrain project name
	result = userRegistryKey->getStringValue (ALIENBRAIN_PROJECT_NAME_KEY, DEFAULT_ALIENBRAIN_PROJECT_NAME, alienbrainProjectName, sizeof (alienbrainProjectName), true);
	MESSENGER_REJECT (!result, ("failed to initialize Alienbrain projects name\n"));
	SetDirectoryCommand::setDirectoryString(ALIENBRAIN_PROJECT_NAME_INDEX, alienbrainProjectName);

	//------

	//now override with data from the new location

	RegistryKey * const localMachineRegistryKey = RegistryKey::getLocalMachineKey ()->createSubkey (SOE_REGISTRY_KEY);
	if (!userRegistryKey)
	{
		REPORT_LOG (true, ("ERROR: unable to create current user registry key Software\\SOE\\MayaExporter\n"));
		return false;
	}

	wp.length = sizeof (WINDOWPLACEMENT);
	localMachineRegistryKey->getValueInfo(LOG_WINDOW_PLACEMENT_KEY, &placementExist, &placementSize, &placementType);
	if (placementExist && (placementType == REG_BINARY) && (placementSize == sizeof (WINDOWPLACEMENT)))
	{
		// get log window placement data
		localMachineRegistryKey->getValue (LOG_WINDOW_PLACEMENT_KEY, &wp, sizeof (WINDOWPLACEMENT), &placementSize);
		if(s_useStatusWindow)
		{
			LogWindow::setWindowPlacement (&wp);
		}
	}

	char tempDirectory [MAX_PATH];

	// appearance write dir
	result = localMachineRegistryKey->getStringValue (APPEARANCE_WRITE_DIR_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize appearance write directory\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(appearanceWriteDirectory, tempDirectory);
		SetDirectoryCommand::setDirectoryString(APPEARANCE_WRITE_DIR_INDEX, appearanceWriteDirectory);
	}
	MESSENGER_LOG (("appearance write directory: \"%s\"\n", appearanceWriteDirectory));

	// shader template write dir
	result = localMachineRegistryKey->getStringValue (SHADER_TEMPLATE_WRITE_DIR_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize shader template write directory\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(shaderTemplateWriteDirectory, tempDirectory);
		SetDirectoryCommand::setDirectoryString(SHADER_TEMPLATE_WRITE_DIR_INDEX, shaderTemplateWriteDirectory);
	}
	MESSENGER_LOG (("shader template write directory: \"%s\"\n", shaderTemplateWriteDirectory));

	// shader template ref dir
	result = localMachineRegistryKey->getStringValue (SHADER_TEMPLATE_REF_DIR_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize shader template reference directory\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(shaderTemplateReferenceDirectory, tempDirectory);
		convertBackSlashesToFrontSlashes (shaderTemplateReferenceDirectory);
		SetDirectoryCommand::setDirectoryString(SHADER_TEMPLATE_REFERENCE_DIR_INDEX, shaderTemplateReferenceDirectory);
	}
	MESSENGER_LOG (("shader template reference directory: \"%s\"\n", shaderTemplateReferenceDirectory));

	// effect ref dir
	result = localMachineRegistryKey->getStringValue (EFFECT_REF_DIR_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize effect reference directory\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(effectReferenceDirectory, tempDirectory);
		convertBackSlashesToFrontSlashes (effectReferenceDirectory);
		SetDirectoryCommand::setDirectoryString(EFFECT_REFERENCE_DIR_INDEX, effectReferenceDirectory);
	}
	MESSENGER_LOG (("effect reference directory: \"%s\"\n", effectReferenceDirectory));

	// texture ref dir
	result = localMachineRegistryKey->getStringValue (TEXTURE_REF_DIR_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize texture reference directory\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(textureReferenceDirectory, tempDirectory);
		convertBackSlashesToFrontSlashes (textureReferenceDirectory);
		SetDirectoryCommand::setDirectoryString(TEXTURE_REFERENCE_DIR_INDEX, textureReferenceDirectory);
	}
	MESSENGER_LOG (("texture reference directory: \"%s\"\n", textureReferenceDirectory));

	// texture write dir
	result = localMachineRegistryKey->getStringValue (TEXTURE_WRITE_DIR_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize texture write directory\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(textureWriteDirectory, tempDirectory);
		SetDirectoryCommand::setDirectoryString(TEXTURE_WRITE_DIR_INDEX, textureWriteDirectory);
	}
	MESSENGER_LOG (("texture write directory: \"%s\"\n", textureWriteDirectory));

	// texture renderer ref dir
	result = localMachineRegistryKey->getStringValue (TEXTURE_RENDERER_REF_DIR_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize texture renderer reference directory\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(textureReferenceRendererDirectory, tempDirectory);
		convertBackSlashesToFrontSlashes (textureReferenceRendererDirectory);
		SetDirectoryCommand::setDirectoryString(TEXTURE_RENDERER_REFERENCE_DIR_INDEX, textureReferenceRendererDirectory);
	}
	MESSENGER_LOG (("texture renderer reference directory: \"%s\"\n", textureReferenceRendererDirectory));

	// animation write dir
	result = localMachineRegistryKey->getStringValue (ANIMATION_WRITE_DIR_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize animation write directory\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(animationWriteDirectory, tempDirectory);
		SetDirectoryCommand::setDirectoryString(ANIMATION_WRITE_DIR_INDEX, animationWriteDirectory);
	}
	MESSENGER_LOG (("animation write directory: \"%s\"\n", animationWriteDirectory));

	// author
	result = localMachineRegistryKey->getStringValue (AUTHOR_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize author\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(author, tempDirectory);
		SetDirectoryCommand::setDirectoryString(AUTHOR_INDEX, author);
	}
	MESSENGER_LOG (("author: \"%s\"\n", author));

	// active drive
	result = localMachineRegistryKey->getStringValue (ACTIVE_DRIVE_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize active drive\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(activeDrive, tempDirectory);
		SetDirectoryCommand::setDirectoryString(ACTIVE_DRIVE_INDEX, activeDrive);
	}
	MESSENGER_LOG (("activeDrive: \"%s\"\n",activeDrive));

	// skeleton template write dir
	result = localMachineRegistryKey->getStringValue (SKELETON_TEMPLATE_WRITE_DIR_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize skeleton template write directory\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(skeletonTemplateWriteDirectory, tempDirectory);
		SetDirectoryCommand::setDirectoryString(SKELETON_TEMPLATE_WRITE_DIR_INDEX, skeletonTemplateWriteDirectory);
	}
	MESSENGER_LOG (("skeleton template write directory: \"%s\"\n", skeletonTemplateWriteDirectory));

	// skeleton template ref dir
	result = localMachineRegistryKey->getStringValue (SKELETON_TEMPLATE_REFERENCE_DIR_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize skeleton template reference directory\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(skeletonTemplateReferenceDirectory, tempDirectory);
		convertBackSlashesToFrontSlashes (skeletonTemplateReferenceDirectory);
		SetDirectoryCommand::setDirectoryString(SKELETON_TEMPLATE_REFERENCE_DIR_INDEX, skeletonTemplateReferenceDirectory);
	}
	MESSENGER_LOG (("skeleton template reference directory: \"%s\"\n", skeletonTemplateReferenceDirectory));

	// log dir
	result = localMachineRegistryKey->getStringValue (LOG_DIR_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize log directory\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(logDirectory, tempDirectory);
		SetDirectoryCommand::setDirectoryString(LOG_DIR_INDEX, logDirectory);
	}
	MESSENGER_LOG (("log directory: \"%s\"\n", logDirectory));

	// appearance reference dir
	result = localMachineRegistryKey->getStringValue (APPEARANCE_REFERENCE_DIR_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize log directory\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(appearanceReferenceDirectory, tempDirectory);
		convertBackSlashesToFrontSlashes (appearanceReferenceDirectory);
		SetDirectoryCommand::setDirectoryString(APPEARANCE_REFERENCE_DIR_INDEX, appearanceReferenceDirectory);
	}
	MESSENGER_LOG (("appearance reference directory: \"%s\"\n", appearanceReferenceDirectory));

	//-- sat file write dir
	result = localMachineRegistryKey->getStringValue (SAT_WRITE_DIR_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize appearance write directory\n"));
	//override with new location if necessary
	if (strlen(tempDirectory) != 0)
	{
		strcpy(satWriteDirectory, tempDirectory);
		SetDirectoryCommand::setDirectoryString(SAT_WRITE_DIR_INDEX, satWriteDirectory);
	}
	else if (strlen(satWriteDirectory) == 0)
	{
		//if nothing defined this, default to appearance write dir
		strcpy(satWriteDirectory, SetDirectoryCommand::getDirectoryString(APPEARANCE_WRITE_DIR_INDEX));
		SetDirectoryCommand::setDirectoryString(SAT_WRITE_DIR_INDEX, satWriteDirectory);
	}			
	MESSENGER_LOG (("sat write directory: \"%s\"\n", satWriteDirectory));

	// asset db name
	result = localMachineRegistryKey->getStringValue (ASSET_DB_NAME_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize asset database name\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(assetDBName, tempDirectory);
		SetDirectoryCommand::setDirectoryString(ASSET_DB_NAME_DIR_INDEX, assetDBName);
	}
	MESSENGER_LOG (("AssetDatabase Name: \"%s\"\n", assetDBName));
	
	// asset db loginname
	result = localMachineRegistryKey->getStringValue (ASSET_DB_LOGINNAME_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize asset database login name\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(assetDBLogin, tempDirectory);
		SetDirectoryCommand::setDirectoryString(ASSET_DB_LOGINNAME_DIR_INDEX, assetDBLogin);
	}
	MESSENGER_LOG (("AssetDatabase Login Name: \"%s\"\n", assetDBLogin));

	// asset db password
	result = localMachineRegistryKey->getStringValue (ASSET_DB_PASSWORD_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize asset database password\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(assetDBPassword, tempDirectory);
		SetDirectoryCommand::setDirectoryString(ASSET_DB_PASSWORD_DIR_INDEX, assetDBPassword);
	}
	MESSENGER_LOG (("AssetDatabase Password: \"%s\"\n", assetDBPassword));

	// asset db activated state
	result = localMachineRegistryKey->getStringValue (ASSET_DB_ACTIVATED_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize asset database activated state\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(assetDBActivated, tempDirectory);
		SetDirectoryCommand::setDirectoryString(ASSET_DB_ACTIVATED_INDEX, assetDBActivated);
#if !NO_DATABASE
		if(std::string(assetDBActivated) == "false")
			DatabaseImporter::activate(false, false);
		else
			DatabaseImporter::activate(true, false);
#endif
	}
	MESSENGER_LOG (("AssetDatabase Activated: \"%s\"\n", assetDBActivated));

	result = localMachineRegistryKey->getStringValue (VIEWER_LOCATION_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize viewer location\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(viewerLocation, tempDirectory);
		SetDirectoryCommand::setDirectoryString(VIEWER_LOCATION_INDEX, viewerLocation);
	}
	else
	{
		viewerLocation[0] = 0;
	}
	MESSENGER_LOG (("viewer location: \"%s\"\n", viewerLocation));

	// p4 branch list
	result = localMachineRegistryKey->getStringValue (PERFORCE_BRANCH_LIST_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize perforce branch list\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(perforceBranchList, tempDirectory);
		SetDirectoryCommand::setDirectoryString(PERFORCE_BRANCH_LIST_INDEX, perforceBranchList);
		ExportManager::setValidBranchesPacked(perforceBranchList);
	}
	MESSENGER_LOG (("Perforce Branch List: \"%s\"\n", perforceBranchList));

	//alienbrain project name
	result = localMachineRegistryKey->getStringValue (ALIENBRAIN_PROJECT_NAME_KEY, "", tempDirectory, sizeof (tempDirectory), true);
	MESSENGER_REJECT (!result, ("failed to initialize Alienbrain projects name\n"));
	if(strlen(tempDirectory) != 0)
	{
		strcpy(alienbrainProjectName, tempDirectory);
		SetDirectoryCommand::setDirectoryString(ALIENBRAIN_PROJECT_NAME_INDEX, alienbrainProjectName);
	}
	MESSENGER_LOG (("Alienbrain Project Name: \"%s\"\n", alienbrainProjectName));

	return true;
}

// ----------------------------------------------------------------------

static void SaveRegistrySettings ()
{
	WriteRegistrySettings ();

	// save log window placement
	WINDOWPLACEMENT  wp;
	wp.length = sizeof (WINDOWPLACEMENT);
	if(s_useStatusWindow)
	{
		LogWindow::getWindowPlacement(&wp);
	}

	RegistryKey * const registryKey = RegistryKey::getLocalMachineKey ()->createSubkey (SOE_REGISTRY_KEY);
	registryKey->setValue (LOG_WINDOW_PLACEMENT_KEY, &wp, sizeof (WINDOWPLACEMENT), REG_BINARY);
}

// ----------------------------------------------------------------------

static void OnCloseCallback(void * /*data*/)
{
	SaveRegistrySettings ();
	StopEngine (true);
	messenger = 0;
}


// ----------------------------------------------------------------------

MStatus __declspec (dllexport) initializePlugin (MObject object)
{
	MStatus  status;

	messenger = &theMessenger;

	// start the engine
	if (!StartEngine ())
		return MS::kFailure;

	ExporterLog::setMayaExporterVersion(MAYA_EXPORTER_VERSION);

	MESSENGER_LOG (("MAYA Exporter version %s %s %s\n", MAYA_EXPORTER_VERSION, __DATE__, __TIME__));

	if (!LoadRegistrySettings())
		return MS::kFailure;
	SaveRegistrySettings();

	MESSENGER_LOG (("Memory allocated: %d Megs\n", s_allocatedMemory));

	// hookup plugin object
	MFnPlugin plugin (object, "Sony Online Entertainment", MAYA_EXPORTER_VERSION, "Any");

	// register the plugin commands
	status = plugin.registerCommand ("exportStaticMesh", ExportStaticMesh::creator);
	if (!status)
	{
		MESSENGER_LOG (("failed to register exportStaticMesh command\n"));
		REPORT_LOG (true, ("failed to register exportStaticMesh command\n"));
		StopEngine ();
		return status;
	}

	status = plugin.registerCommand ("e", ExportStaticMesh::creator);
	if (!status)
	{
		MESSENGER_LOG (("failed to register e command\n"));
		REPORT_LOG (true, ("failed to register e command\n"));
		StopEngine ();
		return status;
	}

	status = plugin.registerCommand ("setBaseDir", SetBaseDirectory::creator);
	if (!status)
	{
		MESSENGER_LOG (("failed to register setBaseDir command\n"));
		REPORT_LOG (true, ("failed to register setBaseDir command\n"));
		StopEngine ();
		return status;
	}

	//-- register "exportSkeletonTemplate" command
	status = plugin.registerCommand ("exportSkeleton", ExportSkeleton::creator);
	if (!status)
	{
		MESSENGER_LOG (("failed to register exportSkeleton command\n"));
		REPORT_LOG (true, ("failed to register exportSkeleton command\n"));
		StopEngine ();
		return status;
	}

	//-- register "exportSkeletalMeshGenerator" command
	status = plugin.registerCommand ("exportSkeletalMeshGenerator", ExportSkeletalMeshGenerator::creator);
	if (!status)
	{
		MESSENGER_LOG (("failed to register exportSkeletalMeshGenerator command\n"));
		REPORT_LOG (true, ("failed to register exportSkeletalMeshGenerator command\n"));
		StopEngine ();
		return status;
	}

	//-- register "exportKeyframeSkeletalAnimationTemplate" command
	status = plugin.registerCommand ("exportSkeletalAnimation", ExportKeyframeSkeletalAnimation::creator);
	if (!status)
	{
		MESSENGER_LOG (("failed to register exportSkeletalAnimation command\n"));
		REPORT_LOG (true, ("failed to register exportSkeletalAnimation command\n"));
		StopEngine ();
		return status;
	}

	status = plugin.registerCommand ("addAnimation", AddAnimationCommand::creator);
	if (!status)
	{
		MESSENGER_LOG (("failed to register command\n"));
		REPORT_LOG (true, ("failed to register command\n"));
		StopEngine ();
		return status;
	}

	status = plugin.registerCommand ("visitAnimation", VisitAnimationCommand::creator);
	if (!status)
	{
		MESSENGER_LOG (("failed to register command\n"));
		REPORT_LOG (true, ("failed to register command\n"));
		StopEngine ();
		return status;
	}

	status = plugin.registerCommand ("deleteAnimation", DeleteAnimationCommand::creator);
	if (!status)
	{
		MESSENGER_LOG (("failed to register command\n"));
		REPORT_LOG (true, ("failed to register command\n"));
		StopEngine ();
		return status;
	}

	status = plugin.registerCommand ("setAuthor", SetAuthorCommand::creator);
	if (!status)
	{
		MESSENGER_LOG (("failed to register command\n"));
		REPORT_LOG (true, ("failed to register command\n"));
		StopEngine ();
		return status;
	}

	status = plugin.registerCommand ("reexport", ReexportCommand::creator);
	if (!status)
	{
		MESSENGER_LOG (("failed to register command\n"));
		REPORT_LOG (true, ("failed to register command\n"));
		StopEngine ();
		return status;
	}

	status = plugin.registerCommand ("export", ExportCommand::creator);
	if (!status)
	{
		MESSENGER_LOG (("failed to register command\n"));
		REPORT_LOG (true, ("failed to register command\n"));
		StopEngine ();
		return status;
	}

	status = plugin.registerCommand ("exportSatFile", ExportSkeletalAppearanceTemplate::creator);
	if (!status)
	{
		MESSENGER_LOG (("failed to register command\n"));
		REPORT_LOG (true, ("failed to register command\n"));
		StopEngine ();
		return status;
	}

#if !NO_DATABASE
	status = plugin.registerCommand ("useAssetDatabase", UseAssetDatabaseCommand::creator);
	if (!status)
	{
		MESSENGER_LOG (("failed to register command\n"));
		REPORT_LOG (true, ("failed to register command\n"));
		StopEngine ();
		return status;
	}
#endif

	status = plugin.registerCommand ("createTransformMask", CreateTransformMask::creator);
	if (!status)
	{
		MESSENGER_LOG (("failed to register createTransformMask command\n"));
		REPORT_LOG (true, ("failed to register createTransformMask command\n"));
		StopEngine ();
		return status;
	}

/*
// JU_TODO: this guy isn't linking in maya7
	// callback to shutdown engine on maya exit - maya doesn't unload plugins

	MSceneMessage::addCallback(
				MSceneMessage::kMayaExiting,
				OnCloseCallback,
				NULL,
				&status);
*/

	// indicate successful initialization
	return MS::kSuccess;
}

// ----------------------------------------------------------------------

MStatus __declspec (dllexport) uninitializePlugin (MObject object)
{
	MStatus  status;

	// hookup plugin object
	MFnPlugin plugin (object, "Bootprint Entertainment", "1.0", "Any");

	// uninitialize the plugin commands
	status = plugin.deregisterCommand ("exportStaticMesh");
	if (!status)
	{
		MESSENGER_LOG (("failed to deregister exportStaticMesh command\n"));
		REPORT_LOG (true, ("failed to deregister exportStaticMesh command\n"));
	}

	// uninitialize the plugin commands
	status = plugin.deregisterCommand ("e");
	if (!status)
	{
		MESSENGER_LOG (("failed to deregister e command\n"));
		REPORT_LOG (true, ("failed to deregister e command\n"));
	}

	status = plugin.deregisterCommand ("setBaseDir");
	if (!status)
	{
		MESSENGER_LOG (("failed to deregister setBaseDir command\n"));
		REPORT_LOG (true, ("failed to deregister setBaseDir command\n"));
	}

	//-- deregister exportSkeletonTemplate command
	status = plugin.deregisterCommand ("exportSkeleton");
	if (!status)
	{
		MESSENGER_LOG (("failed to deregister exportSkeleton command\n"));
		REPORT_LOG (true, ("failed to deregister exportSkeleton command\n"));
	}

	//-- deregister ExportSkeletalMeshGenerator command
	status = plugin.deregisterCommand ("exportSkeletalMeshGenerator");
	if (!status)
	{
		MESSENGER_LOG (("failed to deregister exportSkeletalMeshGenerator command\n"));
		REPORT_LOG (true, ("failed to deregister exportSkeletalMeshGenerator command\n"));
	}

	//-- deregister exportKeyframeSkeletalAnimationTemplate command
	status = plugin.deregisterCommand ("exportSkeletalAnimation");
	if (!status)
	{
		MESSENGER_LOG (("failed to deregister exportSkeletalAnimation command\n"));
		REPORT_LOG (true, ("failed to deregister exportSkeletalAnimation command\n"));
	}

	status = plugin.deregisterCommand ("addAnimation");
	if (!status)
	{
		MESSENGER_LOG (("failed to deregister command\n"));
		REPORT_LOG (true, ("failed to deregister command\n"));
	}

	status = plugin.deregisterCommand ("visitAnimation");
	if (!status)
	{
		MESSENGER_LOG (("failed to deregister command\n"));
		REPORT_LOG (true, ("failed to deregister command\n"));
	}

	status = plugin.deregisterCommand ("deleteAnimation");
	if (!status)
	{
		MESSENGER_LOG (("failed to deregister command\n"));
		REPORT_LOG (true, ("failed to deregister command\n"));
	}

	status = plugin.deregisterCommand ("setAuthor");
	if (!status)
	{
		MESSENGER_LOG (("failed to deregister command\n"));
		REPORT_LOG (true, ("failed to deregister command\n"));
	}

	status = plugin.deregisterCommand ("reexport");
	if (!status)
	{
		MESSENGER_LOG (("failed to deregister command\n"));
		REPORT_LOG (true, ("failed to deregister command\n"));
	}

	status = plugin.deregisterCommand ("perforceStartMultiExport");
	if (!status)
	{
		MESSENGER_LOG (("failed to deregister command\n"));
		REPORT_LOG (true, ("failed to deregister command\n"));
	}

	status = plugin.deregisterCommand ("perforceEndMultiExport");
	if (!status)
	{
		MESSENGER_LOG (("failed to deregister command\n"));
		REPORT_LOG (true, ("failed to deregister command\n"));
	}

	status = plugin.deregisterCommand ("export");
	if (!status)
	{
		MESSENGER_LOG (("failed to deregister command\n"));
		REPORT_LOG (true, ("failed to deregister command\n"));
	}

	status = plugin.deregisterCommand ("exportSatFile");
	if (!status)
	{
		MESSENGER_LOG (("failed to deregister command\n"));
		REPORT_LOG (true, ("failed to deregister command\n"));
	}

#if !NO_DATABASE
	status = plugin.deregisterCommand ("useAssetDatabase");
	if (!status)
	{
		MESSENGER_LOG (("failed to deregister command\n"));
		REPORT_LOG (true, ("failed to deregister command\n"));
	}
#endif

	status = plugin.deregisterCommand ("createTransformMask");
	if (!status)
	{
		MESSENGER_LOG (("failed to deregister createTransformMask command\n"));
		REPORT_LOG (true, ("failed to deregister createTransformMask command\n"));
	}

	// delete registry key
	SaveRegistrySettings ();

	// stop the engine
	StopEngine ();

	messenger = 0;

	// return success
	return MS::kSuccess;		
}

// ======================================================================
/**
 * Retrieve the HINSTANCE for the plug-in's DLL.
 *
 * @return  the HINSTANCE for the plug-in's DLL.
 */

HINSTANCE GetPluginInstanceHandle()
{
	return MhInstPlugin;
}

// ======================================================================
