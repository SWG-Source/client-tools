						 /**
 * Title:        script_entry
 * Description:  Java entry point for executing scripts.
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;

//import com.ibm.jvm.Dump;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.Stack;
import java.util.Vector;
import java.lang.reflect.InvocationTargetException;
import script.base_class;

public class script_entry
{
	/**************************************************************************/

	/**
	 * Class listener_data keeps track of what we need to do when a listened for
	 * function gets called.
	 */
	static class listener_data
	{
		private obj_id m_listener;		// who is listening
		private String m_script;		// listener script to call
		private String m_function;		// listener function to call

		public listener_data(obj_id listener, String script, String function)
		{
			m_listener = listener;
			m_script = script;
			m_function = function;
		}	// listener_data

		public obj_id getListener()
		{
			return m_listener;
		}	// getListener

		public String getScript()
		{
			return m_script;
		}	// getScript

		public String getFunction()
		{
			return m_function;
		}	// getFunction

		public boolean equals(Object obj)
		{
			// assume the object is a listener_data
			try
			{
				listener_data data = (listener_data)obj;
				if ( data.m_listener == m_listener )
					return true;
				return false;
			}
			catch( ClassCastException err )
			{
			}

			// see if the object is an obj_id
			try
			{
				obj_id data = (obj_id)obj;
				if ( data == m_listener )
					return true;
				return false;
			}
			catch( ClassCastException err )
			{
			}
			return false;
		}	// equals
	}	// class listener_data


	/**************************************************************************/

	// flag to enable profiling calls
	private static boolean ENABLE_PROFILING = false;
	private static boolean profileSettingInitialized = false;

	private static void checkEnableProfiling()
	{
		String result = base_class.getConfigSetting("GameServer", "javaEngineProfiling");
		boolean ret = result != null && (result.equals("on") || result.equals("1") || result.equals("true"));
		System.err.println("Profiling: " + ret);
		ENABLE_PROFILING = ret;
		profileSettingInitialized = true;
	}

	// internal return value for triggers
	private static final int SCRIPT_INTERNAL_ERROR = 99;

	// starting value for listener hash tables
	private static final int LISTENER_HASH_TABLE_SIZE = 30;

	// members
	private static String scriptPath;								// path where scripts are
	private static Hashtable scriptListeners = new Hashtable();  	// scripts that are listening for other script functions to be called
																	// the table is keyed on the id of the object whose function we want
																	// to listen for, with the data being another Hashtable. The data
																	// Hashtable is keyed on the function being called, with the data
																	// being a list of objects who are listening for the function call.
	private static Stack ownerContext = new Stack(); 				// obj_id of the object currently running a script
	private static StringBuffer scriptNameBuffer = null;
	private static Class[] messageHandlerClasses = null;			// used to get the Method object for messageHandlers
	private static dictionary messageHandlerDictionary = null;		// used for messageHandlers with null params
	private static boolean printDebugInfo = false;					// flag to print debug info
	private static boolean crashOnException = false;  				// flag to crash if an exception occurs
	private static boolean enableJediTracking = false;				// flag to track players to see if they can become Jedi
	private static Stack runningScripts = new Stack();              // scripts being run
	private static Stack runningMethods = new Stack();              // methods being run
	private static long scriptStartTime = 0;                        // time at which a script started to run
	private static long scriptWarnMs = 0;                           // length of time a script can run before we print a warning
	private static long scriptInterruptMs = 0;                      // length of time a script can run before we stop it from running
	private static boolean scriptWarnPrinted = false;               // flag that we've printed a warning about scripts running long
	private static int stackOverflowLengthLimit = 35;               // depth of stack on a StackOverflowError we decide is a legit error
	private static boolean crashOnStackOverflow = true;             // flag to crash on StackOverflowError, if it below stackOverflowLengthLimit

	/**************************************************************************/

	/**
	 * Class constructor.
	 *
	 * @param path     directory where the scripts are
	 */
    public script_entry(String path, boolean debug, boolean exception, int watcherWarnMs, int watcherInterruptMs,
		int stackErrorLimit, boolean stackErrorCrash)
	{
		printDebugInfo = debug;
		crashOnException = exception;
		stackOverflowLengthLimit = stackErrorLimit;
		crashOnStackOverflow = stackErrorCrash;
		scriptPath = path;
		// add a '/' to the end of the path if there isn't one
		if (!scriptPath.endsWith("\\") && !scriptPath.endsWith("/"))
			scriptPath = scriptPath + "/";

   		// start a watcher thread that will prevent scripts from running too long
		if (watcherWarnMs > 0 && watcherInterruptMs > 0)
		{
			scriptWarnMs = watcherWarnMs;
			scriptInterruptMs = watcherInterruptMs;
		}

		if (debug)
		{
			System.out.println("Java script_entry initialized, free/total Java memory = " + Runtime.getRuntime().freeMemory() +
				"/" + Runtime.getRuntime().totalMemory());
		}
	}   // script_entry

	/**
	 * Turns script logging on or off.
	 *
	 * @param enable		flag to turn logging on or off
	 */
	public static void enableScriptLogging(boolean enable)
	{
		printDebugInfo = enable;
	}

	/**
	 * Turns tracking of players to see if they can become Jedi on or off.
	 *
	 * @param enable	flag to turn tracking on or off
	 */
	public static void enableNewJediTracking(boolean enable)
	{
		enableJediTracking = enable;
	}

	/**
	 * Returns if new Jedi tracking is enabled.
	 */
	public static boolean getEnableNewJediTracking()
	{
		return enableJediTracking;
	}

	/**
	 * Returns the script path.
	 */
	public static String getScriptPath()
	{
		return scriptPath;
	}   // getScriptPath

	/**
  	* Finds the Class associated with a class name.
  	*
  	* @param name      the class name
  	*
  	* @return the class
	*/
  	public static Class getClass(String name) throws ClassNotFoundException
  	{
  		// find the class loader for the class
  		script_class_loader loader = script_class_loader.getClassLoader(name);
  		if (loader == null)
  		{
  			System.err.println("WARNING: ERROR in java class script_entry.getClass: cannot find loader for class " + name);
  			throw new ClassNotFoundException();
  		}
  	    return loader.getMyClass();
	}   // getClass

	/**
	 * Causes a class loader to be unloaded so that the class file will be
	 * re-read the next time the script is run.
	 *
	 * @param name      the class name to unload
	 *
	 * @return true if the class was unloaded, false if the class doesn't exist
	 */
	public static boolean unloadClass(String name)
	{
		return script_class_loader.unloadClass(name);
	}   // unloadClass

	/**
	 * Returns the available function names we can call on a script.
	 *
	 * @param script		the script name
	 *
	 * @return an array of the available function names, or null on error
	 */
	public static String[] getScriptFunctions(String script) throws ClassNotFoundException
	{
		Class scriptClass = getClass(script);
		if (scriptClass == null)
			return null;

		Class baseClass = getClass("script.base_class");
		if (baseClass == null)
		{
			System.err.println("script_entry.java, getScriptFunctions could not find base class");
			return null;
		}
		Class objectClass = Class.forName("java.lang.Object");
		if (objectClass == null)
		{
			System.err.println("script_entry.java, getScriptFunctions could not find Object class");
			return null;
		}
		Method[] scriptMethods = scriptClass.getMethods();

		// put the function names into an array, filtering out functions that are derived from base_class.java
		// and other non-script classes
		Vector functions = new Vector();
		for ( int i = 0; i < scriptMethods.length; ++i )
		{
			if (scriptMethods[i].getDeclaringClass() != baseClass &&
				scriptMethods[i].getDeclaringClass() != objectClass)
			{
				functions.add(scriptMethods[i].getName());
			}
		}

		// convert the Vector of function names to an array
		String[] returnValue = new String[functions.size()];
		functions.toArray(returnValue);
/*
		System.out.println("script_entry.java, getScriptFunctions returning names for script " + script);
		for ( int i = 0; i < returnValue.length; ++i )
		{
			System.out.println("\t" + returnValue[i]);
		}
*/
		return returnValue;
	}	// getScriptFunctions

	/**
	 * Returns the obj_id of the object currently running scripts.
	 *
	 * @return the owner's obj_id
	 **/
	static obj_id getOwnerContext()
	{
		return (obj_id)ownerContext.peek();
	}	// getOwnerContext

	static long getScriptStartTime()
	{
		return scriptStartTime;
	}

	static long getScriptWarnMs()
	{
		return scriptWarnMs;
	}

	static long getScriptInterruptMs()
	{
		return scriptInterruptMs;
	}

	static String getCurrentScriptAndMethod()
	{
		if (!runningScripts.empty() && !runningMethods.empty())
		{
			return runningScripts.peek() + "." + runningMethods.peek();
		}
		return "<null>";
	}

	static void setScriptWarnPrinted()
	{
		scriptWarnPrinted = true;
	}

	static boolean isScriptWarnPrinted()
	{
		return scriptWarnPrinted;
	}

	private static void reportMemory(String msg)
	{
		long freeMem = Runtime.getRuntime().freeMemory();
		long totalMem = Runtime.getRuntime().totalMemory();
		System.err.println("WARNING: " + msg + ", memory = " + freeMem + "/" + totalMem);
	}

	public static long getFreeMemory()
	{
		return Runtime.getRuntime().freeMemory();
	}

	/**
	 * Executes a script.
	 *
	 * @param script        name of the script
	 * @param method        name of the script method to call
	 * @param params        paramaters to pass to the method
	 *
	 * @return SCRIPT_CONTINUE or SCRIPT_OVERRIDE
	 */
	public static int runScript(String script, String method, Object[] params) throws internal_script_error, internal_script_exception
	{
		boolean scriptCalled = false;
		boolean isProfiling = false;

		if (!profileSettingInitialized)
			checkEnableProfiling();

		int result = base_class.SCRIPT_CONTINUE;

//		System.out.println("*****pushing ownerContext, script = " + script + ", method = " + method + ", count = " +
//			Integer.toString(ownerContext.size()) + "*****");
		ownerContext.push(params[0]);

		Hashtable methods = null;
		try
		{
			script_class_loader loader = script_class_loader.getClassLoader(script);
			if (loader == null)
			{
				System.err.println("WARNING: couldn't load script " + script);
				return result;
			}
			Class cls = loader.getMyClass();
			if (cls == null)
			{
				System.err.println("WARNING: couldn't get class for script " + script);
				return result;
			}
			Object obj = loader.getMyObject();
			if (obj == null)
			{
				System.err.println("WARNING: couldn't get object for script " + script);
				return result;
			}
			methods = loader.getMyMethods();
			if (methods == null)
			{
				System.err.println("WARNING: couldn't get methods for script " + script);
				return result;
			}
//			System.out.println("Got loader and class for script " + script);

			Method meth = (Method)methods.get(method);
			if (meth == null)
			{
				if (params == null)
				{
					meth = cls.getDeclaredMethod(method, null);
				}
				else
				{
					// get the parameter classes
					String paramClassName;
					Class[] paramClass = new Class[params.length];
					for (int i = 0; i < params.length; ++i)
					{
						if (params[i] == null)
						{
							paramClass = null;
							System.err.println("ERROR: Calling script \"" + script + "\" method \"" + method + "\" parameter " + i + " is null!");
							break;
						}
						if (params[i] instanceof Integer)
						    paramClass[i] = java.lang.Integer.TYPE;
						else if (params[i] instanceof Float)
						    paramClass[i] = java.lang.Float.TYPE;
						else if (params[i] instanceof Boolean)
						    paramClass[i] = java.lang.Boolean.TYPE;
						else
							paramClass[i] = params[i].getClass();
					}
					if (paramClass != null)
						meth = cls.getMethod(method, paramClass);
				}
				// cache the method
				if (meth != null)
				{
					if (meth.isAccessible() == false)
						meth.setAccessible(true);
					methods.put(method, meth);
				}
			}
			if (meth != null && meth != script_class_loader.NO_METHOD)
			{
				scriptCalled = true;
				if (printDebugInfo)
					System.out.println("Java runScript enter, script = " + script + ", method = " + method);
				startScriptTimer(script, method);

				if (ENABLE_PROFILING)
				{
					isProfiling = true;
					base_class.PROFILER_START(script + "." + method);
				}

				Object scriptResult = meth.invoke(obj, params);
				if ( scriptResult != null )
					result = ((Integer)scriptResult).intValue();
			}
		}
		catch (NoSuchMethodException err)
		{
//			System.err.println("Error getting method " + method + " in script " + script);

			// store a dummy method tag in the methods table to avoid looking up this method again
			if (methods != null)
				methods.put(method, script_class_loader.NO_METHOD);

			result = base_class.SCRIPT_CONTINUE;
		}
		catch (InvocationTargetException err)
		{
			if ( err != null )
			{
				Throwable realErr = err.getTargetException();
				if (realErr != null)
				{
					if (realErr instanceof internal_script_error)
					{
						System.err.println("WARNING: " + realErr);
						err.printStackTrace(System.err);
						throw (internal_script_error)realErr;
					}
					else if (realErr instanceof internal_script_exception)
					{
						System.err.println("WARNING: " + realErr);
						err.printStackTrace(System.err);
						throw (internal_script_exception)realErr;
					}

					System.err.println("WARNING: Java Error: InvocationTargetException for script " + script + ", trigger " + method);
					System.err.println("WARNING: " + realErr);
					if (realErr instanceof Error)
					{
						System.err.println("WARNING: Major error! Printing stack trace:");
						err.printStackTrace(System.err);
						if (realErr instanceof StackOverflowError)
						{
							reportMemory("Java stack overflow");
							if (crashOnStackOverflow)
								crashOnException = true;
							else
								throw new internal_script_error((Error)realErr);
						}
						else
							throw new internal_script_error((Error)realErr);
					}
					else
						err.printStackTrace(System.err);
				}
				else
					System.err.println("WARNING: Java Error: Unknown InvocationTargetException.getTargetException for script " + script + ", trigger " + method);
			}
			else
				System.err.println("WARNING: Java Error: Unknown InvocationTargetException for script " + script + ", trigger " + method);
			if (crashOnException)
			{
				base_class.disableGameMemoryDump();
				base_class.generateJavacore();
				System.exit(-1);
			}
			result = SCRIPT_INTERNAL_ERROR;
		}
		catch (ClassNotFoundException err)
		{
			System.err.println("WARNING: Java Error: " + err + ", trigger = " + method);
			result = base_class.SCRIPT_CONTINUE;
		}
		catch (StackOverflowError err)
		{
			reportMemory("Java stack overflow");
			System.err.println("WARNING: Java Error in runScript from calling " + method + " : " + err);
			err.printStackTrace(System.err);
			System.err.println("WARNING: current stack = ");
			Thread.dumpStack();
//			com.ibm.jvm.Dump.JavaDump();
			if (crashOnStackOverflow)
			{
				base_class.generateJavacore();
				System.exit(-1);
			}
			else
				result = SCRIPT_INTERNAL_ERROR;
		}
		catch (Exception err)
		{
			// any other exception is an error
			System.err.println("WARNING: Java Error: Unhandled Exception for script " + script + ", trigger " + method);
			System.err.println("WARNING: " + err);
			result = SCRIPT_INTERNAL_ERROR;
		}
		catch (Error err)
		{
			if (err instanceof internal_script_error)
				throw (internal_script_error)err;
			System.err.println("WARNING: Java Error: Unhandled Error for script " + script + ", trigger " + method);
			System.err.println("WARNING: " + err.toString());
			result = SCRIPT_INTERNAL_ERROR;
		}
		finally
		{
			if (scriptCalled)
			{
				stopScriptTimer();
				if (printDebugInfo)
					System.out.println("Java runScript exit, script = " + script + ", trigger = " + method);
			}
			ownerContext.pop();
			if (isProfiling)
				base_class.PROFILER_STOP(script + "." + method);
		}

		return result;
	}   // runScript

	/**
	 * Executes all the scripts attached to an object. It is assumed that the
	 * 1st parameter of params is the obj_id of the object we want to run the
	 * scripts on.
	 *
	 * @param method        name of the script method to call
	 * @param params        paramaters to pass to the method
	 *
	 * @return SCRIPT_CONTINUE or SCRIPT_OVERRIDE
	 */
	public static int runScripts(String method, Object[] params) throws internal_script_error
	{
		if (params == null || params.length == 0 || params[0] == null)
		{
			System.err.println("WARNING: Java script_entry.runScripts ERROR: invalid params");
			return base_class.SCRIPT_OVERRIDE;
		}

		obj_id object = null;
		try
		{
			object = (obj_id)params[0];
		}
		catch (ClassCastException err)
		{
			System.err.println("WARNING: Java script_entry.runScripts ERROR: param[0] not self");
			return base_class.SCRIPT_OVERRIDE;
		}

		if (object == null || object.getValue() == 0 || !object.isAuthoritative() || !object.isLoaded())
		{
			boolean doubleCheck = false;
			String warning = "WARNING: Java script_entry.runScripts ERROR calling script function " + method + ": invalid self: ";
			if (object == null)
				System.err.println(warning + "no object id");
			else if (object.getValue() == 0)
				System.err.println(warning + "obj_id 0");
			else if (!object.isAuthoritative())
			{
				System.err.println(warning + object + " not authoritative");
				// We've seen errors where Java thinks an object is non-authoritative but C thinks the object
				// is authoritative. _internalIsAuthoritative() will double-check for us and set the obj_id
				// flag if needed
				doubleCheck = base_class._internalIsAuthoritative(object);
				if (doubleCheck)
				{
					System.err.println("WARNING: object " + object + " is flagged as non-authoritative "+
						"in Java but is authoritative in C");
				}
			}
			else if (!object.isLoaded())
				System.err.println(warning + object + " not loaded");
			if (!doubleCheck)
				return base_class.SCRIPT_OVERRIDE;
		}

		object.lockScripts();						// prevent the object's script list from being modified
		int result = base_class.SCRIPT_CONTINUE;
		try
		{
			if(object.getLockCount() > 1)
			{
				String[] scripts = object.getScripts();
				for (int i = 0; i < scripts.length; ++i)
				{
					result = runScript(scripts[i], method, params);
					if (result != base_class.SCRIPT_CONTINUE)
						break;
				}
			}
			else
			{
				ArrayList scripts = object.getScriptArrayList();
				for (int i = 0; i < scripts.size(); ++i)
				{
	//				String script = (String)scripts.get(i);
	//				System.out.println("IN JAVA: script_entry.runScripts executing method " + method + " on script " + (String)scripts.get(i));
	//				scriptNameBuffer.setLength("script.".length());
	//				scriptNameBuffer.append(script);
	//				result = runScript(scriptNameBuffer.toString(), method, params);
					result = runScript((String)scripts.get(i), method, params);
					if (result != base_class.SCRIPT_CONTINUE)
						break;
				}
			}
		}
		catch (ClassCastException err)
		{
			System.err.println("WARNING: Java script_entry.runScripts ERROR: non-string on script list!?!?");
			result = base_class.SCRIPT_OVERRIDE;
		}
		catch (IndexOutOfBoundsException err)
		{
			System.err.println("WARNING: Java script_entry.runScripts ERROR: script list out of bounds!");
			result = base_class.SCRIPT_OVERRIDE;
		}
		catch( internal_script_exception err )
		{
			result = SCRIPT_INTERNAL_ERROR;
		}
		finally
		{
			object.unlockScripts();					// allow modification to the object's scripts
		}
		return result;
	}   // runScripts

	/**
	 * Executes a messageHandler.
	 *
	 * @param script        name of the script
	 * @param method        name of the script method to call
	 * @param params        paramaters to pass to the method
	 *
	 * @return SCRIPT_CONTINUE or SCRIPT_OVERRIDE
	 */
	public static int callMessageHandler(String script, String method, Object[] params)  throws internal_script_error, internal_script_exception
	{
		boolean scriptCalled = false;
		boolean isProfiling = false;

//		System.out.println("(java) runScript: script = " + script +
//			", method = " + method + ", num params = " + params.length);

		int result = base_class.SCRIPT_CONTINUE;

//		System.out.println("*****pushing ownerContext, script = " + script + ", method = " + method + ", count = " +
//			Integer.toString(ownerContext.size()) + "*****");
		ownerContext.push(params[0]);

		Hashtable methods = null;
		try
		{
			script_class_loader loader = script_class_loader.getClassLoader(script);
			if (loader == null)
			{
				System.err.println("WARNING: couldn't load script " + script);
				return result;
			}
			Class cls = loader.getMyClass();
			if (cls == null)
			{
				System.err.println("WARNING: couldn't get class for script " + script);
				return result;
			}
			Object obj = loader.getMyObject();
			if (obj == null)
			{
				System.err.println("WARNING: couldn't get object for script " + script);
				return result;
			}
			methods = loader.getMyMethods();
			if (methods == null)
			{
				System.err.println("WARNING: couldn't get methods for script " + script);
				return result;
			}
//			System.out.println("Got loader and class for script " + script);

			Method meth = (Method)methods.get(method);
			if (meth == null)
			{
				// get the parameter classes
				meth = cls.getMethod(method, messageHandlerClasses);
				// cache the method
				if (meth != null)
				{
					if (meth.isAccessible() == false)
						meth.setAccessible(true);
					methods.put(method, meth);
				}
			}
			if (meth != null && meth != script_class_loader.NO_METHOD)
			{
				scriptCalled = true;
				if (printDebugInfo)
					System.out.println("Java callMessageHandler enter, script = " + script + ", messageHandler = " + method);
				startScriptTimer(script, method);

				if (ENABLE_PROFILING)
				{
					isProfiling = true;
					base_class.PROFILER_START(script + "." + method);
				}

				Object scriptResult = meth.invoke(obj, params);
				if ( scriptResult != null )
					result = ((Integer)scriptResult).intValue();
			}
		}
		catch (NoSuchMethodException err)
		{
//			System.err.println("Error getting method " + method + " in script " + script);

			// store a dummy method tag in the methods table to avoid looking up this method again
			if (methods != null)
				methods.put(method, script_class_loader.NO_METHOD);

			result = base_class.SCRIPT_CONTINUE;
		}
		catch (InvocationTargetException err)
		{
			if ( err != null )
			{
				Throwable realErr = err.getTargetException();
				if (realErr != null)
				{
					if (realErr instanceof internal_script_error)
					{
						System.err.println("WARNING: " + realErr);
						err.printStackTrace(System.err);
						throw (internal_script_error)realErr;
					}
					else if (realErr instanceof internal_script_exception)
					{
						System.err.println("WARNING: " + realErr);
						err.printStackTrace(System.err);
						throw (internal_script_exception)realErr;
					}
					System.err.println("WARNING: Java Error: InvocationTargetException for script " + script + ", messageHandler " + method);
					System.err.println("WARNING: " + realErr);
					if (realErr instanceof Error)
					{
						System.err.println("WARNING: Major error! Printing stack trace:");
						err.printStackTrace(System.err);
						if (realErr instanceof StackOverflowError)
						{
							reportMemory("Java stack overflow");
							if (crashOnStackOverflow)
								crashOnException = true;
							else
								throw new internal_script_error((Error)realErr);
						}
						else
							throw new internal_script_error((Error)realErr);
					}
					else
						err.printStackTrace(System.err);
				}
				else
					System.err.println("WARNING: Java Error: Unknown InvocationTargetException.getTargetException for script " + script + ", messageHandler " + method);
			}
			else
				System.err.println("WARNING: Java Error: Unknown InvocationTargetException for script " + script + ", messageHandler " + method);
			if (crashOnException)
			{
				base_class.disableGameMemoryDump();
				base_class.generateJavacore();
				System.exit(-1);
			}
			result = SCRIPT_INTERNAL_ERROR;
		}
		catch (ClassNotFoundException err)
		{
			System.err.println("WARNING: Java Error: " + err + ", messageHandler = " + method);
			result = base_class.SCRIPT_CONTINUE;
		}
		catch (StackOverflowError err)
		{
			reportMemory("Java stack overflow");
			System.err.println("WARNING: Java Error in callMessageHandler from calling " + method + " : " + err);
			err.printStackTrace(System.err);
			System.err.println("WARNING: current stack = ");
			Thread.dumpStack();
//			com.ibm.jvm.Dump.JavaDump();
			if (crashOnStackOverflow)
			{
				base_class.generateJavacore();
				System.exit(-1);
			}
			else
				result = SCRIPT_INTERNAL_ERROR;
		}
		catch (Exception err)
		{
			// any other exception is an error
			System.err.println("WARNING: Java Error: Unhandled Exception for script " + script + ", messageHandler " + method);
			System.err.println("WARNING: " + err);
			result = SCRIPT_INTERNAL_ERROR;
		}
		catch (Error err)
		{
			if (err instanceof internal_script_error)
				throw (internal_script_error)err;
			System.err.println("WARNING: Java Error: Unhandled Error for script " + script + ", messageHandler " + method);
			System.err.println("WARNING: " + err.toString());
			result = SCRIPT_INTERNAL_ERROR;
		}
		finally
		{
			if (scriptCalled)
			{
				stopScriptTimer();
				if (printDebugInfo)
					System.out.println("Java callMessageHandler exit, script = " + script + ", messageHandler = " + method);
			}
			ownerContext.pop();
			if (isProfiling)
				base_class.PROFILER_STOP(script + "." + method);
		}

		return result;
	}   // callMessageHandler

	/**
	 * Executes a messageHandler for all the scripts attached to an object.
	 *
	 * @param method        name of the script method to call
	 * @param object        the object to run the scripts on
	 * @param params		the dictionary containing the handler's params
	 *
	 * @return SCRIPT_CONTINUE or SCRIPT_OVERRIDE
	 */
	public static int callMessageHandlers(String method, long object, dictionary params) throws internal_script_error
	{
		return callMessageHandlers(method, (object == 0) ? null : obj_id.getObjId(object), params);
	}

	public static int callMessageHandlers(String method, obj_id object, dictionary params) throws internal_script_error
	{
		if (printDebugInfo)
			System.out.println("Java callMessageHandlers enter, method = " + method + ", current time = " + System.currentTimeMillis());

		if (messageHandlerClasses == null)
		{
			System.err.println("WARNING: Java script_entry.callMessageHandlers ERROR: messageHandlerClasses not initialized");
			return base_class.SCRIPT_OVERRIDE;
		}

		if (method == null || object == null)
		{
			if (object == null)
				System.err.println("WARNING: Java script_entry.callMessageHandlers ERROR: null self");
			else
				System.err.println("WARNING: Java script_entry.callMessageHandlers ERROR: null method name");
			return base_class.SCRIPT_OVERRIDE;
		}

		if (object.getValue() == 0 || !object.isAuthoritative() || !object.isLoaded())
		{
			boolean doubleCheck = false;
			if (object.getValue() == 0)
				System.err.println("WARNING: Java script_entry.callMessageHandlers ERROR: invalid self: obj_id 0");
			else if (!object.isAuthoritative())
			{
				System.err.println("WARNING: Java script_entry.callMessageHandlers ERROR: invalid self: " + object.toString() + " not authoritative");
				// We've seen errors where Java thinks an object is non-authoritative but C thinks the object
				// is authoritative. _internalIsAuthoritative() will double-check for us and set the obj_id
				// flag if needed
				doubleCheck = base_class._internalIsAuthoritative(object);
				if (doubleCheck)
				{
					System.err.println("WARNING: object " + object + " is flagged as non-authoritative "+
						"in Java but is authoritative in C");
				}
			}
			else if (!object.isLoaded())
				System.err.println("WARNING: Java script_entry.callMessageHandlers ERROR: invalid self: " + object.toString() + " not loaded");
			if (!doubleCheck)
				return base_class.SCRIPT_OVERRIDE;
		}
		object.lockScripts();						// prevent the object's script list from being modified
		int result = base_class.SCRIPT_CONTINUE;
		try
		{
			// create an array for the parameters that will be sent to Method.invoke()
			Object[] paramArray = new Object[2];
			paramArray[0] = object;
			if (params != null)
				paramArray[1] = params;
			else
			{
				messageHandlerDictionary.clear();
				paramArray[1] = messageHandlerDictionary;
			}

			if(object.getLockCount() > 1)
			{
				String[] scripts = object.getScripts();
				for (int i = 0; i < scripts.length; ++i)
				{
					result = callMessageHandler(scripts[i], method, paramArray);
					if (result != base_class.SCRIPT_CONTINUE)
						break;
				}
			}
			else
			{
				ArrayList scripts = object.getScriptArrayList();
				for (int i = 0; i < scripts.size(); ++i)
				{
	//				String script = "script." + (String)scripts.get(i);
	//				scriptNameBuffer.setLength("script.".length());
	//				scriptNameBuffer.append(script);
	//				script = scriptNameBuffer.toString();
	//				result = callMessageHandler(scriptNameBuffer.toString(), method, paramArray);
					result = callMessageHandler((String)scripts.get(i), method, paramArray);
					if (result != base_class.SCRIPT_CONTINUE)
						break;
				}
			}
		}
		catch (ClassCastException err)
		{
			System.err.println("WARNING: Java script_entry.runScripts ERROR: non-string on script list!?!?");
			result = base_class.SCRIPT_OVERRIDE;
		}
		catch (IndexOutOfBoundsException err)
		{
			System.err.println("WARNING: Java script_entry.runScripts ERROR: script list out of bounds!");
			result = base_class.SCRIPT_OVERRIDE;
		}
		catch( internal_script_exception err )
		{
			result = SCRIPT_INTERNAL_ERROR;
		}
		finally
		{
			object.unlockScripts();					// allow modification to the object's scripts
		}
		return result;
	}   // callMessageHandlers

	/**
	 * Executes a console handler function on a script. The hander must return a string instead of
	 * an int, and has no "self" parameter.
	 *
	 * @param script        name of the script
	 * @param method        name of the script method to call
	 * @param params        paramaters to pass to the method
	 *
	 * @return the string returned from the handler function
	 */
	public static String runConsoleHandler(String script, String method, Object[] params)  throws internal_script_error, internal_script_exception
	{
		boolean scriptCalled = false;
		boolean isProfiling = false;

		if (!profileSettingInitialized)
			checkEnableProfiling();

		String result = null;

		Hashtable methods = null;
		try
		{
			script_class_loader loader = script_class_loader.getClassLoader(script);
			if (loader == null)
			{
				System.err.println("WARNING: couldn't load script " + script);
				return result;
			}
			Class cls = loader.getMyClass();
			if (cls == null)
			{
				System.err.println("WARNING: couldn't get class for script " + script);
				return result;
			}
			Object obj = loader.getMyObject();
			if (obj == null)
			{
				System.err.println("WARNING: couldn't get object for script " + script);
				return result;
			}
			methods = loader.getMyMethods();
			if (methods == null)
			{
				System.err.println("WARNING: couldn't get methods for script " + script);
				return result;
			}

			Method meth = null;
			if (params == null)
			{
				meth = cls.getDeclaredMethod(method, null);
			}
			else
			{
				// get the parameter classes
				String paramClassName;
				Class[] paramClass = new Class[params.length];
				for (int i = 0; i < params.length; ++i)
				{
					if (params[i] == null)
					{
						paramClass = null;
						System.err.println("WARNING: ERROR: param " + i + " is null!");
						break;
					}
					if (params[i] instanceof Integer)
						paramClass[i] = java.lang.Integer.TYPE;
					else if (params[i] instanceof Float)
						paramClass[i] = java.lang.Float.TYPE;
					else if (params[i] instanceof Boolean)
						paramClass[i] = java.lang.Boolean.TYPE;
					else
						paramClass[i] = params[i].getClass();
				}
				if (paramClass != null)
					meth = cls.getMethod(method, paramClass);
			}
			if (meth != null && meth != script_class_loader.NO_METHOD)
			{
				if (meth.isAccessible() == false)
					meth.setAccessible(true);
				scriptCalled = true;
				if (printDebugInfo)
					System.out.println("Java runConsoleHandler enter, script = " + script + ", console handler method = " + method);
				startScriptTimer(script, method);

				if (ENABLE_PROFILING)
				{
					isProfiling = true;
					base_class.PROFILER_START(script + "." + method);
				}

				Object scriptResult = meth.invoke(obj, params);
				if ( scriptResult != null )
					result = (String)scriptResult;
			}
		}
		catch (NoSuchMethodException err)
		{
		}
		catch (InvocationTargetException err)
		{
			if ( err != null )
			{
				Throwable realErr = err.getTargetException();
				if (realErr != null)
				{
					if (realErr instanceof internal_script_error)
					{
						System.err.println("WARNING: " + realErr);
						err.printStackTrace(System.err);
						throw (internal_script_error)realErr;
					}
					else if (realErr instanceof internal_script_exception)
					{
						System.err.println("WARNING: " + realErr);
						err.printStackTrace(System.err);
						throw (internal_script_exception)realErr;
					}

					System.err.println("WARNING: Java Error: InvocationTargetException for script " + script +
						", console handler method " + method);
					System.err.println("WARNING: " + realErr);
					if (realErr instanceof Error)
					{
						System.err.println("WARNING: Major error! Printing stack trace:");
						err.printStackTrace(System.err);
						if (realErr instanceof StackOverflowError)
						{
							reportMemory("Java stack overflow");
							if (crashOnStackOverflow)
								crashOnException = true;
							else
								throw new internal_script_error((Error)realErr);
						}
						else
							throw new internal_script_error((Error)realErr);
					}
					else
						err.printStackTrace(System.err);
				}
				else
				{
					System.err.println("WARNING: Java Error: Unknown InvocationTargetException.getTargetException for script " +
						script + ", console handler method " + method);
				}
			}
			else
			{
				System.err.println("WARNING: Java Error: Unknown InvocationTargetException for script " + script +
					", console handler method " + method);
			}
			if (crashOnException)
			{
				base_class.disableGameMemoryDump();
				base_class.generateJavacore();
				System.exit(-1);
			}
		}
		catch (ClassNotFoundException err)
		{
			System.err.println("WARNING: Java Error: " + err + ", console handler method = " + method);
		}
		catch (StackOverflowError err)
		{
			reportMemory("Java stack overflow");
			System.err.println("WARNING: Java Error in runConsoleHandler from calling " + method + " : " + err);
			err.printStackTrace(System.err);
			System.err.println("WARNING: current stack = ");
			Thread.dumpStack();
			if (crashOnStackOverflow)
			{
				base_class.generateJavacore();
				System.exit(-1);
			}
		}
		catch (Exception err)
		{
			// any other exception is an error
			System.err.println("WARNING: Java Error: Unhandled Exception for script " + script + ", console handler method " + method);
			System.err.println("WARNING: " + err);
		}
		catch (Error err)
		{
			if (err instanceof internal_script_error)
				throw (internal_script_error)err;
			System.err.println("WARNING: Java Error: Unhandled Error for script " + script + ", console handler method " + method);
			System.err.println("WARNING: " + err.toString());
		}
		finally
		{
			if (scriptCalled)
			{
				stopScriptTimer();
				if (printDebugInfo)
					System.out.println("Java runScript exit, script = " + script + ", console handler method = " + method);
			}
			if (isProfiling)
				base_class.PROFILER_STOP(script + "." + method);
		}

		return result;
	}   // runConsoleHandler

	/**
	 * Adds a listener to the list of scripts listening for function calls.
	 *
	 * @param listenee				object who function we want to listen for
	 * @param listeneeFunction		function name on listee we are listening for
	 * @param listener      		object listening for a function to be called
	 * @param listenerScript		script on listener to call when the listenee function is called
	 * @param listenerFunction		function name on listener to call when the listenee function is called
	 **/
	public static void startScriptListening(obj_id listenee, String listeneeFunction,
		obj_id listener, String listenerScript, String listenerFunction)
	{
		// make sure the listenee is authoritative
		if (!listenee.isAuthoritative())
		{
			// @todo: send to authoritative object?
			System.err.println("WARNING: Scripting error: trying to listen to non-authoritative object!");
			System.err.println("WARNING: \tlistenee = " + listenee.toString() + ", function = " + listeneeFunction);
			System.err.println("WARNING: \tlistener = " + listener.toString() + ", script = " + listenerScript + ", function = " + listenerFunction);
			return;
		}

		Hashtable functions = (Hashtable)scriptListeners.get(listenee);
		if (functions == null)
		{
			functions = new Hashtable(LISTENER_HASH_TABLE_SIZE);
			scriptListeners.put(listenee, functions);
		}
		LinkedList listeners = (LinkedList)functions.get(listeneeFunction);
		if (listeners == null)
		{
			listeners = new LinkedList();
			functions.put(listeneeFunction, listeners);
		}

		listener_data data = new listener_data(listener, listenerScript, listenerFunction);
		listeners.remove(data);
		listeners.addFirst(data);
	}	// startScriptListening

	/**
	 * Removes a listener from the list of scripts listening for function calls.
	 *
	 * @param listenee				object that was being listened to
	 * @param listeneeFunction		function name that was being listened for
	 * @param listener      		object that wants to stop listening
	 **/
	public static void stopScriptListening(obj_id listenee, String listeneeFunction, obj_id listener)
	{
		Hashtable functions = (Hashtable)scriptListeners.get(listenee);
		if ( functions != null )
		{
			LinkedList listeners = (LinkedList)functions.get(listeneeFunction);
			if ( listeners != null )
			{
				listeners.remove(listener);
			}
		}
	}	// stopListening

	/**
	 * Removes a listenee from the listeners list. This is generally done when the listenee is removed from the game.
	 *
	 * @param listenee		the object we want to remove
	 **/
	public static void removeScriptListenee(obj_id listenee)
	{
		scriptListeners.remove(listenee);
	}	// removeScriptListener

	/**
	 * Calls any scripts that are listening for function calls on other scripts.
	 *
	 * @param listenee		the script whose original function was called
	 * @param function		function that was called on the listenee
	 * @param params		parameters that were sent to the listenee's function
	 **/
	public void callScriptListeners(obj_id listenee, String function, Object[] params) throws internal_script_error, internal_script_exception
	{
		Hashtable functions = (Hashtable)scriptListeners.get(listenee);
		if (functions != null)
		{
			LinkedList listeners = (LinkedList)functions.get(function);
			if (listeners != null && listeners.size() > 0)
			{
				// make the parameters to send to the listeners; they are the same as the ones sent to the listenee,
				// but the 1st parameter is the listener's id
				Object[] listenerParams = new Object[params.length + 1];
				for ( int i = 0; i < params.length; ++i )
					listenerParams[i+1] = params[i];

				ListIterator i = listeners.listIterator(0);
				for (listener_data listener = (listener_data)i.next(); listener != null; listener = (listener_data)i.next())
				{
					if (listener.getListener().getValue() != 0)
					{
						if (listener.getListener().isAuthoritative())
						{
							// call the listener's function
							listenerParams[0] = listener.getListener();
							runScript(listener.getScript(), listener.getFunction(), listenerParams);
						}
						else
						{
							// @todo: send a message to the authoritative object?
							System.err.println("WARNING: Scripting error: trying to signal non-authoritative listener object!");
							System.err.println("WARNING: \tlistener = " + listener.getListener().toString() +
								", script = " + listener.getScript() + ", function = " + listener.getFunction());
						}
					}
					else
					{
						// the listener is not in the game anymore, remove it from the list
						i.remove();
					}
				}
			}
		}
	}	// callScriptListeners

	/**
	 * Starts a timer for the current script.
	 */
	private static void startScriptTimer(String script, String method)
	{
		if (getScriptWarnMs() > 0 || getScriptInterruptMs() > 0)
		{
			runningScripts.push(script);
			runningMethods.push(method);
			scriptWarnPrinted = false;
			scriptStartTime = System.currentTimeMillis();
		}
	}

	/**
	 * Stops the timer for the current script.
	 */
	private static void stopScriptTimer()
	{
		if (getScriptWarnMs() > 0 || getScriptInterruptMs() > 0)
		{
			if (runningScripts.empty() || runningMethods.empty())
			{
				System.err.println("WARNING: Java script_entry.stopScriptTimer, " +
					"m_runningScripts or m_runningMethods is empty! Shutting down scripting timeout.");
				scriptWarnMs = 0;
				scriptInterruptMs = 0;
				scriptStartTime = 0;
				return;
			}
			runningScripts.pop();
			runningMethods.pop();
			scriptWarnPrinted = false;
			if (!runningScripts.empty())
			{
				// restart the timer for the current script
				scriptStartTime = System.currentTimeMillis();
			}
			else
				scriptStartTime = 0;
		}
	}

	// initialize static data
	static
	{
		scriptNameBuffer = new StringBuffer(512);
//		scriptNameBuffer.append("script.");

		try
		{
			messageHandlerClasses = new Class[2];
			messageHandlerClasses[0] = obj_id.NULL_ID.getClass();
			script_class_loader loader = script_class_loader.getClassLoader("script.dictionary");
			messageHandlerClasses[1] = loader.getMyClass();
		}
		catch( ClassNotFoundException err )
		{
			messageHandlerClasses[0] = null;
			messageHandlerClasses[1] = null;
		}

		if (messageHandlerClasses[0] == null || messageHandlerClasses[1] == null)
		{
			System.err.println("WARNING: Java script_entry.static ERROR: didn't initialize messageHandlerClasses");
			messageHandlerClasses = null;
		}
		else
			messageHandlerDictionary = new dictionary();
	}

}   // class script_entry


