package com.sony.soe.swg.cts;

import java.util.ArrayList;

/**
 * Holds all of the object data for a SWG object.
 * 
 * @author bmauer
 *
 */
public class SWGObject
{
    /**
     * Object id of the object.
     */
    private long _id;
    /**
     * Name of the object.
     */
    private String _name;
    /**
     * Template id of the object (if any).
     */
    private long _templateId;
    /**
     * Template name of the object (if any).
     */
    private String _templateName;
    /**
     * Objects contained by this object.
     */
    private ArrayList _contains;
    /**
     * Objvars set for this object.
     */
    private ArrayList _objvars;
    
    /**
     * Initialize all data structures for this object, and set the id
     * to the given id.
     * @param id object id of the object
     */
    public SWGObject( long id )
    {
        _id = id;
        _name = "";
        _contains = new ArrayList();
        _objvars = new ArrayList();
        _templateId = -1;
        _templateName = "";
    }
    
    public long getId()
    {
        return _id;
    }
    
    public String getName()
    {
        return _name;
    }
    
    public void setName( String name )
    {
        _name = name;
    }
    
    public long getTemplateId()
    {
        return _templateId;
    }
    
    public void setTemplateId( long id )
    {
        _templateId = id;
    }
    
    public String getTemplateName()
    {
        return _templateName;
    }
    
    public void setTemplateName( String name )
    {
        _templateName = name;
    }
    
    public void addContains( SWGObject obj )
    {
        _contains.add( obj );
    }
    
    public SWGObject[] getContains()
    {
        SWGObject ret[] = new SWGObject[_contains.size()];
        _contains.toArray( ret );
        return ret;
    }
    
    public void addObjvar( SWGObjectVariable objvar )
    {
        _objvars.add( objvar );
    }
    
    public SWGObjectVariable[] getObjvars()
    {
        SWGObjectVariable ret[] = new SWGObjectVariable[ _objvars.size() ];
        _objvars.toArray( ret );
        return ret;
    }
    
    public int containsCount()
    {
        return _contains.size();
    }
    
    public int objvarCount()
    {
        return _objvars.size();
    }
}
