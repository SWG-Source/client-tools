//===================================================================
//
// PreloadedAssetManager.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PreloadedAssetManager.h"

#include "clientAudio/SoundTemplate.h"
#include "clientAudio/SoundTemplateList.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientSkeletalAnimation/AnimationStateHierarchyTemplate.h"
#include "clientSkeletalAnimation/AnimationStateHierarchyTemplateList.h"
#include "clientSkeletalAnimation/LogicalAnimationTableTemplate.h"
#include "clientSkeletalAnimation/LogicalAnimationTableTemplateList.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplate.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedMath/PaletteArgb.h"
#include "sharedMath/PaletteArgbList.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/ArrangementDescriptor.h"
#include "sharedObject/ArrangementDescriptorList.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/SlotDescriptor.h"
#include "sharedObject/SlotDescriptorList.h"

#include <vector>

//===================================================================

namespace TextureListNamespace
{
	typedef std::vector<const AppearanceTemplate*>         InternalAppearanceTemplateList;
	typedef std::vector<const ObjectTemplate*>             InternalObjectTemplateList;
	typedef std::vector<const ShaderTemplate*>             InternalShaderTemplateList;
	typedef std::vector<const SkeletalAnimationTemplate*>  InternalSkeletalAnimationTemplateList;
	typedef std::vector<const ClientEffectTemplate*>       InternalClientEffectTemplateList;
	typedef std::vector<Object*>                           InternalObjectList;
	typedef std::vector<const AnimationStateHierarchyTemplate*>  InternalAnimationStateHierarchyTemplateList;
	typedef std::vector<const LogicalAnimationTableTemplate*>    InternalLogicalAnimationTableTemplateList;
	typedef std::vector<const MeshGeneratorTemplate*>            InternalMeshGeneratorTemplateList;
	typedef std::vector<const Texture*>                          InternalTextureList;
	typedef std::vector<const PaletteArgb*>                      InternalPaletteList;
	typedef std::vector<const ArrangementDescriptor*>            InternalArrangementDescriptorList;
	typedef std::vector<const SlotDescriptor*>                   InternalSlotDescriptorList;
	typedef std::vector<SoundTemplate const *> InternalSoundTemplateList;

	InternalAppearanceTemplateList*                        s_appearanceTemplateList;
	InternalObjectTemplateList*                            s_objectTemplateList;
	InternalShaderTemplateList*                            s_shaderTemplateList;
	InternalSkeletalAnimationTemplateList*                 s_skeletalAnimationTemplateList;
	InternalClientEffectTemplateList*                      s_clientEffectTemplateList;
	InternalObjectList*                                    s_objectList;
	InternalAnimationStateHierarchyTemplateList*           s_animationStateHierarchyTemplateList;
	InternalLogicalAnimationTableTemplateList*             s_logicalAnimationTableTemplateList;
	InternalMeshGeneratorTemplateList*                     s_meshGeneratorTemplateList;
	InternalTextureList*                                   s_textureList;
	InternalPaletteList*                                   s_paletteList;
	InternalArrangementDescriptorList*                     s_arrangementDescriptorList;
	InternalSlotDescriptorList*                            s_slotDescriptorList;
	InternalSoundTemplateList * s_soundTemplateList;

	bool                                                   s_installed;
}

using namespace TextureListNamespace;

//===================================================================

