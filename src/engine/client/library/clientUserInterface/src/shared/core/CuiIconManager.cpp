//======================================================================
//
// CuiIconManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiIconManager.h"

#include "clientGame/ClientIncubatorManager.h"
#include "clientGame/ClientMacroManager.h"
#include "clientGame/ClientMissionObject.h"
#include "clientGame/ClientObjectUsabilityManager.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/DraftSchematicManager.h"
#include "clientGame/Game.h"
#include "clientGame/MoodManagerClient.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/ResourceContainerObject.h"
#include "clientObject/SpriteAppearance.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiIconManagerCallback.h"
#include "clientUserInterface/CuiIconManagerObjectProperties.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSocialsManager.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/Command.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/MoodManager.h"
#include "sharedGame/ObjectUsabilityManager.h"
#include "sharedGame/SharedMissionObjectTemplate.h"
#include "sharedGame/SharedResourceContainerObjectTemplate.h"
#include "sharedGame/SharedWaypointObjectTemplate.h"
#include "sharedGame/SocialsManager.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/VolumeContainer.h" 
#include "sharedUtility/CurrentUserOptionManager.h"

#include "UIButtonStyle.h"
#include "UIImageStyle.h"
#include "UIManager.h"
#include "UIPage.h"
#include "UIPalette.h"
#include "UIRectangleStyle.h"
#include "UIUtils.h"

#include <map>
#include <list>

//======================================================================

namespace CuiIconManagerNamespace
{
	enum StyleList 
	{
		isl_mood,
		isl_social,
		isl_ui,
		isl_commands,
		isl_state,
		isl_posture,
		isl_music,
		isl_faction,
		isl_jedi,
		isl_def,
		isl_fallback,
		isl_fallback_btn,
		isl_combat1,
		isl_combat2,
		isl_combat3,
		isl_social_btn,
		isl_ui_btn,
		isl_object,
		isl_inv,
		isl_astromech,
		isl_space
	};

	typedef std::map<StyleList, std::string /*Path*/> StyleMap;
	typedef std::map<int /*scene*/, StyleMap> StyleMapMap;

	StyleMapMap s_stylePathMap;
	std::string defaultString("default");
	std::string s_droidCommandKey("droid+droidcommand_");

	std::string const & GetStylePath(StyleList style)
	{
		int const activeStyleMapIndex = CuiIconManager::getActiveStyleMapIndex();
		StyleMap const & styleMap = s_stylePathMap[activeStyleMapIndex];
		StyleMap::const_iterator itStyle = styleMap.find(style);

		if (itStyle != styleMap.end())
		{
			return itStyle->second;
		}

		return defaultString;
	}

	namespace DisplayGroups
	{
		const uint32 jedi = Crc::normalizeAndCalculate ("jedi");
		const uint32 ship = Crc::normalizeAndCalculate ("ship");
	}

	const std::string s_waypointOverlayPath = "/Styles.New.active.rs_default";

	namespace Commands
	{
		const std::string CMD_uiClientSocial = "CMD_clientSocial";
		const std::string CMD_uiClientMood   = "CMD_clientMood";
	}

	struct ObjectInfo
	{
		CuiIconManagerCallback *         callback;
		NetworkId                        containedById;
		int                              lastAttribUpdate;
		bool                             lastActive;
		bool                             initialized;
		int                              lastContent1;
		int                              lastContent2;
		bool                             minimalTooltip;
		bool                             lastInsured;
		CuiIconManagerObjectProperties   objectProperties;


		explicit ObjectInfo (CuiIconManagerCallback * _callback = 0, bool _minimalTooltip = true, bool _lastInsured = false): 
		callback            (_callback),
		containedById       (), 
		lastAttribUpdate    (0),
		lastActive          (false),
		initialized         (false),
		lastContent1        (-1),
		lastContent2        (-1),
		minimalTooltip      (_minimalTooltip),
		lastInsured         (_lastInsured),
		objectProperties    ()
		{
		}

		ObjectInfo (CuiIconManagerCallback * _callback, Object & obj, bool _minimalTooltip = true, bool _lastInsured = false) :
		callback            (_callback),
		containedById       (),
		lastAttribUpdate    (0),
		lastActive          (false),
		initialized         (false),
		lastContent1        (-1),
		lastContent2        (-1),
		minimalTooltip      (_minimalTooltip),
		lastInsured         (_lastInsured),
		objectProperties    ()
		{
			ContainedByProperty * const containedByProp = obj.getContainedByProperty ();
			if (containedByProp)
				containedById = containedByProp->getContainedByNetworkId ();

			ClientObject const * const clientObject = obj.asClientObject ();
			if (clientObject)
				objectProperties.updateFromObject (*clientObject);
		}

		ObjectInfo (const ObjectInfo & rhs) :
		callback            (rhs.callback),
		containedById       (rhs.containedById),
		lastAttribUpdate    (rhs.lastAttribUpdate),
		lastActive          (false),
		initialized         (false),
		lastContent1        (-1),
		lastContent2        (-1),
		minimalTooltip      (rhs.minimalTooltip),
		lastInsured         (rhs.lastInsured),
		objectProperties    (rhs.objectProperties)
		{
		}

		ObjectInfo & operator= (const ObjectInfo & rhs)
		{
			if (this != &rhs)
			{
				callback           = rhs.callback;
				containedById      = rhs.containedById;
				lastAttribUpdate   = rhs.lastAttribUpdate;
				initialized        = false;
				lastActive         = rhs.lastActive;
				lastContent1       = -1;
				lastContent2       = -1;
				minimalTooltip     = rhs.minimalTooltip;
				lastInsured        = rhs.lastInsured;
				objectProperties   = rhs.objectProperties;
			}
			return *this;
		}
	};

	//-- ImageStyle cache.
	typedef std::map<CuiDragInfo, UIImageStyle *>  ImageStyleCache;
	typedef std::map<int /*scene*/, ImageStyleCache> ImageStyleCacheMap;
	ImageStyleCacheMap  s_imageStyleCache;

	ImageStyleCache & GetImageStyleCache()
	{
		return s_imageStyleCache[CuiIconManager::getActiveStyleMapIndex()];
	}


	//-- ButtonStyle cache.
	typedef std::map<CuiDragInfo, UIButtonStyle *> ButtonStyleCache;
	typedef std::map<int /*scene*/, ButtonStyleCache> ButtonStyleCacheMap;
	ButtonStyleCacheMap s_buttonStyleCache;

	ButtonStyleCache & GetButtonStyleCache()
	{
		return s_buttonStyleCache[CuiIconManager::getActiveStyleMapIndex()];
	}


	//-- Fallback Styles.
	typedef std::map<int /*scene*/, UIImageStyle *> FallbackStyleMap;
	FallbackStyleMap s_fallback;
	UIImageStyle * GetFallback()
	{	
		UIImageStyle * fallbackStyle = NULL;
		
		FallbackStyleMap::const_iterator itFallback = s_fallback.find(CuiIconManager::getActiveStyleMapIndex());
		if (itFallback != s_fallback.end())
		{
			fallbackStyle = itFallback->second;
		}

		return fallbackStyle;
	}

