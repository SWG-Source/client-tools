// ======================================================================
//
// SwgCuiG15Lcd.cpp
// Copyright 2006 Sony Online Entertainment LLC
// All rights reserved
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiG15Lcd.h"

#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerMoneyManagerClient.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiChatManager.h"
#include "clientUserInterface/CuiPersistentMessageManager.h"
#include "sharedGame/PvpData.h"
#include "sharedObject/VolumeContainer.h"
#include "swgClientUserInterface/SwgCuiChatWindow.h"
#include "swgClientUserInterface/SwgCuiChatWindow_tab.h"
#include "swgClientUserInterface/SwgCuiHud.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"

#include "EZ_LCD.h"

//#undef this if the LCD causes problems
#define USE_LCD

// ======================================================================

namespace SwgCuiG15LcdNamespace
{
#ifdef USE_LCD
	CEzLcd * s_lcd = 0;
	int s_lcdDisplaying = 1;
	HANDLE s_lcdStaticTitle;
	HANDLE s_lcdStatic1Text;
	HANDLE s_lcdStaticHpText;
	HANDLE s_lcdStaticHpValue;
	HANDLE s_lcdStaticActionText;
	HANDLE s_lcdStaticActionValue;
	HANDLE s_lcdStaticBankText;
	HANDLE s_lcdStaticBankValue;
	HANDLE s_lcdStaticCashText;
	HANDLE s_lcdStaticCashValue;
	HANDLE s_lcdStaticMailText;
	HANDLE s_lcdStaticMailValue;
	HANDLE s_lcdStaticGcwPointsText;
	HANDLE s_lcdStaticGcwPointsValue;
	HANDLE s_lcdStaticGcwKillsText;
	HANDLE s_lcdStaticGcwKillsValue;
	HANDLE s_lcdStaticGcwTimeValue;
	HANDLE s_lcdStaticPvpFactionText;
	HANDLE s_lcdStaticPvpFactionValue;
	HANDLE s_lcdStaticPvpStatusText;
	HANDLE s_lcdStaticPvpStatusValue;

	static std::string s_result;

	std::string::const_iterator skipPotentialTimestamp(std::string::const_iterator i, std::string::const_iterator end)
	{
		std::string::const_iterator result = i;
		if(CuiChatManager::getChatBoxTimestamp ())
		{
			//Make sure to skip the line feed if there is one
			if((result != end) && ((*result) == 10)) ++result;

			//Format of a timestamp is XX:XX:XX.  If we don't
			//find the full timestamp, abort and return the original iterator.
			if((result != end) && (isdigit(*result))) ++result; else return i;
			if((result != end) && (isdigit(*result))) ++result; else return i;
			if((result != end) && ((*result) == ':')) ++result; else return i;
			if((result != end) && (isdigit(*result))) ++result; else return i;
			if((result != end) && (isdigit(*result))) ++result; else return i;
			if((result != end) && ((*result) == ':')) ++result; else return i;
			if((result != end) && (isdigit(*result))) ++result; else return i;
			if((result != end) && (isdigit(*result))) ++result; else return i;
		}
		return result;
	}

	std::string const & stripTextForLcd(std::string const &in)
	{
		s_result.clear();
		bool ignoreNextEscape = false;
		int lineCount = 0;
		std::string::const_iterator i;
		for (i = in.begin(); i != in.end(); ++i)
		{
			if (*i == '\n')
			{
				lineCount++;
				s_result.append(&(*i), 1);
				i = skipPotentialTimestamp(i,in.end());
			}
			else if(*i == '\r')   //Remove line feeds
				ignoreNextEscape = false;
			else if (*i == '\\')
				ignoreNextEscape = true;
			else if (!ignoreNextEscape)
				s_result.append(&(*i), 1);
			else if (*i == '#')
			{
				ignoreNextEscape = false;
				++i;
				if (i == in.end())
					return s_result;

				Unicode::unicode_char_t const first_char = *i;

				switch (first_char)
				{
					// Color escape sequence: \#. set to default
				case '.':
					break;

					// Color escape sequence: \#p<paletteentry>
				case 'p':
					for (++i;i != in.end(); ++i)
					{
						const Unicode::unicode_char_t c = *i;
						if (c == ' ')
							break;
					}
					break;

					// Color escape sequence: \#XXXXXX
				default:
					{
						int	ic;
						for( ic = 0; (ic < 5) && (i != in.end()); ++ic )
						{
							++i;			
							if (i == in.end()) return s_result;
						}
					}
					break;
				}
			}
			else if ((*i == '@') || (*i == '>') || (*i == '%') || (*i == '^'))
			{
				ignoreNextEscape = false;
				++i; if (i == in.end()) return s_result;
				++i; if (i == in.end()) return s_result;
				++i; if (i == in.end()) return s_result;
			}
			else
			{
				ignoreNextEscape = false;
				s_result.append(&(*i), 1);
			}
		}
		int cutThisManyLines = lineCount - 3;
		int charCount = 0;
		for (i = s_result.begin(); (cutThisManyLines > 0) && (i != s_result.end()); ++i)
		{
			charCount++;
			if (*i == '\n')
				cutThisManyLines--;
		}
		s_result.erase(0, charCount);
		return s_result;
	}

