// ======================================================================
//
// SwgCuiCommandParserScene.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommandParserScene.h"

#include "clientDirectInput/ForceFeedbackEffectTemplate.h"
#include "clientDirectInput/ForceFeedbackEffectTemplateList.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientController.h"
#include "clientGame/ClientEffectManager.h"
#include "clientGame/ClientEventManager.h"
#include "clientGame/ClientSecureTradeManager.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/DebugPortalCamera.h"
#include "clientGame/DynamicDebrisObject.h"
#include "clientGame/FollowCreatureController.h"
#include "clientGame/FormManagerClient.h"
#include "clientGame/FreeCamera.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/GroundScene.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/RemoteCreatureController.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipStation.h"
#include "clientGame/VehicleHoverDynamicsClient.h"
#include "clientGame/WanderCreatureController.h"
#include "clientGame/WorldSnapshot.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/DynamicMeshAppearance.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientSkeletalAnimation/AnimationTrackId.h"
#include "clientSkeletalAnimation/BasicMeshGeneratorTemplate.h"
#include "clientSkeletalAnimation/MeshGenerator.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplate.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimationDebugging.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "clientSkeletalAnimation/StateHierarchyAnimationController.h"
#include "clientSkeletalAnimation/TrackAnimationController.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientTerrain/ClientTerrainSorter.h"
#include "clientTerrain/GroundEnvironment.h"
#include "clientTerrain/WeatherManager.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiDamageManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSocialsParser.h"
#include "clientUserInterface/CuiSpatialChatManager.h"
#include "clientUserInterface/CuiStaticLootItemManager.h"
#include "clientUserInterface/CuiTextManager.h"
#include "pcre/pcre.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedDebug/VTune.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/DebugInfoManager.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Production.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedGame/CollectionsDataTable.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/GameScheduler.h"
#include "sharedGame/LfgDataTable.h"
#include "sharedGame/MoodManager.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/QuestManager.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/SpatialChatManager.h"
#include "sharedMath/Plane.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedNetworkMessages/MessageQueueSpatialChat.h"
#include "sharedNetworkMessages/ShipDamageMessage.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Controller.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/RotationDynamics.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/World.h"
#include "sharedRegex/RegexServices.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedUtility/FileName.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiStructurePlacement.h"
#include "swgSharedUtility/Postures.def"
#include "swgSharedUtility/States.def"
#include "clientGame/ClientPathObject.h"

#include <cstdlib>

// ======================================================================
namespace PlayerCreatureControllerNamespace
{
#if PRODUCTION == 0
	extern bool s_usePlayerServerSpeed;
#endif
}

namespace
{
	const char *const ms_attachCommand                  = "attach";
	const char *const ms_detachAllCommand               = "detachAll";

	const char *const ms_wearCommand                    = "wear";
	const char *const ms_stopWearingAllCommand          = "stopWearingAll";

	const char *const ms_equipCommand                   = "equip";
	const char *const ms_unequipAllCommand              = "unequipAll";

	const char *const ms_loadSkeletalMeshGenerators     = "loadSkeletalMeshGenerators";

	const char *const ms_playAnimationCommand           = "playAnimation";
	const char *const ms_setAnimationFocusObject        = "setAnimationFocusObject";
	const char *const ms_clearAnimationFocusObject      = "clearAnimationFocusObject";

	const char *const cms_scaleAppearance               = "scaleAppearance";
	const char *const cms_toggleAim                     = "toggleAim";

	const std::string ms_animationReferencePath         = "appearance/animation/";

	const std::string ms_slottedContainerPropertyName   = "SlottedContainer";

	const char* const ms_cameraFar                      = "cameraFar";
	const char* const ms_cameraFov                      = "cameraFov";
	const char* const ms_cameraNear                     = "cameraNear";
	const char* const ms_cameraPathClear                = "cameraPathClear";
	const char* const ms_cameraPathLoad                 = "cameraPathLoad";
	const char* const ms_cameraPathSave                 = "cameraPathSave";
	const char* const ms_cameraPathStats                = "cameraPathStats";
	const char* const ms_cameraPathTime                 = "cameraPathTime";
	const char* const ms_damageSetState                 = "damageSetState";
	const char* const ms_debugKeyContext                = "debugKeyContext";
	const char* const ms_debugKeyContextHelp            = "debugKeyContextHelp";
	const char* const ms_objectCreateAt                 = "objectCreateAt";
	const char* const ms_objectSetName                  = "objectSetName";
	const char* const ms_placeStructureCommand          = "placeStructure";
	const char* const ms_popupDebugMenu                 = "popupDebugMenu";
	const char* const ms_purchaseTicketCommand          = "purchaseTicket";
	const char* const ms_renderRunTimeRules             = "renderRunTimeRules";
	const char* const ms_renderOverlappingRunTimeRules  = "renderOverlappingRunTimeRules";
	const char* const ms_showPassable                   = "showPassable";
	const char * const ms_specular                      = "specular";
	const char* const ms_setFrameRateLimit              = "setFrameRateLimit";
	const char* const ms_setPosture                     = "setPosture";
	const char* const ms_stateOn                        = "stateOn";
	const char* const ms_stateOff                       = "stateOff";
	const char* const ms_setCounter                     = "setCounter";
	const char* const ms_setTime                        = "setTime";
	const char* const ms_getTime                        = "getTime";
	const char* const ms_skinningTest                   = "skinningTest";
	const char* const ms_skeletalAppearanceLoadTest     = "skeletalAppearanceLoadTest";
	const char* const ms_snapAllObjectsToTerrainCommand = "snapAllObjectsToTerrain";
	const char* const ms_onOff                          = "onOff";
	const char* const ms_getHeight                      = "getHeight";
	const char* const ms_getHeightForce                 = "getHeightForce";
	const char* const ms_getHeightExpensive             = "getHeightExpensive";
	const char* const ms_worldSnapshotDetailLevelBias   = "worldSnapshotDetailLevelBias";

	const char * const ms_drawNetworkIds                = "drawNetworkIds";
	const char * const ms_drawObjectNames               = "drawObjectNames";
	const char * const ms_drawSelfName                  = "drawSelfName";

	const char * const ms_clientDraftRevoke             = "clientDraftRevoke";
	const char * const ms_clientDraftGrant              = "clientDraftGrant";
	const char * const ms_clientCommandRevoke           = "clientCommandRevoke";
	const char * const ms_clientCommandGrant            = "clientCommandGrant";
	const char * const ms_clientSkillGrant              = "clientSkillGrant";
	const char * const ms_clientExpGrant                = "clientExpGrant";
	const char * const ms_clientSpeak                   = "clientSpeak";
	const char * const ms_clientSocial                  = "clientSocial";
	const char * const ms_clientListContainer           = "clientListContainer";

	const char * const ms_clientDamage                  = "clientDamage";

	const char * const ms_clientSetDifficulty           = "clientSetDifficulty";

	const char * const ms_wind                          = "wind";

	const char * const ms_validateWorld                 = "validateWorld";

	const char * const ms_setConditionOn                = "setConditionOn";
	const char * const ms_setConditionOff               = "setConditionOff";

	const char * const ms_setForcePower                 = "setForcePower";
	const char * const ms_setForcePowerMax              = "setForcePowerMax";

	const char * const ms_setVehicle                    = "setVehicle";
	const char * const ms_mountShip                     = "mountShip";
	const char * const ms_dismountShip                  = "dismountShip";

	const char * const ms_reloadForms                   = "reloadForms";

	const char * const ms_pseudoDamageShip              = "pseudoDamageShip";
	const char * const ms_splitRandom                   = "splitRandom";
	const char * const ms_splitObject                   = "splitObject";

	const char * const ms_playForceFeedbackTemplate     = "playForceFeedbackTemplate";

	const char * const ms_reloadQuests                  = "reloadQuests";
	const char * const ms_reloadTable                   = "reloadTable";

	const char * const ms_clientSetMovementScale        = "clientSetMovementScale";

	const char * const ms_getObjectsInWorldObjectList   = "getObjectsInWorldObjectList";

	const char * const ms_pathCreate                    = "pathCreate";
	const char * const ms_pathAppearance                = "pathAppearance";
	const char * const ms_getObjectInfo                 = "getObjectInfo";
	const char * const ms_findObjectInfo                = "findObjectInfo";
	const char * const ms_setWeather                    = "setWeather";

	const char * const ms_showFlyText                   = "showFlyText";

	const char * const ms_showStaticLootItemData        = "showStaticLootItemData";
	const char * const ms_listStaticLootItemData        = "listStaticLootItemData";

	const char * const ms_listCollectionDataByCategory  = "listCollectionDataByCategory";
	const char * const ms_listCollectionDataBrief       = "listCollectionDataBrief";
	const char * const ms_listCollectionDataBriefLocalized = "listCollectionDataBriefLocalized";
	const char * const ms_listCollectionDataDetailed    = "listCollectionDataDetailed";
	const char * const ms_listCollectionDataDetailedLocalized = "listCollectionDataDetailedLocalized";
	const char * const ms_listCollectionServerFirst     = "listCollectionServerFirst";

	const char * const ms_listLfgData                   = "listLfgData";

	const char * const ms_listGcwRegionsData            = "listGcwRegionsData";
	const char * const ms_listGcwGroupsData             = "listGcwGroupsData";
#if PRODUCTION == 0
	const char * const ms_videoCaptureConfig             = "videoCaptureConfig";
	const char * const ms_videoCaptureStart             = "videoCaptureStart";
	const char * const ms_videoCaptureStop              = "videoCaptureStop";
#endif // PRODUCTION

	void performSceneChange (const void * context)
	{
		NOT_NULL (context);
		const std::pair <std::string, std::string> * const data = reinterpret_cast<const std::pair <std::string, std::string> *>(context);

		GroundScene* gs = dynamic_cast<GroundScene*> (Game::getScene ());
		if (gs != 0)
		{
			if (gs->getPlayer () && gs->getPlayer ()->getAttachedTo () != 0)
			{
				WARNING (true, ("no"));
				return;
			}

			gs->close ();
			delete gs;
			gs = 0;
		}

		const FileName scenefilename(FileName::P_terrain, data->first.c_str (), "trn");
		
		Game::setSinglePlayer (true);
		Game::setScene(true, scenefilename, data->second.c_str (), 0);
	}

	void reloadTerrain(void const * /*const context*/)
	{
		GroundScene * const groundScene = dynamic_cast<GroundScene *>(Game::getScene());
		if (groundScene)
			groundScene->reloadTerrain();
	}

	std::string getCollectionSlotIdAsText(CollectionsDataTable::CollectionInfoSlot const & slot)
	{
		char buffer[128];
		if (slot.endSlotId < 0)
			snprintf(buffer, sizeof(buffer)-1, "[%d]", slot.absoluteBeginSlotId);
		else if (slot.maxSlotValue == 0)
			snprintf(buffer, sizeof(buffer)-1, "[%d-%d (%d,%lu)]", slot.absoluteBeginSlotId, slot.absoluteEndSlotId, (slot.endSlotId - slot.beginSlotId + 1), slot.maxValueForNumBits);
		else
			snprintf(buffer, sizeof(buffer)-1, "[%d-%d (%d,%lu), %lu]", slot.absoluteBeginSlotId, slot.absoluteEndSlotId, (slot.endSlotId - slot.beginSlotId + 1), slot.maxValueForNumBits, slot.maxSlotValue);

		buffer[sizeof(buffer)-1] = '\0';

		return std::string(buffer);
	}

	void getLfgNodeDebugText(LfgDataTable::LfgNode const & lfgNode, std::string & sNodeData, std::string sIndention)
	{
		sNodeData += sIndention;
		sNodeData += lfgNode.name;
		sNodeData += FormattedString<256>().sprintf(" (%d, %d) (%d, %d) (%d, %d) (%s) ", lfgNode.minValueBeginSlotId, lfgNode.minValueEndSlotId, lfgNode.maxValueBeginSlotId, lfgNode.maxValueEndSlotId, lfgNode.minValue, lfgNode.maxValue, (lfgNode.internalAttribute ? "internal" : "external"));
		if (lfgNode.defaultMatchCondition == LfgDataTable::DMCT_NA)
			sNodeData += "(NA)";
		else if (lfgNode.defaultMatchCondition == LfgDataTable::DMCT_Any)
			sNodeData += "(Any)";
		else if (lfgNode.defaultMatchCondition == LfgDataTable::DMCT_AnyOnly)
			sNodeData += "(AnyOnly)";
		else if (lfgNode.defaultMatchCondition == LfgDataTable::DMCT_All)
			sNodeData += "(All)";
		else if (lfgNode.defaultMatchCondition == LfgDataTable::DMCT_AllOnly)
			sNodeData += "(AllOnly)";
		else
			sNodeData += "(***UNKNOWN***)";

		sNodeData += FormattedString<256>().sprintf(" (anyAllGroupingParent=%s)", (lfgNode.anyAllGroupingParent ? lfgNode.anyAllGroupingParent->name.c_str() : "NULL"));

		sNodeData += "\n";

		sIndention += "    ";
		for (std::vector<LfgDataTable::LfgNode const *>::const_iterator iterNode = lfgNode.children.begin(); iterNode != lfgNode.children.end(); ++iterNode)
		{
			getLfgNodeDebugText(**iterNode, sNodeData, sIndention);
		}
	}

	ClientPathObject * s_clientPathObject = NULL;
}
using namespace PlayerCreatureControllerNamespace;
//-----------------------------------------------------------------

static const CommandParser::CmdInfo terrain_cmds[] =
{
	{ "reload",               0, "",            "Unload and reload terrain appearance template" },
	{ "purge",                0, "",            "Delete all terrain chunks and regenerate." },
	{ "lodThreshold",         1, "<threshold>", "floating point threshold" },
	{ "lodHighThreshold",     1, "<threshold>", "floating point threshold" },
	{ ms_renderRunTimeRules, 1, "<0|1>", "toggle flag for rendering run-time rules" },
	{ ms_renderOverlappingRunTimeRules, 1, "<0|1>", "toggle flag for rendering overlapping run-time rules" },
	{ ms_showPassable, 1, "<0|1>", "toggle display of passable/impassable terrain" },
	{ ms_specular, 1, "<0|1>", "toggle specular terrain" },
	
	{ "", 0, "", ""} // this must be last
};

// ======================================================================

class TerrainParser : public CommandParser
{
public:
	TerrainParser () : CommandParser ("terrain", 0, "...", "terrain commands.", 0)
	{
		createDelegateCommands (terrain_cmds);
	}

	virtual bool     performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	// disabled
	TerrainParser (const TerrainParser & rhs);              //lint -esym(754, TerrainParser::TerrainParser) // unreferenced // disabled
	TerrainParser &  operator= (const TerrainParser & rhs); //lint -esym(754, TerrainParser::operator=) // unreferenced // disabled
};	
//-----------------------------------------------------------------


static const CommandParser::CmdInfo trade_cmds[] =
{
	{ "initiate",             1, "<targetId>",  "Initiate trade with remote player." },
	{ "acceptRequest",        0, "",            "Accept trade request from other player." },
	{ "denyRequest",          0, "",            "Deny trade request from other player." },
	{ "addItem",              1, "<itemId>",    "Add item to the trade." },
	{ "removeItem",           1, "<itemId>",    "Remove item from the trade." },
	{ "giveMoney",            1, "<amount>",    "Give X amount of money to the other person." },
	{ "acceptTrade",          0, "",            "Accept trade with other player." },
	{ "unAcceptTrade",        0, "",            "UnAccept trade with other player." },
	{ "verifyTrade",          0, "",            "Verify trade with other player." },
	{ "unVerifyTrade",        0, "",            "UnVerify trade with other player." },
	{ "abortTrade",           0, "",            "Cancel trade" },
	{ "", 0, "", ""} // this must be last
};

// ======================================================================

class TradeParser : public CommandParser
{
public:
	TradeParser () : CommandParser ("trade", 0, "...", "trade commands.", 0)
	{
		createDelegateCommands (trade_cmds);
	}

	virtual bool     performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	// disabled
	TradeParser (const TradeParser & rhs);              //lint -esym(754, TerrainParser::TerrainParser) // unreferenced // disabled
	TradeParser &  operator= (const TradeParser & rhs); //lint -esym(754, TerrainParser::operator=) // unreferenced // disabled
};	
//-----------------------------------------------------------------

static const CommandParser::CmdInfo cmds[] =
{
	{"warpme",                          3, "<x> <y> <z>",                            "Warp avatar to a position"},
	{ms_cameraPathLoad,                 1, "<filename>",                             "Load a camera path" },
	{ms_cameraPathSave,                 1, "<filename>",                             "Save current camera path" },
	{ms_cameraPathClear,                0, "",                                       "Clear current camera path" },
	{ms_cameraPathTime,                 1, "<float>",                                "Set amount of time for camera path (default is 30 seconds)" },
	{ms_cameraPathStats,                0, "",                                       "Show stats about last path played" },
	{ms_setTime,                        1, "<float>",                                "Set normalized time of day (from 0 to 1)"},
	{ms_getTime,                        0, "",                                       "Get the time of day"},
	{ms_wind,                           3, "<x> <y> <z>",                            "Sets the wind which affects only particles"},
#if PRODUCTION == 0
	{"warp",                            4, "<ent> <x> <y> <z>",                      "Warp an entity to a position."},
	{ms_drawNetworkIds,                 1, "<0|1>",                                  "Toggle network IDs." },
	{ms_drawObjectNames,                1, "<0|1>",                                  "Toggle object names." },
	{ms_drawSelfName,                   1, "<0|1>",                                  "Toggle drawing your name. (debugging use)"},
	{ms_getHeight,                      2, "<x> <z>",                                "Get terrain height at a x,z position, return false if chunk doesn't exist"},
	{ms_getHeightForce,                 2, "<x> <z>",                                "Get terrain height at a x,z position, force chunk if chunk doesn't exist"},
	{ms_getHeightExpensive,             2, "<x> <z>",                                "Get terrain height at a x,z position, ensuring by [expensively] generating terrain"},
	{ms_worldSnapshotDetailLevelBias,   1, "<float 0..1>",                           "Set the detail level bias of the world snapshot"},
	{"load",                            1, "<scene> [avatar]",                       "Load a GroundScene."},
	{ms_playAnimationCommand,           1, "<animation filename> [<loop=0|1>] OR @<response file> [<regex>]", "Play specified animation on player." },
	{ms_setAnimationFocusObject,        0, "",                                       "Set animation debugger's focus object to look-at target (if set) or self." },
	{ms_clearAnimationFocusObject,      0, "",                                       "Clear animation debugger's focus object so all objects are considered." },
	{ms_wearCommand,                    1, "<sat | mgn filename>",                   "Wear specified appearance or mesh generator on player."},
	{ms_stopWearingAllCommand,          0, "",                                       "Remove all wearables on player."},
	{ms_attachCommand,                  2, "<transform name> <appearance filename>", "Attach appearance to specified hardpoint/joint name on player."},
	{ms_detachAllCommand,               0, "",                                       "Detach all appearances attached to player."},
	{ms_equipCommand,                   1, "<object template name>",                 "Equip object on player."},
	{ms_unequipAllCommand,              0, "",                                       "Unequip everything on player."},
	{ms_loadSkeletalMeshGenerators,     1, "<preloaded asset file>",                 "Load all MGNs specified in the given preload config file."},
	{"freeCameraSpeedFast",             1, "<float>",                                "speed in meters per second." },
	{"freeCameraSpeedSlow",             1, "<float>",                                "speed in meters per second." },
	{"warpToFreeCamera",                0, "",                                       "Warp player object to free camera position." },
	{"objectCreate",                    4, "<templatename> <x> <z> <heading>",       "Create a clientside tangible object." },
	{ms_objectCreateAt,                 1, "<templatename>",                         "Create a clientside tangible object at the player's position." },
	{ms_objectSetName,                  1, "<name> [id]",                            "Set a clientside object's name." },
	{"appearanceCreate",                1, "<templatename>",                         "Create a clientside object with the specified appearance" },
	{"followCreate",                    1, "<templatename>",                         "Create a clientside creature object template that follows the player" },
	{"wanderCreate",                    1, "<templatename>",                         "Create a clientside creature object template that wanders around" },
	{"objectList",                      0, "",                                       "List clientside tangible objects by NetworkId." },
	{"objectDelete",                    1, "<NetworkId>",                            "Delete a clientside tangible object by NetworkId." },
	{ms_snapAllObjectsToTerrainCommand, 0, "",                                       "Snap all clientside objects to the terrain." },
	{ms_onOff,                          1, "<0|1>",                                  "" },
	{ms_placeStructureCommand,          1, "<filename>",                             "Place a structure" },
	{ms_purchaseTicketCommand,          0, "",                                       "Purchase a ticket" },
	{ms_clientSkillGrant,               1, "<skillname>",                            "" },
	{ms_clientExpGrant,                 2, "<exptype> <amount>",                     "" },
	{ms_clientCommandGrant,             1, "<command>",                              "" },
	{ms_clientCommandRevoke,            1, "<command>",                              "" },
	{ms_clientDraftGrant,               1, "<command>",                              "" },
	{ms_clientDraftRevoke,              1, "<command>",                              "" },
	{ms_clientSpeak,                    3, "<type> <mood> <text>",                   "" },
	{ms_clientSocial,                   1, "<type> <target>",                        "" },
	{ms_clientListContainer,            1, "<oid>",                                  "" },
	{"playEffectAtOrigin",              1, "<effectTemplateName>",                   "Play an effect at the origin"},
	{"playEffectAtPosition",            4, "<effectTemplateName> <x> <y> <z>",       "Play an effect at the point"},
	{"playEffectOn",                    3, "<effectTemplateName> <NetworkId> <hardpoint>","Play an effect on the object"},
	{"playEffectAtHardpointWorldPos",   3, "<effectTemplateName> <NetworkId> <hardpoint>","Play an effect at the hardpoint loc in the world space"},
	{"playEventOn",                     3, "<eventName> <NetworkId> <hardpoint>",    "Play an event on the object"},
	{"playEventAt",                     5, "<sourceType> <destType> <x> <y> <z>",    "Play an event at the point"},
	{cms_scaleAppearance,               1, "<float>",                                "Scale the player appearance by the specified amount, 1=non-scaled" },
	{cms_toggleAim,                     0, "",                                       "Toggle the client-side aim state for the player" },
	{ms_popupDebugMenu,                 3, "<section> <variable> <0|1>",             "Toggle popup debug menu flag" },
	{ms_clientDamage,                   2, "<id> <h> [a] [m]",                       "Give damage to someone"},
	{ms_setFrameRateLimit,                  1, "<float>",                                "Set the maximum frame rate the client will run at"},
	{ms_skinningTest,                   0, "",                                       "Create some characters for testing skinning performance"},
	{ms_skeletalAppearanceLoadTest,     0, "",                                       "Create some characters for testing character system load performance"},
	{ms_debugKeyContext,                2, "<context> <subcontext>",                 "Put the game in a state for debugging with CTRL-V/B"},
	{ms_debugKeyContextHelp,            0, "",                                       "List valid contexts"},
	{ms_cameraFov,                      1, "<float>",                                "Set the camera field of view"},
	{ms_cameraNear,                     1, "<float>",                                "Set the camera near plane"},
	{ms_cameraFar,                      1, "<float>",                                "Set the camera far plane"},
	{ms_setPosture,                     1, "<int>",                                  "Set the target's posture"},
	{ms_stateOn,                        1, "<int>",                                  "Set the bitfield on"},
	{ms_stateOff,                       1, "<int>",                                  "Set the bitfield off"},
	{ms_setCounter,                     1, "<int>",                                  "Set the object's counter"},
	{ms_damageSetState,                 1, "<0|1|2|3>",                              "Set damage state"},
	{ms_clientSetDifficulty,            1, "<difficulty>",                           "Set difficulty of your lookat target"},
	{ms_validateWorld,                  0, "",                                       "Run a sanity check on the world"},
	{ms_setConditionOn,                 1, "<bits>",                                 "Set condition bits"},
	{ms_setConditionOff,                1, "<bits>",                                 "Clear condition bits"},
	{ms_setForcePower,                  1, "<power>",                                "Set force power"},
	{ms_setForcePowerMax,               1, "<power max>",                            "Set force power max"},
	{ms_setVehicle,                     4, "<template> <yaw> <h> <cdf>",             "Set vehicle on player"},
	{ms_mountShip,                      0, "",                                       "Mount player into targeted ship"},
	{ms_dismountShip,                   0, "",                                       "Dismount player from ship"},
	{ms_reloadForms,                    0, "",                                       "Reload the client's GodClient Form Data"},
	{ms_pseudoDamageShip,               0, "",                                       "Create some fake damage from the lookAt target."},
	{ms_splitRandom,                    1, "<num cuts>",                             "split something up randomly"},
	{ms_splitObject,                    1, "<num cuts>",                             "split something up randomly"},
	{ms_playForceFeedbackTemplate,      1, "<filename>",                             "Play a test force feedback file, i.e. \"forcefeedback/sample/rocket1.ffe\""},
	{ms_reloadQuests,                   0, "",                                       "Reload CLIENT quest data"},
	{ms_reloadTable,                    1, "<datatable>",                            "Reload CLIENT datatable, i.e. \"datatables/skill/skills.iff\""},
	{ms_clientSetMovementScale,         1, "<scale>",                                "clientSetMovementScale"},
	{ms_getObjectsInWorldObjectList,    1, "<tangible|tangible_not_targetable|intangible|tangible_flora|inactive>", "getObjectsInWorldObjectList"},	
	{ms_pathCreate,                     1, "<radius>",                               "Create a circular path."},
	{ms_pathAppearance,                 1, "name",                                   "Set path appearance name."},
	{ms_getObjectInfo,                  1, "<networkid> [sectionName]",              "Display information about an object." },
	{ms_findObjectInfo,                 2, "<networkid> [keyword1] [keyword2]...",   "Search object info for keywords, returns section/key->value pairs that match." },
	{ms_setWeather,                     1, "<index>",                                "Set weather type."},
	{ms_showFlyText,                    1, "<text>",                                 "Play red flytext over the player's head."},
	{ms_showStaticLootItemData,         1, "<staic item name>",                      "Get the static item data."},
	{ms_listStaticLootItemData,         0, "",                                       "Get the names of all static items."},
	{"collisionCheck",                  2, "<source NetworkId> <target NetworkId>",  "client side collision test"},
	{"collisionCheckXYZ",               6, "<startX> <startY> <startZ> <endX> <endY> <endZ>",  "client side collision test"},
	{ms_listCollectionDataByCategory,   0, "",                                       "List all possible collections organized by category"},
	{ms_listCollectionDataBrief,        0, "",                                       "List all possible collections (brief, non-localized)"},
	{ms_listCollectionDataBriefLocalized, 0, "",                                     "List all possible collections (brief, localized)"},
	{ms_listCollectionDataDetailed,     0, "",                                       "List all possible collections (detailed, non-localized)"},
	{ms_listCollectionDataDetailedLocalized, 0, "",                                  "List all possible collections (detailed, localized)"},
	{ms_listCollectionServerFirst, 0, "",                                            "Display collections \"server first\" information"},
	{ms_listLfgData,                    0, "",                                       "List all possible LFG search criteria"},
	{ms_listGcwRegionsData,             0, "",                                       "List the GCW score categories region data"},
	{ms_listGcwGroupsData,              0, "",                                       "List the GCW score categories group data"},
	{ms_videoCaptureConfig,             4, "<resolution (1-21)> <max seconds> <quality (1-100)> <filename>", "Configure video capture"},
	{ms_videoCaptureStart,              0, "",                                       "Start video capture"},
	{ms_videoCaptureStop,               0, "",                                       "Stop video capture"},
#endif // PRODUCTION
#ifdef _DEBUG
	{"setMovementPercent",              1, "<value (like 1.5)>",                     "/setSpeed client side only"},
	{"useServerSpeedOverride",          1, "<true|false>",                           "Clamps client speed to the server's speed"},
#endif // _DEBUG
	{"", 0, "", ""} // this must be last
};