	//-- Fallback Button Styles.
	typedef std::map<int /*scene*/, UIButtonStyle *> FallbackButtonStyleMap;
	FallbackButtonStyleMap s_fallbackButton;
	UIButtonStyle * GetFallbackButton()
	{	
		UIButtonStyle * fallbackButtonStyle = NULL;
		
		FallbackButtonStyleMap::const_iterator itFallbackButton = s_fallbackButton.find(CuiIconManager::getActiveStyleMapIndex());
		if (itFallbackButton != s_fallbackButton.end())
		{
			fallbackButtonStyle = itFallbackButton->second;
		}
		
		return fallbackButtonStyle;
	}

	// Viewer map.
	typedef stdmultimap<CuiWidget3dObjectListViewer *, ObjectInfo>::fwd ViewerMap;
	typedef std::map<int /*scene*/, ViewerMap> ViewerMapMap;
	ViewerMapMap s_viewerMapMap;
	ViewerMap & GetActiveViewMap()
	{
		return s_viewerMapMap[CuiIconManager::getActiveStyleMapIndex()];
	}

	bool s_installed = false;

	UIColor s_colorTextInsured;
	UIColor s_colorEquipped;
	UIColor s_colorDisabled;
	UIColor s_appearanceEquipped;

	const UILowerString s_palColorTextNormal  ("text1");
	const UILowerString s_palColorTextMagic   ("textMagic");
	const UILowerString s_palColorTextInsured ("textInsured");
	const UILowerString s_palColorEquipped    ("equipped");
	const UILowerString s_palColorDisabled    ("disabledObject");
	const UILowerString s_palColorAppearance  ("appearanceEquipped");


	bool ms_entangleResistance;
	bool ms_coldResist;
	bool ms_conductivity;
	bool ms_decayResist;
	bool ms_flavor;
	bool ms_heatResist;
	bool ms_malleability;
	bool ms_potentialEnergy;
	bool ms_overallQuality;
	bool ms_shockResistance;
	bool ms_toughness;

	bool canBeDisabled(TangibleObject const & obj)
	{
		int const got = obj.getGameObjectType();

		return ((obj.getNetworkId() != NetworkId::cms_invalid) && !ClientObject::isFakeNetworkId(obj.getNetworkId())) &&
			(GameObjectTypes::isTypeOf(got, SharedObjectTemplate::GOT_weapon) || 
			GameObjectTypes::isTypeOf(got, SharedObjectTemplate::GOT_armor) || 
			GameObjectTypes::isTypeOf(got, SharedObjectTemplate::GOT_jewelry) || 
			GameObjectTypes::isTypeOf(got, SharedObjectTemplate::GOT_clothing) ||
			GameObjectTypes::isTypeOf(got, SharedObjectTemplate::GOT_tool) ||
			GameObjectTypes::isTypeOf(got, SharedObjectTemplate::GOT_component_saber_crystal));
	}
}

using namespace CuiIconManagerNamespace;

//----------------------------------------------------------------------

const std::string  CuiIconManager::ms_fontPathLarge = "default_16";
const std::string  CuiIconManager::ms_fontPathSmall = "default_12";
UIColor            CuiIconManager::ms_colorTextMagic;
UIColor            CuiIconManager::ms_colorTextNormal;
int CuiIconManager::ms_activeStyleMap = 0;

const UILowerString CuiIconManager::Properties::ModifyBackgroundOpacity = UILowerString ("ModifyBackgroundOpacity");

//----------------------------------------------------------------------

void CuiIconManager::install ()
{
	DEBUG_FATAL (s_installed, ("installed"));

	(s_stylePathMap[0])[ isl_mood        ] = "/styles.icon.mood.";
	(s_stylePathMap[0])[ isl_social      ] = "/styles.icon.social.";
	(s_stylePathMap[0])[ isl_ui          ] = "/styles.icon.ui.";
	(s_stylePathMap[0])[ isl_commands    ] = "/styles.icon.command.";
	(s_stylePathMap[0])[ isl_state       ] = "/styles.icon.state.";
	(s_stylePathMap[0])[ isl_posture     ] = "/styles.icon.posture.";
	(s_stylePathMap[0])[ isl_music       ] = "/styles.icon.music.";
	(s_stylePathMap[0])[ isl_faction     ] = "/styles.icon.faction.";
	(s_stylePathMap[0])[ isl_jedi        ] = "/styles.icon.jedi.";
	(s_stylePathMap[0])[ isl_def         ] = "default";
	(s_stylePathMap[0])[ isl_fallback    ] = "/styles.icon.fallback";
	(s_stylePathMap[0])[ isl_fallback_btn] = "/styles.icon.button.neutral.style";
	(s_stylePathMap[0])[ isl_combat1     ] = "/styles.icon.button.combat1.style";
	(s_stylePathMap[0])[ isl_combat2     ] = "/styles.icon.button.combat2.style";
	(s_stylePathMap[0])[ isl_combat3     ] = "/styles.icon.button.combat3.style";
	(s_stylePathMap[0])[ isl_social_btn  ] = "/styles.icon.button.social.style";
	(s_stylePathMap[0])[ isl_ui_btn      ] = "/styles.icon.button.ui.style";
	(s_stylePathMap[0])[ isl_object      ] = "/styles.icon.button.3d.style";
	(s_stylePathMap[0])[ isl_inv         ] = "/styles.icon.button.inv.style";
	(s_stylePathMap[0])[ isl_astromech   ] = "/styles.icon.droid.";
	(s_stylePathMap[0])[ isl_space       ] = "/styles.space.icon.command.";

	(s_stylePathMap[1])[ isl_mood        ] = "/styles.icon.mood.";
	(s_stylePathMap[1])[ isl_social      ] = "/styles.icon.social.";
	(s_stylePathMap[1])[ isl_ui          ] = "/styles.icon.ui.";
	(s_stylePathMap[1])[ isl_commands    ] = "/styles.icon.command.";
	(s_stylePathMap[1])[ isl_state       ] = "/styles.icon.state.";
	(s_stylePathMap[1])[ isl_posture     ] = "/styles.icon.posture.";
	(s_stylePathMap[1])[ isl_music       ] = "/styles.icon.music.";
	(s_stylePathMap[1])[ isl_faction     ] = "/styles.icon.faction.";
	(s_stylePathMap[1])[ isl_jedi        ] = "/styles.icon.jedi.";
	(s_stylePathMap[1])[ isl_def         ] = "default";
	(s_stylePathMap[1])[ isl_fallback    ] = "/styles.icon.fallback";
	(s_stylePathMap[1])[ isl_fallback_btn] = "/styles.icon.button.neutral.style";
	(s_stylePathMap[1])[ isl_combat1     ] = "/styles.icon.button.combat1.style";
	(s_stylePathMap[1])[ isl_combat2     ] = "/styles.icon.button.combat2.style";
	(s_stylePathMap[1])[ isl_combat3     ] = "/styles.icon.button.combat3.style";
	(s_stylePathMap[1])[ isl_social_btn  ] = "/styles.icon.button.social.style";
	(s_stylePathMap[1])[ isl_ui_btn      ] = "/styles.icon.button.ui.style";
	(s_stylePathMap[1])[ isl_object      ] = "/styles.icon.button.3d.style";
	(s_stylePathMap[1])[ isl_inv         ] = "/styles.icon.button.inv.style";
	(s_stylePathMap[1])[ isl_astromech   ] = "/styles.space.icon.droid.";
	(s_stylePathMap[1])[ isl_space       ] = "/styles.space.icon.command.";

	s_installed = true;

	CurrentUserOptionManager::registerOption(ms_entangleResistance, "ClientUserInterface", "entangleResistance");
	CurrentUserOptionManager::registerOption(ms_coldResist, "ClientUserInterface", "coldResist");
	CurrentUserOptionManager::registerOption(ms_conductivity, "ClientUserInterface", "conductivity");
	CurrentUserOptionManager::registerOption(ms_decayResist, "ClientUserInterface", "decayResist");
	CurrentUserOptionManager::registerOption(ms_flavor, "ClientUserInterface", "flavor");
	CurrentUserOptionManager::registerOption(ms_heatResist, "ClientUserInterface", "heatResist");
	CurrentUserOptionManager::registerOption(ms_malleability, "ClientUserInterface", "malleability");
	CurrentUserOptionManager::registerOption(ms_potentialEnergy, "ClientUserInterface", "potentialEnergy");
	CurrentUserOptionManager::registerOption(ms_overallQuality, "ClientUserInterface", "overallQuality");
	CurrentUserOptionManager::registerOption(ms_shockResistance, "ClientUserInterface", "shockResistance");
	CurrentUserOptionManager::registerOption(ms_toughness, "ClientUserInterface", "toughness");
}

