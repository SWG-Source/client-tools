//======================================================================
//
// SwgCuiActions.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiActions_H
#define INCLUDED_SwgCuiActions_H

//======================================================================

#define MAKE_ACTION(a) const std::string a = #a

namespace SwgCuiActions
{
	MAKE_ACTION(characterSheet);
	MAKE_ACTION(chatCollapse);
	MAKE_ACTION(chatIconify);
	MAKE_ACTION(chatMaximize);
	MAKE_ACTION(chatTab);
	MAKE_ACTION(chatTabEdit);
	MAKE_ACTION(chatTabKill);
	MAKE_ACTION(chatTabNew);
	MAKE_ACTION(chatTabNext);
	MAKE_ACTION(chatTabPrev);
	MAKE_ACTION(clientMood);
	MAKE_ACTION(clientSocial);
	MAKE_ACTION(combatQueueCollapse);
	MAKE_ACTION(combatQueueExpand);
	MAKE_ACTION(dataStorage);
	MAKE_ACTION(debugPrint);
	MAKE_ACTION(inventoryClose);
	MAKE_ACTION(inventoryIconify);
	MAKE_ACTION(inventoryMaximize);
	MAKE_ACTION(mfdIconify);
	MAKE_ACTION(mfdMaximize);
	MAKE_ACTION(mfdNext);
	MAKE_ACTION(mfdPrev);
	MAKE_ACTION(persistentMessageBrowser);
	MAKE_ACTION(persistentMessageComposer);
	MAKE_ACTION(radarIconify);
	MAKE_ACTION(radarMaximize);
	MAKE_ACTION(radarRangeDecrease);
	MAKE_ACTION(radarRangeIncrease);
	MAKE_ACTION(radarUpdate);
	MAKE_ACTION(radarConMode);
	MAKE_ACTION(radarAlwaysShowRange);
	MAKE_ACTION(skillCommands);
	MAKE_ACTION(skills);
	MAKE_ACTION(startChat);
	MAKE_ACTION(survey);
	MAKE_ACTION(permissionList);
	MAKE_ACTION(targetsClearAll);
	MAKE_ACTION(targetsClearSelected);
	MAKE_ACTION(targetsIconify);
	MAKE_ACTION(targetsPeaceAttackToggle);
	MAKE_ACTION(toggleChatBubblesMinimized);
	MAKE_ACTION(toggleHud);
	MAKE_ACTION(workspaceFrameClose);
	MAKE_ACTION(workspaceFrameIconify);
	MAKE_ACTION(workspaceFrameMaximize);
	MAKE_ACTION(workspaceFrameNext);
	MAKE_ACTION(workspaceFramePrev);
	MAKE_ACTION(harvester);
	MAKE_ACTION(editBiography);
	MAKE_ACTION(questAcceptance);
	MAKE_ACTION(toggleButtonBar);
	MAKE_ACTION(toggleVoiceFlyBar);
	MAKE_ACTION(toggleVoiceActiveSpeakers);
}

#undef MAKE_ACTION

//======================================================================

#endif