//-----------------------------------------------------------------

SwgCuiCommandParserScene::SwgCuiCommandParserScene () :
CommandParser ("scene", 0, "...", "Scene related commands.", 0)
{
	createDelegateCommands (cmds);
	IGNORE_RETURN (addSubCommand (new TerrainParser ()));         //lint !e1524 // new in ctor w no explicit dtor
	IGNORE_RETURN (addSubCommand (new TradeParser ())); //lint !e1524
}

//-----------------------------------------------------------------
// temporary

inline static void sendMessageToServer (const CommandParser::String_t & originalCommand)
{
	ConGenericMessage m(Unicode::wideToNarrow (originalCommand).c_str ());
	GameNetwork::send(m, true);
}

// ----------------------------------------------------------------------

static Object *GetFocusObject()
{
	//-- get the Object instance for the target on which we'll play the animation.
	//   This will be either the active selection, or if none, the player.
	Object *focusObject = CuiCombatManager::getLookAtTarget().getObject();
	if (!focusObject)
	{
		// get the scene
		NetworkScene* networkScene = dynamic_cast<NetworkScene*> (Game::getScene ());
		if (!networkScene)
		{
			DEBUG_WARNING(true, ("no network scene.\n"));
			return NULL;
		}

		// get the player
		focusObject = networkScene->getPlayer();
	}

	return focusObject;
}

// ----------------------------------------------------------------------

static void PlayAnimation (const SwgCuiCommandParserScene::StringVector_t & argv, SwgCuiCommandParserScene::String_t & result)
{
	if (argv.size () < 2)
		return;

	//-- Get the focus object for this command.
	Object *targetObject = GetFocusObject();
	if (!targetObject)
	{
		result += Unicode::narrowToWide ("neither a selected target nor a player object could be retrieved.\n");
		return;
	}

	//-- get the animation controller for the target
	// get the appearance
	SkeletalAppearance2 *const skeletalAppearance = dynamic_cast<SkeletalAppearance2*> (targetObject->getAppearance ());

	if (!skeletalAppearance)
	{
		result += Unicode::narrowToWide ("target object doesn't have a skeletal appearance.\n");
		return;
	}

	// Get the appearance animation controller.
	TransformAnimationController *const baseAnimationController = skeletalAppearance->getAnimationController ();
	TrackAnimationController           *animationController     = dynamic_cast<TrackAnimationController*> (baseAnimationController);

	if (!animationController)
	{
		//-- Try to get track animation controller from state hierarchy animation controller.
		StateHierarchyAnimationController *const shAnimationController = dynamic_cast<StateHierarchyAnimationController*> (baseAnimationController);
		if (shAnimationController)
			animationController = &(shAnimationController->getTrackAnimationController());
	}

	if (!animationController)
	{
		result += Unicode::narrowToWide ("object appearance doesn't have a track-based animation controller.\n");
		return;
	}

	//-- Get the transform name map for the primary skeleton template (.skt) associated with the appearance.
	//   This transform name map is needed to create the animation.
	const TransformNameMap *const skeletonTemplateTransformNameMap = skeletalAppearance->getAnimationResolver().getPrimarySkeletonTransformNameMap();
	if (!skeletonTemplateTransformNameMap)
	{
		result += Unicode::narrowToWide ("appearance doesn't have a transform name map for the primary skeleton.\n");
		return;
	}

	const std::string filename = Unicode::wideToNarrow (argv [1]);
	if (filename.empty ())
		return;

	std::vector<std::string> animationPaths;
	bool                     loopAnimation = false;

	if (filename [0] != '@')
	{
		// just a normal file
		animationPaths.push_back (ms_animationReferencePath + filename);

		// in this form of the command (single filename), the next parameter is (0 = non-looping, 1 = looping)
		if (argv.size() >= 3)
		{
			const std::string loopString = Unicode::wideToNarrow (argv [2]);
			loopAnimation = (strcmp(loopString.c_str(), "0") != 0);
		}
	}
	else
	{
		// caller is using an indirection file that specifies all the animations to play

		//-- open indirection file
		const std::string indirectionFilename (filename, 1);

		FILE *indirectionFile = fopen (indirectionFilename.c_str(), "ra");
		if (!indirectionFile)
		{
			result += Unicode::narrowToWide ("failed to open indirection file.\n");
			return;
		}

		//-- check if a match string was specified
		pcre *regularExpression = 0;

		const bool doRegex = (argv.size() >= 3);
		if (doRegex)
		{
			//-- Compile the regular expression.
			char const *errorString = 0;
			int         errorOffset = 0;

			regularExpression = pcre_compile(Unicode::wideToNarrow(argv [2]).c_str(), 0, &errorString, &errorOffset, NULL);
			if (!regularExpression)
			{
				//-- Error.
				char messageBuffer[1024];
				snprintf(messageBuffer, sizeof(messageBuffer) - 1, "Error in regular expression [%s]: compile error=[%s], offset=[%d].\n", Unicode::wideToNarrow(argv[2]).c_str(), errorString, errorOffset);
				messageBuffer[sizeof(messageBuffer) - 1] = '\0';

				result = Unicode::narrowToWide(messageBuffer);
				return;
			}
		}

		//-- process the file
		char        pathBuffer [2*MAX_PATH];
		std::string pathAsString; 

		int const   maxSupportedCaptureCount = 10;
		int const   subscriptCount           = (maxSupportedCaptureCount + 1) * 3;
		int         captureData[subscriptCount];

		while (!feof (indirectionFile))
		{
			//-- read the line
			char *const readResult = fgets(pathBuffer, sizeof (pathBuffer) - 1, indirectionFile);
			if (!readResult)
				break;

			pathBuffer[sizeof(pathBuffer)-1] = 0;

			//-- remove linefeed
			pathAsString = pathBuffer;

			if (!pathAsString.empty())
			{
				std::string::size_type lastIndex = pathAsString.length () - 1;
				if (pathAsString[lastIndex] == '\n')
					IGNORE_RETURN( pathAsString.erase(lastIndex) );
			}

			//-- Add string to list of playback animations if we match the regex or we're not doing a regex.
			bool addAnimation = true;

			if (doRegex)
			{
				//-- Apply regular expression to the animation filename.
				NOT_NULL(regularExpression);
				int const resultCode = pcre_exec(regularExpression, NULL, pathAsString.c_str(), pathAsString.length(), 0, 0, &captureData[0], sizeof(captureData));
				if (resultCode < 0)
				{
					//-- The animation name did not match.
					addAnimation = false;
					DEBUG_WARNING(resultCode < -1, ("pcre_exec error: return code = (%d).", resultCode));
				}
			}

			if (addAnimation)
				animationPaths.push_back(ms_animationReferencePath + pathAsString);
		}

		IGNORE_RETURN (fclose (indirectionFile));
		indirectionFile = 0;

		if (regularExpression)
		{
			//-- Cleanup regular expression memory allocation.
			RegexServices::freeMemory(regularExpression);
			regularExpression = 0;
		}
	}

	// strategy: for each animation file specified, try to load it and
	//           apply it to the player character.
	int playedAnimationCount = 0;

	const std::vector<std::string>::const_iterator itEnd = animationPaths.end();
	for (std::vector<std::string>::const_iterator it = animationPaths.begin(); it != itEnd; ++it)
	{
		const std::string &animationPath = *it;

		//-- load skeletal animation template
		result += Unicode::narrowToWide ("loading animation file [");
		result += Unicode::narrowToWide (animationPath);
		result += Unicode::narrowToWide ("] ...");

		const SkeletalAnimationTemplate *skeletalAnimationTemplate = 0;

		if (!TreeFile::exists(animationPath.c_str()))
		{
			result += Unicode::narrowToWide ("does not exist.\n");
			continue;
		}
		skeletalAnimationTemplate = SkeletalAnimationTemplateList::fetch(CrcLowerString(animationPath.c_str()));

		if (!skeletalAnimationTemplate)
		{
			result += Unicode::narrowToWide ("failed to load.\n");
			continue;
		}

		result += Unicode::narrowToWide ("success.\n");

		//-- Create the skeletal animation.
		SkeletalAnimation *skeletalAnimation = skeletalAnimationTemplate->fetchSkeletalAnimation(skeletalAppearance->getAnimationEnvironment(), *skeletonTemplateTransformNameMap);
		NOT_NULL(skeletalAnimation);

		skeletalAnimationTemplate->release();
		skeletalAnimationTemplate = 0;

		//-- setup animation playback parameters
		const float                                blendTime = 0.25f;
		const TrackAnimationController::PlayMode   playMode  = TrackAnimationController::PM_queue;
		const TrackAnimationController::BlendMode  blendMode = (blendTime > 0.0f) ? TrackAnimationController::BM_linearBlend : TrackAnimationController::BM_noBlend;

		//-- lookup primary track id
		//   -TRF- this only needs to be done once per execution.  Should go in an install, but there isn't one as of this writing.
		AnimationTrackId  primaryTrackId(AnimationTrackId::cms_invalid);

		const bool lookupResult = TrackAnimationController::getTrackIdByName(ConstCharCrcLowerString("loop"), primaryTrackId);
		if (!lookupResult)
		{
			result += Unicode::narrowToWide ("failed to lookup primary track id.\n");
			return;
		}

		//-- submit animation for playing
		animationController->playAnimation (primaryTrackId, skeletalAnimation, playMode, loopAnimation, blendMode, blendTime, NULL);
		++playedAnimationCount;

		//-- release local animation
		skeletalAnimation->release ();
		skeletalAnimation = 0;
	}

	if (playedAnimationCount > 0)
	{
		//-- shut down the console page
		IGNORE_RETURN (CuiMediatorFactory::deactivate (CuiMediatorTypes::Console));
	}
}

// ----------------------------------------------------------------------

static void SetAnimationFocusObject (const SwgCuiCommandParserScene::StringVector_t & /* argv */, SwgCuiCommandParserScene::String_t & result)
{
	//-- Get the focus object for this command.
	Object *targetObject = GetFocusObject();
	if (!targetObject)
	{
		result += Unicode::narrowToWide ("There is no selected target and the player object is NULL, aborting command.\n");
		return;
	}
	else
	{
		result += Unicode::narrowToWide ("Animation debugger focus object set to object id=[");
		result += Unicode::narrowToWide (targetObject->getNetworkId().getValueString().c_str());
		result += Unicode::narrowToWide ("],obj template=[");
		result += Unicode::narrowToWide (targetObject->getObjectTemplateName() ? targetObject->getObjectTemplateName() : "<NULL>");
		result += Unicode::narrowToWide ("]\n");
	}

	SkeletalAnimationDebugging::setFocusObject(targetObject);
}


// ----------------------------------------------------------------------

static void ScaleAppearance (const SwgCuiCommandParserScene::StringVector_t & argv, SwgCuiCommandParserScene::String_t & result)
{
	if (argv.size () < 2)
		return;

	//-- Get the Object instance for the target on which we'll play the animation.
	//   This will be either the active selection, or if none, the player.

	Object *targetObject = CuiCombatManager::getLookAtTarget().getObject();
	if (!targetObject)
	{
		// get the scene
		NetworkScene* networkScene = dynamic_cast<NetworkScene*> (Game::getScene ());
		if (!networkScene)
		{
			result += Unicode::narrowToWide ("no network scene.\n");
			return;
		}

		// get the player
		targetObject = networkScene->getPlayer();
	}

	if (!targetObject)
	{
		result += Unicode::narrowToWide ("neither a selected target nor a player object could be retrieved.\n");
		return;
	}

	//-- Convert second parameter to scale.
	std::string scaleAsString = Unicode::wideToNarrow (argv [1]);
	const float scale = static_cast<float>(atof (scaleAsString.c_str ()));

	//-- Set the scale.
	targetObject->setScale (Vector (scale, scale, scale));

	//-- Set the scale factor, too.
	// @todo one of these scale settings calls shouldn't be necessary.  For now, single player scale testing needs the scale factor set appropriately.
	CreatureObject *const creatureObject = dynamic_cast<CreatureObject*> (targetObject);
	if (creatureObject)
		creatureObject->setScaleFactor (scale);
}

// ----------------------------------------------------------------------

static void ToggleAim (SwgCuiCommandParserScene::String_t & result)
{
	//-- Get the Object instance for the target on which we'll play the animation.
	//   This will be either the active selection, or if none, the player.

	Object *targetObject = CuiCombatManager::getLookAtTarget().getObject();
	if (!targetObject)
	{
		// get the scene
		NetworkScene* networkScene = dynamic_cast<NetworkScene*> (Game::getScene ());
		if (!networkScene)
		{
			result += Unicode::narrowToWide ("no network scene.\n");
			return;
		}

		// get the player
		targetObject = networkScene->getPlayer();
	}

	if (!targetObject)
	{
		result += Unicode::narrowToWide ("neither a selected target nor a player object could be retrieved.\n");
		return;
	}

	//-- Convert target object to a CreatureObject.
	CreatureObject *const creatureObject = dynamic_cast<CreatureObject*> (targetObject);
	if (!creatureObject)
	{
		result += Unicode::narrowToWide ("target object is not a CreatureObject, can't set state.\n");
		return;
	}

	//-- Toggle the state.
	creatureObject->setState (States::Aiming, !creatureObject->getState (States::Aiming));
}

// ----------------------------------------------------------------------

static void Wear (const SwgCuiCommandParserScene::StringVector_t & argv, SwgCuiCommandParserScene::String_t & result)
{
#if 1
		UNREF (argv);
		result += Unicode::narrowToWide ("command temporarily disabled.\n");
#else
	if (argv.size () < 2)
		return;

	//-- get the appearance for the character
	// get the scene
	NetworkScene* networkScene = dynamic_cast<NetworkScene*> (Game::getScene ());
	if (!networkScene)
	{
		result += Unicode::narrowToWide ("no network scene.\n");
		return;
	}

	// get the player
	Object *const player = networkScene->getPlayer();
	if (!player)
	{
		result += Unicode::narrowToWide ("no player character.\n");
		return;
	}

	// get the player appearance
	SkeletalAppearance2 *const skeletalAppearance = dynamic_cast<SkeletalAppearance2*> (player->getAppearance ());
	if (!skeletalAppearance)
	{
		result += Unicode::narrowToWide ("player object doesn't have a skeletal appearance.\n");
		return;
	}

	//-- create an appearance from the given filename
	// get filename, convert to lowercase
	std::string filename = Unicode::wideToNarrow (argv [1]);
	std::transform(filename.begin(), filename.end(), filename.begin(), tolower);

	std::string::size_type  filenameSize = filename.size();
	if (filenameSize < 4)
	{
		result += Unicode::narrowToWide ("filename doesn't appear to have an extension (must be .sat or .mgn).\n");
		return;
	}

	// check for skeletal appearance template
	SkeletalAppearance2 *wearableAppearance = 0;

	if ((filename[filenameSize-3] == 's') || (filename[filenameSize-2] == 'a') || (filename[filenameSize-1] == 't'))
	{
		// we've got a skeletal appearance template
		Appearance *const newAppearance = AppearanceTemplateList::createAppearance(filename.c_str());
		if (!newAppearance)
		{
			result += Unicode::narrowToWide ("failed to create appearance for given filename\n");
			return;
		}

		// setup the wearableAppearance
		wearableAppearance = dynamic_cast<SkeletalAppearance2*>(newAppearance);
		if (!wearableAppearance)
		{
			result += Unicode::narrowToWide ("appearance template file created a non-skeletal appearance\n");
			delete newAppearance;
			return;
		}
	}
	else if ((filename[filenameSize-3] == 'm') || (filename[filenameSize-2] == 'g') || (filename[filenameSize-1] == 'n'))
	{
		// Fetch the MeshGeneratorTemplate.
		const MeshGeneratorTemplate *const mgTemplate = MeshGeneratorTemplateList::fetch(CrcLowerString(filename.c_str()));

		// Fetch the first LOD's BasicMeshGeneratorTemplate.
		const BasicMeshGeneratorTemplate *const basicMgTemplate = mgTemplate->fetchBasicMeshGeneratorTemplate(0);

		// Create the MeshGenerator.
		const MeshGenerator *const meshGenerator = basicMgTemplate->createMeshGenerator();
		if (!meshGenerator)
		{
			result += Unicode::narrowToWide ("failed to load the mesh generator\n");
			basicMgTemplate->release();
			mgTemplate->release();
			return;
		}

		// Create an appearance from the MeshGenerator instance.
		Appearance *const newAppearance = meshGenerator->createAppearance();
		if (!newAppearance)
		{
			result += Unicode::narrowToWide ("failed to create appearance from mesh generator\n");
			meshGenerator->release();
			basicMgTemplate->release();
			mgTemplate->release();
			return;
		}

		// Setup the wearableAppearance.
		wearableAppearance = dynamic_cast<SkeletalAppearance2*>(newAppearance);

		// Release local references.
		meshGenerator->release();
		basicMgTemplate->release();
		mgTemplate->release();

		if (!wearableAppearance)
		{
			result += Unicode::narrowToWide ("mesh generator did not produce a skeletal-based appearance\n");
			delete newAppearance;
			return;
		}
	}
	else
	{
		result += Unicode::narrowToWide ("appearance filename is not of a supported format for wearing.\n");
		return;
	}

	NOT_NULL(wearableAppearance);

	skeletalAppearance->wear(wearableAppearance);
	skeletalAppearance->rebuildIfDirty();
#endif
}

// ----------------------------------------------------------------------

