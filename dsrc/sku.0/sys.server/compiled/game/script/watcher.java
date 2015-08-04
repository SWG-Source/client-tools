/**
 * Title:        watcher
 * Description:  Watcher thread that prevents scripts from executing too long
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;

import java.util.Stack;


class watcher extends Thread
{
	private static final boolean debugOn = false;

	private static final int STATE_SCRIPT_WAIT    = 0;  // waiting for a script to run
	private static final int STATE_SCRIPT_TIMEOUT = 1;  // waiting for a script to timeout
	private static final int STATE_SCRIPT_KILL    = 2;  // waiting for a script to be killed

	private int    m_warnTime;				// time a script is allowed to run before a warning is generated
	private int    m_interruptTime;			// time a script is allowed to run before it is interrupted
	private int    m_state;
	private Thread m_scriptThread;			// thread the scripts are running from
	private Stack  m_scripts;				// scripts being run
	private Stack  m_methods;				// methods being run
	private String m_lastScript = null;		// last script run
	private String m_lastMethod = null;		// last method run

	/**
	 * Class constructor.
	 *
	 * @param warnMs		time a script is allowed to run before a warning is generated
	 * @param interruptMs	time a script is allowed to run before it is interrupted
	 */
	public watcher(int warnMs, int interruptMs)
	{
		super("watcher_Thread");
		m_warnTime = warnMs;
		m_interruptTime = interruptMs;
		m_state = STATE_SCRIPT_WAIT;
		m_scriptThread = currentThread();
		m_scripts = new Stack();
		m_methods = new Stack();
		if (debugOn)
			System.out.println("watcher set up to watch thread " + m_scriptThread);
		setDaemon(true);
	}	// watcher()

	/**
	 * Thread entry point.
	 */
	public void run()
	{
		boolean signalScript = false;

		synchronized (this)
		{
			if (debugOn)
			{
				System.out.println("Script watcher run enter, watch thread = " + Thread.currentThread());
			}

			for ( ;; )
			{
				try
				{
					m_state = STATE_SCRIPT_WAIT;
					if (signalScript)
					{
						// signal the main thread that we have switched states
						synchronized (m_scriptThread)
						{
							m_scriptThread.notifyAll();
						}
					}
					wait();
				}
				catch( InterruptedException err )
				{
					System.err.println("WARNING: Script watcher SCRIPT_WAIT interrupted, this should never happen!");
					continue;
				}
				// wait for our warn time to pass
				try
				{
					if (debugOn)
					{
						if (!m_scripts.empty())
						{
							System.out.println("Script watcher waiting for script "  + m_scripts.peek() + ", function " +
								m_methods.peek() + " to finish running, time = " + System.currentTimeMillis());
						}
						else
						{
							System.out.println("Script watcher waiting for a script <unknown> to finish running, time = " +
								System.currentTimeMillis());
						}
					}
					m_state = STATE_SCRIPT_TIMEOUT;
					// signal the main thread that we have switched states
					synchronized (m_scriptThread)
					{
						m_scriptThread.notifyAll();
					}
					wait(m_warnTime);
					if (m_state != STATE_SCRIPT_TIMEOUT)
					{
						signalScript = true;
						if (debugOn)
						{
							if (m_lastScript != null)
							{
								System.out.println("Script watcher signaled that script " + m_lastScript + ", function " +
									m_lastMethod + " finished");
							}
							else
							{
								System.out.println("Script watcher signaled that script <unknown> finished");
							}
						}
						continue;
					}
				}
				catch ( InterruptedException err )
				{
					System.err.println("WARNING: Script watcher STATE_SCRIPT_TIMEOUT interrupted, this should never happen!");
					continue;
				}
				// wait for our interrupt time to pass
				try
				{
					if (!m_scripts.empty())
					{
						System.err.println("WARNING: script " + m_scripts.peek() + ", function " +
							m_methods.peek() + " running for " + m_warnTime + "ms, currentTime = " +  System.currentTimeMillis());
					}
					else
					{
						System.err.println("WARNING: script <unknown> running for " + m_warnTime + "ms, currentTime = " +
							System.currentTimeMillis());
					}
					m_state = STATE_SCRIPT_KILL;
					wait(m_interruptTime);
					if (m_state != STATE_SCRIPT_KILL)
					{
						signalScript = true;
						if (debugOn)
						{
							if (m_lastScript != null)
							{
								System.out.println("Script watcher signaled that script " + m_lastScript + ", function " +
									m_lastMethod + " finished");
							}
							else
							{
								System.out.println("Script watcher signaled that script <unknown> finished");
							}
						}
						continue;
					}
					else
					{
						if (!m_scripts.empty())
						{
							System.err.println("WARNING: Script watcher forcing script " + m_scripts.peek() +
								", function " + m_methods.peek() + " to exit, currentTime = " +  System.currentTimeMillis());
						}
						else
						{
							System.err.println("WARNING: Script watcher forcing script <unknown> to exit, currentTime = " +
								System.currentTimeMillis());
						}
						m_scriptThread.interrupt();
					}
				}
				catch ( InterruptedException err )
				{
					System.err.println("WARNING: Script watcher STATE_SCRIPT_KILL interrupted, this should never happen!");
				}
				catch( SecurityException err )
				{
					System.err.println("WARNING Script watcher failed to interrupt script! Err = " + err);
				}
			}
		}
    }	// run()

	/**
	 * Flags that a script is starting to be run.
	 */
	public void scriptStart(String script, String method)
	{
		if (debugOn)
		{
			System.out.println("Script watcher scriptStart enter, script "  + script + ", function " + method);
		}

		m_lastScript = null;
		m_lastMethod = null;
		resetToWait();

		m_scripts.push(script);
		m_methods.push(method);
		startTimeout();
	}	// scriptStart()

	/**
	 * Flags that a script has finished being run.
	 */
	public void scriptStop()
	{
		if (debugOn)
		{
			if (!m_scripts.empty())
			{
				System.out.println("Script watcher scriptStop enter, script " + m_scripts.peek() +
					", function " + m_methods.peek());
			}
			else
			{
				System.out.println("Script watcher scriptStop enter, script <unknown>");
			}
		}

		if (!m_scripts.empty())
		{
			m_lastScript = (String)m_scripts.pop();
			m_lastMethod = (String)m_methods.pop();
		}

		resetToWait();

		// if we are still running scripts, we need to start the timer again
		if (!m_scripts.empty())
			startTimeout();
	}	// scriptStop()

	/**
	 * Forces the watcher into the starting wait state.
	 */
	private void resetToWait()
	{
		if (m_state != STATE_SCRIPT_WAIT)
		{
			synchronized (m_scriptThread)
			{
				if (debugOn)
				{
					System.out.println("Script forcing watcher to wait state");
				}

				m_state = STATE_SCRIPT_WAIT;
				synchronized (this)
				{
					notifyAll();
				}
				// wait until the watcher has changed states
				try
				{
					if (debugOn)
					{
						System.out.println("Script waiting for watcher to enter wait state");
					}
					m_scriptThread.wait();
				}
				catch ( InterruptedException err )
				{
				}
				if (debugOn)
				{
					System.out.println("Watcher waiting, continuing script");
				}
			}
		}
	}	// resetToWait()

	/**
	 * Starts the watcher timing out the current script.
	 */
	private void startTimeout()
	{
		if (m_state != STATE_SCRIPT_WAIT)
			return;

		synchronized (m_scriptThread)
		{
			synchronized (this)
			{
				if (debugOn)
				{
					System.out.println("Script notifying watcher to start timeout");
				}
				notifyAll();
			}
			// wait until the watcher has changed states
			try
			{
				if (debugOn)
				{
					System.out.println("Script waiting for watcher to change states");
				}
				m_scriptThread.wait();
			}
			catch ( InterruptedException err )
			{
			}
			if (debugOn)
			{
				System.out.println("Script ready to exectute");
			}
		}
	}	// startTimeout()

}	// class watcher


