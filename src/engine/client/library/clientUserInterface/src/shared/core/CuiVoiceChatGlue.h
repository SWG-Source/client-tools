#ifndef SWGVIVOX_H
#define SWGVIVOX_H


#pragma warning(disable:4100 4244)
#include "vivoxSharedWrapper/Vivox.h"
#pragma warning(default:4100 4244)


#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Clock.h"

#include "CuiVoiceChatManager.h"

struct SwgVivoxGlue
{
	static void log( VivoxLogSeverity severity, const char* pattern, ... )
	{
		char buffer[4 * 1024];
		UNREF(severity);
		va_list valist;
		va_start( valist, pattern );
		vsnprintf(buffer, sizeof(buffer), pattern, valist);
		//ConsoleWarning(buffer);
		va_end( valist );

		CuiVoiceChatManager::DebugOutputLevel level = CuiVoiceChatManager::DOL_Spam;
		switch (severity)
		{
		case VLS_DEBUG: level = CuiVoiceChatManager::DOL_Debug; break;
		case VLS_INFO: level = CuiVoiceChatManager::DOL_Info; break;
		case VLS_WARN: level = CuiVoiceChatManager::DOL_Warning; break;
		case VLS_ERROR: level = CuiVoiceChatManager::DOL_Error; break;
		case VLS_CRITICAL: level = CuiVoiceChatManager::DOL_Error; break;
		default: break;
		}
		CuiVoiceChatManager::debugOuputString(level, buffer);	
	}

	static uint64 getTimeMS()
	{
		return Clock::timeMs();
	}

	static void sleep( unsigned ms )
	{
		Sleep(ms);
	}

	static void warning( const char* sFile, int iLine, const char* pattern, ... )
	{
		char buffer[4 * 1024];
		UNREF(sFile);
		UNREF(iLine);
		va_list valist;
		va_start( valist, pattern );
		vsnprintf(buffer, sizeof(buffer), pattern, valist);
		Warning(buffer);
		va_end( valist );

		CuiVoiceChatManager::debugOuputString(CuiVoiceChatManager::DOL_Warning,buffer);
	}

	static void fatal( const char* sFile, int iLine, const char* pattern, ... )
	{
		char buffer[4 * 1024];
		UNREF(sFile);
		UNREF(iLine);
		va_list valist;
		va_start( valist, pattern );
		vsnprintf(buffer, sizeof(buffer), pattern, valist);
		Fatal(buffer);
		va_end( valist );
	}

	static const char* getGameName()
	{
		return "SWG";
	}

	static int getLogLevel()
	{
		return CuiVoiceChatManager::getApiLogLevel();
	}
};

typedef Vivox< std::string, SwgVivoxGlue > SwgVivox;

#endif