static void StopWearingAll (SwgCuiCommandParserScene::String_t & result)
{
#if 1
		result += Unicode::narrowToWide ("command temporarily disabled.\n");
#else
	//-- get the appearance for the character
	// get the scene
	NetworkScene* networkScene = dynamic_cast<NetworkScene*> (Game::getScene ());
	if (!networkScene)
	{
		result += Unicode::narrowToWide ("no network scene.\n");
		return;
	}

	// get the player
	Object *const player = networkScene->getPlayer();
	if (!player)
	{
		result += Unicode::narrowToWide ("no player character.\n");
		return;
	}

	// get the player appearance
	SkeletalAppearance2 *const skeletalAppearance = dynamic_cast<SkeletalAppearance2*> (player->getAppearance ());
	if (!skeletalAppearance)
	{
		result += Unicode::narrowToWide ("player object doesn't have a skeletal appearance.\n");
		return;
	}

	// since we (presumably) created the appearances we're wearing, we will also delete them
	std::vector<SkeletalAppearance2*>  deadAppearances;

	const int wearableCount = skeletalAppearance->getWearableCount();
	deadAppearances.reserve(static_cast<size_t>(wearableCount));

	// get the appearances
	{
		for (int i = 0; i < wearableCount; ++i)
			deadAppearances.push_back(skeletalAppearance->getWearableAppearance(i));
	}

	// stop wearing the appearances, delete it
	{
		for (int i = 0; i < wearableCount; ++i)
		{
			SkeletalAppearance2 *deadAppearance = deadAppearances[static_cast<size_t>(i)];

			skeletalAppearance->stopWearing(deadAppearance);
			delete deadAppearance;
		}
	}

	skeletalAppearance->rebuildIfDirty();
#endif
}

// ----------------------------------------------------------------------

static void Attach (const SwgCuiCommandParserScene::StringVector_t & argv, SwgCuiCommandParserScene::String_t & result)
{
	if (argv.size () < 3)
		return;

	//-- Get the focus object for this command.
	Object *player = GetFocusObject();
	if (!player)
	{
		result += Unicode::narrowToWide ("neither a selected target nor a player object could be retrieved.\n");
		return;
	}

	// get the player appearance
	SkeletalAppearance2 *const skeletalAppearance = dynamic_cast<SkeletalAppearance2*> (player->getAppearance ());
	if (!skeletalAppearance)
	{
		result += Unicode::narrowToWide ("selected player object doesn't have a skeletal appearance.\n");
		return;
	}

	//-- ensure attachment point exists
	// get skeleton
	const Skeleton &skeleton = skeletalAppearance->getSkeleton(0);

	TemporaryCrcString  attachmentTransformName (Unicode::wideToNarrow (argv [1]).c_str (), false);
	bool                attachmentPointExists   = false;
	int                 attachmentPointIndex    = -1;

	skeleton.findTransformIndex (attachmentTransformName, &attachmentPointIndex, &attachmentPointExists);
	if (!attachmentPointExists)
	{
		result += Unicode::narrowToWide ("named attachment point does not exist\n");
		return;
	}

	//-- create an object from the given filename
	// get filename, convert to lowercase
	std::string filename = Unicode::wideToNarrow (argv [2]);
	std::transform(filename.begin(), filename.end(), filename.begin(), tolower);

	// create the attached object
	Object *const attachedObject = ObjectTemplateList::createObject (CrcLowerString (filename.c_str ()));
	if (!attachedObject)
	{
		result += Unicode::narrowToWide ("failed to create attachment object for given filename\n");
		return;
	}

	//-- attach appearance to player
	skeletalAppearance->attach (attachedObject, attachmentTransformName);

	//-- Adjust Object's held item state.  (Container transfers trigger this automatically, but we're not doing a container transfer).
	CreatureObject *const creatureObject = dynamic_cast<CreatureObject*>(player);
	if (creatureObject)
		creatureObject->setAppearanceHeldItemState ();
} //lint !e429

// ----------------------------------------------------------------------

static void DetachAll (SwgCuiCommandParserScene::String_t & result)
{
	//-- Get the focus object for this command.
	Object *player = GetFocusObject();
	if (!player)
	{
		result += Unicode::narrowToWide ("neither a selected target nor a player object could be retrieved.\n");
		return;
	}

	// get the player appearance
	SkeletalAppearance2 *const skeletalAppearance = dynamic_cast<SkeletalAppearance2*> (player->getAppearance ());
	if (!skeletalAppearance)
	{
		result += Unicode::narrowToWide ("player object doesn't have a skeletal appearance.\n");
		return;
	}

	// since we (presumably) created the appearances that are attached, we will also delete them
	std::vector<Object*>  deadObjects;

	const int attachmentCount = skeletalAppearance->getAttachedAppearanceCount();
	deadObjects.reserve(static_cast<size_t>(attachmentCount));

	// get the appearances
	{
		for (int i = 0; i < attachmentCount; ++i)
			deadObjects.push_back (const_cast<Object*> (skeletalAppearance->getAttachedObject (i)));
	}

	// detatch the appearances, delete it
	{
		for (int i = 0; i < attachmentCount; ++i)
		{
			Object *const deadObject = deadObjects [static_cast<size_t>(i)];

			skeletalAppearance->detach (deadObject);
			delete deadObject;
		}
	}

	//-- Adjust Object's held item state.  (Container transfers trigger this automatically, but we're not doing a container transfer).
	CreatureObject *const creatureObject = dynamic_cast<CreatureObject*>(player);
	if (creatureObject)
		creatureObject->setAppearanceHeldItemState ();
}

//-----------------------------------------------------------------

static void Equip (const SwgCuiCommandParserScene::StringVector_t & argv, SwgCuiCommandParserScene::String_t & result)
{
	if (argv.size () < 2)
		return;

	// get the player
	ClientObject * const player = Game::getClientPlayer();
	if (!player)
	{
		result += Unicode::narrowToWide ("no player character.\n");
		return;
	}

	//-- get the slotted equipment container

	// get the equipment container
	SlottedContainer *const equipmentContainer = player->getSlottedContainerProperty();
	if (!equipmentContainer)
	{
		result += Unicode::narrowToWide ("player does not have an slotted container for equipment.\n");
		return;
	}

	//-- create the wearable object from the object template

	// get the object template name
	std::string filename = Unicode::wideToNarrow (argv [1]);
	std::transform(filename.begin(), filename.end(), filename.begin(), tolower);

	// create the equipped object
	ClientObject * equippedObject = dynamic_cast<ClientObject*>(ObjectTemplate::createObject(filename.c_str()));
	if (!equippedObject)
	{
		result += Unicode::narrowToWide ("failed to create an object from specified object template\n");
		return;
	}
	else
	{
		// fake the endBaselines() call
		ClientObject *clientObject = safe_cast<ClientObject*> (equippedObject);
		clientObject->endBaselines ();
	}


	//-- get the first valid arrangement for the equipped object
	int validArrangementIndex = -1;
	Container::ContainerErrorCode tmp = Container::CEC_Success;

	if (!equipmentContainer->getFirstUnoccupiedArrangement(*equippedObject, validArrangementIndex, tmp) || (validArrangementIndex < 0))
	{
		result += Unicode::narrowToWide ("no valid arrangements found for given object\n");
		delete equippedObject;
		return;
	}

	if (!ContainerInterface::transferItemToSlottedContainer(*player, *equippedObject, validArrangementIndex))
	{
		delete equippedObject;
		return;
	}
}

//-----------------------------------------------------------------

static void UnequipAll (SwgCuiCommandParserScene::String_t & result)
{
	//-- get the player object

	// get the player
	Object *const player = Game::getPlayer();
	if (!player)
	{
		result += Unicode::narrowToWide ("no player character.\n");
		return;
	}

	//-- get the slotted equipment container

	// get the equipment container
	SlottedContainer *const equipmentContainer = player->getSlottedContainerProperty();
	if (!equipmentContainer)
	{
		result += Unicode::narrowToWide ("player does not have an slotted container for equipment.\n");
		return;
	}
	Container::ContainerErrorCode tmp = Container::CEC_Success;
	//-- remove and delete all objects in the equipment container
	// -TRF- definitely a HACK for now, need a way to iterate over all filled slotted container objects
	for (int i = 1; i <= 5; ++i)
	{
		const SlotId  slotId(i);

		// check if container supports this slot
		if (!equipmentContainer->hasSlot(slotId))
			continue;

		// check if any object occupies this slot
		Container::ContainedItem item = equipmentContainer->getObjectInSlot(slotId, tmp);
		if (item == CachedNetworkId::cms_cachedInvalid)
			continue;

		// get the object
		Object *equippedObject = item.getObject();
		if (!equippedObject)
			continue;

		// detatch item from container
		IGNORE_RETURN (equipmentContainer->remove(*equippedObject, tmp));
//		delete equippedObject; @todo, need to clean this object up somehow, add it to the inactive list on world?
	}
}

// ----------------------------------------------------------------------

static void LoadSkeletalMeshGenerators(const SwgCuiCommandParserScene::StringVector_t & argv, SwgCuiCommandParserScene::String_t &result)
{
	//-- Validate arguments.
	if (argv.size() < 2)
	{
		result = Unicode::narrowToWide("syntax: scene loadSkeletalMeshGenerators <rsp file with mesh generator declarations>\n");
		return;
	}

	//-- Load the response file.
	int loadedFileCount = 0;

	std::string const rspFileName = Unicode::wideToNarrow(argv[1]);
	FILE *file = fopen(rspFileName.c_str(), "r");
	if (!file)
	{
		result = Unicode::narrowToWide("failed to open specified rsp file.\n");
		return;
	}

	char line[512];
	while (!feof(file) && fgets(line, sizeof(line) - 1, file))
	{
		line[sizeof(line) - 1] = '\0';
		char *startOfFilename = strrchr(line, '=');
		if (startOfFilename && *(startOfFilename + 1))
		{
			++startOfFilename;
			size_t const length = strlen(startOfFilename);

			char *endOfFilename = startOfFilename + (length - 1);
			while ((endOfFilename > startOfFilename) && isspace(*endOfFilename))
			{
				*endOfFilename = '\0';
				--endOfFilename;
			}

			MeshGeneratorTemplate const *const mgTemplate  = MeshGeneratorTemplateList::fetch(CrcLowerString(startOfFilename));
			if (mgTemplate)
			{
				MeshGenerator *const mg  = NON_NULL(dynamic_cast<BasicMeshGeneratorTemplate const*>(mgTemplate))->createMeshGenerator();
				if (mg)
				{
					while (!AsynchronousLoader::isIdle()) 
						AsynchronousLoader::processCallbacks();

					if (mg)
						mg->release();
				}
				mgTemplate->release();
			}
			else
				REPORT_LOG(true, ("file [%s] returned a NULL MeshGeneratorTemplate.\n", startOfFilename));

			++loadedFileCount;
		}
	}

	fclose(file);

	char buffer[1024];
	snprintf(buffer, sizeof(buffer) - 1, "Loaded and unloaded [%d] MeshGeneratorTemplate files.\n", loadedFileCount);
	buffer[sizeof(buffer) - 1] = '\0';

	result = Unicode::narrowToWide(buffer);
}

// ----------------------------------------------------------------------

static void SkeletalAppearanceLoadTest(SwgCuiCommandParserScene::String_t &result)
{
#ifdef _DEBUG
	bool  useVtune         = true;
#endif
	bool  trackAllocations = false;
	bool  trackFileOpens   = false;

	bool *const reportLogAllocationsFlag = DebugFlags::findFlag("SharedMemoryManager", "reportLogAllocations");
	if (!reportLogAllocationsFlag)
	{
		result += Unicode::narrowToWide ("Skipping, failed to find DebugFlag for memory allocation.\n");
		return;
	}

	bool *const logTreeFileOpensFlag = DebugFlags::findFlag("SharedFile", "logTreeFileOpens");
	if (!logTreeFileOpensFlag)
	{
		result += Unicode::narrowToWide ("Skipping, failed to find SharedFile for tree file allocations.\n");
		return;
	}

	const char *objectNames[] =
	{
		"object/creature/player/shared_human_male.iff",
		"object/creature/player/shared_human_female.iff",
		"object/creature/player/shared_bothan_female.iff",
		"object/creature/player/shared_bothan_male.iff",
		"object/creature/player/shared_moncal_female.iff",
		"object/creature/player/shared_moncal_male.iff",
		"object/creature/player/shared_rodian_female.iff",
		"object/creature/player/shared_rodian_male.iff",
		"object/creature/player/shared_trandoshan_female.iff",
		"object/creature/player/shared_trandoshan_male.iff",
		"object/creature/player/shared_twilek_female.iff",
		"object/creature/player/shared_twilek_male.iff",
		"object/creature/player/shared_wookiee_female.iff",
		"object/creature/player/shared_wookiee_male.iff",
		"object/creature/player/shared_zabrak_female.iff",
		"object/creature/player/shared_zabrak_male.iff"
		"object/creature/player/shared_ithorian_female.iff",
		"object/creature/player/shared_ithorian_male.iff"
		"object/creature/player/shared_sullustan_female.iff",
		"object/creature/player/shared_sullustan_male.iff"
	};

	const Vector playerPosition = dynamic_cast<NetworkScene*>(Game::getScene ())->getPlayer()->getPosition_w();

	PerformanceTimer timer;
	timer.start();

	int index = 0;
	for (int x = 0; x < 4; ++x)
		for (int z = 0; z < 4; ++z, ++index)
		{
			// Create the object.
			if (trackFileOpens)
				*logTreeFileOpensFlag = true;

			if (trackAllocations)
				*reportLogAllocationsFlag = true;

#ifdef _DEBUG
			if (useVtune)
				VTune::resume();
#endif

			ClientObject * const obj = safe_cast<ClientObject *>(ObjectTemplate::createObject (objectNames[index]));
			
#ifdef _DEBUG
			if (useVtune)
				VTune::pause();
#endif

			if (trackAllocations)
				*reportLogAllocationsFlag = false;

			if (trackFileOpens)
				*logTreeFileOpensFlag = false;

			if (!obj)
			{
				result += Unicode::narrowToWide ("failed to create an object from specified object template\n");
				return;
			}

			// Setup position.			
			Vector v = playerPosition + Vector(static_cast<float>(x) * 2, 0, static_cast<float>(z) * 2);
			const TerrainObject* const terrainObject = TerrainObject::getConstInstance ();
			if (terrainObject)
			{
				float height;
				if (terrainObject->getHeight (v, height))
					v.y = height;
			}

			char buf [128];
			IGNORE_RETURN (_itoa (static_cast<int>(obj->getNetworkId().getValue()), buf, 10));

			obj->setPosition_p (v);
			obj->setObjectName (Unicode::narrowToWide ("Object #") + Unicode::narrowToWide (buf));

			RenderWorld::addObjectNotifications (*obj);

			PortalProperty* const property = obj->getPortalProperty ();
			if (property)
				property->clientSinglePlayerInitializeFirstTimeObject ();

			// Add object to world.
			obj->endBaselines ();
			obj->addToWorld();
		}		

	timer.stop();

	char buffer[128];
	sprintf(buffer, "Load test complete (%1.2f seconds)...\n", timer.getElapsedTime());
	result += Unicode::narrowToWide (buffer);
}

//-----------------------------------------------------------------

