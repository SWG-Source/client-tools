/**
 * Title:        obj_id
 * Description:  Wrapper for an object id.
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;

import java.io.IOException;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.ObjectStreamException;
import java.io.Serializable;
import java.lang.ref.Reference;
import java.lang.ref.ReferenceQueue;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Vector;

public final class obj_id implements Comparable, Serializable
{
	private final static long serialVersionUID = 84998777444285203L;

	private final static boolean DEBUG = false;
	private final static boolean VALIDATE_SCRIPTS = true;
	public final static obj_id NULL_ID = new obj_id(0);
	public final static String SCRIPT_PREFIX = "script.";

	private static long m_nextSerializedId = -1;						// temporary id storage for deserialization
	private static HashSet potentiallyDirtyObjects = new HashSet();		// list of ids whose scriptvars may have changed
	private static Hashtable m_factory = new Hashtable(100003);			// keeps track of all ids currently in the game
	private static obj_id_cleanup m_cleanup;							// removes obj_ids from the factory after all references to the id have been cleared

	private transient Long m_id;					// the actual id
	private transient boolean m_authoritative;		// flag if the object is authoritative on this server
	private transient int m_processId;				// process id of the authoritative game server for this object
	private transient boolean m_loaded;				// flag if the object is loaded
	private transient boolean m_initialized;		// flag if the object is initialized
	private transient boolean m_loggedIn;	     	// flag if the object is logged in (if a player)
	private transient boolean m_beingDestroyed;     // flag that the object is in the process of being destroyed
	private transient ArrayList m_scripts;			// list of scripts attached to this object
	private transient int m_scriptLock;				// semaphore-type counter to prevent modification of the script list
	private transient ArrayList m_pendingScripts;
	private transient deltadictionary scriptVars = null;
	private transient dictionary scriptDictionary = null;

	/**************************************************************************/
	/**************************************************************************/
	//
	// Internal classes
	//
	/**************************************************************************/
	/**************************************************************************/

	// class to keep track of modifications made to the script list while locked
	private final class pending_script
	{
		public boolean add;			// true = add the script, false = remove it
		public String script;		// the script to add/remove
		public pending_script(boolean add, String script) { this.add = add; this.script = script; }
	}	// class pending_script

	/**
	 * Used to help remove an obj_id from the factory table when all references to it have been removed.
	 */
	private final static class obj_id_reference extends WeakReference
	{
		private Long m_id;

		public obj_id_reference(obj_id id, ReferenceQueue q)
		{
			super(id, q);
			m_id = id.m_id;
		}

		public Long getId()
		{
			return m_id;
		}
	}	// class obj_id_reference

	/**
	 * Cleanup thread that removes obj_ids from the factory table.
	 */
	private final static class obj_id_cleanup extends Thread
	{
		ReferenceQueue m_clearedObjectQueue;

		obj_id_cleanup()
		{
			super("obj_id_cleanup_Thread");
			m_clearedObjectQueue = new ReferenceQueue();
			setDaemon(true);
		}

		public ReferenceQueue getClearedObjectQueue()
		{
			return m_clearedObjectQueue;
		}

		public synchronized void run()
		{
			// the thread exists when all other threads are removed
			for (; ; )
			{
				try
				{
					// wait for a obj_id reference to be cleared
					Reference ref = m_clearedObjectQueue.remove();
					if (ref instanceof obj_id_reference)
					{
						// remove the reference from the factory
						if (obj_id.DEBUG)
						{
							System.out.println("Removing obj_id " + ((obj_id_reference)ref).getId() + " from factory");
						}
						obj_id.m_factory.remove(((obj_id_reference)ref).getId());
					}
				}
				catch (InterruptedException err)
				{
					System.err.println("Java obj_id.obj_id_cleanup error: " + err);
				}
			}
		}
	}	// class obj_id_cleanup

	/**************************************************************************/
	/**************************************************************************/
	//
	// Public methods
	//
	/**************************************************************************/
	/**************************************************************************/

	/**
	 * Returns an obj_id for a given id number. If an obj_id with that number
	 * has already been created, will return a reference to that obj_id.
	 *
	 * @param id		the object id number
	 *
	 * @return the obj_id for that number
	 */
	static public obj_id getObjId(long id)
	{
		if (id == 0)
			return NULL_ID;

		return getObjId(new Long(id));
	} // getObjId

	/**
	 * Returns an obj_id for a given id number. If an obj_id with that number
	 * has already been created, will return a reference to that obj_id.
	 *
	 * @param id		the object id number
	 *
	 * @return the obj_id for that number
	 */
	static public obj_id getObjId(Long id)
	{
		if (id == null || id.longValue() == 0)
			return NULL_ID;

		if (DEBUG)
		{
			System.out.println("obj_id factory request for id " + id);
		}

		Object value = m_factory.get(id);
		if (value == null)
		{
			if (DEBUG)
			{
				System.out.println("obj_id factory creating new id " + id);
			}

			obj_id newId = new obj_id(id);
			value = newId;
			// store the id as a weak reference; if it is later marked loaded, we will replace it with a hard reference
			m_factory.put(id, new obj_id_reference(newId, m_cleanup.getClearedObjectQueue()));
		}
		else if (value instanceof obj_id_reference)
		{
			value = ((obj_id_reference)value).get();
		}

		return (obj_id)value;
	} // getObjId

	public dictionary getScriptDictionary()
	{
		if (scriptDictionary == null)
		{
			scriptDictionary = new dictionary();
		}
		return scriptDictionary;
	}
	
	public void clearScriptDictionary()
	{
		if(scriptDictionary != null)
		{
			scriptDictionary.clear();
		}
	}

	public deltadictionary getScriptVars()
	{
		if (scriptVars == null)
		{
			scriptVars = new deltadictionary();
		}
		potentiallyDirtyObjects.add(this);
		return scriptVars;
	}

	public void clearScriptVars()
	{
		if (scriptVars != null)
		{
			scriptVars.removeAll();
			potentiallyDirtyObjects.add(this);
		}
	}

	/**
	 * Note: the 3 setScriptVar functions below were written so that the C code could set script vars for these data types.
	 * If wanted feel free to add other scriptvar setters/getters.
	 */
	public void setScriptVar(String name, int value)
	{
		if (scriptVars != null)
		{
			scriptVars.put(name, value);
			potentiallyDirtyObjects.add(this);
		}
	}

	public void setScriptVar(String name, float value)
	{
		if (scriptVars != null)
		{
			scriptVars.put(name, value);
			potentiallyDirtyObjects.add(this);
		}
	}

	public void setScriptVar(String name, String value)
	{
		if (scriptVars != null)
		{
			scriptVars.put(name, value);
			potentiallyDirtyObjects.add(this);
		}
	}

	public boolean hasScriptDictionary(Object key)
	{
		boolean result = false;
		if (scriptDictionary != null)
		{
			result = scriptDictionary.containsKey(key);
		}
		return result;
	}

	public boolean hasScriptVar(Object key)
	{
		boolean result = false;
		if (scriptVars != null)
		{
			result = scriptVars.hasKey(key);
		}
		return result;
	}

	/**
	 * Accessor function.
	 *
	 * @return the object id
	 */
	public long getValue()
	{
		return m_id.longValue();
	}	// getValue

	public int getProcessId()
	{
		return m_processId;
	}

	/**
	 * Returns true if the object was loaded from the buildout
	 * Note it will return false if the object was exported from the
	 * gold data into buildout files.
	 */

	public boolean isBuildoutObject()
	{
		return m_id.longValue() < 0;
	}

	/**
	 * Returns if this object is authoritative on the server or not.
	 *
	 * @return true if authoritative, false if not
	 */
	public boolean isAuthoritative()
	{
		return m_authoritative;
	}	// isAuthoritative

	/**
	 * Returns if this object has been loaded or not.
	 *
	 * @return true if loaded, false if not
	 */
	public boolean isLoaded()
	{
		return m_loaded;
	}	// isLoaded

	/**
	 * Returns if this object has been initialized or not.
	 *
	 * @return true if initialized, false if not
	 */
	public boolean isInitialized()
	{
		return m_initialized;
	}	// isInitialized

	/**
	 * Returns if this object is being destroyed or not.
	 *
	 * @return true if it is being destroyed, false if not
	 */
	public boolean isBeingDestroyed()
	{
		return m_beingDestroyed;
	}	// isBeingDestroyed

	public ArrayList getScriptArrayList()
	{
		return m_scripts;
	}	// getScripts

	/**
	 * Returns the scripts attached to this object.
	 *
	 * @return string array of script names, or null on error
	 */
	public String[] getScripts()
	{
		if (m_scripts == null)
			return null;
		if (m_scripts.size() == 0)
			return new String[0];

		String[] strings = null;
		if (m_scriptLock > 0)
		{
			// we have to set up a list of our scripts, adding and removing scripts from the pending list as needed
			LinkedList scriptList = getAllScriptsWithPending();
			if (scriptList != null)
			{
				strings = new String[scriptList.size()];
				scriptList.toArray(strings);
			}
		}
		else
		{
			strings = new String[m_scripts.size()];
			m_scripts.toArray(strings);
		}
		return strings;
	}	// getScripts()

	/**
	 * Checks to see if the object has a script attached.
	 *
	 * @param scriptName	the name of the script to look for
	 *
	 * @return true if the script is attached, false if not
	 */
	public boolean hasScript(String scriptName)
	{
		if (m_scripts == null)
			return false;

		String fullPath = SCRIPT_PREFIX + scriptName;
		if (m_scriptLock > 0)
		{
			LinkedList scriptList = getAllScriptsWithPending();
			if (scriptList != null)
				return scriptList.contains(fullPath);
		}
		return m_scripts.contains(fullPath);
	}	// hasScript()

	public void lockScripts()
	{
		++m_scriptLock;
	}	// lockScripts

	public int getLockCount()
	{
		return m_scriptLock;
	}

	public void unlockScripts()
	{
		if (m_scriptLock > 0)
		{
			if (--m_scriptLock == 0)
			{
				// update the script list for the pending scripts
				if (DEBUG)
					System.out.println("Updating pending scripts on object " + m_id);
				for (int i = 0; i < m_pendingScripts.size(); ++i)
				{
					pending_script pending = (pending_script)m_pendingScripts.get(i);
					if (pending.add)
						attachScript(pending.script);
					else
						detachScript(pending.script);
				}
				m_pendingScripts.clear();
			}
		}
	}	// unlockScripts

	/**
	 * Conversion function.
	 *
	 * @return the id as a string.
	 */
	public String toString()
	{
		return m_id.toString();
	}	// toString

	/**
	 * Compares this to a generic object.
	 *
	 * @returns <, =, or > 0 if the object is a obj_id, else throws
	 *		ClassCastException
	 */
	public int compareTo(Object o) throws ClassCastException
	{
		return compareTo((obj_id)o);
	}	// compareTo(Object)

	/**
	 * Compares this to another obj_id.
	 *
	 * @returns <, =, or > 0
	 */
	public int compareTo(obj_id id)
	{
		if (id == null)
			return 1;

		if (m_id.longValue() > id.m_id.longValue())
			return 1;
		else if (m_id.longValue() < id.m_id.longValue())
			return -1;
		return 0;
	}	// compareTo(obj_id)

	/**
	 * Compares this to a generic object.
	 *
	 * @returns true if the objects have the same data, false if not
	 */
	public boolean equals(Object o)
	{
		if (o == null)
			return false;

		try
		{
			int result = compareTo(o);
			if (result == 0)
				return true;
		}
		catch (ClassCastException err)
		{
		}
		return false;
	}	// equals

	/**
	 * Returns a hash code value for the obj_id.
	 *
	 * @return a hash code value for this obj_id
	 */
	public int hashCode()
	{
		// hash code from Effective Java, item 8, page 38
		int result = 17;
		result = 37 * result + (int)(m_id.longValue() ^ (m_id.longValue() >>> 32));
		return result;
	}	// hashCode

	/**************************************************************************/
	/**************************************************************************/
	//
	// Default access methods
	//
	/**************************************************************************/
	/**************************************************************************/

	/**
 	 * Called when an object is going to be destroyed, but hasn't actually been removed from the game yet.
	 */
	void flagDestroyed()
	{
		m_beingDestroyed = true;
	}

	private static void flagDestroyed(long id)
	{
		flagDestroyed((id == 0) ? null : obj_id.getObjId(id));
	}

	private static void flagDestroyed(obj_id id)
	{
		if (id == null || id.getValue() == 0)
			return;

		id.flagDestroyed();
	}

	/**************************************************************************/
	/**************************************************************************/
	//
	// Private methods
	//
	/**************************************************************************/
	/**************************************************************************/

	/**
	 * Class constructor.
	 *
	 * @param id		the object id value
	 */
	private obj_id(long id)
	{
		m_id = new Long(id);
		m_authoritative = false;
		m_loaded = false;
		m_initialized = false;
		m_loggedIn = false;
		m_beingDestroyed = false;
		m_scripts = new ArrayList();
		m_pendingScripts = new ArrayList();
		m_scriptLock = 0;
	}	// obj_id

	/**
	 * Class constructor.
	 *
	 * @param id		the object id value
	 */
	private obj_id(Long id)
	{
		m_id = id;
		m_authoritative = false;
		m_loaded = false;
		m_initialized = false;
		m_loggedIn = false;
		m_beingDestroyed = false;
		m_scripts = new ArrayList();
		m_pendingScripts = new ArrayList();
		m_scriptLock = 0;
	}	// obj_id

	private static void packAllScriptVars()
	{
		Iterator i = potentiallyDirtyObjects.iterator();
		if (i != null)
		{
			while (i.hasNext())
			{
				obj_id pdo = (obj_id)i.next();
				if (pdo != null)
				{
					// don't need to push out scriptvars if object doesn't exist on any other game server
					if (!base_class.hasProxyOrAuthObject(pdo))
					{
						if (pdo.scriptVars != null)
							pdo.scriptVars.clearDelta();

						continue;
					}

					byte[] deltaBuffer = pdo.packDeltaScriptVars();
					if (deltaBuffer != null && deltaBuffer.length > 0)
					{
						base_class.sendScriptVarsToProxies(pdo, deltaBuffer);
					}
				}
			}
		}
		potentiallyDirtyObjects.clear();
	}

	private static byte[] packScriptVars(long target)
	{
		return packScriptVars((target == 0) ? null : obj_id.getObjId(target));
	}
	private static byte[] packScriptVars(obj_id target)
	{
		byte[] result = null;
		if (target != null && target.scriptVars != null && target.scriptVars.size() > 0)
		{
			result = target.scriptVars.pack();
		}
		return result;
	}

	private byte[] packDeltaScriptVars()
	{
		byte[] result = null;
		if (scriptVars != null)
		{
			try
			{
				result = scriptVars.packDelta();
			}
			catch (IOException x)
			{
				System.err.println(x.toString());
				return null;
			}
		}
		return result;
	}

	private static void unpackScriptVars(long target, byte[] input)
	{
		unpackScriptVars((target == 0) ? null : obj_id.getObjId(target), input);
	}
	private static void unpackScriptVars(obj_id target, byte[] input)
	{
		if (target != null && input != null)
		{
			deltadictionary sv = target.getScriptVars();
			sv.unpack(input);
		}
	}

	private static void unpackDeltaScriptVars(long target, byte[] input)
	{
		unpackDeltaScriptVars((target == 0) ? null : obj_id.getObjId(target), input);
	}
	private static void unpackDeltaScriptVars(obj_id target, byte[] input)
	{
		if (target != null && input != null)
		{
			deltadictionary sv = target.getScriptVars();
			try
			{
				//				ByteArrayInputStream byteInput = new ByteArrayInputStream(input.getBytes());
				//				ObjectInputStream objectInput = new ObjectInputStream(byteInput);
				sv.unpackDelta(input);
			}
			catch (ClassNotFoundException c)
			{
				System.err.println(c);
			}
			catch (IOException x)
			{
				System.err.println("IOException: " + x);
			}
		}
	}

	/**
	 * Called when an obj_id is removed from the game. Note that the obj_id will
	 * stick around until all references to it have been removed.
	 *
	 * @param id		the object id
	 */
	private static void clearObjId(long id)
	{
		clearObjId((id == 0) ? null : obj_id.getObjId(id));
	}
	private static void clearObjId(obj_id id)
	{
		if (id == null || id.getValue() == 0)
			return;

		if (DEBUG)
		{
			System.out.println("obj_id factory request to clear id " + id);
		}

		script_entry.removeScriptListenee(id);
		id.m_authoritative = false;
		id.m_loaded = false;
		id.m_initialized = false;
		id.m_loggedIn = false;
		id.m_beingDestroyed = false;
		
		id.clearScriptVars();
		id.clearScriptDictionary();

		Object value = m_factory.get(id.m_id);
		if (value != null)
		{
			if (value instanceof obj_id)
			{
				if (value != id)
				{
					System.err.println("***ERROR: id passed to obj_id.clearObjId is not the id stored in the factory map!!! Id #" +
						Long.toString(id.getValue()));
				}
				if (DEBUG)
				{
					System.out.println("obj_id factory clearing id");
				}

				// replace the obj_id with a weak reference to itself; if there are no other references to it, it will be removed
				m_factory.put(id.m_id, new obj_id_reference(id, m_cleanup.getClearedObjectQueue()));
			}
		}
	}	// clearObjId

	/**
	 * Sets the authoritative flag for the obj_id.
	 *
	 * @param authoritative		flag that the object is authoritative
	 */
	private void setAuthoritative(boolean authoritative, int processId)
	{
		m_authoritative = authoritative;
		m_processId = processId;
		if (m_authoritative)
			setLoaded();				// if it's authoritative, it better be loaded!

		if (DEBUG)
		{
			System.out.println("Java obj_id " + this + " authority set to " + m_authoritative);
		}
	}	// setAuthoritative

	/**
	 * Sets the loaded flag for the object. The flag cannot be cleared, the obj_id
	 * must be cleared instead.
	 **/
	private void setLoaded()
	{
		m_loaded = true;

		// replace the soft reference to this id with a hard reference
		m_factory.put(m_id, this);

		if (DEBUG)
		{
			System.out.println("Java obj_id " + this + " loaded");
		}
	}	// setLoaded

	/**
	 * Sets the initialized flag for the object. The flag cannot be cleared, the obj_id
	 * must be cleared instead.
	 **/
	private void setInitialized()
	{
		m_initialized = true;

		// replace the soft reference to this id with a hard reference
		m_factory.put(m_id, this);

		if (DEBUG)
		{
			System.out.println("Java obj_id " + this + " initialized");
		}
	}	// setInitialized

	/**
	 * Sets/clears the logged in flag for the object.
	 **/
	private void setLoggedIn(boolean loggedIn)
	{
		if (!m_loggedIn && loggedIn)
		{
		}

		m_loggedIn = loggedIn;
		if (DEBUG)
		{
			System.out.println("Java obj_id " + this + " logged in = " + m_loggedIn);
		}
	}	// setInitialized

	/**
	 * Adds a single script to this object. The script is put on the end of the script list.
	 *
	 * @param script		the script to attach
	 */
	private void attachScript(String script)
	{
		if (DEBUG)
			System.out.println("Java obj_id.attachScript enter, script = " + script);

		if (m_scripts == null)
		{
			System.err.println("ERROR: Java obj_id.attachScript: m_scripts == null!");
			return;
		}
		if (script == null || script.length() == 0)
		{
			System.err.println("Java obj_id.attachScript trying to attach empty script name to object " + m_id);
			return;
		}

		if (m_scriptLock > 0)
		{
			m_pendingScripts.add(new pending_script(true, script));
			if (DEBUG)
				System.out.println("Java obj_id.attachScript pending attach script " + script + " to object " + m_id);
			return;
		}

		if (DEBUG)
			System.out.println("Java obj_id.attachScript attaching script " + script + " to object " + m_id);

		//		System.out.println("Java obj_id.attachScript m_scripts before add, size = " + Integer.toString(m_scripts.size()) + ", items = ");
		//		for (int i = 0; i < m_scripts.size(); ++i)
		//			System.out.println(m_scripts.get(i).toString());

		if (!script.startsWith(SCRIPT_PREFIX))
			script = SCRIPT_PREFIX + script;

		if (VALIDATE_SCRIPTS)
		{
			if (m_scripts.contains(script))
			{
				System.err.println("WARNING: Java obj_id.attachScript(single) tried to add script " +
					script + " to object " + this + " more than once!");
				return;
			}
		}
		m_scripts.add(0, script);

		//		System.out.println("Java obj_id.attachScript m_scripts after add, size = " + Integer.toString(m_scripts.size()) + ", items = ");
		//		for (int i = 0; i < m_scripts.size(); ++i)
		//			System.out.println(m_scripts.get(i).toString());

		//  		System.out.println("Java obj_id.attachScript exit");
	}	// attachScript(String)

	/**
	 * Adds multiple scripts to this object. The scripts are put on the end of the script list.
	 *
	 * @param scripts		the scripts to attach
	 */
	private void attachScript(String[] scripts)
	{
		if (m_scripts == null)
		{
			System.err.println("ERROR: Java obj_id.attachScript[]: m_scripts == null!");
			return;
		}
		if (scripts == null)
		{
			System.err.println("ERROR: Java obj_id.attachScript[]: scripts == null!");
			return;
		}

		// the scripts need to be added in reverse order to the front of the list
		for (int i = 0; i < scripts.length; ++i)
		{
			if (scripts[i] == null || scripts[i].length() == 0)
			{
				System.err.println("Java obj_id.attachScript[] trying to attach empty script name to object " + m_id);
				continue;
			}

			String script = scripts[i];
			if (!script.startsWith(SCRIPT_PREFIX))
				script = SCRIPT_PREFIX + script;

			if (VALIDATE_SCRIPTS)
			{
				if (m_scripts.contains(script))
				{
					System.err.println("WARNING: Java obj_id.attachScript(array) tried to add script " +
						script + " to object " + this + " more than once!");
					continue;
				}
			}
			if (DEBUG)
				System.out.println("Java obj_id.attachScript[] attaching script " + script + " to object " + m_id);
			m_scripts.add(0, script);
		}
	}	// attachScript(String[])

	/**
	 * Removes a script from this object.
	 *
	 * @param script		the script to detach
	 */
	private void detachScript(String script)
	{
		//		System.out.println("Java obj_id.detachScript enter");
		if (m_scripts == null)
		{
			System.err.println("ERROR: Java obj_id.detachScript: m_scripts == null!");
			return;
		}
		if (script == null)
		{
			System.err.println("ERROR: Java obj_id.detachScript: script == null!");
			return;
		}
		if (m_scriptLock > 0)
		{
			m_pendingScripts.add(new pending_script(false, script));
			if (DEBUG)
				System.out.println("Java obj_id.detachScript pending detach script " + script);
			return;
		}
		if (DEBUG)
		{
			System.out.println("Java obj_id.detachScript detaching script " + script);
		}
		int index = m_scripts.indexOf(SCRIPT_PREFIX + script);
		if (index >= 0)
			m_scripts.remove(index);

		//		System.out.println("Java obj_id.detachScript exit");
	}	// detachScript()

	/**
	 * Removes all the scripts from this object.
	 */
	private void detachAllScripts()
	{
		//		System.out.println("Java obj_id.detachAllScripts enter");
		if (m_scripts == null)
		{
			System.err.println("ERROR: Java obj_id.detachAllScripts: m_scripts == null!");
			return;
		}
		if (DEBUG)
		{
			System.out.println("Java obj_id.detachAllScripts detaching all scripts");
		}
		m_scripts.clear();

		//		System.out.println("Java obj_id.detachAllScripts exit");
	}	// detachAllScripts()

	private LinkedList getAllScriptsWithPending()
	{
		if (m_scripts == null)
			return null;

		// we have to set up a list of our scripts, adding and removing scripts from the pending list as needed
		LinkedList scriptList = new LinkedList(m_scripts);

		if (m_pendingScripts != null)
		{
			int count = m_pendingScripts.size();
			for (int i = 0; i < count; ++i)
			{
				pending_script pending = (pending_script)m_pendingScripts.get(i);
				String scriptName = null;
				if (!pending.script.startsWith(SCRIPT_PREFIX))
					scriptName = SCRIPT_PREFIX + pending.script;
				else
					scriptName = pending.script;
				if (pending.add)
					scriptList.addFirst(scriptName);
				else
					scriptList.remove(scriptName);
			}
		}
		return scriptList;
	}

	/**
	 * \defgroup serialize Serialize support functions
	 * @{ */

	private void writeObject(ObjectOutputStream out) throws IOException
	{
		out.defaultWriteObject();
		// write the obj_id value
		out.writeLong(m_id.longValue());
		if (DEBUG)
		{
			System.out.println("obj_id Serialize writeObject writing id #" + m_id.longValue());
		}
	}

	private void readObject(ObjectInputStream in) throws IOException, ClassNotFoundException
	{
		in.defaultReadObject();
		if (m_nextSerializedId == -1)
		{
			// read the obj_id value
			m_nextSerializedId = in.readLong();
			if (DEBUG)
			{
				System.out.println("obj_id Serialize readObject reading id #" + m_nextSerializedId);
			}
		}
		else
			System.err.println("ERROR: Reading new obj_id while serializedId still pending!!!");
	}

	private Object readResolve() throws ObjectStreamException
	{
		if (m_nextSerializedId != -1)
		{
			if (DEBUG)
			{
				System.out.println("obj_id Serialize readResolve creating obj_id #" + m_nextSerializedId);
			}
			obj_id result = obj_id.getObjId(m_nextSerializedId);
			m_nextSerializedId = -1;
			return result;
		}
		System.err.println("ERROR: Deserializing readResolve obj_id while serializedId == -1!!!");
		return null;
	}

	/*@}*/

	static
	{
		// start the cleanup thread
		m_cleanup = new obj_id_cleanup();
		m_cleanup.start();
	}

}	// class obj_id
