//=============================================================================
//
// SwgCuiBuffUtils.cpp
// copyright(c) 2005 Sony Online Entertainment
//
//=============================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiBuffUtils.h"

#include "clientGame/ClientBuffManager.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/Buff.h"

#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIManager.h"
#include "UIPie.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UIVolumePage.h" 

#include <list>

//=============================================================================

namespace SwgCuiBuffUtilsNamespace
{
	static const UILowerString BUFF_ID_PROPERTY = UILowerString("BuffId");
	static const UILowerString BUFF_DESCRIPTION_PROPERTY = UILowerString("BuffDescription");
	static const UILowerString BUFF_TIMESTAMP_PROPERTY = UILowerString("BuffTimestamp");
	static const UILowerString BUFF_LENGTH_PROPERTY = UILowerString("BuffLength");
	static const UILowerString BUFF_SORT_PROPERTY = UILowerString("BuffSort");
	static const UILowerString ISGROUPWINDOW_PROPERTY = UILowerString("IsGroupWindow");

	static const int CREATURE_BUFF_BLINK_TIME = 10;

	static const char CHILD_ICON_NAME[] = "icon";
	static const char CHILD_PIE_NAME[] = "pie";

	enum SortValue
	{
		SV_Front,
		SV_Self,
		SV_Others,
		SV_Misc,
		SV_Invisible = 100
	};


};

using namespace SwgCuiBuffUtilsNamespace;


//=============================================================================
uint32 SwgCuiBuffUtils::getBuffCrc (UIWidget *buffWidget)
{
   int crc = 0;
   
   if (buffWidget && (buffWidget->IsA(TUIImage) || buffWidget->IsA(TUIPage)))
   {
	   buffWidget->GetPropertyInteger(BUFF_ID_PROPERTY, crc); 
   }
	
   return static_cast<uint32> (crc);
}

UIImage * SwgCuiBuffUtils::addBuffIcon(const Buff & buff, UIVolumePage & bufPage, const UIImage & sampleStateIcon, bool const isPlayerHamBar, bool const isGod)
{
	UIImageStyle * imageStyle = ClientBuffManager::getBuffIconStyle(buff.m_nameCrc);
	if (imageStyle == NULL)
		return NULL;
	
	UIImage * const image = dynamic_cast<UIImage *>(sampleStateIcon.DuplicateObject ());
	if (image == NULL)
		return NULL;

	image->Attach(0);
	image->SetStyle(imageStyle);

	image->Link();
	image->SetVisible(true);
	image->SetEnabled(false);
	image->SetEnabled(true);

	Unicode::String tooltipStr;
	ClientBuffManager::getBuffDescription(buff, tooltipStr);
	image->SetProperty(BUFF_DESCRIPTION_PROPERTY, tooltipStr);
	image->SetPropertyInteger(BUFF_ID_PROPERTY, buff.m_nameCrc);
	image->SetPropertyInteger(BUFF_TIMESTAMP_PROPERTY, buff.m_timestamp);
	image->SetColor(UIColor::white);
	
	if(isGod && !ClientBuffManager::getBuffIsGroupVisible(buff.m_nameCrc))
	{
		UIImageStyle * imageStyle = safe_cast<UIImageStyle *>(UIManager::gUIManager().GetRootPage()->GetObjectFromPath (UIString(Unicode::narrowToWide("/Styles.Icon.mood.careful")), TUIImageStyle));

		if (imageStyle)
		{
			image->SetStyle(imageStyle);
		}
	}

	bool isGroupWindow  = false;
	bufPage.GetPropertyBoolean(ISGROUPWINDOW_PROPERTY,isGroupWindow);
	
	if (isPlayerHamBar || isGroupWindow)
	{
		image->SetPropertyInteger(BUFF_SORT_PROPERTY, getSortValue(buff, isGroupWindow));
		addSortedBuff(bufPage, *image);
	}
	else
	{
		bufPage.AddChild(image);
	}

	if (isGod)
	{
		tooltipStr += Unicode::narrowToWide("\nCaster: " + buff.m_caster.getValueString());

		UINarrowString sortString; 
		UIUtils::FormatLong(sortString, getSortValue(buff, isGroupWindow));
		tooltipStr += Unicode::narrowToWide("\nSort Value:  " + sortString);
	}
	
	image->SetLocalTooltip(UIString(tooltipStr));

	return image;
}

