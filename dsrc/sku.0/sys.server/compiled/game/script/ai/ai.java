package script.ai;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.ai.ai_aggro;
import script.ai.ai_combat;
import script.ai.ai_combat_assist;
import java.lang.Math;
import script.library.ai_lib;
import script.library.attrib;
import script.library.beast_lib;
import script.library.buff;
import script.library.callable;
import script.library.camping;
import script.library.chat;
import script.library.collection;
import script.library.colors;
import script.library.corpse;
import script.library.create;
import script.library.factions;
import script.library.gcw;
import script.library.group;
import script.library.groundquests;
import script.library.instance;
import script.library.locations;
import script.library.loot;
import script.library.performance;
import script.library.permissions;
import script.library.pet_lib;
import script.library.posture;
import script.library.prose;
import script.library.resource;
import script.library.scheduled_drop;
import script.library.scenario;
import script.library.scout;
import script.library.static_item;
import script.library.stealth;
import script.library.storyteller;
import script.library.sui;
import script.library.utils;
import script.library.xp;


public class ai extends script.base_script
{
	public ai()
	{
	}
	public static final boolean LOGGING_ON = false;
	public static final String LOGGING_CATEGORY = "ai_script";
	
	public static final String MENU_FILE = "pet/pet_menu";
	
	public static final float CORPSE_CLEANUP_DELAY = 300.0f;
	
	public static final String CREATURE_TABLE = "datatables/mob/creatures.iff";
	
	public static final string_id SID_NOTIFY_STALKED = new string_id("skl_use","notify_stalked");
	public static final string_id SID_CANT_MILK = new string_id("skl_use","milk_cant");
	public static final string_id SID_MILK_NOT_HIDDEN = new string_id("skl_use","milk_not_hidden");
	public static final string_id SID_MILK_BEGIN = new string_id("skl_use","milk_begin");
	public static final string_id SID_MILK_TOO_FAR = new string_id("skl_use","milk_too_far");
	public static final string_id SID_CANT_MILK_THE_DEAD = new string_id("skl_use","milk_cant_milk_the_dead");
	public static final string_id SID_MILK_CONTINUE = new string_id("skl_use","milk_continue");
	public static final string_id SID_MILK_SUCCESS = new string_id("skl_use","milk_success");
	public static final string_id SID_BEING_MILKED = new string_id("skl_use","being_milked");
	public static final string_id SID_CANT_MILK_MOUNTED = new string_id("skl_use","milk_mounted");
	public static final string_id SID_CANT_MILK_COMBAT = new string_id("skl_use","milk_combat");
	public static final string_id SID_CANT_MILK_INV_FULL = new string_id("skl_use","milk_inventory_full");
	