	void getInventoryStats(int &currentVolume, int &totalVolume)
	{
		const CreatureObject * const player = Game::getPlayerCreature ();
		if (player)
		{
			const ClientObject * const inventory = player->getInventoryObject();
			if (inventory)
			{
				const VolumeContainer* const volume = ContainerInterface::getVolumeContainer(*inventory);
				if (volume)
				{
					totalVolume = volume->getTotalVolume();
					currentVolume = volume->getCurrentVolume();
				}
			}
		}
	}
#endif
}

using namespace SwgCuiG15LcdNamespace;

// ======================================================================

void SwgCuiG15Lcd::updateLcd()
{
#ifdef USE_LCD
	if (s_lcd)
	{
		if (s_lcd->ButtonTriggered(LG_BUTTON_1) || s_lcd->ButtonTriggered(LG_BUTTON_2) 
			|| s_lcd->ButtonTriggered(LG_BUTTON_3) || s_lcd->ButtonTriggered(LG_BUTTON_4))
			s_lcdDisplaying = (s_lcdDisplaying == 0) ? 1 : 0;
		if (s_lcdDisplaying == 0)
		{
			s_lcd->SetVisible(s_lcdStatic1Text, TRUE);
			s_lcd->SetVisible(s_lcdStaticHpText, FALSE);
			s_lcd->SetVisible(s_lcdStaticHpValue, FALSE);
			s_lcd->SetVisible(s_lcdStaticActionText, FALSE);
			s_lcd->SetVisible(s_lcdStaticActionValue, FALSE);
			s_lcd->SetVisible(s_lcdStaticBankText, FALSE);
			s_lcd->SetVisible(s_lcdStaticBankValue, FALSE);
			s_lcd->SetVisible(s_lcdStaticCashText, FALSE);
			s_lcd->SetVisible(s_lcdStaticCashValue, FALSE);
			s_lcd->SetVisible(s_lcdStaticMailText, FALSE);
			s_lcd->SetVisible(s_lcdStaticMailValue, FALSE);
			s_lcd->SetVisible(s_lcdStaticPvpFactionText, FALSE);
			s_lcd->SetVisible(s_lcdStaticPvpFactionValue, FALSE);
			s_lcd->SetVisible(s_lcdStaticPvpStatusText, FALSE);
			s_lcd->SetVisible(s_lcdStaticPvpStatusValue, FALSE);
			s_lcd->SetVisible(s_lcdStaticGcwPointsText, FALSE);
			s_lcd->SetVisible(s_lcdStaticGcwPointsValue, FALSE);
			s_lcd->SetVisible(s_lcdStaticGcwKillsText, FALSE);
			s_lcd->SetVisible(s_lcdStaticGcwKillsValue, FALSE);
			s_lcd->SetVisible(s_lcdStaticGcwTimeValue, FALSE);
			
			SwgCuiHud * const hud = SwgCuiHudFactory::findMediatorForCurrentHud();
			if (hud)
			{
				SwgCuiChatWindow * const chatWindow = NON_NULL(hud->getChatWindow  ());					
				if (chatWindow)
				{
					if(chatWindow && chatWindow->getLcdTab())
					{		
						s_lcd->SetText(s_lcdStatic1Text, stripTextForLcd(Unicode::wideToNarrow(chatWindow->getLcdTab()->getText())).c_str());
						s_lcd->Update();
					}
				}
			}
		}
		else if (s_lcdDisplaying == 1)
		{
			s_lcd->SetVisible(s_lcdStatic1Text, FALSE);
			s_lcd->SetVisible(s_lcdStaticHpText, TRUE);
			s_lcd->SetVisible(s_lcdStaticHpValue, TRUE);
			s_lcd->SetVisible(s_lcdStaticActionText, TRUE);
			s_lcd->SetVisible(s_lcdStaticActionValue, TRUE);
			s_lcd->SetVisible(s_lcdStaticBankText, TRUE);
			s_lcd->SetVisible(s_lcdStaticBankValue, TRUE);
			s_lcd->SetVisible(s_lcdStaticCashText, TRUE);
			s_lcd->SetVisible(s_lcdStaticCashValue, TRUE);
			s_lcd->SetVisible(s_lcdStaticMailText, TRUE);
			s_lcd->SetVisible(s_lcdStaticMailValue, TRUE);
			s_lcd->SetVisible(s_lcdStaticPvpFactionText, TRUE);
			s_lcd->SetVisible(s_lcdStaticPvpFactionValue, TRUE);
			s_lcd->SetVisible(s_lcdStaticPvpStatusText, TRUE);
			s_lcd->SetVisible(s_lcdStaticPvpStatusValue, TRUE);
			s_lcd->SetVisible(s_lcdStaticGcwPointsText, TRUE);
			s_lcd->SetVisible(s_lcdStaticGcwPointsValue, TRUE);
			s_lcd->SetVisible(s_lcdStaticGcwKillsText, TRUE);
			s_lcd->SetVisible(s_lcdStaticGcwKillsValue, TRUE);
			s_lcd->SetVisible(s_lcdStaticGcwTimeValue, TRUE);

			CreatureObject const * const player = Game::getPlayerCreature();
			char tmp[512];
			int cash, bank;
			PlayerMoneyManagerClient::getPlayerMoney(cash, bank);
			bool const hasNewMail = CuiPersistentMessageManager::hasNewMail ();
			PlayerObject const * const playerObj = Game::getPlayerObject();
			if (playerObj && player)
			{
				int32 const gcwPoints = playerObj->getCurrentGcwPoints();
				int32 const gcwKills  = playerObj->getCurrentPvpKills();
				time_t time = playerObj->getGcwRatingActualCalcTime();
				int timeUntil = static_cast<int>(time - ::time(NULL));
				uint32 const pvpFaction = player->getPvpFaction();
				uint32 const pvpStatus = player->getPvpType();

				sprintf(tmp, "%5d", player->getAttribute(Attributes::Health));
				s_lcd->SetText(s_lcdStaticHpValue, tmp);

				sprintf(tmp, "%5d", player->getAttribute(Attributes::Action));
				s_lcd->SetText(s_lcdStaticActionValue, tmp);
				
				sprintf(tmp, "%5d", bank + cash);
				s_lcd->SetText(s_lcdStaticBankValue, tmp);
				
				int currentVolume, totalVolume;
				getInventoryStats(currentVolume, totalVolume);
				sprintf(tmp, "%2d/%2d", currentVolume, totalVolume);
				s_lcd->SetText(s_lcdStaticCashValue, tmp);				
				
				sprintf(tmp, "%s", hasNewMail ? "Yes" : "No");
				s_lcd->SetText(s_lcdStaticMailValue, tmp);
				
				sprintf(tmp, "%5d", gcwPoints);
				s_lcd->SetText(s_lcdStaticGcwPointsValue, tmp);
				
				sprintf(tmp, "%5d", gcwKills);
				s_lcd->SetText(s_lcdStaticGcwKillsValue, tmp);
				
				float hoursLeft = ((timeUntil + 0.0f) / 3600);
				if (hoursLeft <= 0.0f)
					sprintf(tmp, "0h");
				else if (hoursLeft >= 100.0f)
					sprintf(tmp, ">99h");
				else
					sprintf(tmp, "%2.1fh", hoursLeft);
				s_lcd->SetText(s_lcdStaticGcwTimeValue, tmp);
				
				sprintf(tmp, "%s", PvpData::isRebelFactionId(pvpFaction) ? "Rebel" : (PvpData::isImperialFactionId(pvpFaction) ? "Imperial" : "Neutral"));
				s_lcd->SetText(s_lcdStaticPvpFactionValue, tmp);
				
				sprintf(tmp, "%s", (pvpStatus == 0) ? "On Leave" : ((pvpStatus == 1) ? "Combatant" : "Spec for"));
				s_lcd->SetText(s_lcdStaticPvpStatusValue, tmp);
			}
			s_lcd->Update();
		}
	}
#endif
}


