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
import script.ai.ai_combat_damage;
import script.ai.ai_combat_movement;
import script.ai.ai_combat_target;
import script.library.ai_lib;
import script.library.beast_lib;
import script.library.buff;
import script.library.colors;
import script.library.combat;
import script.library.combat_consts;
import script.library.factions;
import script.library.instance;
import script.library.movement;
import script.library.pclib;
import script.library.pet_lib;
import script.library.posture;
import script.library.scout;
import script.library.stealth;
import script.library.storyteller;
import script.library.trial;
import script.library.utils;
import script.library.xp;
import script.library.stealth;



public class creature_combat extends script.systems.combat.combat_base
{
	public creature_combat()
	{
	}
	public static final float MIN_MOVEMENT_DURING_COMBAT = 0.015f;
	public static final float MAX_MOVEMENT_DURING_COMBAT = 0.05f;
	
	public static final float TOO_CLOSE_DISTANCE = 3f;
	
	
	public void clog(String text) throws InterruptedException
	{
		if (text != null)
		{
			LOGC(aiLoggingEnabled(getSelf()), "debug_ai", "creature_combat::"+ text);
		}
	}
	
	
	public int OnEnteredCombat(obj_id self) throws InterruptedException
	{
		
		clog("OnEnteredCombat() ----- unit("+ self + ":"+ getName(self) + ")");
		
		clearCombatActions();
		if (!stealth.isDecoy(self))
		{
			aiEquipPrimaryWeapon(self);
		}
		
		removeTriggerVolume(ai_lib.ALERT_VOLUME_NAME);
		removeTriggerVolume(ai_lib.AGGRO_VOLUME_NAME);
		
		ai_combat_movement.aiClearMoveMode();
		setMovementRun(self);
		
		deltadictionary dict = self.getScriptVars();
		dict.put("ai.combat.cover.foundTarget", isPlayer(getHateTarget(self)) ? false : true);
		
		if (ai_combat_assist.isWaiting())
		{
			ai_combat_movement.aiIdle();
		}
		else
		{
			obj_id target = getHateTarget(self);
			boolean targetCovered = stealth.hasInvisibleBuff(target);
			boolean targetProne = posture.isProne(target);
			
			if (targetCovered || targetProne)
			{
				showFlyText(self, new string_id("npc_reaction/flytext", "alert"), 2.0f, colors.STEELBLUE);
			}
			
			utils.setScriptVar(self, "creature_combat.lastCombatFrame", getGameTime());
			
			if (ai_combat_assist.canCallForAssist())
			{
				ai_combat_assist.callForAssist();
			}
			
			if (beast_lib.isBeast(self) && getState(self, STATE_COVER) == 1)
			{
				stealth.makeCreatureVisible(self);
			}
			
			doCombatFrame();
		}
		
		messageTo(self, "checkForBeastSpecialsTakeOne", null, 1.0f, false);
		
		setRegenRate(self, HEALTH, 0);
		
		if (!beast_lib.isBeast(self))
		{
			setRegenRate(self, ACTION, 20);
		}
		else
		{
			
			setRegenRate(self, ACTION, 4);
		}
		
		if (combat.isPersistCombatMode(self))
		{
			messageTo(self, "persist_combat", trial.getSessionDict(self, combat.PERSIST_COMBAT), 5.0f, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnExitedCombat(obj_id self) throws InterruptedException
	{
		if (!isIdValid(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		LOG("get_participants", "OnExitedCombat init");
		
		clog("OnExitedCombat() ----- unit("+ self + ":"+ getName(self) + ")");
		
		ai_combat_target.clearTargetList();
		ai_combat_damage.clearAttackerList();
		ai_combat_movement.aiClearMoveMode();
		ai_combat_assist.clearNoAssistCall();
		setLookAtTarget(self, null);
		
		utils.removeScriptVarTree(self, "me_evasion");
		
		if (!beast_lib.isBeast(self))
		{
			ai_lib.resetCombatTriggerVolumes();
			
			float healthRegenMod = hasObjVar(self, "regen_mod.health") ? getFloatObjVar(self, "regen_mod.health") : 1.0f;
			float actionRegenMod = hasObjVar(self, "regen_mod.action") ? getFloatObjVar(self, "regen_mod.action") : 1.0f;
			
			float healthRegen = (getMaxAttrib(self, HEALTH) / 10.0f) * healthRegenMod;
			float actionRegen = (getMaxAttrib(self, ACTION) / 10.0f) * actionRegenMod;
			
			setRegenRate(self, HEALTH, (int)healthRegen);
			setRegenRate(self, ACTION, (int)actionRegen);
		}
		else
		{
			setRegenRate(self, HEALTH, 150);
			setRegenRate(self, ACTION, 10);
			
			if (stealth.canBeastStealth(self))
			{
				stealth.makeBeastInvisible(getMaster(self), "appearance/pt_smoke_puff.prt");
			}
		}
		
		if (!ai_lib.isAiDead(self))
		{
			if (posture.isKnockedDown(self))
			{
				queueCommand(self, (-1322133240), self, "", COMMAND_PRIORITY_DEFAULT);
			}
			else if (!posture.isStanding(self))
			{
				posture.stand(self);
			}
			
			if (!beast_lib.isBeast(self))
			{
				if (pet_lib.isPet(self))
				{
					messageTo(self, "postCombatPathHome", null, 1, false);
				}
				else
				{
					aiTether(self);
				}
			}
		}
		
		bumpSession(self);
		trial.bumpSession(self, combat.PERSIST_COMBAT);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAiTetherStart(obj_id self) throws InterruptedException
	{
		
		if (!isIdValid(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		ai_lib.setMood(self, ai_lib.MOOD_CALM);
		ai_lib.clearCombatData();
		xp.cleanupCreditForKills();
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAiTetherComplete(obj_id self) throws InterruptedException
	{
		clog("ai::OnAiTetherComplete() --- BEGIN --- self("+ self + ":"+ getName(self) + ")");
		
		ai_lib.resetAi();
		
		clog("ai::OnAiTetherComplete() --- END --- self("+ self + ":"+ getName(self) + ")");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnHateTargetChanged(obj_id self, obj_id target) throws InterruptedException
	{
		clog("OnHateTargetChanged() self("+ self + ":"+ getName(self) + ") target("+ target + ")");
		
		setLookAtTarget(self, target);
		
		if (ai_lib.isInCombat(self))
		{
			ai_combat_assist.clearAssist();
		}
		
		if (pet_lib.isPet(self) && utils.hasScriptVar(self, "ai.combat.target"))
		{
			obj_id combatTarget = utils.getObjIdScriptVar(self, "ai.combat.target");
			
			if (target != combatTarget)
			{
				float maxHate = getMaxHate(self);
				setHate(self, combatTarget, maxHate + 5000);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnHateTargetAdded(obj_id self, obj_id target) throws InterruptedException
	{
		clog("creature_combat::OnHateTargetAdded() self("+ self + ":"+ getName(self) + ") target("+ target + ")");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnHateTargetRemoved(obj_id self, obj_id target) throws InterruptedException
	{
		clog("creature_combat::OnHateTargetRemoved() self("+ self + ":"+ getName(self) + ") target("+ target + ")");
		
		if (!isIdValid(target) || !exists(target))
		{
			return SCRIPT_CONTINUE;
		}
		
		final int factionStatus = factions.getFactionStatus(self, target);
		
		if (factionStatus == factions.STATUS_FRIEND || (factionStatus == factions.STATUS_NEUTRAL && !aiIsAggressive(self)))
		{
			
			if (hasScript(self, "systems.missions.dynamic.mission_bounty_target"))
			{
				obj_id objHunter = getObjIdObjVar(self, "objHunter");
				
				if (!isIdValid(objHunter) || (objHunter != target))
				{
					pvpRemovePersonalEnemyFlags(self, target);
				}
			}
			else
			{
				pvpRemovePersonalEnemyFlags(self, target);
			}
		}
		
		if (pet_lib.isPet(self) && utils.hasScriptVar(self, "ai.combat.target"))
		{
			obj_id combatTarget = utils.getObjIdScriptVar(self, "ai.combat.target");
			
			if (target == combatTarget)
			{
				utils.removeScriptVar(self, "ai.combat.target");
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAiPrimaryWeaponEquipped(obj_id self, obj_id primaryWeapon) throws InterruptedException
	{
		clog("OnAiPrimaryWeaponEquipped() ai("+ self + ") primaryWeapon("+ primaryWeapon + ")");
		clearCombatActions();
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAiSecondaryWeaponEquipped(obj_id self, obj_id secondaryWeapon) throws InterruptedException
	{
		clog("OnAiSecondaryWeaponEquipped() ai("+ self + ") secondaryWeapon("+ secondaryWeapon + ")");
		clearCombatActions();
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAddedToWorld(obj_id self) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAiCombatFrame(obj_id self) throws InterruptedException
	{
		
		boolean assistWaiting = ai_combat_assist.isWaiting();
		
		if (assistWaiting || getSkillStatisticModifier(self, "stifle") > 0)
		{
			
			final obj_id hateTarget = getHateTarget(self);
			faceTo(self, hateTarget);
			
			if (getCombatDuration(self) > ai_combat_assist.getWaitTime())
			{
				ai_combat_assist.clearAssist();
			}
		}
		else
		{
			doCombatFrame();
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void doCombatFrame() throws InterruptedException
	{
		
		final obj_id self = getSelf();
		final obj_id target = getBestTarget(self);
		
		int lastFrame = utils.getIntScriptVar(self, "creature_combat.lastCombatFrame");
		
		utils.setScriptVar(self, "creature_combat.lastCombatFrame", getGameTime());
		
		if (getState(self, STATE_COVER) == 1)
		{
			stealth.makeCreatureVisible(self);
		}
		
		if (utils.hasScriptVar(self, "petIgnoreAttacks"))
		{
			
			return;
		}
		
		if (!isIdValid(target))
		{
			
			obj_id[] haters = getHateList(self);
			
			if (haters != null && haters.length > 0 && !utils.hasScriptVar(self, "hateListRetry"))
			{
				
				Vector goodIds = new Vector();
				goodIds.setSize(0);
				for (int i = 0; i < haters.length; ++i)
				{
					testAbortScript();
					
					if (isIdValid(haters[i]) && exists(haters[i]))
					{
						utils.addElement(goodIds, haters[i]);
					}
				}
				
				if (goodIds != null & goodIds.size() > 0)
				{
					
					int targetIndex = rand(0, goodIds.size() - 1);
					
					forceHateTarget(self, ((obj_id)(goodIds.get(targetIndex))));
					
					utils.setScriptVar(self, "hateListRetry", 1);
					
					doCombatFrame();
				}
				
			}
			
			if (utils.hasScriptVar(self, "hateListRetry"))
			{
				utils.removeScriptVar(self, "hateListRetry");
			}
			
			if (beast_lib.isBeast(self))
			{
				obj_id master = getMaster(self);
				
				if (isIdValid(master) && master.isLoaded())
				{
					beast_lib.beastFollowTarget(self, master);
				}
				else
				{
					aiSetHomeLocation(self, getLocation(self));
				}
				
				return;
			}
			
			if (pet_lib.isPet(self))
			{
				messageTo(self, "postCombatPathHome", null, 1, false);
				return;
			}
			
			aiTether(self);
			
			return;
		}
		
		boolean checkVE = isIdValid(target) && exists(target) && isIdValid(self) && exists(self);
		
		if (!storyteller.storytellerCombatCheck(self, target))
		{
			
			if (checkVE)
			{
				removeHateTarget(self, target);
			}
			
			return;
		}
		
		if (getState(target, STATE_FEIGN_DEATH) == 1)
		{
			
			if (checkVE)
			{
				removeHateTarget(self, target);
			}
			
			return;
		}
		else if (!ai_combat_target.isValidTarget(target))
		{
			
			if (checkVE)
			{
				removeHateTarget(self, target);
			}
			
			return;
		}
		
		final boolean targetCovered = stealth.hasInvisibleBuff(target);
		final boolean targetProne = posture.isProne(target);
		final int freeShotCount = targetCovered ? 3 : (targetProne ? 1 : 0);
		
		deltadictionary dict = self.getScriptVars();
		
		if (!dict.getBoolean("ai.combat.cover.foundTarget"))
		{
			if ((getCombatDuration(self) > 10) || (freeShotCount == 0) || (ai_combat_damage.getAttackCount(target) > freeShotCount))
			{
				if (freeShotCount > 0 && !hasScript(self, "systems.dungeon_sequencer.ai_controller"))
				{
					showFlyText(self, new string_id("npc_reaction/flytext", "threaten"), 2.0f, colors.ORANGERED);
				}
				
				dict.put("ai.combat.cover.foundTarget", true);
			}
		}
		
		int combatRound = utils.getIntScriptVar(self, "aiCombatRoundCounter");
		combatRound++;
		utils.setScriptVar(self, "aiCombatRoundCounter", combatRound);
		
		selectWeapon(target);
		
		final boolean commandQueueClear = (getCurrentCommand(self) == 0);
		
		if (commandQueueClear)
		{
			utils.removeScriptVar(self, "creature_combat.commandQueueTime");
			
			if (posture.isKnockedDown(self))
			{
				
				final int knockDownTime = dict.getInt("ai.combat.knockdown_time");
				
				if ((getGameTime() - knockDownTime) > aiGetKnockDownRecoveryTime(self))
				{
					
					queueCommand(self, (-1322133240), self, "", COMMAND_PRIORITY_DEFAULT);
				}
			}
			else if (!posture.isStanding(self))
			{
				posture.stand(self);
			}
			else
			{
				if (dict.getBoolean("ai.combat.cover.foundTarget"))
				{
					attack(target);
				}
			}
		}
		else
		{
			if (!utils.hasScriptVar(self, "creature_combat.commandQueueTime"))
			{
				utils.setScriptVar(self, "creature_combat.commandQueueTime", getGameTime());
			}
			else
			{
				int time = utils.getIntScriptVar(self, "creature_combat.commandQueueTime");
				
				if (getGameTime() - time > 2)
				{
					clearCombatActions();
				}
			}
		}
		
		move(target);
		
	}
	
	
	public obj_id getBestTarget(obj_id self) throws InterruptedException
	{
		obj_id primaryTarget = getHateTarget(self);
		
		if (isIdValid(primaryTarget) && exists(primaryTarget) && !isDead(primaryTarget) && !stealth.hasInvisibleBuff(primaryTarget))
		{
			return primaryTarget;
		}
		
		obj_id[] hateList = getHateList(self);
		
		if (hateList == null || hateList.length == 0)
		{
			return null;
		}
		
		float topHate = 0.0f;
		primaryTarget = null;
		
		for (int i = 0; i < hateList.length; i++)
		{
			testAbortScript();
			if (!isIdValid(hateList[i]) || !exists(primaryTarget))
			{
				continue;
			}
			
			if (isDead(hateList[i]) || stealth.hasInvisibleBuff(hateList[i]))
			{
				
				continue;
			}
			
			if (hasObjVar(self, "noPursue") && !combat.cachedCanSee(self, hateList[i]))
			{
				continue;
			}
			
			if (getHate(self, hateList[i]) > topHate)
			{
				primaryTarget = hateList[i];
			}
		}
		
		return primaryTarget;
	}
	
	
	public void move(obj_id target) throws InterruptedException
	{
		
		final obj_id self = getSelf();
		
		if (ai_combat_movement.isAiImmobile(self))
		{
			
			return;
		}
		
		if (!posture.isStanding(self))
		{
			
			return;
		}
		
		if (!ai_lib.isWithinLeash(self))
		{
			stopCombat(self);
			
			return;
		}
		
		deltadictionary dict = self.getScriptVars();
		setLookAtTarget(self, target);
		
		final obj_id weapon = getCurrentWeapon(self);
		final range_info weaponRange = aiGetWeaponRangeInfo(weapon);
		final float distanceToTarget = getDistance(self, target);
		final int weaponCategory = combat.getWeaponCategory(getWeaponType(weapon));
		final boolean targetCovered = stealth.hasInvisibleBuff(target);
		
		if (!canSee(self, target))
		{
			ai_combat_movement.aiMoveToSee(target);
			
			return;
		}
		
		if ((weaponCategory == combat.RANGED_WEAPON) && (combat.getWeaponCategory(getWeaponType(aiGetPrimaryWeapon(self))) != combat.MELEE_WEAPON))
		{
			if ((distanceToTarget < TOO_CLOSE_DISTANCE) && isPlayer(target))
			{
				ai_combat_movement.aiEvade(target, self, weaponRange, 1f, 1f);
				
				return;
			}
			else if (!ai_combat_movement.aiIsFleeing(self))
			{;
			}
			{
				final float midRange = (weaponRange.maxRange - weaponRange.minRange) / 2.0f;
				final float rangeVariation = midRange * 0.2f;
				final float minFollowDistance = (weaponRange.minRange + midRange - rangeVariation);
				final float maxFollowDistance = (weaponRange.minRange + midRange + rangeVariation);
				
				ai_combat_movement.aiFollow(target, minFollowDistance, maxFollowDistance);
				faceTo(self, target);
			}
		}
		else
		{
			float minFollowDistance = 1.5f;
			float maxFollowDistance = 3.0f;
			
			if (!hasObjVar(self, "ai.noFollow"))
			{
				ai_combat_movement.aiFollow(target, minFollowDistance, maxFollowDistance);
			}
			else
			{
				if (distanceToTarget > 64)
				{
					stopCombat(self);
					resumeMovement(self);
					
					return;
				}
				else
				{
					suspendMovement(self);
				}
			}
			
			faceTo(self, target);
		}
		
		ai_combat_movement.aiEvade(target, self, weaponRange, MIN_MOVEMENT_DURING_COMBAT, MAX_MOVEMENT_DURING_COMBAT);
		
	}
	
	
	public void selectWeapon(obj_id target) throws InterruptedException
	{
		
		final obj_id self = getSelf();
		
		if (beast_lib.isBeast(self))
		{
			return;
		}
		
		if (!aiHasPrimaryWeapon(self))
		{
			
		}
		
		if (!aiHasSecondaryWeapon(self))
		{
			aiEquipPrimaryWeapon(self);
			return;
		}
		
		boolean equipPrimary = true;
		final float distanceToTarget = getDistance(self, target);
		
		final obj_id primaryWeapon = aiGetPrimaryWeapon(self);
		final obj_id secondaryWeapon = aiGetSecondaryWeapon(self);
		
		if (!exists(primaryWeapon) || !exists(secondaryWeapon))
		{
			return;
		}
		
		final range_info primaryWeaponRange = aiGetWeaponRangeInfo(primaryWeapon);
		final boolean primaryWeaponInRange = (distanceToTarget >= primaryWeaponRange.minRange) && (distanceToTarget <= primaryWeaponRange.maxRange);
		
		final range_info secondaryWeaponRange = aiGetWeaponRangeInfo(secondaryWeapon);
		final boolean secondaryWeaponInRange = (distanceToTarget >= secondaryWeaponRange.minRange) && (distanceToTarget <= secondaryWeaponRange.maxRange);
		
		if (aiUsingPrimaryWeapon(self) && aiHasSecondaryWeapon(self))
		{
			if (secondaryWeaponInRange)
			{
				if (!primaryWeaponInRange)
				{
					aiEquipSecondaryWeapon(self);
				}
				else if (primaryWeaponRange.maxRange > secondaryWeaponRange.maxRange)
				{
					aiEquipSecondaryWeapon(self);
				}
			}
		}
		else if (aiUsingSecondaryWeapon(self) && primaryWeaponInRange)
		{
			if (!secondaryWeaponInRange)
			{
				aiEquipPrimaryWeapon(self);
			}
			else if (secondaryWeaponRange.maxRange > primaryWeaponRange.maxRange)
			{
				aiEquipPrimaryWeapon(self);
			}
		}
	}
	
	
	public void attack(obj_id target) throws InterruptedException
	{
		
		final obj_id self = getSelf();
		String DEFAULT_ATTACK = combat.getAttackName(self);
		
		String currentActionString = null;
		String forcedActionString = getStringObjVar(self, "ai.combat.forcedAction");
		String pendingActionString = getStringObjVar(self, "ai.combat.pendingAction");
		String oneShotActionString = getStringObjVar(self, "ai.combat.oneShotAction");
		
		if (oneShotActionString != null && !hasObjVar(self, "oneShotActionComplete"))
		{
			forcedActionString = oneShotActionString;
			setObjVar(self, "oneShotActionComplete", 1);
		}
		else
		{
			if (oneShotActionString != null)
			{
				
				return;
			}
		}
		
		int pendingActionTime = getIntObjVar(self, "ai.combat.pendingActionTime");
		
		currentActionString = DEFAULT_ATTACK;
		
		clog("attack() pre-check forcedActionString: "+ forcedActionString + " pendingActionString: "+ pendingActionString + (pendingActionTime > 0 ? " pendingActionTime: "+ (getGameTime() - pendingActionTime): ""));
		
		if (forcedActionString != null)
		{
			currentActionString = forcedActionString;
		}
		else if (pendingActionString != null)
		{
			if (getGameTime() - pendingActionTime < 2)
			{
				currentActionString = pendingActionString;
			}
			else
			{
				removeObjVar(self, "ai.combat.pendingAction");
				removeObjVar(self, "ai.combat.pendingActionTime");
				currentActionString = DEFAULT_ATTACK;
			}
		}
		else
		{
			pendingActionString = aiGetCombatAction(self);
			
			if (pendingActionString != null)
			{
				currentActionString = pendingActionString;
				
				setObjVar(self, "ai.combat.pendingAction", pendingActionString);
				setObjVar(self, "ai.combat.pendingActionTime", getGameTime());
			}
			else
			{
				currentActionString = DEFAULT_ATTACK;
			}
		}
		
		clog("attack() post-check currentActionString: "+ currentActionString + " pendingActionString: "+ pendingActionString);
		
		int currentActionCrc = 0;
		
		final int result = combat.canPerformAction(currentActionString, self);
		
		clog("attack() post-check currentActionString: "+ currentActionString + " pendingActionString: "+ pendingActionString + " canPerformAction: "+ result);
		
		if (result == combat.ACTION_SUCCESS || oneShotActionString != null)
		{
			currentActionCrc = getStringCrc(currentActionString.toLowerCase());
		}
		else if (result == combat.ACTION_TOO_TIRED)
		{
			
			if (forcedActionString != null)
			{
				clog("attack() self("+ self + ") target("+ target + ") ACTION("+ forcedActionString + ") TOO TIRED - WAITING TO ATTEMPT FORCED ACTION");
			}
			else
			{
				if (pendingActionString != null)
				{
					clog("attack() self("+ self + ") target("+ target + ") ACTION("+ pendingActionString + ") TOO TIRED - ATTEMPTING DEFAULT ATTACK");
				}
				
				if (!currentActionString.equals(DEFAULT_ATTACK))
				{
					
					if (combat.canPerformAction(DEFAULT_ATTACK, self) == combat.ACTION_SUCCESS)
					{
						currentActionString = DEFAULT_ATTACK;
						currentActionCrc = getStringCrc(DEFAULT_ATTACK.toLowerCase());
					}
				}
			}
		}
		else if (result == combat.ACTION_INVALID_WEAPON)
		{
			
			clearCombatActions();
			clog("attack() ERROR self("+ self + getName(self) + ") ACTION("+ currentActionString + ") weapon("+ (aiUsingPrimaryWeapon(self) ? "primary weapon": "secondary weapon") + ") INVALID WEAPON FOR ACTION, FIX THE DATA");
		}
		else
		{
			
			clearCombatActions();
			clog("attack() ERROR self("+ self + getName(self) + ") ACTION("+ currentActionString + ") UNEXPECTED RESULT FROM combat.canPerformAction()");
		}
		
		if (currentActionCrc != 0)
		{
			combat_data cd = combat_engine.getCombatData(currentActionString);
			
			if (cd == null)
			{
				
				return;
			}
			
			int groupCrc = getStringCrc(cd.cooldownGroup);
			final float coolDownLeft = getCooldownTimeLeft(self, groupCrc);
			
			if (coolDownLeft <= 0.0f)
			{
				final float distanceToTarget = getDistance(self, target);
				final range_info weaponRange = aiGetWeaponRangeInfo(getCurrentWeapon(self));
				
				if (distanceToTarget < weaponRange.maxRange || distanceToTarget < cd.maxRange)
				{
					if (canSee(self, target))
					{
						clog("attack() self("+ self + ") target("+ target + ") ACTION("+ currentActionCrc + ":"+ currentActionString + ")");
						
						queueCommand(self, currentActionCrc, target, "", COMMAND_PRIORITY_DEFAULT);
						
						if (currentActionString.equals(forcedActionString))
						{
							removeObjVar(self, "ai.combat.forcedAction");
						}
						else if (currentActionString.equals(pendingActionString))
						{
							removeObjVar(self, "ai.combat.pendingAction");
						}
					}
				}
			}
		}
		
	}
	
	
	public int OnCreatureDamaged(obj_id self, obj_id attacker, obj_id weapon, int[] damage) throws InterruptedException
	{
		int lastFrame = utils.getIntScriptVar(self, "creature_combat.lastCombatFrame");
		
		if (!stealth.hasInvisibleBuff(attacker))
		{
			ai_combat_damage.addAttack(attacker);
		}
		
		if (getGameTime() - lastFrame > 2)
		{
			clog("OnCreatureDamaged() !!!COMBAT STUCK!!! lastFrame: "+ lastFrame + " time elapsed: "+ (getGameTime() - lastFrame));
			doCombatFrame();
		}
		
		ai_lib.triggerAgroLinks(self, attacker);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnChangedPosture(obj_id self, int oldPosture, int newPosture) throws InterruptedException
	{
		if (newPosture == POSTURE_KNOCKED_DOWN)
		{
			final deltadictionary dict = self.getScriptVars();
			
			dict.put("ai.combat.knockdown_time", getGameTime());
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int vocalizeEndCombat(obj_id self, dictionary params) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int postCombatPathHome(obj_id self, dictionary params) throws InterruptedException
	{
		if (ai_lib.isInCombat(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (pet_lib.isPet(self) || beast_lib.isBeast(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isFollowing(self))
		{
			ai_lib.resumeFollow(self);
		}
		else
		{
			pathTo(self, aiGetHomeLocation(self));
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean isNearLair(obj_id npc) throws InterruptedException
	{
		obj_id myLair = getObjIdObjVar(npc, "npc_lair.target");
		
		if (!hasObjVar(npc, "lairSpawn") && !isIdValid(myLair))
		{
			return false;
		}
		
		if (!isIdValid(myLair))
		{
			myLair = getObjIdObjVar(npc, "poi.baseObject");
		}
		
		if (isIdValid(myLair) && isInWorld(myLair) && exists(myLair))
		{
			if (getDistance(npc, myLair) < 40f)
			{
				return true;
			}
		}
		return false;
	}
	
	
	public void killPlayer(obj_id npc, obj_id target) throws InterruptedException
	{
		if (!isIdValid(target) || !isIncapacitated(target) || !isInWorld(target) || !exists(target) || ai_lib.isInCombat(npc))
		{
			return;
		}
		
		obj_id weapon = getCurrentWeapon(npc);
		int weaponType = getWeaponType(weapon);
		int weaponCat = combat.getWeaponCategory(weaponType);
		float range = getDistance(npc, target);
		
		if (range > 2.0f)
		{
			setObjVar(npc, "ai.pathingToKill", target);
			ai_lib.pathNear(npc, getLocation(target), 2f);
			return;
		}
		
		String skeleton = dataTableGetString("datatables/ai/species.iff", ai_lib.aiGetSpecies(npc), "Skeleton");
		
		if (skeleton != null && !skeleton.equals("human"))
		{
			doAnimationAction(npc, "eat");
			pclib.coupDeGrace(target, npc);
		}
		else
		{
			pclib.coupDeGrace(target, npc, true);
		}
	}
	
	
	public int OnFleeTargetLost(obj_id self, obj_id target) throws InterruptedException
	{
		if (!ai_lib.isInCombat(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		clog("OnFleeTargetLost() self("+ self + ":"+ getName(self) + ") target("+ target + ")");
		
		stopCombat(self);
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int OnFleeWaypoint(obj_id self, obj_id target) throws InterruptedException
	{
		if (!ai_lib.isInCombat(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int OnFleePathNotFound(obj_id self, obj_id target) throws InterruptedException
	{
		if (!ai_lib.isInCombat(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		clog("OnFleePathNotFound() self("+ self + ":"+ getName(self) + ") target("+ target + ")");
		
		stopCombat(self);
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int OnFollowWaiting(obj_id self, obj_id target) throws InterruptedException
	{
		
		if (!ai_lib.isInCombat(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar(self, "ai.combat.pathingToHeal"))
		{
			
			obj_id healingTarget = utils.getObjIdScriptVar(self, "ai.combat.pathingToHeal");
			
			ai_combat.executeHealingMove(self, healingTarget);
			return SCRIPT_OVERRIDE;
		}
		
		faceTo(self, target);
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int OnFollowMoving(obj_id self, obj_id followTarget) throws InterruptedException
	{
		if (!ai_lib.isInCombat(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar(self, "ai.combat.pathingToHeal"))
		{
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int OnFollowTargetLost(obj_id self, obj_id target) throws InterruptedException
	{
		if (!ai_lib.isInCombat(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		clog("OnFollowTargetLost() self("+ self + ":"+ getName(self) + ") target("+ target + ")");
		
		stopCombat(self);
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int OnFollowPathNotFound(obj_id self, obj_id target) throws InterruptedException
	{
		if (!ai_lib.isInCombat(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		clog("OnFollowPathNotFound() self("+ self + ":"+ getName(self) + ") target("+ target + ")");
		
		stopCombat(self);
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int OnMovePathComplete(obj_id self) throws InterruptedException
	{
		if (!ai_lib.isInCombat(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		clog("OnMovePathComplete() self("+ self + ") The AI is dead.");
		
		if (ai_combat_movement.aiIsMovingToSee())
		{
			ai_combat_movement.aiClearMoveMode();
			ai_combat_movement.aiMoveToSee(ai_combat_movement.aiGetMoveToSeeTarget());
		}
		else if (ai_combat_movement.aiIsMovingToDeathBlow())
		{
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnMoveMoving(obj_id self) throws InterruptedException
	{
		if (!ai_lib.isInCombat(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		clog("OnMoveMoving() self("+ self + ") The AI is dead.");
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int OnMovePathNotFound(obj_id self) throws InterruptedException
	{
		if (!ai_lib.isInCombat(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (ai_combat_movement.aiIsMovingToSee())
		{
			final obj_id target = ai_combat_movement.aiGetMoveToSeeTarget();
			
			clog("OnMovePathNotFound() self("+ self + ") The AI was moving to see("+ target + ":"+ getName(target) + ") and something went wrong so forcing tether.");
			
			stopCombat(self);
		}
		else if (ai_combat_movement.aiIsMovingToDeathBlow())
		{
			
		}
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int handleMoveRandomClear(obj_id self, dictionary params) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTaunt(obj_id self, dictionary params) throws InterruptedException
	{
		if (ai_lib.isInCombat(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id target = params.getObjId("player");
		
		if (!isIdValid(target))
		{
			return SCRIPT_CONTINUE;
		}
		
		utils.setScriptVar(self, "ai.combat.isTaunted", (getGameTime() + 20));
		
		stop(self);
		
		setTarget(self, target);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int getConfusionDuration(obj_id npc, obj_id attacker) throws InterruptedException
	{
		
		if (getDistance(npc, attacker) < 10.0f)
		{
			return 0;
		}
		
		int attackerPosture = getPosture(attacker);
		int duration = 0;
		
		if (attackerPosture == POSTURE_CROUCHED)
		{
			duration = 5;
		}
		else if (attackerPosture == POSTURE_PRONE)
		{
			duration = 10;
		}
		
		if (!ai_lib.isMonster(npc))
		{
			return duration;
		}
		
		boolean masked = false;
		
		if (scout.isScentMasked(attacker, npc))
		{
			masked = true;
			duration += 7;
		}
		
		return duration;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		clog("OnDestroy() unit("+ self + ":"+ getName(self) + ")");
		
		if (ai_lib.isAiDead(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		int combatDestructionDelay = 0;
		
		if (pet_lib.isPet(self) || beast_lib.isBeast(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isInCombat(self))
		{
			combatDestructionDelay = getGameTime() + 30;
			utils.setScriptVar(self, "combatDestructionDelay", combatDestructionDelay);
			
			messageTo(self, "handleCombatDestructionDelay", null, 60, false);
			
			return SCRIPT_OVERRIDE;
		}
		
		if (!utils.hasScriptVar(self, "combatDestructionDelay"))
		{
			return SCRIPT_CONTINUE;
		}
		else
		{
			combatDestructionDelay = utils.getIntScriptVar(self, "combatDestructionDelay");
		}
		
		if (getGameTime() > combatDestructionDelay)
		{
			return SCRIPT_CONTINUE;
		}
		
		messageTo(self, "handleCombatDestructionDelay", null, 30, false);
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int handleCombatDestructionDelay(obj_id self, dictionary params) throws InterruptedException
	{
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDotPulse(obj_id self, dictionary params) throws InterruptedException
	{
		clog("OnDotPulse() self("+ self + ":"+ getName(self) + ")");
		
		if (ai_lib.isInCombat(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isAiDead(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (pet_lib.isPet(self) || beast_lib.isBeast(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] players = pvpGetEnemiesInRange(self, self, 128.0f);
		
		if (players == null || players.length < 1)
		{
			return SCRIPT_CONTINUE;
		}
		
		for (int i=0; i < players.length; i++)
		{
			testAbortScript();
			if (isIdValid(players[i]) && exists(players[i]) && players[i] != self)
			{
				startCombat(self, players[i]);
				return SCRIPT_CONTINUE;
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnIncapacitateTarget(obj_id self, obj_id target) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			clog("OnIncapacitateTarget() self("+ self + ":"+ getName(self) + ") target("+ target + ":"+ getName(target) + ") INVALID TARGET");
			return SCRIPT_CONTINUE;
		}
		
		if (!isPlayer(target) && !pet_lib.isPet(target) && !beast_lib.isBeast(target))
		{
			clog("OnIncapacitateTarget() self("+ self + ":"+ getName(self) + ") target("+ target + ":"+ getName(target) + ") TARGET IS NON-PLAYER");
			return SCRIPT_CONTINUE;
		}
		
		if (aiIsKiller(self))
		{
			
			if (pet_lib.isPet(target))
			{
				clog("OnIncapacitateTarget() self("+ self + ":"+ getName(self) + ") target("+ target + ":"+ getName(target) + ") DEATH BLOW PET");
				pet_lib.killPet(target);
			}
			else if (beast_lib.isBeast(target))
			{
				clog("OnIncapacitateTarget() self("+ self + ":"+ getName(self) + ") target("+ target + ":"+ getName(target) + ") DEATH BLOW BEAST");
				beast_lib.killBeast(target, self);
			}
			else
			{
				clog("OnIncapacitateTarget() self("+ self + ":"+ getName(self) + ") target("+ target + ":"+ getName(target) + ") DEATH BLOW PLAYER");
				pclib.coupDeGrace(target, self);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnIncapacitated(obj_id self, obj_id killer) throws InterruptedException
	{
		obj_id[] myHateList = getHateList(self);
		
		if (myHateList == null || myHateList.length == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		for (int i=0; i<myHateList.length; i++)
		{
			testAbortScript();
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int handleAggroCheck(obj_id self, dictionary params) throws InterruptedException
	{
		final obj_id target = params.getObjId("target");
		
		if (!isIdValid(target) || !exists(target) || isDead(target))
		{
			return SCRIPT_CONTINUE;
		}
		
		ai_aggro.stopAttemptingAggroCheck(target);
		ai_aggro.requestAggroCheck(target);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleAggroStart(obj_id self, dictionary params) throws InterruptedException
	{
		if (beast_lib.isBeast(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		final obj_id target = params.getObjId("target");
		
		if (isIdValid(target) && exists(target) && !isDead(target))
		{
			if (!stealth.hasServerCoverState(target) && canSee(self, target))
			{
				startCombatWithAssist(self, target);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSawRecapacitation(obj_id self, dictionary params) throws InterruptedException
	{
		clog("handleSawRecapacitation() self("+ self + ":"+ getName(self) + ")");
		
		final obj_id player = params.getObjId("player");
		
		stopListeningToMessage(player, "handleSawRecapacitation");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void clearCombatActions() throws InterruptedException
	{
		obj_id self = getSelf();
		
		removeObjVar(self, "ai.combat.forcedAction");
		removeObjVar(self, "ai.combat.pendingAction");
	}
	
	
	public int checkForSpecials(obj_id self, dictionary params) throws InterruptedException
	{
		if (!verifyMessage(self, params))
		{
			return SCRIPT_CONTINUE;
		}
		
		String bestBeastAbility = beast_lib.getBestAutoRepeatAbility(self);
		
		obj_id target = getTarget(self);
		
		if (!isIdValid(target))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!bestBeastAbility.equals(""))
		{
			queueCommand(getMaster(self), getStringCrc(bestBeastAbility), target, "", COMMAND_PRIORITY_DEFAULT);
		}
		
		messageTo(self, "checkForSpecials", stampMessage(self), 1.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public dictionary stampMessage(obj_id self) throws InterruptedException
	{
		int session = getMessageSession(self);
		utils.setScriptVar(self, "messageStamp", session);
		dictionary dict = new dictionary();
		dict.put("sessionId", session);
		
		return dict;
	}
	
	
	public int getMessageSession(obj_id self) throws InterruptedException
	{
		return utils.hasScriptVar(self, "messageStamp") ? utils.getIntScriptVar(self, "messageStamp") : 0;
	}
	
	
	public boolean verifyMessage(obj_id self, dictionary params) throws InterruptedException
	{
		int messageId = params.getInt("sessionId");
		int currentId = utils.getIntScriptVar(self, "messageStamp");
		return messageId == currentId;
	}
	
	
	public void bumpSession(obj_id self) throws InterruptedException
	{
		int session = getMessageSession(self);
		session++;
		utils.setScriptVar(self, "messageStamp", session);
	}
	
	
	public int checkForBeastSpecialsTakeOne(obj_id self, dictionary params) throws InterruptedException
	{
		if (beast_lib.isBeast(self))
		{
			messageTo(self, "checkForSpecials", stampMessage(self), 1.0f, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int persist_combat(obj_id self, dictionary params) throws InterruptedException
	{
		if (!trial.verifySession(self, params, combat.PERSIST_COMBAT))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!combat.isValidPersistCombat(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		resetHateTimer(self);
		
		messageTo(self, "persist_combat", trial.getSessionDict(self, combat.PERSIST_COMBAT), 5.0f, false);
		return SCRIPT_CONTINUE;
	}
}