//----------------------------------------------------------------------

void CuiIconManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("!installed"));

	for(int imageStyleCacheCount = 0; imageStyleCacheCount < Game::ST_numTypes; ++imageStyleCacheCount)
	{
		ImageStyleCache const & imageStyleCache = s_imageStyleCache[imageStyleCacheCount];
		for (ImageStyleCache::const_iterator itImage = imageStyleCache.begin (); itImage != imageStyleCache.end (); ++itImage)
		{
			UIImageStyle * const style = (*itImage).second;
			if (style)
				style->Detach (0);
		}
	}
	s_imageStyleCache.clear();
	

	for(int buttonStyleCacheCount = 0; buttonStyleCacheCount < Game::ST_numTypes; ++buttonStyleCacheCount)
	{
		ButtonStyleCache const & buttonStyleCache = s_buttonStyleCache[buttonStyleCacheCount];
		for (ButtonStyleCache::const_iterator itButton = buttonStyleCache.begin (); itButton != buttonStyleCache.end (); ++itButton)
		{
			UIButtonStyle * const style = (*itButton).second;
			if (style)
				style->Detach (0);
		}
	}
	s_buttonStyleCache.clear();


	for(FallbackStyleMap::iterator itFallback = s_fallback.begin(); itFallback != s_fallback.end(); ++itFallback)
	{
		UIImageStyle * fallbackStyle = itFallback->second;
		if (fallbackStyle)
			fallbackStyle->Detach(0);
	}
	s_fallback.clear();
	

	for(FallbackButtonStyleMap::iterator itFallbackButton = s_fallbackButton.begin(); itFallbackButton != s_fallbackButton.end(); ++itFallbackButton)
	{
		UIButtonStyle * fallbackButtonStyle = itFallbackButton->second;
		if (fallbackButtonStyle)
			fallbackButtonStyle->Detach(0);
	}
	s_fallbackButton.clear();

	s_installed = false;
}

//----------------------------------------------------------------------

UIImageStyle * CuiIconManager::getFallback ()
{
	UIImageStyle * fallbackStyle = GetFallback();
	if (!fallbackStyle)
	{
		const UIPage * const rootPage = UIManager::gUIManager().GetRootPage();
		NOT_NULL (rootPage);
		fallbackStyle = safe_cast<UIImageStyle *>(rootPage->GetObjectFromPath(GetStylePath(isl_fallback).c_str()));

		if (fallbackStyle)
		{
			if (s_fallback.insert(std::make_pair(getActiveStyleMapIndex(), fallbackStyle)).second)
			{
				fallbackStyle->Attach(0);
			}
		}
	}

	return fallbackStyle;
}

//----------------------------------------------------------------------

std::string CuiIconManager::findIconPath (const CuiDragInfo & info)
{
	DEBUG_FATAL (!s_installed, ("!installed"));
	switch (info.type)
	{
		//----------------------------------------------------------------------


		//----------------------------------------------------------------------

	case CuiDragInfoTypes::CDIT_object:
		return std::string ();
		break;

	case CuiDragInfoTypes::CDIT_macro:
		{
			const ClientMacroManager::Data * const data = ClientMacroManager::findMacroData (info.cmd);
			if (data)
			{
				const UIImageStyle * const imageStyle = CuiIconManager::findIcon (data->icon);
				if (imageStyle)
					return imageStyle->GetFullPath ();
			}
			
			return std::string ();
		}

		break;

	case CuiDragInfoTypes::CDIT_command:
		{
			if (!info.cmd.empty ())
				return GetStylePath(isl_ui) + info.cmd;

			if (info.str.empty ())
			{
				WARNING (true, ("Bad str"));
				return std::string ();
			}

			std::string cmdCopy = info.str;
			if (cmdCopy.empty ())
				return std::string ();

			if (cmdCopy [0] == '/')
				cmdCopy.erase (0, 1);

			int const size = cmdCopy.size();

			if ((size > 2)
					&& (cmdCopy[size - 2] == '_')
					&& (isdigit(cmdCopy[size - 1])))
			{
				cmdCopy.resize(size - 2);
			}

			/*
			int const plusPosition = static_cast<int>(cmdCopy.rfind("+"));

			if (plusPosition != std::string::npos)
			{
				cmdCopy.resize(plusPosition);
			}
			*/
			
			std::string token;

			size_t endpos = 0;
			if (Unicode::getFirstToken (cmdCopy, 0, endpos, token))
			{
				//- see if it is a social

				const uint32 socialId = CuiSocialsManager::getSocialIdForCommandName (token);

				if (socialId)
				{
					std::string socialName;
					if (!SocialsManager::getSocialNameByType (socialId, socialName))
					{
						WARNING (true, ("Unable to find social for id: %d (localized %s)", socialId, token.c_str ()));
						return std::string ();
					}
					return GetStylePath(isl_social) + socialName;
				}

				const std::string & lowerMoodCommand = Unicode::wideToNarrow (Unicode::toLower (MoodManagerClient::getMoodCommand ()));

				//-- see if it is a mood

				if (lowerMoodCommand == Unicode::toLower (token))
				{
					std::string localizedMoodName;
					if (Unicode::getFirstToken (cmdCopy, endpos+1, endpos, localizedMoodName))
					{
						if (!_stricmp (localizedMoodName.c_str (), "none"))
						{
							return GetStylePath(isl_mood) + "none";
						}
						else
						{
							const uint32 moodId = MoodManagerClient::getMoodByName (Unicode::narrowToWide (localizedMoodName));
							
							if (moodId)
							{
								std::string moodName;
								if (!MoodManager::getCanonicalName (moodId, moodName))
								{
									WARNING (true, ("Unable to find mood for id: %d (localized %s)", moodId, localizedMoodName.c_str ()));
									return std::string ();
								}
								else
									return GetStylePath(isl_mood) + moodName;
							}
						}
					}
				}

				//- ui action, strip off the parameters
				if (!_strnicmp (cmdCopy.c_str (), "ui action ", 10))
				{
					const size_t paramStartSpace = cmdCopy.find (' ', size_t(10));
					if (paramStartSpace != static_cast<size_t>(std::string::npos))
					{
						return GetStylePath(isl_ui) + cmdCopy.substr (10, paramStartSpace - 10);
					}
					else
						WARNING (true, ("CuiIconManager bad ui action [%s]", cmdCopy.c_str ()));
				}				

				cmdCopy = token;
			}

			const Command & commandTableCmd = CommandTable::getCommand (Crc::normalizeAndCalculate (cmdCopy.c_str ()));
			if (!commandTableCmd.isNull ())			
			{
				const uint32 displayGroup = commandTableCmd.m_displayGroup;

					
				if (displayGroup == DisplayGroups::jedi)
					return GetStylePath(isl_jedi) + cmdCopy;
				if (displayGroup == DisplayGroups::ship)
					return GetStylePath(isl_space) + cmdCopy;
			}
					
			// Get the droid command icons to show up.
			size_t const droidIndex = cmdCopy.find(s_droidCommandKey);
			if (droidIndex != std::string::npos)
			{
				return GetStylePath(isl_astromech) + std::string("droid") + cmdCopy.erase(0, s_droidCommandKey.size());
			}

			/*
			if(commandTableCmd.isNull())
			{
				//HACK HACK HACK If we get this far with a command that doesn't exist perhaps it's a space command
				return GetStylePath(isl_space) + cmdCopy;
			}
			*/
			
			return GetStylePath(isl_commands) + cmdCopy;
		}
		break;
	}

	return std::string ();
}

