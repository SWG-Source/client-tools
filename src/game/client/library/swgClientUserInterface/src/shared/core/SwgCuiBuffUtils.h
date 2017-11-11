//=============================================================================
//
// SwgCuiBuffUtils.h
// copyright(c) 2005 Sony Online Entertainment
//
//=============================================================================

#ifndef INCLUDED_SwgCuiBuffUtils_H
#define INCLUDED_SwgCuiBuffUtils_H

class Buff;
class CreatureObject;
class UIEffector;
class UIImage;
class UIPage;
class UIVolumePage;
class UIWidget;


//-----------------------------------------------------------------------------

class SwgCuiBuffUtils
{
public:

	enum UpdateBuffsReturnType
	{
		UBRT_none = 0,
		UBRT_hasBuffs = 0x01,
		UBRT_hasDebufs = 0x02,
	};

	static UIImage * addBuffIcon(const Buff & buff, UIVolumePage & bufPage, const UIImage & sampleStateIcon, bool const isPlayerHamBar = false, bool const isGod = false);
	static UIPage * addBuffIcon(const Buff & buff, UIVolumePage & buffPage, const UIPage & sampleIconPage, bool const isPlayerHamBar = false, bool const isGod = false);
	static void clearBuffIcons(UIVolumePage & bufPage);

	static void setBuffDurationWhirlygigOpacity(UIVolumePage & page, float opacity);

	// return true if visibility changes.
	static uint32 updateBuffs(const CreatureObject & creature, UIVolumePage & bufPage, UIVolumePage & debuffPage, const UIImage & sampleStateIcon, UIEffector * effectorBlink, UIPage * samepleIconPage);

	static uint32 getBuffCrc (UIWidget *buffWidget);
	static uint32 getSortValue(Buff const & buff, bool const isGroupWindow);
	static void addSortedBuff (UIVolumePage & buffPage, UIWidget  & sampleIconPage);

private:
	SwgCuiBuffUtils();
	~SwgCuiBuffUtils();
};


#endif	// INCLUDED_SwgCuiBuffUtils_H


//=============================================================================
