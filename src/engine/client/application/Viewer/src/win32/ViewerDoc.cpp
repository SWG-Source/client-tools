// ======================================================================
//
// ViewerDoc.cpp
// Portions Copyright 1999, Bootprint Enterainment, Inc.
// Portions Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "FirstViewer.h"
#include "viewer.h"

#include "viewerDoc.h"
#include "ChildFrm.h"
#include "ClientBakedWearableWriter.h"
#include "DialogAddSkeleton.h"	
#include "DialogMapAnimation.h"
#include "DialogSetVariable.h"
#include "MainFrm.h"
#include "SkeletalAppearanceTemplateView.h"
#include "VariableSetView.h"
#include "ViewerPreferences.h"
#include "ViewerView.h"
#include "AnimationTreeDialog.h"

#include "clientGraphics/Light.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientObject/HardpointObject.h"
#include "clientObject/ShaderAppearance.h"
#include "clientSkeletalAnimation/ActionGeneratorSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/BasicMeshGeneratorTemplate.h"
#include "clientSkeletalAnimation/BasicSkeletonTemplate.h"
#include "clientSkeletalAnimation/CompositeMesh.h"
#include "clientSkeletalAnimation/DirectionSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/LogicalAnimationTableTemplate.h"
#include "clientSkeletalAnimation/LogicalAnimationTableTemplateList.h"
#include "clientSkeletalAnimation/MeshGenerator.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplate.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplateList.h"
#include "clientSkeletalAnimation/PriorityBlendAnimationTemplate.h"
#include "clientSkeletalAnimation/ProxySkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/SkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"
#include "clientSkeletalAnimation/SkeletalMeshGenerator.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "clientSkeletalAnimation/SkeletonTemplate.h"
#include "clientSkeletalAnimation/SkeletonTemplateList.h"
#include "clientSkeletalAnimation/SpeedSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/StateHierarchyAnimationController.h"
#include "clientSkeletalAnimation/StringSelectorSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/TimeScaleSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/TrackAnimationController.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "clientTextureRenderer/TextureRenderer.h"
#include "clientTextureRenderer/TextureRendererList.h"
#include "clientTextureRenderer/TextureRendererTemplate.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedMath/Quaternion.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/CustomizationVariable.h"
#include "sharedObject/Hardpoint.h"
#include "sharedObject/MemoryBlockManagedObject.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplateCustomizationDataWriter.h"
#include "sharedObject/PortalProperty.h"
#include "sharedUtility/FileName.h"

#include <map>
#include <algorithm>
#include <afxcoll.h>
//#include <mmsystem.h>
#include <vector>
#include <string>

const Tag TAG_MAIN = TAG(M,A,I,N);

// ======================================================================
// constants
// ======================================================================

namespace
{
	const int             ms_skeletalAppearanceInstanceIffSize  = 32 * 1024;
	const int             ms_skeletalAppearanceTemplateIffSize  = 128 * 1024;
	const int             ms_skeletalAppearanceWorkspaceIffSize = 32 * 1024;

	const ConstCharCrcLowerString  cms_primaryTrackName("loop");

	AnimationTrackId      ms_primaryTrackId(AnimationTrackId::cms_invalid);

	NetworkId::NetworkIdType  ms_networkId;

	bool ms_buildingAsynchronousLoaderData;

	typedef std::vector<std::string> FileNameVector;
	std::string ms_workingDirectory;
	FileNameVector ms_appearanceFiles;
	int const ms_maxHardpointAttachDepth = 8;
}

// ======================================================================
// static storage definition
// ======================================================================

bool                                 CViewerDoc::ms_installed;
CViewerDoc::FileTypeLoaderContainer *CViewerDoc::ms_fileTypeLoaders;

// ======================================================================
// embedded entity declarations
// ======================================================================

struct CViewerDoc::FileTypeLoaderContainer
{
public:

	typedef std::map<CString, FileTypeLoader> Container;

public:

	Container  m_container;

};

// ======================================================================

IMPLEMENT_DYNCREATE(CViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CViewerDoc, CDocument)
	//{{AFX_MSG_MAP(CViewerDoc)
	ON_COMMAND(ID_ANIMATION_ADDKEYMAPPING_SKELETALANIMATION, OnAnimationAddkeymappingSkeletalanimation)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_ADDKEYMAPPING_SKELETALANIMATION, OnUpdateAnimationAddkeymappingSkeletalanimation)
	ON_COMMAND(ID_ANIMATION_SKELETALANIMATION_STOPWEARING, OnAnimationSkeletalanimationStopwearing)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_SKELETALANIMATION_STOPWEARING, OnUpdateAnimationSkeletalanimationStopwearing)
	ON_COMMAND(ID_ANIMATION_SKELETALANIMATION_WEAR, OnAnimationSkeletalanimationWear)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_SKELETALANIMATION_WEAR, OnUpdateAnimationSkeletalanimationWear)
	ON_COMMAND(ID_ANIMATION_MESHANIMATION_LOADANIMATIONSET, OnAnimationMeshanimationLoadanimationset)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_MESHANIMATION_LOADANIMATIONSET, OnUpdateAnimationMeshanimationLoadanimationset)
	ON_COMMAND(ID_ANIMATION_MESHANIMATION_SAVEANIMATIONSET, OnAnimationMeshanimationSaveanimationset)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_MESHANIMATION_SAVEANIMATIONSET, OnUpdateAnimationMeshanimationSaveanimationset)
	ON_COMMAND(ID_ANIMATION_SKELETALANIMATION_LOADANIMATIONSET, OnAnimationSkeletalanimationLoadanimationset)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_SKELETALANIMATION_LOADANIMATIONSET, OnUpdateAnimationSkeletalanimationLoadanimationset)
	ON_COMMAND(ID_ANIMATION_SKELETALANIMATION_SAVEANIMATIONSET, OnAnimationSkeletalanimationSaveanimationset)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_SKELETALANIMATION_SAVEANIMATIONSET, OnUpdateAnimationSkeletalanimationSaveanimationset)
	ON_COMMAND(ID_ANIMATION_SKELETALANIMATION_ATTACHTOSELECTEDBONE, OnAnimationSkeletalanimationAttachtoselectedbone)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_SKELETALANIMATION_ATTACHTOSELECTEDBONE, OnUpdateAnimationSkeletalanimationAttachtoselectedbone)
	ON_COMMAND(ID_ANIMATION_SKELETALANIMATION_DETACH, OnAnimationSkeletalanimationDetach)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_SKELETALANIMATION_DETACH, OnUpdateAnimationSkeletalanimationDetach)
	ON_COMMAND(ID_ANIMATION_SKELETALANIMATION_SELECTSHADOWVOLUME, OnAnimationSkeletalanimationSelectshadowvolume)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_SKELETALANIMATION_SELECTSHADOWVOLUME, OnUpdateAnimationSkeletalanimationSelectshadowvolume)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RENDERSKELETON, OnUpdateViewRenderSkeleton)
	ON_COMMAND(ID_VIEW_RENDERSKELETON, OnViewRenderSkeleton)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RENDERMESH, OnUpdateViewRenderMesh)
	ON_COMMAND(ID_VIEW_RENDERMESH, OnViewRenderMesh)
	ON_UPDATE_COMMAND_UI(ID_VIEW_VARIABLE_SET_VIEW, OnUpdateViewVariableSetView)
	ON_COMMAND(ID_VIEW_VARIABLE_SET_VIEW, OnViewVariableSetView)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SKELETAL_APPEARANCE_TEMPLATE, OnUpdateViewSkeletalAppearanceTemplate)
	ON_COMMAND(ID_SKELTEMPGROUP_ADD, OnSkeletonTemplateAdd)
	ON_COMMAND(ID_MESHGENGROUP_ADD, OnMeshGeneratorAdd)
	ON_COMMAND(ID_FILE_WRITE_CUSTOMIZATION_DATA, OnFileWriteObjTemplateCustomizationData)
	ON_UPDATE_COMMAND_UI(ID_FILE_WRITE_CUSTOMIZATION_DATA, OnUpdateFileWriteCustomizationData)
	ON_COMMAND(ID_BUTTON_CENTER_OBJECT, OnButtonCenterObject)
	ON_COMMAND(ID_BUTTON_TOGGLE_LOCOMOTION, OnButtonToggleLocomotion)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_TOGGLE_LOCOMOTION, OnUpdateButtonToggleLocomotion)
	ON_COMMAND(ID_BUTTON_DEBUGDUMP, OnButtonDebugdump)
	ON_COMMAND(ID_FILE_WRITE_CDF_BAKED_WEARABLE_DATA, OnFileWriteCdfBakedWearableData)
	ON_COMMAND(ID_SAVE_HARDPOINT_HIERARCHY, OnSaveHardpointHierarchy)
	ON_COMMAND(ID_LOAD_ALL_HARDPOINTS, OnLoadAllHardpoints)
	ON_COMMAND(ID_VIEW_SKELETAL_APPEARANCE_TEMPLATE, OnViewSkeletalAppearanceTemplate)
	ON_UPDATE_COMMAND_UI(ID_LOAD_ALL_HARDPOINTS, OnUpdateLoadAllHardpoints)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

void CViewerDoc::install()
{
	DEBUG_FATAL(ms_installed, ("CViewerDoc already installed"));

	//-- populate the file type loaders.
	//   these handle special cases where the viewer needs to perform
	//   special handling for a particular extension.
	//   NOTE: add extensions in all lower case.  do not include a period.

	ms_fileTypeLoaders = NON_NULL(new FileTypeLoaderContainer());
	
	ms_fileTypeLoaders->m_container.insert(std::make_pair(CString("dds"), &CViewerDoc::loadTexture));
	ms_fileTypeLoaders->m_container.insert(std::make_pair(CString("lmg"), &CViewerDoc::loadMeshGenerator));
	ms_fileTypeLoaders->m_container.insert(std::make_pair(CString("mgn"), &CViewerDoc::loadMeshGenerator));
	ms_fileTypeLoaders->m_container.insert(std::make_pair(CString("trt"), &CViewerDoc::loadTextureRendererTemplate));
	ms_fileTypeLoaders->m_container.insert(std::make_pair(CString("sat"), &CViewerDoc::loadSkeletalAppearanceTemplate));
	ms_fileTypeLoaders->m_container.insert(std::make_pair(CString("saw"), &CViewerDoc::loadSkeletalAppearanceWorkspace));
	ms_fileTypeLoaders->m_container.insert(std::make_pair(CString("sht"), &CViewerDoc::loadShaderTemplate));
	ms_fileTypeLoaders->m_container.insert(std::make_pair(CString("skt"), &CViewerDoc::loadSkeletonTemplate));
	ms_fileTypeLoaders->m_container.insert(std::make_pair(CString("pob"), &CViewerDoc::loadPortalObject));
	ms_fileTypeLoaders->m_container.insert(std::make_pair(CString("hard"), &CViewerDoc::loadHardpointHierarchy));

	const bool gtidSuccess = TrackAnimationController::getTrackIdByName (cms_primaryTrackName, ms_primaryTrackId);
	DEBUG_FATAL (!gtidSuccess, ("failed to get TrackId for track [%s]", cms_primaryTrackName.getString ()));
	UNREF (gtidSuccess);

	ms_installed = true;
}

// ----------------------------------------------------------------------

void CViewerDoc::remove()
{
	DEBUG_FATAL(!ms_installed, ("CViewerDoc not installed"));

	delete ms_fileTypeLoaders;
	ms_fileTypeLoaders = 0;

	ms_installed = false;
}

// ----------------------------------------------------------------------

CViewerDoc::FileTypeLoader CViewerDoc::getFileTypeLoader(const char *extensionNoDot)
{
	CString extension(extensionNoDot);
	extension.MakeLower();

	NOT_NULL(ms_fileTypeLoaders);
	FileTypeLoaderContainer::Container::iterator it = ms_fileTypeLoaders->m_container.find(extension);
	if (it == ms_fileTypeLoaders->m_container.end())
		return 0;
	else
		return (*it).second;
}

// ----------------------------------------------------------------------

void CViewerDoc::setBuildingAsynchronousLoaderData(bool const buildingAsynchronousLoaderData)
{
	ms_buildingAsynchronousLoaderData = buildingAsynchronousLoaderData;
}

// ======================================================================

CViewerDoc::CViewerDoc() :	
	defaultAnimationFileName(""),	
	defaultBlendTime(0.00f),
	defaultKey("1"),
	defaultTimeScale(1.0f),
	defaultWeight (CONST_REAL (1)),
	defaultPlayModeIndex(1),
	defaultIsLooping(FALSE),
	defaultIsInterruptible(TRUE),
	defaultSoundFilename (""),
	badAlterCount(0),
	messageQueue(NON_NULL(new MessageQueue(32))),
	hardpointIndex(-1),
	m_skeletalAppearanceTemplate(0),
	m_skeletalAppearanceTemplateFilename(),
	m_skeletalAppearanceWorkspaceFilename(),
	m_texture(0),
	m_textureRenderer(0),
	m_fatalThrowsExceptionsOnLoad(true),
	m_lightList(new LightList),
	m_locomotionEnabled(true)
{
	DEBUG_FATAL(!ms_installed, ("CViewerDoc not installed"));

	m_animationFileList[0] = new AnimationFileList;
	m_animationFileList[1] = new AnimationFileList;

	//-- Configure viewer document from config file.
	m_fatalThrowsExceptionsOnLoad = ConfigFile::getKeyBool("Viewer", "fatalThrowsExceptionsOnLoad", true);

	object = new Object ();
	object->setNetworkId(NetworkId(++ms_networkId));
	RenderWorld::addObjectNotifications(*object);
}