//----------------------------------------------------------------------

UIImageStyle * CuiIconManager::findIconImageStyle( const std::string &inputPath, bool recurse )
{
	std::string path( inputPath );

	const UIPage * const rootPage = UIManager::gUIManager ().GetRootPage ();
	NOT_NULL (rootPage);

	UIImageStyle * imageStyle = safe_cast<UIImageStyle *>(rootPage->GetObjectFromPath (path.c_str ()));

	if ( ! imageStyle )
	{
		std::string redirectPath;

		if (rootPage->GetPropertyNarrow (UILowerString (path), redirectPath))
		{
			if (!redirectPath.empty ())
			{
				if (redirectPath [0] != '/')
				{
					const size_t dotpos = path.rfind ('.');

					if (dotpos != static_cast<size_t>(std::string::npos))
					{
						path.erase (dotpos+1);
						path.append (redirectPath);
					}
				}
				else
					path = redirectPath;

				imageStyle = safe_cast<UIImageStyle *>(rootPage->GetObjectFromPath (path.c_str ()));
			}
		}
	}

	if ( recurse && !imageStyle )
	{

		bool isSpaceIcon = path.find( "/styles.space." ) != std::string::npos;

		if ( isSpaceIcon )
		{
			path.replace( 0, std::string( "/styles.space." ).size() , "/styles." );
		}
		else
		{
			path.replace( 0, std::string( "/styles." ).size(), "/styles.space." );
		}

		imageStyle = findIconImageStyle( path, false );
	}

	return imageStyle;

}
//----------------------------------------------------------------------

UIImageStyle * CuiIconManager::findIconImageStyle (const CuiDragInfo & info )
{
	if (info.type != CuiDragInfoTypes::CDIT_macro)
	{
		ImageStyleCache const & imageStyleCache = GetImageStyleCache();
		const ImageStyleCache::const_iterator it = imageStyleCache.find (info);
		
		if (it != imageStyleCache.end ())
			return (*it).second;
	}

	DEBUG_FATAL (!s_installed, ("!installed"));

	std::string path = findIconPath (info);


	const UIPage * const rootPage = UIManager::gUIManager ().GetRootPage ();
	NOT_NULL (rootPage);

	UIImageStyle * imageStyle = findIconImageStyle( path, true );

	if (!imageStyle)
	{
		imageStyle = CuiIconManager::getFallback ();
		WARNING (true, ("Retrieving fallback icon for [%s] cmd=[%s] str=[%s]", CuiDragInfo::getDragTypeString (info.type), info.cmd.c_str (), info.str.c_str ()));
	}
		
	if (info.type != CuiDragInfoTypes::CDIT_macro)
	{
		ImageStyleCache & imageStyleCache = GetImageStyleCache();
		if (imageStyleCache.insert(std::make_pair(info, imageStyle)).second)
		{
			if(imageStyle)
			{
				imageStyle->Attach(0);
			}
		}
	}

	return imageStyle;
}

//----------------------------------------------------------------------

const std::string & CuiIconManager::getSocialCommand ()
{
	return Commands::CMD_uiClientSocial;
}

//----------------------------------------------------------------------

const std::string & CuiIconManager::getMoodCommand   ()
{
	return Commands::CMD_uiClientMood;
}

//----------------------------------------------------------------------

CuiWidget3dObjectListViewer * CuiIconManager::createObjectIcon (ClientObject & obj, const char * const dragType)
{
	CuiWidget3dObjectListViewer * viewer = createObjectIcon(static_cast<Object&>(obj), dragType);
	viewer->SetName (std::string ("IV_") + Unicode::wideToNarrow (obj.getLocalizedName ()));
	return viewer;
}

//----------------------------------------------------------------------

CuiWidget3dObjectListViewer * CuiIconManager::createObjectIcon (Object & obj, const char * const dragType)
{
	UIPage * const root = NON_NULL (UIManager::gUIManager ().GetRootPage ());

	CuiWidget3dObjectListViewer * const viewer = new CuiWidget3dObjectListViewer;

	if (dragType)
		viewer->SetPropertyNarrow     (UIWidget::PropertyName::DragType,    dragType);

	viewer->SetPropertyNarrow (CuiWidget3dObjectListViewer::PropertyName::TextStyleBottom, ms_fontPathSmall);
	viewer->SetPropertyNarrow (CuiWidget3dObjectListViewer::PropertyName::TextStyleTop,    ms_fontPathSmall);
	
	viewer->SetParent (root);
	{
		UIButtonStyle * const buttonStyle = safe_cast<UIButtonStyle *>(root->GetObjectFromPath (GetStylePath(isl_inv).c_str (), TUIButtonStyle));
		
		if (buttonStyle)
		{
			buttonStyle->ApplyToWidget (*viewer);
		}
		viewer->Link ();
	}
	viewer->SetParent (0);
				
	const SpriteAppearance * const sprite = dynamic_cast<const SpriteAppearance *>(obj.getAppearance ());
	if (sprite)
		viewer->SetPropertyNarrow (CuiWidget3dObjectListViewer::PropertyName::LightAmbientColor, "#ffffff");

	viewer->SetBackgroundColor   (UIColor::white);
	viewer->SetBackgroundTint    (UIColor::white);

	viewer->setAlterObjects (false);
	viewer->addObject                (obj);
	viewer->setCameraPitch           (0.4f);
	viewer->setCameraLookAtCenter    (true);
	viewer->setDragYawOk             (false);
	viewer->setPaused                (false);
	viewer->SetDragable              (false);	
	viewer->SetContextCapable        (true, false);
	viewer->SetContextCapable        (true, true);
	if(CuiPreferences::getRotateInventoryObjects())
		viewer->setRotateSpeed           (1.0f);
	else
		viewer->setRotateSpeed           (0.0f);
	viewer->setCameraLodBias         (2.0f);
	viewer->setCameraForceTarget     (true);
	viewer->setAutoZoomOutOnly       (true);
	viewer->setCameraTransformToObj  (true);
	viewer->setCameraLodBiasOverride (false);
	
	viewer->setDrawName (true);	
	
	if (dragType)
	{
		viewer->SetDragable     (true);
		CuiWidget3dObjectListViewer * const dragWidget = safe_cast<CuiWidget3dObjectListViewer *>(viewer->DuplicateObject ());
		dragWidget->SetName     ("Dragger");
		
		dragWidget->SetParent   (root);
		dragWidget->Link        ();
		dragWidget->SetParent   (0);
		
		dragWidget->setAlterObjects (false);
		dragWidget->addObject               (obj);
		dragWidget->SetLocation             (UIPoint::zero);
		dragWidget->SetSize                 (UISize (64L, 64L));
		dragWidget->setPaused               (false);
		dragWidget->setCameraForceTarget    (true);
		dragWidget->setAutoZoomOutOnly      (true);
		dragWidget->setCameraTransformToObj (true);
		
		viewer->SetCustomDragWidget (dragWidget);
	}
	
	return viewer;
}

