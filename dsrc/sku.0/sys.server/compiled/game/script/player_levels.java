/**************************************************************************
 *
 * Java-side player_level caching.
 *
 * copyright 2001 Sony Online Entertainment
 *
 * @todo: where do error messages from this class go? Right now we're just
 * printing to stderr, but do we want to hook in with our logging code?
 *
 *************************************************************************/

package script;

import script.base_class;
import java.util.Hashtable;

public class player_levels
{
	/**
	 * player_levels constants
	 */

	private static Hashtable m_PlayerLevelDataCache = new Hashtable();
	private static Hashtable m_SkillTemplateDataCache = new Hashtable();

	/*********************************************************************/

	public static level_data getPlayerLevelData(String profession, int level)
	{
		if(level < 1)
		{
			level = 1;
		}
		else
		{
			if(level > 90)
			{
				level = 90;
			}
		}

		if(profession == null || profession.length() <= 0)
		{
			return null;
		}

		if(m_PlayerLevelDataCache.containsKey(profession + level))
		{
			return level_data.clone((level_data)m_PlayerLevelDataCache.get(profession + level));
		}

		level_data dat = loadLevelData(profession, level);

		if(dat != null)
		{
			m_PlayerLevelDataCache.put(profession + level, dat);
		}
		else
		{
			return null;
		}

		return level_data.clone(dat);
	}

	private static level_data loadLevelData(String profession, int level)
	{
		level_data dat = new level_data();

		if(level < 0)
		{
			level = 1;
		}
		else
		{
			if(level > 90)
			{
				level = 90;
			}
		}

		dictionary d = base_class.dataTableGetRow("datatables/skill/levels.iff", level - 1);

		if(d == null)
		{
			return null;
		}

		dat.action			= d.getInt(profession + "_action");
		dat.action_regen	= d.getFloat(profession + "_action_regen");
		dat.agility			= d.getInt(profession + "_agility");
		dat.constitution	= d.getInt(profession + "_constitution");
		dat.health			= d.getInt(profession + "_health");
		dat.health_regen	= d.getFloat(profession + "_health_regen");
		dat.luck			= d.getInt(profession + "_luck");
		dat.precision		= d.getInt(profession + "_precision");
		dat.stamina			= d.getInt(profession + "_stamina");
		dat.strength		= d.getInt(profession + "_strength");
		dat.willpower		= d.getInt(profession + "_willpower");

		return dat;
	}

	/*********************************************************************/

	public static class level_data implements java.lang.Cloneable
	{
		public static level_data clone(level_data template)
		{
			try
			{
				level_data pl = (level_data)template.clone();
				return pl;
			}
			catch(java.lang.CloneNotSupportedException exc)
			{
				return null;
			}
		}

		public int action			= 0;
		public float action_regen	= 0.0f;
		public int agility			= 0;
		public int constitution		= 0;
		public int health			= 0;
		public float health_regen	= 0.0f;
		public int luck				= 0;
		public int precision		= 0;
		public int stamina			= 0;
		public int strength			= 0;
		public int willpower		= 0;
	}




	/*********************************************************************/

	public static skill_template_data getSkillTemplateData(String professionTemplate)
	{
		if(professionTemplate == null || professionTemplate.length() <= 0)
		{
			return null;
		}

		if(m_SkillTemplateDataCache.containsKey(professionTemplate))
		{
			return skill_template_data.clone((skill_template_data)m_SkillTemplateDataCache.get(professionTemplate));
		}

		skill_template_data dat = loadSkillTemplateData(professionTemplate);

		if(dat != null)
		{
			m_SkillTemplateDataCache.put(professionTemplate, dat);
		}
		else
		{
			return null;
		}

		return skill_template_data.clone(dat);
	}

	private static skill_template_data loadSkillTemplateData(String professionTemplate)
	{
		skill_template_data dat = new skill_template_data();

		dictionary d = base_class.dataTableGetRow("datatables/skill_template/skill_template.iff", professionTemplate);

		if(d == null)
		{
			return null;
		}

		dat.startingTemplateName	= d.getString("startingTemplateName");
		dat.strClassName			= d.getString("strClassName");
		dat.userInterfacePriority	= d.getInt("userInterfacePriority");
		dat.professionTemplate		= d.getString("template");
		dat.levelBased				= d.getInt("levelBased");
		dat.expertiseTrees			= d.getString("expertiseTrees");

		return dat;
	}

	/*********************************************************************/

	public static class skill_template_data implements java.lang.Cloneable
	{
		public static skill_template_data clone(skill_template_data template)
		{
			try
			{
				skill_template_data pl = (skill_template_data)template.clone();
				return pl;
			}
			catch(java.lang.CloneNotSupportedException exc)
			{
				return null;
			}
		}

		public String startingTemplateName	= "";
		public String strClassName			= "";
		public int userInterfacePriority	= 0;
		public String professionTemplate	= "";
		public int levelBased				= 0;
		public String expertiseTrees		= "";
	}

}	// class player_levels



