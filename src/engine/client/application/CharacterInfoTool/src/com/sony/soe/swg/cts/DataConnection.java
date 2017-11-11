package com.sony.soe.swg.cts;

import java.io.PrintStream;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.Properties;

/**
 * Holds a connection to the database, and gets objects and objvars for objects in 
 * the SWG database.  Gets more information for character objects.
 * 
 * @author bmauer
 *
 */
public class DataConnection
{
    /**
     * Name of the TNSNAMES entry for the database. 
     */
    private String _db;
    /**
     * Username for the database.
     */
    private String _username;
    /**
     * Password for the username.
     */
    private String _password;
    /**
     * Connection to the database.
     */
    private Connection _connection;
    
    /**
     * Save the values for a connection to the DB.  Does not create a connection.
     * @param db TNSNAMES entry for the database
     * @param username Username for the database
     * @param password Password for the username
     */
    public DataConnection( String db, String username, String password )
    {
        _db = db;
        _username = username;
        _password = password;
    }
    
    /**
     * Connect to the database using the db, username, and password
     * Given in the constructor.
     */
    public void init()
    {
        try
        {
            DriverManager.registerDriver( new oracle.jdbc.OracleDriver() );
            Properties info = new Properties();
            info.put( "user", _username );
            info.put( "password", _password );
            _connection = DriverManager.getConnection( "jdbc:oracle:oci8:@" + _db, info );
        }
        catch ( SQLException ex )
        {
            ex.printStackTrace();
        }
    }
    
    /**
     * Recursive call to get all objects and objvars associated with the
     * object given.
     * @param obj Object to get objects and objvars for
     * @return obj with objects and objvars contained
     */
    private SWGObject getAllObjects( SWGObject obj )
    {
        long id = obj.getId();
        PreparedStatement pstmt = null;
        ResultSet results = null;
        try
        {
            String sql = "SELECT ot.id id, ot.name name " +
                         "FROM objects o, object_templates ot " +
                         "WHERE o.object_id = ? " +
                         "  AND o.object_template_id = ot.id";
            pstmt = _connection.prepareStatement( sql );
            pstmt.setLong( 1, id );
            results = pstmt.executeQuery();
            if ( results.next() )
            {
                obj.setTemplateId( results.getLong( "id" ) );
                obj.setTemplateName( results.getString( "name" ) );
            }
            close( results, pstmt );
            
            sql = "SELECT o.name name, o.type type, o.value value " +
                  "FROM object_variables_view o " +
                  "WHERE o.object_id = ?";
            pstmt = _connection.prepareStatement( sql );
            pstmt.setLong( 1, id );
            results = pstmt.executeQuery();
            while ( results.next() )
            {
                SWGObjectVariable objvar = new SWGObjectVariable( results.getString( "name"), results.getLong( "type" ), results.getString( "value" ) );
                obj.addObjvar( objvar );
            }
            close( results, pstmt );

            sql = "SELECT o.object_name name " +
                  "FROM objects o " +
                  "WHERE o.object_id = ?";
            pstmt = _connection.prepareStatement( sql );
            pstmt.setLong( 1, id );
            results = pstmt.executeQuery();
            if ( results.next() )
            {
                obj.setName( results.getString( "name" ) );
            }
            close( results, pstmt );
            
            ArrayList objectIds = new ArrayList();
            sql = "SELECT o.object_id object_id " +
                  "FROM objects o " +
                  "WHERE o.contained_by = ? " +
                  "  AND o.deleted = 0";
            pstmt = _connection.prepareStatement( sql );
            pstmt.setLong( 1, id );
            results = pstmt.executeQuery();
            while ( results.next() )
            {
                objectIds.add( new Long(results.getLong( "object_id" ) ) );
            }
            close( results, pstmt );
            
            for ( int i = 0; i < objectIds.size(); i++ )
            {
                obj.addContains( getAllObjects( new SWGObject( ((Long)objectIds.get( i )).longValue() ) ) );
            }
        }
        catch ( SQLException ex )
        {
            ex.printStackTrace();
        }
        finally
        {
            close( results, pstmt );
        }
        return obj;
    }
    