//----------------------------------------------------------------------

void CuiIconManager::registerObjectIcon (CuiWidget3dObjectListViewer & viewer, CuiIconManagerCallback * callback, bool minimalTooltip)
{
	Object * const object = viewer.getLastObject ();

	if (object)
	{
		IGNORE_RETURN(GetActiveViewMap().insert(std::make_pair (&viewer, ObjectInfo (callback, *object, minimalTooltip))));
		ObjectAttributeManager::requestUpdate (object->getNetworkId ());
	}
	else
		IGNORE_RETURN(GetActiveViewMap().insert(std::make_pair (&viewer, ObjectInfo (callback, minimalTooltip))));

	viewer.Attach (0);
}

//----------------------------------------------------------------------

void CuiIconManager::unregisterObjectIcon (CuiWidget3dObjectListViewer & viewer)
{
	ViewerMap & viewerMap = GetActiveViewMap();
	ViewerMap::iterator it;
	while ((it = viewerMap.find(&viewer)) != viewerMap.end())
	{
		viewer.Detach(0);
		viewerMap.erase(it);
	}
}

//----------------------------------------------------------------------

void CuiIconManager::unregisterIconsForPage (UIPage & page)
{
	const UIBaseObject::UIObjectList & olist = page.GetChildrenRef ();

	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		UIBaseObject * const obj = NON_NULL (*it);
		if (obj->IsA (TUIWidget))
		{
			CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(obj);
			if (viewer)
				CuiIconManager::unregisterObjectIcon (*viewer);
		}
	}
}

//----------------------------------------------------------------------

