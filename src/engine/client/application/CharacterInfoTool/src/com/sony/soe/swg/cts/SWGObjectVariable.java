package com.sony.soe.swg.cts;

/**
 * Holds the name, type, and value of an objvar.
 * @author bmauer
 *
 */
public class SWGObjectVariable
{
    /**
     * Name of the objvar.
     */
    private String  _name;
    /**
     * Type of the objvar.
     */
    private long    _type;
    /**
     * Value of the objvar.
     */
    private String  _value;
    
    /**
     * Creates a representation of an objvar with the given name, type, and value.
     * @param name Name of the objvar
     * @param type Type of the objvar
     * @param value Value of the objvar
     */
    public SWGObjectVariable( String name, long type, String value )
    {
        _name = name;
        _type = type;
        _value = value;
    }
    
    public String getName()
    {
        return _name;
    }
    
    public long getType()
    {
        return _type;
    }
    
    public String getValue()
    {
        return _value;
    }
}
