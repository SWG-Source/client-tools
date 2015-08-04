// ======================================================================
//
// Options.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_Options_H
#define INCLUDED_Options_H

// ======================================================================

class Options
{
public:

	static const TCHAR* getFileName ();

	static CString getInformationString();

	static void load (int langCode);
	static bool save ();

	//-- graphics
	static int  getScreenWidth ();
	static void setScreenWidth (int screenWidth);
	static int  getScreenHeight ();
	static void setScreenHeight (int screenHeight);
	static int  getFullScreenRefreshRate ();
	static void setFullScreenRefreshRate (int fullScreenRefreshRate);
	static bool getDisableHardwareMouseCursor ();
	static void setDisableHardwareMouseCursor (bool disableHardwareMouseCursor);
	static bool getDisableBumpMapping ();
	static void setDisableBumpMapping (bool disableBumpMapping);
	static int  getPixelShaderMajorVersion ();
	static int  getPixelShaderMinorVersion ();
	static void setPixelShaderVersion (int pixelShaderMajorVersion, int pixelShaderMinorVersion);
	static bool getDiscardHighestMipMapLevels ();
	static void setDiscardHighestMipMapLevels (bool discardHighestMipMapLevels);
	static bool getDiscardHighestNormalMipMapLevels ();
	static void setDiscardHighestNormalMipMapLevels (bool discardHighestNormalMipMapLevels);
	static bool getDisableMultipassRendering ();
	static void setDisableMultipassRendering (bool disableMultipassRendering);
	static bool getAllowTearing ();
	static void setAllowTearing (bool allowTearing);
	static bool getDisableFastMouseCursor ();
	static void setDisableFastMouseCursor (bool disableFastMouseCursor);
	static bool getUseSafeRenderer ();
	static void setUseSafeRenderer (bool useSafeRenderer);

	static bool getWindowed ();
	static void setWindowed (bool windowed);
	static bool getBorderlessWindow ();
	static void setBorderlessWindow (bool borderlessWindow);

	//-- sound
	static bool getDisableAudio ();
	static void setDisableAudio (bool disableAudio);
	static const TCHAR* getSoundProvider ();
	static void setSoundProvider (const TCHAR* soundProvider);

	//-- game 
	static bool getDisableCharacterLodManager ();
	static void setDisableCharacterLodManager (bool disableCharacterLodManager);
	static bool getSkipIntro ();
	static void setSkipIntro (bool skipIntro);
	static bool getUseJoysticks();
	static void setUseJoysticks(bool useJoystick);
	static bool getUseForceFeedback();
	static void setUseForceFeedback(bool useForceFeedback);
	static int  getUseJoystick();
	static void setUseJoystick(int joystick);
	static void setLocaleCode(CString const & localeCode);
	static const TCHAR* getLocaleCode();
	static void setFontLocaleCode(CString const & fontLocaleCode);
	static const TCHAR* getFontLocaleCode();

	//-- advanced
	static bool getDisableWorldPreloading ();
	static void setDisableWorldPreloading (bool disableWorldPreloading);
	static bool getSkipL0Characters ();
	static void setSkipL0Characters (bool skipL0Characters);
	static bool getSkipL0Meshes ();
	static void setSkipL0Meshes (bool skipL0Meshes);
	static bool getDisableTextureBaking ();
	static void setDisableTextureBaking (bool disableTextureBaking);
	static bool getDisableFileCaching ();
	static void setDisableFileCaching (bool disableFileCaching);
	static bool getDisableAsynchronousLoader ();
	static void setDisableAsynchronousLoader (bool disableAsynchronousLoader);

	//-- debug
	static bool getSendStationId ();
	static void setSendStationId (bool sendStationId);
	static bool getAllowCustomerContact ();
	static void setAllowCustomerContact (bool allowCustomerContact);

private:
	static void setOptionLocaleDefaults(int langCode);
};

// ======================================================================

#endif
