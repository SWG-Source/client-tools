/**************************************************************************
 *
 * Java-side combat processing.
 *
 * copyright 2001 Sony Online Entertainment
 *
 * @todo: where do error messages from this class go? Right now we're just
 * printing to stderr, but do we want to hook in with our logging code?
 *
 *************************************************************************/

package script;

import script.location;
import script.obj_id;
import script.library.colors_hex;
import script.library.combat_consts;
import script.library.utils;
import script.base_class;
import java.util.Hashtable;
import java.util.Enumeration;

public class gcw_score
{
	private static Hashtable m_GcwScoreCache = new Hashtable();
	private static int invasionPhase = 0;
	private static int invasionWinner = 0;

	// Player GCW score data

	public static gcw_data getPlayerGcwData(obj_id player)
	{
		if(m_GcwScoreCache.containsKey("" + player))
		{
			return gcw_data.clone((gcw_data)m_GcwScoreCache.get("" + player));
		}

		return null;
	}

	public static String[][] getAllGcwData()
	{
		if(m_GcwScoreCache == null || m_GcwScoreCache.isEmpty())
		{
			return null;
		}
	
		int size = m_GcwScoreCache.size();
		String[][] allData = new String[m_GcwScoreCache.size()][10];
		Enumeration e = m_GcwScoreCache.elements();
		
		for(int i = 0; e.hasMoreElements(); i++)
		{
			gcw_data playerGcwData = (gcw_data)e.nextElement();
			
			if(playerGcwData != null && i < size)
			{
				allData[i][0] = playerGcwData.playerName;
				allData[i][1] = playerGcwData.playerFaction;
				allData[i][2] = playerGcwData.playerProfession;
				allData[i][3] = "" + playerGcwData.playerLevel;
				allData[i][4] = "" + playerGcwData.playerGCW;
				allData[i][5] = "" + playerGcwData.playerPvpKills;
				allData[i][6] = "" + playerGcwData.playerKills;
				allData[i][7] = "" + playerGcwData.playerAssists;
				allData[i][8] = "" + playerGcwData.playerCraftedItems;
				allData[i][9] = "" + playerGcwData.playerDestroyedItems;
			}
		}
	
		return allData;
	}

	public static void setGcwPhase(int phase)
	{
		invasionPhase = phase;
	}

	public static int getGcwPhase()
	{
		return invasionPhase;
	}
	
	public static void setGcwWinner(int winner)
	{
		invasionWinner = winner;
	}

	public static int getGcwWinner()
	{
		return invasionWinner;
	}

	public static void setPlayerGcwData(obj_id player, String playerName, String playerFaction, String playerProfession, int playerLevel, int playerGCW, int playerPvpKills, int playerKills, int playerAssists, int playerCraftedItems, int playerDestroyedItems)
	{
		gcw_data playerGcwData = new gcw_data();
		
		playerGcwData.playerName = playerName;
		playerGcwData.playerFaction = playerFaction;
		playerGcwData.playerProfession = playerProfession;
		playerGcwData.playerLevel = playerLevel;
		playerGcwData.playerGCW = playerGCW;
		playerGcwData.playerPvpKills = playerPvpKills;
		playerGcwData.playerKills = playerKills;
		playerGcwData.playerAssists = playerAssists;
		playerGcwData.playerCraftedItems = playerCraftedItems;
		playerGcwData.playerDestroyedItems = playerDestroyedItems;
		
		m_GcwScoreCache.put("" + player, playerGcwData);
	}

	public static void clearGcwData()
	{
		m_GcwScoreCache.clear();
		invasionPhase = 0;
		invasionWinner = 0;
	}

	public static class gcw_data implements java.lang.Cloneable
	{
		public static gcw_data clone(gcw_data template)
		{
			try
			{
				gcw_data cd = (gcw_data)template.clone();
				return cd;
			}
			catch(java.lang.CloneNotSupportedException exc)
			{
				return null;
			}
		}

		public String 	playerName 		= "";
		public String 	playerFaction		= "";
		public String	playerProfession	= "";
		public int	playerLevel		= 0;
		public int	playerGCW		= 0;
		public int	playerPvpKills		= 0;
		public int 	playerKills		= 0;
		public int 	playerAssists		= 0;
		public int 	playerCraftedItems	= 0;
		public int 	playerDestroyedItems	= 0;
	}
}	// class gcw_score