// ----------------------------------------------------------------------

CViewerDoc::~CViewerDoc()
{
	// Detach and delete any attached objects.
	// @todo re-enable this once these functions are re-implemented.
#if 0
	OnAnimationSkeletalanimationDetach();
	OnAnimationSkeletalanimationStopwearing();
#endif

	if (m_textureRenderer)
		m_textureRenderer->release();
	if (m_texture)
		m_texture->release();

	clearSkeletalAnimationList ();

	LightList::iterator iEnd = m_lightList->end();
	for (LightList::iterator i = m_lightList->begin(); i != iEnd; ++i)
		delete (*i);
	delete m_lightList;
	delete m_animationFileList[0];
	delete m_animationFileList[1];

	delete object;
	delete messageQueue;

//	sndPlaySound (NULL, 0);

	AppearanceTemplateList::release(m_skeletalAppearanceTemplate);
	m_skeletalAppearanceTemplate = 0;
}

// ----------------------------------------------------------------------

BOOL CViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------

void CViewerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void CViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

// ----------------------------------------------------------------------

void CViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

void CViewerDoc::reload (void)
{
	load (filename);
	UpdateAllViews (0);
}

// ----------------------------------------------------------------------

bool CViewerDoc::loadShaderTemplate(const char *filename)
{
	try
	{
		Shader* shader = ShaderTemplateList::fetchModifiableShader (filename);
		object->setAppearance (new ShaderAppearance (CONST_REAL (10), CONST_REAL (10), shader, VectorArgb::solidWhite));
	}
	catch (FatalException& e)
	{
		object->setAppearance (0);
		CONSOLE_PRINT (e.getMessage ());
		CONSOLE_PRINT ("\r\n");
		return false;
	}

	//-- success
	return true;
}

// ----------------------------------------------------------------------

bool CViewerDoc::loadTexture(const char *filename)
{
	try
	{
		Shader* shader = ShaderTemplateList::fetchModifiableShader("shader/viewer.sht");
		const Texture *texture = TextureList::fetch(filename);
		static_cast<StaticShader *>(shader)->setTexture(TAG_MAIN, *texture);
		texture->release();
		object->setAppearance (new ShaderAppearance (CONST_REAL (10), CONST_REAL (10), shader, VectorArgb::solidWhite));
	}
	catch (FatalException& e)
	{
		object->setAppearance (0);
		CONSOLE_PRINT (e.getMessage ());
		CONSOLE_PRINT ("\r\n");
		return false;
	}

	//-- success
	return true;
}

// ----------------------------------------------------------------------

bool CViewerDoc::loadTextureRendererTemplate(const char *filename)
{
	const TextureRendererTemplate *trtPointer = 0;
	Shader                        *shader = 0;

	try
	{
		//-- first cleanup in case this is called on an existing doc
		if (m_textureRenderer)
		{
			m_textureRenderer->release();
			m_textureRenderer = 0;
		}

		if (m_texture)
		{
			m_texture->release();
			m_texture = 0;
		}

		//-- load up the rendered texture template
		const TextureRendererTemplate *textureRendererTemplate = TextureRendererList::fetch(filename);
		if (!textureRendererTemplate)
			return false;

		trtPointer = textureRendererTemplate;

		//@todo fixup so TextureRenderer CustomizationData variables properly get loaded
		//      into the Object instance.

		//-- create the texture renderer, get its texture
		m_textureRenderer = textureRendererTemplate->createTextureRenderer();
		m_texture         = const_cast<Texture*>(m_textureRenderer->fetchTexture());


		textureRendererTemplate->release();
		trtPointer = 0;

		//-- create a temporary object appearance that renders with this texture
		shader = ShaderTemplateList::fetchModifiableShader("shader\\viewer.sht");
		static_cast<StaticShader *>(shader)->setTexture(TAG_MAIN, *m_texture);

		const float textureWidthToHeightRatio = static_cast<float>(m_texture->getWidth()) / static_cast<float>(m_texture->getHeight());
		const float appearanceHeight          = 10.0f;
		const float appearanceWidth           = appearanceHeight * textureWidthToHeightRatio;

		object->setAppearance (new ShaderAppearance (appearanceWidth, appearanceHeight, shader, VectorArgb::solidWhite));
		shader = 0;

		UpdateAllViews(0);
	}
	catch (FatalException& e)
	{
		if (trtPointer)
			trtPointer->release();

		if (shader)
			shader->release();

		object->setAppearance (0);
		CONSOLE_PRINT (e.getMessage ());
		CONSOLE_PRINT ("\r\n");
		return false;
	}

	//-- no errors
	return true;
}

// ----------------------------------------------------------------------

bool CViewerDoc::loadSkeletonTemplate(const char *filename)
{
	try
	{
		//-- clear out old skeletal mesh appearance and any object appearances
		if (m_skeletalAppearanceTemplate)
		{
			AppearanceTemplateList::release(m_skeletalAppearanceTemplate);
			m_skeletalAppearanceTemplate = 0;
		}

		//-- Fetch the SkeletonTemplate.
		const SkeletonTemplate *const skeletonTemplate = SkeletonTemplateList::fetch(CrcLowerString(filename));
		if (!skeletonTemplate)
			return false;

		//-- construct a SkeletalAppearanceTemplate containing only the specified SkeletonTemplate filename.
		m_skeletalAppearanceTemplate = new SkeletalAppearanceTemplate();
		IGNORE_RETURN (AppearanceTemplateList::fetchNew(m_skeletalAppearanceTemplate));

		//-- Construct a LAT pathname that might exist for this SkeletonTemplate.
		// Strip directories.
		std::string  shortFilename;
		std::string  baseFilename;

		const char *lastDirectoryChar = strrchr(filename, '\\');
		if (lastDirectoryChar)
			shortFilename = lastDirectoryChar + 1;
		else
			shortFilename = filename;

		// Strip extension.
		std::string::size_type  extensionStartPos = shortFilename.find_last_of('.');
		if (static_cast<int>(extensionStartPos) != static_cast<int>(std::string::npos))
			baseFilename.assign(shortFilename, 0, extensionStartPos);
		else
			baseFilename = shortFilename;

		// Construct LAT filename.
		std::string latFilename("appearance\\lat\\");
		latFilename += baseFilename;
		latFilename += ".lat";

		//-- Check if LAT filename exists.
		bool  createAnimationController = false;

		if (TreeFile::exists(latFilename.c_str()))
		{
			createAnimationController = true;
		}
		else if (!baseFilename.empty() && (baseFilename[baseFilename.length() - 1] == 'b'))
		{
			//-- Super hacky, convert a LAT filename for a gender-neutral skeleton into a male variety.
			latFilename  = "appearance\\lat\\";
			latFilename += baseFilename;

			latFilename[latFilename.length() - 1] = 'm';

			latFilename += ".lat";

			if (TreeFile::exists(latFilename.c_str()))
				createAnimationController = true;
		}

		//-- If I found a LAT file for the skeleton, use it.
		if (createAnimationController)
		{
			//-- Add .skt to .lat mapping.
			m_skeletalAppearanceTemplate->setSktToLatMapping(CrcLowerString(skeletonTemplate->getName().getString()), CrcLowerString(latFilename.c_str()));
			m_skeletalAppearanceTemplate->setCreateAnimationController(true);
		}

		//-- Add the skeleton template to the SkeletalAppearanceTemplate.
		m_skeletalAppearanceTemplate->addSkeletonTemplate(filename, NULL);

		//-- create a SkeletalAppearance2
		SkeletalAppearance2 *const appearance = new SkeletalAppearance2 (m_skeletalAppearanceTemplate);
		skeletonTemplate->release();

		if (!appearance)
		{
			AppearanceTemplateList::release(m_skeletalAppearanceTemplate);
			m_skeletalAppearanceTemplate = 0;
			return false;
		}

		//-- view the skeleton, not the mesh
		appearance->setShowMesh (false);
		appearance->setShowSkeleton (true);

		//-- set the object's appearance
		object->setAppearance (appearance);
	}
	catch (FatalException& e)
	{
		object->setAppearance (0);
		CONSOLE_PRINT (e.getMessage ());
		CONSOLE_PRINT ("\r\n");
		return false;
	}

	//-- success
	return true;
}

// ----------------------------------------------------------------------

SkeletalAppearance2 *CViewerDoc::createAppearanceFromMeshGenerator(const char *filename)
{
	SkeletalAppearance2 *appearance = 0;

	try
	{
		//-- Load the mesh generator.
		const MeshGeneratorTemplate *const mgTemplate = MeshGeneratorTemplateList::fetch(CrcLowerString(filename));
		if (!mgTemplate)
			return 0;

		const BasicMeshGeneratorTemplate *const basicMgTemplate = mgTemplate->fetchBasicMeshGeneratorTemplate(0);
		if (!basicMgTemplate)
		{
			mgTemplate->release();
			return 0;
		}

		const MeshGenerator *const meshGenerator = basicMgTemplate->createMeshGenerator();
		basicMgTemplate->release();
		mgTemplate->release();

		if (!meshGenerator)
		{
			return 0;
		}

		//-- get referenced skeleton template name from mesh generator
		const SkeletalMeshGenerator *const skeletalMeshGenerator = dynamic_cast<const SkeletalMeshGenerator*>(meshGenerator);
		if (!skeletalMeshGenerator || !skeletalMeshGenerator->getReferencedSkeletonTemplateCount())
		{
			meshGenerator->release();
			return 0;
		}

		//-- get name for skeleton template
		const CrcLowerString &crcSkeletonTemplateName = skeletalMeshGenerator->getReferencedSkeletonTemplateName(0);

		//-- create a suitable SkeletalAppearanceTemplate
		// create the new template
		SkeletalAppearanceTemplate *const sat = new SkeletalAppearanceTemplate();
		IGNORE_RETURN(AppearanceTemplateList::fetchNew(sat));

		// populate it
		sat->addSkeletonTemplate(crcSkeletonTemplateName.getString(), "");
		sat->addMeshGenerator(filename);

		//-- create a SkeletalAppearance2 with the skeleton
		appearance = new SkeletalAppearance2(sat);
		skeletalMeshGenerator->release();
		AppearanceTemplateList::release(sat);

		if (!appearance)
			return 0;
	}
	catch (FatalException& e)
	{
		CONSOLE_PRINT (e.getMessage ());
		CONSOLE_PRINT ("\r\n");

		return 0;
	}
	
	return appearance;
}

// ----------------------------------------------------------------------
/**
 * Initialize the CustomizationData for the specified object, adding
 * any customization variables that influence the given Appearance.
 *
 * If a CustomizationDataProperty does not yet exist for the given
 * object, it will be added.
 *
 * @param object      this object's customization data will be modified.
 * @param appearance  CustomizationData variables influencing this appearance
 *                    will be added to the CustomizationData for object.
 */

void CViewerDoc::setupObjectCustomizationData(Object *object, Appearance *appearance)
{
	NOT_NULL(object);
	NOT_NULL(appearance);

	//-- get or create the CustomizationDataProperty
	CustomizationDataProperty *property = safe_cast<CustomizationDataProperty*>(object->getProperty( CustomizationDataProperty::getClassPropertyId()));
	if (!property)
	{
		// create the CustomizationDataProperty
		property = new CustomizationDataProperty(*object);
		object->addProperty(*property);
	}

	//-- get the CustomizationData for the object
	NOT_NULL(property);

	CustomizationData *const customizationData = property->fetchCustomizationData();
	NOT_NULL(customizationData);

	//-- add variables influencing the appearance to the CustomizationData
	appearance->addCustomizationVariables(*customizationData);

	//-- set the appearance's CustomizationData
	appearance->setCustomizationData(customizationData);

	//-- release local reference
	customizationData->release();
}

// ----------------------------------------------------------------------

bool CViewerDoc::loadMeshGenerator(const char *filename)
{
	try
	{
		//-- release existing SkeletalAppearanceTemplate data.
		if (m_skeletalAppearanceTemplate != NULL)
		{
			AppearanceTemplateList::release(m_skeletalAppearanceTemplate);
			m_skeletalAppearanceTemplate = 0;
		}

		//-- create the Appearance from the MeshGenerator file.
		SkeletalAppearance2 *const appearance = createAppearanceFromMeshGenerator(filename);
		if (!appearance)
		{
			object->setAppearance (0);
			return false;
		}

		//-- hang on to the SkeletalAppearanceTemplate.
		m_skeletalAppearanceTemplate = const_cast<SkeletalAppearanceTemplate*>(safe_cast<const SkeletalAppearanceTemplate*>(appearance->getAppearanceTemplate()));
		NOT_NULL(m_skeletalAppearanceTemplate);
		IGNORE_RETURN(AppearanceTemplateList::fetch(m_skeletalAppearanceTemplate));

		//-- view the mesh, not the skeleton
		appearance->setShowMesh(true);
		appearance->setShowSkeleton(false);

		//-- build the composite mesh
		IGNORE_RETURN(appearance->rebuildIfDirtyAndAvailable());

		//-- set the object's appearance
		object->setAppearance(appearance);
	}
	catch (FatalException& e)
	{
		object->setAppearance (0);
		CONSOLE_PRINT (e.getMessage ());
		CONSOLE_PRINT ("\r\n");
		return false;
	}

	//-- success
	return true;
}