void CuiIconManager::update (float deltaTimeSecs)
{
	updateColors ();

	UNREF (deltaTimeSecs);

	CreatureObject * const player = Game::getPlayerCreature ();
	if (!player)
		return;

	Unicode::String header;
	Unicode::String desc;
	Unicode::String attribs;
	
	ViewerMap & viewMap = GetActiveViewMap();
	for (ViewerMap::iterator it = viewMap.begin (); it != viewMap.end (); ++it)
	{
		CuiWidget3dObjectListViewer * const viewer = NON_NULL ((*it).first);
		ObjectInfo &                        info   = (*it).second;

		Object * const obj = viewer->getLastObject ();

		if (!obj)
			continue;
		
		bool needsCursorUpdate = false;

		ClientObject * const clientObject = obj->asClientObject ();
		const ResourceContainerObject * rc = 0;
		const VolumeContainer         * volumeContainer = 0;
		TangibleObject * tangible   = 0;

		if (clientObject)
		{
			tangible = clientObject->asTangibleObject ();

			if (clientObject->getObjectType () == SharedResourceContainerObjectTemplate::SharedResourceContainerObjectTemplate_tag)
			{
				rc = safe_cast<const ResourceContainerObject *>(clientObject);
				const int quantity = rc->getQuantity ();
				
				if (quantity != info.lastContent1)
				{
					header.clear ();
					if (quantity >= 1000)
					{
						UIUtils::FormatInteger (header, quantity / 1000);
						header.push_back ('k');
					}
					else
						UIUtils::FormatInteger (header,quantity);
					
					ObjectAttributeManager::AttributeVector attributeVector;
					int count = 0;
					int sum = 0;
					if (ObjectAttributeManager::getAttributes(rc->getResourceType(), attributeVector, false))
					{
						for (ObjectAttributeManager::AttributeVector::iterator i = attributeVector.begin(); i != attributeVector.end(); ++i)
						{
							ObjectAttributeManager::AttributePair & attributePair = *i;

							if (ms_entangleResistance && attributePair.first == "entangle_resistance")
								count += 1, sum += atoi(Unicode::wideToNarrow(attributePair.second).c_str());

							if (ms_coldResist && attributePair.first == "res_cold_resist")
								count +=1, sum += atoi(Unicode::wideToNarrow(attributePair.second).c_str());

							if (ms_conductivity && attributePair.first == "res_conductivity")
								count +=1, sum += atoi(Unicode::wideToNarrow(attributePair.second).c_str());

							if (ms_decayResist && attributePair.first == "res_decay_resist")
								count +=1, sum += atoi(Unicode::wideToNarrow(attributePair.second).c_str());

							if (ms_flavor && attributePair.first == "res_flavor")
								count +=1, sum += atoi(Unicode::wideToNarrow(attributePair.second).c_str());

							if (ms_heatResist && attributePair.first == "res_heat_resist")
								count +=1, sum += atoi(Unicode::wideToNarrow(attributePair.second).c_str());

							if (ms_malleability && attributePair.first == "res_malleability")
								count +=1, sum += atoi(Unicode::wideToNarrow(attributePair.second).c_str());

							if (ms_potentialEnergy && attributePair.first == "res_potentialEnergy")
								count +=1, sum += atoi(Unicode::wideToNarrow(attributePair.second).c_str());

							if (ms_overallQuality && attributePair.first == "res_quality")
								count +=1, sum += atoi(Unicode::wideToNarrow(attributePair.second).c_str());

							if (ms_shockResistance && attributePair.first == "res_shock_resistance")
								count +=1, sum += atoi(Unicode::wideToNarrow(attributePair.second).c_str());

							if (ms_toughness && attributePair.first == "res_toughness")
								count +=1, sum += atoi(Unicode::wideToNarrow(attributePair.second).c_str());
						}

						if (count)
						{
							Unicode::String amount;
							UIUtils::FormatInteger (amount , sum / count);
							header.push_back ('\n');
							header += Unicode::narrowToWide("\\#pcontrast1 ");
							header += amount;
						}
					}

					viewer->setText (CuiWidget3dObjectListViewer::TOT_topRight, header, ms_colorTextNormal);
					info.lastContent1 = quantity;
				}
			}
			else
			{
				const int got = clientObject->getGameObjectType ();
				
				if (got == SharedObjectTemplate::GOT_misc_food || 
					got == SharedObjectTemplate::GOT_misc_drink || 
					got == SharedObjectTemplate::GOT_misc_pharmaceutical || 
					got == SharedObjectTemplate::GOT_weapon_ranged_thrown ||
					got == SharedObjectTemplate::GOT_weapon_heavy_mine ||
					got == SharedObjectTemplate::GOT_misc_factory_crate ||
					got == SharedObjectTemplate::GOT_tool_crafting ||
					(tangible && tangible->getCount () > 0))
				{
					if (tangible)
					{
						int const quantity = tangible->getCount ();
						if (info.lastContent1 != quantity)
						{
							header.clear ();
							
							if (quantity != 0)
							{
								if (quantity >= 1000)
								{
									UIUtils::FormatInteger (header, quantity / 1000);
									header.push_back ('k');
								}
								else
									UIUtils::FormatInteger (header,quantity);
							}

							viewer->setText (CuiWidget3dObjectListViewer::TOT_topRight, header, ms_colorTextNormal);
							info.lastContent1 = quantity;
						}
					}
				}
				
				else
				{
					if (got != SharedObjectTemplate::GOT_data_manufacturing_schematic && got != SharedObjectTemplate::GOT_chronicles_quest_holocron && got != SharedObjectTemplate::GOT_chronicles_quest_holocron_recipe )
					{
						volumeContainer = ContainerInterface::getVolumeContainer (*clientObject);
						if (volumeContainer)
						{
							int const quantity = volumeContainer->getCurrentVolume ();
							if (info.lastContent1 != quantity)
							{
								header.clear ();

								if (quantity >= 1000)
								{
									UIUtils::FormatInteger (header, quantity / 1000);
									header.push_back ('k');
								}
								else
									UIUtils::FormatInteger (header,quantity);

								viewer->setText (CuiWidget3dObjectListViewer::TOT_topRight, header, ms_colorTextNormal);
								info.lastContent1 = quantity;
							}
						}
					}
				}
			}
			
			if (tangible)
			{
				const bool isInsured = tangible->hasCondition (TangibleObject::C_insured);
				if (isInsured && (!info.initialized || !info.lastInsured))
				{
					static const Unicode::String s_insuredString = Unicode::narrowToWide ("(i)");
					viewer->setText (CuiWidget3dObjectListViewer::TOT_topLeft, s_insuredString, s_colorTextInsured);
					info.lastInsured = isInsured;
				}
				else if (!isInsured && (!info.initialized || info.lastInsured))
				{
					viewer->setText (CuiWidget3dObjectListViewer::TOT_topLeft, Unicode::emptyString, s_colorTextInsured);
					info.lastInsured = isInsured;
				}
			}
		}

		NetworkId containedById;
		ContainedByProperty * const containedByProp = obj->getContainedByProperty ();
		if (containedByProp)
			containedById = containedByProp->getContainedByNetworkId ();

		//-- not been initialized or containedBy has changed or damageTaken has changed
		if (!info.initialized || containedById != info.containedById || (tangible && tangible->getDamageTaken() != info.objectProperties.getDamageTaken()))
		{
			info.containedById = containedById;

			//-- default to true
			bool modifyBgOpacity = true;
			viewer->GetPropertyBoolean (Properties::ModifyBackgroundOpacity, modifyBgOpacity);
						
			//----------------------------------------------------------------------
			//-- tangibles

			static UILowerString const prop_palBgTint     = UILowerString ("PalBgTint");
			static UILowerString const prop_savePalBgTint = UILowerString ("SavePalBgTint");
			
			if (tangible && player == ContainerInterface::getContainedByObject (*tangible))
			{
				Unicode::String palBgTint;
				if (viewer->GetProperty (prop_palBgTint, palBgTint))
				{
					IGNORE_RETURN(viewer->SetProperty(prop_savePalBgTint, palBgTint));
					viewer->RemoveProperty (prop_palBgTint);
				}
				if (modifyBgOpacity)
					viewer->SetBackgroundOpacity (0.8f);
				
				viewer->SetBackgroundTint    (s_colorEquipped);
			}
			else if (tangible && player->getAppearanceInventoryObject() && player->getAppearanceInventoryObject() == ContainerInterface::getContainedByObject(*tangible))
			{
				Unicode::String palBgTint;
				if (viewer->GetProperty (prop_palBgTint, palBgTint))
				{
					IGNORE_RETURN(viewer->SetProperty(prop_savePalBgTint, palBgTint));
					viewer->RemoveProperty (prop_palBgTint);
				}
				if (modifyBgOpacity)
					viewer->SetBackgroundOpacity (0.8f);

				viewer->SetBackgroundTint    (UIColor::cyan);
			}
			else if (tangible && (tangible->getGameObjectType() == SharedObjectTemplate::GOT_misc_appearance_only || tangible->getGameObjectType() == SharedObjectTemplate::GOT_misc_appearance_only_invisible ) )
			{
				Unicode::String palBgTint;
				if (viewer->GetProperty (prop_palBgTint, palBgTint))
				{
					IGNORE_RETURN(viewer->SetProperty(prop_savePalBgTint, palBgTint));
					viewer->RemoveProperty (prop_palBgTint);
				}
				if (modifyBgOpacity)
					viewer->SetBackgroundOpacity (0.8f);

				viewer->SetBackgroundTint    (s_appearanceEquipped);
			}
			else if (tangible && (tangible->getDamageTaken() >= tangible->getMaxHitPoints()) && canBeDisabled(*tangible))
			{
				Unicode::String palBgTint;
				if (viewer->GetProperty (prop_palBgTint, palBgTint))
				{
					IGNORE_RETURN(viewer->SetProperty    (prop_savePalBgTint, palBgTint));
					viewer->RemoveProperty (prop_palBgTint);
				}
				if (modifyBgOpacity)
					viewer->SetBackgroundOpacity (0.8f);
				
				viewer->SetBackgroundTint    (s_colorDisabled);
			}
			else if (!ClientObjectUsabilityManager::canWear(player, tangible))
			{
				Unicode::String palBgTint;
				if(viewer->GetProperty(prop_palBgTint, palBgTint))
				{
					IGNORE_RETURN(viewer->SetProperty(prop_savePalBgTint, palBgTint));
					viewer->RemoveProperty(prop_palBgTint);
				}
				if(modifyBgOpacity)
					viewer->SetBackgroundOpacity(0.8f);
				
				viewer->SetBackgroundTint(s_colorDisabled);
			}
			else
			{
				if (modifyBgOpacity)
					viewer->SetBackgroundOpacity (0.0f);
				
				Unicode::String palBgTint;
				if (viewer->GetProperty (prop_savePalBgTint, palBgTint))
				{
					IGNORE_RETURN(viewer->SetProperty(prop_palBgTint, palBgTint));
					viewer->RemoveProperty (prop_savePalBgTint);
				}
				else if (!viewer->HasProperty (prop_palBgTint))
					viewer->SetBackgroundTint    (UIColor::white);
			}

			needsCursorUpdate = true;
		}

		const bool isWaypoint = obj->getObjectType () == SharedWaypointObjectTemplate::SharedWaypointObjectTemplate_tag;
		
		if (isWaypoint)
		{
			const ClientWaypointObject * const waypoint = safe_cast<const ClientWaypointObject *>(obj);
			
			if (waypoint->isWaypointActive ())
			{
				if (!info.lastActive || !info.initialized)
				{
					UIRectangleStyle * const rs = safe_cast<UIRectangleStyle *>(UIManager::gUIManager ().GetObjectFromPath (s_waypointOverlayPath.c_str (), TUIRectangleStyle));
					viewer->setOverlay (rs);
				}
			}
			else if (info.lastActive || !info.initialized)
				viewer->setOverlay (0);

			info.lastActive = waypoint->isWaypointActive ();

			needsCursorUpdate = true;
		}

		
		if(clientObject && clientObject->getGameObjectType() == SharedObjectTemplate::GOT_misc_enzyme)
		{
			if(ClientIncubatorManager::getSlotIndexForNetworkId(clientObject->getNetworkId()) != ClientIncubatorManager::slot_invalid)
			{
				viewer->SetBackgroundTint    (UIColor::red);
				viewer->SetBackgroundOpacity (1.0f);
			}
			else
			{
				viewer->SetBackgroundOpacity (0.0f);
			}
		}
		

		const bool isMission = obj->getObjectType () == SharedMissionObjectTemplate::SharedMissionObjectTemplate_tag;
		
		if (isMission)
		{
			const ClientMissionObject * const mission = safe_cast<const ClientMissionObject *>(obj);

			if(mission)
			{
				const Container* c = ContainerInterface::getContainer(*mission);
				if(!c)
				{
					DEBUG_WARNING(true, ("ClientMissionObject has no container"));
					return;
				}

				for(ContainerConstIterator i = c->begin(); i != c->end(); ++i)
				{
					const Container::ContainedItem item = *i;
					Object const * const o = item.getObject();
					ClientWaypointObject const * const w = dynamic_cast<const ClientWaypointObject*>(o);
					if(w)
					{
						if (w->isWaypointActive ())
						{
							if (!info.lastActive || !info.initialized)
							{
								UIRectangleStyle * const rs = safe_cast<UIRectangleStyle *>(UIManager::gUIManager ().GetObjectFromPath (s_waypointOverlayPath.c_str (), TUIRectangleStyle));
								viewer->setOverlay (rs);
							}
						}
						else if (info.lastActive || !info.initialized)
							viewer->setOverlay (0);

						info.lastActive = w->isWaypointActive ();

						break;
					}
				}
			}
			needsCursorUpdate = true;
		}

		const NetworkId & objId = obj->getNetworkId ();
		static Unicode::String tooltip;
		tooltip.clear ();
		bool tooltipOk = false;
		
		if (objId != NetworkId::cms_invalid)
		{
			if (clientObject)
			{
				if (info.objectProperties.updateAndCompareFromObject (*clientObject))
				{
					//-- force an attrib update
					info.lastAttribUpdate = 0;
					ObjectAttributeManager::requestUpdate(objId);
				}	
			}

			if (ObjectAttributeManager::formatDescriptionIfNewer (objId, header, desc, attribs, info.minimalTooltip, info.lastAttribUpdate))
			{				
				tooltipOk = true;
			}
		}
		//-- it may be a draft schematic
		else if (!obj->isInWorld () && obj->getObjectType () != SharedWaypointObjectTemplate::SharedWaypointObjectTemplate_tag)
		{
			if (clientObject)
			{
				if (DraftSchematicManager::formatDescriptionIfNewer (*clientObject, header, desc, attribs, info.minimalTooltip, info.lastAttribUpdate))
				{
					tooltipOk = true;
				}
			}
		}
	
		if (tooltipOk)
		{
			if (ObjectAttributeManager::hasExtendedTooltipInformation(objId)) 
			{
				ObjectAttributeManager::getExtendedTooltipInformation(objId, tooltip);
			}
			else
			{
				IGNORE_RETURN(tooltip.append(header));
				IGNORE_RETURN(tooltip.append(1, '\n'));
				IGNORE_RETURN(tooltip.append(attribs));
			}

			if (info.callback && clientObject)
				info.callback->overrideTooltip(*clientObject, tooltip);
			
			viewer->SetLocalTooltip(tooltip);
		}
		
		
		if (needsCursorUpdate)
		{
			if (clientObject)
			{
				UICursor * cursor = Cui::MenuInfoTypes::findDefaultCursor (*clientObject);
				
				if (info.callback)
					info.callback->overrideCursor (*clientObject, cursor);
				
				viewer->SetMouseCursor (cursor);
			}
		}
		
		info.initialized = true;
	}
}