UIPage * SwgCuiBuffUtils::addBuffIcon(const Buff & buff, UIVolumePage & buffPage, const UIPage & sampleIconPage, bool const isPlayerHamBar, bool const isGod)
{
	UIImageStyle * imageStyle = ClientBuffManager::getBuffIconStyle(buff.m_nameCrc);
	if (imageStyle == NULL)
		return NULL;

	UIBaseObject * tobj = sampleIconPage.DuplicateObject ();
	if ( tobj == NULL || !tobj->IsA(TUIPage))
		return NULL;
	UIPage * const iconPage =  static_cast<UIPage *>(tobj);

	iconPage->Attach(0);

#ifdef _DEBUG
	static uint32 count = 0;
	FormattedString<100> fs;
	std::string newName(fs.sprintf("made child %u", count));
	count++;
	iconPage->SetName(newName);
#endif

	tobj = iconPage->GetChild (CHILD_ICON_NAME);
	if (tobj && tobj->IsA (TUIImage))
	{
		UIImage * const image = static_cast<UIImage *>(tobj);
		image->SetStyle(imageStyle);
		image->SetColor(UIColor::white);
		image->SetVisible(true);
		
		if(isGod && !ClientBuffManager::getBuffIsGroupVisible(buff.m_nameCrc))
		{
			UIImageStyle * imageStyle = safe_cast<UIImageStyle *>(UIManager::gUIManager().GetRootPage()->GetObjectFromPath (UIString(Unicode::narrowToWide("/Styles.Icon.mood.careful")), TUIImageStyle));

			if (imageStyle)
			{
				image->SetStyle(imageStyle);
			}
		}				
	}	

	tobj = iconPage->GetChild (CHILD_PIE_NAME);
	if(tobj != NULL && tobj->IsA (TUIPie))
	{
		UIPie * const pie = static_cast<UIPie *>(tobj);
		pie->SetVisible(true);
		pie->SetValue(0.0f);
		pie->SetOpacity(CuiPreferences::getBuffIconWhirlygigOpacity());
	}

	bool isGroupWindow = false;
	buffPage.GetPropertyBoolean(ISGROUPWINDOW_PROPERTY, isGroupWindow);

	if (isPlayerHamBar || isGroupWindow)
	{
		iconPage->SetPropertyInteger(BUFF_SORT_PROPERTY, getSortValue(buff, isGroupWindow));
		addSortedBuff(buffPage, *iconPage);
	}
	else
	{
		buffPage.AddChild(iconPage);
	}

	//copy-paste from above
	iconPage->Link();
	iconPage->SetVisible(true);
	iconPage->SetEnabled(false);
	iconPage->SetEnabled(true);

	Unicode::String tooltipStr;
	ClientBuffManager::getBuffDescription(buff, tooltipStr);
	iconPage->SetProperty(BUFF_DESCRIPTION_PROPERTY, tooltipStr);
	iconPage->SetPropertyInteger(BUFF_ID_PROPERTY, buff.m_nameCrc);
	iconPage->SetPropertyInteger(BUFF_TIMESTAMP_PROPERTY, buff.m_timestamp);
	iconPage->SetPropertyInteger(BUFF_LENGTH_PROPERTY, static_cast<int>(buff.m_duration));

	if (isGod)
	{
		tooltipStr += Unicode::narrowToWide("\nCaster: " + buff.m_caster.getValueString());

		UINarrowString sortString; 
		UIUtils::FormatLong(sortString, getSortValue(buff, isGroupWindow));
		tooltipStr += Unicode::narrowToWide("\nSort Value:  " + sortString);
	}

	iconPage->SetLocalTooltip(UIString(tooltipStr));

	return iconPage;
}


//----------------------------------------------------------------------

void SwgCuiBuffUtils::clearBuffIcons(UIVolumePage & bufPage)
{
	UIBaseObject::UIObjectList shownBuffs;
	bufPage.GetChildren(shownBuffs);

	for (UIBaseObject::UIObjectList::iterator ibuff = shownBuffs.begin(); ibuff != shownBuffs.end(); ++ibuff)
	{
		UIBaseObject * object = *ibuff;
		bufPage.RemoveChild(object);
		object->Detach(0);
	}
}