	public static final string_id SID_ALREADY_RECRUITED = new string_id("collection", "already_recruited");
	public static final string_id SID_ENEMY_RECRUIT = new string_id("collection", "enemy_recruit");
	public static final string_id SID_GAVE_RECRUIT_ITEM = new string_id("collection", "gave_recruit_item");
	public static final string_id SID_NPC_MEATLUMP_SPEAK = new string_id("collection", "npc_meatlump_speak");
	public static final string_id SID_NO_RECRUIT_REB_IMP = new string_id("collection", "no_recruit_reb_imp");
	
	
	public void initializeScript() throws InterruptedException
	{
		obj_id self = getSelf();
		LOGC(aiLoggingEnabled(self), "debug_ai", "ai::initializeScript() self("+ self + ":"+ getName(self) + ")");
		
		if (hasObjVar(self, "ai.threat"))
		{
			removeObjVar(self, "ai.threat");
		}
		
		if (ai_lib.isAggro(self))
		{
			setCondition(self, CONDITION_AGGRESSIVE);
		}
		
		location loc = getLocation(self);
		boolean setHomeLoc = true;
		if (loc == null)
		{
			setHomeLoc = false;
			LOG("debug_ai", "WARNING: got null location in ai.initializeScript for creature "+ self);
		}
		else if (loc.area == null)
		{
			setHomeLoc = false;
			LOG("debug_ai", "WARNING: null scene from getLocation in ai.initializeScript for creature "+ self);
		}
		else if (loc.area.length() == 0)
		{
			setHomeLoc = false;
			LOG("debug_ai", "WARNING: empty scene from getLocation in ai.initializeScript for creature "+ self);
		}
		
		if (setHomeLoc)
		{
			aiSetHomeLocation(self, loc);
		}
		
		setWantSawAttackTriggers(self, aiIsAssist(self) || aiIsGuard(self));
		
		if (!hasObjVar(self, "ai.defaultCalmBehavior"))
		{
			setObjVar(self, "ai.defaultCalmBehavior", ai_lib.BEHAVIOR_LOITER);
		}
		
		ai_lib.resetAi();
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnAttach() self("+ self + ":"+ getName(self) + ")");
		
		initializeScript();
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAddedToWorld(obj_id self) throws InterruptedException
	{
		LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnAddedToWorld() self("+ self + ":"+ getName(self) + ")");
		
		messageTo(self, "handleOnAddedToWorldDelay", null, 2, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleOnAddedToWorldDelay(obj_id self, dictionary params) throws InterruptedException
	{
		LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnAddedToWorld() self("+ self + ":"+ getName(self) + ")");
		
		initializeScript();
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnRemovingFromWorld(obj_id self) throws InterruptedException
	{
		if (ai_lib.isAiDead(self) && !pet_lib.isPet(self) && !beast_lib.isBeast(self) && isObjectPersisted(self))
		{
			destroyObject(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnFleeTargetLost(obj_id self, obj_id oldTarget) throws InterruptedException
	{
		if (aiLoggingEnabled(self))
		{
			debugSpeakMsg(self, "OnFleeTargetLost("+ oldTarget + ")");
		}
		
		removeObjVar(self, "ai.threat");
		doDefaultCalmBehavior(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnFleePathNotFound(obj_id self, obj_id oldTarget) throws InterruptedException
	{
		if (aiLoggingEnabled(self))
		{
			debugSpeakMsg(self, "OnFleePathNotFound("+ oldTarget + ")");
		}
		
		doDefaultCalmBehavior(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnFleeWaypoint(obj_id self, obj_id target) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int resumeDefaultCalmBehavior(obj_id self, dictionary params) throws InterruptedException
	{
		if (isInNpcConversation(self) || aiIsTethered(self))
		{
			if (!hasMessageTo(self, "resumeDefaultCalmBehavior"))
			{
				messageTo(self, "resumeDefaultCalmBehavior", null, 5, false);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		LOGC(aiLoggingEnabled(self), "debug_ai", "ai::resumeDefaultCalmBehavior() self("+ self + getName(self) + ")");
		
		doDefaultCalmBehavior(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void doDefaultCalmBehavior(obj_id npc) throws InterruptedException
	{
		if (!npc.isLoaded() || !exists(npc))
		{
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::doDefaultCalmBehavior() npc(?) ISN'T LOADED!");
			return;
		}
		
		if (ai_lib.isInCombat(npc))
		{
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::doDefaultCalmBehavior() npc("+ npc + ") I THINK I AM IN COMBAT");
			return;
		}
		
		if (ai_lib.isAiDead(npc))
		{
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::doDefaultCalmBehavior() npc("+ npc + ") I THINK I AM INCAPACITATED");
			return;
		}
		
		removeObjVar(npc, "ai.threat");
		ai_lib.setMood(npc, ai_lib.MOOD_CALM);
		
		if (!hasObjVar(npc, "ai.rangedOnly"))
		{
			
		}
		
		if (ai_lib.isFollowing(npc))
		{
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::doDefaultCalmBehavior() npc("+ npc + ") FOLLOWING");
			
			obj_id target = ai_lib.getFollowTarget(npc);
			
			if (isIdValid(target) && exists(target) && !ai_lib.isAiDead(target) && isInWorld(target))
			{
				ai_lib.resumeFollow(npc);
				return;
			}
			else
			{
				removeObjVar(npc, "ai.persistantFollowing");
			}
		}
		
		if (hasObjVar(npc, "ai.inFormation"))
		{
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::doDefaultCalmBehavior() npc("+ npc + ") IN FORMATION");
			
			if (getPosture(npc) != POSTURE_UPRIGHT)
			{
				posture.stand(npc);
			}
			
			ai_lib.resumeFormationFollowing(npc);
			return;
		}
		
		if (ai_lib.canPatrol(npc))
		{
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::doDefaultCalmBehavior() npc("+ npc + ") PATROLING");
			
			if (getPosture(npc) != POSTURE_UPRIGHT)
			{
				posture.stand(npc);
			}
			
			ai_lib.resumePatrol(npc);
			return;
		}
		
		if (hasObjVar(npc, "ai.persistentPathing"))
		{
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::doDefaultCalmBehavior() npc("+ npc + ") PATHING");
			
			if (getPosture(npc) != POSTURE_UPRIGHT)
			{
				posture.stand(npc);
			}
			
			location pathLoc = getLocationObjVar(npc, "ai.persistentPathing");
			pathTo(npc, pathLoc);
			return;
		}
		
		if (hasObjVar(npc, "ai.persistentPathingWaypoint"))
		{
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::doDefaultCalmBehavior() npc("+ npc + ") ai.persistentPathingWaypoint");
			
			if (getPosture(npc) != POSTURE_UPRIGHT)
			{
				posture.stand(npc);
			}
			
			String pathWaypoint = getStringObjVar(npc, "ai.persistentPathingWaypoint");
			pathTo(npc, pathWaypoint);
			return;
		}
		
		if (getConfigSetting("GameServer", "disableAILoitering")!=null)
		{
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::doDefaultCalmBehavior() npc("+ npc + ") disableAILoitering");
			return;
		}
		
		final int defaultBehavior = ai_lib.getDefaultCalmBehavior(npc);
		
		switch (defaultBehavior)
		{
			case ai_lib.BEHAVIOR_STOP:
			case ai_lib.BEHAVIOR_SENTINEL:
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::doDefaultCalmBehavior() npc("+ npc + getName(npc) + ") BEHAVIOR_STOP");
			stop(npc);
			
			break;
			
			default:
			case ai_lib.BEHAVIOR_LOITER:
			case ai_lib.BEHAVIOR_WANDER:
			if (pet_lib.isPet(npc) || beast_lib.isBeast(npc))
			{
				aiSetHomeLocation(npc, getLocation(npc));
			}
			
			final float collisionRadius = getObjectCollisionRadius(npc);
			float minDistance = 0.0f;
			float maxDistance = 0.0f;
			
			if (hasObjVar(npc, "ai.loiterMinRange"))
			{
				minDistance = getFloatObjVar(npc, "ai.loiterMinRange");
				maxDistance = getFloatObjVar(npc, "ai.loiterMaxRange");
			}
			else
			{
				
				final float cellDistanceModifier = !isIdValid(getLocation(npc).cell) ? 3.0f : 1.0f;
				
				maxDistance = Math.min(128.0f, collisionRadius * 7.0f * cellDistanceModifier);
				minDistance = 0.0f;
			}
			
			final float minDelay = Math.min(Math.max(8.0f, collisionRadius * 2.0f), 16.0f);
			final float maxDelay = Math.min(Math.max(8.0f, collisionRadius * 4.0f), 16.0f);
			
			loiterLocation(npc, aiGetHomeLocation(npc), minDistance, maxDistance, minDelay, maxDelay);
			
			break;
		}
	}
	
	
	public int OnFollowMoving(obj_id self, obj_id target) throws InterruptedException
	{
		if (ai_lib.isInCombat(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isAiDead(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (getPosture(self) != POSTURE_UPRIGHT)
		{
			stop(self);
			removeObjVar(self, "ai.combat.moveMode");
			posture.stand(self);
			
			messageTo(self, "resumeDefaultCalmBehavior", null, 2, false);
			
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnFollowTargetLost(obj_id self, obj_id oldTarget) throws InterruptedException
	{
		LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnFollowTargetLost() self("+ self + ":"+ getName(self) + ")");
		
		if (!ai_lib.isInCombat(self))
		{
			ai_lib.aiStopFollowing(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnExitedCombat(obj_id self) throws InterruptedException
	{
		if (utils.hasScriptVar(self, "sentAllyDistressCall"))
		{
			utils.removeScriptVar(self, "sentAllyDistressCall");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnFollowWaiting(obj_id self, obj_id target) throws InterruptedException
	{
		if (!target.isLoaded() || !exists(target))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(target, "gm"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isInCombat(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(target) || ai_lib.isAiDead(target))
		{
			ai_lib.aiStopFollowing(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnFollowPathNotFound(obj_id self, obj_id target) throws InterruptedException
	{
		LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnFollowPathNotFound() self("+ self + ":"+ getName(self) + ")");
		
		if (!ai_lib.isInCombat(self))
		{
			ai_lib.aiStopFollowing(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnMovePathComplete(obj_id self) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnMovePathComplete() self("+ self + ")");
		
		if (hasObjVar(self, "ai.persistentPathing"))
		{
			location pathLoc = getLocationObjVar(self, "ai.persistentPathing");
			if (utils.getDistance(self, pathLoc) < 2.0f)
			{
				removeObjVar(self, "ai.persistentPathing");
			}
		}
		
		if (hasObjVar(self, "ai.persistentWaypoint"))
		{
			removeObjVar(self, "ai.persistentPathingWaypoint");
		}
		
		if (!ai_lib.isInCombat(self))
		{
			
			if (getBehavior(self) <= BEHAVIOR_CALM)
			{
				ai_lib.resumePatrol(self);
			}
			
			if (utils.hasScriptVar(self, "pathingToAttack"))
			{
				obj_id target = utils.getObjIdScriptVar(self, "pathingToAttack");
				
				if (isIdValid(target))
				{
					
					startCombat(self, target);
				}
				
				utils.removeScriptVar(self, "pathingToAttack");
			}
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnMovePathNotFound(obj_id self) throws InterruptedException
	{
		LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnMovePathNotFound() self("+ self + ":"+ getName(self) + ")");
		
		if (utils.hasScriptVar(self, "pathingToAttack"))
		{
			utils.removeScriptVar(self, "pathingToAttack");
		}
		
		stop(self);
		
		if (getBehavior(self) <= BEHAVIOR_CALM && !ai_lib.isAiDead(self))
		{
			setLocation(self, aiGetHomeLocation(self));
			messageTo(self, "resumeDefaultCalmBehavior", null, 5, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTriggerVolumeEntered(obj_id self, String volumeName, obj_id breacher) throws InterruptedException
	{
		
		if (utils.hasScriptVar(self, "petBeingInitialized"))
		{
			LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") self("+ self + ":"+ getName(self) + ") PET BEING INITIALIZED");
			return SCRIPT_CONTINUE;
		}
		
		if (!isMob(breacher))
		{
			LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") SCRIPT_CONTINUE - The breacher is not a mob");
			return SCRIPT_CONTINUE;
		}
		
		if (breacher == self)
		{
			LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") SCRIPT_CONTINUE - The breacher == self");
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isAiDead(self))
		{
			LOGC(aiLoggingEnabled(self), "debug_ai", "WARNING: ai::OnTriggerVolumeEntered("+ volumeName + ") self("+ self + ":"+ getName(self) + ") I AM DEAD");
			return SCRIPT_CONTINUE;
		}
		
		if (beast_lib.isBeast(self) && (!beast_lib.getBeastDefensive(self) || !hasCommand(getMaster(self), beast_lib.BM_COMMAND_ATTACK)))
		{
			return SCRIPT_CONTINUE;
		}
		
		LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") self("+ self + ":"+ getName(self) + ") radius("+ getTriggerVolumeRadius(self, volumeName) + ") breacher("+ breacher + ":"+ getName(breacher) + ")");
		
		final boolean breacherIsAPlayer = isPlayer(breacher);
		
		if (volumeName.equals(ai_lib.ALERT_VOLUME_NAME))
		{
			if (isPlayer(breacher) && !ai_lib.isMonster(self))
			{
				ai_lib.greet(self, breacher);
			}
			
			if (hasTriggerVolume(self, ai_lib.AGGRO_VOLUME_NAME))
			{
				boolean aggro = false;
				
				if (pvpIsEnemy(self, breacher))
				{
					LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") Breacher is a threat (pvp enemy)");
					aggro = true;
				}
				else
				{
					if (isPlayer(breacher))
					{
						
						final int factionStatus = factions.getFactionStatus(self, breacher);
						
						if (factionStatus == factions.STATUS_ENEMY)
						{
							LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") Player breacher is a threat (faction enemy)");
							aggro = true;
						}
						
						else if ((factionStatus == factions.STATUS_NEUTRAL) && aiIsAggressive(self))
						{
							LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") Player breacher is a threat (faction neutral, I am aggressive)");
							aggro = true;
						}
						else if (pvpGetAlignedFaction(self) != 0)
						{
							LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") Player breacher is a NOT currently a threat (but could be with faction alignment change)");
							aggro = true;
						}
						else
						{
							LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") Player breacher is a NOT a threat");
						}
					}
					else
					{
						if (aiIsGuard(self))
						{
							
							final String breacherFaction = factions.getFaction(breacher);
							
							if (( (breacherFaction != null) && (breacherFaction.equals("townsperson") || breacherFaction.equals("ImperialCitizen") || breacherFaction.equals("RebelCitizen"))))
							{
								LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") guard: AI breacher is a NOT a threat (townsperson)");
							}
							else
							{
								if (ai_lib.isSameSocialGroup(self, breacher))
								{
									LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") guard: AI breacher is NOT a threat (same social group)");
								}
								else if (pet_lib.isPet(breacher) || beast_lib.isBeast(breacher))
								{
									LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") guard: AI breacher is NOT a threat (pet)");
								}
								else
								{
									if (ai_lib.isAnimal(breacher))
									{
										LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") guard: AI breacher is a threat (animal)");
										aggro = true;
									}
									else if (factions.getFactionStatus(self, breacher) == factions.STATUS_ENEMY)
									{
										LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") guard: AI breacher is a threat (factional enemy)");
										aggro = true;
									}
									else if (hasAttributeAttained(breacher, attrib.THUG))
									{
										LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") guard: AI breacher is a threat (thug)");
										aggro = true;
									}
									else
									{
										LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") guard: AI breacher is NOT a threat");
									}
								}
							}
						}
						else
						{
							
							if (ai_lib.isSameSocialGroup(self, breacher))
							{
								LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") AI breacher is NOT a threat (same social group)");
							}
							else
							{
								final int factionStatus = factions.getFactionStatus(self, breacher);
								
								if (factionStatus == factions.STATUS_ENEMY)
								{
									LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") AI breacher is a threat (factional enemy)");
									aggro = true;
								}
								else
								{
									LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") AI breacher is NOT a threat");
								}
							}
						}
					}
				}
				
				if (hasObjVar(self, "storytellerid"))
				{
					if (!utils.hasScriptVar(breacher, "storytellerid"))
					{
						LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") Player breacher is a NOT a member of my Story");
						aggro = false;
					}
					else
					{
						obj_id selfStoryTellerId = getObjIdObjVar(self, "storytellerid");
						obj_id breacherStoryTellerId = utils.getObjIdScriptVar(breacher, "storytellerid");
						if (selfStoryTellerId != breacherStoryTellerId)
						{
							LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeEntered("+ volumeName + ") Player breacher is a NOT a member of my Story (but he is a member of another Story)");
							aggro = false;
						}
					}
				}
				
				if (aggro)
				{
					addTriggerVolumeEventSource(ai_lib.AGGRO_VOLUME_NAME, breacher);
					
				}
			}
		}
		else if (volumeName.equals(ai_lib.AGGRO_VOLUME_NAME))
		{
			if (!isPlayer(breacher))
			{
				ai_aggro.requestAggroCheck(breacher);
			}
			else
			{
				if (scout.isScentMasked(breacher, self))
				{
					
				}
				else
				{
					if (isIncapacitated(breacher))
					{
						LOGC(aiLoggingEnabled(self), "debug_ai", "ai::requestSawRecapacitation() self("+ self + ":"+ getName(self) + ") breacher("+ breacher + ":"+ getName(breacher) + ") Listening for breacher recapacitation");
						
						listenToMessage(breacher, "handleSawRecapacitation");
					}
					
					ai_aggro.requestAggroCheck(breacher);
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTriggerVolumeExited(obj_id self, String volumeName, obj_id breacher) throws InterruptedException
	{
		LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnTriggerVolumeExited("+ volumeName + ") self("+ self + ":"+ getName(self) + ") radius("+ getTriggerVolumeRadius(self, volumeName) + ") breacher("+ breacher + ":"+ getName(breacher) + ")");
		
		if (utils.hasScriptVar(self, "petBeingInitialized"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (volumeName.equals(ai_lib.ALERT_VOLUME_NAME))
		{
			
			if (( isPlayer(breacher) && !ai_lib.isMonster(self)))
			{
				ai_lib.dismiss(self, breacher);
			}
			
			removeTriggerVolumeEventSource(ai_lib.AGGRO_VOLUME_NAME, breacher);
			
			if (hasScript(self, "systems.missions.dynamic.mission_bounty_target"))
			{
				obj_id objHunter = getObjIdObjVar(self, "objHunter");
				if (!isIdValid(objHunter) || objHunter != breacher)
				{
					pvpRemovePersonalEnemyFlags(self, breacher);
				}
			}
			else
			{
				pvpRemovePersonalEnemyFlags(self, breacher);
			}
		}
		else if (volumeName.equals(ai_lib.AGGRO_VOLUME_NAME))
		{
			if (isPlayer(breacher))
			{
				LOGC(aiLoggingEnabled(self), "debug_ai", "ai::requestSawRecapacitation() self("+ self + ":"+ getName(self) + ") breacher("+ breacher + ":"+ getName(breacher) + ") Stop listening for breacher recapacitation");
				
				stopListeningToMessage(breacher, "handleSawRecapacitation");
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnLoiterWaiting(obj_id self, modifiable_float time) throws InterruptedException
	{
		
		if (ai_lib.isHerdingCreature(self))
		{
			ai_lib.doAction(self, "eat");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean isThreat(obj_id npc, obj_id breacher) throws InterruptedException
	{
		if (hasObjVar(breacher, "gm"))
		{
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::isThreat() npc("+ npc + ") breacher("+ breacher + ") breacher is a gm");
			return false;
		}
		
		if (!isValidId(breacher))
		{
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::isThreat() npc("+ npc + ") breacher("+ breacher + ") invalid breacher id");
			return false;
		}
		
		if (!isMob(breacher))
		{
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::isThreat() npc("+ npc + ") breacher("+ breacher + ") breacher is not a mob");
			return false;
		}
		
		if (ai_lib.isAiDead(npc))
		{
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::isThreat() npc("+ npc + ") breacher("+ breacher + ") npc is dead");
			return false;
		}
		
		if (!isPlayer(breacher) && ai_lib.isAiDead(breacher))
		{
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::isThreat() npc("+ npc + ") breacher("+ breacher + ") breacher is dead ai");
			return false;
		}
		
		if (isInvulnerable(npc))
		{
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::isThreat() npc("+ npc + ") breacher("+ breacher + ") npc is invulnerable");
			return false;
		}
		
		if (isInvulnerable(breacher))
		{
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::isThreat() npc("+ npc + ") breacher("+ breacher + ") breacher is invulnerable");
			return false;
		}
		
		if (!pvpCanAttack(npc, breacher))
		{
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::isThreat() !pvpCanAttack - npc("+ npc + ":"+ getName(npc) + ") npcPvpType("+ pvpGetType(npc) + ") npcPvpAlignedFaction("+ pvpGetAlignedFaction(npc) + ") breacher("+ breacher + ":"+ getName(breacher) + ") breacherPvpType("+ pvpGetType(breacher) + ") breacherPvpAlignedFaction("+ pvpGetAlignedFaction(breacher) + ")");
			return false;
		}
		
		obj_id master = getMaster(breacher);
		
		if (isIdValid(master) && exists(master))
		{
			LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::isThreat() npc("+ npc + ") breacher("+ breacher + ") breacher is a pet/beast with a master");
			return false;
		}
		
		return (ai_lib.isAggroToward(npc, breacher) || ai_lib.isAggroToward(breacher, npc));
	}
	
	
	public void goAlert(obj_id npc) throws InterruptedException
	{
		
		if (ai_lib.isAiDead(npc))
		{
			return;
		}
		
		if (ai_lib.isInCombat(npc))
		{
			return;
		}
		
		if (!hasObjVar(npc, "ai.pathingAwayFrom"))
		{
			stop(npc);
		}
		
		ai_lib.doAction(npc, ai_lib.ACTION_ALERT);
		ai_lib.setMood(npc, ai_lib.MOOD_NERVOUS);
		
		obj_id threat = getObjIdObjVar(npc, "ai.threat");
		
		if (isIdValid(threat))
		{
			debugSpeakMsg(npc, "FEAR 7");
			addToMentalStateToward(npc, threat, FEAR, -5.0f);
		}
		
	}
	
	
	public void goThreaten(obj_id npc) throws InterruptedException
	{
		
		if (ai_lib.isAiDead(npc))
		{
			return;
		}
		
		if (ai_lib.isInCombat(npc))
		{
			return;
		}
		
		obj_id threat = getObjIdObjVar(npc, "ai.threat");
		
		if (!isIdValid(threat))
		{
			return;
		}
		
		if (hasObjVar(threat, "gm"))
		{
			return;
		}
		
		stop(npc);
		faceTo(npc, threat);
		ai_lib.doThreatenAnimation(npc);
		debugSpeakMsg(npc, "FEAR 8");
		addToMentalStateToward(npc, threat, FEAR, -5.0f);
		
		if (!isPlayer(threat))
		{
			debugSpeakMsg(threat, "FEAR 9");
			addToMentalStateToward(threat, npc, FEAR, 10.0f);
		}
		
		ai_lib.setMood(npc, ai_lib.MOOD_NERVOUS);
	}
	
	
	public int OnBehaviorChange(obj_id self, int newBehavior, int oldBehavior, int[] changeFlags) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSetDefaultBehavior(obj_id self, dictionary params) throws InterruptedException
	{
		doDefaultCalmBehavior(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		int idx = utils.getValidAttributeIndex(names);
		
		if (idx == -1)
		{
			blog("idx was negative 1");
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		String milkType = ai_lib.getMilkType(self);
		
		if (milkType != null && ai_lib.isMonster(self) && !beast_lib.isBeast(self))
		{
			names[idx] = "got_milk";
			string_id strIdMilkType = new string_id("resource/resource_names", milkType);
			attribs[idx] = utils.packStringId(strIdMilkType);
			idx++;
		}
		
		final String creatureName = getCreatureName(self);
		
		if (creatureName == null)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		dictionary creatureKnowledge = beast_lib.getBeastMasterExamineInfo(player, self, names, attribs, idx);
		names = creatureKnowledge.getStringArray("names");
		attribs = creatureKnowledge.getStringArray("attribs");
		idx = creatureKnowledge.getInt("idx");
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public int snareEffectOff(obj_id self, dictionary params) throws InterruptedException
	{
		
		string_id strFlyText = new string_id("combat_effects", "no_snare");
		color colFlyText = colors.TOMATO;
		showFlyText(self, strFlyText, 1.0f, colFlyText);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDefenderCombatAction(obj_id self, obj_id attacker, obj_id weapon, int combatResult) throws InterruptedException
	{
		if (ai_lib.isAiDead(self) || isInvulnerable(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (combatResult == COMBAT_RESULT_HIT)
		{
			chat.setAngryMood(self);
			ai_lib.barkString(self, "hit");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnSawAttack(obj_id self, obj_id defender, obj_id[] attackers) throws InterruptedException
	{
		LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnSawAttack() self("+ self + ":"+ getName(self) + ") defender ("+ defender + ") attackers.length("+ attackers.length + ")");
		
		if (beast_lib.isBeast(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (defendLair(self, defender, attackers, false))
		{
			if (hasScript(self, "npc.converse.npc_convo"))
			{
				if (!isInNpcConversation(self) && !pet_lib.isPet(self) && !beast_lib.isBeast(self))
				{
					detachScript(self, "npc.converse.npc_convo");
					clearCondition(self, CONDITION_CONVERSABLE);
				}
			}
			
			debugSpeakMsgc(aiLoggingEnabled(self), self, "lair defense");
		}
		else
		{
			if (pet_lib.isPet(self) && pet_lib.isGuarding(self, defender))
			{
				for (int i = 0; i < attackers.length; ++i)
				{
					testAbortScript();
					final obj_id attacker = attackers[i];
					
					if (isIdValid(attacker))
					{
						startCombat(self, attacker);
						
						if (!ai_lib.isInCombat(self) && (defender == getMaster(self)))
						{
							ai_lib.barkString(self, "ally");
						}
					}
				}
			}
			else if (aiIsGuard(self))
			{
				boolean assistDefender = false;
				final String defenderFaction = factions.getFaction(defender);
				
				if (defenderFaction != null)
				{
					if (defenderFaction.equals("townsperson") || defenderFaction.equals("ImperialCitizen") || defenderFaction.equals("RebelCitizen"))
					{
						assistDefender = true;
					}
				}
				
				if (!assistDefender && ai_lib.isSameSocialGroup(self, defender))
				{
					assistDefender = true;
				}
				
				for (int i = 0; i < attackers.length; ++i)
				{
					testAbortScript();
					final obj_id attacker = attackers[i];
					
					if (isIdValid(attacker))
					{
						
						if ((assistDefender || hasAttributeAttained(attacker, attrib.THUG) || (ai_lib.isMonster(attacker) && (!pet_lib.isPet(attacker)))))
						{
							debugSpeakMsgc(aiLoggingEnabled(self), self, "OnSawAttackGuard() attacking("+ attacker + ")");
							
							ai_combat_assist.assist(self, attacker);
							break;
						}
						else if (ai_lib.isSameSocialGroup(self, attacker))
						{
							debugSpeakMsgc(aiLoggingEnabled(self), self, "OnSawAttack() attacking("+ defender + ")");
							ai_combat_assist.assist(self, defender);
							break;
						}
					}
				}
			}
			else
			{
				
				for (int i = 0; i < attackers.length; ++i)
				{
					testAbortScript();
					final obj_id attacker = attackers[i];
					
					if (isIdValid(attacker))
					{
						if (hasObjVar(self, "storytellerid"))
						{
							if (!utils.hasScriptVar(defender, "storytellerid") && !utils.hasScriptVar(attacker, "storytellerid"))
							{
								debugSpeakMsgc(aiLoggingEnabled(self), self, "OnSawAttack() not attacking("+ defender + ") or ("+ attacker + ") as they are not a part of my story");
								break;
							}
							else if (hasObjVar(defender, "storytellerid") && utils.hasScriptVar(attacker, "storytellerid"))
							{
								obj_id attackerStorytellerId = utils.getObjIdScriptVar(attacker, "storytellerid");
								obj_id defenderStorytellerId = getObjIdObjVar(defender, "storytellerid");
								obj_id myStorytellerId = getObjIdObjVar(self, "storytellerid");
								
								if (myStorytellerId == defenderStorytellerId && ai_lib.isSameSocialGroup(self, defender))
								{
									debugSpeakMsgc(aiLoggingEnabled(self), self, "OnSawAttack() attacking("+ attacker + ")");
									ai_combat_assist.assist(self, defender);
									break;
								}
								
							}
							else if (utils.hasScriptVar(defender, "storytellerid") && hasObjVar(attacker, "storytellerid"))
							{
								obj_id attackerStorytellerId = getObjIdObjVar(attacker, "storytellerid");
								obj_id defenderStorytellerId = utils.getObjIdScriptVar(defender, "storytellerid");
								obj_id myStorytellerId = getObjIdObjVar(self, "storytellerid");
								
								if (myStorytellerId == attackerStorytellerId && ai_lib.isSameSocialGroup(self, attacker))
								{
									debugSpeakMsgc(aiLoggingEnabled(self), self, "OnSawAttack() attacking("+ defender + ")");
									ai_combat_assist.assist(self, attacker);
									break;
								}
							}
						}
						
						if (ai_lib.isSameSocialGroup(self, defender))
						{
							debugSpeakMsgc(aiLoggingEnabled(self), self, "OnSawAttack() attacking("+ attacker + ")");
							ai_combat_assist.assist(self, attacker);
							break;
						}
						else if (ai_lib.isSameSocialGroup(self, attacker))
						{
							debugSpeakMsgc(aiLoggingEnabled(self), self, "OnSawAttack() attacking("+ defender + ")");
							ai_combat_assist.assist(self, defender);
							break;
						}
					}
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean defendLair(obj_id npc, obj_id defender, obj_id[] attackers, boolean isInCombat) throws InterruptedException
	{
		
		if (isMob(defender))
		{
			return false;
		}
		
		if (utils.hasScriptVar(npc, "ai.noLair"))
		{
			return false;
		}
		
		obj_id myPOI = ai_lib.getLair(npc);
		
		if (!isIdValid(myPOI))
		{
			return false;
		}
		
		if (!hasObjVar(defender, "poi.baseObject"))
		{
			return false;
		}
		
		if (getObjIdObjVar(defender, "poi.baseObject") == myPOI)
		{
			
			int curHP = getHitpoints(defender);
			int maxHP = getMaxHitpoints(defender);
			
			if (curHP >= maxHP)
			{
				return true;
				
			}
			
			if (pvpGetAlignedFaction(npc) == (221551254))
			{
				factions.setFaction(npc, "angry");
			}
			
			int endLoop = attackers.length;
			
			if (endLoop > 20)
			{
				endLoop = 20;
			}
			
			for (int i = 0; i < endLoop; i++)
			{
				testAbortScript();
				if (isIdValid(attackers[i]))
				{
					if (!isInCombat)
					{
						
						startCombat(npc, attackers[i]);
						
						if (utils.hasScriptVar(npc, "pathingToAttack"))
						{
							utils.removeScriptVar(npc, "pathingToAttack");
						}
						
						return true;
					}
				}
			}
			return true;
		}
		
		return false;
	}
	
	
	public int stopFleeing(obj_id self, dictionary params) throws InterruptedException
	{
		debugSpeakMsg(self, "stopping fleeing");
		
		if (!ai_lib.isInCombat(self))
		{
			if (getDistance(getLocation(self), aiGetHomeLocation(self)) > 40.0f)
			{
				pathTo(self, aiGetHomeLocation(self));
			}
		}
		
		if (getBehavior(self) <= BEHAVIOR_CALM)
		{
			messageTo(self, "resumeDefaultCalmBehavior", null, 5, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnIncapacitated(obj_id self, obj_id attacker) throws InterruptedException
	{
		LOGC(aiLoggingEnabled(self), "debug_ai", ("ai::OnIncapacitated() self("+ self + ":"+ getName(self) + ") attacker("+ attacker + ")"));
		
		debugSpeakMsgc(aiLoggingEnabled(self), self, "OnIncapacitated");
		
		chat.setBadMood(self);
		ai_lib.barkString(self, "death");
		
		if (!pet_lib.isPet(self) && !beast_lib.isBeast(self) && !utils.hasScriptVar(self, "petBeingInitialized"))
		{
			
			obj_id weapon = getCurrentWeapon(self);
			
			if (isIdValid(weapon) && !isDefaultWeapon(weapon))
			{
				destroyObject(weapon);
			}
			
			obj_id cInv = utils.getInventoryContainer(self);
			
			if (isIdValid(cInv))
			{
				utils.emptyContainerExceptStorytellerLoot(cInv);
			}
			
			ai_lib.clearCombatData();
			
			setObjVar(self, xp.VAR_LANDED_DEATHBLOW, attacker);
			
			if (!kill(self))
			{
				obj_id[] haters = getHateList(self);
				
				if (haters != null || haters.length > 0)
				{
					for (int i=0; i<haters.length; i++)
					{
						testAbortScript();
						removeHateTarget(haters[i], self);
					}
				}
				
				destroyObject(self);
			}
			
			clearCondition(self, CONDITION_INTERESTING);
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDeath(obj_id self, obj_id killer, obj_id corpseId) throws InterruptedException
	{
		if (isIdValid(killer) && exists(killer) && isIdValid(self) && exists(self))
		{
			removeHateTarget(killer, self);
		}
		
		obj_id player = utils.getObjIdScriptVar(self, "ai.listeningTo");
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, gcw.GCW_ENTERTAINMENT_FLAG) && utils.hasScriptVar(self, gcw.GCW_SCRIPTVAR_PARENT + ".isGcwEntertained"))
		{
			LOG("gcw_entertainer","NPC is saving time player stopped entertaining");
			utils.setScriptVar(self, gcw.GCW_STOPPED_ENTERTAINING, getGameTime());
			
			int pid = sui.getPid(player, gcw.ENTERTAIN_GCW_TROOPS_PID);
			if (pid > 0)
			{
				forceCloseSUIPage(pid);
			}
			
			if (hasObjVar(self, gcw.GCW_PATROL_OBJ))
			{
				obj_id patrolObject = getObjIdObjVar(self, gcw.GCW_PATROL_OBJ);
				if (isValidId(patrolObject))
				{
					dictionary webster = new dictionary();
					webster.put("npc",self);
					webster.put("player",player);
					messageTo(patrolObject, "handleOffensiveEntertainedNpcCleanUp", webster, 1, false);
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int xpDelegated(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id cInv = utils.getInventoryContainer(self);
		
		if (isIdValid(cInv))
		{
			setOwner(cInv, obj_id.NULL_ID);
		}
		
		if (!beast_lib.isBeast(self))
		{
			attachScript(self, corpse.SCRIPT_AI_CORPSE);
			
			messageTo(self, "aiCorpsePrepared", null, 0, false);
		}
		else
		{
			xp.cleanupCreditForKills();
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int aiCorpsePrepared(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id[] killers = getObjIdArrayObjVar(self, scenario.VAR_PRIMARY_KILLERS);
		
		obj_id[] pks = null;
		
		if (killers != null && killers.length > 0)
		{
			pks = xp.getPlayersFromKillerList(killers);
		}
		
		if (pks == null || pks.length <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		boolean hasLoot = loot.addLoot(self);
		
		int corpseLevel = getLevel(self);
		boolean doNotDropCard = false;
		int difficultyClass = getIntObjVar(self, "difficultyClass");
		int sourceSystem = scheduled_drop.SYSTEM_COMBAT_NORMAL;
		
		switch(difficultyClass)
		{
			
			case 0:
			sourceSystem = scheduled_drop.SYSTEM_COMBAT_NORMAL;
			break;
			case 1:
			sourceSystem = scheduled_drop.SYSTEM_COMBAT_ELITE;
			break;
			case 2:
			sourceSystem = scheduled_drop.SYSTEM_COMBAT_BOSS;
			break;
		}
		
		int delayCount = 0;
		
		for (int i = 0, j = pks.length; i < j; i++)
		{
			testAbortScript();
			if (isIdValid(pks[i]) && (corpseLevel + 5 < getLevel(pks[i]) || utils.isFreeTrial(pks[i])))
			{
				doNotDropCard = true;
			}
			
			if (isIdValid(pks[i]) && scheduled_drop.hasCardDelay(pks[i], sourceSystem))
			{
				delayCount++;
			}
		}
		
		if (delayCount > pks.length / 2)
		{
			doNotDropCard = true;
		}
		
		boolean canDrop = scheduled_drop.canDropCard(sourceSystem);
		
		for (int i = 0, j = pks.length; i < j; i++)
		{
			testAbortScript();
			if (isIdValid(pks[i]))
			{
				utils.setScriptVar(pks[i], scheduled_drop.PLAYER_SCRIPTVAR_DROP_TIME, getGameTime());
				
				if (isGod(pks[i]) && hasObjVar(pks[i], "qa_tcg_always_drop"))
				{
					if (!doNotDropCard)
					{
						canDrop = true;
					}
				}
				
				if (isGod(pks[i]) && hasObjVar(pks[i], "qa_tcg"))
				{
					sendSystemMessageTestingOnly(pks[i], "QA TCG COMBAT. Do not drop card? "+ doNotDropCard + " hasCardDelay? "+ scheduled_drop.hasCardDelay(pks[i], sourceSystem) + " isTrial? "+ utils.isFreeTrial(pks[i]) + " bad level? "+ (corpseLevel + 5 < getLevel(pks[i])) );
				}
			}
		}
		
		if (!doNotDropCard)
		{
			obj_id inv = utils.getInventoryContainer(self);
			
			if (isIdValid(inv) && canDrop)
			{
				scheduled_drop.dropCard(sourceSystem, inv);
				hasLoot = true;
			}
		}
		
		if (hasLoot)
		{
			corpse.showLootMeParticle(self);
		}
		
		setObjVar(self, "readyToLoot", true);
		
		xp.cleanupCreditForKills();
		return SCRIPT_CONTINUE;
	}
	
	
	public int corpseCleanup(obj_id self, dictionary params) throws InterruptedException
	{
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnStartNpcConversation(obj_id self, obj_id speaker) throws InterruptedException
	{
		LOGC(aiLoggingEnabled(self), "debug_ai", ("ai::OnStartNpcConversation() self("+ self + ":"+ getName(self) + ") speaker("+ speaker + ":"+ getName(speaker) + ")"));
		
		if (!isInNpcConversation(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isAiDead(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (ai_lib.isInCombat(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isInCombat(speaker))
		{
			return SCRIPT_CONTINUE;
		}
		
		ai_lib.setMood(self, ai_lib.MOOD_CALM);
		stop(self);
		faceTo(self, speaker);
		
		messageTo(self, "handleAbortConversation", null, 30, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleAbortConversation(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isInNpcConversation(self))
		{
			if (getBehavior(self) <= BEHAVIOR_CALM)
			{
				messageTo(self, "resumeDefaultCalmBehavior", null, 5, false);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnEndNpcConversation(obj_id self, obj_id speaker) throws InterruptedException
	{
		LOGC(aiLoggingEnabled(self), "debug_ai", ("ai::OnEndNpcConversation() self("+ self + ":"+ getName(self) + ") speaker("+ speaker + ":"+ getName(speaker) + ")"));
		
		if (ai_lib.isAiDead(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (getBehavior(self) <= BEHAVIOR_CALM)
		{
			messageTo(self, "resumeDefaultCalmBehavior", null, 5, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int disableAI(obj_id self, dictionary params) throws InterruptedException
	{
		String[] scriptList = getScriptList(self);
		
		for (int i = 0; i < scriptList.length; i++)
		{
			testAbortScript();
			if (scriptList[i].startsWith("ai."))
			{
				detachScript(self, scriptList[i]);
			}
		}
		
		stop(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleClearPathingFlag(obj_id self, dictionary params) throws InterruptedException
	{
		removeObjVar(self, "ai.pathingAwayFrom");
		
		if (getBehavior(self) <= BEHAVIOR_CALM)
		{
			messageTo(self, "resumeDefaultCalmBehavior", null, 5, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleAnimationAction(obj_id self, dictionary params) throws InterruptedException
	{
		
		String anim = params.getString("anim");
		
		if (anim != null)
		{
			
			ai_lib.doAction(self, anim);
		}
		else
		{
			
			String animMood = params.getString("animMood");
			
			if (animMood != null)
			{
				ai_lib.setMood(self, animMood);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleAddMaster(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id pet = params.getObjId("pet");
		obj_id controlDevice = params.getObjId("controlDevice");
		
		if (!isIdValid(controlDevice))
		{
			controlDevice = callable.getCallableCD(pet);
		}
		
		if (!isIdValid(controlDevice))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(pet))
		{
			return SCRIPT_CONTINUE;
		}
		
		if ((pet != self))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id master = params.getObjId("master");
		
		if (!isIdValid(master))
		{
			return SCRIPT_CONTINUE;
		}
		
		int petType = pet_lib.getPetType(self);
		
		if (pet_lib.hasMaxPets(master, petType, pet) && callable.hasCallableCD(self))
		{
			destroyObject(self);
			
			return SCRIPT_CONTINUE;
		}
		if (!hasScript(self, "ai.pet"))
		{
			setMaster(self, master);
			setOwner(utils.getInventoryContainer(self), master);
			attachScript(self, "ai.pet");
			setObjVar(pet, "ai.pet.masterName", getEncodedName(master));
			
			if (utils.hasScriptVar(self, "petBeingInitialized"))
			{
				utils.removeScriptVar(self, "petBeingInitialized");
			}
		}
		
		if (!ai_lib.isMonster(self))
		{
			pet_lib.setupDefaultCommands(self);
		}
		
		pet_lib.addToPetList(master, self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleRemoveMaster(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id pet = params.getObjId("pet");
		
		if (pet != self || !isIdValid(pet))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!pet_lib.hasMaster(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id master = getMaster(self);
		params.put("master", master);
		
		final String creatureName = getCreatureName(self);
		
		if (creatureName != null)
		{
			params.put("ai.creatureName", creatureName);
		}
		
		obj_id group = getGroupObject(pet);
		queueCommand(pet, (1188179258), group, "", COMMAND_PRIORITY_DEFAULT);
		
		removeObjVar(pet, "ai.pet.masterName");
		
		ai_lib.aiStopFollowing(pet);
		stop(pet);
		
		setMaster(pet, null);
		setOwner(utils.getInventoryContainer(self), obj_id.NULL_ID);
		
		removeTriggerVolume(ai_lib.ALERT_VOLUME_NAME);
		
		removeObjVar(pet, "ai.pet.friendList");
		removeObjVar(pet, "ai.pet.guarding");
		removeObjVar(pet, "ai.inFormation");
		
		pet_lib.removeFromPetList(pet);
		
		if (hasScript(pet, "ai.pet"))
		{
			detachScript(pet, "ai.pet");
		}
		
		messageTo(pet, "destroyAbandonedPet", null, 300, true);
		
		ai_lib.setDefaultCalmBehavior(pet, ai_lib.BEHAVIOR_LOITER);
		
		if (ai_lib.isMonster(pet))
		{
			messageTo(pet, "handleWailingPet", null, 5, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleWailingPet(obj_id self, dictionary params) throws InterruptedException
	{
		if (pet_lib.hasMaster(self) || ai_lib.isAiDead(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		switch(rand(1,3))
		{
			case 1 :
			stop(self);
			ai_lib.doAction(self, "ashamed");
			break;
			case 2 :
			stop(self);
			ai_lib.doAction(self, "vocalize");
			break;
			case 3 :
			if (getBehavior(self) <= BEHAVIOR_CALM)
			{
				doDefaultCalmBehavior(self);
			}
			break;
		}
		
		messageTo(self, "handleWailingPet", null, 15, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int destroyAbandonedPet(obj_id self, dictionary params) throws InterruptedException
	{
		if (!pet_lib.hasMaster(self))
		{
			destroyObject(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean crossPortalBreach(obj_id npc, obj_id breacher) throws InterruptedException
	{
		obj_id myCell = getLocation(npc).cell;
		obj_id yourCell = getLocation(breacher).cell;
		
		if (isIdValid(myCell) == isIdValid(yourCell))
		{
			if (!canSee(npc,breacher))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		
		return true;
	}
	
	
	public void manageCrossPortalBreach(obj_id npc, obj_id breacher) throws InterruptedException
	{
		if (!isPlayer(breacher))
		{
			return;
		}
		
		dictionary params = new dictionary();
		params.put("breacher", breacher);
		messageTo(npc, "handleMonitorMovement", params, 5, false);
		return;
	}
	
	
	public int handleMonitorMovement(obj_id self, dictionary params) throws InterruptedException
	{
		LOGC(aiLoggingEnabled(self), "debug_ai", "ai::handleMonitorMovement() self("+ self + ")");
		
		if (ai_lib.isAiDead(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id breacher = params.getObjId("breacher");
		
		if (!isIdValid(breacher) || !exists(breacher) || !isInWorld(breacher))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isAiDead(breacher))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] myContents = getTriggerVolumeContents(self, ai_lib.ALERT_VOLUME_NAME);
		
		if (utils.getElementPositionInArray(myContents, breacher) == -1)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id myCell = getLocation(self).cell;
		obj_id yourCell = getLocation(breacher).cell;
		
		if (!isIdValid(yourCell))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (yourCell != myCell)
		{
			messageTo(self, "handleMonitorMovement", params, 3, false);
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isInCombat(self))
		{
			messageTo(self, "handleMonitorMovement", params, 3, false);
			return SCRIPT_CONTINUE;
		}
		
		startCombat(self, breacher);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAboutToBeTransferred(obj_id self, obj_id destContainer, obj_id transferer) throws InterruptedException
	{
		if (pet_lib.isPet(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (beast_lib.isBeast(self))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id bldg = getTopMostContainer(destContainer);
		
		if (isPlayer(bldg))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(destContainer))
		{
			return SCRIPT_CONTINUE;
		}
		
		boolean wontEnter = false;
		int niche = ai_lib.aiGetNiche(self);
		
		if (niche == NICHE_VEHICLE)
		{
			wontEnter = true;
		}
		
		if (!permissionsIsAllowed(destContainer, self))
		{
			wontEnter = true;
		}
		
		if (wontEnter)
		{
			if (ai_lib.isInCombat(self))
			{
				removeObjVar(self, "ai.combat.moveMode");
				stop(self);
			}
			else
			{
				stop(self);
				pathTo(self, aiGetHomeLocation(self));
				messageTo(self, "resumeDefaultCalmBehavior", null, 20, false);
			}
			
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean fightNotFlee(obj_id npc, obj_id threat) throws InterruptedException
	{
		LOGC(aiLoggingEnabled(npc), "debug_ai", "ai::fightNotFlee() npc("+ npc + ") threat("+ threat + ")");
		
		if (hasObjVar(threat, "gm"))
		{
			return false;
		}
		
		if (!ai_lib.isInCombat(npc))
		{
			if (factions.isAggro(npc) && factions.getFactionReaction(npc, threat)==factions.REACTION_NEGATIVE)
			{
				
				startCombat(npc, threat);
				
				return true;
			}
			
			if (pvpIsEnemy(npc, threat) && pvpCanAttack(npc, threat))
			{
				
				startCombat(npc, threat);
				
				return true;
			}
			
			if (ai_lib.isMonster(npc) && ai_lib.isMonster(threat) && ai_lib.isAggro(npc))
			{
				
				startCombat(npc, threat);
				
				return true;
			}
			
			if ((ai_lib.aiGetNiche(npc) == NICHE_CARNIVORE || ai_lib.aiGetNiche(npc) == NICHE_PREDATOR) && ai_lib.aiGetNiche(threat) == NICHE_HERBIVORE)
			{
				
				startCombat(npc, threat);
				
				return true;
			}
		}
		
		return false;
	}
	
	
	public int handleEntertainmentStart(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("entertainer");
		
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isInCombat(self) || ai_lib.aiIsDead(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (isInNpcConversation(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!ai_lib.isInSameBuilding(self,player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasScript (self, "ai.soldier") && !hasObjVar(self, gcw.GCW_ENTERTAINMENT_FLAG))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (instance.isInInstanceArea(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, performance.NPC_ENTERTAINMENT_NO_ENTERTAIN))
		{
			return SCRIPT_CONTINUE;
		}
		
		int niche = ai_lib.aiGetNiche(self);
		
		if (niche != NICHE_NPC)
		{
			return SCRIPT_CONTINUE;
		}
		
		String npcType = getCreatureName(self);
		
		if (npcType.equals("darth_vader"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, gcw.GCW_ENTERTAINMENT_FLAG) && !gcw.canEntertainGcwNonPlayingCharacter(player, self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, gcw.GCW_ENTERTAINMENT_FLAG) && utils.hasScriptVar(player, gcw.GCW_SCRIPTVAR_PARENT + ".gcwNpc"))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		String entertainmentType = params.getString("perf_type");
		String skillName = "healing_music_ability";
		
		if (entertainmentType != null)
		{
			if (entertainmentType.equals(performance.PERFORMANCE_TYPE_DANCE))
			{
				skillName = "healing_dance_ability";
			}
			else if (entertainmentType.equals(performance.PERFORMANCE_TYPE_JUGGLE))
			{
				skillName = "healing_juggle_ability";
			}
		}
		
		int skillMod = rand(1,100);
		
		if (entertainmentType != null)
		{
			skillMod = getSkillStatisticModifier(player, skillName);
		}
		
		if (skillMod == 0)
		{
			skillMod = rand(1,100);
		}
		
		if (utils.hasScriptVar(self, "ai.listeningTo"))
		{
			
			int oldSkillMod = utils.getIntScriptVar(self, "ai.oldEntertainerSkillMod");
			
			if (skillMod < oldSkillMod)
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		if (hasObjVar(self, "faction_recruit_quest") && groundquests.isQuestActive(player, gcw.GCW_ENTERTAIN_RALLY))
		{
			if (((factions.isImperialorImperialHelper(player) && factions.isImperialorImperialHelper(self)) || (factions.isRebelorRebelHelper(player) && factions.isRebelorRebelHelper(self))))
			{
				utils.setScriptVar(self, "faction_ent",player);
				
				messageTo(self, "readyToTalk", params, 20, false);
				
				params.put("lostInterestTime", getGameTime());
				utils.setScriptVar(self, "lostInterestTime", getGameTime());
				messageTo(self, "lostInterest", params, 60, false );
			}
			else
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		utils.setScriptVar(self, "ai.oldEntertainerSkillMod", skillMod);
		
		if (!hasObjVar(self, "ai.oldYaw"))
		{
			setObjVar(self, "ai.oldYaw", getYaw(self));
		}
		
		utils.setScriptVar(self, "ai.listeningTo", player);
		faceToBehavior(self, player);
		
		if (skillMod < 20)
		{
			if (rand(1,2)==1)
			{
				setAnimationMood(self, "npc_sad");
			}
			else
			{
				setAnimationMood(self, "bored");
			}
		}
		
		if (skillMod < 50)
		{
			if (rand(1,2)==1)
			{
				setAnimationMood(self, "bored");
			}
			else
			{
				setAnimationMood(self, "entertained");
			}
		}
		else
		{
			setAnimationMood(self, "entertained");
		}
		
		listenToMessage(player, "handlePerformerStopPerforming");
		messageTo(self, "handleNoEntertainment", null, 30, false);
		
		if (hasObjVar(self, gcw.GCW_ENTERTAINMENT_FLAG))
		{
			gcw.setEntertainGcwNonPlayerCharacter(player, self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleNoEntertainment(obj_id self, dictionary params) throws InterruptedException
	{
		boolean stopListening = false;
		
		if (!utils.hasScriptVar(self, "ai.listeningTo"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = utils.getObjIdScriptVar(self, "ai.listeningTo");
		
		if (!isIdValid(player))
		{
			stopListening = true;
		}
		else if (!exists(player) || !isInWorld(player) || !ai_lib.isInSameBuilding(self,player) || getPerformanceType(player) == 0)
		{
			stopListening = true;
		}
		
		if (stopListening)
		{
			
			utils.removeScriptVar(self, "ai.oldEntertainerSkillMod");
			utils.removeScriptVar(self, "ai.listeningTo");
			stop(self);
			messageTo(self, "resumeDefaultCalmBehavior", null, 0, false);
		}
		else
		{
			
			messageTo(self, "handleNoEntertainment", null, 30, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlePerformerStopPerforming(obj_id self, dictionary params) throws InterruptedException
	{
		
		utils.notifyObject(self, "handleEntertainmentStop", params);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleEntertainmentStop(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id player = obj_id.NULL_ID;
		if (params.containsKey("entertainer"))
		{
			player = params.getObjId("entertainer");
		}
		else if (params.containsKey("performer"))
		{
			player = params.getObjId("performer");
		}
		
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id entertainer = utils.getObjIdScriptVar(self, "ai.listeningTo");
		
		if (entertainer != player)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.aiIsDead(self) || ai_lib.isInCombat(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		setAnimationMood(self, "calm");
		
		int skillMod = utils.getIntScriptVar(self, "ai.oldEntertainerSkillMod");
		
		if (skillMod == 0)
		{
			skillMod = rand(1,100);
		}
		if (skillMod < 20)
		{
			switch (rand(1,4))
			{
				case 1 :
				doAnimationAction(self, "hold_nose");
				break;
				case 2 :
				doAnimationAction(self, "cover_ears_mocking");
				break;
				case 3 :
				doAnimationAction(self, "point_away");
				break;
				case 4 :
				doAnimationAction(self, "shake_head_no");
				break;
			}
		}
		else if (skillMod < 40)
		{
			switch (rand(1,4))
			{
				case 1 :
				doAnimationAction(self, "shake_head_no");
				break;
				case 2 :
				doAnimationAction(self, "applause_polite");
				break;
				case 3 :
				doAnimationAction(self, "yes");
				break;
				case 4 :
				doAnimationAction(self, "snap_finger1");
				break;
			}
		}
		else if (skillMod < 60)
		{
			switch(rand(1,4))
			{
				case 1 :
				doAnimationAction(self, "snap_finger1");
				break;
				case 2 :
				doAnimationAction(self, "applause_polite");
				break;
				case 3 :
				doAnimationAction(self, "yes");
				break;
				case 4 :
				doAnimationAction(self, "applause_excited");
				break;
			}
		}
		else
		{
			switch (rand(1,3))
			{
				case 1 :
				doAnimationAction(self, "applause_excited");
				break;
				case 2 :
				doAnimationAction(self, "clap_rousing");
				break;
				case 3 :
				doAnimationAction(self, "implore");
				break;
				case 4 :
				doAnimationAction(self, "applause_polite");
				break;
			}
		}
		
		utils.removeScriptVar(self, "ai.oldEntertainerSkillMod");
		utils.removeScriptVar(self, "ai.listeningTo");
		stop(self);
		messageTo(self, "resumeDefaultCalmBehavior", null, 5, false);
		
		if (hasObjVar(self, gcw.GCW_ENTERTAINMENT_FLAG) && utils.hasScriptVar(self, gcw.GCW_SCRIPTVAR_PARENT + ".isGcwEntertained"))
		{
			LOG("gcw_entertainer","NPC is saving time player stopped entertaining");
			utils.setScriptVar(self, gcw.GCW_STOPPED_ENTERTAINING, getGameTime());
			if (hasObjVar(self, gcw.GCW_PATROL_OBJ))
			{
				obj_id patrolObject = getObjIdObjVar(self, gcw.GCW_PATROL_OBJ);
				if (isValidId(patrolObject))
				{
					dictionary webster = new dictionary();
					webster.put("npc",self);
					webster.put("player",player);
					messageTo(patrolObject, "handleOffensiveEntertainedNpcCleanUp", webster, 1, false);
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnEnteredCombat(obj_id self) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnChangedPosture(obj_id self, int before, int after) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	public int dizzyCheckFailed(obj_id self, dictionary params) throws InterruptedException
	{
		ai_lib.aiSetPosture(self, POSTURE_KNOCKED_DOWN);
		return SCRIPT_CONTINUE;
	}
	
	
	public int applyModTrap(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar(self, "trapmod.enable"))
		{
			return SCRIPT_CONTINUE;
		}
		
		java.util.Enumeration keys = params.keys();
		
		while (keys.hasMoreElements())
		{
			testAbortScript();
			String key = (String)keys.nextElement();
			int mod = params.getInt(key);
			
			if (!key.equals("prefix"))
			{
				utils.setScriptVar(self, "trapmod."+key, mod);
			}
		}
		
		utils.setScriptVar(self, "trapmod.enable", 1);
		
		messageTo(self, "removeModTrap", params, 60.f, false);
		
		String prefix = params.getString("prefix");
		showFlyText(self, new string_id("trap/trap", prefix+"_on"), 1.2f, colors.ORANGERED);
		
		obj_id[] players = getPlayerCreaturesInRange(self, 50.f);
		
		for (int i=0; i<players.length; i++)
		{
			testAbortScript();
			playClientEffectLoc(players[i], "clienteffect/combat_trap_"+prefix+".cef", getLocation(self), 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeModTrap(obj_id self, dictionary params) throws InterruptedException
	{
		utils.removeScriptVar(self, "trapmod");
		String prefix = params.getString("prefix");
		
		showFlyText(self, new string_id("trap/trap", prefix+"_off"), 1.2f, colors.ORANGERED);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (pet_lib.isPet(self) || beast_lib.isBeast(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		location here = getLocation(self);
		
		if (here == null)
		{
			LOG("designer_fatal", "ai.ai::OnObjectMenuRequest() -> getLocation(self) == null. WTF?!");
			return SCRIPT_CONTINUE;
		}
		
		if (isIdValid(here.cell))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id myContainer = getContainedBy(self);
		
		if (!isIdValid(myContainer))
		{
			return SCRIPT_CONTINUE;
		}
		
		destroyObject(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		obj_id group = getGroupObject(self);
		
		if (isIdValid(group))
		{
			queueCommand(self, (1188179258), group, "", COMMAND_PRIORITY_DEFAULT);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int forceAnimalAttack(obj_id self, dictionary params) throws InterruptedException
	{
		LOGC(aiLoggingEnabled(self), "debug_ai", "ai::forceAnimalAttack() self("+ self + ")");
		
		if (ai_lib.aiIsDead(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id target = params.getObjId("target");
		if (!isIdValid(target))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!pvpCanAttack(self, target))
		{
			return SCRIPT_CONTINUE;
		}
		
		utils.setScriptVar(self, "ai.combat.isTaunted", (getGameTime() + 20));
		
		if (ai_lib.isInCombat(self))
		{
			removeObjVar(self, "ai.combat.moveMode");
			stop(self);
			setLookAtTarget(self, target);
		}
		else
		{
			
			startCombat(self, target);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int attemptMilkHandler(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = params.getObjId("player");
		
		attemptMilk(player, self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void attemptMilk(obj_id player, obj_id self) throws InterruptedException
	{
		
		obj_id playerCurrentMount = getMountId (player);
		obj_id pInv = utils.getInventoryContainer(player);
		if (!isValidId(pInv) || !exists(pInv))
		{
			return;
		}
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessage(self, SID_CANT_MILK_INV_FULL);
			return;
		}
		
		if (isIdValid(playerCurrentMount))
		{
			sendSystemMessage(player, SID_CANT_MILK_MOUNTED);
			return;
		}
		
		if (utils.hasScriptVar(self, "milk.attempts"))
		{
			sendSystemMessage(player, SID_BEING_MILKED);
			return;
		}
		
		if (pet_lib.isPet(self) || beast_lib.isBeast(self))
		{
			showFlyText(self, new string_id("npc_reaction/flytext", "threaten"), 2.0f, colors.ORANGERED);
			sendSystemMessage(player, SID_CANT_MILK);
			return;
		}
		
		if (stealth.hasInvisibleBuff(player))
		{
			showFlyText(self, new string_id("npc_reaction/flytext", "threaten"), 2.0f, colors.ORANGERED);
			sendSystemMessage(player, SID_MILK_NOT_HIDDEN);
			return;
		}
		
		if (utils.hasScriptVar(self, "milk.lasttime"))
		{
			int milktime = utils.getIntScriptVar(self, "milk.lasttime");
			
			if (getGameTime() - milktime < 3600)
			{
				showFlyText(self, new string_id("npc_reaction/flytext", "threaten"), 2.0f, colors.ORANGERED);
				sendSystemMessage(player, SID_CANT_MILK);
				return;
			}
		}
		
		float milkStunModified = 0.0f;
		
		if (buff.hasBuff(player, "creature_milking_buff") || buff.hasBuff(player, "drink_starshine_surprise"))
		{
			milkStunModified += ((float)getEnhancedSkillStatisticModifierUncapped(player, "milk_stun_modified"));
			CustomerServiceLog("milking_and_lair_search", "handleMilking: Player: "+getName(player)+" OID: "+player+" has a buff that gives a stun chance of: "+milkStunModified+".");
		}
		
		if (milkStunModified > 0)
		{
			int randStunRoll = rand(1, 100);
			if (randStunRoll < milkStunModified)
			{
				CustomerServiceLog("milking_and_lair_search", "handleMilking: Player: "+getName(player)+" OID: "+player+" has successfully stunned creature "+self+". The creature will stand still for a few seconds.");
				
				messageTo(self, "milkingCreatureStunned", null, 0, false);
				messageTo(self, "milkingCreatureStunRemoval", null, 4, false);
			}
			else
			{
				messageTo(self, "milkingCreatureStunFail", null, 0, false);
				CustomerServiceLog("milking_and_lair_search", "handleMilking: Player: "+getName(player)+" OID: "+player+" failed to stun the creature.");
			}
		}
		
		showFlyText(self, new string_id("npc_reaction/flytext", "alert"), 2.0f, colors.MEDIUMSLATEBLUE);
		
		sendSystemMessage(player, SID_MILK_BEGIN);
		
		utils.setScriptVar(self, "milk.attempts", 0);
		dictionary params = new dictionary();
		params.put("player", player);
		messageTo(self, "handleMilking", params, 7, false);
	}
	
	
	public int handleMilking(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		
		if (isDead(self))
		{
			sendSystemMessage(player, SID_CANT_MILK_THE_DEAD);
			return SCRIPT_CONTINUE;
		}
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id playerCurrentMount = getMountId (player);
		
		if (isIdValid(playerCurrentMount))
		{
			sendSystemMessage(player, SID_CANT_MILK_MOUNTED);
			return SCRIPT_CONTINUE;
		}
		
		float dist = getDistance(self, player);
		
		if ((dist < -3) || (dist > 3))
		{
			showFlyText(self, new string_id("npc_reaction/flytext", "threaten"), 2.0f, colors.ORANGERED);
			sendSystemMessage(player, SID_MILK_TOO_FAR);
			utils.removeScriptVar(self, "milk.attempts");
			return SCRIPT_CONTINUE;
		}
		if (stealth.hasInvisibleBuff(player) || ai_lib.isInCombat(self) || ai_lib.isInCombat(player))
		{
			showFlyText(self, new string_id("npc_reaction/flytext", "threaten"), 2.0f, colors.ORANGERED);
			sendSystemMessage(player, SID_MILK_NOT_HIDDEN);
			utils.removeScriptVar(self, "milk.attempts");
			return SCRIPT_CONTINUE;
		}
		
		int attempts = utils.getIntScriptVar(self, "milk.attempts");
		attempts++;
		utils.setScriptVar(self, "milk.attempts", attempts);
		
		boolean shortenAttempts = false;
		float milkQuantityModified = 0.0f;
		float milkExceptionalModified = 0.0f;
		float milkStunModified = 0.0f;
		
		if (buff.hasBuff(player, "creature_milking_buff") || buff.hasBuff(player, "drink_starshine_surprise"))
		{
			if (buff.hasBuff(player, "creature_milking_buff"))
			{
				CustomerServiceLog("buff", "creature_milking_buff Buff used by player: "+player+" Name: "+getName(player)+ " has creature_milking_buff ");
			}
			else if (buff.hasBuff(player, "drink_starshine_surprise"))
			{
				CustomerServiceLog("buff", "creature_milking_buff Buff used by player: "+player+" Name: "+getName(player)+ " has drink_starshine_surprise buff ");
			}
			
			milkExceptionalModified += ((float)getEnhancedSkillStatisticModifierUncapped(player, "milk_exceptional_modified"));
			CustomerServiceLog("buff", "creature_milking_buff Buff used by player: "+player+" Name: "+getName(player)+ " has a exceptional milk modifier of: "+milkExceptionalModified);
			milkQuantityModified += ((float)getEnhancedSkillStatisticModifierUncapped(player, "milk_quantity_modified"));
			CustomerServiceLog("buff", "creature_milking_buff Buff used by player: "+player+" Name: "+getName(player)+ " has a milk quantity modifier of: "+milkQuantityModified);
			shortenAttempts = true;
		}
		
		if (attempts == 3 || shortenAttempts)
		{
			
			obj_id pInv = utils.getInventoryContainer(player);
			if (!isValidId(pInv) || !exists(pInv))
			{
				return SCRIPT_CONTINUE;
			}
			
			else if (getVolumeFree(pInv) <= 0)
			{
				sendSystemMessage(self, SID_CANT_MILK_INV_FULL);
				return SCRIPT_CONTINUE;
			}
			
			int lev = ai_lib.getLevel(self);
			if (lev > 50)
			{
				lev = 50;
			}
			int mlev = lev/2;
			if (mlev < 1)
			{
				mlev = 1;
			}
			int amt = rand(mlev*10, lev*10);
			
			if (shortenAttempts && milkQuantityModified > 0)
			{
				amt = amt + (int)(amt * milkQuantityModified);
			}
			
			String resourceType = ai_lib.getMilkType(self);
			if (resourceType == null || resourceType.equals(""))
			{
				CustomerServiceLog("milking_and_lair_search", "handleMilking: Player: "+getName(player)+" OID: "+player+" attempted to milk but could not retrieve a valid milk type on "+ self + " "+getName(self));
				blog("ai.handleMilking: cannot get resource type from ai_lib.getMilkType");
				return SCRIPT_CONTINUE;
			}
			String sceneName = getCurrentSceneName();
			if (sceneName == null || sceneName.equals(""))
			{
				CustomerServiceLog("milking_and_lair_search", "handleMilking: Player: "+getName(player)+" OID: "+player+" attempted to milk but could not retrieve a valid scene name (planet name) while milking "+ self + " "+getName(self));
				blog("ai.handleMilking: cannot get scene name");
				return SCRIPT_CONTINUE;
			}
			
			String rsrcMapTable = "datatables/creature_resource/resource_scene_map.iff";
			if (rsrcMapTable == null || rsrcMapTable.equals(""))
			{
				blog("ai.handleMilking: cannot get resource map table data");
				CustomerServiceLog("milking_and_lair_search", "handleMilking: Player: "+getName(player)+" OID: "+player+" attempted to milk but could not retrieve resource map table data to receive the milk resources while milking "+ self + " "+getName(self));
				return SCRIPT_CONTINUE;
			}
			
			String correctedPlanetName = dataTableGetString(rsrcMapTable, sceneName, 1);
			if (correctedPlanetName == null || correctedPlanetName.equals(""))
			{
				correctedPlanetName = "tatooine";
				CustomerServiceLog("milking_and_lair_search", "handleMilking: Player: "+getName(player)+" OID: "+player+" attempted to milk but received an invlaid Planet Name. As a result the milk resource planet attrib has been defaulted to TATOOINE.");
			}
			
			resourceType = resourceType + "_"+ correctedPlanetName;
			
			int useDistMap = dataTableGetInt(rsrcMapTable, sceneName, "useDistributionMap");
			if (useDistMap < 0)
			{
				blog("ai.handleMilking: cannot get int data from distribution data table");
				CustomerServiceLog("milking_and_lair_search", "handleMilking: Player: "+getName(player)+" OID: "+player+" attempted to milk but could not retrieve resource distribution data while milking "+ self + " "+getName(self));
				return SCRIPT_CONTINUE;
			}
			location worldLoc = getWorldLocation(self);
			if (worldLoc == null)
			{
				blog("ai.handleMilking: cannot get world location from cpp");
				CustomerServiceLog("milking_and_lair_search", "handleMilking: Player: "+getName(player)+" OID: "+player+" attempted to milk but could not retrieve world location while milking "+ self + " "+getName(self));
				return SCRIPT_CONTINUE;
			}
			if (useDistMap == 0)
			{
				worldLoc.area = correctedPlanetName;
			}
			
			obj_id[] resourceList = resource.createRandom(resourceType, amt, worldLoc, pInv, player, 2);
			if (resourceList == null)
			{
				blog("ai.handleMilking: cannot get resource data from resource.createRandom function");
				CustomerServiceLog("milking_and_lair_search", "handleMilking: Player: "+getName(player)+" OID: "+player+" attempted to milk but could not resource data from resource.createRandom function while milking "+ self + " "+getName(self));
				return SCRIPT_CONTINUE;
			}
			
			location curloc = getLocation(player);
			if (curloc == null)
			{
				blog("ai.handleMilking: cannot get resource data from getLocation cpp");
				CustomerServiceLog("milking_and_lair_search", "handleMilking: Player: "+getName(player)+" OID: "+player+" attempted to milk but could not retrieve location data while milking "+ self + " "+getName(self));
				return SCRIPT_CONTINUE;
			}
			
			for (int i = 0; i < resourceList.length; i++)
			{
				testAbortScript();
				
				blog(""+resourceList[i]);
				setLocation(resourceList[i], curloc);
				putIn(resourceList[i], pInv, player);
			}
			
			sendSystemMessage(player, SID_MILK_SUCCESS);
			utils.setScriptVar(self, "milk.lasttime", getGameTime());
			utils.removeScriptVar(self, "milk.attempts");
			
			if (!collection.collectionResource(player, "milk", (int)milkExceptionalModified))
			{
				blog("ai.handleMilking: cannot get resource data from getLocation cpp");
				CustomerServiceLog("milking_and_lair_search", "handleMilking: Player: "+getName(player)+" OID: "+player+" did not receive an exceptional milk from creature "+ self + " "+getName(self)+". Check the collection logs for additional details.");
				return SCRIPT_CONTINUE;
			}
			
			return SCRIPT_CONTINUE;
		}
		else
		{
			sendSystemMessage(player, SID_MILK_CONTINUE);
		}
		
		messageTo(self, "handleMilking", params, 7, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSpawnParameters(obj_id self, dictionary params) throws InterruptedException
	{
		LOGC(aiLoggingEnabled(self), "debug_ai", "ai::handleSpawnParameters() self("+ self + ")");
		
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		int actionCode = params.getInt("actionCode");
		obj_id target = params.getObjId("actionTarget");
		
		switch (actionCode)
		{
			case gcw.AC_ATTACK:
			if (isIdValid(target))
			{
				startCombat(self, target);
			}
			break;
			
			case gcw.AC_SCAN:
			int scanInterests = params.getInt("scanInterests");
			
			if (scanInterests == 0)
			{
				gcw.assignScanInterests(self);
			}
			else
			{
				utils.setScriptVar(self, gcw.SCRIPTVAR_SCAN_INTEREST, scanInterests);
			}
			
			gcw.harass(self, target);
			break;
			
			default:
			break;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int playDelayedClientEffect(obj_id self, dictionary params) throws InterruptedException
	{
		String clientEffect = params.getString("effect");
		obj_id target = params.getObjId("target");
		
		playClientEffectObj(target, clientEffect, target, "");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int clearScriptVar(obj_id self, dictionary params) throws InterruptedException
	{
		String scriptVarName = params.getString("name");
		
		utils.removeScriptVar(self, scriptVarName);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInvulnerableChanged(obj_id self, boolean invulnerable) throws InterruptedException
	{
		LOGC(aiLoggingEnabled(self), "debug_ai", "ai::OnInvulnerableChanged() self("+ self + ") invulerable("+ invulnerable + ")");
		
		if (invulnerable)
		{
			removeTriggerVolume(ai_lib.ALERT_VOLUME_NAME);
			removeTriggerVolume(ai_lib.AGGRO_VOLUME_NAME);
		}
		else
		{
			ai_lib.resetCombatTriggerVolumes();
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnIncapacitateTarget(obj_id self, obj_id victim) throws InterruptedException
	{
		if (!utils.hasScriptVar(self,"experienced"))
		{
			if (isIdValid(victim))
			{
				if (isPlayer(victim))
				{
					if (rand(1,1000)==999)
					{
						ai_lib.creatureLevelUp(self, victim);
						utils.setScriptVar(self,"experienced",1);
					}
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int clearCollectionCameraParticle(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id target = params.getObjId("target");
		
		stopClientEffectObjByLabel(target, "root_buff");
		utils.removeScriptVar(target, "picture_taken_debuff");
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGiveItem(obj_id self, obj_id item, obj_id giver) throws InterruptedException
	{
		if (!hasCompletedCollectionSlot(giver, "meatlump_recruiter_starter"))
		{
			return SCRIPT_CONTINUE;
		}
		if (!isIdValid(giver) || !exists(giver))
		{
			return SCRIPT_CONTINUE;
		}
		if (!isIdValid(item) || !exists(item))
		{
			return SCRIPT_CONTINUE;
		}
		
		String itemName = getStaticItemName(item);
		if (!itemName.equals("col_meatlump_recruit_manual_02_01"))
		{
			return SCRIPT_CONTINUE;
		}
		
		String mobName = getCreatureName(self);
		String socialMeatlump = dataTableGetString(CREATURE_TABLE, mobName, "socialGroup");
		
		if (socialMeatlump.startsWith("meatlump") || socialMeatlump.endsWith("meatlump"))
		{
			if (!ai_lib.isNpc(self))
			{
				return SCRIPT_CONTINUE;
			}
			else
			{
				sendSystemMessage(giver, SID_ALREADY_RECRUITED);
			}
			return SCRIPT_CONTINUE;
		}
		if (toLower(socialMeatlump).equals("rebel") || toLower(socialMeatlump).equals("rebel"))
		{
			if (!ai_lib.isNpc(self))
			{
				return SCRIPT_CONTINUE;
			}
			else
			{
				sendSystemMessage(giver, SID_NO_RECRUIT_REB_IMP);
			}
			return SCRIPT_CONTINUE;
		}
		
		if (hasCompletedCollection(giver, "col_meatlump_recruiter_01"))
		{
			return SCRIPT_CONTINUE;
		}
		if (utils.hasScriptVar(self, "collection.recruited"))
		{
			sendSystemMessage(giver, SID_ALREADY_RECRUITED);
			return SCRIPT_CONTINUE;
		}
		if (utils.hasScriptVar(self, "collection.investigator"))
		{
			sendSystemMessage(giver, SID_ENEMY_RECRUIT);
			return SCRIPT_CONTINUE;
		}
		
		if (!ai_lib.isNpc(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isInvulnerable(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		int attackRoll = rand(1,100);
		if (attackRoll <= 10)
		{
			chat.chat(self, SID_NPC_MEATLUMP_SPEAK);
			
			float searchSize = 12.0f;
			
			obj_id mob = null;
			
			int mobLevel = getLevel(giver) -2;
			
			location currentLoc = getLocation(giver);
			
			location locGoodLocation = locations.getGoodLocationAroundLocationAvoidCollidables(currentLoc, searchSize, searchSize, searchSize, searchSize, false, false, 6.0f);
			if (locGoodLocation == null)
			{
				locGoodLocation = (location)currentLoc.clone();
			}
			
			int mobCount = rand(1, 3);
			
			if (utils.isProfession(giver, utils.TRADER))
			{
				
				mobCount = 2;
			}
			
			if (mobCount == 1)
			{
				mob = create.object("mtp_recruiter_corsec_detective", locGoodLocation, mobLevel);
			}
			if (mobCount == 2)
			{
				mob = create.object("mtp_recruiter_corsec_investigator", locGoodLocation, mobLevel);
			}
			if (mobCount == 3)
			{
				mob = create.object("mtp_recruiter_corsec_agent", locGoodLocation, mobLevel);
			}
			addHate(mob, giver, 1000.0f);
			
			utils.setScriptVar(self, "collection.investigator", 1);
			messageTo(self, "removeMeatlumpInvestigatorScriptVar", null, 82800, false);
		}
		else
		{
			
			sendSystemMessage(giver, SID_GAVE_RECRUIT_ITEM);
			
			int count = getCount(item);
			count--;
			
			if (count <= 0)
			{
				
				detachScript(item, "item.special.nodestroy");
				destroyObject(item);
			}
			else
			{
				setCount(item, count);
			}
			
			modifyCollectionSlotValue(giver, "meatlump_recruiter_01", 1);
			
			utils.setScriptVar(self, "collection.recruited", 1);
			messageTo(self, "removeMeatlumpRecruitmentScriptVar", null, 82800, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeMeatlumpRecruitmentScriptVar(obj_id self, dictionary params) throws InterruptedException
	{
		if (utils.hasScriptVar(self, "collection.recruited"))
		{
			utils.removeScriptVar(self, "collection.recruited");
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeMeatlumpInvestigatorScriptVar(obj_id self, dictionary params) throws InterruptedException
	{
		if (utils.hasScriptVar(self, "collection.investigator"))
		{
			utils.removeScriptVar(self, "collection.investigator");
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int milkingCreatureStunned(obj_id self, dictionary params) throws InterruptedException
	{
		showFlyText(self, new string_id("npc_reaction/flytext", "stun"), 2.0f, colors.ORANGERED);
		
		ai_lib.setDefaultCalmBehavior(self, ai_lib.BEHAVIOR_SENTINEL);
		return SCRIPT_CONTINUE;
	}
	
	
	public int milkingCreatureStunFail(obj_id self, dictionary params) throws InterruptedException
	{
		showFlyText(self, new string_id("npc_reaction/flytext", "stun_fail"), 2.0f, colors.ORANGERED);
		return SCRIPT_CONTINUE;
	}
	
	
	public int milkingCreatureStunRemoval(obj_id self, dictionary params) throws InterruptedException
	{
		showFlyText(self, new string_id("npc_reaction/flytext", "stun_expired"), 2.0f, colors.ORANGERED);
		ai_lib.setDefaultCalmBehavior(self, ai_lib.BEHAVIOR_LOITER);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean blog(String msg) throws InterruptedException
	{
		if (LOGGING_ON && !msg.equals(""))
		{
			LOG(LOGGING_CATEGORY, msg);
		}
		return true;
	}
}