//----------------------------------------------------------------------

void CuiIconManager::updateColors ()
{
	const UIPalette * const pal = UIPalette::GetInstance ();
	if (pal)
	{
		IGNORE_RETURN(pal->FindColor (s_palColorTextNormal,  ms_colorTextNormal));
		
		if (!pal->FindColor (s_palColorTextMagic,   ms_colorTextMagic))
			ms_colorTextMagic = UIColor (0xff, 0xff, 0x00);
		
		if (!pal->FindColor (s_palColorTextInsured, s_colorTextInsured))
			s_colorTextInsured = UIColor (0x00, 0xff, 0xff);

		if (!pal->FindColor (s_palColorEquipped, s_colorEquipped))
			s_colorEquipped = UIColor::green;

		if (!pal->FindColor (s_palColorDisabled, s_colorDisabled))
			s_colorDisabled = UIColor::red;

		if (!pal->FindColor (s_palColorAppearance, s_appearanceEquipped))
			s_appearanceEquipped = UIColor (0x80, 0x00, 0xFF);
	}
}

//----------------------------------------------------------------------

UIButtonStyle * CuiIconManager::getFallbackButton ()
{
	UIButtonStyle * fallbackButtonStyle = GetFallbackButton();
	
	if (!fallbackButtonStyle)
	{
		const UIPage * const rootPage = UIManager::gUIManager ().GetRootPage ();
		NOT_NULL (rootPage);

		fallbackButtonStyle = safe_cast<UIButtonStyle *>(rootPage->GetObjectFromPath(GetStylePath(isl_fallback_btn).c_str()));
		if(fallbackButtonStyle)
		{
			if(s_fallbackButton.insert(std::make_pair(getActiveStyleMapIndex(), fallbackButtonStyle)).second)
			{
				fallbackButtonStyle->Attach(0);
			}
		}
	}

	return fallbackButtonStyle;
}

//----------------------------------------------------------------------

std::string CuiIconManager::findButtonPath (const CuiDragInfo & info)
{
	DEBUG_FATAL (!s_installed, ("!installed"));
	switch (info.type)
	{
		//----------------------------------------------------------------------


		//----------------------------------------------------------------------

	case CuiDragInfoTypes::CDIT_object:
		return GetStylePath(isl_object);

	case CuiDragInfoTypes::CDIT_macro:
		return GetStylePath(isl_ui_btn);

	case CuiDragInfoTypes::CDIT_command:
		{
			if (!info.cmd.empty ())
				return GetStylePath(isl_ui_btn);

			if (info.str.empty ())
			{
				WARNING (true, ("Bad str"));
				return GetStylePath(isl_fallback_btn);
			}

			std::string cmdCopy = info.str;
			if (cmdCopy.empty ())
				return GetStylePath(isl_fallback_btn);

			if (cmdCopy [0] == '/')
				IGNORE_RETURN(cmdCopy.erase(0, 1));

			std::string token;

			size_t endpos = 0;
			if (Unicode::getFirstToken (cmdCopy, 0, endpos, token) && !token.empty ())
			{
				//- see if it is a social

				const uint32 socialId = CuiSocialsManager::getSocialIdForCommandName (token);

				if (socialId)
				{
					return GetStylePath(isl_social_btn);
				}

				const std::string & lowerMoodCommand = Unicode::wideToNarrow (Unicode::toLower (MoodManagerClient::getMoodCommand ()));

				//-- see if it is a mood

				if (lowerMoodCommand == Unicode::toLower (token))
				{
					std::string localizedMoodName;
					if (Unicode::getFirstToken (cmdCopy, endpos+1, endpos, localizedMoodName))
					{
						return GetStylePath(isl_social_btn);
					}
				}

				//- ui action, strip off the parameters
				if (!_strnicmp (cmdCopy.c_str (), "ui action ", 10))
				{
					return GetStylePath(isl_ui_btn);
				}

				if (CuiCombatManager::isCombatCommand (token))
				{
					const size_t token_len = token.size ();
					const char c = token [token_len - 1];

					if (c == '3')
						return GetStylePath(isl_combat3);
					else if (c == '2')
						return GetStylePath(isl_combat2);
					else
						return GetStylePath(isl_combat1);

				}
			}
		}
		break;
	}

	return GetStylePath(isl_fallback_btn);
}