void PreloadedAssetManager::install ()
{
	InstallTimer const installTimer("PreloadedAssetManager::install");

	DEBUG_FATAL (s_installed, ("PreloadedAssetManager::install - already installed"));
	s_installed = true;

	const char* result = 0;

	s_appearanceTemplateList        = new InternalAppearanceTemplateList;
	s_objectTemplateList            = new InternalObjectTemplateList;
	s_shaderTemplateList            = new InternalShaderTemplateList;
	s_skeletalAnimationTemplateList = new InternalSkeletalAnimationTemplateList;
	s_clientEffectTemplateList      = new InternalClientEffectTemplateList;
	s_objectList                    = new InternalObjectList;
	s_animationStateHierarchyTemplateList = new InternalAnimationStateHierarchyTemplateList;
	s_logicalAnimationTableTemplateList   = new InternalLogicalAnimationTableTemplateList;
	s_meshGeneratorTemplateList           = new InternalMeshGeneratorTemplateList;
	s_textureList                         = new InternalTextureList;
	s_paletteList                         = new InternalPaletteList;
	s_arrangementDescriptorList           = new InternalArrangementDescriptorList;
	s_slotDescriptorList                  = new InternalSlotDescriptorList;
	s_soundTemplateList = new InternalSoundTemplateList;

	const bool asynchronousLoaderEnabled = AsynchronousLoader::isEnabled ();
	AsynchronousLoader::disable ();

#ifdef _DEBUG
	const bool isLoggingFiles = TreeFile::isLoggingFiles ();
	TreeFile::setLogTreeFileOpens (isLoggingFiles || ConfigFile::getKeyBool ("ClientGame", "timePreloading", false));
#endif

	//-- preload shader templates
	int i = 0;
	do
	{
		result = ConfigFile::getKeyString ("PreloadedAssets", "shaderTemplate", i, 0);
		if (result)
		{
			if (TreeFile::exists (result))
			{
				//DEBUG_REPORT_LOG (true, ("Preloading asset %s\n", result));

				ShaderTemplate const * const shaderTemplate = ShaderTemplateList::fetch(result);
				if (shaderTemplate)
					s_shaderTemplateList->push_back(shaderTemplate);
				else
					DEBUG_WARNING(true, ("PreloadedAssetManager::install: file %s not found", result));
			}

			++i;
		}
	}
	while (result);

	//-- preload appearance templates (heavy preloading via AppearanceTemplate::preloadAssets ())
	i = 0;
	do
	{
		result = ConfigFile::getKeyString ("PreloadedAssets", "appearanceTemplate", i, 0);
		if (result)
		{
			if (TreeFile::exists (result))
			{
				//DEBUG_REPORT_LOG (true, ("Preloading asset %s (heavy)\n", result));

				AppearanceTemplate const *const appearanceTemplate = AppearanceTemplateList::fetch (result);
				if (appearanceTemplate)
				{
					appearanceTemplate->preloadAssets ();
					s_appearanceTemplateList->push_back (appearanceTemplate);
				}
				else
					DEBUG_WARNING(true, ("PreloadedAssetManager::install: file %s not found", result));
			}

			++i;
		}
	}
	while (result);

	//-- preload appearance templates (light preloading via AppearanceTemplate::preloadAssetsLight ())
	i = 0;
	do
	{
		result = ConfigFile::getKeyString ("PreloadedAssets", "appearanceTemplateLight", i, 0);
		if (result)
		{
			if (TreeFile::exists (result))
			{
				//DEBUG_REPORT_LOG (true, ("Preloading asset %s (light)\n", result));

				AppearanceTemplate const *const appearanceTemplate = AppearanceTemplateList::fetch (result);
				if (appearanceTemplate)
				{
					appearanceTemplate->preloadAssetsLight ();
					s_appearanceTemplateList->push_back (appearanceTemplate);
				}
				else
					DEBUG_WARNING(true, ("PreloadedAssetManager::install: file %s not found", result));
			}

			++i;
		}
	}
	while (result);

	//-- preload object templates
	i = 0;
	do
	{
		result = ConfigFile::getKeyString ("PreloadedAssets", "objectTemplate", i, 0);
		if (result)
		{
			if (TreeFile::exists (result))
			{
				//DEBUG_REPORT_LOG (true, ("Preloading asset %s\n", result));

				const ObjectTemplate* const objectTemplate = ObjectTemplateList::fetch (result);
				if (objectTemplate)
				{
					objectTemplate->preloadAssets ();
					s_objectTemplateList->push_back (objectTemplate);
				}
				else
					DEBUG_WARNING(true, ("PreloadedAssetManager::install: file %s not found", result));
			}

			++i;
		}
	}
	while (result);

	//-- preload object templates (light preloading by not calling preloadAssets)
	i = 0;
	do
	{
		result = ConfigFile::getKeyString ("PreloadedAssets", "objectTemplateLight", i, 0);
		if (result)
		{
			if (TreeFile::exists (result))
			{
				//DEBUG_REPORT_LOG (true, ("Preloading asset %s\n", result));

				const ObjectTemplate* const objectTemplate = ObjectTemplateList::fetch (result);
				if (objectTemplate)
					s_objectTemplateList->push_back (objectTemplate);
				else
					DEBUG_WARNING(true, ("PreloadedAssetManager::install: file %s not found", result));
			}

			++i;
		}
	}
	while (result);

	//-- preload skeletal animation templates
	i = 0;
	do
	{
		result = ConfigFile::getKeyString ("PreloadedAssets", "skeletalAnimationTemplate", i, 0);
		if (result)
		{
			if (TreeFile::exists (result))
			{
				//DEBUG_REPORT_LOG (true, ("Preloading asset %s\n", result));

				SkeletalAnimationTemplate const * const skeletalAnimationTemplate = SkeletalAnimationTemplateList::fetch(CrcLowerString(result));
				if (skeletalAnimationTemplate)
					s_skeletalAnimationTemplateList->push_back(skeletalAnimationTemplate);
				else
					DEBUG_WARNING(true, ("PreloadedAssetManager::install: file %s not found", result));
			}

			++i;
		}
	}
	while (result);

	//-- preload client effect templates
	i = 0;
	do
	{
		result = ConfigFile::getKeyString ("PreloadedAssets", "clientEffectTemplate", i, 0);
		if (result)
		{
			if (TreeFile::exists (result))
			{
				//DEBUG_REPORT_LOG (true, ("Preloading asset %s\n", result));

				ClientEffectTemplate const * const clientEffectTemplate = ClientEffectTemplateList::fetch(CrcLowerString(result));
				if (clientEffectTemplate)
					s_clientEffectTemplateList->push_back(clientEffectTemplate);
				else
					DEBUG_WARNING(true, ("PreloadedAssetManager::install: file %s not found", result));
			}

			++i;
		}
	}
	while (result);

	//-- preload portal objects
	i = 0;
	do
	{
		result = ConfigFile::getKeyString ("PreloadedAssets", "portalObject", i, 0);
		if (result)
		{
			if (TreeFile::exists (result))
			{
				//DEBUG_REPORT_LOG (true, ("Preloading asset %s\n", result));

				Object* object = new Object;
				PortalProperty *portalProperty = new PortalProperty(*object, result);
				object->addProperty(*portalProperty);
				portalProperty->createAppearance();
				portalProperty->clientSinglePlayerInitializeFirstTimeObject();

				s_objectList->push_back (object);
			} //lint !e429 // Custodial pointer 'portalProperty' has not been freed or returned // OK: It's owned by Object.

			++i;
		}
	}
	while (result);

	//-- preload animation state hierarchy template files (.ash files)
	i = 0;
	do
	{
		result = ConfigFile::getKeyString ("PreloadedAssets", "animationStateHierarchyTemplate", i, 0);
		if (result)
		{
			if (TreeFile::exists (result))
			{
				//DEBUG_REPORT_LOG (true, ("Preloading asset %s\n", result));

				AnimationStateHierarchyTemplate const * const animationStateHierarchyTemplate = AnimationStateHierarchyTemplateList::fetch(CrcLowerString(result));
				if (animationStateHierarchyTemplate)
					s_animationStateHierarchyTemplateList->push_back(animationStateHierarchyTemplate);
				else
					DEBUG_WARNING(true, ("PreloadedAssetManager::install: file %s not found", result));
			}

			++i;
		}
	}
	while (result);

	//-- preload logical animation table template files (.lat files)
	i = 0;
	do
	{
		result = ConfigFile::getKeyString ("PreloadedAssets", "logicalAnimationTableTemplate", i, 0);
		if (result)
		{
			if (TreeFile::exists (result))
			{
				//DEBUG_REPORT_LOG (true, ("Preloading asset %s\n", result));

				LogicalAnimationTableTemplate const * const logicalAnimationTableTemplate = LogicalAnimationTableTemplateList::fetch(CrcLowerString(result));
				if (logicalAnimationTableTemplate)
					s_logicalAnimationTableTemplateList->push_back(logicalAnimationTableTemplate);
				else
					DEBUG_WARNING(true, ("PreloadedAssetManager::install: file %s not found", result));
			}

			++i;
		}
	}
	while (result);

	//-- preload mesh generator template files (.mgn/.lmg files)
	i = 0;
	do
	{
		result = ConfigFile::getKeyString ("PreloadedAssets", "meshGeneratorTemplate", i, 0);
		if (result)
		{
			if (TreeFile::exists (result))
			{
				//DEBUG_REPORT_LOG (true, ("Preloading asset %s\n", result));

				MeshGeneratorTemplate const * const meshGeneratorTemplate = MeshGeneratorTemplateList::fetch(CrcLowerString(result));
				if (meshGeneratorTemplate)
					s_meshGeneratorTemplateList->push_back(meshGeneratorTemplate);
				else
					DEBUG_WARNING(true, ("PreloadedAssetManager::install: file %s not found", result));
			}

			++i;
		}
	}
	while (result);

	//-- preload texture files (.dds files)
	i = 0;
	do
	{
		result = ConfigFile::getKeyString ("PreloadedAssets", "texture", i, 0);
		if (result)
		{
			if (TreeFile::exists (result))
			{
				//DEBUG_REPORT_LOG (true, ("Preloading asset %s\n", result));

				Texture const * const texture = TextureList::fetch(TemporaryCrcString(result, true));
				if (texture)
					s_textureList->push_back(texture);
				else
					DEBUG_WARNING(true, ("PreloadedAssetManager::install: file %s not found", result));
			}

			++i;
		}
	}
	while (result);

	//-- preload palette files (.pal files)
	i = 0;
	do
	{
		result = ConfigFile::getKeyString ("PreloadedAssets", "palette", i, 0);
		if (result)
		{
			if (TreeFile::exists (result))
			{
				//DEBUG_REPORT_LOG (true, ("Preloading asset %s\n", result));

				PaletteArgb const * const paletteArgb = PaletteArgbList::fetch(TemporaryCrcString(result, true));
				if (paletteArgb)
					s_paletteList->push_back(paletteArgb);
				else
					DEBUG_WARNING(true, ("PreloadedAssetManager::install: file %s not found", result));
			}

			++i;
		}
	}
	while (result);

	//-- arrangement descriptor files (.iff files)
	i = 0;
	do
	{
		result = ConfigFile::getKeyString ("PreloadedAssets", "arrangementDescriptor", i, 0);
		if (result)
		{
			if (TreeFile::exists (result))
			{
				//DEBUG_REPORT_LOG (true, ("Preloading asset %s\n", result));

				ArrangementDescriptor const * const arrangementDescriptor = ArrangementDescriptorList::fetch(CrcLowerString(result));
				if (arrangementDescriptor)
					s_arrangementDescriptorList->push_back(arrangementDescriptor);
				else
					DEBUG_WARNING(true, ("PreloadedAssetManager::install: file %s not found", result));
			}

			++i;
		}
	}
	while (result);

	//-- slot descriptor files (.iff files)
	i = 0;
	do
	{
		result = ConfigFile::getKeyString ("PreloadedAssets", "slotDescriptor", i, 0);
		if (result)
		{
			if (TreeFile::exists (result))
			{
				//DEBUG_REPORT_LOG (true, ("Preloading asset %s\n", result));

				SlotDescriptor const * const slotDescriptor = SlotDescriptorList::fetch(CrcLowerString(result));
				if (slotDescriptor)
					s_slotDescriptorList->push_back(slotDescriptor);
				else
					DEBUG_WARNING(true, ("PreloadedAssetManager::install: file %s not found", result));
			}

			++i;
		}
	}
	while (result);

	//-- sound template files (.snd files)
	i = 0;
	do
	{
		result = ConfigFile::getKeyString("PreloadedAssets", "soundTemplate", i, 0);
		if (result)
		{
			if (TreeFile::exists(result))
				addSoundTemplate(result);

			++i;
		}
	}
	while (result);

#ifdef _DEBUG
	TreeFile::setLogTreeFileOpens (isLoggingFiles);
#endif

	if (asynchronousLoaderEnabled)
		AsynchronousLoader::enable ();

	//--
	ExitChain::add (remove, "PreloadedAssetManager::remove");
}