// ----------------------------------------------------------------------

int getTargetAnimationTemplateName(const SkeletalAnimationTemplate* skeletalAnimationTemplate, std::vector<std::string>& animFileVector)
{
	int ret = 0;

	const ProxySkeletalAnimationTemplate* proxySkeletalAnimationTemplate = dynamic_cast<const ProxySkeletalAnimationTemplate *>(skeletalAnimationTemplate);

	if(proxySkeletalAnimationTemplate)
	{
		std::string elem = proxySkeletalAnimationTemplate->getTargetAnimationTemplateName().getString();
		if(!elem.empty())
		{
			std::string::size_type idx = elem.find_last_of("/");
			if(idx != std::string::npos)
			{
				elem = elem.substr(idx+1);
				animFileVector.push_back(elem);
				return 1;
			}
			else
			{
				return 0;
			}			
		}
		else
		{
			return 0;
		}
	}

	const TimeScaleSkeletalAnimationTemplate* timeScaleSkeletalAnimationTemplate = dynamic_cast<const TimeScaleSkeletalAnimationTemplate *>(skeletalAnimationTemplate);
	if(timeScaleSkeletalAnimationTemplate)
	{
		const SkeletalAnimationTemplate* baseSkeletalAnimationTemplate = timeScaleSkeletalAnimationTemplate->fetchBaseAnimationTemplate();
		if(baseSkeletalAnimationTemplate)
		{
			ret = getTargetAnimationTemplateName(baseSkeletalAnimationTemplate,animFileVector);
			baseSkeletalAnimationTemplate->release();
		}
		return ret;
	}

	const DirectionSkeletalAnimationTemplate* directionSkeletalAnimationTemplate = dynamic_cast<const DirectionSkeletalAnimationTemplate *>(skeletalAnimationTemplate);
	if(directionSkeletalAnimationTemplate)
	{
		uint32 numDirections = directionSkeletalAnimationTemplate->getNumDirectionTemplates();
		for(uint32 i = 0; i < numDirections; ++i)
		{
			const SkeletalAnimationTemplate* skelAnimationTemplate = directionSkeletalAnimationTemplate->fetchDirectionalAnimationTemplate(i);
			if(skelAnimationTemplate)
			{
				ret += getTargetAnimationTemplateName(skelAnimationTemplate,animFileVector);
				skelAnimationTemplate->release();
			}
		}
		return ret;
	}

	const SpeedSkeletalAnimationTemplate* speedSkeletalAnimationTemplate = dynamic_cast<const SpeedSkeletalAnimationTemplate *>(skeletalAnimationTemplate);
	if(speedSkeletalAnimationTemplate)
	{
		int32 numChoices = speedSkeletalAnimationTemplate->getSpeedChoiceCount();
		for(int32 i = 0; i < numChoices; ++i)
		{
			const SkeletalAnimationTemplate* skelAnimationTemplate = speedSkeletalAnimationTemplate->fetchSpeedChoiceAnimationTemplate(i);
			if(skelAnimationTemplate)
			{
				ret += getTargetAnimationTemplateName(skelAnimationTemplate,animFileVector);
				skelAnimationTemplate->release();
			}
		}
		return ret;
	}

	const StringSelectorSkeletalAnimationTemplate* stringSelectorSkeletalAnimationTemplate = dynamic_cast<const StringSelectorSkeletalAnimationTemplate *>(skeletalAnimationTemplate);
	if(stringSelectorSkeletalAnimationTemplate)
	{
		int32 numSelections = stringSelectorSkeletalAnimationTemplate->getSelectionAnimationTemplateCount();
		for(int32 i = 0; i < numSelections; ++i)
		{
			const SkeletalAnimationTemplate* skelAnimationTemplate = stringSelectorSkeletalAnimationTemplate->fetchSelectionAnimationTemplate(i);
			if(skelAnimationTemplate)
			{
				ret += getTargetAnimationTemplateName(skelAnimationTemplate,animFileVector);
				skelAnimationTemplate->release();
			}
		}
		return ret;
	}

	const PriorityBlendAnimationTemplate* priorityBlendAnimationTemplate = dynamic_cast<const PriorityBlendAnimationTemplate *>(skeletalAnimationTemplate);
	if(priorityBlendAnimationTemplate)
	{
		int32 numAnims = PriorityBlendAnimationTemplate::MAX_ANIMATION_COUNT;
		for(int32 i = 0; i < numAnims; ++i)
		{
			const SkeletalAnimationTemplate* skelAnimationTemplate = priorityBlendAnimationTemplate->fetchComponentAnimationTemplate(i);
			if(skelAnimationTemplate)
			{
				ret += getTargetAnimationTemplateName(skelAnimationTemplate,animFileVector);
				skelAnimationTemplate->release();
			}
		}
		return ret;
	}

	const ActionGeneratorSkeletalAnimationTemplate* actionGeneratorSkeletalAnimationTemplate = dynamic_cast<const ActionGeneratorSkeletalAnimationTemplate *>(skeletalAnimationTemplate);
	if(actionGeneratorSkeletalAnimationTemplate)
	{
		const SkeletalAnimationTemplate* skelAnimationTemplate = actionGeneratorSkeletalAnimationTemplate->fetchLoopingAnimationTemplate();
		if(skelAnimationTemplate)
		{
			ret += getTargetAnimationTemplateName(skelAnimationTemplate,animFileVector);
			skelAnimationTemplate->release();
		}
		return ret;
	}

	DEBUG_REPORT_LOG(true,("**** getTargetAnimationTemplateName ... unable to resolve template name\n"));

	return ret;
}

bool CViewerDoc::loadSkeletalAppearanceTemplate(const char *filename)
{
	// whenever we load a skeletal appearance template, load it in such a way
	// that we're setup to edit it.

	//-- load the file
	try
	{
		//-- clear out old skeletal mesh appearance and any object appearances
		if (m_skeletalAppearanceTemplate)
		{
			AppearanceTemplateList::release(m_skeletalAppearanceTemplate);
			m_skeletalAppearanceTemplate = 0;
		}

		if (object)
		{
			Appearance *const appearance = object->getAppearance();
			delete appearance;
			object->setAppearance(0);
		}

		m_skeletalAppearanceTemplateFilename = "";

		const AppearanceTemplate *at = AppearanceTemplateList::fetch(filename);
		m_skeletalAppearanceTemplate = const_cast<SkeletalAppearanceTemplate*>(dynamic_cast<const SkeletalAppearanceTemplate*>(at));
		if (!m_skeletalAppearanceTemplate)
		{
			if (at)
			{
				AppearanceTemplateList::release(at);
				DEBUG_REPORT_LOG(true, ("[%s] was an appearance, but not a skeletal appearance template\n", filename));
			}
			return false;
		}
	
		//-- make an appearance from default instance if one is available
		//   gracefully handle a broken instance specification here.
		Appearance* appearance = m_skeletalAppearanceTemplate->createAppearance();

		//-- create and fill the Object's CustomizationData property
		// setupObjectCustomizationData(object, appearance);

		if (!appearance)
			return false;

		//-- set the appearance
		object->setAppearance(appearance);

		// setup appearance template view name
		m_skeletalAppearanceTemplateFilename = filename;
		SkeletalAppearanceTemplateView *const view = getSkeletalAppearanceTemplateView();
		if (view)
		{
			view->OnUpdate(0, 0, 0);
		}

		// ...............................................
		// start - build up data for animation tree dialog

		m_skeletalAppearanceTemplate->preloadAssetsLight();
		std::vector<std::string> animFileVector[2];
		int totalAnimFiles = 0;
		const int numSkeletons = m_skeletalAppearanceTemplate->getSkeletonTemplateCount();
		DEBUG_REPORT_LOG(true,("numSkeletons = %d\n",numSkeletons));
		DEBUG_FATAL((numSkeletons > 2),("CViewerDoc::loadSkeletalAppearanceTemplate numSkeletons > 2 (%d)",numSkeletons));
		
		for(int i = 0; i < numSkeletons; ++i)
		{
			const SkeletalAppearanceTemplate::SkeletonTemplateInfo &sti = m_skeletalAppearanceTemplate->getSkeletonTemplateInfo(i);
			const CrcLowerString &skeletonTemplateName = m_skeletalAppearanceTemplate->getSkeletonTemplateName(sti);
			//DEBUG_REPORT_LOG(true,("(%d)skeletonTemplateName = [%s]\n",i,skeletonTemplateName.getString()));

			const CrcLowerString &latName = m_skeletalAppearanceTemplate->lookupLatForSkeleton(skeletonTemplateName);
			//DEBUG_REPORT_LOG(true,("(%d)latName = [%s]\n",i,latName.getString()));
			
			if(latName.isEmpty())
				continue;

			const LogicalAnimationTableTemplate *const latTemplate = LogicalAnimationTableTemplateList::fetch(latName);
		
			const int numAnimations = latTemplate->getAnimationCount();
			//DEBUG_REPORT_LOG(true,("(%d)numAnimations = %d\n",i,numAnimations));
			
			for(int animIndex = 0; animIndex < numAnimations; ++animIndex)
			{
				const CrcLowerString &animationName = latTemplate->getLogicalAnimationName(animIndex);
				//DEBUG_REPORT_LOG(true,("(%d)(%d)animationName=[%s]\n",i,animIndex,animationName.getString()));	
				const SkeletalAnimationTemplate *const skeletalAnimationTemplate = latTemplate->fetchConstAnimationTemplate(animationName);
		
				if(skeletalAnimationTemplate)
				{
					totalAnimFiles += getTargetAnimationTemplateName(skeletalAnimationTemplate,animFileVector[i]);	
					skeletalAnimationTemplate->release();
				}
			}
			latTemplate->release();
		}

		for(int i = 0; i < 2; ++i)
		{
			if(!animFileVector[i].size())
				continue;

			std::sort(animFileVector[i].begin(),animFileVector[i].end());
			//DEBUG_REPORT_LOG(true,("sizeof animFileVector[%d]==%d, totalAnimFiles==%d\n",i,animFileVector[i].size(),totalAnimFiles));
		
			// normalize and copy into ms_animationFiles
			m_animationFileList[i]->clear();
			m_animationFileList[i]->resize(animFileVector[i].size()); 
			std::vector<std::string>::iterator pos = std::unique_copy(animFileVector[i].begin(),animFileVector[i].end(),m_animationFileList[i]->begin());
			m_animationFileList[i]->erase(pos,m_animationFileList[i]->end());
		}
		// end - build up data for animation tree dialog
		// .............................................
	}
	catch (FatalException& e)
	{
		object->setAppearance (0);
		CONSOLE_PRINT (e.getMessage ());
		CONSOLE_PRINT ("\r\n");
		return false;
	}

	//-- success
	return true;
}

// ----------------------------------------------------------------------

