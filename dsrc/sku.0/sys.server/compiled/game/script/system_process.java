package script;

import java.lang.Runtime;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.io.IOException;
import java.lang.InterruptedException;

public class system_process
{
	public static String runAndGetOutput(String commandLine) 
	{
		system_process p = new system_process(commandLine);
		p.waitFor();
		return p.getOutput();
	}

	public static int runAndGetExitCode(String commandLine)
	{
		system_process p = new system_process(commandLine);
		p.waitFor();
		return p.getExitValue();
	}		
	
	public system_process(String commandLine)
	{
		run = Runtime.getRuntime();
		if(run != null)
		{
			try
			{
				process = run.exec(commandLine);
			}
			catch(Exception e)
			{
			}
		}
	}

	public void waitFor()
	{
		if(process != null)
		{
			try
			{
				process.waitFor();
			}
			catch(Exception e)
			{
			}
		}
	}
	
	public void putAndCloseInput(String input)
	{
		if(process != null)
		{
			OutputStream processInput = process.getOutputStream();
			byte[] toWrite = input.getBytes();
			try
			{
				processInput.write(toWrite);
				processInput.flush();
				processInput.close();
			}
			catch(Exception e)
			{
			}
		}
	}

	public int getExitValue()
	{
		if(process != null)
			return process.exitValue();
		return -1;
	}

	public String getOutput()
	{
		String result = getStandardOutput();
		String errorOutput = getErrorOutput();
		if(result == null && errorOutput != null)
		{
			result = errorOutput;
		}
		else if(result != null && errorOutput != null)
		{
				result += errorOutput;
		}
		return result;
	}

	public String getStandardOutput()
	{
		String result = null;
		if(process != null)
		{
			return getProcessOutput(process.getInputStream());
		}
		return result;
	}

	public String getErrorOutput()
	{
		String result = null;
		if(process != null)
		{
			return getProcessOutput(process.getErrorStream());
		}
		return result;
	}
	
	private String getProcessOutput(InputStream processOutput)
	{
		String result = null;
		if(processOutput != null)
		{
			try
			{
				InputStreamReader reader = new InputStreamReader(processOutput);
				BufferedReader buffered = new BufferedReader(reader, 1024 * 32);
				while(buffered.ready())
				{
					if(result == null)
						result = new String();
					result += buffered.readLine() + "\n";
				}
			}
			catch(Exception e)
			{
				result = "an exception occurred while retrieving process output " + e;
			}
		}
		return result;
	}
	
	private Runtime run = null;
	private Process process = null;
}
