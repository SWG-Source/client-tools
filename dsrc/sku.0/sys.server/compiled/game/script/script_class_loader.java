/**
 * Title:        script_class_loader
 * Description:  Responsible for loading script classes.
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;

import java.io.File;
import java.io.RandomAccessFile;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.TreeSet;

public class script_class_loader extends ClassLoader
{
	// keep a dummy method object around to identify methods that are missing from scripts
	private static final Object NO_OBJECT = new Object();
	public static Method NO_METHOD = null;

	private String     myClassName;			// class this loader is responsible for
	private Class      myClass;				// class this loader has loaded
	private Object     myObject;            // object instance of our class
	private Hashtable  methods;				// cached methods for the class
	private ArrayList  derivedClasses;		// names of classes that derive from the class

	private static Hashtable loaderCache = new Hashtable(2003);		// class name -> class_loader map
	private static TreeSet defaultLoad;								// names of classes that should be loaded by the default loader

	/**
	 * Class constructor.
	 *
	 * @param name      name of class this loader is responsible for
	 */
    private script_class_loader(String name)  throws ClassNotFoundException
	{
//		System.out.println("Creating new class loader for class " + name);

		// initialize basic data
		myClassName = name;
		myClass = null;
		myObject = null;
		methods = new Hashtable(53);
		derivedClasses = null;

		loaderCache.put(name, this);

		// load the class and create an instance of it
		loadClass(name);
		if (myClass != null)
		{
			try
			{
				myObject = myClass.newInstance();
				if (myObject != null)
				{
//					System.out.println("Created new script_class_loader for class " + name + " (" + myClass.getName() + ")");
				}
				else
				{
					System.out.println("WARNING: Java Error creating object for class " + name);
				}
			}
			catch ( InstantiationException err )
			{
				System.err.println("WARNING: Java Error creating class instance " + name + " : " + err);
			}
			catch ( IllegalAccessException err )
  			{
				System.err.println("WARNING: Java Error creating class instance " + name + " : " + err);
			}
		}
		else
			System.err.println("WARNING: Java Error script_class_loader creating class " + name);
    }   // script_class_loader()

	/**
	 * Finds the class loader for a given class. If the class loader doesn't
	 * exist, creates it.
	 *
	 * @param name      name of class we want the loader for
	 */
	public static script_class_loader getClassLoader(String name) throws ClassNotFoundException
	{
		Object test = loaderCache.get(name);
		if (test != null)
			return (script_class_loader)test;

//		System.out.println("script_class_loader getClassLoader creating new loader for class " + name);

		if (name.indexOf("script.") != 0)
		{
			ClassNotFoundException err =  new ClassNotFoundException("Class " + name + " does not start with 'script.'");
			err.printStackTrace();
			throw err;
		}

		script_class_loader loader = new script_class_loader(name);
		if (loader != null && (loader.getMyClass() == null || loader.getMyObject() == null))
			loader = null;
		return loader;
	}	// getClassLoader()

	/**
	 * Removes a loader for a given script, so the class can be loaded again
	 *
	 * @param name      name of class whose loader we want to remove
	 *
	 * @return true if the class was unloaded, false if the class doesn't exist
	 */
	public static boolean unloadClass(String name)
	{
		System.out.println("script_class_loader unloadClass " + name);

		boolean result = false;
		if (loaderCache.containsKey(name))
		{
			script_class_loader loader = (script_class_loader)loaderCache.remove(name);
			// unload all the classes that derive from this one
			if (loader.derivedClasses != null)
			{
				int count = loader.derivedClasses.size();
				for (int i = 0; i < count; ++i)
				{
					unloadClass((String)loader.derivedClasses.get(i));
				}
				loader.derivedClasses.clear();
				loader.derivedClasses = null;
			}
			// unload the class methods
			if (loader.methods != null)
			{
				loader.methods.clear();
				loader.methods = null;
			}
			loader.myClass = null;
			loader.myObject = null;
			loader.myClassName = null;
			loader = null;
			result = true;
		}
		else
		{
			// if the class file exists, go ahead and return true
			String pathedName = name.replace('.', java.io.File.separatorChar);
			String fullname = script_entry.getScriptPath() + pathedName + ".class";
			File file = new File(fullname);
			if (file != null)
			{
				if (file.exists())
					result = true;
			}
		}
		return result;
	}	// unloadClass()

	/**
	 * Adds a derived class name to our derived classes list.
	 *
	 * @param className		the derived class name
	 */
	public void addDerivedClass(String className)
	{
		if (derivedClasses == null)
			derivedClasses = new ArrayList();
		derivedClasses.add(className);
	}	// addDerivedClass()

	/**
	 * Gets the cached class for this loader.
	 *
	 * @return the class
	 */
	public Class getMyClass()
	{
		return myClass;
	}	// getMyClass()

	/**
	 * Gets the cached object for a class.
	 *
	 * @return the object
	 */
	public Object getMyObject()
	{
	    return myObject;
	}	// getMyObject()

	/**
	 * Gets the cached methods for a class.
	 *
	 * @return the methods hash table
	 */
	public Hashtable getMyMethods()
	{
		return methods;
	}	// getMyMethods()

	/**
	 * Loads a class. If the class is the one this loader is responsible for,
	 * loads it, else passes the request to another loader.
	 *
	 * @param name      class name
	 * @param resolve   flag to resolve the class
	 *
	 * @return the class
	 */
    protected Class loadClass(String name, boolean resolve) throws ClassNotFoundException
    {
//		System.out.println("script_class_loader loadClass " + name);

		// filter out classes that will be loaded by the default loader
		int nameLength = name.length();
		if ((nameLength > 5 && name.substring(0, 5).compareTo("java.") == 0) ||
			(nameLength > 4 && name.substring(0, 4).compareTo("sun.") == 0) ||
			(nameLength > 10 && name.substring(0, 10).compareTo("intuitive.") == 0) ||
			defaultLoad.contains(name))
		{
//			System.out.println("****script_class_loader calling default loader**** class = " + name);
			Class cls = super.loadClass(name, resolve);
			if (myClass == null)
				myClass = cls;
			if (name.equals("java.lang.Throwable"))
			{

			}
			return cls;
		}

		// if the class we want to load is this class loader's responsibility,
		// load it, otherwise find/create a loader for the class
		if (name.equals(myClassName))
		{
			if (myClass != null)
				return myClass;

			// load the class from it's file
			Class c = findClass(name);
			if (c != null && resolve == true)
				resolveClass(c);
			return c;
		}
		else
		{
			// see if we already have a loader for the class we want
			script_class_loader newLoader = getClassLoader(name);
			if (newLoader != null)
				return newLoader.getMyClass();
		}
		System.err.println("WARNING: couldn't load class " + name + " in script_class_loader.loadClass");
		return null;
    }   // loadClass(String, boolean)

	/**
	 * Finds/creates a class.
	 *
	 * @param name      class name
	 *
	 * @return the class
	 */
	protected Class findClass(String name) throws ClassNotFoundException
	{
//		System.out.println("script_class_loader findClass " + name);
		if (myClass == null)
		{
			try
			{
				byte data[] = loadClassData(name);
			    myClass = defineClass(name, data, 0, data.length);

				// we need to keep track of all the superclasses of this class, in case one of them is reloaded
				for (Class superClass = myClass.getSuperclass(); superClass != null; superClass = superClass.getSuperclass())
				{
					Object superLoader = loaderCache.get(superClass.getName());
					if (superLoader != null)
						((script_class_loader)superLoader).addDerivedClass(name);
				}
			}
			catch (ClassFormatError err)
			{
				throw new ClassNotFoundException();
			}
		}
		return myClass;
	}   // findClass

	/**
	 * Loads a .class file from a file.
	 *
	 * @param name      name of the class to load
	 *
	 * @return byte array containing the class data
	 */
	private byte[] loadClassData(String name) throws ClassNotFoundException
	{
//		System.out.println("script_class_loader loadClassData enter: " + name);

		byte[] data = null;

		// if the script name has '.' in it, convert them to '\'
		String pathedName = name.replace('.', java.io.File.separatorChar);
		String fullname = script_entry.getScriptPath() + pathedName + ".class";
		System.out.println("script_class_loader loadClassData " + fullname);
		try
		{
			RandomAccessFile file = new RandomAccessFile(fullname, "r");
			data = new byte[(int)file.length()];
			file.read(data);
			file.close();
		}
		catch (FileNotFoundException err)
		{
			System.err.println("WARNING: " + err.toString());
			throw new ClassNotFoundException("file " + fullname + " not found");
		}
		catch (IOException err)
		{
			System.err.println("WARNING: " + err.toString());
			throw new ClassNotFoundException("error reading file " + fullname);
		}

		return data;
	}   // loadClassData

	/**
	 * Static data initialization.
	 */
	static
	{
		// initialize the NO_METHOD object
		try
		{
			NO_METHOD = NO_OBJECT.getClass().getMethod("hashCode", null);
		}
		catch( NoSuchMethodException err )
		{
			System.err.println("WARNING: " + err);
		}

		// initialize the defaultLoad set
		defaultLoad = new TreeSet();
		defaultLoad.add("script.attrib_mod");
		defaultLoad.add("script.attribute");
		defaultLoad.add("script.base_class");
		defaultLoad.add("script.base_class$ammo_info");
		defaultLoad.add("script.base_class$attacker_results");
		defaultLoad.add("script.base_class$defender_results");
		defaultLoad.add("script.base_class$range_info");
		defaultLoad.add("script.collections");
		defaultLoad.add("script.color");
		defaultLoad.add("script.combat_engine");
		defaultLoad.add("script.combat_engine$attack_roll_result");
		defaultLoad.add("script.combat_engine$hit_result");
		defaultLoad.add("script.combat_engine$combatant_data");
		defaultLoad.add("script.combat_engine$attacker_data");
		defaultLoad.add("script.combat_engine$defender_data");
		defaultLoad.add("script.combat_engine$weapon_data");
		defaultLoad.add("script.combat_engine$effect_data");
		defaultLoad.add("script.combat_engine$combat_data");
		defaultLoad.add("script.combat_engine$buff_data");
		defaultLoad.add("script.custom_var");
		defaultLoad.add("script.deltadictionary");
		defaultLoad.add("script.dictionary");
		defaultLoad.add("script.draft_schematic");
		defaultLoad.add("script.draft_schematic$slot");
		defaultLoad.add("script.draft_schematic$attribute");
		defaultLoad.add("script.location");
		defaultLoad.add("script.map_location");
		defaultLoad.add("script.menu_info");
		defaultLoad.add("script.menu_info_data");
		defaultLoad.add("script.menu_info_types");
		defaultLoad.add("script.modifiable_float");
		defaultLoad.add("script.modifiable_int");
		defaultLoad.add("script.modifiable_string_id");
		defaultLoad.add("script.obj_id");
		defaultLoad.add("script.obj_id$pending_script");
		defaultLoad.add("script.obj_id$obj_id_reference");
		defaultLoad.add("script.obj_id$obj_id_cleanup");
		defaultLoad.add("script.obj_id$emitter_tuple");
		defaultLoad.add("script.obj_var");
		defaultLoad.add("script.obj_var_list");
		defaultLoad.add("script.palcolor_custom_var");
		defaultLoad.add("script.player_levels");
		defaultLoad.add("script.player_levels$level_data");
		defaultLoad.add("script.player_levels$skill_template_data");
		defaultLoad.add("script.prose_package");
		defaultLoad.add("script.prose_package$participant_info");
		defaultLoad.add("script.random");
		defaultLoad.add("script.ranged_int_custom_var");
		defaultLoad.add("script.region");
		defaultLoad.add("script.resource_attribute");
		defaultLoad.add("script.resource_density");
		defaultLoad.add("script.resource_weight");
		defaultLoad.add("script.resource_weight$weight");
		defaultLoad.add("script.gcw_score");
		defaultLoad.add("script.gcw_score$gcw_data");
		defaultLoad.add("script.script_class_loader");
		defaultLoad.add("script.script_entry");
		defaultLoad.add("script.script_entry$listener_data");
		defaultLoad.add("script.slot_data");
		defaultLoad.add("script.string_id");
		defaultLoad.add("script.transform");
		defaultLoad.add("script.vector");
		defaultLoad.add("script.system_process");
	}

}   // class script_class_loader