//----------------------------------------------------------------------

void SwgCuiBuffUtils::setBuffDurationWhirlygigOpacity(UIVolumePage & page, float opacity)
{
	UIBaseObject::UIObjectList shownBuffs;
	page.GetChildren(shownBuffs);

	for (UIBaseObject::UIObjectList::iterator ibuff = shownBuffs.begin(); ibuff != shownBuffs.end(); ++ibuff)
	{
		UIBaseObject * object = *ibuff;
		if (object->IsA (TUIPage))
		{
			UIBaseObject* pieWidgit = object->GetChild (CHILD_PIE_NAME);
			if (pieWidgit && pieWidgit->IsA (TUIPie))
			{
				static_cast<UIPie*>(pieWidgit)->SetOpacity (opacity);
			}
		}
	}
}

//----------------------------------------------------------------------

uint32 SwgCuiBuffUtils::updateBuffs(const CreatureObject & creature, UIVolumePage & buffPage, UIVolumePage & debuffPage, const UIImage & sampleStateIcon, UIEffector * effectorBlink, UIPage * sampleIconPage)
{
	uint32 currentTime = creature.getPlayedTime();
	uint32 creatureTime = currentTime;
	uint32 serverTime = 0;
	 
	
	GroundScene const * const groundScene = dynamic_cast<GroundScene *>(Game::getScene());

	if (groundScene)
		serverTime = groundScene->getServerTime();

	bool isPlayerGod = false;
	bool isPlayerHamBar = false;
	
	PlayerObject const * const player = Game::getPlayerObject();
	
	if (player)
		isPlayerGod = player->isAdmin();
	
	CreatureObject const * const playerCreature = Game::getPlayerCreature();

	if (playerCreature)
		isPlayerHamBar = (playerCreature->getNetworkId() == creature.getNetworkId());
	
	std::map<uint32, Buff> buffs;
	creature.getBuffs(buffs);

	bool hasBuffs = !buffs.empty();

	uint32 returnValue = UBRT_none;

	if (hasBuffs)
	{
		// go through the buffs already showing, removing any that aren't in the buff map
		int crc;
		UIBaseObject::UIObjectList shownBuffs;
		buffPage.GetChildren(shownBuffs);
		debuffPage.GetChildren(shownBuffs);
		UIBaseObject::UIObjectList buffsToRemove;
		{
			for (UIBaseObject::UIObjectList::iterator i = shownBuffs.begin(); i != shownBuffs.end(); ++i)
			{
				UIBaseObject * object = *i;
				if (object->GetPropertyInteger(BUFF_ID_PROPERTY, crc))
				{
					std::map<uint32, Buff>::iterator found = buffs.find(crc);
					if (found == buffs.end())
					{
						// the buff is not on the creature's list
						buffsToRemove.push_back(object);
					}
					else if (!ClientBuffManager::getBuffIsGroupVisible((*found).second.m_nameCrc) && !isPlayerGod)
					{
						buffsToRemove.push_back(object);
					}
					else
					{
						// the buff is on the creature's list
						returnValue |= (ClientBuffManager::getBuffIsDebuff(crc) ? UBRT_hasDebufs : UBRT_hasBuffs);

						const Buff & buff = (*found).second;

						if (ClientBuffManager::getBuffIsCelestial(buff.m_nameCrc))
							currentTime = serverTime;
						else
							currentTime = creatureTime;
	
						int timeLeft = 0;
						if (buff.m_timestamp > currentTime)
							timeLeft = buff.m_timestamp - currentTime;
						// make the image blink if needed
						if (effectorBlink != NULL)
						{
							if ((timeLeft > 0) && (timeLeft <= CREATURE_BUFF_BLINK_TIME))
							{
								UIManager::gUIManager().ExecuteEffector(effectorBlink, object, false);
							}
						}						
						if (timeLeft > 0)
						{
							if (object->IsA (TUIPage))
							{
								UIPage * const iconPage = static_cast<UIPage *>(object);
								Unicode::String tooltipStr;
								iconPage->GetProperty(BUFF_DESCRIPTION_PROPERTY, tooltipStr);
								Unicode::String result;
								ClientBuffManager::addTimestampToBuffDescription(tooltipStr, timeLeft, result);
								
								if (isPlayerGod)
								{
									result += Unicode::narrowToWide("\nCaster: " + buff.m_caster.getValueString());
									
									int sortVal = -1;
									iconPage->GetPropertyInteger(BUFF_SORT_PROPERTY, sortVal);
									
									UINarrowString sortValue; 
									UIUtils::FormatLong(sortValue, sortVal);
									result += Unicode::narrowToWide("\nSort Value:  " + sortValue);
									
								}
								
								iconPage->SetLocalTooltip(result);

								int buffLength = 0;
								if (object->GetPropertyInteger (BUFF_LENGTH_PROPERTY, buffLength) && buffLength > 0)
								{
									UIBaseObject * pieChild = object->GetChild (CHILD_PIE_NAME);
									UIPie * pie = pieChild && pieChild->IsA (TUIPie) ? static_cast<UIPie *>(pieChild) : NULL;
									if (pie)
									{
										pie->SetValue (1.0f - std::min(1.0f,(static_cast<float>(timeLeft)/static_cast<float>(buffLength))));
									}
								}							
							}
							else if (object->IsA (TUIImage))
							{
								UIImage * const image = static_cast<UIImage *>(object);
								Unicode::String tooltipStr;
								image->GetProperty(BUFF_DESCRIPTION_PROPERTY, tooltipStr);
								Unicode::String result;
								ClientBuffManager::addTimestampToBuffDescription(tooltipStr, timeLeft, result);

								if (isPlayerGod)
								{
									result += Unicode::narrowToWide("\nCaster: " + buff.m_caster.getValueString());
									
									int sortVal = -1;
									UINarrowString sortValue; 
									UIUtils::FormatLong(sortValue, sortVal);
									result += Unicode::narrowToWide("\nSort Value:  " + sortValue);
								}

								image->SetLocalTooltip(result);
							}
						}
						
						if (object->IsA (TUIPage))
						{
							UIPage * const iconPage = static_cast<UIPage *>(object);

							UIText * buffStack = static_cast<UIText *>(iconPage->GetChild("textstack"));

							if (buffStack && ClientBuffManager::getBuffMaxStacks(buff.m_nameCrc) > 1 )
							{
								buffStack->SetVisible(true);
								buffStack->SetText(Unicode::narrowToWide(FormattedString<32>().sprintf("%d", buff.m_stackCount)));
							}
							else
							{
								buffStack->SetVisible(false);
							}
						}

						buffs.erase(found);
					}
				}
			}
		}

		//remove buffs that need removing
		{				
			for (UIBaseObject::UIObjectList::iterator i = buffsToRemove.begin(); i != buffsToRemove.end();++i)
			{
				UIBaseObject * object = *i;

				if (object->GetPropertyInteger(BUFF_ID_PROPERTY, crc))
				{
   					if (ClientBuffManager::getBuffIsDebuff(crc))
					{
						debuffPage.RemoveChild(object);
					}
					else
					{
						buffPage.RemoveChild(object);
					}
		   		}

				object->Detach(0);
			}
		}
							
		// add any new buffs to the display
		{
			for (std::map<uint32, Buff>::iterator i = buffs.begin(); i != buffs.end(); ++i)
			{
				const Buff & buff = (*i).second;

				if(ClientBuffManager::getBuffIsGroupVisible(buff.m_nameCrc) || isPlayerGod)
				{
					UIWidget * buffIcon = NULL;
					if(sampleIconPage != NULL)
					{
						if (ClientBuffManager::getBuffIsDebuff(buff.m_nameCrc))
						{
							buffIcon = addBuffIcon(buff, debuffPage, *sampleIconPage, isPlayerHamBar, isPlayerGod);
							returnValue |= UBRT_hasDebufs;
						}
						else
						{
							buffIcon =  addBuffIcon(buff, buffPage, *sampleIconPage, isPlayerHamBar, isPlayerGod);
							returnValue |= UBRT_hasBuffs;
						}
					}
					else
					{
						if (ClientBuffManager::getBuffIsDebuff(buff.m_nameCrc))
						{
							buffIcon = addBuffIcon(buff, debuffPage, sampleStateIcon, isPlayerHamBar, isPlayerGod);
							returnValue |= UBRT_hasDebufs;
						}
						else
						{
							buffIcon =  addBuffIcon(buff, buffPage, sampleStateIcon, isPlayerHamBar, isPlayerGod);
							returnValue |= UBRT_hasBuffs;
						}
					}

					if (buffIcon != NULL && effectorBlink != NULL)
					{
						// see if we need to blink the image
						int timeLeft = 0;
						if (buff.m_timestamp > currentTime)
							timeLeft = buff.m_timestamp - currentTime;
						if ((timeLeft > 0) && (timeLeft <= CREATURE_BUFF_BLINK_TIME))
						{
							UIManager::gUIManager().ExecuteEffector(effectorBlink, buffIcon, false);
						}
						if (timeLeft > 0)
						{
							Unicode::String tooltipStr;
							buffIcon->GetProperty(BUFF_DESCRIPTION_PROPERTY, tooltipStr);
							Unicode::String result;
							ClientBuffManager::addTimestampToBuffDescription(tooltipStr, timeLeft, result);
							buffIcon->SetLocalTooltip(result);
						}
					}
				}
			}
		}
	}
	else
	{
		if (buffPage.GetChildCount() > 0)
			buffPage.Clear();

		if (debuffPage.GetChildCount() > 0)
			debuffPage.Clear();
	}

	return returnValue;
}