//----------------------------------------------------------------------

UIButtonStyle * CuiIconManager::findButtonStyle (const CuiDragInfo & info)
{
	DEBUG_FATAL (!s_installed, ("!installed"));

	{
		ButtonStyleCache const & buttonStyleCache = GetButtonStyleCache();
		const ButtonStyleCache::const_iterator it = buttonStyleCache.find (info);
		
		if (it != buttonStyleCache.end ())
			return (*it).second;
	}

	std::string path = findButtonPath (info);

	const UIPage * const rootPage = UIManager::gUIManager ().GetRootPage ();
	NOT_NULL (rootPage);

	UIButtonStyle * buttonStyle = safe_cast<UIButtonStyle *>(rootPage->GetObjectFromPath (path.c_str ()));
		
	{
		ButtonStyleCache & buttonStyleCache = GetButtonStyleCache();
		if(buttonStyleCache.insert(std::make_pair (info, buttonStyle)).second)
		{
			if (buttonStyle)
				buttonStyle->Attach (0);
		}
	}

	return buttonStyle;
}

//----------------------------------------------------------------------

UIImageStyle * CuiIconManager::findIcon (const std::string& styleName)
{
	DEBUG_FATAL (!s_installed, ("!installed"));

	const UIPage * const rootPage = UIManager::gUIManager ().GetRootPage ();
	NOT_NULL (rootPage);
		

	// RLS TODO - This code needs to be rewritten.
	std::string path = 	GetStylePath(isl_mood) + styleName;
	UIImageStyle * imageStyle = safe_cast<UIImageStyle *>(rootPage->GetObjectFromPath (path.c_str ()));
	if(!imageStyle)
	{
		path       = GetStylePath(isl_social) + styleName;
		imageStyle = safe_cast<UIImageStyle *>(rootPage->GetObjectFromPath (path.c_str ()));
		if(!imageStyle)
		{
			path       = GetStylePath(isl_ui) + styleName;
			imageStyle = safe_cast<UIImageStyle *>(rootPage->GetObjectFromPath (path.c_str ()));
			if(!imageStyle)
			{
				path       = GetStylePath(isl_commands) + styleName;
				imageStyle = safe_cast<UIImageStyle *>(rootPage->GetObjectFromPath (path.c_str ()));
				if(!imageStyle)
				{
					path       = GetStylePath(isl_jedi) + styleName;
					imageStyle = safe_cast<UIImageStyle *>(rootPage->GetObjectFromPath (path.c_str ()));
					if(!imageStyle)
					{
						
						path       = GetStylePath(isl_state) + styleName;
						imageStyle = safe_cast<UIImageStyle *>(rootPage->GetObjectFromPath (path.c_str ()));
						if(!imageStyle)
						{
							path       = GetStylePath(isl_posture) + styleName;
							imageStyle = safe_cast<UIImageStyle *>(rootPage->GetObjectFromPath (path.c_str ()));
							if(!imageStyle)
							{
								path       = GetStylePath(isl_music) + styleName;
								imageStyle = safe_cast<UIImageStyle *>(rootPage->GetObjectFromPath (path.c_str ()));
								if(!imageStyle)
								{
									path       = GetStylePath(isl_faction) + styleName;
									imageStyle = safe_cast<UIImageStyle *>(rootPage->GetObjectFromPath (path.c_str ()));
								}
							}
						}
					}
				}
			}
		}
	}

	return imageStyle;
}

//----------------------------------------------------------------------

void CuiIconManager::setEntangleResistance(bool enabled)
{
	ms_entangleResistance = enabled;
}

//----------------------------------------------------------------------

bool CuiIconManager::getEntangleResistance()
{
	return ms_entangleResistance;
}

//----------------------------------------------------------------------

void CuiIconManager::setColdResist(bool enabled)
{
	ms_coldResist = enabled;
}

//----------------------------------------------------------------------

bool CuiIconManager::getColdResist()
{
	return ms_coldResist;
}

//----------------------------------------------------------------------

void CuiIconManager::setConductivity(bool enabled)
{
	ms_conductivity = enabled;
}

//----------------------------------------------------------------------

bool CuiIconManager::getConductivity()
{
	return ms_conductivity;
}

//----------------------------------------------------------------------

void CuiIconManager::setDecayResist(bool enabled)
{
	ms_decayResist = enabled;
}

//----------------------------------------------------------------------

bool CuiIconManager::getDecayResist()
{
	return ms_decayResist;
}

//----------------------------------------------------------------------

void CuiIconManager::setFlavor(bool enabled)
{
	ms_flavor = enabled;
}

//----------------------------------------------------------------------

bool CuiIconManager::getFlavor()
{
	return ms_flavor;
}

//----------------------------------------------------------------------

void CuiIconManager::setHeatResist(bool enabled)
{
	ms_heatResist = enabled;
}

//----------------------------------------------------------------------

bool CuiIconManager::getHeatResist()
{
	return ms_heatResist;
}

//----------------------------------------------------------------------

void CuiIconManager::setMalleability(bool enabled)
{
	ms_malleability = enabled;
}

//----------------------------------------------------------------------

bool CuiIconManager::getMalleability()
{
	return ms_malleability;
}

//----------------------------------------------------------------------

void CuiIconManager::setPotentialEnergy(bool enabled)
{
	ms_potentialEnergy = enabled;
}

//----------------------------------------------------------------------

bool CuiIconManager::getPotentialEnergy()
{
	return ms_potentialEnergy;
}

//----------------------------------------------------------------------

void CuiIconManager::setOverallQuality(bool enabled)
{
	ms_overallQuality = enabled;
}

//----------------------------------------------------------------------

bool CuiIconManager::getOverallQuality()
{
	return ms_overallQuality;
}

//----------------------------------------------------------------------

void CuiIconManager::setShockResistance(bool enabled)
{
	ms_shockResistance = enabled;
}

//----------------------------------------------------------------------

bool CuiIconManager::getShockResistance()
{
	return ms_shockResistance;
}

//----------------------------------------------------------------------

void CuiIconManager::setToughness(bool enabled)
{
	ms_toughness = enabled;
}

//----------------------------------------------------------------------

bool CuiIconManager::getToughness()
{
	return ms_toughness;
}

//----------------------------------------------------------------------

void CuiIconManager::setActiveStyleMap(int index)
{
	ms_activeStyleMap = index;
}

//----------------------------------------------------------------------

int CuiIconManager::getActiveStyleMapIndex()
{
	return ms_activeStyleMap;
}

//======================================================================
