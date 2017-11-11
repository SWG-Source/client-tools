// ======================================================================
//
// SetBaseDirectory.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "SetBaseDirectory.h"

#include "maya/MArgList.h"
#include "MayaUtility.h"
#include "Messenger.h"
#include "PluginMain.h"
#include "SetDirectoryCommand.h"
#include <string>

// ======================================================================

namespace
{
	Messenger *messenger;
}

// ======================================================================

void SetBaseDirectory::install(Messenger *newMessenger)
{
	messenger = newMessenger;
}

// ----------------------------------------------------------------------

void SetBaseDirectory::remove()
{
	messenger = 0;
}

// ----------------------------------------------------------------------

void *SetBaseDirectory::creator()
{
	return new SetBaseDirectory();
}

// ======================================================================

MStatus SetBaseDirectory::doIt(const MArgList &argList)
{
	MStatus status;

	const unsigned argCount = argList.length(&status);
	MESSENGER_REJECT_STATUS(!status, ("failed to get number of arguments\n"));
	MESSENGER_REJECT_STATUS(argCount != 1, ("command requires one string argument\n"));

	MString mayaArg = argList.asString(0, &status);
	MESSENGER_REJECT_STATUS(!status, ("failed to get first argument as string\n"));

	//-- build the base directory
	std::string baseDirectory = mayaArg.asChar();
	if (baseDirectory[baseDirectory.size()-1] != '\\')
		baseDirectory.push_back('\\');

	//-- populate the write directories

	// create the write directory strings 
	const std::string appearanceWriteDir       = baseDirectory      + "appearance\\";
	const std::string shaderTemplateWriteDir   = baseDirectory      + "shader\\";
	const std::string textureWriteDir          = baseDirectory      + "texture\\";
	const std::string animationWriteDir        = appearanceWriteDir + "animation\\";
	const std::string skeletonTemplateWriteDir = appearanceWriteDir + "skeleton\\";

	const std::string meshWriteDir             = appearanceWriteDir + "mesh\\";
	const std::string logWriteDir              = baseDirectory      + "log\\";
	const std::string satWriteDir              = appearanceWriteDir;

	// set the write directories
	SetDirectoryCommand::setDirectoryString(APPEARANCE_WRITE_DIR_INDEX, appearanceWriteDir.c_str());
	SetDirectoryCommand::setDirectoryString(SHADER_TEMPLATE_WRITE_DIR_INDEX, shaderTemplateWriteDir.c_str());
	SetDirectoryCommand::setDirectoryString(TEXTURE_WRITE_DIR_INDEX, textureWriteDir.c_str());
	SetDirectoryCommand::setDirectoryString(ANIMATION_WRITE_DIR_INDEX, animationWriteDir.c_str());
	SetDirectoryCommand::setDirectoryString(SKELETON_TEMPLATE_WRITE_DIR_INDEX, skeletonTemplateWriteDir.c_str());
	SetDirectoryCommand::setDirectoryString(LOG_DIR_INDEX, logWriteDir.c_str());
	SetDirectoryCommand::setDirectoryString(SAT_WRITE_DIR_INDEX, satWriteDir.c_str());

	// create the directories if they don't exist
	IGNORE_RETURN(MayaUtility::createDirectory(appearanceWriteDir.c_str()));
	IGNORE_RETURN(MayaUtility::createDirectory(shaderTemplateWriteDir.c_str()));
	IGNORE_RETURN(MayaUtility::createDirectory(textureWriteDir.c_str()));
	IGNORE_RETURN(MayaUtility::createDirectory(animationWriteDir.c_str()));
	IGNORE_RETURN(MayaUtility::createDirectory(skeletonTemplateWriteDir.c_str()));
	IGNORE_RETURN(MayaUtility::createDirectory(meshWriteDir.c_str()));
	IGNORE_RETURN(MayaUtility::createDirectory(logWriteDir.c_str()));
	IGNORE_RETURN(MayaUtility::createDirectory(satWriteDir.c_str()));

	// print write directories
	MESSENGER_LOG(("new appearance write directory:   [%s]\n", appearanceWriteDir.c_str()));
	MESSENGER_LOG(("new shader write directory:       [%s]\n", shaderTemplateWriteDir.c_str()));
	MESSENGER_LOG(("new texture write directory:      [%s]\n", textureWriteDir.c_str()));
	MESSENGER_LOG(("new animation write directory:    [%s]\n", animationWriteDir.c_str()));
	MESSENGER_LOG(("new skeleton write directory:     [%s]\n", skeletonTemplateWriteDir.c_str()));
	MESSENGER_LOG(("new log write directory:          [%s]\n", logWriteDir.c_str()));
	MESSENGER_LOG(("new sat write directory:          [%s]\n", satWriteDir.c_str()));

	//-- populate the reference directories
	
	// create the reference directory strings
	const std::string shaderTemplateReferenceDir   = "shader/";
	const std::string effectReferenceDir           = "effect/";
	const std::string textureReferenceDir          = "texture/";
	const std::string textureRendererReferenceDir  = "texturerenderer/";
	const std::string skeletonTemplateReferenceDir = "appearance/skeleton/";
	const std::string appearanceReferenceDir       = "appearance/";

	// set the reference directories
	SetDirectoryCommand::setDirectoryString(APPEARANCE_REFERENCE_DIR_INDEX, appearanceReferenceDir.c_str());
	SetDirectoryCommand::setDirectoryString(SHADER_TEMPLATE_REFERENCE_DIR_INDEX, shaderTemplateReferenceDir.c_str());
	SetDirectoryCommand::setDirectoryString(EFFECT_REFERENCE_DIR_INDEX, effectReferenceDir.c_str());
	SetDirectoryCommand::setDirectoryString(TEXTURE_REFERENCE_DIR_INDEX, textureReferenceDir.c_str());
	SetDirectoryCommand::setDirectoryString(TEXTURE_RENDERER_REFERENCE_DIR_INDEX, textureRendererReferenceDir.c_str());
	SetDirectoryCommand::setDirectoryString(SKELETON_TEMPLATE_REFERENCE_DIR_INDEX, skeletonTemplateReferenceDir.c_str());

	// print reference directories
	MESSENGER_LOG(("new shader reference directory:           [%s]\n", shaderTemplateReferenceDir.c_str()));
	MESSENGER_LOG(("new effect reference directory:           [%s]\n", effectReferenceDir.c_str()));
	MESSENGER_LOG(("new texture reference directory:          [%s]\n", textureReferenceDir.c_str()));
	MESSENGER_LOG(("new texture renderer reference directory: [%s]\n", textureRendererReferenceDir.c_str()));
	MESSENGER_LOG(("new skeleton reference directory:         [%s]\n", skeletonTemplateReferenceDir.c_str()));

	return MStatus(MStatus::kSuccess);
}

// ======================================================================