bool SwgCuiCommandParserScene::performParsing (const NetworkId & , const StringVector_t & argv, const String_t & originalCommand , String_t & result, const CommandParser * node )
{
	UNREF (result);
	NOT_NULL (node);

	CreatureObject * const player = dynamic_cast<CreatureObject *>(Game::getClientPlayer());

	//-----------------------------------------------------------------
	//-- load must be first, because we abort afterwards if there is no scene.

	if (isCommand( argv [0], "load"))
	{
		static std::pair <std::string, std::string> loadInfo;
		
		GroundScene* gs = dynamic_cast<GroundScene*> (Game::getScene ());
		if (gs && gs->getPlayer () && gs->getPlayer ()->getAttachedTo () != 0)
		{
			result += Unicode::narrowToWide ("you must be outside to load a scene\n");
			return true;
		}

		const std::string sceneName (Unicode::wideToNarrow (argv [1]));
		const std::string avatarName (Unicode::NarrowString ("object/creature/player/shared_") + 
			(argv.size () > 2 ? Unicode::wideToNarrow (argv [2]) : Unicode::NarrowString ("human_male")) + ".iff");		
			
		if (!TreeFile::exists (avatarName.c_str ()))
		{
			result += Unicode::narrowToWide ("The specified avatar does not exist.\n");
			return true;
		}
		
		FileName scenefilename(FileName::P_terrain, sceneName.c_str (), "trn");
		
		if (!TreeFile::exists (scenefilename))
		{
			result += Unicode::narrowToWide ("The specified scene does not exist.\n");
			return true;
		}

		loadInfo.first  = sceneName;
		loadInfo.second = avatarName;

		GameScheduler::addCallback (performSceneChange, &loadInfo, 0.0f);

		result += Unicode::narrowToWide ("Scene load scheduled.");

		return true;
	}

	//----------------------------------------------------------------------

	if (!Game::getScene ())
	{
		result += node->getFullErrorMessage (ERR_NO_SCENE);
		return true;
	}

	NetworkScene * const ns = dynamic_cast <NetworkScene *> (Game::getScene ());

	//-----------------------------------------------------------------
	
	if (isCommand( argv [0], ms_drawNetworkIds))
	{		
		CuiPreferences::setDrawNetworkIds (atoi(Unicode::wideToNarrow (argv [1]).c_str()));
	}		

	//-----------------------------------------------------------------
	
	else if (isCommand( argv [0], ms_drawObjectNames))
	{		
		CuiPreferences::setDrawObjectNames (argv [1][0] != '0');
	}

	//-----------------------------------------------------------------
	
	else if (isCommand( argv [0], ms_drawSelfName))
	{		
		CuiPreferences::setDrawSelfName (argv [1][0] != '0');
	}
	
	//-----------------------------------------------------------------
	else if (isCommand( argv [0], "warpme"))
	{
		const Vector position (
			static_cast<float> (atof (Unicode::wideToNarrow (argv [1]).c_str ())),
			static_cast<float> (atof (Unicode::wideToNarrow (argv [2]).c_str ())),
			static_cast<float> (atof (Unicode::wideToNarrow (argv [3]).c_str ())));

		GroundScene *groundScene = dynamic_cast<GroundScene *>(Game::getScene());
		Object* const player = groundScene ? groundScene->getPlayer () : 0;

		if (player)
		{
			if (Game::getSinglePlayer())
			{
				if (player->getAttachedTo () != 0)
					player->setParentCell (CellProperty::getWorldCellProperty ());

				CellProperty::setPortalTransitionsEnabled (false);
					player->setPosition_p (position);
				CellProperty::setPortalTransitionsEnabled (true);

				CollisionWorld::objectWarped (player);
			}
			else
			{
				Unicode::String params;
				params = argv[1];
				params += ' ';
				params += argv[2];
				params += ' ';
				params += argv[3];
				ClientCommandQueue::enqueueCommand("teleport", NetworkId::cms_invalid, params);
			}
		}

		return true;
	} //lint !e429

	//-----------------------------------------------------------------

	else if (isCommand( argv [0], ms_getHeight))
	{
		if (TerrainObject::getConstInstance ())
		{
			const float x = static_cast<float> (atof (Unicode::wideToNarrow (argv [1]).c_str ()));
			const float z = static_cast<float> (atof (Unicode::wideToNarrow (argv [2]).c_str ()));
			float height = 0.f;

			if (TerrainObject::getConstInstance ()->getHeight (Vector (x, 0.f, z), height))
			{
				char buffer [128];
				sprintf (buffer, "height: %f\n", height);
				result += Unicode::narrowToWide (buffer);
			}
			else
				result += Unicode::narrowToWide ("getHeight failed\n");

			//send to server too
			sendMessageToServer (originalCommand);
			result += getErrorMessage (argv.front (), ERR_SENDING_COMMAND_TO_SERVER);
		}
		else
			result += Unicode::narrowToWide ("no terrain\n");
	}

	//----------------------------------------------------------------------

	else if (isCommand( argv [0], ms_getHeightForce))
	{
		if (TerrainObject::getConstInstance ())
		{
			const float x = static_cast<float> (atof (Unicode::wideToNarrow (argv [1]).c_str ()));
			const float z = static_cast<float> (atof (Unicode::wideToNarrow (argv [2]).c_str ()));
			float height = 0.f;

			if (TerrainObject::getConstInstance ()->getHeightForceChunkCreation (Vector (x, 0.f, z), height))
			{
				char buffer [128];
				sprintf (buffer, "height: %f\n", height);
				result += Unicode::narrowToWide (buffer);
			}
			else
				result += Unicode::narrowToWide ("getHeightForce failed\n");

			//send to server too
			sendMessageToServer (originalCommand);
			result += getErrorMessage (argv.front (), ERR_SENDING_COMMAND_TO_SERVER);
		}
		else
			result += Unicode::narrowToWide ("no terrain\n");
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_getHeightExpensive))
	{
		if (TerrainObject::getConstInstance ())
		{
			const float x = static_cast<float> (atof (Unicode::wideToNarrow (argv [1]).c_str ()));
			const float z = static_cast<float> (atof (Unicode::wideToNarrow (argv [2]).c_str ()));

			Appearance* const appearance = TerrainObject::getInstance () ? TerrainObject::getInstance ()->getAppearance () : 0;
			ProceduralTerrainAppearance* const proceduralTerrainAppearance = dynamic_cast<ProceduralTerrainAppearance*> (appearance);
			if (proceduralTerrainAppearance)
			{
				char buffer [128];
				sprintf (buffer, "forced height: %f\n", proceduralTerrainAppearance->generateHeight_expensive (Vector2d (x, z)));
				result += Unicode::narrowToWide (buffer);
			}
			else
				result += Unicode::narrowToWide ("getHeightExpensive failed\n");
		}
		else
			result += Unicode::narrowToWide ("no terrain\n");
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_worldSnapshotDetailLevelBias))
	{
		const float detailLevelBias = static_cast<float> (atof (Unicode::wideToNarrow (argv [1]).c_str ()));
		WorldSnapshot::setDetailLevelBias (detailLevelBias);
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], ms_clientDamage))
	{
		const CachedNetworkId id (Unicode::wideToNarrow (argv [1]).c_str ());

		CreatureObject * const creature = dynamic_cast<CreatureObject*>(id.getObject ());
		const int h = atoi (Unicode::wideToNarrow (argv [2]).c_str ());

		if (creature && !GameObjectTypes::isTypeOf (creature->getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
		{
			const int a = argv.size () > 3 ? atoi (Unicode::wideToNarrow (argv [3]).c_str ()) : 0;
			const int m = argv.size () > 4 ? atoi (Unicode::wideToNarrow (argv [4]).c_str ()) : 0;

			if (h)
			{
				const Attributes::Value max   = creature->getMaxAttribute (Attributes::Health);
				const Attributes::Value cur   = creature->getAttribute    (Attributes::Health);

				creature->clientResetAttribute (Attributes::Health, cur - h, max);
			}
			if (a)
			{
				const Attributes::Value max   = creature->getMaxAttribute (Attributes::Action);
				const Attributes::Value cur   = creature->getAttribute    (Attributes::Action);

				creature->clientResetAttribute (Attributes::Action, cur - a, max);

			}
			if (m)
			{
				const Attributes::Value max   = creature->getMaxAttribute (Attributes::Mind);
				const Attributes::Value cur   = creature->getAttribute    (Attributes::Mind);

				creature->clientResetAttribute (Attributes::Mind, cur - m, max);

			}
			return true;
		}
		else
		{
			TangibleObject * const tangible = dynamic_cast<TangibleObject*>(id.getObject ());
			if (tangible)
			{
				if (h)
				{
					tangible->clientSetDamageTaken (tangible->getDamageTaken () + h);
				}
				return true;
			}
		}

		result += Unicode::narrowToWide ("No object");
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand( argv [0], ms_playAnimationCommand))
	{
		PlayAnimation (argv, result);
	}

	// ---------------------------------------------------------------------

	else if (isCommand( argv [0], ms_setAnimationFocusObject))
	{
		SetAnimationFocusObject (argv, result);
	}

	// ---------------------------------------------------------------------

	else if (isCommand( argv [0], ms_clearAnimationFocusObject))
	{
		SkeletalAnimationDebugging::setFocusObject(NULL);
		result += Unicode::narrowToWide ("Animation debugging focus object cleared. Animation debug logging enabled for all objects.\n");
	}

	// ---------------------------------------------------------------------

	else if (isCommand( argv [0], cms_scaleAppearance))
	{
		ScaleAppearance (argv, result);
	}

	// ---------------------------------------------------------------------

	else if (isAbbrev( argv [0], cms_toggleAim))
	{
		ToggleAim (result);
	}

	//----------------------------------------------------------------------

	else if (isCommand( argv [0], ms_wearCommand))
	{
		Wear (argv, result);
	}

	//----------------------------------------------------------------------

	else if (isCommand( argv [0], ms_stopWearingAllCommand))
	{
		StopWearingAll (result);
	}

	//----------------------------------------------------------------------

	else if (isCommand( argv [0], ms_attachCommand))
	{
		Attach (argv, result);
	}

	//----------------------------------------------------------------------

	else if (isCommand( argv [0], ms_detachAllCommand))
	{
		DetachAll (result);
	}

	//----------------------------------------------------------------------
	
	else if (isCommand( argv [0], ms_equipCommand))
	{
		Equip (argv, result);
	}

	//----------------------------------------------------------------------

	else if (isCommand( argv [0], ms_unequipAllCommand))
	{
		UnequipAll (result);
	}

	// ---------------------------------------------------------------------

	else if (isCommand( argv [0], ms_loadSkeletalMeshGenerators))
	{
		LoadSkeletalMeshGenerators (argv, result);
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], "freeCameraSpeedFast"))
	{
		const float r = static_cast<float> (atof (Unicode::wideToNarrow (argv [1]).c_str ()));
		ConfigClientGame::setFreeCameraSpeedFast (r);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], "freeCameraSpeedSlow"))
	{
		const float r = static_cast<float> (atof (Unicode::wideToNarrow (argv [1]).c_str ()));
		ConfigClientGame::setFreeCameraSpeedSlow (r);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], "warpToFreeCamera"))
	{							
		if (!ns)
			return true;

		Object * const player = ns->getPlayer();
		
		if (player == 0)
		{
			result += node->getFullErrorMessage (ERR_NO_AVATAR);
			return true;
		}

		Controller * const controller = player->getController ();
		
		if (controller == 0)
		{
			result += node->getFullErrorMessage (ERR_NO_CONTROLLER);
			return true;
		}
		
		controller->appendMessage (static_cast<int> (CM_warpPlayerToFreeCamera), 0.0f, static_cast<unsigned long>(0));
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], "objectCreate"))
	{
		if (!Game::getSinglePlayer ())
			return true;

		// get the object template name
		std::string filename = Unicode::wideToNarrow (argv [1]);
		std::transform(filename.begin (), filename.end (), filename.begin (), tolower);
		
		// create the equipped object
		ClientObject * const object = safe_cast<ClientObject *>(ObjectTemplate::createObject (filename.c_str()));
		if (!object)
		{
			result += Unicode::narrowToWide ("failed to create an object from specified object template\n");

			return true;
		}
		
		Vector v (static_cast<real> (atof (Unicode::wideToNarrow (argv [2]).c_str ())),
				0.0f,
				static_cast<real> (atof (Unicode::wideToNarrow (argv [3]).c_str ())));

		const TerrainObject* const terrainObject = TerrainObject::getConstInstance ();

		if (terrainObject)
		{
			float height;
			if (terrainObject->getHeight (v, height))
				v.y = height;
		}
		char buf [128];
		IGNORE_RETURN (_itoa (static_cast<int>(object->getNetworkId().getValue()), buf, 10));

		object->setPosition_p (v);

		const float heading = static_cast<real> (atof (Unicode::wideToNarrow (argv [4]).c_str ()));
		object->yaw_o (convertDegreesToRadians (heading));

		object->setObjectName (Unicode::narrowToWide ("Object #") + Unicode::narrowToWide (buf));

		RenderWorld::addObjectNotifications (*object);

		PortalProperty* const property = object->getPortalProperty ();
		if (property)
			property->clientSinglePlayerInitializeFirstTimeObject ();

		object->endBaselines ();
		object->addToWorld();

		result += Unicode::narrowToWide ("object ") + Unicode::narrowToWide (object->getNetworkId ().getValueString ().c_str ()) + Unicode::narrowToWide (" added to world...\n");
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_objectCreateAt))
	{
		//-- get the object template name
		std::string filename = Unicode::wideToNarrow (argv [1]);
		std::transform (filename.begin (), filename.end (), filename.begin (), tolower);
		
		//-- get the player
		const Object *const player = Game::getPlayer ();
		if (!player)
		{
			result += Unicode::narrowToWide ("no player character.\n");
			return true;
		}
		
		//-- create the equipped object
		ClientObject* const object = safe_cast<ClientObject*> (ObjectTemplate::createObject (filename.c_str ()));
		if (!object)
		{
			result += Unicode::narrowToWide ("failed to create an object from specified object template\n");
			return true;
		}
		
		//-- create a RemoteCreatureController controller if object is a creature object.
		if (object->asCreatureObject())
			object->setController(new RemoteCreatureController(object->asCreatureObject()));

		object->setParentCell (player->getParentCell ());
		CellProperty::setPortalTransitionsEnabled (false);
		{
			object->setTransform_o2p (player->getTransform_o2p ());

			if (argv.size() > 2)
			{
				const Vector offset (0.f, static_cast<float> (atof (Unicode::wideToNarrow (argv [2]).c_str ())), 0.f);
				object->move_o (offset);
			}
		}
		CellProperty::setPortalTransitionsEnabled (true);
		
		RenderWorld::addObjectNotifications (*object);

		PortalProperty* const property = object->getPortalProperty ();
		if (property)
			property->clientSinglePlayerInitializeFirstTimeObject ();

		object->endBaselines ();
		object->addToWorld ();

		result += Unicode::narrowToWide ("object ") + Unicode::narrowToWide (object->getNetworkId ().getValueString ().c_str ()) + Unicode::narrowToWide (" added to world...\n");

		if (argv.size() > 3)
		{
			std::string const & childFilename = Unicode::toLower(Unicode::wideToNarrow(argv[3]));

			//-- create the child object
			ClientObject* const childObject = safe_cast<ClientObject*> (ObjectTemplate::createObject (childFilename.c_str ()));
			if (!childObject)
			{
				result += Unicode::narrowToWide ("failed to create an child object from specified object template\n");
				return true;
			}

			childObject->endBaselines();
			object->addChildObject_o(childObject);
			childObject->move_o(Vector(0.0f, 3.0f, 0.0f));
		}

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_objectSetName))
	{
		if (!player)
			return true;

		NetworkId id;

		if (argv.size () > 2)
		{
			id = NetworkId (Unicode::wideToNarrow (argv [2]));
		}
		else
			id = player->getLookAtTarget ();

		ClientObject * const obj = safe_cast<ClientObject *>(NetworkIdManager::getObjectById (id));

		if (!obj)
			result += Unicode::narrowToWide ("no object\n");
		else
			obj->setObjectName (argv [1]);
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], "appearanceCreate"))
	{
		if (!player)
			return true;

		if (player->getParentCell () != CellProperty::getWorldCellProperty ())
		{
			result += Unicode::narrowToWide ("this command does not work within interiors\n");

			return true;
		}

		// get the object template name
		std::string filename = Unicode::wideToNarrow (argv [1]);
		std::transform (filename.begin (), filename.end (), filename.begin (), tolower);
		
		// create the equipped object
		if (!TreeFile::exists (filename.c_str ()))
		{
			result += Unicode::narrowToWide ("failed to create an object from specified appearance template\n");

			return true;
		}

		Appearance* const appearance = AppearanceTemplateList::createAppearance (filename.c_str ());
		Object* object = new Object ();
		object->addNotification(ClientWorld::getTangibleNotTargetableNotification());
		object->setAppearance (appearance);
		object->setTransform_o2p (player->getTransform_o2w ());
		RenderWorld::addObjectNotifications (*object);
		object->addToWorld();

		result += Unicode::narrowToWide ("object ") + Unicode::narrowToWide (object->getNetworkId ().getValueString ().c_str ()) + Unicode::narrowToWide (" added to world...\n");
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], "followCreate"))
	{
		if (!player)
			return true;

		std::string filename = Unicode::wideToNarrow (argv [1]);
		std::transform (filename.begin (), filename.end (), filename.begin (), tolower);
		
		Object* const object = ObjectTemplate::createObject (filename.c_str ());
		if (!object || !dynamic_cast<CreatureObject*> (object))
		{
			if (object)
				delete object;

			result += Unicode::narrowToWide ("failed to create a creature object from specified object template\n");

			return true;
		}

		CreatureObject* const creatureObject = safe_cast<CreatureObject*> (object);
		creatureObject->setParentCell (player->getParentCell ());
		CellProperty::setPortalTransitionsEnabled (false);
			creatureObject->setTransform_o2p (player->getTransform_o2p ());
		CellProperty::setPortalTransitionsEnabled (true);
		creatureObject->setController (new FollowCreatureController (creatureObject, safe_cast<const CreatureObject*> (Game::getPlayer ())));
		RenderWorld::addObjectNotifications (*creatureObject);
		CellProperty::addPortalCrossingNotification(*creatureObject);

		PortalProperty* const property = creatureObject->getPortalProperty ();
		if (property)
			property->clientSinglePlayerInitializeFirstTimeObject ();

		creatureObject->endBaselines ();
		creatureObject->addToWorld ();

		result += Unicode::narrowToWide ("object ") + Unicode::narrowToWide (object->getNetworkId ().getValueString ().c_str ()) + Unicode::narrowToWide (" added to world...\n");

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], "wanderCreate"))
	{
		if (!player)
			return true;

		std::string filename = Unicode::wideToNarrow (argv [1]);
		std::transform (filename.begin (), filename.end (), filename.begin (), tolower);
		
		Object* const object = ObjectTemplate::createObject (filename.c_str ());
		if (!object || !dynamic_cast<CreatureObject*> (object))
		{
			if (object)
				delete object;

			result += Unicode::narrowToWide ("failed to create a creature object from specified object template\n");

			return true;
		}

		CreatureObject* const creatureObject = safe_cast<CreatureObject*> (object);
		creatureObject->setParentCell (player->getParentCell ());
		CellProperty::setPortalTransitionsEnabled (false);
			creatureObject->setTransform_o2p (player->getTransform_o2p ());
		CellProperty::setPortalTransitionsEnabled (true);
		creatureObject->setController (new WanderCreatureController (creatureObject));
		RenderWorld::addObjectNotifications (*creatureObject);
		CellProperty::addPortalCrossingNotification(*creatureObject);

		PortalProperty* const property = creatureObject->getPortalProperty ();
		if (property)
			property->clientSinglePlayerInitializeFirstTimeObject ();

		creatureObject->endBaselines ();
		creatureObject->addToWorld ();

		result += Unicode::narrowToWide ("object ") + Unicode::narrowToWide (object->getNetworkId ().getValueString ().c_str ()) + Unicode::narrowToWide (" added to world...\n");

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_cameraPathLoad))
	{
		std::string filename = Unicode::wideToNarrow (argv [1]);
		std::transform (filename.begin (), filename.end (), filename.begin (), tolower);

		GroundScene* const groundScene = dynamic_cast<GroundScene*> (Game::getScene ());
		if (!groundScene)
		{
			result += Unicode::narrowToWide ("not in a ground scene\n");

			return true;
		}

		DebugPortalCamera* const debugPortalCamera = dynamic_cast<DebugPortalCamera*> (groundScene->getCamera (GroundScene::CI_debugPortal));		
		if (!debugPortalCamera)
		{
			result += Unicode::narrowToWide ("no path camera available\n");

			return true;
		}

		if (debugPortalCamera->loadPath (filename.c_str ()))
		{
			result += Unicode::narrowToWide ("path loaded\n");

			char buffer [1000];
			sprintf (buffer, "path time = %i seconds\n", static_cast<int> (debugPortalCamera->getPathTime ()));
			result += Unicode::narrowToWide (buffer);
		}
		else
			result += Unicode::narrowToWide ("file not found\n");

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_cameraPathSave))
	{
		std::string filename = Unicode::wideToNarrow (argv [1]);
		std::transform (filename.begin (), filename.end (), filename.begin (), tolower);

		GroundScene* const groundScene = dynamic_cast<GroundScene*> (Game::getScene ());
		if (!groundScene)
		{
			result += Unicode::narrowToWide ("not in a ground scene\n");

			return true;
		}

		DebugPortalCamera* const debugPortalCamera = dynamic_cast<DebugPortalCamera*> (groundScene->getCamera (GroundScene::CI_debugPortal));		
		if (!debugPortalCamera)
		{
			result += Unicode::narrowToWide ("no path camera available\n");

			return true;
		}

		if (debugPortalCamera->savePath (filename.c_str ()))
			result += Unicode::narrowToWide ("path saved\n");
		else
			result += Unicode::narrowToWide ("could not write file\n");

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_cameraPathClear))
	{
		GroundScene* const groundScene = dynamic_cast<GroundScene*> (Game::getScene ());
		if (!groundScene)
		{
			result += Unicode::narrowToWide ("not in a ground scene\n");

			return true;
		}

		DebugPortalCamera* const debugPortalCamera = dynamic_cast<DebugPortalCamera*> (groundScene->getCamera (GroundScene::CI_debugPortal));		
		if (!debugPortalCamera)
		{
			result += Unicode::narrowToWide ("no path camera available\n");

			return true;
		}

		debugPortalCamera->clearPath ();

		result += Unicode::narrowToWide ("path cleared\n");

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_cameraPathTime))
	{
		const float pathTime = static_cast<float> (atof (Unicode::wideToNarrow (argv [1]).c_str ()));

		GroundScene* const groundScene = dynamic_cast<GroundScene*> (Game::getScene ());
		if (!groundScene)
		{
			result += Unicode::narrowToWide ("not in a ground scene\n");

			return true;
		}

		DebugPortalCamera* const debugPortalCamera = dynamic_cast<DebugPortalCamera*> (groundScene->getCamera (GroundScene::CI_debugPortal));		
		if (!debugPortalCamera)
		{
			result += Unicode::narrowToWide ("no path camera available\n");

			return true;
		}

		char buffer [1000];
		sprintf (buffer, "path time changed from %i to %i..\n", static_cast<int> (debugPortalCamera->getPathTime ()), static_cast<int> (pathTime));
		result += Unicode::narrowToWide (buffer);

		debugPortalCamera->setPathTime (pathTime);

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_cameraPathStats))
	{
		const GroundScene* const groundScene = dynamic_cast<const GroundScene*> (Game::getConstScene ());
		if (!groundScene)
		{
			result += Unicode::narrowToWide ("not in a ground scene\n");

			return true;
		}

		const DebugPortalCamera* const debugPortalCamera = dynamic_cast<const DebugPortalCamera*> (groundScene->getCamera (GroundScene::CI_debugPortal));		
		if (!debugPortalCamera)
		{
			result += Unicode::narrowToWide ("no path camera available\n");

			return true;
		}

		char buffer [1000];
		sprintf (buffer, "minimum frame rate = %1.2f\n", debugPortalCamera->getMinimumFrameRate ());
		result += Unicode::narrowToWide (buffer);
		sprintf (buffer, "maximum frame rate = %1.2f\n", debugPortalCamera->getMaximumFrameRate ());
		result += Unicode::narrowToWide (buffer);
		sprintf (buffer, "average frame rate = %1.2f\n", debugPortalCamera->getAverageFrameRate ());
		result += Unicode::narrowToWide (buffer);

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_setTime))
	{
		const float time = static_cast<float> (atof (Unicode::wideToNarrow (argv [1]).c_str ()));

		if (TerrainObject::getInstance ())
			TerrainObject::getInstance ()->setTime (time, true);

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_getTime))
	{
		if (TerrainObject::getInstance())
		{
			int hour;
			int minute;

			TerrainObject::getInstance()->getTime(hour, minute);

			result += Unicode::narrowToWide(FormattedString<64>().sprintf("Game Time: %02d:%02d (%f)\n",
				hour, minute, TerrainObject::getInstance()->getTime()));
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_setFrameRateLimit))
	{
		const float newLimit = static_cast<float> (atof (Unicode::wideToNarrow (argv [1]).c_str ()));

		Clock::setFrameRateLimit(newLimit);

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_debugKeyContext))
	{
		const int         debugKeyContext = static_cast<int> (atoi (Unicode::wideToNarrow (argv [1]).c_str ()));
		const std::string debugKeySubContext = Unicode::wideToNarrow (argv [2]);

		GroundScene* const groundScene = dynamic_cast<GroundScene*> (Game::getScene ());
		if (!groundScene)
		{
			result += Unicode::narrowToWide ("not in a ground scene\n");

			return true;
		}

		groundScene->setDebugKeyContext (debugKeyContext, debugKeySubContext);

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_debugKeyContextHelp))
	{
#ifdef _DEBUG
		GroundScene* const groundScene = dynamic_cast<GroundScene*> (Game::getScene ());
		if (groundScene)
		{
			std::string debugKeyContextHelp;
			groundScene->getDebugKeyContextHelp (debugKeyContextHelp);

			result += Unicode::narrowToWide (debugKeyContextHelp);
		}
		else
			result += Unicode::narrowToWide ("not in a ground scene\n");
#endif

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_cameraFov))
	{
		const float fov = static_cast<float> (atof (Unicode::wideToNarrow (argv [1]).c_str ()));

		GroundScene* const groundScene = dynamic_cast<GroundScene*> (Game::getScene ());
		if (groundScene)
			groundScene->getCurrentCamera ()->setHorizontalFieldOfView (convertDegreesToRadians (fov));
		else
			result += Unicode::narrowToWide ("not in a ground scene\n");
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_cameraNear))
	{
		const float distance = static_cast<float> (atof (Unicode::wideToNarrow (argv [1]).c_str ()));

		GroundScene* const groundScene = dynamic_cast<GroundScene*> (Game::getScene ());
		if (groundScene)
			groundScene->getCurrentCamera ()->setNearPlane (distance);
		else
			result += Unicode::narrowToWide ("not in a ground scene\n");
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_cameraFar))
	{
		const float distance = static_cast<float> (atof (Unicode::wideToNarrow (argv [1]).c_str ()));

		GroundScene* const groundScene = dynamic_cast<GroundScene*> (Game::getScene ());
		if (groundScene)
			groundScene->getCurrentCamera ()->setFarPlane (distance);
		else
			result += Unicode::narrowToWide ("not in a ground scene\n");
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_damageSetState))
	{
		if (Game::getSinglePlayer ())
		{
			Object* const object = player->getLookAtTarget ().getObject ();
			if (object)
			{
				const float damageLevel = clamp (0.f, static_cast<float> (atof (Unicode::wideToNarrow (argv [1]).c_str ())), 1.f);

				TangibleObject* const tangibleObject = dynamic_cast<TangibleObject*> (object);
				if (tangibleObject)
					tangibleObject->setTestDamageLevel (damageLevel);
			}
			else
				result += Unicode::narrowToWide ("no lookAtTarget\n");
		}
		else
			result += Unicode::narrowToWide ("can only use in single player\n");
	}
	
	//----------------------------------------------------------------------
	
	else if (isCommand (argv [0], ms_clientSetDifficulty))
	{
		Object* const object = player->getLookAtTarget ().getObject ();
		if (object)
		{
			const int16 difficulty = static_cast<int16> (atoi (Unicode::wideToNarrow (argv [1]).c_str ()));
			
			CreatureObject* const creature = dynamic_cast<CreatureObject *>(object);
			if (creature)
				creature->setLevel (difficulty);
		}
	}
	
	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_setPosture))
	{
		if (Game::getSinglePlayer ())
		{
			CreatureObject* const creature = dynamic_cast<CreatureObject *>(player->getLookAtTarget ().getObject ());
			if (creature)
			{
				const int posture = static_cast<int> (atoi (Unicode::wideToNarrow (argv [1]).c_str ()));
				if (posture >= 0 && posture < Postures::NumberOfPostures)
					creature->setVisualPosture (static_cast<Postures::Enumerator> (posture));
				else
					result += Unicode::narrowToWide ("posture index out of range\n");
			}
		}
		else
			result += Unicode::narrowToWide ("can only use in single player\n");
	}

	//----------------------------------------------------------------------
	
	else if (isCommand (argv [0], ms_stateOn))
	{
		CreatureObject* const creature = dynamic_cast<CreatureObject *>(player->getLookAtTarget ().getObject ());
		if (creature)
		{
			const int state = static_cast<int> (atoi (Unicode::wideToNarrow (argv [1]).c_str ()));
			if (state >= 0 && state < States::NumberOfStates)
				creature->setState (static_cast<States::Enumerator>(state), true);
			else
				result += Unicode::narrowToWide ("state index out of range\n");
		}
	}

	//----------------------------------------------------------------------
	
	else if (isCommand (argv [0], ms_stateOff))
	{
		CreatureObject* const creature = dynamic_cast<CreatureObject *>(player->getLookAtTarget ().getObject ());
		if (creature)
		{
			const int state = static_cast<int> (atoi (Unicode::wideToNarrow (argv [1]).c_str ()));
			if (state >= 0 && state < States::NumberOfStates)
				creature->setState (static_cast<States::Enumerator>(state), false);
			else
				result += Unicode::narrowToWide ("state index out of range\n");
		}
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_setCounter))
	{
		if (Game::getSinglePlayer ())
		{
			TangibleObject * tangible = 0;

			if (argv.size () > 2)
				tangible = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById (NetworkId (Unicode::wideToNarrow (argv [2]))));
			else
				tangible = dynamic_cast<TangibleObject *>(player->getLookAtTarget ().getObject ());

			if (tangible)
			{
				const int count = static_cast<int> (atoi (Unicode::wideToNarrow (argv [1]).c_str ()));
				tangible->setCount (count);
			}
		}
		else
			result += Unicode::narrowToWide ("can only use in single player\n");
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_popupDebugMenu))
	{
		const std::string section = Unicode::wideToNarrow (argv [1]);
		const std::string name    = Unicode::wideToNarrow (argv [2]);
		const bool        value   = argv [3][0] != '0';

		bool* const flag = DebugFlags::findFlag (section.c_str (), name.c_str ());
		if (flag)
		{
			*flag = value;
			result += Unicode::narrowToWide ("set\n");
		}
		else
			result += Unicode::narrowToWide ("debug flag not found\n");

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_skinningTest))
	{
		const char *objectNames[] =
		{
			"object/creature/player/shared_bothan_female.iff",
			"object/creature/player/shared_bothan_male.iff",
			"object/creature/player/shared_human_female.iff",
			"object/creature/player/shared_human_male.iff",
			"object/creature/player/shared_moncal_female.iff",
			"object/creature/player/shared_moncal_male.iff",
			"object/creature/player/shared_rodian_female.iff",
			"object/creature/player/shared_rodian_male.iff",
			"object/creature/player/shared_trandoshan_female.iff",
			"object/creature/player/shared_trandoshan_male.iff",
			"object/creature/player/shared_twilek_female.iff",
			"object/creature/player/shared_twilek_male.iff",
			"object/creature/player/shared_wookiee_female.iff",
			"object/creature/player/shared_wookiee_male.iff",
			"object/creature/player/shared_zabrak_female.iff",
			"object/creature/player/shared_zabrak_male.iff"
		};

		const Vector playerPosition = dynamic_cast<NetworkScene*>(Game::getScene ())->getPlayer()->getPosition_w();

		int index = 0;
		for (int x = 0; x < 4; ++x)
			for (int z = 0; z < 4; ++z, ++index)
			{
				// create the equipped object
				ClientObject * const obj = safe_cast<ClientObject *>(ObjectTemplate::createObject (objectNames[index]));
				if (!obj)
				{
					result += Unicode::narrowToWide ("failed to create an object from specified object template\n");
					return true;
				}
				
				Vector v = playerPosition + Vector(static_cast<float>(x) * 2, 0, static_cast<float>(z) * 2);
				const TerrainObject* const terrainObject = TerrainObject::getConstInstance ();
				if (terrainObject)
				{
					float height;
					if (terrainObject->getHeight (v, height))
						v.y = height;
				}

				char buf [128];
				IGNORE_RETURN (_itoa (static_cast<int>(obj->getNetworkId().getValue()), buf, 10));

				obj->setPosition_p (v);
				obj->setObjectName (Unicode::narrowToWide ("Object #") + Unicode::narrowToWide (buf));

				RenderWorld::addObjectNotifications (*obj);

				PortalProperty* const property = obj->getPortalProperty ();
				if (property)
					property->clientSinglePlayerInitializeFirstTimeObject ();

				obj->endBaselines ();
				obj->addToWorld();
			}		

		result += Unicode::narrowToWide ("objects added to world...\n");

		return true;
	}

	// ---------------------------------------------------------------------

	else if (isCommand (argv [0], ms_skeletalAppearanceLoadTest))
	{
		SkeletalAppearanceLoadTest(result);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], "objectList"))
	{
		const int numObjects = ClientWorld::getNumberOfObjects (static_cast<int>(WOL_Tangible));

		for (int i = 0; i < numObjects; ++i)
		{
			ClientObject * const obj = safe_cast<ClientObject *>(ClientWorld::getObject (static_cast<int>(WOL_Tangible), i));

			char buf [128];
			IGNORE_RETURN (_itoa (static_cast<int>(obj->getNetworkId().getValue()), buf, 10));

			IGNORE_RETURN (Unicode::appendStringField (result, buf, 8, Unicode::FA_RIGHT));
			IGNORE_RETURN (result.append (4, ' '));

			result += obj->getLocalizedName ();
			IGNORE_RETURN (result.append (1, '\n'));
		}
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], "objectDelete"))
	{
		NetworkId id = NetworkId(Unicode::wideToNarrow (argv [1]));
		Object * const obj = NetworkIdManager::getObjectById(id);

		if (!obj)
		{
			result += Unicode::narrowToWide ("object id not found.\n");
			return true;
		}

		if (obj == player)
		{
			result += Unicode::narrowToWide ("don't be silly.\n");
			return true;
		}

		delete obj;
		result += Unicode::narrowToWide ("object deleted\n");
		return true;
	}

	//-----------------------------------------------------------------
	
	else if (isCommand (argv [0], ms_snapAllObjectsToTerrainCommand))
	{
		ClientWorld::snapAllObjectsToTerrain ();
		result += Unicode::narrowToWide ("objects snapped to terrain\n");
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_onOff))
	{
		if (Game::getSinglePlayer ())
		{
			Object* const object = player->getLookAtTarget ().getObject ();
			if (object)
			{
				const bool on = argv [1][0] != '0';

				TangibleObject* const tangibleObject = dynamic_cast<TangibleObject*> (object);
				if (tangibleObject)
				{
					if (on)
						tangibleObject->setCondition (TangibleObject::C_onOff);
					else
						tangibleObject->clearCondition (TangibleObject::C_onOff);
				}
			}
			else
				result += Unicode::narrowToWide ("no lookAtTarget\n");
		}
		else
			result += Unicode::narrowToWide ("can only use in single player\n");
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_placeStructureCommand))
	{
		GroundScene* const groundScene = dynamic_cast<GroundScene*> (Game::getScene ());

		if (groundScene)
		{
			std::string sharedObjectTemplateName = Unicode::wideToNarrow (argv [1]);
			std::transform (sharedObjectTemplateName.begin (), sharedObjectTemplateName.end (), sharedObjectTemplateName.begin (), tolower);

			if (TreeFile::exists (sharedObjectTemplateName.c_str ()))
			{
				CuiMediatorFactory::activate (CuiMediatorTypes::StructurePlacement);
				safe_cast<SwgCuiStructurePlacement*> (NON_NULL (CuiMediatorFactory::get (CuiMediatorTypes::StructurePlacement, false)))->setData (NetworkId::cms_invalid, sharedObjectTemplateName.c_str ());

				CuiMediatorFactory::deactivate (CuiMediatorTypes::Console);
			}
			else
				result += Unicode::narrowToWide ("file not found");
		}
	}

	//----------------------------------------------------------------------
	
	else if (isCommand (argv [0], ms_purchaseTicketCommand))
	{
		CuiActionManager::performAction (CuiActions::ticketPurchase, Unicode::emptyString);
	}

	//----------------------------------------------------------------------
	
	else if (isCommand (argv [0], ms_clientCommandGrant))
	{
		if (!player)
			return true;

		const std::string & command = Unicode::wideToNarrow (argv [1]);
		player->clientGrantCommand (command);
		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand (argv [0], ms_clientCommandRevoke))
	{
		if (!player)
			return true;

		const std::string & command = Unicode::wideToNarrow (argv [1]);
		player->clientRevokeCommand (command);
		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand (argv [0], ms_clientDraftGrant))
	{
		if (!player)
			return true;

		const std::string & schematic = Unicode::wideToNarrow (argv [1]);
		std::string::size_type slash = schematic.rfind('/');
		std::string shared_schematic(schematic.substr(0, slash + 1) + "shared_" + schematic.substr(slash + 1));
		player->clientGrantSchematic (Crc::calculate(schematic.c_str()), Crc::calculate(shared_schematic.c_str()));
		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand (argv [0], ms_clientDraftRevoke))
	{
		if (!player)
			return true;

		const std::string & schematic = Unicode::wideToNarrow (argv [1]);
		std::string::size_type slash = schematic.rfind('/');
		std::string shared_schematic(schematic.substr(0, slash + 1) + "shared_" + schematic.substr(slash + 1));
		player->clientRevokeSchematic (Crc::calculate(schematic.c_str()), Crc::calculate(shared_schematic.c_str()));
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_clientSpeak))
	{
		if (!player)
			return true;

		const std::string & chatTypeName = Unicode::wideToNarrow (argv [1]);
		const std::string & moodName     = Unicode::wideToNarrow (argv [2]);

		const uint16 mood     = static_cast<uint16>(MoodManager::getMoodByCanonicalName (moodName));
		const uint16 chatType = static_cast<uint16>(SpatialChatManager::getChatTypeByName (chatTypeName));

		Unicode::String str;
		reconstructString (argv, 3, argv.size (), true, str);

		ProsePackage pp;
		pp.stringId  = StringId ("ui", "test_pp");
		pp.actor.id  = player->getLookAtTarget ();

		pp.other.str = Unicode::narrowToWide ("other...here, ok");
		pp.digitInteger = 666;
		pp.digitFloat   = 0.333f;

		Unicode::String oob;
		OutOfBandPackager::pack (pp, -1, oob);

		const MessageQueueSpatialChat msg (player->getLookAtTarget (), NetworkId::cms_invalid, str, 100, chatType, mood, 0, 0, oob);
		CuiSpatialChatManager::processMessage (msg);

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_clientSocial))
	{
		if (!player)
			return true;

		const Object * const actor = player->getLookAtTarget ().getObject ();

		if (!actor)
			return true;

		Unicode::String targetName;

		if (argv.size () > 2)
			targetName = argv [2];

		const std::string & socialName = Unicode::wideToNarrow (argv [1]);

		CuiSocialsParser::performSocialClient (*actor, targetName, socialName, result);
		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_clientListContainer))
	{
		if (!player)
			return true;

		NetworkId id = NetworkId(Unicode::wideToNarrow (argv [1]));
		const Object * const obj = NetworkIdManager::getObjectById(id);

		if (!obj)
		{
			result += Unicode::narrowToWide ("oid not found.\n");
			return true;
		}

		const Container* const c = ContainerInterface::getContainer(*obj);
		if (!c)
		{
			result += Unicode::narrowToWide ("oid not a Container.\n");
			return true;
		}

		std::string info;
		c->debugPrint(info);

		result += Unicode::narrowToWide (info);
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], "collisionCheck"))
	{
		if (!player)
			return true;

		NetworkId sourceId = NetworkId(Unicode::wideToNarrow (argv [1]));
		NetworkId targetId = NetworkId(Unicode::wideToNarrow (argv [2]));

		const Object * const sourceObj = NetworkIdManager::getObjectById(sourceId);
		const Object * const targetObj = NetworkIdManager::getObjectById(targetId);

		char buffer[1024];
		if (sourceObj && targetObj)
		{
			sprintf (buffer, "collisionCheck source=%s, target=%s found both source and target objects\n", sourceId.getValueString().c_str(), targetId.getValueString().c_str());
			result += Unicode::narrowToWide (buffer);

			const CollisionProperty *sourceCollisionProperty = sourceObj->getCollisionProperty();
			const CellProperty *parentCell = sourceObj->getParentCell();
			bool collided;
			uint16 collisionFlags = 0;
			for (int i = 1; i <= 16; ++i)
			{
				if (i == 1)
					collisionFlags = 1;
				else
					collisionFlags *= 2;

				ClientWorld::CollisionInfoVector collisionInfo;
				collided = ClientWorld::collide(parentCell, sourceObj->getPosition_w(), targetObj->getPosition_w(), CollideParameters::cms_default, collisionInfo, collisionFlags);
				if (collided)
				{
					for (ClientWorld::CollisionInfoVector::const_iterator collisionInfoIter = collisionInfo.begin(); collisionInfoIter != collisionInfo.end(); ++collisionInfoIter)
					{
						Object const * collidedObj = collisionInfoIter->getObject();
						if (collidedObj)
						{
							bool sourceCanCollideWith = (sourceCollisionProperty ? sourceCollisionProperty->canCollideWith(collidedObj->getCollisionProperty()) : false);
							const char *objectTemplateName = collidedObj->getObjectTemplateName();

							sprintf (buffer, "    collided - ClientWorld::CollisionFlags bit %d (%hu) (source object %s collide with %s (%s))\n", i, collisionFlags, (sourceCanCollideWith ? "CAN" : "CANNOT"), collidedObj->getNetworkId().getValueString().c_str(), (objectTemplateName ? objectTemplateName : ""));
						}
						else
						{
							sprintf (buffer, "    collided - ClientWorld::CollisionFlags bit %d (%hu)\n", i, collisionFlags);
						}

						result += Unicode::narrowToWide (buffer);
					}
				}
				else
				{
					sprintf (buffer, "NOT collided - ClientWorld::CollisionFlags bit %d (%hu)\n", i, collisionFlags);
					result += Unicode::narrowToWide (buffer);
				}
			}

			{
				ClientWorld::CollisionInfoVector collisionInfo;
				collided = ClientWorld::collide(parentCell, sourceObj->getPosition_w(), targetObj->getPosition_w(), CollideParameters::cms_default, collisionInfo, ClientWorld::CF_all);
				if (collided)
				{
					for (ClientWorld::CollisionInfoVector::const_iterator collisionInfoIter = collisionInfo.begin(); collisionInfoIter != collisionInfo.end(); ++collisionInfoIter)
					{
						Object const * collidedObj = collisionInfoIter->getObject();
						if (collidedObj)
						{
							bool sourceCanCollideWith = (sourceCollisionProperty ? sourceCollisionProperty->canCollideWith(collidedObj->getCollisionProperty()) : false);
							const char *objectTemplateName = collidedObj->getObjectTemplateName();

							sprintf (buffer, "    collided - ClientWorld::CollisionFlags CF_all (source object %s collide with %s (%s))\n", (sourceCanCollideWith ? "CAN" : "CANNOT"), collidedObj->getNetworkId().getValueString().c_str(), (objectTemplateName ? objectTemplateName : ""));
						}
						else
						{
							sprintf (buffer, "    collided - ClientWorld::CollisionFlags CF_all\n");
						}

						result += Unicode::narrowToWide (buffer);
					}
				}
				else
				{
					sprintf (buffer, "NOT collided - ClientWorld::CollisionFlags CF_all\n");
					result += Unicode::narrowToWide (buffer);
				}
			}

			{
				ClientWorld::CollisionInfoVector collisionInfo;
				collided = ClientWorld::collide(parentCell, sourceObj->getPosition_w(), targetObj->getPosition_w(), CollideParameters::cms_default, collisionInfo, ClientWorld::CF_allExtentsOnly);
				if (collided)
				{
					for (ClientWorld::CollisionInfoVector::const_iterator collisionInfoIter = collisionInfo.begin(); collisionInfoIter != collisionInfo.end(); ++collisionInfoIter)
					{
						Object const * collidedObj = collisionInfoIter->getObject();
						if (collidedObj)
						{
							bool sourceCanCollideWith = (sourceCollisionProperty ? sourceCollisionProperty->canCollideWith(collidedObj->getCollisionProperty()) : false);
							const char *objectTemplateName = collidedObj->getObjectTemplateName();

							sprintf (buffer, "    collided - ClientWorld::CollisionFlags CF_allExtentsOnly (source object %s collide with %s (%s))\n", (sourceCanCollideWith ? "CAN" : "CANNOT"), collidedObj->getNetworkId().getValueString().c_str(), (objectTemplateName ? objectTemplateName : ""));
						}
						else
						{
							sprintf (buffer, "    collided - ClientWorld::CollisionFlags CF_allExtentsOnly\n");
						}

						result += Unicode::narrowToWide (buffer);
					}
				}
				else
				{
					sprintf (buffer, "NOT collided - ClientWorld::CollisionFlags CF_allExtentsOnly\n");
					result += Unicode::narrowToWide (buffer);
				}
			}

			{
				ClientWorld::CollisionInfoVector collisionInfo;
				collided = ClientWorld::collide(parentCell, sourceObj->getPosition_w(), targetObj->getPosition_w(), CollideParameters::cms_default, collisionInfo, ClientWorld::CF_allCamera);
				if (collided)
				{
					for (ClientWorld::CollisionInfoVector::const_iterator collisionInfoIter = collisionInfo.begin(); collisionInfoIter != collisionInfo.end(); ++collisionInfoIter)
					{
						Object const * collidedObj = collisionInfoIter->getObject();
						if (collidedObj)
						{
							bool sourceCanCollideWith = (sourceCollisionProperty ? sourceCollisionProperty->canCollideWith(collidedObj->getCollisionProperty()) : false);
							const char *objectTemplateName = collidedObj->getObjectTemplateName();

							sprintf (buffer, "    collided - ClientWorld::CollisionFlags CF_allCamera (source object %s collide with %s (%s))\n", (sourceCanCollideWith ? "CAN" : "CANNOT"), collidedObj->getNetworkId().getValueString().c_str(), (objectTemplateName ? objectTemplateName : ""));
						}
						else
						{
							sprintf (buffer, "    collided - ClientWorld::CollisionFlags CF_allCamera\n");
						}

						result += Unicode::narrowToWide (buffer);
					}
				}
				else
				{
					sprintf (buffer, "NOT collided - ClientWorld::CollisionFlags CF_allCamera\n");
					result += Unicode::narrowToWide (buffer);
				}
			}

			{
				ClientWorld::CollisionInfoVector collisionInfo;
				collided = ClientWorld::collide(parentCell, sourceObj->getPosition_w(), targetObj->getPosition_w(), CollideParameters::cms_default, collisionInfo, ClientWorld::CF_allSkeletalOnly);
				if (collided)
				{
					for (ClientWorld::CollisionInfoVector::const_iterator collisionInfoIter = collisionInfo.begin(); collisionInfoIter != collisionInfo.end(); ++collisionInfoIter)
					{
						Object const * collidedObj = collisionInfoIter->getObject();
						if (collidedObj)
						{
							bool sourceCanCollideWith = (sourceCollisionProperty ? sourceCollisionProperty->canCollideWith(collidedObj->getCollisionProperty()) : false);
							const char *objectTemplateName = collidedObj->getObjectTemplateName();

							sprintf (buffer, "    collided - ClientWorld::CollisionFlags CF_allSkeletalOnly (source object %s collide with %s (%s))\n", (sourceCanCollideWith ? "CAN" : "CANNOT"), collidedObj->getNetworkId().getValueString().c_str(), (objectTemplateName ? objectTemplateName : ""));
						}
						else
						{
							sprintf (buffer, "    collided - ClientWorld::CollisionFlags CF_allSkeletalOnly\n");
						}

						result += Unicode::narrowToWide (buffer);
					}
				}
				else
				{
					sprintf (buffer, "NOT collided - ClientWorld::CollisionFlags CF_allSkeletalOnly\n");
					result += Unicode::narrowToWide (buffer);
				}
			}
		}
		else
		{
			sprintf (buffer, "collisionCheck source=%s, target=%s couldn't find source and/or target object\n", sourceId.getValueString().c_str(), targetId.getValueString().c_str());
			result += Unicode::narrowToWide (buffer);
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], "collisionCheckXYZ"))
	{
		if (!player)
			return true;

		Vector start, end;
		start.x = static_cast<real>(atof(Unicode::wideToNarrow(argv[1].c_str()).c_str()));
		start.y = static_cast<real>(atof(Unicode::wideToNarrow(argv[2].c_str()).c_str()));
		start.z = static_cast<real>(atof(Unicode::wideToNarrow(argv[3].c_str()).c_str()));
		end.x = static_cast<real>(atof(Unicode::wideToNarrow(argv[4].c_str()).c_str()));
		end.y = static_cast<real>(atof(Unicode::wideToNarrow(argv[5].c_str()).c_str()));
		end.z = static_cast<real>(atof(Unicode::wideToNarrow(argv[6].c_str()).c_str()));

		char buffer[1024];
		sprintf (buffer, "collisionCheckXYZ start=(%.10f,%.10f,%.10f), end=(%.10f,%.10f,%.10f)\n", start.x, start.y, start.z, end.x, end.y, end.z);
		result += Unicode::narrowToWide (buffer);

		const CollisionProperty *playerCollisionProperty = player->getCollisionProperty();
		bool collided;
		uint16 collisionFlags = 0;
		for (int i = 1; i <= 16; ++i)
		{
			if (i == 1)
				collisionFlags = 1;
			else
				collisionFlags *= 2;

			ClientWorld::CollisionInfoVector collisionInfo;
			collided = ClientWorld::collide(CellProperty::getWorldCellProperty(), start, end, CollideParameters::cms_default, collisionInfo, collisionFlags);
			if (collided)
			{
				for (ClientWorld::CollisionInfoVector::const_iterator collisionInfoIter = collisionInfo.begin(); collisionInfoIter != collisionInfo.end(); ++collisionInfoIter)
				{
					Object const * collidedObj = collisionInfoIter->getObject();
					if (collidedObj)
					{
						bool playerCanCollideWith = (playerCollisionProperty ? playerCollisionProperty->canCollideWith(collidedObj->getCollisionProperty()) : false);
						const char *objectTemplateName = collidedObj->getObjectTemplateName();

						sprintf (buffer, "    collided - ClientWorld::CollisionFlags bit %d (%hu) (player object %s collide with %s (%s))\n", i, collisionFlags, (playerCanCollideWith ? "CAN" : "CANNOT"), collidedObj->getNetworkId().getValueString().c_str(), (objectTemplateName ? objectTemplateName : ""));
					}
					else
					{
						sprintf (buffer, "    collided - ClientWorld::CollisionFlags bit %d (%hu)\n", i, collisionFlags);
					}

					result += Unicode::narrowToWide (buffer);
				}
			}
			else
			{
				sprintf (buffer, "NOT collided - ClientWorld::CollisionFlags bit %d (%hu)\n", i, collisionFlags);
				result += Unicode::narrowToWide (buffer);
			}
		}

		{
			ClientWorld::CollisionInfoVector collisionInfo;
			collided = ClientWorld::collide(CellProperty::getWorldCellProperty(), start, end, CollideParameters::cms_default, collisionInfo, ClientWorld::CF_all);
			if (collided)
			{
				for (ClientWorld::CollisionInfoVector::const_iterator collisionInfoIter = collisionInfo.begin(); collisionInfoIter != collisionInfo.end(); ++collisionInfoIter)
				{
					Object const * collidedObj = collisionInfoIter->getObject();
					if (collidedObj)
					{
						bool playerCanCollideWith = (playerCollisionProperty ? playerCollisionProperty->canCollideWith(collidedObj->getCollisionProperty()) : false);
						const char *objectTemplateName = collidedObj->getObjectTemplateName();

						sprintf (buffer, "    collided - ClientWorld::CollisionFlags bit CF_all (player object %s collide with %s (%s))\n", (playerCanCollideWith ? "CAN" : "CANNOT"), collidedObj->getNetworkId().getValueString().c_str(), (objectTemplateName ? objectTemplateName : ""));
					}
					else
					{
						sprintf (buffer, "    collided - ClientWorld::CollisionFlags bit CF_all\n");
					}

					result += Unicode::narrowToWide (buffer);
				}
			}
			else
			{
				sprintf (buffer, "NOT collided - ClientWorld::CollisionFlags bit CF_all\n");
				result += Unicode::narrowToWide (buffer);
			}
		}

		{
			ClientWorld::CollisionInfoVector collisionInfo;
			collided = ClientWorld::collide(CellProperty::getWorldCellProperty(), start, end, CollideParameters::cms_default, collisionInfo, ClientWorld::CF_allExtentsOnly);
			if (collided)
			{
				for (ClientWorld::CollisionInfoVector::const_iterator collisionInfoIter = collisionInfo.begin(); collisionInfoIter != collisionInfo.end(); ++collisionInfoIter)
				{
					Object const * collidedObj = collisionInfoIter->getObject();
					if (collidedObj)
					{
						bool playerCanCollideWith = (playerCollisionProperty ? playerCollisionProperty->canCollideWith(collidedObj->getCollisionProperty()) : false);
						const char *objectTemplateName = collidedObj->getObjectTemplateName();

						sprintf (buffer, "    collided - ClientWorld::CollisionFlags bit CF_allExtentsOnly (player object %s collide with %s (%s))\n", (playerCanCollideWith ? "CAN" : "CANNOT"), collidedObj->getNetworkId().getValueString().c_str(), (objectTemplateName ? objectTemplateName : ""));
					}
					else
					{
						sprintf (buffer, "    collided - ClientWorld::CollisionFlags bit CF_allExtentsOnly\n");
					}

					result += Unicode::narrowToWide (buffer);
				}
			}
			else
			{
				sprintf (buffer, "NOT collided - ClientWorld::CollisionFlags bit CF_allExtentsOnly\n");
				result += Unicode::narrowToWide (buffer);
			}
		}

		{
			ClientWorld::CollisionInfoVector collisionInfo;
			collided = ClientWorld::collide(CellProperty::getWorldCellProperty(), start, end, CollideParameters::cms_default, collisionInfo, ClientWorld::CF_allCamera);
			if (collided)
			{
				for (ClientWorld::CollisionInfoVector::const_iterator collisionInfoIter = collisionInfo.begin(); collisionInfoIter != collisionInfo.end(); ++collisionInfoIter)
				{
					Object const * collidedObj = collisionInfoIter->getObject();
					if (collidedObj)
					{
						bool playerCanCollideWith = (playerCollisionProperty ? playerCollisionProperty->canCollideWith(collidedObj->getCollisionProperty()) : false);
						const char *objectTemplateName = collidedObj->getObjectTemplateName();

						sprintf (buffer, "    collided - ClientWorld::CollisionFlags bit CF_allCamera (player object %s collide with %s (%s))\n", (playerCanCollideWith ? "CAN" : "CANNOT"), collidedObj->getNetworkId().getValueString().c_str(), (objectTemplateName ? objectTemplateName : ""));
					}
					else
					{
						sprintf (buffer, "    collided - ClientWorld::CollisionFlags bit CF_allCamera\n");
					}

					result += Unicode::narrowToWide (buffer);
				}
			}
			else
			{
				sprintf (buffer, "NOT collided - ClientWorld::CollisionFlags bit CF_allCamera\n");
				result += Unicode::narrowToWide (buffer);
			}
		}

		{
			ClientWorld::CollisionInfoVector collisionInfo;
			collided = ClientWorld::collide(CellProperty::getWorldCellProperty(), start, end, CollideParameters::cms_default, collisionInfo, ClientWorld::CF_allSkeletalOnly);
			if (collided)
			{
				for (ClientWorld::CollisionInfoVector::const_iterator collisionInfoIter = collisionInfo.begin(); collisionInfoIter != collisionInfo.end(); ++collisionInfoIter)
				{
					Object const * collidedObj = collisionInfoIter->getObject();
					if (collidedObj)
					{
						bool playerCanCollideWith = (playerCollisionProperty ? playerCollisionProperty->canCollideWith(collidedObj->getCollisionProperty()) : false);
						const char *objectTemplateName = collidedObj->getObjectTemplateName();

						sprintf (buffer, "    collided - ClientWorld::CollisionFlags bit CF_allSkeletalOnly (player object %s collide with %s (%s))\n", (playerCanCollideWith ? "CAN" : "CANNOT"), collidedObj->getNetworkId().getValueString().c_str(), (objectTemplateName ? objectTemplateName : ""));
					}
					else
					{
						sprintf (buffer, "    collided - ClientWorld::CollisionFlags bit CF_allSkeletalOnly\n");
					}

					result += Unicode::narrowToWide (buffer);
				}
			}
			else
			{
				sprintf (buffer, "NOT collided - ClientWorld::CollisionFlags bit CF_allSkeletalOnly\n");
				result += Unicode::narrowToWide (buffer);
			}
		}
	}

	//-----------------------------------------------------------------
