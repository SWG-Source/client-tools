package script.player;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.badge;
import script.library.collection;
import script.library.prose;
import script.library.utils;


public class player_collection extends script.base_script
{
	public player_collection()
	{
	}
	public static final string_id SID_SLOT_ADDED = new string_id("collection","player_slot_added");
	public static final string_id SID_HIDDEN_SLOT = new string_id("collection","player_hidden_slot_added");
	public static final string_id SID_COLLECTION_COMPLETE = new string_id("collection","player_collection_complete");
	public static final string_id SID_SERVER_FIRST = new string_id("collection","player_server_first");
	
	public static final String SERVER_FIRST_SLOT = "bdg_server_first_01";
	
	
	public int OnCollectionSlotModified(obj_id self, String bookName, String pageName, String collectionName, String slotName, boolean isCounterTypeSlot, int previousValue, int currentValue, int maxSlotValue, boolean slotCompleted) throws InterruptedException
	{
		
		if (bookName.equals(badge.BADGE_BOOK))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] catagories = getCollectionSlotCategoryInfo(slotName);
		if (catagories != null && catagories.length > 0)
		{
			
			for (int i = 0; i < catagories.length; ++i)
			{
				testAbortScript();
				if (catagories[i].equals(collection.REWARD_ON_UPDATE_CATEGORY))
				{
					collection.grantCollectionReward(self, slotName);
					break;
				}
			}
		}
		
		if (slotCompleted)
		{
			boolean giveMessage = true;
			
			if (catagories != null && catagories.length > 0)
			{
				for (int i = 0; i < catagories.length; ++i)
				{
					testAbortScript();
					if (catagories[i].equals(collection.REWARD_ON_COMPLETE_CATEGORY))
					{
						collection.grantCollectionReward(self, slotName);
						break;
					}
				}
				for (int i = 0; i < catagories.length; ++i)
				{
					testAbortScript();
					if (catagories[i].equals(collection.NO_MESSAGE_CATEGORY))
					{
						giveMessage = false;
						break;
					}
				}
			}
			
			prose_package pp = new prose_package();
			
			int row = dataTableSearchColumnForString(slotName, "slotName", collection.COLLECTION_TABLE);
			if (row < 0)
			{
				
				return SCRIPT_CONTINUE;
			}
			
			int isHidden = dataTableGetInt(collection.COLLECTION_TABLE, row, "hidden");
			if (isHidden > 0 && giveMessage)
			{
				prose.setStringId(pp, SID_HIDDEN_SLOT);
				prose.setTU(pp, new string_id("collection_n", slotName));
				prose.setTO(pp, new string_id("collection_n", collectionName));
				sendSystemMessageProse(self, pp);
				play2dNonLoopingMusic(self, "sound/utinni.snd");
			}
			else if (giveMessage)
			{
				prose.setStringId(pp, SID_SLOT_ADDED);
				prose.setTU(pp, new string_id("collection_n", slotName));
				prose.setTO(pp, new string_id("collection_n", collectionName));
				sendSystemMessageProse(self, pp);
				play2dNonLoopingMusic(self, "sound/utinni.snd");
			}
			
			if (hasCompletedCollection(self, collectionName))
			{
				
				prose.setTO(pp, new string_id("collection_n", collectionName));
				prose.setStringId(pp, SID_COLLECTION_COMPLETE);
				sendSystemMessageProse(self, pp);
				collection.grantCollectionReward(self, collectionName);
			}
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnCollectionServerFirst(obj_id self, String bookName, String pageName, String collectionName) throws InterruptedException
	{
		
		prose_package pp = new prose_package();
		
		prose.setStringId(pp, SID_SERVER_FIRST);
		prose.setTO(pp, new string_id("collection_n", collectionName));
		prose.setTT(pp, getClusterName());
		sendSystemMessageProse(self, pp);
		
		if (!badge.hasBadge(self, SERVER_FIRST_SLOT))
		{
			badge.grantBadge(self, SERVER_FIRST_SLOT);
		}
		
		return SCRIPT_CONTINUE;
	}
	
}