    /**
     * Get the data associated with the character object found
     * using the given id.
     * @param id id of the object representing the character
     * @return A SWGCharacterObject containing all information for the character
     */
    public SWGCharacterObject loadCharacter( long id )
    {
        SWGCharacterObject ret = (SWGCharacterObject)getAllObjects( new SWGCharacterObject( id ) );
        PreparedStatement pstmt = null;
        ResultSet results = null;
        try
        {   
            String sql = "SELECT b.biography biography " + 
                         "FROM biographies b " +
                         "WHERE object_id = ?";
            pstmt = _connection.prepareStatement( sql );
            pstmt.setLong( 1, id );
            results = pstmt.executeQuery();
            if ( results.next() )
            {
               ret.setBiography( results.getString( "biography" ) );
            }
            close( results, pstmt );
            
            ArrayList buildingIds = new ArrayList();
            sql = "SELECT o.object_id object_id " +
                  "FROM objects o, tangible_objects t, building_objects b " +
                  "WHERE t.owner_id = ? " +
                  "  AND t.object_id = o.object_id " +
                  "  AND o.object_id = b.object_id " +
                  "  AND o.deleted = 0";
            pstmt = _connection.prepareStatement( sql );
            pstmt.setLong( 1, id );
            results = pstmt.executeQuery();
            while ( results.next() )
            {
                buildingIds.add( new Long( results.getLong( "object_id" ) ) );
            }
            close( results, pstmt );
            
            for ( int i = 0; i < buildingIds.size(); i++ )
            {
                ret.addHouse( getAllObjects( new SWGObject( ((Long)buildingIds.get( i )).longValue() ) ) );
            }
            
            buildingIds = new ArrayList();
            sql = "SELECT o.object_id object_id " +
                  "FROM objects o, tangible_objects t, installation_objects i " +
                  "WHERE t.owner_id = ? " +
                  "  AND t.object_id = o.object_id " +
                  "  AND o.object_id = i.object_id " +
                  "  AND o.deleted = 0";
            pstmt = _connection.prepareStatement( sql );
            pstmt.setLong( 1, id );
            results = pstmt.executeQuery();
            while ( results.next() )
            {
                buildingIds.add( new Long( results.getLong( "object_id" ) ) );
            }
            close( results, pstmt );
            
            for ( int i = 0; i < buildingIds.size(); i++ )
            {
                ret.addInstallation( getAllObjects( new SWGObject( ((Long)buildingIds.get( i )).longValue() ) ) );
            }
            
            buildingIds = new ArrayList();
            sql = "SELECT o.object_id object_id " +
                  "FROM objects o, tangible_objects t, harvester_installation_objects i " +
                  "WHERE t.owner_id = ? " +
                  "  AND t.object_id = o.object_id " +
                  "  AND o.object_id = i.object_id " +
                  "  AND o.deleted = 0";
            pstmt = _connection.prepareStatement( sql );
            pstmt.setLong( 1, id );
            results = pstmt.executeQuery();
            while ( results.next() )
            {
                buildingIds.add( new Long( results.getLong( "object_id" ) ) );
            }
            close( results, pstmt );
            
            for ( int i = 0; i < buildingIds.size(); i++ )
            {
                ret.addHarvester( getAllObjects( new SWGObject( ((Long)buildingIds.get( i )).longValue() ) ) );
            }
        }
        catch ( SQLException ex )
        {
            ex.printStackTrace();
        }
        finally
        {
            close( results, pstmt );
        }
        return ret;
    }

    /**
     * Straight dump of the character object data to the given stream.
     * @param obj Object to be dumped
     * @param stream Stream to dump object to
     */
    public void outputObjects( SWGCharacterObject obj, PrintStream stream )
    {
        internalOutputObjects( obj, stream, "" );
        stream.println( "  BIOGRAPHY:" );
        stream.println( obj.getBiography() );
        stream.println( "  HOUSES:" );
        SWGObject objs[] = obj.getHouses();
        for ( int i = 0; i < objs.length; i++ )
        {
            internalOutputObjects( objs[i], stream, "\t" );
        }
        stream.println( "  INSTALLATIONS:" );
        objs = obj.getInstallations();
        for ( int i = 0; i < objs.length; i++ )
        {
            internalOutputObjects( objs[i], stream, "\t" );
        }
        stream.println( "  HARVESTERS:" );
        objs = obj.getHarvesters();
        for ( int i = 0; i < objs.length; i++ )
        {
            internalOutputObjects( objs[i], stream, "\t" );
        }
    }
    
    /**
     * Straight, recursive dump of the object to the stream.  Tabs in recursions for
     * readability using depth.
     * @param obj Object to dump to stream
     * @param stream Stream to dump object to
     * @param depth Tabs to make recursion readable
     */
    public void internalOutputObjects( SWGObject obj, PrintStream stream, String depth )
    {
        stream.println( depth + obj.getName() + " (" + obj.getId() + ")" );
        if ( obj.getTemplateId() != -1 )
        {
            stream.println( depth + "  TEMPLATE:" );
            stream.println( depth + "    " + obj.getTemplateName() + " (" + obj.getTemplateId() + ")" );
        }
        SWGObjectVariable objvars[] = obj.getObjvars();
        if ( objvars.length > 0 )
        {
            stream.println( depth + "  OBJECT VARIABLES:" );
            for ( int i = 0; i < objvars.length; i++ )
            {
                stream.println( depth + "    " + objvars[i].getName() + "\t" + objvars[i].getType() + "\t" + objvars[i].getValue() );
            }
        }
        SWGObject objs[] = obj.getContains();
        if ( objs.length > 0 )
        {
            stream.println( depth + "  CONTAINED OBJECTS:" );
            for ( int i = 0; i < objs.length; i++ )
            {
                internalOutputObjects( objs[i], stream, depth + "\t" );
            }
        }
    }
    
    /**
     * Safely close the ResultSet and Statement.
     * @param results ResultSet to close
     * @param stmt Statement to close
     */
    private void close( ResultSet results, Statement stmt )
    {
        if ( results != null )
        {
            try
            {
                results.close();
            }
            catch ( SQLException ex )
            {
            }
        }
        if ( stmt != null )
        {
            try
            {
                stmt.close();
            }
            catch ( SQLException ex )
            {
            }
        }
    }
    
    /**
     * Close the DB connection.
     */
    public void disconnect()
    {
        if ( _connection != null )
        {
            try
            {
                _connection.close();
            }
            catch (Exception ex)
            {
            }
        }
    }

    /**
     * A 4 argument command-line call to the a dump to stdout of a character.  The arguments:
     *  - first is a TNSNAMES value for a db
     *  - second is the username to login with
     *  - third is the password for the username
     *  - fourth is the object id of the character
     * @param args 4 arguments used
     */
    public static void main( String args[] )
    {
        if ( args.length < 4 )
        {
            System.out.println( "Usage: java DataConnection <database> <username> <password> <object id>");
            System.exit(1);
        }
        DataConnection connection = new DataConnection( args[0], args[1], args[2] );
        connection.init();
        SWGCharacterObject value = connection.loadCharacter( Long.parseLong(args[3]) );
        connection.outputObjects( value, System.out );
        connection.disconnect();
    }
}