bool CViewerDoc::loadStandardAppearance(const char *filename)
{
	// try to load an appearance through standard channels
	try
	{
		const AppearanceTemplate* at = AppearanceTemplateList::fetch (filename);
		Appearance* appearance       = at->createAppearance ();

		object->setAppearance (appearance);
		AppearanceTemplateList::release (at);
	}
	catch (FatalException& e)
	{
		object->setAppearance (0);
		CONSOLE_PRINT (e.getMessage ());
		CONSOLE_PRINT ("\r\n");

		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

Object * CViewerDoc::attachStandardAppearanceToHardpoint(Object * parent, char const * const filename, char const * const hardpointName)
{
	HardpointObject * hardpointObject = new HardpointObject(CrcLowerString(hardpointName));

	if (!parent)
		parent = object;

	try
	{
		const AppearanceTemplate* at = AppearanceTemplateList::fetch (filename);
		Appearance* appearance = at->createAppearance ();

		hardpointObject->setAppearance (appearance);

		parent->addChildObject_o(hardpointObject);

		AppearanceTemplateList::release (at);
	}
	catch (FatalException& e)
	{
		delete hardpointObject;
		hardpointObject = 0;

		CONSOLE_PRINT (e.getMessage ());
		CONSOLE_PRINT ("\r\n");

		return 0;
	}

	return hardpointObject;
}

// ----------------------------------------------------------------------

bool CViewerDoc::loadPortalObject(const char *filename)
{
	try
	{
		PortalProperty *portalProperty = new PortalProperty(*object, filename);
		object->addProperty(*portalProperty);
		portalProperty->createAppearance();
		portalProperty->clientSinglePlayerInitializeFirstTimeObject();
	
#if 1
		// hack to load lights into cells.  This is usually done by the client's CellObject
		const int numberOfCells = portalProperty->getNumberOfCells();
		for (int j = 0; j < numberOfCells; ++j)
		{
#if 0

			const int numberOfLights = portalProperty->getNumberOfLights(j);
			for (int i = 0; i < numberOfLights; ++i)
			{
				const PortalProperty::LightData &lightData = portalProperty->getLightData(j, i);

				Light *light = NULL;

				switch (lightData.type)
				{
					case PortalProperty::LightData::T_ambient:
						light = new Light(Light::T_ambient, lightData.diffuseColor);
						break;

					case PortalProperty::LightData::T_parallel:
						light = new Light(Light::T_parallel, lightData.diffuseColor);
						light->setTransform_o2p(lightData.transform);
						light->setSpecularColor(lightData.specularColor);
						break;

					case PortalProperty::LightData::T_point:
						light = new Light(Light::T_point, lightData.diffuseColor);
						light->setTransform_o2p(lightData.transform);
						light->setSpecularColor(lightData.specularColor);
						light->setConstantAttenuation(lightData.constantAttenuation);
						light->setLinearAttenuation(lightData.linearAttenuation);
						light->setQuadraticAttenuation(lightData.quadraticAttenuation);
						break;
				}

				light->setParentCell(portalProperty->getCell(j));
				m_lightList->push_back(light);
			}
#else
			if (j != 0)
			{
				CellProperty *const cellProperty = portalProperty->getCell(j);

				Light *light = new Light(Light::T_ambient, VectorArgb::solidWhite);
				light->setParentCell(cellProperty);
				m_lightList->push_back(light);

				const Texture *const environmentTexture = ViewerPreferences::getEnvironmentMapName() ? TextureList::fetch(ViewerPreferences::getEnvironmentMapName()) : TextureList::fetchDefaultEnvironmentTexture();
				cellProperty->setEnvironmentTexture(environmentTexture);
				environmentTexture->release();
			}
#endif
		}
#endif
	}
	catch (FatalException& e)
	{
		object->setAppearance (0);
		CONSOLE_PRINT (e.getMessage ());
		CONSOLE_PRINT ("\r\n");

		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

BOOL CViewerDoc::load (LPCTSTR lpszPathName)
{
	filename = lpszPathName;

	delete object;
	object = new Object();
	object->setNetworkId(NetworkId(++ms_networkId));
	RenderWorld::addObjectNotifications(*object);

	//-- strip off extension
	const char     *extensionStart = strrchr(lpszPathName, '.');
	FileTypeLoader  fileTypeLoader = &CViewerDoc::loadStandardAppearance;

	if (extensionStart)
	{
		//-- check if we have special load code for this extension type
		FileTypeLoader  specialTypeLoader = getFileTypeLoader(extensionStart+1);
		if (specialTypeLoader)
		{
			// we've got a file type loader that overrides default behavior. use it.
			fileTypeLoader = specialTypeLoader;
		}
	}

	unsigned long const bytesBefore = MemoryManager::getCurrentNumberOfBytesAllocated();
	bool const asynchronousLoaderEnabled = AsynchronousLoader::isEnabled();
	if (!ms_buildingAsynchronousLoaderData)
		AsynchronousLoader::disable();

	unsigned long const startTime = Clock::timeMs();

	//-- go ahead and load the file
	FatalSetThrowExceptions (m_fatalThrowsExceptionsOnLoad);
	const bool loadResult = (this->*fileTypeLoader)(lpszPathName);
	FatalSetThrowExceptions (false);

	unsigned long const totalTime = Clock::timeMs() - startTime;

	if (!ms_buildingAsynchronousLoaderData && asynchronousLoaderEnabled)
		AsynchronousLoader::enable();

	int const bytesTotal = MemoryManager::getCurrentNumberOfBytesAllocated() - bytesBefore;

	{
		CString fileName(lpszPathName);
		int const index = std::max(fileName.ReverseFind('/'), fileName.ReverseFind('\\'));
		if (index != -1)
			fileName = fileName.Right(fileName.GetLength() - index - 1);

		REPORT_LOG(true, ("CViewerDoc::load: [%s] tool %1.2f seconds to load and consumed %i bytes\n", fileName, static_cast<float>(totalTime) / 1000.f, bytesTotal));
	}

	//-- add customization variables
	if (loadResult)
	{
		if (object)
		{
			Appearance *const appearance = object->getAppearance();
			if (appearance)
			{
				//-- create and fill the Object's CustomizationData property
				setupObjectCustomizationData(object, appearance);
			}
		}
	}

	object->addProperty(*(new CollisionProperty(*object)));

	return (loadResult ? TRUE : FALSE);
}

// ----------------------------------------------------------------------

BOOL CViewerDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	return load (lpszPathName);
}

// ----------------------------------------------------------------------

BOOL CViewerDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CDocument::OnSaveDocument(lpszPathName);
}

// ----------------------------------------------------------------------

static void ApplyAnimationDrivenLocomotion(Object &object)
{
	SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(object.getAppearance());
	if (!appearance)
		return;

	TransformAnimationController *const animationController = appearance->getAnimationController();
	if (!animationController)
		return;
	
	//-- get locomotion
	Quaternion  locomotionRotation;
	Vector      locomotionTranslation;

	animationController->getObjectLocomotion(locomotionRotation, locomotionTranslation);

	//-- apply rotation
	// convert to Transform
	Transform animationRotationTransform(Transform::IF_none);
	locomotionRotation.getTransform(&animationRotationTransform);

	// create resultant Transform
	Transform objectToParent = object.getTransform_o2p();
	Transform rotatedObjectToParent(Transform::IF_none);
	rotatedObjectToParent.multiply(objectToParent, animationRotationTransform);

	// apply rotation transform to Object
	object.setTransform_o2p(rotatedObjectToParent);

	//-- apply translation (translation is relative to the Object's frame of reference, not the parent's)
	object.move_o(locomotionTranslation);
}

// ----------------------------------------------------------------------

void CViewerDoc::alter(real elapsedTime)
{
	// don't alter if no object
	if (!object)
		return;

	// don't alter if no appearance or if appearance is not an animating appearance
//	if (!object->getAppearance () || (dynamic_cast<const AnimatingMeshAppearance*> (object->getAppearance ()) == 0))
//		return;
	
	messageQueue->beginFrame();

	FatalSetThrowExceptions(false);
	try
	{
		const bool remove = !object->alter(elapsedTime);
		// -TRF- handle object remove?  skip for now
		UNREF(remove);
		object->conclude();
	}
	catch (FatalException& e)
	{
		FatalSetThrowExceptions(false);
		++badAlterCount;
		if (badAlterCount == 1)
		{
			// only display this message the first time
			MessageBox(AfxGetApp()->m_pMainWnd->m_hWnd, e.getMessage(), "Error playing animation, is animation for this mesh?", MB_ICONSTOP);
		}
		return;
	}
	FatalSetThrowExceptions(false);
	badAlterCount = 0;

	// apply animation-driven locomotion
	if (m_locomotionEnabled)
		ApplyAnimationDrivenLocomotion(*object);

	// scan message queue
	int   messageId;
	real  value;
	for (int index = 0; index < messageQueue->getNumberOfMessages(); ++index)
	{
		messageQueue->getMessage(index, &messageId, &value);
		REPORT_LOG(true, ("anim message sent: %d\n", messageId));
		UNREF(value);
	}

	// tell all graphics views to update (really only want to do this if alter changed
	// anything, but we currently don't have a way to get that info back.

	POSITION  viewPosition = GetFirstViewPosition();
	while (viewPosition != NULL)
	{
		CView *const view = GetNextView(viewPosition);

		// we'll update a CViewerView during alter()
		CViewerView *viewerView = dynamic_cast<CViewerView*>(view);
		if (viewerView)
		{
  			viewerView->OnUpdate(NULL, 0, 0);
			viewerView->alter (elapsedTime);
		}
	}
}

// ----------------------------------------------------------------------
// stop playing any animation that might be running on the object
//
// Remarks:
//
//   Currently this is called when the object fails to render.

void CViewerDoc::stopAnimation(void)
{
	if (!object)
		return;

	{
#if 1
		// -TRF- add this
#else
		SkeletalAppearance2* sa= dynamic_cast<SkeletalAppearance2*> (object->getAppearance());
		if (sa)
			sa->stop ();
#endif
	}
}

// ----------------------------------------------------------------------

void CViewerDoc::playAnim(const char* filename,bool queueAnimation, bool usePrimarySkeleton)
{
	if(!object)
		return;

	Appearance *const appearance = object->getAppearance();
	
	if (!appearance)
		return;
	
	if(!usePrimarySkeleton)
	{
		DEBUG_REPORT_LOG(true,("non-primary skeleton animation playing not supported yet\n"));
		return;
	}

	//-- handle skeleton 2 skeletal appearance
	SkeletalAppearance2 *const sma = dynamic_cast<SkeletalAppearance2*>(appearance);
	if (sma)
	{
		//-- get the animation controller
		TransformAnimationController *const baseController = sma->getAnimationController();
		if (!baseController)
			return;

		//-- Get a TrackAnimationController from the appearance.
		TrackAnimationController *animationController = 0;
		animationController = dynamic_cast<TrackAnimationController*>(baseController);

		//-- Check for state hierarchy animation controller
		if (!animationController)
		{
			StateHierarchyAnimationController *const shAnimationController = dynamic_cast<StateHierarchyAnimationController*>(baseController);
			if (shAnimationController)
				animationController = &(shAnimationController->getTrackAnimationController());
		}

		//-- fetch the animation
		const TransformNameMap *const primarySkeletonTransformNameMap = sma->getAnimationResolver().getPrimarySkeletonTransformNameMap();

		if (!primarySkeletonTransformNameMap)
		{
			WARNING(true, ("Appearance's primary skeleton does not have a transform name map.  Can't create animation."));
			return;
		}
			
		const SkeletalAnimationTemplate* skeletalAnimationTemplate = SkeletalAnimationTemplateList::fetch(CrcLowerString(filename));

		SkeletalAnimation *const skeletalAnimation = skeletalAnimationTemplate->fetchSkeletalAnimation(sma->getAnimationEnvironment(), *primarySkeletonTransformNameMap);

		if (!skeletalAnimation)
		{
			WARNING(true, ("Failed to fetch a skeletal animation from the template."));
			return;
		}

		// JU_TODO: check defaults
		//-- submit for playing
		const TrackAnimationController::PlayMode   playMode  = queueAnimation ?  TrackAnimationController::PM_queue : TrackAnimationController::PM_immediate;
		//const TrackAnimationController::PlayMode playMode = TrackAnimationController::PM_immediate;
		//const TrackAnimationController::BlendMode  blendMode = (sak->blendTime > 0.0f) ? TrackAnimationController::BM_linearBlend : TrackAnimationController::BM_noBlend;
		const TrackAnimationController::BlendMode  blendMode = TrackAnimationController::BM_linearBlend;

		//skeletalAnimation->setPlaybackFramesPerSecond(sak->timeScale * skeletalAnimation->getRecordedFramesPerSecond());
		skeletalAnimation->setPlaybackFramesPerSecond(1.0f /*ju_todo: check def*/ * skeletalAnimation->getRecordedFramesPerSecond());
		//animationController->playAnimation(sak->trackId, skeletalAnimation, playMode, sak->isLooping, blendMode, sak->blendTime, NULL);
		animationController->playAnimation(ms_primaryTrackId, skeletalAnimation, playMode, true/*ju_todo:sak->isLooping*/, blendMode, 0.5f/*ju_todo:sak->blendTime*/, NULL);
		// JU_TODO: end check defaults

		//-- release local animation reference
		skeletalAnimation->release();
		skeletalAnimationTemplate->release();
	}
	return;
}


void CViewerDoc::processKeyPress(UINT key)
{
	if (!object)
		return;

	Appearance *const appearance = object->getAppearance();
	if (!appearance)
		return;

	//-- handle skeleton 2 skeletal appearance
	SkeletalAppearance2 *const sma = dynamic_cast<SkeletalAppearance2*>(appearance);
	if (sma)
	{
		//-- get the animation controller
		TransformAnimationController *const baseController = sma->getAnimationController();
		if (!baseController)
			return;

		//-- Get a TrackAnimationController from the appearance.
		TrackAnimationController *animationController = 0;
		animationController = dynamic_cast<TrackAnimationController*>(baseController);

		//-- Check for state hierarchy animation controller
		if (!animationController)
		{
			StateHierarchyAnimationController *const shAnimationController = dynamic_cast<StateHierarchyAnimationController*>(baseController);
			if (shAnimationController)
				animationController = &(shAnimationController->getTrackAnimationController());
		}

		//-- get the animation associated with this key
		SkeletalAnimationKey::Map::const_iterator iter = skeletalAnimationKeyMap.find(static_cast<char>(key));
		if (iter == skeletalAnimationKeyMap.end())
			return;
		const SkeletalAnimationKey* const sak = (*iter).second;

		//-- fetch the animation
		const TransformNameMap *const primarySkeletonTransformNameMap = sma->getAnimationResolver().getPrimarySkeletonTransformNameMap();
		if (!primarySkeletonTransformNameMap)
		{
			WARNING(true, ("Appearance's primary skeleton does not have a transform name map.  Can't create animation."));
			return;
		}

		SkeletalAnimation *const skeletalAnimation = sak->skeletalAnimationTemplate->fetchSkeletalAnimation(sma->getAnimationEnvironment(), *primarySkeletonTransformNameMap);
		if (!skeletalAnimation)
		{
			WARNING(true, ("Failed to fetch a skeletal animation from the template."));
			return;
		}

		//-- submit for playing
		const TrackAnimationController::PlayMode   playMode  = sak->playForced ? TrackAnimationController::PM_immediate : TrackAnimationController::PM_queue;
		const TrackAnimationController::BlendMode  blendMode = (sak->blendTime > 0.0f) ? TrackAnimationController::BM_linearBlend : TrackAnimationController::BM_noBlend;

		skeletalAnimation->setPlaybackFramesPerSecond(sak->timeScale * skeletalAnimation->getRecordedFramesPerSecond());
		animationController->playAnimation(sak->trackId, skeletalAnimation, playMode, sak->isLooping, blendMode, sak->blendTime, NULL);

		//-- release local animation reference
		skeletalAnimation->release();
	}
}

// ======================================================================

void CViewerDoc::OnAnimationAddkeymappingSkeletalanimation() 
{
	// TODO: Add your command handler code here
	DialogMapAnimation  dlg;

	// set default values
	dlg.animationFileName = static_cast<CViewerApp*>(AfxGetApp())->getAnimationMapFilename();
	dlg.blendTime         = defaultBlendTime;
	dlg.key               = defaultKey;
	dlg.timeScale         = defaultTimeScale;
	dlg.m_weight          = defaultWeight;
	dlg.playModeIndex     = defaultPlayModeIndex;
	dlg.isLooping         = defaultIsLooping;
	dlg.isInterruptible   = defaultIsInterruptible;
	dlg.m_soundFilename   = defaultSoundFilename;

	mapSkeletalAnimation (dlg);

	// save defaults for next time around
	static_cast<CViewerApp*>(AfxGetApp())->setAnimationMapFilename(dlg.animationFileName);
	defaultBlendTime         = dlg.blendTime;
	defaultKey               = CString(static_cast<char>(dlg.key[0]+1));
	defaultTimeScale         = dlg.timeScale;
	defaultPlayModeIndex     = dlg.playModeIndex;
	defaultWeight            = dlg.m_weight;
	defaultIsLooping         = dlg.isLooping;
	defaultIsInterruptible   = dlg.isInterruptible;
	defaultSoundFilename     = dlg.m_soundFilename;
}

// ----------------------------------------------------------------------

void CViewerDoc::mapSkeletalAnimation (DialogMapAnimation& dlg)
{
	if (!object)
		return;

	if (!dynamic_cast<SkeletalAppearance2*>(object->getAppearance()))
		return;

	if (dlg.DoModal() == IDOK)
	{
		if (!dlg.key.GetLength())
		{
			MessageBox(NULL, "User must specify a key", "Error", MB_OK | MB_ICONSTOP);
			return;
		}

		// load the animation 
		FatalSetThrowExceptions(true);
		try
		{
			// add key to key map
			SkeletalAnimationKey::Map::iterator iter = skeletalAnimationKeyMap.find(dlg.key[0]);
			// simplify this: delete the map entry if it already exists, then recreate.
			if (iter != skeletalAnimationKeyMap.end())
			{
				delete (*iter).second;
				skeletalAnimationKeyMap.erase(iter);
			}

			// look up the track id
			AnimationTrackId  trackId(AnimationTrackId::cms_invalid);

			const bool lookupSuccess = TrackAnimationController::getTrackIdByName(dlg.getTrackName(), trackId);
			if (!lookupSuccess)
			{
				MessageBox(NULL, "failed to lookup track name", "Track Name Lookup Failure", MB_OK | MB_ICONSTOP);
				return;
			}

			// create the new map entry
			const bool playForced = (dlg.playModeIndex == 1);
			skeletalAnimationKeyMap[dlg.key[0]]  = new SkeletalAnimationKey(dlg.key[0], dlg.animationFileName, dlg.timeScale, (dlg.isLooping != FALSE), playForced, dlg.blendTime, trackId);

			//-- tell the animation view about it
			GetMainFrame ()->updateAnimationView (0, skeletalAnimationKeyMap);
		}
		catch (FatalException& e)
		{
			FatalSetThrowExceptions (false);
			MessageBox (AfxGetApp()->m_pMainWnd->m_hWnd, e.getMessage(), "Error Loading File", MB_OK | MB_ICONSTOP);
			return;
		}

		FatalSetThrowExceptions(false);
	}	
}

//-------------------------------------------------------------------

void CViewerDoc::OnUpdateAnimationAddkeymappingSkeletalanimation(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	// only enable when our object's appearance is a AnimatingMeshAppearance
	BOOL enabled = FALSE;

	if (object)
	{
		Appearance *const appearance = object->getAppearance();
		enabled = dynamic_cast<SkeletalAppearance2*>(appearance) ? TRUE : FALSE;
	}
	pCmdUI->Enable(enabled);
}

//-------------------------------------------------------------------

void CViewerDoc::OnAnimationSkeletalanimationStopwearing() 
{
	SkeletalAppearance2 *sa = dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
	if (!sa)
		return;

	std::vector<SkeletalAppearance2*>  appearancesToDelete;

	//-- collect all appearances attached to the skeletal appearance.
	const int appearanceCount = sa->getWearableCount();
	appearancesToDelete.reserve(static_cast<size_t>(appearanceCount));

	{
		for (int i = 0; i < appearanceCount; ++i)
		{
			// we take away constness because we know we created these appearances for attachment
			appearancesToDelete.push_back(sa->getWearableAppearance(i));
		}
	}

	//-- detach and delete the appearances
	{
		for (int i = 0; i < appearanceCount; ++i)
		{
			// @todo fix this, currently these appearances are not attached to objects.
			// sa->stopWearing(appearancesToDelete[static_cast<size_t>(i)]);
			delete appearancesToDelete[static_cast<size_t>(i)];
		}
	}

	//-- rebuild mesh
	IGNORE_RETURN(sa->rebuildIfDirtyAndAvailable());
}

//-------------------------------------------------------------------

void CViewerDoc::OnUpdateAnimationSkeletalanimationStopwearing(CCmdUI* pCmdUI) 
{
	BOOL enabled = (object && (dynamic_cast<SkeletalAppearance2*>(object->getAppearance()) != 0)) ? TRUE : FALSE;
	pCmdUI->Enable(enabled);
}

//-------------------------------------------------------------------

void CViewerDoc::OnAnimationSkeletalanimationWear () 
{
	SkeletalAppearance2 *sa = dynamic_cast<SkeletalAppearance2*> (object->getAppearance ());
	if (!sa)
		return;

	//-- select item to wear
	CFileDialog dlg(TRUE, "", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR), "Skeletal Appearance (*.sat)|*.sat|Mesh Generators (*.mgn;*.lmg)|*.mgn;*.lmg||");

	dlg.m_ofn.lpstrInitialDir = "";
	dlg.m_ofn.lpstrTitle      = "Select Skeletal Appearance to Wear";

	if (dlg.DoModal () != IDOK)
		return;

	//-- load skeletal mesh generators and skeletal appearance templates
	std::string  pathName (dlg.GetPathName ());

	// convert to lower case
	std::transform(pathName.begin(), pathName.end(), pathName.begin(), tolower);

	const size_t size = pathName.size();
	if (size < 4)
		return;

	SkeletalAppearance2 *wearableAppearance = 0;

	if ((pathName[size-3] == 's') && (pathName[size-2] == 'a') && (pathName[size-1] == 't'))
	{
		// handle skeletal appearance
		Appearance *const newAppearance = AppearanceTemplateList::createAppearance (pathName.c_str ());
		if (!newAppearance)
			return;

		wearableAppearance = dynamic_cast<SkeletalAppearance2*> (newAppearance);
		if (!wearableAppearance)
		{
			delete newAppearance;
			return;
		}
	}
	else if ((pathName[size-3] == 'm') && (pathName[size-2] == 'g') && (pathName[size-1] == 'n'))
	{
		// handle mesh generator
		wearableAppearance = createAppearanceFromMeshGenerator (pathName.c_str ());
	}

#if 1
	WARNING(true, ("wear/unware functionality disabled."));
#else
	// @todo fix this.  API change requires an Object instance now.
	if (wearableAppearance)
	{
		sa->wear (wearableAppearance);
		sa->rebuildIfDirty();
	}
#endif
}

//-------------------------------------------------------------------

void CViewerDoc::OnUpdateAnimationSkeletalanimationWear(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	UNREF (pCmdUI);	
}

//-------------------------------------------------------------------

void CViewerDoc::newAnimation (void)
{
	// -TRF- to do
#if 0
	SkeletalAppearance2 *sa = dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
	if (sa)
		OnAnimationAddkeymappingSkeletalanimation ();
#endif
}

//-------------------------------------------------------------------

void CViewerDoc::removeAnimation (char key)
{
	UNREF(key);

#if 0
	// -TRF- to do
	SkeletalAppearance2 *sa = dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
	if (sa)
	{
		//-- find key in list
		SkeletalAnimationKey::Map::iterator iter = skeletalAnimationKeyMap.find(static_cast<char>(key));
		if (iter == skeletalAnimationKeyMap.end())
			return;

		//--
		skeletalAnimationKeyMap.erase (iter);

		//-- tell the animation view about it
		GetMainFrame ()->updateAnimationView (0, skeletalAnimationKeyMap);
	}
#endif
}

//-------------------------------------------------------------------

void CViewerDoc::editAnimation (char key)
{
	UNREF(key);

	// -TRF- to do
#if 0
	SkeletalAppearance2 *sa = dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
	if (sa)
	{
		//-- find key in list
		SkeletalAnimationKey::Map::iterator iter = skeletalAnimationKeyMap.find (key);
		if (iter != skeletalAnimationKeyMap.end())
		{
			// mapping already exists for key
			SkeletalAnimationKey* sak = (*iter).second;

			// TODO: Add your command handler code here
			DialogMapAnimation  dlg;

			// set default values
			dlg.animationFileName = sak->filename;
			dlg.blendTime         = sak->blendTime;
			dlg.key               = sak->key;
			dlg.timeScale         = sak->timeScale;
			dlg.m_weight          = sak->weight;
			dlg.playModeIndex     = sak->playForced ? 1 : 0;
			dlg.isLooping         = sak->isLooping;
			dlg.isInterruptible   = sak->isInterruptible;

			mapSkeletalAnimation (dlg);
		}

		//-- tell the animation view about it
		GetMainFrame ()->updateAnimationView (0, skeletalAnimationKeyMap, sa);
	}
#endif
}

//-------------------------------------------------------------------

void CViewerDoc::OnAnimationMeshanimationLoadanimationset() 
{
}

//-------------------------------------------------------------------

void CViewerDoc::OnUpdateAnimationMeshanimationLoadanimationset(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable (false);
}

//-------------------------------------------------------------------

void CViewerDoc::OnAnimationMeshanimationSaveanimationset() 
{
}

//-------------------------------------------------------------------

void CViewerDoc::OnUpdateAnimationMeshanimationSaveanimationset(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable (false);
}

//-------------------------------------------------------------------

void CViewerDoc::clearSkeletalAnimationList (void)
{
	SkeletalAnimationKey::Map::iterator begin;
	SkeletalAnimationKey::Map::iterator end = skeletalAnimationKeyMap.end();
	while ((begin = skeletalAnimationKeyMap.begin()) != end)
	{
		delete begin->second;
		skeletalAnimationKeyMap.erase(begin);
	}

	skeletalAnimationKeyMap.clear ();

	//-- tell the animation view about it
	GetMainFrame ()->updateAnimationView (0, skeletalAnimationKeyMap);
}

//-------------------------------------------------------------------

void CViewerDoc::skeletalAnimationSet_load0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterChunk (TAG_DATA);

			const float blendTime = iff.read_float ();

			char* filename = iff.read_string ();
			FileName::stripPath (filename, istrlen (filename) + 1);
			FileName fn (FileName::P_animation, filename);
			CString tempFn = fn;

			const bool  isInterruptible = iff.read_int32 () != 0;
			const bool  isLooping       = iff.read_int32 () != 0;
			const char  key             = iff.read_int8 ();
			const bool  playForced      = iff.read_int32 () != 0;
			const float timeScale       = iff.read_float ();
			const float weight          = iff.read_float ();

			iff.exitChunk ();

			skeletalAnimationKeyMap[key] = new SkeletalAnimationKey(key, filename, timeScale, isLooping, playForced, blendTime, ms_primaryTrackId);
			UNREF(weight);
			UNREF(isInterruptible);

			delete [] filename;
			filename = 0;
		}
	
	iff.exitForm ();
}