#ifdef _DEBUG
	else if (isCommand (argv [0], "setMovementPercent"))
	{
		if (!player)
			return true;

		player->setMovementPercent(static_cast<float>(atof(Unicode::wideToNarrow(argv[1].c_str()).c_str())));
		return true;
	}
	else if (isCommand (argv [0], "useServerSpeedOverride"))
	{
#if PRODUCTION == 0
		s_usePlayerServerSpeed = (argv[1] == Unicode::narrowToWide("true"));
#endif
		return true;
	}
#endif
	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_clientSkillGrant))
	{
		if (!player)
			return true;

		const std::string skillname = Unicode::wideToNarrow (argv [1]);
		const SkillObject * const skill = SkillManager::getInstance ().getSkill (skillname);
		if (!skill)
			result += Unicode::narrowToWide ("No such skill");
		else if (!player->clientGrantSkill (*skill))
			result += Unicode::narrowToWide ("Could not grant");
		else
			result += Unicode::narrowToWide ("Skill granted");

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_clientExpGrant))
	{
		if (!player)
			return true;

		const std::string exptype = Unicode::wideToNarrow (argv [1]);
		const int amount          = atoi (Unicode::wideToNarrow (argv [2]).c_str ());
		player->clientGrantExp (exptype, amount);
		result += Unicode::narrowToWide ("Exp granted");
		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], "playEffectAtOrigin"))
	{
		const CrcLowerString effectName(Unicode::wideToNarrow(argv [1]).c_str());
		ClientEffectManager::playClientEffect(effectName, CellProperty::getWorldCellProperty(), Vector(0, 0, 0), Vector::unitY);
		result += Unicode::narrowToWide ("Effect played");
		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], "playEffectAtPosition"))
	{
		const CrcLowerString effectName(Unicode::wideToNarrow(argv [1]).c_str());
		Vector v (static_cast<real> (atof (Unicode::wideToNarrow (argv [2]).c_str ())),
				  static_cast<real> (atof (Unicode::wideToNarrow (argv [3]).c_str ())),
				  static_cast<real> (atof (Unicode::wideToNarrow (argv [4]).c_str ())));
		ClientEffectManager::playClientEffect(effectName, CellProperty::getWorldCellProperty(), v, Vector::unitY);
		result += Unicode::narrowToWide ("Effect played");
		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], "playEffectOn"))
	{
		const CrcLowerString effectName(Unicode::wideToNarrow(argv [1]).c_str());
		NetworkId id = NetworkId(Unicode::wideToNarrow(argv [2]));
		Object * const obj = NetworkIdManager::getObjectById(id);
		const CrcLowerString hardpoint(Unicode::wideToNarrow(argv [3]).c_str());

		if (!obj)
		{
			result += Unicode::narrowToWide ("object id not found.\n");
			return true;
		}
		ClientEffectManager::playClientEffect(effectName, obj, hardpoint);
		result += Unicode::narrowToWide ("Effect played\n");
		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], "playEffectAtHardpointWorldPos"))
	{
		const CrcLowerString effectName(Unicode::wideToNarrow(argv [1]).c_str());
		NetworkId id (Unicode::wideToNarrow(argv [2]));
		Object * const obj = NetworkIdManager::getObjectById(id);
		if (!obj)
		{
			result += Unicode::narrowToWide ("object id not found.\n");
			return true;
		}
		const CrcLowerString hardpoint(Unicode::wideToNarrow(argv [3]).c_str());
		
		//-- Get the appearance.
		const Appearance *const appearance = obj->getAppearance();
		if (!appearance)
		{
			DEBUG_WARNING(true, ("object has no skeleton"));
			return true;
		}

		//-- Get the transform for the hardpoint.
		Transform  hardpointTransform(Transform::IF_none);

		const bool hardpointExists = appearance->findHardpoint(hardpoint, hardpointTransform);
		if (!hardpointExists)
		{
			return true;
		}

		//-- Convert hardpointToObject into hardpointToWorld.
		Vector position = obj->rotateTranslate_o2w(hardpointTransform.getPosition_p());

		ClientEffectManager::playClientEffect(effectName, obj->getParentCell(), position, Vector::unitY);
		result += Unicode::narrowToWide ("Effect played\n");
		return true;
	}


	//-----------------------------------------------------------------

	else if (isCommand (argv [0], "playEventOn"))
	{
		const CrcLowerString eventName(Unicode::wideToNarrow(argv [1]).c_str());
		NetworkId id = NetworkId(Unicode::wideToNarrow (argv [2]));
		Object * const obj = NetworkIdManager::getObjectById(id);
		const CrcLowerString hardpoint(Unicode::wideToNarrow(argv [3]).c_str());

		if (!obj)
		{
			result += Unicode::narrowToWide ("object id not found.\n");
			return true;
		}
		ClientEventManager::playEvent(eventName, obj, hardpoint);
		result += Unicode::narrowToWide ("Event played\n");
		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], "playEventAt"))
	{
		const CrcLowerString source(Unicode::wideToNarrow(argv [1]).c_str());
		const CrcLowerString dest  (Unicode::wideToNarrow(argv [2]).c_str());
		Vector v (static_cast<real> (atof (Unicode::wideToNarrow (argv [3]).c_str ())),
				  static_cast<real> (atof (Unicode::wideToNarrow (argv [4]).c_str ())),
				  static_cast<real> (atof (Unicode::wideToNarrow (argv [5]).c_str ())));

		ClientEventManager::playEvent(source, dest, CellProperty::getWorldCellProperty(), v, Vector::unitY);
		result += Unicode::narrowToWide ("Event played\n");
		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_wind))
	{
		Vector v (static_cast<real> (atof (Unicode::wideToNarrow (argv [1]).c_str ())),
				  static_cast<real> (atof (Unicode::wideToNarrow (argv [2]).c_str ())),
				  static_cast<real> (atof (Unicode::wideToNarrow (argv [3]).c_str ())));

		WeatherManager::setNormalizedWindVelocity_w (v);

		result += Unicode::narrowToWide ("Wind adjusted\n");
		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_validateWorld))
	{
		World::validate();
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_setConditionOn))
	{
		TangibleObject * const target = dynamic_cast<TangibleObject *>(player->getLookAtTarget ().getObject ());
		if (!target)
			return true;

		const int bits = atoi (Unicode::wideToNarrow (argv [1]).c_str ());
		target->setCondition (bits);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_setConditionOff))
	{
		TangibleObject * const target = dynamic_cast<TangibleObject *>(player->getLookAtTarget ().getObject ());
		if (!target)
			return true;

		const int bits = atoi (Unicode::wideToNarrow (argv [1]).c_str ());
		target->clearCondition (bits);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_setForcePower))
	{
		CreatureObject * const target = dynamic_cast<CreatureObject *>(player->getLookAtTarget ().getObject ());
		if (!target)
			return true;

		PlayerObject * const player = target->getPlayerObject ();
		if (!player)
			return true;

		const int val = atoi (Unicode::wideToNarrow (argv [1]).c_str ());
		player->clientSetForcePower (val);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_setForcePowerMax))
	{
		CreatureObject * const target = dynamic_cast<CreatureObject *>(player->getLookAtTarget ().getObject ());
		if (!target)
			return true;

		PlayerObject * const player = target->getPlayerObject ();
		if (!player)
			return true;

		const int val = atoi (Unicode::wideToNarrow (argv [1]).c_str ());
		player->clientSetMaxForcePower (val);
		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_setVehicle))
	{
		CreatureObject * const player = Game::getPlayerCreature ();
		if (!player)
			return true;

		const std::string & appearanceName = Unicode::wideToNarrow (argv [1]);
		const float yaw     = convertDegreesToRadians (static_cast<float>(atof (Unicode::wideToNarrow (argv [2]).c_str ())));
		const float h       = static_cast<float>(atof (Unicode::wideToNarrow (argv [3]).c_str ()));
		std::string cdfName = Unicode::wideToNarrow (argv [4]);
		if (!cdfName.empty () && cdfName [0] == '.')
			cdfName.clear ();		

		const int numChildren = player->getNumberOfChildObjects ();

		{
			for (int i = 0; i < numChildren; ++i)
				player->getChildObject (i)->kill ();
			
			Appearance * const app = AppearanceTemplateList::createAppearance (appearanceName.c_str ());
			if (app)
			{
				Object * const vehicle = new Object ();
				vehicle->setAppearance (app);
				RenderWorld::addObjectNotifications(*vehicle);
				player->addChildObject_o (vehicle);
				vehicle->setDynamics (new VehicleHoverDynamicsClient (vehicle, yaw, h, cdfName.c_str ()));
			}			
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_mountShip))
	{

		CreatureObject * const player = Game::getPlayerCreature ();
		if (!player)
		{
			result += Unicode::narrowToWide ("no player.\n");
			return true;
		}

		if (player->getShipStation() == ShipStation::ShipStation_Pilot)
		{
			result += Unicode::narrowToWide ("you are already piloting a ship.\n");
			return true;
		}

		if (!Game::getSinglePlayer ())
		{
			result += Unicode::narrowToWide ("this only works in single player.\n");
			return true;
		}

		const CachedNetworkId & target = player->getLookAtTarget();
		if (target == NetworkId::cms_invalid)
		{
			result += Unicode::narrowToWide ("no lookat target.\n");
			return true;
		}

		Object* o = NetworkIdManager::getObjectById(target);
		ShipObject* ship = dynamic_cast<ShipObject*>(o);
		if (!ship)
		{
			result += Unicode::narrowToWide ("can't get the ship.\n");
			return true;
		}

		ClientController *controller = dynamic_cast<ClientController *>(ship->getController());
		if (!controller)
		{
			ship->createDefaultController();
			controller = dynamic_cast<ClientController *>(ship->getController());
			if (controller)
			{
				controller->endBaselines();
			}
			ship->scheduleForAlter();
		}

		SlottedContainer *const shipContainer = ship->getSlottedContainerProperty();
		if (!shipContainer)
		{
			PortalProperty * const portalProperty = ship->getPortalProperty();
			if (portalProperty)
			{
				// It's a pob ship, so stick them in a cell of it
				ContainerIterator i = portalProperty->begin();
				if (i != portalProperty->end())
				{
					ClientObject * const cell = dynamic_cast<ClientObject *>((*i).getObject());
					if (cell)
						ContainerInterface::transferItemToCell(*cell, *player, Transform::identity);
				}
				return true;
			}

			result += Unicode::narrowToWide ("ship does not have an slotted container for play.\n");
			return true;
		}

		int validArrangementIndex = -1;
		Container::ContainerErrorCode tmp = Container::CEC_Success;

		if (!shipContainer->getFirstUnoccupiedArrangement(*player, validArrangementIndex, tmp) || (validArrangementIndex < 0))
		{
			result += Unicode::narrowToWide ("no valid arrangements found for player\n");
			return true;
		}

		player->setState(States::PilotingShip, true);

		ContainerInterface::transferItemToSlottedContainer(*ship, *player, validArrangementIndex);
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_dismountShip))
	{
		CreatureObject * const player = Game::getPlayerCreature ();
		if (!player)
		{
			result += Unicode::narrowToWide ("no player.\n");
			return true;
		}

		if (!Game::getSinglePlayer ())
		{
			result += Unicode::narrowToWide ("this only works in single player.\n");
			return true;
		}

		if (player->getShipStation() != ShipStation::ShipStation_Pilot)
		{
			result += Unicode::narrowToWide ("not piloting a ship.\n");
			return true;
		}

		ShipObject* ship = player->getPilotedShip();
		if (!ship)
		{
			result += Unicode::narrowToWide ("couldn't get ship.\n");
			return true;
		}

		SlottedContainer *const shipContainer = ship->getSlottedContainerProperty();
		if (!shipContainer)
		{
			result += Unicode::narrowToWide ("ship does not have an slotted container for player.\n");
			return true;
		}

		std::vector<SlotId> slots;
		shipContainer->getSlotIdList(slots);

		Container::ContainerErrorCode tmp = Container::CEC_Success;
		for(std::vector<SlotId>::iterator i = slots.begin(); i != slots.end(); ++i)
		{
			const SlotId slotId = *i;

			// check if container supports this slot
			if (!shipContainer->hasSlot(slotId))
			{
				result += Unicode::narrowToWide ("ship doesn't have slot.\n");
				return true;
			}

			// check if any object occupies this slot
			Container::ContainedItem item = shipContainer->getObjectInSlot(slotId, tmp);
			if (item == CachedNetworkId::cms_cachedInvalid)
			{
				result += Unicode::narrowToWide ("couldn't get item in slot.\n");
				return true;
			}

			// get the object
			Object *equippedObject = item.getObject();
			if (!equippedObject)
			{
				result += Unicode::narrowToWide ("couldn't turn containeditem into object for slot.\n");
				return true;
			}

			// detatch item from container
			IGNORE_RETURN (shipContainer->remove(*equippedObject, tmp));
		}

		player->setState(States::PilotingShip, false);
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_reloadForms))
	{
		FormManagerClient::reloadData();
		sendMessageToServer(Unicode::narrowToWide("server reloadForms"));
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_pseudoDamageShip))
	{
		CreatureObject * const player = Game::getPlayerCreature();
		if (player)
		{
			Object* const targetObject = player->getLookAtTarget().getObject();
			if (targetObject)
			{
				ShipDamageMessage shipDamageMsg(targetObject->getNetworkId(), targetObject->getPosition_w(), 0.0f, true);
				CuiDamageManager::handleShipDamage(shipDamageMsg);
			}
			else
				result += Unicode::narrowToWide ("no lookAtTarget\n");
		}
	}
	
	//----------------------------------------------------------------------

	else if (isCommand(argv[0], ms_splitObject))
	{
		CreatureObject * const player = Game::getPlayerCreature();
		if (!player)
			return true;

		Object* const targetObject = player->getLookAtTarget().getObject();
		if (!targetObject)
			return true;

		Appearance * const appearance = targetObject->getAppearance();
		if (appearance == NULL)
			return true;

		Vector initialVelocity;

		if (argv.size() > 4)
		{
			initialVelocity.x = static_cast<float>(atof(Unicode::wideToNarrow(argv[2]).c_str()));
			initialVelocity.y = static_cast<float>(atof(Unicode::wideToNarrow(argv[3]).c_str()));
			initialVelocity.z = static_cast<float>(atof(Unicode::wideToNarrow(argv[4]).c_str()));

			initialVelocity = targetObject->rotate_o2w(initialVelocity);
		}

		int const numCuts = atoi(Unicode::wideToNarrow(argv[1]).c_str());

		typedef stdvector<DynamicDebrisObject *>::fwd DynamicDebrisObjectVector;
		DynamicDebrisObjectVector dynamicDebrisObjectVector;

		float const radius = appearance->getSphere().getRadius();
		float const halfRadius = radius * 0.5f;

		PerformanceTimer ptimer;
		ptimer.start();
		
		DynamicDebrisObject::PlaneVector planeVector;

		for (int i = 0; i < numCuts; ++i)
		{
			Vector const & planePos = Vector::randomUnit() * Random::randomReal(0, halfRadius);
			Vector const & planeNormal = Vector::randomUnit();
			Plane const plane(planeNormal, planePos);

			planeVector.push_back(plane);
		}

		DynamicDebrisObject::splitObjects(*targetObject, planeVector, dynamicDebrisObjectVector, -1.0f);

		/*
		//-- first cut
		{
			Vector const & planePos = Vector::randomUnit() * Random::randomReal(0, halfRadius);
			Vector const & planeNormal = Vector::randomUnit();
			Plane const plane(planeNormal, planePos);

			DynamicDebrisObject * newDynamicDebrisObject[2] = {NULL, NULL};
			if (DynamicDebrisObject::splitObjects(*targetObject, plane, newDynamicDebrisObject[0], newDynamicDebrisObject[1]))
			{
				if (newDynamicDebrisObject[0] != NULL)
					dynamicDebrisObjectVector.push_back(newDynamicDebrisObject[0]);
				if (newDynamicDebrisObject[1] != NULL)
					dynamicDebrisObjectVector.push_back(newDynamicDebrisObject[1]);
			}
		}

		//-- recursively cut
		for (int i = 1; i < numCuts; ++i)
		{
			Vector const & planePos = Vector::randomUnit() * Random::randomReal(0, halfRadius);
			Vector const & planeNormal = Vector::randomUnit();
			Plane const plane(planeNormal, planePos);

			DynamicDebrisObjectVector dynamicDebrisObjectVectorToAdd;

			for (DynamicDebrisObjectVector::iterator it = dynamicDebrisObjectVector.begin(); it != dynamicDebrisObjectVector.end(); ++it)
			{
				DynamicDebrisObject * const dynamicDebrisObject = *it;
				if (dynamicDebrisObject == NULL)
					continue;
				
				DynamicDebrisObject * newDynamicDebrisObject[2] = {NULL, NULL};
				if (DynamicDebrisObject::splitObjects(*dynamicDebrisObject, plane, newDynamicDebrisObject[0], newDynamicDebrisObject[1]))
				{
					if (newDynamicDebrisObject[0] != NULL)
						dynamicDebrisObjectVectorToAdd.push_back(newDynamicDebrisObject[0]);
					if (newDynamicDebrisObject[1] != NULL)
						dynamicDebrisObjectVectorToAdd.push_back(newDynamicDebrisObject[1]);

					//-- this  mesh appearance has been subdivided, so get rid of it
					delete dynamicDebrisObject;
					(*it) = NULL;
				}
			}

			dynamicDebrisObjectVector.insert(dynamicDebrisObjectVector.end(), dynamicDebrisObjectVectorToAdd.begin(), dynamicDebrisObjectVectorToAdd.end());
		}
		
		  */

		for (DynamicDebrisObjectVector::iterator it = dynamicDebrisObjectVector.begin(); it != dynamicDebrisObjectVector.end(); ++it)
		{
			DynamicDebrisObject * const dynamicDebrisObject = *it;
			if (dynamicDebrisObject == NULL)
				continue;

			Vector const & offset_w = dynamicDebrisObject->rotate_o2w(dynamicDebrisObject->getAppearanceSphereCenter());
			dynamicDebrisObject->setTransform_o2w(targetObject->getTransform_o2w());
			dynamicDebrisObject->setPhysicsParameters(10.0f, initialVelocity + offset_w, dynamicDebrisObject->getAppearanceSphereCenter());
				
			RenderWorld::addObjectNotifications (*dynamicDebrisObject);
			dynamicDebrisObject->addToWorld();
			dynamicDebrisObject->scheduleForAlter();
		}

		ptimer.stop();
		ptimer.logElapsedTime(" *** random splitting complete in ");

//		targetObject->kill();

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], ms_splitRandom))
	{
		CreatureObject * const player = Game::getPlayerCreature();
		if (!player)
			return true;

		Object* const targetObject = player->getLookAtTarget().getObject();
		if (!targetObject)
			return true;

		Appearance * const appearance = targetObject->getAppearance();
		if (appearance == NULL)
			return true;

		int const numCuts = atoi(Unicode::wideToNarrow(argv[1]).c_str());

		typedef std::pair<DynamicMeshAppearance *, Vector> DynamicMeshAppearanceVectorPair;
		typedef stdvector<DynamicMeshAppearanceVectorPair>::fwd DynamicMeshAppearanceVector;
		DynamicMeshAppearanceVector dynamicMeshAppearanceVector;

		float const radius = appearance->getSphere().getRadius();
		float const halfRadius = radius * 0.5f;

		Vector const randomizationVector(0.2f, 0.2f, 0.2f);

		PerformanceTimer ptimer;
		ptimer.start();
		
		float const seperationDistance = 10.0f;

		//-- first cut
		{
			Vector const & planePos = Vector::randomUnit() * Random::randomReal(0, halfRadius);
			Vector const & planeNormal = Vector::randomUnit();
			Plane const plane(planeNormal, planePos);

			DynamicMeshAppearance * newDynamicMeshAppearance[2] = {NULL, NULL};
			if (DynamicMeshAppearance::splitAppearance(*appearance, plane, randomizationVector, newDynamicMeshAppearance[0], newDynamicMeshAppearance[1], -1.0f))
			{
				if (newDynamicMeshAppearance[0] != NULL)
					dynamicMeshAppearanceVector.push_back(DynamicMeshAppearanceVectorPair(newDynamicMeshAppearance[0], planeNormal * (seperationDistance)));
				if (newDynamicMeshAppearance[1] != NULL)
					dynamicMeshAppearanceVector.push_back(DynamicMeshAppearanceVectorPair(newDynamicMeshAppearance[1], planeNormal * (-seperationDistance)));
			}
		}

		//-- recursively cut
		for (int i = 1; i < numCuts; ++i)
		{
			Vector const & planePos = Vector::randomUnit() * Random::randomReal(0, halfRadius);
			Vector const & planeNormal = Vector::randomUnit();
			Plane const plane(planeNormal, planePos);

			DynamicMeshAppearanceVector dynamicMeshAppearanceVectorToAdd;

			for (DynamicMeshAppearanceVector::iterator it = dynamicMeshAppearanceVector.begin(); it != dynamicMeshAppearanceVector.end(); ++it)
			{
				DynamicMeshAppearance * const dynamicMeshAppearance = (*it).first;
				if (dynamicMeshAppearance == NULL)
					continue;

				Vector const & offset = (*it).second;

				DynamicMeshAppearance * newDynamicMeshAppearance[2] = {NULL, NULL};
				if (DynamicMeshAppearance::splitDynamicMeshAppearance(*dynamicMeshAppearance, plane, randomizationVector, newDynamicMeshAppearance[0], newDynamicMeshAppearance[1]))
				{
					if (newDynamicMeshAppearance[0] != NULL)
						dynamicMeshAppearanceVectorToAdd.push_back(DynamicMeshAppearanceVectorPair(newDynamicMeshAppearance[0], offset + (planeNormal * seperationDistance)));
					if (newDynamicMeshAppearance[1] != NULL)
						dynamicMeshAppearanceVectorToAdd.push_back(DynamicMeshAppearanceVectorPair(newDynamicMeshAppearance[1], offset - (planeNormal * seperationDistance)));

					//-- this  mesh appearance has been subdivided, so get rid of it
					delete dynamicMeshAppearance;
					(*it).first = NULL;
				}
			}

			dynamicMeshAppearanceVector.insert(dynamicMeshAppearanceVector.end(), dynamicMeshAppearanceVectorToAdd.begin(), dynamicMeshAppearanceVectorToAdd.end());
		}
		
		for (DynamicMeshAppearanceVector::iterator it = dynamicMeshAppearanceVector.begin(); it != dynamicMeshAppearanceVector.end(); ++it)
		{
			DynamicMeshAppearance * const dynamicMeshAppearance = (*it).first;
			Vector const & offset = (*it).second;

			Object * const obj = new Object;
			obj->setAppearance(dynamicMeshAppearance);
			obj->setTransform_o2w(targetObject->getTransform_o2w());
			obj->move_o(offset);
			RotationDynamics * const rotationDynamics = new RotationDynamics(obj, Vector::randomUnit());
			rotationDynamics->setState(true);
			rotationDynamics->setRotateAroundAppearanceCenter(true);
			obj->setDynamics(rotationDynamics);

			RenderWorld::addObjectNotifications (*obj);
			obj->addToWorld();

			obj->scheduleForAlter();
		}

		ptimer.stop();
		ptimer.logElapsedTime(" *** random splitting complete in ");

		targetObject->kill();

		return true;
	}

	else if (isCommand(argv[0], ms_playForceFeedbackTemplate))
	{
		std::string file = Unicode::wideToNarrow(argv[1]);
		ForceFeedbackEffectTemplate const * const effectTemplate = ForceFeedbackEffectTemplateList::fetch(file);
		if (effectTemplate)
		{
			effectTemplate->playEffect(1);
			ForceFeedbackEffectTemplateList::release(effectTemplate);
		}
		else
		{
			result += Unicode::narrowToWide ("Unknown force feedback file.\n");
		}
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], ms_reloadQuests))
	{
		QuestManager::reloadQuests();
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], ms_reloadTable))
	{
		std::string const & tableName = Unicode::wideToNarrow(argv[1]);
		return (DataTableManager::reloadIfOpen(tableName) != 0);
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], ms_clientSetMovementScale))
	{
		CreatureObject * const player = Game::getPlayerCreature();
		if (!player)
			return true;

		float const scale = static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str()));
		player->clientSetMovementScale(scale);
		player->clientSetAccelScale(scale);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], ms_getObjectsInWorldObjectList))
	{
		int index = -1;
		std::string const & listType = Unicode::wideToNarrow(argv[1]);
		if(listType == "tangible")
		{
			index = WOL_Tangible;
		}
		else if(listType == "tangible_not_targetable")
		{
			index = WOL_TangibleNotTargetable;
		}
		else if(listType == "intangible")
		{
			index = WOL_Intangible;
		}
		else if(listType == "tangible_flora")
		{
			index = WOL_TangibleFlora;
		}
		else if(listType == "inactive")
		{
			index = WOL_Inactive;
		}

		if(index != -1)
		{
			char buffer[512];

			if(index < 0 || index >= WOL_Count)
			{
				sprintf (buffer, "Invalid index.  Valid indexes are [0, %d)", WOL_Count);
				result += Unicode::narrowToWide (buffer);
			}
			else
			{
				int num = World::getNumberOfObjects (index);
				sprintf (buffer, "%d Objects in List: %d\n", num, index);
				result += Unicode::narrowToWide (buffer);
				Unicode::String const & text    = Unicode::narrowToWide("Object [");
				Unicode::String const & text2   = Unicode::narrowToWide("] objecttemplate [");
				Unicode::String const & text3   = Unicode::narrowToWide("] appearancetemplate [");
				Unicode::String const & text4   = Unicode::narrowToWide("]. In World: [");
				Unicode::String const & endText = Unicode::narrowToWide("].\n");
				Unicode::String const & t       = Unicode::narrowToWide("true");
				Unicode::String const & f       = Unicode::narrowToWide("false");
				for(int i = 0; i < num; ++i)
				{
					Object const * const o = World::getObject(index, i);
					if(o)
					{
						char const * const objectTemplateName = o->getObjectTemplateName();
						char const * objectTemplateFinalName = NULL;
						if(objectTemplateName)
						{
							objectTemplateFinalName = objectTemplateName;
						}
						else
						{
							objectTemplateFinalName = "Unknown";
						}

						char const * const appearanceTemplateName = o->getAppearanceTemplateName();
						char const * appearanceTemplateFinalName = NULL;
						if(appearanceTemplateName)
						{
							appearanceTemplateFinalName = appearanceTemplateName;
						}
						else
						{
							appearanceTemplateFinalName = "Unknown";
						}

						NetworkId const & nid = o->getNetworkId();
						result += text;
						result += Unicode::narrowToWide (nid.getValueString().c_str());
						result += text2;
						result += Unicode::narrowToWide (objectTemplateFinalName);
						result += text3;
						result += Unicode::narrowToWide (appearanceTemplateFinalName);
						result += text4;
						bool const isInWorld = o->isInWorld();
						if(isInWorld)
						{
							result += t;
						}
						else
						{
							result += f;
						}
						result += endText;
					}
				}
			}
		}
		else
		{
			result += Unicode::narrowToWide("Unknown list type");
		}
	}
	else if (isCommand(argv[0], ms_getObjectInfo))
	{
		std::string const & nidStr = Unicode::wideToNarrow(argv[1]);
		NetworkId nid(nidStr);

		Object const * const o = NetworkIdManager::getObjectById(nid);
		if(o)
		{
			std::map<std::string, std::map<std::string, Unicode::String> > m;
			o->getObjectInfo(m);

			bool displayedOneSection = false;

			if(argv.size() > 2)
			{
				std::string const & sectionName = Unicode::wideToNarrow(argv[2]);
				if(m.find(sectionName) != m.end())
				{
					DebugInfoManager::PropertyMap::const_iterator const i = m.find(sectionName);
					DebugInfoManager::PropertySection const & sectionMap = i->second;
					result += Unicode::narrowToWide("[") + Unicode::narrowToWide(sectionName) + Unicode::narrowToWide("]\n");
					for(DebugInfoManager::PropertySection::const_iterator i2 = sectionMap.begin(); i2 != sectionMap.end(); ++i2)
					{
						std::string const & propertyName  = i2->first;
						Unicode::String const & propertyValue = i2->second;
						result += Unicode::narrowToWide("\t") + Unicode::narrowToWide(propertyName) + Unicode::narrowToWide("=") + propertyValue + Unicode::narrowToWide("\n");
					}
					displayedOneSection = true;
				}
			}
			if(!displayedOneSection)
			{
				result += Unicode::narrowToWide("Select a section:\n");
				for(DebugInfoManager::PropertyMap::const_iterator i = m.begin(); i != m.end(); ++i)
				{
					std::string const & sectionName = i->first;
					result += Unicode::narrowToWide("\t[") + Unicode::narrowToWide(sectionName) + Unicode::narrowToWide("]...\n");
				}
			}
		}
		else
		{
			result += Unicode::narrowToWide("Invalid id.");
		}
	}
	else if (isCommand(argv[0], ms_findObjectInfo))
	{
		std::string const & nidStr = Unicode::wideToNarrow(argv[1]);
		NetworkId nid(nidStr);

		Object const * const o = NetworkIdManager::getObjectById(nid);
		if(o)
		{
			std::map<std::string, std::map<std::string, Unicode::String> > m;
			o->getObjectInfo(m);

			std::vector<std::string> searchStrings;
			for (int i = 2; i < static_cast<int>(argv.size()); ++i)
				searchStrings.push_back(Unicode::wideToNarrow(Unicode::toLower(argv[i])));

			bool matchedAProperty = false;
			for(DebugInfoManager::PropertyMap::const_iterator it = m.begin(); it != m.end(); ++it)
			{
				std::string const & sectionName = it->first;
				DebugInfoManager::PropertySection const & sectionMap = it->second;
				for(DebugInfoManager::PropertySection::const_iterator it2 = sectionMap.begin(); it2 != sectionMap.end(); ++it2)
				{
					std::string const & propertyName  = it2->first;
					Unicode::String const & propertyValue = it2->second;
					std::string const & propertyNameNarrow = Unicode::wideToNarrow(Unicode::toLower(Unicode::narrowToWide(propertyName)));
					bool matchesAllTerms = true;
					for (std::vector<std::string>::const_iterator it3 = searchStrings.begin(); (it3 != searchStrings.end()) && matchesAllTerms; ++it3)
					{
						if(propertyNameNarrow.find(*it3) == propertyNameNarrow.npos)
						{
							matchesAllTerms = false;
						}
					}
					if(matchesAllTerms)
					{
						result += Unicode::narrowToWide("[") + Unicode::narrowToWide(sectionName) + Unicode::narrowToWide("]") + Unicode::narrowToWide(propertyName) + Unicode::narrowToWide("=") + propertyValue + Unicode::narrowToWide("\n");
						matchedAProperty = true;
					}
				}
			}
			if(!matchedAProperty)
			{
				result += Unicode::narrowToWide("No results.");
			}
		}
		else
		{
			result += Unicode::narrowToWide("Invalid id.");
		}
	}

	//----------------------------------------------------------------------

	else if(isCommand(argv[0], ms_setWeather))
	{
		int weatherIndex = atoi (Unicode::wideToNarrow (argv [1]).c_str ());
		
		GroundEnvironment::getInstance().setWeatherIndex(weatherIndex);
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], ms_pathCreate))
	{
		CreatureObject * const player = Game::getPlayerCreature();
		if (!player)
			return true;

		float const scale = static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str()));
		std::vector<Vector> pointList;

		for (float c = 0; c < PI_TIMES_2; c += (PI_OVER_180 * 4.0f))
		{
			Vector pos;

			float const newScale = Random::randomReal(scale / 2.0f, scale);

			pos.x = cosf(c) * newScale;
			pos.z = sinf(c) * newScale;
			pos.y = 0.0f;

			pointList.push_back(pos);
		}
		
		if (s_clientPathObject)
		{
			delete s_clientPathObject;
			s_clientPathObject = 0;
		}

		if (pointList.size())
		{
			s_clientPathObject = new ClientPathObject(pointList);
			s_clientPathObject->addNotification(ClientWorld::getIntangibleNotification());
			s_clientPathObject->addToWorld();
		}

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], ms_pathAppearance))
	{
		CreatureObject * const player = Game::getPlayerCreature();
		if (!player)
			return true;

		ClientPathObject::setAppearance(Unicode::wideToNarrow(argv[1]));

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], ms_showFlyText))
	{
		CreatureObject * const player = Game::getPlayerCreature();
		if (!player)
			return true;

		Unicode::String const & s = argv[1];

		player->addFlyText(s, 3.0, VectorArgb::solidRed, 1.0, CuiTextManagerTextEnqueueInfo::TW_starwars, true, true, true);

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], ms_listStaticLootItemData))
	{
		std::vector<std::string> const & itemNames = CuiStaticLootItemManager::getAllKnownLootItems();
		for(std::vector<std::string>::const_iterator i = itemNames.begin(); i != itemNames.end(); ++i)
		{
			result += Unicode::narrowToWide(*i);
			result += Unicode::narrowToWide("\n");
		}

		if(itemNames.empty())
		{
			result += Unicode::narrowToWide("No known items.\n");
		}
		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], ms_showStaticLootItemData))
	{
		CuiStaticLootItemManager::ItemDictionary const * itemData = CuiStaticLootItemManager::getItemData(Unicode::wideToNarrow(argv[1]));

		if(itemData && !itemData->empty())
		{
			for(CuiStaticLootItemManager::ItemDictionary::const_iterator i = itemData->begin(); i != itemData->end(); ++i)
			{
				result += i->first;
				result += Unicode::narrowToWide("=");
				result += i->second;
				result += Unicode::narrowToWide("\n");
			}
		}
		else
		{
			result += Unicode::narrowToWide("Item not found");
		}
		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_listCollectionDataByCategory))
	{
		std::set<std::string> const & categories = CollectionsDataTable::getAllSlotCategories();
		for (std::set<std::string>::const_iterator iterCategory = categories.begin(); iterCategory != categories.end(); ++iterCategory)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s\n",
				iterCategory->c_str()));

			std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCategory(*iterCategory);
			for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iterSlot = slots.begin(); iterSlot != slots.end(); ++iterSlot)
			{
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("        %s (slotId=%s)\n",
					(*iterSlot)->name.c_str(),
					getCollectionSlotIdAsText(**iterSlot).c_str()));
			}
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_listCollectionDataBrief))
	{
		std::string categories;
		std::string prereqs;
		std::vector<CollectionsDataTable::CollectionInfoBook const *> const & books = CollectionsDataTable::getAllBooks();
		for (std::vector<CollectionsDataTable::CollectionInfoBook const *>::const_iterator iterBook = books.begin(); iterBook != books.end(); ++iterBook)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s\n",
				(*iterBook)->name.c_str()));

			std::vector<CollectionsDataTable::CollectionInfoPage const *> const & pages = CollectionsDataTable::getPagesInBook((*iterBook)->name);
			for (std::vector<CollectionsDataTable::CollectionInfoPage const *>::const_iterator iterPage = pages.begin(); iterPage != pages.end(); ++iterPage)
			{
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("    %s\n",
					(*iterPage)->name.c_str()));

				std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & collections = CollectionsDataTable::getCollectionsInPage((*iterPage)->name);
				for (std::vector<CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iterCollection = collections.begin(); iterCollection != collections.end(); ++iterCollection)
				{
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("        %s\n",
						(*iterCollection)->name.c_str()));

					std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCollection((*iterCollection)->name);
					for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iterSlot = slots.begin(); iterSlot != slots.end(); ++iterSlot)
					{
						for (std::vector<std::string>::const_iterator iterCategories = (*iterSlot)->categories.begin(); iterCategories != (*iterSlot)->categories.end(); ++iterCategories)
						{
							if (!categories.empty())
								categories += ", ";

							categories += *iterCategories;
						}

						for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iterPrereqs = (*iterSlot)->prereqsPtr.begin(); iterPrereqs != (*iterSlot)->prereqsPtr.end(); ++iterPrereqs)
						{
							if (!prereqs.empty())
								prereqs += ", ";

							prereqs += (*iterPrereqs)->name;
						}

						result += Unicode::narrowToWide(FormattedString<512>().sprintf("            %s (slotId=%s, category=%s, prereq=%s)\n",
							(*iterSlot)->name.c_str(),
							getCollectionSlotIdAsText(**iterSlot).c_str(),
							categories.c_str(), prereqs.c_str()));

						categories.clear();
						prereqs.clear();
					}
				}
			}
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_listCollectionDataBriefLocalized))
	{
		std::string categories;
		std::string prereqs;
		std::vector<CollectionsDataTable::CollectionInfoBook const *> const & books = CollectionsDataTable::getAllBooks();
		for (std::vector<CollectionsDataTable::CollectionInfoBook const *>::const_iterator iterBook = books.begin(); iterBook != books.end(); ++iterBook)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s, %s, %s\n",
				(*iterBook)->name.c_str(),
				Unicode::wideToNarrow(CollectionsDataTable::localizeCollectionName((*iterBook)->name)).c_str(),
				Unicode::wideToNarrow(CollectionsDataTable::localizeCollectionDescription((*iterBook)->name)).c_str()));

			std::vector<CollectionsDataTable::CollectionInfoPage const *> const & pages = CollectionsDataTable::getPagesInBook((*iterBook)->name);
			for (std::vector<CollectionsDataTable::CollectionInfoPage const *>::const_iterator iterPage = pages.begin(); iterPage != pages.end(); ++iterPage)
			{
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("    %s, %s, %s\n",
					(*iterPage)->name.c_str(),
					Unicode::wideToNarrow(CollectionsDataTable::localizeCollectionName((*iterPage)->name)).c_str(),
					Unicode::wideToNarrow(CollectionsDataTable::localizeCollectionDescription((*iterPage)->name)).c_str()));

				std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & collections = CollectionsDataTable::getCollectionsInPage((*iterPage)->name);
				for (std::vector<CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iterCollection = collections.begin(); iterCollection != collections.end(); ++iterCollection)
				{
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("        %s, %s, %s\n",
						(*iterCollection)->name.c_str(),
						Unicode::wideToNarrow(CollectionsDataTable::localizeCollectionName((*iterCollection)->name)).c_str(),
						Unicode::wideToNarrow(CollectionsDataTable::localizeCollectionDescription((*iterCollection)->name)).c_str()));

					std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCollection((*iterCollection)->name);
					for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iterSlot = slots.begin(); iterSlot != slots.end(); ++iterSlot)
					{
						for (std::vector<std::string>::const_iterator iterCategories = (*iterSlot)->categories.begin(); iterCategories != (*iterSlot)->categories.end(); ++iterCategories)
						{
							if (!categories.empty())
								categories += ", ";

							categories += *iterCategories;
						}

						for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iterPrereqs = (*iterSlot)->prereqsPtr.begin(); iterPrereqs != (*iterSlot)->prereqsPtr.end(); ++iterPrereqs)
						{
							if (!prereqs.empty())
								prereqs += ", ";

							prereqs += (*iterPrereqs)->name;
						}

						result += Unicode::narrowToWide(FormattedString<512>().sprintf("            %s (slotId=%s, category=%s, prereq=%s), %s, %s\n",
							(*iterSlot)->name.c_str(),
							getCollectionSlotIdAsText(**iterSlot).c_str(),
							categories.c_str(),
							prereqs.c_str(),
							Unicode::wideToNarrow(CollectionsDataTable::localizeCollectionName((*iterSlot)->name)).c_str(),
							Unicode::wideToNarrow(CollectionsDataTable::localizeCollectionDescription((*iterSlot)->name)).c_str()));

						categories.clear();
						prereqs.clear();
					}
				}
			}
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_listCollectionDataDetailed))
	{
		std::string categories;
		std::string prereqs;
		std::vector<CollectionsDataTable::CollectionInfoBook const *> const & books = CollectionsDataTable::getAllBooks();
		for (std::vector<CollectionsDataTable::CollectionInfoBook const *>::const_iterator iterBook = books.begin(); iterBook != books.end(); ++iterBook)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s, %s, showIfNotYetEarned=%s, hidden=%s\n",
				(*iterBook)->name.c_str(),
				(*iterBook)->icon.c_str(),
				CollectionsDataTable::getShowIfNotYetEarnedTypeString((*iterBook)->showIfNotYetEarned),
				((*iterBook)->hidden ? "yes" : "no")));

			std::vector<CollectionsDataTable::CollectionInfoPage const *> const & pages = CollectionsDataTable::getPagesInBook((*iterBook)->name);
			for (std::vector<CollectionsDataTable::CollectionInfoPage const *>::const_iterator iterPage = pages.begin(); iterPage != pages.end(); ++iterPage)
			{
				std::string title;
				if (!(*iterPage)->titles.empty())
				{
					for (std::vector<std::string>::const_iterator iterTitle = (*iterPage)->titles.begin(); iterTitle != (*iterPage)->titles.end(); ++iterTitle)
					{
						if (title.empty())
							title = "(";
						else
							title += ", ";

						title += *iterTitle;
					}
				}

				if (title.empty())
					title = "no";
				else
					title += ")";

				result += Unicode::narrowToWide(FormattedString<512>().sprintf("    %s, %s, title=%s, showIfNotYetEarned=%s, hidden=%s\n",
					(*iterPage)->name.c_str(),
					(*iterPage)->icon.c_str(),
					title.c_str(),
					CollectionsDataTable::getShowIfNotYetEarnedTypeString((*iterPage)->showIfNotYetEarned),
					((*iterPage)->hidden ? "yes" : "no")));

				std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & collections = CollectionsDataTable::getCollectionsInPage((*iterPage)->name);
				for (std::vector<CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iterCollection = collections.begin(); iterCollection != collections.end(); ++iterCollection)
				{
					std::string title;
					if (!(*iterCollection)->titles.empty())
					{
						for (std::vector<std::string>::const_iterator iterTitle = (*iterCollection)->titles.begin(); iterTitle != (*iterCollection)->titles.end(); ++iterTitle)
						{
							if (title.empty())
								title = "(";
							else
								title += ", ";

							title += *iterTitle;
						}
					}

					if (title.empty())
						title = "no";
					else
						title += ")";

					result += Unicode::narrowToWide(FormattedString<512>().sprintf("        %s, %s, title=%s, showIfNotYetEarned=%s, hidden=%s, noReward=%s, trackServerFirst=%s\n",
						(*iterCollection)->name.c_str(),
						(*iterCollection)->icon.c_str(),
						title.c_str(),
						CollectionsDataTable::getShowIfNotYetEarnedTypeString((*iterCollection)->showIfNotYetEarned),
						((*iterCollection)->hidden ? "yes" : "no"),
						((*iterCollection)->noReward ? "yes" : "no"),
						((*iterCollection)->trackServerFirst ? "yes" : "no")));

					std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCollection((*iterCollection)->name);
					for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iterSlot = slots.begin(); iterSlot != slots.end(); ++iterSlot)
					{
						for (std::vector<std::string>::const_iterator iterCategories = (*iterSlot)->categories.begin(); iterCategories != (*iterSlot)->categories.end(); ++iterCategories)
						{
							if (!categories.empty())
								categories += ", ";

							categories += *iterCategories;
						}

						for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iterPrereqs = (*iterSlot)->prereqsPtr.begin(); iterPrereqs != (*iterSlot)->prereqsPtr.end(); ++iterPrereqs)
						{
							if (!prereqs.empty())
								prereqs += ", ";

							prereqs += (*iterPrereqs)->name;
						}

						std::string title;
						if (!(*iterSlot)->titles.empty())
						{
							for (std::vector<std::string>::const_iterator iterTitle = (*iterSlot)->titles.begin(); iterTitle != (*iterSlot)->titles.end(); ++iterTitle)
							{
								if (title.empty())
									title = "(";
								else
									title += ", ";

								title += *iterTitle;
							}
						}

						if (title.empty())
							title = "no";
						else
							title += ")";

						result += Unicode::narrowToWide(FormattedString<512>().sprintf("            %s (slotId=%s, category=%s, prereq=%s), %s, %s, title=%s, showIfNotYetEarned=%s, hidden=%s\n",
							(*iterSlot)->name.c_str(),
							getCollectionSlotIdAsText(**iterSlot).c_str(),
							categories.c_str(),
							prereqs.c_str(),
							(*iterSlot)->icon.c_str(),
							(*iterSlot)->music.c_str(),
							title.c_str(),
							CollectionsDataTable::getShowIfNotYetEarnedTypeString((*iterSlot)->showIfNotYetEarned),
							((*iterSlot)->hidden ? "yes" : "no")));

						categories.clear();
						prereqs.clear();
					}
				}
			}
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_listCollectionDataDetailedLocalized))
	{
		std::string categories;
		std::string prereqs;
		std::vector<CollectionsDataTable::CollectionInfoBook const *> const & books = CollectionsDataTable::getAllBooks();
		for (std::vector<CollectionsDataTable::CollectionInfoBook const *>::const_iterator iterBook = books.begin(); iterBook != books.end(); ++iterBook)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s, %s, %s, %s, showIfNotYetEarned=%s, hidden=%s\n",
				(*iterBook)->name.c_str(),
				Unicode::wideToNarrow(CollectionsDataTable::localizeCollectionName((*iterBook)->name)).c_str(),
				Unicode::wideToNarrow(CollectionsDataTable::localizeCollectionDescription((*iterBook)->name)).c_str(),
				(*iterBook)->icon.c_str(),
				CollectionsDataTable::getShowIfNotYetEarnedTypeString((*iterBook)->showIfNotYetEarned),
				((*iterBook)->hidden ? "yes" : "no")));

			std::vector<CollectionsDataTable::CollectionInfoPage const *> const & pages = CollectionsDataTable::getPagesInBook((*iterBook)->name);
			for (std::vector<CollectionsDataTable::CollectionInfoPage const *>::const_iterator iterPage = pages.begin(); iterPage != pages.end(); ++iterPage)
			{
				std::string title;
				Unicode::String localizedTitle;
				if (!(*iterPage)->titles.empty())
				{
					for (std::vector<std::string>::const_iterator iterTitle = (*iterPage)->titles.begin(); iterTitle != (*iterPage)->titles.end(); ++iterTitle)
					{
						if (title.empty())
							title = "(";
						else
							title += ", ";

						title += *iterTitle;

						if (localizedTitle.empty())
							localizedTitle = Unicode::narrowToWide("[");
						else
							localizedTitle += Unicode::narrowToWide(", ");

						localizedTitle += CollectionsDataTable::localizeCollectionTitle(*iterTitle);
					}
				}

				if (title.empty())
					title = "no";
				else
					title += ")";

				if (localizedTitle.empty())
					localizedTitle = Unicode::narrowToWide("[]");
				else
					localizedTitle += Unicode::narrowToWide("]");

				result += Unicode::narrowToWide(FormattedString<512>().sprintf("    %s, %s, %s, %s, title=%s %s, showIfNotYetEarned=%s, hidden=%s\n",
					(*iterPage)->name.c_str(),
					Unicode::wideToNarrow(CollectionsDataTable::localizeCollectionName((*iterPage)->name)).c_str(),
					Unicode::wideToNarrow(CollectionsDataTable::localizeCollectionDescription((*iterPage)->name)).c_str(),
					(*iterPage)->icon.c_str(),
					title.c_str(),
					Unicode::wideToNarrow(localizedTitle).c_str(),
					CollectionsDataTable::getShowIfNotYetEarnedTypeString((*iterPage)->showIfNotYetEarned),
					((*iterPage)->hidden ? "yes" : "no")));

				std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & collections = CollectionsDataTable::getCollectionsInPage((*iterPage)->name);
				for (std::vector<CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iterCollection = collections.begin(); iterCollection != collections.end(); ++iterCollection)
				{
					std::string title;
					Unicode::String localizedTitle;
					if (!(*iterCollection)->titles.empty())
					{
						for (std::vector<std::string>::const_iterator iterTitle = (*iterCollection)->titles.begin(); iterTitle != (*iterCollection)->titles.end(); ++iterTitle)
						{
							if (title.empty())
								title = "(";
							else
								title += ", ";

							title += *iterTitle;

							if (localizedTitle.empty())
								localizedTitle = Unicode::narrowToWide("[");
							else
								localizedTitle += Unicode::narrowToWide(", ");

							localizedTitle += CollectionsDataTable::localizeCollectionTitle(*iterTitle);
						}
					}

					if (title.empty())
						title = "no";
					else
						title += ")";

					if (localizedTitle.empty())
						localizedTitle = Unicode::narrowToWide("[]");
					else
						localizedTitle += Unicode::narrowToWide("]");

					result += Unicode::narrowToWide(FormattedString<512>().sprintf("        %s, %s, %s, %s, title=%s %s, showIfNotYetEarned=%s, hidden=%s, noReward=%s, trackServerFirst=%s\n",
						(*iterCollection)->name.c_str(),
						Unicode::wideToNarrow(CollectionsDataTable::localizeCollectionName((*iterCollection)->name)).c_str(),
						Unicode::wideToNarrow(CollectionsDataTable::localizeCollectionDescription((*iterCollection)->name)).c_str(),
						(*iterCollection)->icon.c_str(),
						title.c_str(),
						Unicode::wideToNarrow(localizedTitle).c_str(),
						CollectionsDataTable::getShowIfNotYetEarnedTypeString((*iterCollection)->showIfNotYetEarned),
						((*iterCollection)->hidden ? "yes" : "no"),
						((*iterCollection)->noReward ? "yes" : "no"),
						((*iterCollection)->trackServerFirst ? "yes" : "no")));

					std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCollection((*iterCollection)->name);
					for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iterSlot = slots.begin(); iterSlot != slots.end(); ++iterSlot)
					{
						for (std::vector<std::string>::const_iterator iterCategories = (*iterSlot)->categories.begin(); iterCategories != (*iterSlot)->categories.end(); ++iterCategories)
						{
							if (!categories.empty())
								categories += ", ";

							categories += *iterCategories;
						}

						for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iterPrereqs = (*iterSlot)->prereqsPtr.begin(); iterPrereqs != (*iterSlot)->prereqsPtr.end(); ++iterPrereqs)
						{
							if (!prereqs.empty())
								prereqs += ", ";

							prereqs += (*iterPrereqs)->name;
						}

						std::string title;
						Unicode::String localizedTitle;
						if (!(*iterSlot)->titles.empty())
						{
							for (std::vector<std::string>::const_iterator iterTitle = (*iterSlot)->titles.begin(); iterTitle != (*iterSlot)->titles.end(); ++iterTitle)
							{
								if (title.empty())
									title = "(";
								else
									title += ", ";

								title += *iterTitle;

								if (localizedTitle.empty())
									localizedTitle = Unicode::narrowToWide("[");
								else
									localizedTitle += Unicode::narrowToWide(", ");

								localizedTitle += CollectionsDataTable::localizeCollectionTitle(*iterTitle);
							}
						}

						if (title.empty())
							title = "no";
						else
							title += ")";

						if (localizedTitle.empty())
							localizedTitle = Unicode::narrowToWide("[]");
						else
							localizedTitle += Unicode::narrowToWide("]");

						result += Unicode::narrowToWide(FormattedString<512>().sprintf("            %s (slotId=%s, category=%s, prereq=%s), %s, %s, %s, %s, title=%s %s, showIfNotYetEarned=%s, hidden=%s\n",
							(*iterSlot)->name.c_str(),
							getCollectionSlotIdAsText(**iterSlot).c_str(),
							categories.c_str(),
							prereqs.c_str(),
							Unicode::wideToNarrow(CollectionsDataTable::localizeCollectionName((*iterSlot)->name)).c_str(),
							Unicode::wideToNarrow(CollectionsDataTable::localizeCollectionDescription((*iterSlot)->name)).c_str(),
							(*iterSlot)->icon.c_str(),
							(*iterSlot)->music.c_str(),
							title.c_str(),
							Unicode::wideToNarrow(localizedTitle).c_str(),
							CollectionsDataTable::getShowIfNotYetEarnedTypeString((*iterSlot)->showIfNotYetEarned),
							((*iterSlot)->hidden ? "yes" : "no")));

						categories.clear();
						prereqs.clear();
					}
				}
			}
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_listCollectionServerFirst))
	{
		std::map<std::string, CollectionsDataTable::CollectionInfoCollection const *> const & allServerFirstCollections = CollectionsDataTable::getAllServerFirstCollections();
		std::map<std::string, CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iter;
		int discovered = 0;

		for (iter = allServerFirstCollections.begin(); iter != allServerFirstCollections.end(); ++iter)
		{
			if (iter->second->serverFirstClaimTime > 0)
				++discovered;
		}

		int const undiscovered = allServerFirstCollections.size() - discovered;

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d Discovered:\n", discovered));
		for (iter = allServerFirstCollections.begin(); iter != allServerFirstCollections.end(); ++iter)
		{
			if (iter->second->serverFirstClaimTime > 0)
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s discovered by (%s, %s) at %ld (%s)\n", iter->second->name.c_str(), iter->second->serverFirstClaimantId.getValueString().c_str(), Unicode::wideToNarrow(iter->second->serverFirstClaimantName).c_str(), iter->second->serverFirstClaimTime, CalendarTime::convertEpochToTimeStringLocal(iter->second->serverFirstClaimTime).c_str()));
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d Undiscovered:\n", undiscovered));
		for (iter = allServerFirstCollections.begin(); iter != allServerFirstCollections.end(); ++iter)
		{
			if (iter->second->serverFirstClaimTime <= 0)
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s\n", iter->second->name.c_str()));
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_listLfgData))
	{
		std::vector<LfgDataTable::LfgNode const *> const & lfgData = LfgDataTable::getTopLevelLfgNodes();
		for (std::vector<LfgDataTable::LfgNode const *>::const_iterator iterNode = lfgData.begin(); iterNode != lfgData.end(); ++iterNode)
		{
			std::string sNodeData;
			getLfgNodeDebugText(**iterNode, sNodeData, std::string());
			result += Unicode::narrowToWide(sNodeData);
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_listGcwRegionsData))
	{
		std::map<std::string, std::map<std::string, std::pair<std::pair<float, float>, float> > > const & gcwScoreCategoryRegions = GameNetwork::getGcwScoreCategoryRegions();
		for (std::map<std::string, std::map<std::string, std::pair<std::pair<float, float>, float> > >::const_iterator iterPlanet = gcwScoreCategoryRegions.begin(); iterPlanet != gcwScoreCategoryRegions.end(); ++iterPlanet)
		{
			for (std::map<std::string, std::pair<std::pair<float, float>, float> >::const_iterator iterRegion = iterPlanet->second.begin(); iterRegion != iterPlanet->second.end(); ++iterRegion)
			{
				result += Unicode::narrowToWide(FormattedString<1024>().sprintf("(%s) (%s) (%.10g, %.10g) (%.10g)\n", iterPlanet->first.c_str(), iterRegion->first.c_str(), iterRegion->second.first.first, iterRegion->second.first.second, iterRegion->second.second));
			}

			result += Unicode::narrowToWide("\n");
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], ms_listGcwGroupsData))
	{
		std::map<std::string, std::map<std::string, int> > const & gcwScoreCategoryGroups = GameNetwork::getGcwScoreCategoryGroups();
		for (std::map<std::string, std::map<std::string, int> >::const_iterator iterGroup = gcwScoreCategoryGroups.begin(); iterGroup != gcwScoreCategoryGroups.end(); ++iterGroup)
		{
			for (std::map<std::string, int>::const_iterator iterCategory = iterGroup->second.begin(); iterCategory != iterGroup->second.end(); ++iterCategory)
			{
				result += Unicode::narrowToWide(FormattedString<1024>().sprintf("(%s) (%s) (%s) (%d / 1,000,000,000 = %.10f%%)\n", iterGroup->first.c_str(), iterCategory->first.c_str(), Unicode::wideToNarrow(StringId("gcw_regions", iterCategory->first).localize()).c_str(), iterCategory->second, (static_cast<double>(iterCategory->second) / static_cast<double>(1000000000) * static_cast<double>(100))));
			}

			result += Unicode::narrowToWide("\n");
		}
	}

	//-----------------------------------------------------------------

