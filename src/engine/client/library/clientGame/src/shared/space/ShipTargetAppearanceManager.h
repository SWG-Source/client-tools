//======================================================================
//
// ShipTargetAppearanceManager.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipTargetAppearanceManager_H
#define INCLUDED_ShipTargetAppearanceManager_H

//======================================================================

class AppearanceTemplate;
class ClientEffectTemplate;
class CrcLowerString;

class ShipTargetAppearanceManager
{
public:

	static void install();
	static void remove();

	static AppearanceTemplate const * getAppearanceTemplate(uint32 chassisType, bool isEnemy = false);
	static AppearanceTemplate const * getAppearanceTemplateForComponent(uint32 chassisType, bool isEnemy = false);
	static ClientEffectTemplate const * getClientEffectTemplate(uint32 chassisType, bool isActive, bool isEnemy = false);
	static float getScale(uint32 chassisType);
	static float getSilhouetteDistance(uint32 chassisType);
	static bool useParentOrientation(uint32 chassisType);
	static CrcLowerString const & getHardpointName(uint32 chassisType);
	static AppearanceTemplate const * getAcquiredAppearanceTemplate(uint32 chassisType);
	static ClientEffectTemplate const * getAcquiringEffectTemplate(uint32 chassisType);

	//-- Audio effects.
	static void playActivateSound(uint32 chassisType);
	static void playDeactivateSoundName(uint32 chassisType);
	static void playTargetAcquiringSoundName(uint32 chassisType);
	static void playTargetAcquiredSoundName(uint32 chassisType);
};

//======================================================================

#endif