// ----------------------------------------------------------------------

void CViewerDoc::skeletalAnimationSet_load0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

	int n = iff.getNumberOfBlocksLeft ();
	int i;
	for (i = 0; i < n; i++)
	{
		iff.enterChunk (TAG_DATA);

		float blendTime       = iff.read_float ();
		char* filename        = iff.read_string ();
		bool  isInterruptible = iff.read_int32 () != 0;
		bool  isLooping       = iff.read_int32 () != 0;
		char  key             = iff.read_int8 ();
		bool  playForced      = iff.read_int32 () != 0;
		float timeScale       = iff.read_float ();
		float weight          = iff.read_float ();

		iff.exitChunk ();

		skeletalAnimationKeyMap[key] = new SkeletalAnimationKey(key, filename, timeScale, isLooping, playForced, blendTime, ms_primaryTrackId);
		UNREF(weight);
		UNREF(isInterruptible);

		delete [] filename;
		filename = 0;
	}
	
	iff.exitForm ();
}

// ----------------------------------------------------------------------

void CViewerDoc::skeletalAnimationSet_load0002 (Iff& iff)
{
	iff.enterForm (TAG_0002);

	int n = iff.getNumberOfBlocksLeft ();
	int i;
	for (i = 0; i < n; i++)
	{
		iff.enterChunk (TAG_DATA);

		float blendTime       = iff.read_float ();
		char* filename        = iff.read_string ();
		bool  isInterruptible = iff.read_int32 () != 0;
		bool  isLooping       = iff.read_int32 () != 0;
		char  key             = iff.read_int8 ();
		bool  playForced      = iff.read_int32 () != 0;
		float timeScale       = iff.read_float ();
		float weight          = iff.read_float ();
		char* soundFilename   = iff.read_string ();

		iff.exitChunk ();

		skeletalAnimationKeyMap[key] = new SkeletalAnimationKey(key, filename, timeScale, isLooping, playForced, blendTime, ms_primaryTrackId);
		UNREF(weight);
		UNREF(isInterruptible);
		UNREF(soundFilename);

		delete [] filename;
		filename = 0;

		delete [] soundFilename;
		soundFilename = 0;
	}
	
	iff.exitForm ();
}