//----------------------------------------------------------------------

void SwgCuiBuffUtils::addSortedBuff (UIVolumePage & buffPage, UIWidget & sampleIconPage)
{
	int sortValue = 0;

	sampleIconPage.GetPropertyInteger(BUFF_SORT_PROPERTY, sortValue);

	UIBaseObject::UIObjectList children;
	buffPage.GetChildren(children);

	UIBaseObject::UIObjectList::iterator iter = children.begin();

	bool inserted = false;

	for (; iter != children.end(); ++iter)
	{
		UIBaseObject * child = (*iter);
		int childSortValue = 0;
		child->GetPropertyInteger(BUFF_SORT_PROPERTY, childSortValue);

		if (sortValue == SV_Front || sortValue < childSortValue)
		{
			buffPage.InsertChildBefore(&sampleIconPage, child);
			inserted = true;
			break;
		}
	}

	if (!inserted)
		buffPage.AddChild(&sampleIconPage);

	children.clear();
}

//----------------------------------------------------------------------

uint32 SwgCuiBuffUtils::getSortValue(Buff const & buff, bool const isGroupWindow)
{
	if (!ClientBuffManager::getBuffIsGroupVisible(buff.m_nameCrc))
		return SV_Invisible;

	int const displayOrder = ClientBuffManager::getBuffDisplayOrder(buff.m_nameCrc);
	
	if (displayOrder > 0)
		return SV_Misc + displayOrder;
	
	uint32 sortValue = 0;

	bool isCastedByPlayer = false;
	CreatureObject const * const player = Game::getPlayerCreature();
	
	if (player)
		isCastedByPlayer = (buff.m_caster == player->getNetworkId());

	if (isGroupWindow)
	{
		if (ClientBuffManager::getBuffIsDebuff(buff.m_nameCrc))
		{
			if (ClientBuffManager::getBuffIsDispellable(buff.m_nameCrc))
				sortValue = SV_Front;
			else
				sortValue = SV_Self;
		}
		else if (isCastedByPlayer)
			sortValue = SV_Others;
		else
		   sortValue = SV_Misc;
	}
	else
	{
		if (ClientBuffManager::getBuffIsDebuff(buff.m_nameCrc))
		{
			if (ClientBuffManager::getBuffIsDispellable(buff.m_nameCrc))
				sortValue = SV_Front;
			else
				sortValue = SV_Self;
		}
		else
		{
			if (isCastedByPlayer)
				sortValue = SV_Self;
			else if (buff.m_caster.isValid())
				sortValue = SV_Others;
			else
				sortValue = SV_Misc;
		}
	}
	
	return sortValue;
}

//=============================================================================