#if PRODUCTION == 0
	else if (isCommand (argv [0], ms_videoCaptureConfig))
	{
		if(5 <= argv.size())
		{
			int i = 1;
			const int res = atoi(Unicode::wideToNarrow(argv[i++]).c_str());
			const int sec = atoi(Unicode::wideToNarrow(argv[i++]).c_str());
			const int qual = atoi(Unicode::wideToNarrow(argv[i++]).c_str());
			const std::string filename(Unicode::wideToNarrow(argv[i++]));
			Game::videoCaptureConfig(res, sec, qual, filename.c_str());
		}
	}
#endif // PRODUCTION

	//-----------------------------------------------------------------

#if PRODUCTION == 0
	else if (isCommand (argv [0], ms_videoCaptureStart))
	{
		Game::videoCaptureStart();
	}
#endif // PRODUCTION

	//-----------------------------------------------------------------

#if PRODUCTION == 0
	else if (isCommand (argv [0], ms_videoCaptureStop))
	{
		Game::videoCaptureStop();
	}
#endif // PRODUCTION

	//-----------------------------------------------------------------

	else
	{
		result += getErrorMessage (argv [0], ERR_NO_HANDLER);
	}

	return true;
}

// ======================================================================

bool TerrainParser::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF (node);
	UNREF (originalCommand);
	UNREF (userId);

	const GroundScene* gs = dynamic_cast<const GroundScene*> (Game::getScene ());

	if (gs == 0)
	{
		result += Unicode::narrowToWide ("No groundScene available!\n");
		return true;
	}
	
	if (isCommand (argv [0], "reload"))
	{
		GameScheduler::addCallback(reloadTerrain, 0, 0.f);

		result += Unicode::narrowToWide ("Terrain reloaded.\n");
		return true;
	}

	//----------------------------------------------------------------------

	TerrainObject * const terrainObject = TerrainObject::getInstance();
	if (!terrainObject)
	{
		result += Unicode::narrowToWide ("No terrain.\n");
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_renderRunTimeRules))
	{
		ClientProceduralTerrainAppearance::setRenderRunTimeRules (argv [1][0] != '0');
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], ms_renderOverlappingRunTimeRules))
	{
		ClientProceduralTerrainAppearance::setRenderOverlappingRunTimeRules (argv [1][0] != '0');
		return true;
	}

	else if (isCommand (argv [0], ms_showPassable))
	{
		ClientProceduralTerrainAppearance::setShowPassable(argv [1][0] != '0');
		terrainObject->purgeChunks ();
		return true;
	}

	else if (isCommand (argv [0], ms_specular))
	{
		ClientProceduralTerrainAppearance::setDeferredSpecularTerrainEnabled(argv[1][0] != '0');
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], "purge"))
	{
		terrainObject->purgeChunks ();
		result += Unicode::narrowToWide ("Chunks purged.\n");

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], "lodThreshold"))
	{
		const real r = static_cast<real> (atof (Unicode::wideToNarrow (argv [1]).c_str ()));
		terrainObject->setLevelOfDetailThreshold (r);
		result += Unicode::narrowToWide ("LOD Threshold set.\n");
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], "lodHighThreshold"))
	{
		const real r = static_cast<real> (atof (Unicode::wideToNarrow (argv [1]).c_str ()));
		terrainObject->setHighLevelOfDetailThreshold (r);
		result += Unicode::narrowToWide ("LOD HighThreshold set.\n");
		return true;
	}

	return false;
}
	