// ----------------------------------------------------------------------

void SwgCuiG15Lcd::initializeLcd()
{
#ifdef USE_LCD
	if (ConfigClientUserInterface::getDisableG15Lcd())
	{
		s_lcd = 0;
		return;
	}
	if (s_lcd)
		return;
	HRESULT hRes = S_OK;

	// Create instance of EZLcd.
	s_lcd = new CEzLcd;

	// Have it initialize itself
	hRes = s_lcd->InitYourself(_T("Star Wars Galaxies"));

	if (hRes != S_OK)
	{
		// Something went wrong, when connecting to the LCD Manager software.  We need to get out now.
		remove();
		return;
	}

	HANDLE text1Static = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 2000, 4);
	s_lcd->SetOrigin(text1Static, 0, 0);
	s_lcdStatic1Text = text1Static;

	s_lcdStaticTitle = s_lcd->AddText(LG_STATIC_TEXT, LG_BIG, DT_LEFT, 2000, 1);
	s_lcd->SetText(s_lcdStaticTitle, _T("Star Wars Galaxies"));
	s_lcd->SetOrigin(s_lcdStaticTitle, 3, 13);
	s_lcd->SetVisible(s_lcdStaticTitle, TRUE);

	s_lcd->Update();

	s_lcd->SetVisible(s_lcdStaticTitle, FALSE);

	s_lcdStaticHpText = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 30, 1);
	s_lcd->SetText(s_lcdStaticHpText, _T("HP"));
	s_lcd->SetOrigin(s_lcdStaticHpText, 0, 0);
	s_lcdStaticHpValue = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, 2, 30, 1);
	s_lcd->SetOrigin(s_lcdStaticHpValue, 30, 0);
	s_lcdStaticActionText = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 30, 1);
	s_lcd->SetText(s_lcdStaticActionText, _T("Action"));
	s_lcd->SetOrigin(s_lcdStaticActionText, 63, 0);
	s_lcdStaticActionValue = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, 2, 30, 1);
	s_lcd->SetOrigin(s_lcdStaticActionValue, 90, 0);
	s_lcdStaticBankText = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 30, 1);
	s_lcd->SetText(s_lcdStaticBankText, _T("Credits"));
	s_lcd->SetOrigin(s_lcdStaticBankText, 0, 10);
	s_lcdStaticBankValue = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, 2, 45, 1);
	s_lcd->SetOrigin(s_lcdStaticBankValue, 30, 10);
	s_lcdStaticCashText = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 20, 1);
	s_lcd->SetText(s_lcdStaticCashText, _T("INV"));
	s_lcd->SetOrigin(s_lcdStaticCashText, 78, 10);
	s_lcdStaticCashValue = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 35, 1);
	s_lcd->SetOrigin(s_lcdStaticCashValue,102, 10);
	s_lcdStaticMailText = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 20, 1);
	s_lcd->SetText(s_lcdStaticMailText, _T("Mail"));
	s_lcd->SetOrigin(s_lcdStaticMailText, 123, 0);
	s_lcdStaticMailValue = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 15, 1);
	s_lcd->SetOrigin(s_lcdStaticMailValue, 143, 0);
	s_lcdStaticGcwPointsText = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 30, 1);
	s_lcd->SetText(s_lcdStaticGcwPointsText, _T("GCW"));
	s_lcd->SetOrigin(s_lcdStaticGcwPointsText, 0, 20);
	s_lcdStaticGcwPointsValue = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, 2, 30, 1);
	s_lcd->SetOrigin(s_lcdStaticGcwPointsValue, 30, 20);
	s_lcdStaticGcwKillsText = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 30, 1);
	s_lcd->SetOrigin(s_lcdStaticGcwKillsText, 63, 20);
	s_lcd->SetText(s_lcdStaticGcwKillsText, _T("Kills"));
	s_lcdStaticGcwKillsValue = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, 2, 30, 1);
	s_lcd->SetOrigin(s_lcdStaticGcwKillsValue, 90, 20);
	s_lcdStaticPvpFactionValue = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 40, 1);
	s_lcd->SetOrigin(s_lcdStaticPvpFactionValue, 00, 30);
	s_lcdStaticPvpStatusValue = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, 2, 50, 1);
	s_lcd->SetOrigin(s_lcdStaticPvpStatusValue, 43, 30);
	s_lcdStaticPvpStatusText = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 35, 1);
	s_lcd->SetText(s_lcdStaticPvpStatusText, _T("Update"));
	s_lcd->SetOrigin(s_lcdStaticPvpStatusText, 98, 30);
	s_lcdStaticGcwTimeValue = s_lcd->AddText(LG_STATIC_TEXT, LG_SMALL, DT_LEFT, 50, 1);
	s_lcd->SetOrigin(s_lcdStaticGcwTimeValue, 138, 30);
	s_lcd->SetAsForeground(true);
#endif
}

// ----------------------------------------------------------------------

void SwgCuiG15Lcd::remove()
{
#ifdef USE_LCD
	if (s_lcd)
		delete s_lcd;

	s_lcd = 0;
#endif
}

// ======================================================================

