package com.sony.soe.swg.cts;

import java.util.ArrayList;

/**
 * Holds all of the object and character specific information of a character object.
 * 
 * @author bmauer
 *
 */
public class SWGCharacterObject extends SWGObject
{
    /**
     * Biography of the character.
     */
    private String _biography;
    /**
     * House objects owned by the character.
     */
    private ArrayList _houses;
    /**
     * Installation objects owned by the character.
     */
    private ArrayList _installations;
    /**
     * Harvester objects owned by the character.
     */
    private ArrayList _harvesters;
    
    /**
     * Initialize the object and character data structures.
     * @param object_id
     */
    public SWGCharacterObject( long object_id )
    {
        super( object_id );
        _biography = "";
        _houses = new ArrayList();
        _installations = new ArrayList();
        _harvesters = new ArrayList();
    }
    
    public String getBiography()
    {
        return _biography;
    }
    
    public void setBiography( String biography )
    {
        _biography = biography;
    }
    
    public void addHouse( SWGObject house )
    {
        _houses.add( house );
    }
    
    public SWGObject[] getHouses()
    {
        SWGObject ret[] = new SWGObject[_houses.size()];
        _houses.toArray( ret );
        return ret;
    }
    
    public void addInstallation( SWGObject installation )
    {
        _installations.add( installation );
    }
    
    public SWGObject[] getInstallations()
    {
        SWGObject ret[] = new SWGObject[_installations.size()];
        _installations.toArray( ret );
        return ret;
    }
    
    public void addHarvester( SWGObject harvester )
    {
        _harvesters.add( harvester );
    }
    
    public SWGObject[] getHarvesters()
    {
        SWGObject ret[] = new SWGObject[_harvesters.size()];
        _harvesters.toArray( ret );
        return ret;
    }
}