// ======================================================================

bool TradeParser::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF (node);
	UNREF (originalCommand);
	UNREF (userId);

	if (Game::getSinglePlayer())
	{
		result += Unicode::narrowToWide("Cannot use trade functions in single player\n");
		return false;
	}
	if (isCommand(argv [0], "initiate"))
	{
		const NetworkId id (Unicode::wideToNarrow (argv [1]));
		ClientSecureTradeManager::initiateTrade(id, NetworkId::cms_invalid, true);
		result += Unicode::narrowToWide ("Command successful.\n");
		return true;
	}
	else if (isCommand(argv [0], "acceptRequest"))
	{
		ClientSecureTradeManager::acceptTradeRequest();
		result += Unicode::narrowToWide ("Command successful.\n");
		return true;
	}
	else if (isCommand(argv [0], "denyRequest"))
	{
		ClientSecureTradeManager::denyTradeRequest();
		result += Unicode::narrowToWide ("Command successful.\n");
		return true;
	}
	else if (isCommand(argv [0], "addItem"))
	{
		NetworkId id = NetworkId(Unicode::wideToNarrow (argv [1]));
		ClientSecureTradeManager::addItem(id);
		result += Unicode::narrowToWide ("Command successful.\n");
		return true;
	}
	else if (isCommand(argv [0], "removeItem"))
	{
		NetworkId id = NetworkId(Unicode::wideToNarrow (argv [1]));
		ClientSecureTradeManager::removeItem(id);
		result += Unicode::narrowToWide ("Command successful.\n");
		return true;
	}
	else if (isCommand(argv [0], "giveMoney"))
	{
		int amt = atoi (Unicode::wideToNarrow (argv [1]).c_str ());
		ClientSecureTradeManager::giveMoney(amt);
		result += Unicode::narrowToWide ("Command successful.\n");
		return true;
	}
	else if (isCommand(argv [0], "acceptTrade"))
	{
		ClientSecureTradeManager::acceptTrade();
		result += Unicode::narrowToWide ("Command successful.\n");
		return true;
	}
	else if (isCommand(argv [0], "unAcceptTrade"))
	{
		ClientSecureTradeManager::unacceptTrade();
		result += Unicode::narrowToWide ("Command successful.\n");
		return true;
	}
	else if (isCommand(argv [0], "verifyTrade"))
	{
		ClientSecureTradeManager::verifyTrade();
		result += Unicode::narrowToWide ("Command successful.\n");
		return true;
	}
	else if (isCommand(argv [0], "unVerifyTrade"))
	{
		ClientSecureTradeManager::denyTrade();
		result += Unicode::narrowToWide ("Command successful.\n");
		return true;
	}
	else if (isCommand(argv [0], "abortTrade"))
	{
		ClientSecureTradeManager::abortTrade();
		result += Unicode::narrowToWide ("Command successful.\n");
		return true;
	}
	return false;
}

// ======================================================================

