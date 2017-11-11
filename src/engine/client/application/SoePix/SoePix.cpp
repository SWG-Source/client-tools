// ======================================================================
//
// SoePix.cpp
// Copyright 2004 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include <windows.h>
#include <PixPlugin.h>
#include <stdio.h>

// ======================================================================
// Data
// ======================================================================

typedef void (__stdcall *PollRoutine)();

PollRoutine        pollRoutine;
UINT               currentFrame = 0xffffffffU;
int                pixCounters;
int const          pixCountersGrow = 32;
int                pixCountersMax;
PIXCOUNTERINFO *   pixCounterInfo;
BYTE * *           pixCounterData;
DWORD *            pixCounterDataSize;
int const          zero = 0;
int                tempBufferLength;
WCHAR *            tempBuffer;
bool *             clientActive;
char const * const configFile = "C:\\Program Files\\Microsoft DirectX 9.0 SDK (October 2004)\\Utilities\\PIX\\SoePix.cfg";

// ======================================================================
// Helper functions
// ======================================================================

void Log(char const * const format, ...)
{
	char buffer[256];

	va_list va;
	va_start(va, format);
    _vsnprintf(buffer, sizeof(buffer), format, va);
	buffer[sizeof(buffer)-1] = 0;
	va_end(va);

    OutputDebugStringA(buffer);
}

bool Match(char const * a, WCHAR const * b)
{
	while (*a || *b)
	{
		if (static_cast<WCHAR>(*a) != *b)
			return false;
		++a;
		++b;
	}
	return true;
}

int GetCounterIndex(char const * const counterName, PIXCOUNTERDATATYPE const type)
{
	for (int i = 0; i < pixCounters; ++i)
		if (Match(counterName, pixCounterInfo[i].pstrName) && pixCounterInfo[i].pcdtDataType == type)
			return i;

	return -1;
}

class Installer
{
public:
	Installer()
	{
		// open the config files
		FILE * f = fopen(configFile, "rt");
		if (!f)
		{
			Log("Could not open %s\n", configFile);
			return;
		}

		// process all of its lines
		int line = 0;
		char buffer[MAX_PATH];
		while (fgets(buffer, sizeof(buffer), f))
		{
			++line;

			// strip any newlines
			char * const newLine = strchr(buffer, '\n');
			if (newLine)
				*newLine = '\0';

			// handle comment lines
			if (buffer[0] != '#')
			{
				// make sure the line is in this format: <type><whitespace<name...>
				if (buffer[0] && (buffer[1] == ' ' || buffer[1] == '\t') && buffer[2])
				{
					char const typeChar = buffer[0];
					char const * const name = buffer+2;

					PIXCOUNTERDATATYPE type = PCDT_RESERVED;
					char const * typeText = "UNKNOWN";
					switch (typeChar)
					{
						case 'i': type = PCDT_INT; typeText = "int"; break;
						case 'f': type = PCDT_FLOAT; typeText = "float"; break;
						case 's': type = PCDT_STRING; typeText = "string"; break;
						case 'I': type = PCDT_INT64; typeText = "int64"; break;
					}

					if (type != PCDT_RESERVED)
					{
						// grow the counter array if it becomes full
						if (pixCounters == pixCountersMax)
						{
							pixCountersMax += pixCountersGrow;
							PIXCOUNTERINFO * newPixCounterInfo = new PIXCOUNTERINFO[pixCountersMax];
							memcpy(newPixCounterInfo, pixCounterInfo, sizeof(*newPixCounterInfo) * pixCounters);
							delete [] pixCounterInfo;
							pixCounterInfo = newPixCounterInfo;
						}

						// convert the name to a wide char name
						int const nameLength = static_cast<int>(strlen(name))+1;
						WCHAR * wbuffer = new WCHAR[nameLength];
						for (int i = 0; i < nameLength; ++i)
							wbuffer[i] = static_cast<WCHAR>(name[i]);

						// add the entry to the table
						PIXCOUNTERINFO & pixCounter = pixCounterInfo[pixCounters];
						pixCounter.counterID = pixCounters;
						pixCounter.pstrName = wbuffer;
						pixCounter.pcdtDataType = type;
						++pixCounters;

						// Log("Adding counter %s type %s line %d\n", buffer+2, typeText, line);
					}
					else
						Log("Unknown counter type '%c' line %d\n", typeChar, line);
				}
				else
					Log("Badly formatted line %d: %s\n", line, buffer);
			}
		}

		// close the file
		fclose(f);
		f = NULL;

		// allocate the arrays for the data and initialize the data to 0
		pixCounterData = new BYTE*[pixCounters];
		pixCounterDataSize = new DWORD[pixCounters];
		memset(pixCounterData, 0, sizeof(*pixCounterData) * pixCounters);
		memset(pixCounterDataSize, 0, sizeof(*pixCounterDataSize) * pixCounters);
	}

	~Installer()
	{
		// free all the memory
		for (int i = 0; i < pixCounters; ++i)
			delete [] pixCounterInfo[i].pstrName;
		delete [] pixCounterInfo;
		delete [] pixCounterData;
		delete [] pixCounterDataSize;
		delete [] tempBuffer;
	}

} installer;