//===================================================================

void PreloadedAssetManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("PreloadedAssetManager::remove - not installed"));
	s_installed = false;

	{
		uint i;
		for (i = 0; i < s_appearanceTemplateList->size (); ++i)
			AppearanceTemplateList::release ((*s_appearanceTemplateList) [i]);

		delete s_appearanceTemplateList;
		s_appearanceTemplateList = 0;
	}

	{
		uint i;
		for (i = 0; i < s_objectTemplateList->size (); ++i)
			(*s_objectTemplateList) [i]->releaseReference ();

		delete s_objectTemplateList;
		s_objectTemplateList = 0;
	}

	{
		uint i;
		for (i = 0; i < s_shaderTemplateList->size (); ++i)
			(*s_shaderTemplateList) [i]->release ();

		delete s_shaderTemplateList;
		s_shaderTemplateList = 0;
	}

	{
		uint i;
		for (i = 0; i < s_logicalAnimationTableTemplateList->size (); ++i)
			(*s_logicalAnimationTableTemplateList) [i]->release ();

		delete s_logicalAnimationTableTemplateList;
		s_logicalAnimationTableTemplateList = 0;
	}

	{
		uint i;
		for (i = 0; i < s_animationStateHierarchyTemplateList->size (); ++i)
			(*s_animationStateHierarchyTemplateList) [i]->release ();

		delete s_animationStateHierarchyTemplateList;
		s_animationStateHierarchyTemplateList = 0;
	}

	{
		uint i;
		for (i = 0; i < s_skeletalAnimationTemplateList->size (); ++i)
			(*s_skeletalAnimationTemplateList) [i]->release ();

		delete s_skeletalAnimationTemplateList;
		s_skeletalAnimationTemplateList = 0;
	}

	{
		uint i;
		for (i = 0; i < s_clientEffectTemplateList->size (); ++i)
			(*s_clientEffectTemplateList) [i]->release ();

		delete s_clientEffectTemplateList;
		s_clientEffectTemplateList = 0;
	}

	{
		uint i;
		for (i = 0; i < s_objectList->size (); ++i)
			delete (*s_objectList) [i];

		delete s_objectList;
		s_objectList = 0;
	}

	{
		uint i;
		for (i = 0; i < s_meshGeneratorTemplateList->size (); ++i)
			(*s_meshGeneratorTemplateList) [i]->release ();

		s_meshGeneratorTemplateList->clear ();
		delete s_meshGeneratorTemplateList;
		s_meshGeneratorTemplateList = 0;
	}

	{
		uint i;
		for (i = 0; i < s_textureList->size (); ++i)
			(*s_textureList) [i]->release ();

		delete s_textureList;
		s_textureList = 0;
	}

	{
		uint i;
		for (i = 0; i < s_paletteList->size (); ++i)
			(*s_paletteList) [i]->release ();

		delete s_paletteList;
		s_paletteList = 0;
	}

	{
		uint i;
		for (i = 0; i < s_arrangementDescriptorList->size (); ++i)
			(*s_arrangementDescriptorList) [i]->release ();

		delete s_arrangementDescriptorList;
		s_arrangementDescriptorList = 0;
	}

	{
		uint i;
		for (i = 0; i < s_slotDescriptorList->size (); ++i)
			(*s_slotDescriptorList) [i]->release ();

		delete s_slotDescriptorList;
		s_slotDescriptorList = 0;
	}

	{
		for (uint i = 0; i < s_soundTemplateList->size (); ++i)
			SoundTemplateList::release((*s_soundTemplateList)[i]);

		delete s_soundTemplateList;
		s_soundTemplateList = 0;
	}
}

// ----------------------------------------------------------------------

void PreloadedAssetManager::addSoundTemplate(char const * const fileName)
{
	//DEBUG_REPORT_LOG(true,("Preloading asset %s\n", fileName));
	DEBUG_FATAL (!s_installed, ("PreloadedAssetManager::addSoundTemplate - not installed"));

	SoundTemplate const * const soundTemplate = SoundTemplateList::fetch(fileName);
	if (soundTemplate)
		s_soundTemplateList->push_back(soundTemplate);
	else
		DEBUG_WARNING(true, ("PreloadedAssetManager::install: file %s not found", fileName));
}

//===================================================================