// ----------------------------------------------------------------------

void CViewerDoc::OnAnimationSkeletalanimationLoadanimationset() 
{
	// TODO: Add your command handler code here
	CFileDialog dlg (true, 0, 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER));
	if (dlg.DoModal () == IDOK)
	{
		clearSkeletalAnimationList ();

		{
			FatalSetThrowExceptions (true);
			try
			{
				Iff iff (dlg.GetPathName ());

					iff.enterForm (TAG (S,K,S,T));

						switch (iff.getCurrentName ())
						{
						case TAG_0000:
							skeletalAnimationSet_load0000 (iff);
							break;

						case TAG_0001:
							skeletalAnimationSet_load0001 (iff);
							break;

						case TAG_0002:
							skeletalAnimationSet_load0002 (iff);
							break;
						}

					iff.exitForm ();

				//-- tell the animation view about it
				GetMainFrame ()->updateAnimationView (0, skeletalAnimationKeyMap);
			}
			catch (FatalException& e)
			{
				CString tmp;
				tmp = "Error loading ";
				tmp += dlg.GetPathName ();
				MessageBox(0, e.getMessage(), tmp, MB_ICONSTOP);
			}
			FatalSetThrowExceptions (false);
		}
	}
}

//-------------------------------------------------------------------

void CViewerDoc::OnUpdateAnimationSkeletalanimationLoadanimationset(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	SkeletalAppearance2 *const sa = dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
	pCmdUI->Enable (sa != 0);
}

//-------------------------------------------------------------------

void CViewerDoc::OnAnimationSkeletalanimationSaveanimationset() 
{
	// TODO: Add your command handler code here
	CFileDialog dlg (false, 0, 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER));
	if (dlg.DoModal () == IDOK)
	{
		FatalSetThrowExceptions (true);
		try
		{
			Iff iff (1000);

			iff.insertForm (TAG (S,K,S,T));

				iff.insertForm (TAG_0002);

				for (SkeletalAnimationKey::Map::const_iterator iter = skeletalAnimationKeyMap.begin (); iter != skeletalAnimationKeyMap.end(); ++iter)
				{
					const SkeletalAnimationKey* const sak = (*iter).second;

						iff.insertChunk (TAG_DATA);

							iff.insertChunkData (sak->blendTime);
							iff.insertChunkString (sak->filename);
							iff.insertChunkData (sak->isInterruptible ? static_cast<int32> (1) : static_cast<int32> (0));
							iff.insertChunkData (sak->isLooping ? static_cast<int32> (1) : static_cast<int32> (0));
							iff.insertChunkData (sak->key);
							iff.insertChunkData (sak->playForced ? static_cast<int32> (1) : static_cast<int32> (0));
							iff.insertChunkData (sak->timeScale);
							iff.insertChunkData (sak->weight);
							iff.insertChunkString (sak->soundFilename);

						iff.exitChunk ();
				}

				iff.exitForm ();

			iff.exitForm ();
				
			iff.write (dlg.GetPathName ());
		}
		catch (FatalException& e)
		{
			CString tmp;
			tmp = "Error saving ";
			tmp += dlg.GetPathName ();
			MessageBox(0, e.getMessage(), tmp, MB_ICONSTOP);
		}
		FatalSetThrowExceptions (false);
	}
}

//-------------------------------------------------------------------

void CViewerDoc::OnUpdateAnimationSkeletalanimationSaveanimationset(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	const SkeletalAppearance2 *const sa = dynamic_cast<const SkeletalAppearance2*>(object->getAppearance());
	pCmdUI->Enable (sa != 0);
}

//-------------------------------------------------------------------

void CViewerDoc::updateAnimationView (const CView* newView, const bool force)
{
	const SkeletalAppearance2 *const sa = dynamic_cast<const SkeletalAppearance2*>(object->getAppearance());
	if (sa)
		GetMainFrame ()->updateAnimationView (newView, skeletalAnimationKeyMap, force);
	else
		GetMainFrame ()->updateAnimationView (newView, force);		
}

//-------------------------------------------------------------------

void CViewerDoc::updateAnimationTreeView (const CView* newView, const bool force)
{
	UNREF(force);
	UNREF(newView);
	const SkeletalAppearance2 *const sa = dynamic_cast<const SkeletalAppearance2*>(object->getAppearance());
	if (sa)
	{
		//DEBUG_REPORT_LOG(true,("**** CViewerDoc::updateAnimationTreeView and it's a sat\n"));
		AnimationTreeDialog* animationTreeDialog = GetMainFrame()->getAnimationTreeDialog();
		if(animationTreeDialog)
		{
			animationTreeDialog->initTreeCtrl(m_animationFileList[0],m_animationFileList[1]);
		}
	}
	else
	{
		AnimationTreeDialog* animationTreeDialog = GetMainFrame()->getAnimationTreeDialog();
		if(animationTreeDialog)
		{
			animationTreeDialog->clear();
		}
		//DEBUG_REPORT_LOG(true,("*** CViewerDoc::updateAnimationTreeView and NOT a sat\n"));
	}	
}

//-------------------------------------------------------------------