// ======================================================================
// Pix functions
// ======================================================================

BOOL WINAPI PIXGetPluginInfo(PIXPLUGININFO * pPIXPluginInfo)
{
	pPIXPluginInfo->iPluginVersion = 0x0100;
	pPIXPluginInfo->iPluginSystemVersion = PIX_PLUGIN_SYSTEM_VERSION;
	pPIXPluginInfo->pstrPluginName = L"SOE PIX Plugin";
	return TRUE;
}

BOOL WINAPI PIXGetCounterInfo(DWORD * pdwReturnCounters, PIXCOUNTERINFO * * ppCounterInfoList)
{
	*pdwReturnCounters = pixCounters;
	*ppCounterInfoList = pixCounterInfo;
	return TRUE;
}

BOOL WINAPI PIXGetCounterDesc(PIXCOUNTERID id, WCHAR * * ppstrCounterDesc)
{
	if (id < 0 || id >= pixCounters)
	{
		Log("PIXGetCounterDesc invalid counter %d\n", id);
		return FALSE;
	}
	*ppstrCounterDesc = pixCounterInfo[id].pstrName;
	return TRUE;
}

BOOL WINAPI PIXBeginExperiment(PIXCOUNTERID id, const WCHAR *)
{
	if (clientActive)
		*clientActive = true;

	if (id < 0 || id >= pixCounters)
	{
		Log("PIXBeginExperiment invalid counter %d\n", id);
		return FALSE;
	}

	if (pixCounterData[id] == NULL || pixCounterDataSize[id] == 0)
	{
		Log("PIXBeginExperiment counter %d has not been bound\n", id);
		pixCounterDataSize[id] = sizeof(int);
		pixCounterData[id] = reinterpret_cast<BYTE *>(const_cast<int *>(&zero));
	}

	return TRUE;
}

BOOL WINAPI PIXEndFrame(PIXCOUNTERID id, UINT iFrame, DWORD * pdwReturnBytes, BYTE * * ppReturnData)
{
	if (iFrame != currentFrame)
	{
		currentFrame = iFrame;
		if (pollRoutine)
			(*pollRoutine)();
	}

	if (id < 0 || id >= pixCounters)
	{
		Log("PIXEndFrame invalid counter %d/%d\n", id, pixCounters);
		return FALSE;
	}

	if (pixCounterInfo[id].pcdtDataType == PCDT_STRING)
	{
		char const * const src = *reinterpret_cast<char const * const *>(pixCounterData[id]);
		if (!src)
		{
			*pdwReturnBytes = sizeof(WCHAR);
			*ppReturnData = reinterpret_cast<BYTE *>(const_cast<int *>(&zero));
			return TRUE;
		}

		int const length = static_cast<int>(strlen(src) + 1);
		if (length > tempBufferLength)
		{
				delete [] tempBuffer;
				tempBufferLength = length;
				tempBuffer = new WCHAR[tempBufferLength];
		}
		for (int i = 0; i < length; ++i)
			tempBuffer[i] = static_cast<WCHAR>(src[i]);

		*pdwReturnBytes = length * sizeof(WCHAR);
		*ppReturnData = reinterpret_cast<BYTE *>(tempBuffer);
		return TRUE;
	}

	*pdwReturnBytes = pixCounterDataSize[id];
	*ppReturnData = pixCounterData[id];
	return TRUE;
}

BOOL WINAPI PIXEndExperiment(PIXCOUNTERID id)
{
	if (id < 0 || id >= pixCounters)
	{
		Log("PIXEndExperiment invalid counter %d\n", id);
		return FALSE;
	}

	return TRUE;
}

// ======================================================================
// Game accessible functions
// ======================================================================

void WINAPI ClientInstall(bool * active, PollRoutine p)
{
	clientActive = active;
	pollRoutine = p;
}

BOOL WINAPI SetCounterFloat(char const * counterName, float * value)
{
	int const id = GetCounterIndex(counterName, PCDT_FLOAT);
	if (id < 0)
		return FALSE;

	pixCounterData[id] = reinterpret_cast<BYTE *>(value);
	pixCounterDataSize[id] = sizeof(float);
	return TRUE;
}

BOOL WINAPI SetCounterInt(char const * counterName, int * value)
{
	int const id = GetCounterIndex(counterName, PCDT_INT);
	if (id < 0)
		return FALSE;

	pixCounterData[id] = reinterpret_cast<BYTE *>(value);
	pixCounterDataSize[id] = sizeof(int);
	return TRUE;
}

BOOL WINAPI SetCounterInt64(char const * counterName, __int64 * value)
{
	int const id = GetCounterIndex(counterName, PCDT_INT64);
	if (id < 0)
		return FALSE;

	pixCounterData[id] = reinterpret_cast<BYTE *>(value);
	pixCounterDataSize[id] = sizeof(__int64);
	return TRUE;
}

BOOL WINAPI SetCounterString(char const * counterName, char const * const * value)
{
	int const id = GetCounterIndex(counterName, PCDT_STRING);
	if (id < 0)
		return FALSE;

	pixCounterData[id] = const_cast<BYTE *>(reinterpret_cast<BYTE const *>(value));
	return TRUE;
}

// ======================================================================
