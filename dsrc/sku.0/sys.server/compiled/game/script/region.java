// ======================================================================
//
// region.java
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

package script;

public class region
{
	/**
	 * Retrieve the name of the region.
	 *
	 * @return the name of the region.
	 */
	public String getName()
	{
		return m_name;
	}

	/**
	 * Retrieve the value representing the PvP state.
	 *
	 * @return the value representing the PvP state.
	 */
	public int getPvPType()
	{
		return m_PvPType;
	}

	/**
	 * Retrieve the value representing the municipal state.
	 *
	 * @return the value representing the municipal state.
	 */
	public int getMunicipalType()
	{
		return m_municipalType;
	}

	/**
	 * Retrieve the value representing the buildable state.
	 *
	 * @return the value representing the buildable state.
	 */
	public int getBuildableType()
	{
		return m_buildableType;
	}

	/**
	 * Retrieve the value representing the geographical state.
	 *
	 * @return the value representing the geographical state.
	 */
	public int getGeographicalType()
	{
		return m_geographicalType;
	}

	/**
	 * Retrieve the value representing the minimum difficulty state.
	 *
	 * @return the value representing the minimum difficulty state.
	 */
	public int getMinDifficultyType()
	{
		return m_minDifficultyType;
 	}

	/**
	 * Retrieve the value representing the max difficulty state.
	 *
	 * @return the value representing the max difficulty state.
	 */
	public int getMaxDifficultyType()
	{
		return m_maxDifficultyType;
 	}

	/**
	 * Retrieve the value representing the spawnable state.
	 *
	 * @return the value representing the spawnable state.
	 */
	public int getSpawnableType()
 	{
 		return m_spawnableType;
 	}

	/**
	 * Retrieve the value representing the mission state.
	 *
	 * @return the value representing the mission state.
	 */
	public int getMissionType()
	{
 		return m_missionType;
	}

	/**
	 * Retrieve the value representing the planet name.
	 *
	 * @return the value representing the planet name.
	 */
	public String getPlanetName()
	{
 		return m_planetName;
	}

	/**
	 * Retrieve a String representation of the instance suitable for
	 * a debug dump.
	 *
	 * The caller should not assume anything about the format of this
	 * output.  It may change at any time.
	 *
	 * @return  a String representation of the instance suitable for
	 *          a debug dump.
	 */
	public String toString()
	{
		return "[region: name = " + getName() + ", PvPType = " + getPvPType() + ", municipalType = " + getMunicipalType() +
			", buildableType = " + getBuildableType() + ", geographicalType = " + getGeographicalType() + ", minDifficultyType = " +
			getMinDifficultyType() + ", maxDifficultyType = " + getMaxDifficultyType() + ", spawnableType = " + getSpawnableType() +
			", missionType = " + getMissionType() + ", planetName = " + m_planetName + "]";
	}

	/**
	 * Construct a region instance.
	 *
	 * Scripters should not try to construct these by hand.
	 *
	 * @param objId      the obj_id for the Object containing the region.
	 *
	 */
	protected region(String name, int pvpType, int buildableType, int municipalType, int geographicalType, int minDifficultyType, int maxDifficultyType, int spawnableType, int missionType, String planetName)
	{
		m_name              = name;
		m_PvPType           = pvpType;
		m_buildableType     = buildableType;
		m_municipalType     = municipalType;
		m_geographicalType  = geographicalType;
		m_minDifficultyType = minDifficultyType;
		m_maxDifficultyType = maxDifficultyType;
		m_spawnableType     = spawnableType;
		m_missionType       = missionType;
		m_planetName        = planetName;
	}

	private final String  m_name;
	private final int     m_PvPType;
	private final int     m_buildableType;
	private final int     m_municipalType;
	private final int     m_geographicalType;
	private final int     m_minDifficultyType;
	private final int     m_maxDifficultyType;
	private final int     m_spawnableType;
	private final int     m_missionType;
	private final String  m_planetName;
};