void CViewerDoc::OnAnimationSkeletalanimationAttachtoselectedbone() 
{
	SkeletalAppearance2* sa = dynamic_cast<SkeletalAppearance2*> (object->getAppearance ());
	if (!sa)
		return;

	//-- get selected transform name
	const CrcLowerString crcAttachmentPointName (GetMainFrame ()->getAnimationDialog ()->getSelectedTransformName ());
	if (!crcAttachmentPointName.getString() || !*crcAttachmentPointName.getString())
		return;

	//-- select item to attach
	CFileDialog dlg (true, "*.*", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
	dlg.m_ofn.lpstrInitialDir = "";
	dlg.m_ofn.lpstrTitle      = "Select Appearance Template File To Attach";

	if (dlg.DoModal () != IDOK)
		return;

	//-- get appearance name
	const CString appearanceTemplateName = dlg.GetPathName ();

	//-- load the appearance
	Appearance *appearance = AppearanceTemplateList::createAppearance (appearanceTemplateName);
	if (appearance)
	{
		Object *attachedObject = new MemoryBlockManagedObject;
		attachedObject->setAppearance (appearance);

		sa->attach (attachedObject, crcAttachmentPointName);
		attachedObject->attachToObject_p(object, true);
	}
}

//-------------------------------------------------------------------

void CViewerDoc::OnUpdateAnimationSkeletalanimationAttachtoselectedbone(CCmdUI* pCmdUI) 
{
	const SkeletalAppearance2* sa = dynamic_cast<const SkeletalAppearance2*> (object->getAppearance());
	const BOOL enabled = (sa != 0);

	pCmdUI->Enable (enabled);
}

//-------------------------------------------------------------------

void CViewerDoc::OnAnimationSkeletalanimationDetach() 
{
	SkeletalAppearance2 *sa = dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
	if (!sa)
		return;

	std::vector<Appearance*>  appearancesToDelete;

	//-- collect all appearances attached to the skeletal appearance.
	const int appearanceCount = sa->getAttachedAppearanceCount();
	appearancesToDelete.reserve(static_cast<size_t>(appearanceCount));

	{
		for (int i = 0; i < appearanceCount; ++i)
		{
			// we take away constness because we know we created these appearances for attachment
			appearancesToDelete.push_back(const_cast<Appearance*>(sa->getAttachedAppearance(i)));
		}
	}

	//-- detach and delete the appearances
	{
		for (int i = 0; i < appearanceCount; ++i)
		{
			Object *const deadObject = appearancesToDelete[static_cast<size_t>(i)]->getOwner();
			sa->detach(deadObject);
			delete deadObject;
		}
	}
}

//-------------------------------------------------------------------

void CViewerDoc::OnUpdateAnimationSkeletalanimationDetach(CCmdUI* pCmdUI) 
{
	const SkeletalAppearance2* sa = dynamic_cast<const SkeletalAppearance2*>(object->getAppearance());
	const BOOL enabled = (sa != 0);

	pCmdUI->Enable (enabled);
}

//-------------------------------------------------------------------

void CViewerDoc::OnAnimationSkeletalanimationSelectshadowvolume() 
{
#if 0
	SkeletalAppearance2 *sa = dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
	if (sa)
	{
		//-- select item to wear
		CFileDialog dlg (true, "*.grn", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
		dlg.m_ofn.lpstrInitialDir = "";
		dlg.m_ofn.lpstrTitle      = "Select Granny File";

		if (dlg.DoModal () == IDOK)
		{
			//-- get new name
			const CString newName = dlg.GetPathName ();

			SkeletalModel    model    = SkeletalSystem::openModel (newName);
			SkeletalSequence sequence = SkeletalSystem::openSequence (model);

			sa->setRenderShadow (true, sequence, 1000.f);
		}
	}
#endif
}

//-------------------------------------------------------------------

void CViewerDoc::OnUpdateAnimationSkeletalanimationSelectshadowvolume(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
#if 1
	pCmdUI->Enable (FALSE);
#else
	const SkeletalAppearance2* sa = dynamic_cast<const SkeletalAppearance2*>(object->getAppearance());
	pCmdUI->Enable (sa != 0);
	pCmdUI->SetCheck (sa && sa->getRenderShadow ());
#endif
}

//-------------------------------------------------------------------

void CViewerDoc::OnUpdateViewRenderSkeleton(CCmdUI* pCmdUI) 
{
	BOOL checkIt = FALSE;
	BOOL enableIt = FALSE;

	if (object)
	{
		SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
		if (appearance)
		{
			checkIt = (appearance->getShowSkeleton() ? TRUE : FALSE);
			enableIt = TRUE;
		}
	}
	pCmdUI->SetCheck (checkIt);
	pCmdUI->Enable (enableIt);
}

// ----------------------------------------------------------------------

void CViewerDoc::OnViewRenderSkeleton() 
{
	// TODO: Add your command handler code here
	if (object)
	{
		SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
		if (appearance)
		{
			appearance->setShowSkeleton(!appearance->getShowSkeleton());
		}
	}
}

// ----------------------------------------------------------------------

void CViewerDoc::OnUpdateViewRenderMesh(CCmdUI* pCmdUI) 
{
	BOOL checkIt = FALSE;
	BOOL enableIt = FALSE;

	if (object)
	{
		SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
		if (appearance)
		{
			checkIt = (appearance->getShowMesh() ? TRUE : FALSE);
			enableIt = TRUE;
		}
	}
	pCmdUI->SetCheck (checkIt);
	pCmdUI->Enable (enableIt);
}

// ----------------------------------------------------------------------

void CViewerDoc::OnViewRenderMesh() 
{
	if (object)
	{
		SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
		if (appearance)
		{
			appearance->setShowMesh(!appearance->getShowMesh());
		}
	}
}

// ----------------------------------------------------------------------

void CViewerDoc::OnUpdateViewVariableSetView(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (TRUE);
	pCmdUI->SetCheck (hasVariableSetView());
}

// ----------------------------------------------------------------------

void CViewerDoc::OnViewVariableSetView() 
{
	if (hasVariableSetView())
		closeVariableSetView();
	else
	{
		CMultiDocTemplate *const variableSetViewDocTemplate = static_cast<CViewerApp*>(AfxGetApp())->getVariableSetViewDocTemplate();
		if (variableSetViewDocTemplate == 0)
			return;

		// create the viewer view
		CChildFrame *const variableSetViewFrame = static_cast<CChildFrame*>(variableSetViewDocTemplate->CreateNewFrame(this, NULL));
		if (variableSetViewFrame)
		{
			variableSetViewDocTemplate->InitialUpdateFrame(variableSetViewFrame, this);

			// set the title for the window
			char windowTitle[MAX_PATH];

			strcpy(windowTitle, "Variable Set View: ");
			if (filename.IsEmpty())
				strcat(windowTitle, "<no filename yet>");
			else
			{
				// strip off the path, leave just base filename
				char fileWithPath[MAX_PATH];

				strcpy(fileWithPath, filename);
				const char *const endOfDir = strrchr(fileWithPath, '\\');
				if (endOfDir)
					strcat(windowTitle, endOfDir+1);
				else
					strcat(windowTitle, fileWithPath);
			}

			variableSetViewFrame->SetWindowText(windowTitle);
		}
	}
}

// ----------------------------------------------------------------------

void CViewerDoc::OnCloseDocument() 
{
	GetMainFrame()->updateHardpointTree(0);
	CDocument::OnCloseDocument();
}

// ----------------------------------------------------------------------

void CViewerDoc::closeSkeletalAppearanceTemplateView()
{
	POSITION  viewPosition = GetFirstViewPosition();
	while (viewPosition != NULL)
	{
		SkeletalAppearanceTemplateView *const view = dynamic_cast<SkeletalAppearanceTemplateView*>(GetNextView(viewPosition));
		if (view)
		{
			CFrameWnd *frameWindow = view->GetParentFrame();
			frameWindow->DestroyWindow();
			break;
		}
	}
}

// ----------------------------------------------------------------------

void CViewerDoc::closeVariableSetView()
{
	POSITION  viewPosition = GetFirstViewPosition();
	while (viewPosition != NULL)
	{
		VariableSetView *const view = dynamic_cast<VariableSetView*>(GetNextView(viewPosition));
		if (view)
		{
			CFrameWnd *frameWindow = view->GetParentFrame();
			frameWindow->DestroyWindow();
			break;
		}
	}
}

// ----------------------------------------------------------------------

bool CViewerDoc::hasVariableSetView() const
{
	// search all attached views for the variable set view
	POSITION  viewPosition = GetFirstViewPosition();
	while (viewPosition != NULL)
	{
		VariableSetView *const view = dynamic_cast<VariableSetView*>(GetNextView(viewPosition));
		if (view)
			return true;
	}

	// didn't find it
	return false;
}

// ----------------------------------------------------------------------

SkeletalAppearanceTemplateView *CViewerDoc::getSkeletalAppearanceTemplateView() const
{
	// search all attached views for the skeletal appearance template view
	POSITION  viewPosition = GetFirstViewPosition();
	while (viewPosition != NULL)
	{
		SkeletalAppearanceTemplateView *const view = dynamic_cast<SkeletalAppearanceTemplateView*>(GetNextView(viewPosition));
		if (view)
			return view;
	}

	return 0;
}

// ----------------------------------------------------------------------

bool CViewerDoc::hasSkeletalAppearanceTemplateView() const
{
	SkeletalAppearanceTemplateView *const view = getSkeletalAppearanceTemplateView();
	if (view)
		return true;
	else
		return false;
}

// ----------------------------------------------------------------------

void CViewerDoc::OnUpdateViewSkeletalAppearanceTemplate(CCmdUI* pCmdUI) 
{
	if (!m_skeletalAppearanceTemplate)
	{
		pCmdUI->Enable(FALSE);
		pCmdUI->SetCheck(FALSE);
	}
	else
	{
		pCmdUI->Enable(TRUE);
		pCmdUI->SetCheck(hasSkeletalAppearanceTemplateView());
	}
	
}

// ----------------------------------------------------------------------

void CViewerDoc::OnViewSkeletalAppearanceTemplate() 
{
	if (hasSkeletalAppearanceTemplateView())
		closeSkeletalAppearanceTemplateView();
	else
	{
		CMultiDocTemplate *const docTemplate = static_cast<CViewerApp*>(AfxGetApp())->getSkeletalAppearanceTemplateViewDocTemplate();
		if (docTemplate == 0)
			return;

		// create the viewer view
		CChildFrame *const viewFrame = static_cast<CChildFrame*>(docTemplate->CreateNewFrame(this, NULL));
		if (viewFrame)
		{
			// perform initial update activities
			docTemplate->InitialUpdateFrame(viewFrame, this);
		}
	}
}

// ----------------------------------------------------------------------

void CViewerDoc::OnMeshGeneratorAdd() 
{
	if (!m_skeletalAppearanceTemplate)
		return;

	// get the app
	CViewerApp *const app = static_cast<CViewerApp*>(AfxGetApp());
	if (!app)
		return;

	// get the last mesh generator file dealt with
	const char *const lastFilename = app->getLastMeshGeneratorFilename();
	CFileDialog dlg(TRUE, ".lmg", lastFilename, 0, "Mesh Generators (*.mgn;*.lmg)|*.mgn;*.lmg||");

	// allow multi-select.
	char multiSelectBuffer[50 * MAX_PATH];

	dlg.m_ofn.Flags     |= OFN_ALLOWMULTISELECT;
	dlg.m_ofn.lpstrFile  = multiSelectBuffer;
	dlg.m_ofn.nMaxFile   = sizeof(multiSelectBuffer) - 1;

	multiSelectBuffer[0] = 0;

	// open the dialog
	if (dlg.DoModal() == IDOK)
	{
		//-- Handle multiple file selection.
		POSITION pos = dlg.GetStartPosition();
		while (pos != NULL)
		{
			// Get the path name for this selection.  Convert to more useful form.
			CString const pathname = dlg.GetNextPathName(pos);
			
			char cPathname[MAX_PATH];
			strcpy(cPathname, pathname);

			// Find start of short name.
			char const *shortName = strrchr(cPathname, '\\');
			if (shortName)
				++shortName;
			else
				shortName = cPathname;

			// build the reference filename
			char referencePath[MAX_PATH];

			strcpy(referencePath, app->getMeshGeneratorReferenceDirectory());
			strcat(referencePath, "/");
			strcat(referencePath, shortName);

			// add skeleton template to the skeletal appearance template
			const int newIndex = m_skeletalAppearanceTemplate->addMeshGenerator(referencePath);
			DEBUG_REPORT_LOG(true, ("SkeletalAppearanceTemplate: adding mesh generator [%s]\n", referencePath));

			// save the full pathname
			app->setLastMeshGeneratorFilename(cPathname);

			// update the skeleton template view
			SkeletalAppearanceTemplateView *view = getSkeletalAppearanceTemplateView();
			if (view)
				view->addAvailableMeshGenerator(referencePath, newIndex);
		}
	}
}

// ----------------------------------------------------------------------

void CViewerDoc::OnSkeletonTemplateAdd() 
{
	if (!m_skeletalAppearanceTemplate)
		return;

	//-- get the app
	CViewerApp *const app = static_cast<CViewerApp*>(AfxGetApp());
	if (!app)
		return;

	//-- get all the skeleton templates available to the skeletal appearance template
	std::vector<const SkeletonTemplate*> skeletonTemplates;

	const int skeletonTemplateCount = m_skeletalAppearanceTemplate->getSkeletonTemplateCount();
	skeletonTemplates.reserve(skeletonTemplateCount);
	for (int i = 0; i < skeletonTemplateCount; ++i)
	{
		const SkeletalAppearanceTemplate::SkeletonTemplateInfo &sti = m_skeletalAppearanceTemplate->getSkeletonTemplateInfo(i);
		const CrcLowerString &crcSkeletonTemplateName = m_skeletalAppearanceTemplate->getSkeletonTemplateName(sti);

		skeletonTemplates.push_back(SkeletonTemplateList::fetch(crcSkeletonTemplateName));
	}

	//-- get the last skeleton template file dealt with
	DialogAddSkeleton dlg(skeletonTemplates);

	dlg.m_filePath = app->getLastSkeletonTemplateFilename();

	//-- open the dialog
	if (dlg.DoModal() == IDOK)
	{
		// build the reference filename
		std::string  referencePath(app->getSkeletonTemplateReferenceDirectory());
		referencePath += '\\';

		const std::string fullPath(dlg.m_filePath);

		const size_t lastDirIndex = fullPath.rfind('\\');
		if (static_cast<int>(lastDirIndex) == fullPath.npos)
			referencePath += fullPath;
		else
			referencePath.append(fullPath, lastDirIndex + 1, fullPath.length());

		// add skeleton template to the skeletal appearance template
		const int newIndex = m_skeletalAppearanceTemplate->addSkeletonTemplate(referencePath.c_str(), dlg.getAttachmentName().getString());
		DEBUG_REPORT_LOG(true, ("SkeletalAppearanceTemplate: adding skeleton template [%s], attachment transform [%s]\n", referencePath.c_str(), dlg.getAttachmentName().getString()));

		// save the full pathname
		app->setLastSkeletonTemplateFilename(dlg.m_filePath);

		// update the skeleton template view
		SkeletalAppearanceTemplateView *view = getSkeletalAppearanceTemplateView();
		if (view)
			view->addAvailableSkeletonTemplate(referencePath.c_str(), dlg.getAttachmentName().getString(), newIndex);
	}

	//-- release the skeleton templates
	std::for_each(skeletonTemplates.begin(), skeletonTemplates.end(), VoidMemberFunction(&SkeletonTemplate::release));
}

// ----------------------------------------------------------------------

void CViewerDoc::saveSkeletalAppearanceTemplate(const char *filename)
{
	if (!filename || !*filename || !m_skeletalAppearanceTemplate)
		return;

	//-- construct the iff
	Iff iff(ms_skeletalAppearanceTemplateIffSize);
	m_skeletalAppearanceTemplate->write(iff);
	DEBUG_REPORT_LOG(true, ("SkeletalAppearanceTemplate: writing [%d] bytes...", iff.getRawDataSize()));
	
	//-- write the iff
	const bool writeSuccess = iff.write(filename, true);
	if (writeSuccess)
	{
		DEBUG_REPORT_LOG(true, ("success.\n"));
		m_skeletalAppearanceTemplateFilename = filename;
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("failure.\n"));

		char buffer[MAX_PATH];
		sprintf(buffer, "Failed to save skeletal appearance template [%s]", filename);
		MessageBox(NULL, buffer, "Write Failure", MB_OK | MB_ICONSTOP);
	}
}

// ----------------------------------------------------------------------

CViewerApp *CViewerDoc::getOurApp() const
{
	CViewerApp *const app = NON_NULL(dynamic_cast<CViewerApp*>(AfxGetApp()));
	return app;
}

// ----------------------------------------------------------------------

bool CViewerDoc::saveSkeletalAppearanceWorkspace(const CString &filename)
{
	if (!filename.GetLength())
	{
		MessageBox(NULL, "Bad workspace name", "Save Failure", MB_OK | MB_ICONSTOP);
		return false;
	}

	if (!m_skeletalAppearanceTemplate)
	{
		// bad data
		MessageBox(NULL, "Failed to save skeletal appearance workspace", "Save Failure", MB_OK | MB_ICONSTOP);
		return false;
	}

	//-- construct the iff image
	Iff iff(ms_skeletalAppearanceWorkspaceIffSize);

	// skeletal appearance workspace
	iff.insertForm(TAG(S,A,W,S));
	{
		// version form
		iff.insertForm(TAG_0002);
		{
			// general info
			iff.insertChunk(TAG_INFO);
			{
				// skeletal appearance template filename
				iff.insertChunkString(static_cast<const char*>(m_skeletalAppearanceTemplateFilename));
			}
			iff.exitChunk(TAG_INFO);
		}
		iff.exitForm(TAG_0002);
	}
	iff.exitForm(TAG(S,A,W,S));

	DEBUG_REPORT_LOG(true, ("skeletal appearance workspace iff size: %d bytes\n", iff.getRawDataSize()));

	// write the iff image
	const bool writeSuccess = iff.write(filename, true);
	if (!writeSuccess)
	{
		char buffer[2 * MAX_PATH];

		sprintf(buffer, "Skeletal apearance workspace [%s] failed to write file", static_cast<const char*>(filename));
		MessageBox(NULL, buffer, "Save Failure", MB_OK | MB_ICONSTOP);
		return false;
	}

	m_skeletalAppearanceWorkspaceFilename = filename;

	// no errors
	return true;
}

// ----------------------------------------------------------------------

void CViewerDoc::resetSkeletalAppearanceWorkspace()
{
	if (object)
	{
		object->setAppearance(0);
	}

	delete m_skeletalAppearanceTemplate;
	m_skeletalAppearanceTemplate = 0;
}

// ----------------------------------------------------------------------

bool CViewerDoc::loadSkeletalAppearanceWorkspace_0001(Iff *iff)
{
	NOT_NULL(iff);

	// version form
	iff->enterForm(TAG_0001);
	{
		int instanceCount;

		// general info
		iff->enterChunk(TAG_INFO);
		{
			// # instances
			instanceCount = iff->read_int32();
			DEBUG_FATAL(instanceCount < 0, ("bad instance count %d", instanceCount));

			// skeletal appearance template filename
			char appearanceTemplateFilename[MAX_PATH];
			iff->read_string(appearanceTemplateFilename, sizeof(appearanceTemplateFilename)-1);

			// load the appearance template
			const bool loadSuccess = loadSkeletalAppearanceTemplate(appearanceTemplateFilename);
			if (!loadSuccess)
				return false;
		}
		iff->exitChunk(TAG_INFO);

		// instance info
		iff->enterChunk(TAG(S,A,I,N));
		{
			for (int i = 0; i < instanceCount; ++i)
			{
				// get instance filename and ignore
				char instanceFilename[MAX_PATH];
				iff->read_string(instanceFilename, sizeof(instanceFilename)-1);
			}
		}
		iff->exitChunk(TAG(S,A,I,N));
	}
	iff->exitForm(TAG_0001);

	//-- no errors
	return true;
}

// ----------------------------------------------------------------------

bool CViewerDoc::loadSkeletalAppearanceWorkspace_0002(Iff *iff)
{
	NOT_NULL(iff);

	// version form
	iff->enterForm(TAG_0002);
	{
		// general info
		iff->enterChunk(TAG_INFO);
		{
			// skeletal appearance template filename
			char appearanceTemplateFilename[MAX_PATH];
			iff->read_string(appearanceTemplateFilename, sizeof(appearanceTemplateFilename)-1);

			// load the appearance template
			const bool loadSuccess = loadSkeletalAppearanceTemplate(appearanceTemplateFilename);
			if (!loadSuccess)
				return false;
		}
		iff->exitChunk(TAG_INFO);
	}
	iff->exitForm(TAG_0002);

	//-- no errors
	return true;
}

// ----------------------------------------------------------------------

bool CViewerDoc::loadSkeletalAppearanceWorkspace(const char *filename)
{
	//-- clear out existing skeletal appearance template and appearance
	resetSkeletalAppearanceWorkspace();

	//-- keep track of the name of the workspace
	m_skeletalAppearanceWorkspaceFilename = filename;

	//-- load the iff image
	Iff iff(filename);

	// skeletal appearance workspace
	bool loadSuccess = false;

	iff.enterForm(TAG(S,A,W,S));
	{
		switch (iff.getCurrentName())
		{
			case TAG_0001:
				loadSuccess = loadSkeletalAppearanceWorkspace_0001(&iff);
				break;

			case TAG_0002:
				loadSuccess = loadSkeletalAppearanceWorkspace_0002(&iff);
				break;

			default:
				{
					char tagName[5];
					ConvertTagToString(iff.getCurrentName(), tagName);

					char buffer[2*MAX_PATH];
					sprintf(buffer, "Unrecognized skeletal appearance workspace version [%s]", tagName);

					MessageBox(NULL, buffer, "Load Failure", MB_OK | MB_ICONSTOP);
				}
		}
	}
	if (loadSuccess)
		iff.exitForm(TAG(S,A,W,S));

	if (!loadSuccess)
	{
		char buffer[2*MAX_PATH];
		sprintf(buffer, "Failed to load skeletal appearance workspace [%s]", filename);
		MessageBox(NULL, buffer, "Load Failure", MB_OK | MB_ICONSTOP);
		return false;
	}

	//-- keep track of the app
	CViewerApp *const app = dynamic_cast<CViewerApp*>(AfxGetApp());
	if (!app)
		return false;
		
	app->setLastSkeletalAppearanceWorkspaceFilename(filename);

	//-- no errors
	return true;
}

// ----------------------------------------------------------------------

void CViewerDoc::renderTextures()
{
	if (!m_textureRenderer)
		return;

	//-- handle rendering our texture through the texture renderer
	m_textureRenderer->render();

	UpdateAllViews(0);
}

// ----------------------------------------------------------------------

void CViewerDoc::rebuildAppearance()
{
	//-- Ensure we've got an object.
	if (!object)
		return;

	//-- Create the skeletal appearance if we have a skeletal appearance template that has at least one skeleton and one mesh generator.
	if (m_skeletalAppearanceTemplate && (m_skeletalAppearanceTemplate->getSkeletonTemplateCount() > 0) && (m_skeletalAppearanceTemplate->getMeshGeneratorCount() > 0))
	{
		//-- Delete the existing appearance.
		Appearance *const baseAppearance = object->getAppearance();
		if (baseAppearance)
		{
			// This will delete the appearance.
			object->setAppearance(0);
		}

		//-- Create the new appearance.
		Appearance *const newAppearance = m_skeletalAppearanceTemplate->createAppearance();
		DEBUG_WARNING(!newAppearance, ("Failed to create new skeletal appearance."));
		object->setAppearance(newAppearance);

		//-- Tell new appearance to rebuild itself.
		SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(newAppearance);
		if (!appearance)
		{
			DEBUG_WARNING(true, ("Skeletal appearance template created non-SkeletalAppearance2 appearance!"));
			return;
		}

		//-- instruct the appearance to rebuild itself
		IGNORE_RETURN(appearance->rebuildIfDirtyAndAvailable());

		//-- Hook up customization data.
		setupObjectCustomizationData(object, newAppearance);
	}
}

// ----------------------------------------------------------------------

void CViewerDoc::OnFileWriteObjTemplateCustomizationData() 
{
	writeObjectTemplateCustomizationData (true);
}

// ----------------------------------------------------------------------

void CViewerDoc::OnUpdateFileWriteCustomizationData(CCmdUI* pCmdUI) 
{
#if USE_OBJ_TEMPLATE_CUSTOM_VAR_WRITER
	pCmdUI->Enable(TRUE);
#else
	pCmdUI->Enable(FALSE);
#endif
}

// ----------------------------------------------------------------------

static void ObjectTemplateCustomVarCollector(const std::string &fullVariablePathName, const CustomizationVariable *customizationVariable, void *context)
{
	NOT_NULL(context);

	ObjectTemplateCustomizationDataWriter *const writer = reinterpret_cast<ObjectTemplateCustomizationDataWriter*>(context);
	customizationVariable->writeObjectTemplateExportString(fullVariablePathName, *writer);
}

// ----------------------------------------------------------------------
/**
 * Write the object template (.tpf) customization data definitions for
 * the CustomizationVariable attributes supported by this appearance.
 *
 * The data will be written to a file named "objtemplate_customvar_decls.txt".
 *
 * Note non-player assets that declare "/shared_owner/*" variable names
 * should not be declared by the non-player asset.  They are references
 * that must come from the player.
 */

void CViewerDoc::writeObjectTemplateCustomizationData (bool allowOverwrite)
{
	//-- retrieve the CustomizationData
	if (!object)
		return;

	CustomizationDataProperty *const property = safe_cast<CustomizationDataProperty*>(object->getProperty( CustomizationDataProperty::getClassPropertyId() ));
	if (!property)
		return;

	CustomizationData *const customizationData = property->fetchCustomizationData();
	if (!customizationData)
		return;

	//-- Figure out if we have a skeletal appearance template with animation controller data.
	//   If so, we have something that is not a wearable and should keep /shared_owner/... variables;
	//   otherwise, we have a wearable that should not declare /shared_owner/... variables because
	//   it inherits them when it is placed on an owner.
	SkeletalAppearanceTemplate const *const sat = dynamic_cast<SkeletalAppearanceTemplate const*>(object->getAppearance() ? object->getAppearance()->getAppearanceTemplate() : 0);
	bool writeSharedOwnerVariables;
	
	if (sat)
	{
		// Write shared owner variables if this SAT has an animation controller.  SATs only define animation controllers when they represent creatures/player species.
		writeSharedOwnerVariables = sat->shouldCreateAnimationController();
	}
	else
		writeSharedOwnerVariables = true;

	//-- create the object template customization data writer
	ObjectTemplateCustomizationDataWriter  writer(writeSharedOwnerVariables);

	//-- collect customization variable data
	customizationData->iterateOverConstVariables(ObjectTemplateCustomVarCollector, &writer, false);

	//-- write ObjectTemplate custom var declarations
	FileName sourceFileName (filename);
	sourceFileName.stripPathAndExt ();
	
	const FileName destinationFileName (FileName::P_none, sourceFileName, "cst");

	if (!writer.writeToFile(std::string (destinationFileName), allowOverwrite))
		WARNING(true, ("failed to write object template custom variable declarations to file [%s].\n", static_cast<char const*>(destinationFileName)));

	//-- cleanup
	customizationData->release();
}

// ----------------------------------------------------------------------
/**
 * Reset the Object's transform so it is at (0,0,0) facing down +z axis.
 */

void CViewerDoc::OnButtonCenterObject() 
{
	if (!object)
		return;

	object->setTransform_o2p(Transform::identity);
}

// ----------------------------------------------------------------------

void CViewerDoc::OnButtonToggleLocomotion() 
{
	m_locomotionEnabled = !m_locomotionEnabled;
}

// ----------------------------------------------------------------------

void CViewerDoc::OnUpdateButtonToggleLocomotion(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_locomotionEnabled ? 1 : 0);
}

