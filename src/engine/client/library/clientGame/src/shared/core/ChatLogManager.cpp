// ============================================================================
//
// ChatLogManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ChatLogManager.h"

#include "clientGame/ClientTextManager.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/TextIterator.h"
#include <vector>

#include <sys/stat.h>
#include <io.h>

// ============================================================================
//
// ChatLogManagerNamespace
//
// ============================================================================

//-----------------------------------------------------------------------------
namespace ChatLogManagerNamespace
{
	typedef std::vector<Unicode::String> ChatText;

	bool               s_installed = false;
	bool               s_enabled = false;
	unsigned int const s_maxCachedLineCount = 50;
	unsigned int const s_autoFlushMaxCachedLineCount = 1000;
	ChatText           s_chatText;

	std::string getFileName();
}

using namespace ChatLogManagerNamespace;

//-----------------------------------------------------------------------------
std::string ChatLogManagerNamespace::getFileName()
{
	std::string result;
	std::string loginId;
	std::string clusterName;
	Unicode::String playerName;
	NetworkId id;

	if (Game::getPlayerPath(loginId, clusterName, playerName, id))
	{
		result += "profiles/";
		result += loginId;
		result += "/";
		result += clusterName;
		result += "/";
		char text[256];
		snprintf(text, sizeof(text), "%s_chatlog.txt", id.getValueString().c_str());
		result += text;
	}

	return result;
}

// ============================================================================
//
// ChatLogManager
//
// ============================================================================

//-----------------------------------------------------------------------------
void ChatLogManager::install()
{
	InstallTimer const installTimer("ChatLogManager::install");

	DEBUG_FATAL(s_installed, ("ChatLogManager::install() - Already installed."));

	s_installed = true;

	if(ConfigClientGame::getEnableChatLogging())
		s_enabled = true;

	ExitChain::add(ChatLogManager::remove, "ChatLogManager::remove", 0, false);
}

//-----------------------------------------------------------------------------
void ChatLogManager::remove()
{
	flush();
	s_installed = false;
}

//-----------------------------------------------------------------------------
void ChatLogManager::setEnabled(bool const enabled)
{
	flush();

	s_enabled = enabled;
}

//-----------------------------------------------------------------------------
bool ChatLogManager::isEnabled()
{
	return s_enabled;
}

//-----------------------------------------------------------------------------
void ChatLogManager::appendLine(Unicode::String const &text)
{
	if (isEnabled() &&
	    !text.empty())
	{
		s_chatText.push_back(text);

		if (s_chatText.size() >= s_maxCachedLineCount)
		{
			// Enough cached lines, dump them to disk

			flush();
		}
	}
}

//-----------------------------------------------------------------------------
void ChatLogManager::gameStart()
{
	Unicode::String date;
	CuiUtils::FormatDate(date, CuiUtils::GetSystemSeconds());

	char text[256];
	snprintf(text, sizeof(text), "Logging In [%s]\n", Unicode::wideToNarrow(date).c_str());

	s_chatText.push_back(Unicode::narrowToWide(text));
}

//-----------------------------------------------------------------------------
void ChatLogManager::flush()
{
	if (!s_chatText.empty())
	{
		std::string fileName(getFileName());

		if (!fileName.empty())
		{
			FILE *fp = fopen(fileName.c_str(), "a");
			
			if (fp != NULL)
			{
				if (_filelength(_fileno(fp)) == 0)
				{
					fprintf(fp, "%c%c%c", 0xEF, 0xBB, 0xBF);
				}

				ChatText::const_iterator iterChatText = s_chatText.begin();

				for (; iterChatText != s_chatText.end(); ++iterChatText)
				{
					Unicode::String printableText(TextIterator(*iterChatText).getPrintableText());

					// Remove any newline characters

					Unicode::UTF8String text(Unicode::wideToUTF8(printableText));
					int position = 0;

					while (position != std::string::npos)
					{
						position = text.find('\n', position);

						if (position != std::string::npos)
						{
							text[position] = ' ';
						}
					}

					// Save the text to a file

					if (!text.empty())
					{
						fprintf(fp, "%s\n", text.c_str());
					}
				}

				fclose(fp);
				fp = NULL;

				s_chatText.clear();
			}
		}

		if (s_chatText.size() > s_autoFlushMaxCachedLineCount)
		{
			s_chatText.clear();
		}
	}
}

//-----------------------------------------------------------------------------
void ChatLogManager::showEnabledText()
{
	CuiChatRoomManager::sendPrelocalizedChat (ClientTextManager::colorText(CuiStringIds::chat_log_enabled.localize(), ClientTextManager::TT_systemMessage));

	std::string fileName(getFileName());

	if (!fileName.empty())
	{
		struct _stat fileInfo;
		int result = _stat(fileName.c_str(), &fileInfo);

		if (result == 0)
		{
			CuiStringVariablesData data;
			char text[256];
			Unicode::String chatLogFileSizeString;

			if (fileInfo.st_size > 1024 * 1024)
			{
				snprintf(text, sizeof(text), "%.1f", static_cast<float>(fileInfo.st_size) / 1024.0f / 1024.0f);
				data.targetName = Unicode::narrowToWide(text);
				CuiStringVariablesManager::process(CuiStringIds::chat_log_file_size_mb, data, chatLogFileSizeString);
			}
			else
			{
				snprintf(text, sizeof(text), "%d", fileInfo.st_size / 1024);
				data.targetName = Unicode::narrowToWide(text);
				CuiStringVariablesManager::process(CuiStringIds::chat_log_file_size_kb, data, chatLogFileSizeString);
			}

			CuiChatRoomManager::sendPrelocalizedChat(ClientTextManager::colorText(chatLogFileSizeString, ClientTextManager::TT_systemMessage));
		}
	}
}

//-----------------------------------------------------------------------------
void ChatLogManager::showDisabledText()
{
	CuiChatRoomManager::sendPrelocalizedChat(ClientTextManager::colorText(CuiStringIds::chat_log_disabled.localize(), ClientTextManager::TT_systemMessage));
}

// ============================================================================