// ----------------------------------------------------------------------

void CViewerDoc::OnButtonDebugdump() 
{
	if (!object || !object->getAppearance ())
		return;

#ifdef _DEBUG
	std::string result;
	object->getAppearance ()->debugDump (result, 0);

	CONSOLE_PRINT (result.c_str ());
#endif
}

// ----------------------------------------------------------------------

void CViewerDoc::debugDump ()
{
	OnButtonDebugdump ();
}

// ----------------------------------------------------------------------
/**
 * Write out the macros to be used by the client data file to create a
 * wearable containing existing .mgn files and customizations.
 *
 * The file is written out to c:\clientBakedWearable.txt.
 */

void CViewerDoc::OnFileWriteCdfBakedWearableData() 
{
	if (!object)
	{
		DEBUG_WARNING(true, ("Nothing to write --- make sure your object is visible and showing up the way you want before executing this command."));
		return;
	}

	char const *const filename = "c:\\clientBakedWearable.txt";

	bool const success = ClientBakedWearableWriter::write(*object, filename);
	WARNING(!success, ("Failed to write client-baked wearables to [%s].\n", filename));
}

// ----------------------------------------------------------------------

bool CViewerDoc::loadHardpointHierarchy(const char *filename)
{
	UNREF(filename);
	return true;
}

// ----------------------------------------------------------------------

void CViewerDoc::OnSaveHardpointHierarchy() 
{
}

// ----------------------------------------------------------------------

void CViewerDoc::OnLoadAllHardpoints() 
{
	// determine the root directory and load all apts that match hardpoint names
	DEBUG_REPORT_LOG(true, ("filename = [%s]\n", filename));

	ms_workingDirectory = FileNameUtils::getDirectory((char const *)filename);

#if 0
	std::string searchPattern =  ms_workingDirectory + "*.apt";
	DEBUG_REPORT_LOG(true, ("[%s]\n", searchPattern.c_str()));

	CFileFind finder;
	BOOL working = finder.FindFile(searchPattern.c_str());

	ms_appearanceFiles.clear();

	while (working)
	{
		working = finder.FindNextFile();

		if (!finder.IsDots() && !finder.IsDirectory())
		{
			DEBUG_REPORT_LOG(true, ("[%s]\n", finder.GetFileTitle()));

			ms_appearanceFiles.push_back((char const *)finder.GetFileTitle());
		}
	}
#endif

	// TODO: remove all child objects

	loadAllHardpoints(object, 0);
}

// ----------------------------------------------------------------------

void CViewerDoc::loadAllHardpoints(Object * parent, int depth)
{
	UNREF(object);
	UNREF(depth);

	if (!parent || depth >= ms_maxHardpointAttachDepth)
		return;

	AppearanceTemplate const * const appearanceTemplate = parent->getAppearance() ? parent->getAppearance()->getAppearanceTemplate () : 0;

	if (appearanceTemplate)
	{
		int const numberOfHardpoints = appearanceTemplate->getHardpointCount();

		for (int i = 0; i < numberOfHardpoints; ++i)
		{
			Hardpoint const & hardpoint = appearanceTemplate->getHardpoint(i);

			std::string appearanceFile = ms_workingDirectory + hardpoint.getName().getString() + ".apt";
			DEBUG_REPORT_LOG(true, ("[%s]\n", appearanceFile.c_str()));

			if (TreeFile::exists(appearanceFile.c_str()))
			{
				Object * hardpointObject = attachStandardAppearanceToHardpoint(parent, appearanceFile.c_str(), hardpoint.getName().getString());

				loadAllHardpoints(hardpointObject, depth + 1);
			}
		}
	}
}

// ----------------------------------------------------------------------

void CViewerDoc::removeAttachedObject(Object * child)
{
	if (object && child && child != object)
	{
		object->removeChildObject(child, Object::DF_none);

		delete child;
	}
}

// ----------------------------------------------------------------------

void CViewerDoc::OnUpdateLoadAllHardpoints(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(FALSE);
}

// ======================================================================
