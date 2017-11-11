// ======================================================================
//
// Options.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// Example options.cfg
/*

	[ClientAudio]
		disableMiles=0
		soundProvider=""

	[ClientGame]
		allowCustomerContact=0
		anonymousCrashReports=1
		preloadWorldSnapshot=0
		skipIntro=1

	[ClientGraphics]
		borderlessWindow=0
		disableOptionTag=DOT3
		disableOptionTag=HIQL
		discardHighestMipMapLevels=0
		discardHighestNormalMipMapLevels=0
		screenHeight=768
		screenWidth=1024
		useSafeRenderer=1
		useHardwareMouseCursor=0
		windowed=0

	[ClientObject/DetailAppearanceTemplate]
		skipL0=1

	[ClientSkeletalAnimation]
		lodManagerEnable=0
		skipL0=1

	[ClientTerrain]
		dot3Terrain=1

	[ClientTextureRenderer]
		disableTextureBaking=0

	[ClientUserInterface]
		alwaysSetMouseCursor=0

	[Direct3d9]
		allowTearing=0
		maxVertexShaderVersion=-1
		maxPixelShaderVersion=-1
		fullscreenRefreshRate=0

	[ClientDirectInput]
		useJoysticks=1
		enableForceFeedback=1
		useJoystick=0

	[SharedFile]
		enableAsynchronousLoader=0

	[SharedUtility]
		cache=misc/cache_large.iff
		disableFileCaching=1

    [SharedGame]
		defaultLocale=en
		fontLocale=en

*/
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "Options.h"

#include "ClientMachine.h"
#include "SwgClientSetup.h"

#include <string>

// ======================================================================
// OptionsNamespace
// ======================================================================

namespace OptionsNamespace
{
	enum Section
	{
		S_unknown,
		S_clientAudio,
		S_clientDirectInput,
		S_clientGame,
		S_clientGraphics,
		S_clientObject,
		S_clientObjectDetailAppearanceTemplate,
		S_clientSkeletalAnimation,
		S_clientUserInterface,
		S_clientTerrain,
		S_clientTextureRenderer,
		S_direct3d9,
		S_sharedFile,
		S_sharedFoundation,
		S_sharedGame,
		S_sharedUtility
	};

	const TCHAR* const cms_fileName = _T("options.cfg");

	bool ms_disableAudio                 = false;
	std::wstring ms_soundProvider (_T("Windows Speaker Configuration"));
	bool ms_disableWorldPreloading       = false;
	bool ms_disableBumpMapping           = false;
	bool ms_disableHardwareMouseCursor   = false;
	bool ms_discardHighestMipMapLevels   = false;
	bool ms_discardHighestNormalMipMapLevels = false;
	bool ms_disableMultipassRendering    = false;
	bool ms_allowTearing                 = false;
	bool ms_disableFastMouseCursor       = false;
	bool ms_useSafeRenderer              = false;
	bool ms_skipL0Meshes                 = false;
	bool ms_skipL0Characters             = false;
	bool ms_disableFileCaching           = false;
	bool ms_disableAsynchronousLoader    = false;
	int  ms_screenWidth                  = 1024;
	int  ms_screenHeight                 = 768;
	int  ms_fullScreenRefreshRate        = 0;
	bool ms_windowed                     = false;
	bool ms_borderlessWindow             = false;
	bool ms_disableTextureBaking         = false;
	int  ms_pixelShaderMajorVersion      = -1;
	int  ms_pixelShaderMinorVersion      = -1;
	bool ms_disableCharacterLodManager   = false;
	bool ms_skipIntro                    = false;
	bool ms_allowCustomerContact         = false;
	bool ms_sendStationId                = false;
	bool ms_useJoysticks                 = true;
	bool ms_enableForceFeedback          = true;
	int  ms_useJoystick                  = 0;
	CString ms_localeCode                = _T("en");
	CString ms_fontLocaleCode            = _T("en");
	bool ms_localeManuallyChanged        = false;
}

using namespace OptionsNamespace;

// ======================================================================
// STATIC PUBLIC Options
// ======================================================================

const TCHAR* Options::getFileName ()
{
	return cms_fileName;
}


// ----------------------------------------------------------------------

CString Options::getInformationString()
{
	CString buffer;
	CString result;

	result = _T("User Info Version: 1\n");

	buffer.Format (_T("Audio: %s"), ms_disableAudio ? _T("disabled") : _T("enabled"));
	buffer += '\n';
	result += buffer;

	buffer.Format (_T("DOT3: %s"), ms_disableBumpMapping ? _T("disabled") : _T("enabled"));
	buffer += '\n';
	result += buffer;

	buffer.Format (_T("HW mouse cursor: %s"), ms_disableHardwareMouseCursor ? _T("disabled") : _T("enabled"));
	buffer += '\n';
	result += buffer;

	buffer.Format (_T("HIQL: %s"), ms_disableMultipassRendering ? _T("disabled") : _T("enabled"));
	buffer += '\n';
	result += buffer;

	buffer.Format (_T("Safe renderer: %s"), ms_useSafeRenderer ? _T("yes") : _T("no"));
	buffer += '\n';
	result += buffer;

	buffer.Format (_T("Screen width: %d"), ms_screenWidth);
	buffer += '\n';
	result += buffer;

	buffer.Format (_T("Screen height: %d"), ms_screenHeight);
	buffer += '\n';
	result += buffer;

	buffer.Format (_T("Windowed: %s"), ms_windowed ? _T("yes") : _T("no"));
	buffer += '\n';
	result += buffer;

	buffer.Format (_T("Borderless windowed: %s"), ms_borderlessWindow ? _T("yes") : _T("no"));
	buffer += '\n';
	result += buffer;

	buffer.Format (_T("Texture baking: %s"), ms_disableTextureBaking ? _T("disabled") : _T("enabled"));
	buffer += '\n';
	result += buffer;

	buffer.Format (_T("UseJoysticks: %s"), ms_useJoysticks ? _T("enabled") : _T("disabled"));
	buffer += '\n';
	result += buffer;

	if (ms_useJoysticks && ms_useJoystick < ClientMachine::getNumberOfJoysticks())
	{
		buffer.Format (_T("UseForceFeedback: %s"), ms_enableForceFeedback ? _T("enabled") : _T("disabled"));
		buffer += '\n';
		result += buffer;
	}

	return result;
}

// ----------------------------------------------------------------------

//If there is no config file this is used to set options based on the locale
void Options::setOptionLocaleDefaults(int langCode)
{
	if(langCode == SwgClientSetupNamespace::cms_languageCodeEnglish)
	{
		ms_localeCode                = _T("en");
		ms_fontLocaleCode            = _T("en");
	}
	else if (langCode == SwgClientSetupNamespace::cms_languageCodeJapanese)
	{
		ms_localeCode                = _T("ja");
		ms_fontLocaleCode            = _T("j5");
	}
}

// ----------------------------------------------------------------------

void Options::load (int langCode)
{
	ms_disableHardwareMouseCursor = _tcsicmp (_T("Windows 98"), ClientMachine::getOs ()) == 0 || _tcsicmp (_T("Windows Me"), ClientMachine::getOs ()) == 0;

	//-- open the config file
	CStdioFile infile;
	if (!infile.Open (getFileName (), CFile::modeRead | CFile::typeText))
	{
		setOptionLocaleDefaults(langCode);
		return;
	}

	Section section = S_unknown;

	//-- read each line...
	CString line;
	while (infile.ReadString (line))
	{
		//-- see if the line is empty
		line.TrimLeft ();
		line.TrimRight ();
		if (line.GetLength () == 0)
			continue;

		//-- see if the first character is a comment
		int index = line.Find (_T("#"));
		if (index == 0)
			continue;

		//find section
		index = line.Find (_T("["));
		if (index == 0)
		{
			section = S_unknown;

			const CString sectionName = line.Mid (1, line.GetLength () - 2);

			if (sectionName == _T("ClientAudio"))
				section = S_clientAudio;
			else
				if (sectionName == _T("ClientDirectInput"))
					section = S_clientDirectInput;
				else
					if (sectionName == _T("ClientGame"))
						section = S_clientGame;
					else
						if (sectionName == _T("ClientGraphics"))
							section = S_clientGraphics;
						else
							if (sectionName == _T("ClientObject"))
								section = S_clientObject;
							else
								if (sectionName == _T("ClientObject/DetailAppearanceTemplate"))
									section = S_clientObjectDetailAppearanceTemplate;
								else
									if (sectionName == _T("ClientSkeletalAnimation"))
										section = S_clientSkeletalAnimation;
									else
										if (sectionName == _T("ClientTerrain"))
											section = S_clientTerrain;
										else
											if (sectionName == _T("ClientTextureRenderer"))
												section = S_clientTextureRenderer;
											else
												if (sectionName == _T("ClientUserInterface"))
													section = S_clientUserInterface;
												else
													if (sectionName == _T("Direct3d9"))
														section = S_direct3d9;
													else
														if (sectionName == _T("SharedFile"))
															section = S_sharedFile;
														else
															if (sectionName == _T("SharedFoundation"))
																section = S_sharedFoundation;
															else
																if (sectionName == _T("SharedGame"))
																	section = S_sharedGame;
																else
																	if (sectionName == _T("SharedUtility"))
																		section = S_sharedUtility;

			continue;
		}
		else
		{
			//-- find the =
			index = line.Find (_T("="));
			if (index == -1)
				continue;

			//-- left half goes in key, right half goes in value
			const int     length = line.GetLength ();
			const CString left   = line.Left (index);
			CString       right  = line.Right (length - index - 1);

			switch (section)
			{
			case S_clientAudio:
				{
					if (left == _T("disable"))
						ms_disableAudio = (_ttoi (right) == 1);
					else
						if (left == _T("disableMiles"))
							ms_disableAudio = (_ttoi (right) == 1);
						else
							if (left == _T("soundProvider"))
							{
								right.Remove ('"');
								ms_soundProvider = right;
							}
				}
				break;
			
			case S_clientDirectInput:
				{
					if (left == _T("useJoysticks"))
						ms_useJoysticks = (_ttoi (right) == 1);
					if (left == _T("enableForceFeedback"))
						ms_enableForceFeedback = (_ttoi (right) == 1);
					if (left == _T("useJoystick"))
						ms_useJoystick = _ttoi (right);

					// validate incoming data
					if (ClientMachine::getNumberOfJoysticks == 0)
					{
						ms_useJoysticks = false;
						ms_enableForceFeedback = true;
						ms_useJoystick = 0;
					}
					if (ms_useJoystick >= ClientMachine::getNumberOfJoysticks())
						ms_useJoystick = 0;
				}

			case S_clientGame:
				{
					if (left == _T("preloadWorldSnapshot"))
						ms_disableWorldPreloading = (_ttoi (right) == 0);
					else
						if (left == _T("disableFileCaching"))
							ms_disableFileCaching = (_ttoi (right) == 1);
						else
							if (left == _T("skipIntro"))
								ms_skipIntro = (_ttoi (right) == 1);
							else
								if (left == _T("anonymousCrashReports"))
									ms_sendStationId = (_ttoi (right) == 0);
								else
									if (left == _T("allowCustomerContact"))
										ms_allowCustomerContact = (_ttoi (right) == 1);
				}
				break;

			case S_clientGraphics:
				{
					if (left == _T("disableOptionTag"))
					{
						if (right == _T("DOT3"))
							ms_disableBumpMapping = true;
						else
							if (right == _T("HIQL"))
								ms_disableMultipassRendering = true;
					}
					else
						if (left == _T("useHardwareMouseCursor"))
							ms_disableHardwareMouseCursor = (_ttoi (right) == 0);
						else
							if (left == _T("discardHighestMipMapLevels"))
								ms_discardHighestMipMapLevels = (_ttoi (right) == 1);
							else
								if (left == _T("discardHighestNormalMipMapLevels"))
									ms_discardHighestNormalMipMapLevels = (_ttoi (right) == 1);
								else
									if (left == _T("screenWidth"))
										ms_screenWidth = _ttoi (right);
									else
										if (left == _T("screenHeight"))
											ms_screenHeight = _ttoi (right);
										else
											if (left == _T("windowed"))
												ms_windowed = (_ttoi (right) == 1);
											else
												if (left == _T("borderlessWindow"))
													ms_borderlessWindow = (_ttoi (right) == 1);
												else
													if (left == _T("alwaysSetMouseCursor"))
														ms_disableFastMouseCursor = (_ttoi (right) == 0);
													else
														if (left == _T("useSafeRenderer"))
															ms_useSafeRenderer = (_ttoi (right) == 1);
				}
				break;

			case S_clientObjectDetailAppearanceTemplate:
				{
					if (left == _T("skipL0"))
						ms_skipL0Meshes = (_ttoi (right) == 1);
				}
				break;

			case S_clientSkeletalAnimation:
				{
					if (left == _T("skipL0"))
						ms_skipL0Characters = (_ttoi (right) == 1);
					else
						if (left == _T("lodManagerEnable"))
							ms_disableCharacterLodManager = (_ttoi (right) == 0);
				}
				break;

			case S_clientTerrain:
				{
					if (left == _T("dot3Terrain"))
						ms_disableBumpMapping = (_ttoi (right) == 0);
				}
				break;

			case S_clientTextureRenderer:
				{
					if (left == _T("disableTextureBaking"))
						ms_disableTextureBaking = (_ttoi (right) == 1);
				}
				break;

			case S_clientUserInterface:
				{
					if (left == _T("alwaysSetMouseCursor"))
						ms_disableFastMouseCursor = (_ttoi (right) == 1);
				}
				break;

			case S_direct3d9:
				{
					if (left == _T("maxPixelShaderVersion"))
					{
						if (right == _T("0"))
						{
							ms_pixelShaderMajorVersion = 0;
							ms_pixelShaderMinorVersion = 0;
						}
						else
							if (right == _T("0x0101"))
							{
								ms_pixelShaderMajorVersion = 1;
								ms_pixelShaderMinorVersion = 1;
							}
							else
								if (right == _T("0x0104"))
								{
									ms_pixelShaderMajorVersion = 1;
									ms_pixelShaderMinorVersion = 4;
								}
								else
									if (right == _T("0x0200"))
									{
										ms_pixelShaderMajorVersion = 2;
										ms_pixelShaderMinorVersion = 0;
									}
					}
					else
						if (left == _T("disableVertexAndPixelShaders"))
						{
							ms_pixelShaderMajorVersion = 0;
							ms_pixelShaderMinorVersion = 0;
						}
						else
							if (left == _T("allowTearing"))
								ms_allowTearing = (_ttoi (right) == 1);
							else
								if (left == _T("fullscreenRefreshRate"))
									ms_fullScreenRefreshRate = _ttoi (right);
				}
				break;

			case S_sharedFile:
				{
					if (left == _T("enableAsynchronousLoader"))
						ms_disableAsynchronousLoader = (_ttoi (right) == 0);
				}
				break;

			case S_sharedFoundation:
				{
					if (left == _T("screenWidth"))
						ms_screenWidth = _ttoi (right);
					else
						if (left == _T("screenHeight"))
							ms_screenHeight = _ttoi (right);
						else
							if (left == _T("windowed"))
								ms_windowed = (_ttoi (right) == 1);
				}
				break;

			case S_sharedGame:
				{
					if (left == _T("defaultLocale"))
					{
						ms_localeCode = right;

						if (ms_localeCode != _T("en") && ms_localeCode != _T("ja"))
							ms_localeCode = _T("en");

						ms_localeManuallyChanged = true;
					}
					else
						if (left == _T("fontLocale"))
						{
							ms_fontLocaleCode = right;

							if (ms_fontLocaleCode != _T("en") && ms_fontLocaleCode != _T("j5"))
								ms_fontLocaleCode = _T("en");
							
							ms_localeManuallyChanged = true;
						}
				}
				break;

			case S_sharedUtility:
				{
					if (left == _T("disableFileCaching"))
						ms_disableFileCaching = (_ttoi (right) == 1);
				}
				break;

			default:
			case S_unknown:
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------

bool Options::save ()
{
	CString buffer;

	CStdioFile outfile;
	if (!outfile.Open (getFileName (), CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		return false;

	outfile.WriteString (_T("# options.cfg - Please do not edit this auto-generated file.\n"));

	//-- ClientAudio
	outfile.WriteString (_T("\n[ClientAudio]\n"));

		if (ms_disableAudio)
			outfile.WriteString (_T("\tdisableMiles=1\n"));

		buffer.Format (_T("\tsoundProvider=\"%s\"\n"), ms_soundProvider.c_str ());
		outfile.WriteString (buffer);
	
	//-- ClientDirectInput
	if (ClientMachine::getNumberOfJoysticks() && (!ms_useJoysticks || (ClientMachine::getJoystickSupportsForceFeedback(ms_useJoystick) && !ms_enableForceFeedback) || ms_useJoystick != 0))
	{
		outfile.WriteString (_T("\n[ClientDirectInput]\n"));

		if (ms_useJoysticks == false)
			outfile.WriteString (_T("\tuseJoysticks=0\n"));

		if (ClientMachine::getJoystickSupportsForceFeedback(ms_useJoystick) && ms_enableForceFeedback == false)
			outfile.WriteString (_T("\tenableForceFeedback=0\n"));

		if (ms_useJoystick != 0)
		{
			buffer.Format (_T("\tuseJoystick=%i\n"), ms_useJoystick);
			outfile.WriteString (buffer);
		}
	}

	//-- ClientGame
	if (ms_disableWorldPreloading || ms_skipIntro || ms_allowCustomerContact || ms_sendStationId)
	{
		outfile.WriteString (_T("\n[ClientGame]\n"));

		if (ms_allowCustomerContact)
			outfile.WriteString (_T("\tallowCustomerContact=1\n"));

		if (ms_sendStationId)
			outfile.WriteString (_T("\tanonymousCrashReports=0\n"));
			
		if (ms_disableWorldPreloading)
			outfile.WriteString (_T("\tpreloadWorldSnapshot=0\n"));

		if (ms_skipIntro)
			outfile.WriteString (_T("\tskipIntro=1\n"));
	}

	//-- ClientGraphics
	outfile.WriteString (_T("\n[ClientGraphics]\n"));

		if (ms_borderlessWindow)
			outfile.WriteString (_T("\tborderlessWindow=1\n"));

		buffer.Format (_T("\tuseHardwareMouseCursor=%i\n"), ms_disableHardwareMouseCursor ? 0 : 1);
		outfile.WriteString (buffer);

		if (ms_disableBumpMapping)
			outfile.WriteString (_T("\tdisableOptionTag=DOT3\n"));

		if (ms_disableMultipassRendering)
			outfile.WriteString (_T("\tdisableOptionTag=HIQL\n"));

		if (ms_discardHighestMipMapLevels)
			outfile.WriteString (_T("\tdiscardHighestMipMapLevels=1\n"));

		if (ms_discardHighestNormalMipMapLevels)
			outfile.WriteString (_T("\tdiscardHighestNormalMipMapLevels=1\n"));

		buffer.Format (_T("\tscreenWidth=%i\n"), ms_screenWidth);
		outfile.WriteString (buffer);

		buffer.Format (_T("\tscreenHeight=%i\n"), ms_screenHeight);
		outfile.WriteString (buffer);

		if (ms_windowed)
			outfile.WriteString (_T("\twindowed=1\n"));

		if (ms_useSafeRenderer)
		{
			outfile.WriteString (_T("\tuseSafeRenderer=1\n"));
			outfile.WriteString (_T("\trasterMajor=5\n"));
		}
		else
		{
			outfile.WriteString (_T("\tuseSafeRenderer=0\n"));
			if (!ClientMachine::supportsPixelShaders () || ms_pixelShaderMajorVersion == 0)
				outfile.WriteString (_T("\trasterMajor=6\n"));
			else
				outfile.WriteString (_T("\trasterMajor=7\n"));
		}

	//-- ClientObject
	if (ms_skipL0Meshes)
		outfile.WriteString (_T("\n[ClientObject/DetailAppearanceTemplate]\n\tskipL0=1\n"));

	//-- ClientSkeletalAnimation
	if (ms_skipL0Characters || ms_disableCharacterLodManager)
	{
		outfile.WriteString (_T("\n[ClientSkeletalAnimation]\n"));

		if (ms_disableCharacterLodManager)
			outfile.WriteString (_T("\tlodManagerEnable=0\n"));

		if (ms_skipL0Characters)
			outfile.WriteString (_T("\tskipL0=1\n"));
	}

	//-- ClientTerrain
	if (ms_disableBumpMapping)
		outfile.WriteString (_T("\n[ClientTerrain]\n\tdot3Terrain=0\n"));

	//-- ClientTextureRenderer
	if (ms_disableTextureBaking)
		outfile.WriteString (_T("\n[ClientTextureRenderer]\n\tdisableTextureBaking=1\n"));

	//-- ClientUserInterface
	if (ms_disableFastMouseCursor)
		outfile.WriteString (_T("\n[ClientUserInterface]\n\talwaysSetMouseCursor=1\n"));

	//-- Direct3d9
	if (ms_allowTearing || ms_pixelShaderMajorVersion != -1  || ms_fullScreenRefreshRate > 0)
		outfile.WriteString (_T("\n[Direct3d9]\n"));
	
		if (ms_allowTearing)
			outfile.WriteString (_T("\tallowTearing=1\n"));

		if (ms_fullScreenRefreshRate > 0)
		{
			buffer.Format (_T("\tfullscreenRefreshRate=%i\n"), ms_fullScreenRefreshRate);
			outfile.WriteString (buffer);
		}

		if (ms_pixelShaderMajorVersion != -1)
		{
			if (ms_pixelShaderMajorVersion == 2)
			{
				outfile.WriteString (_T("\tmaxVertexShaderVersion=0x0200\n"));
				outfile.WriteString (_T("\tmaxPixelShaderVersion=0x0200\n"));
			}
			else
				if (ms_pixelShaderMajorVersion == 1 && ms_pixelShaderMinorVersion == 4)
				{
					outfile.WriteString (_T("\tmaxVertexShaderVersion=0x0101\n"));
					outfile.WriteString (_T("\tmaxPixelShaderVersion=0x0104\n"));
				}
				else
					if (ms_pixelShaderMajorVersion == 1 && ms_pixelShaderMinorVersion == 1)
					{
						outfile.WriteString (_T("\tmaxVertexShaderVersion=0x0101\n"));
						outfile.WriteString (_T("\tmaxPixelShaderVersion=0x0101\n"));
					}
					else
					{
						outfile.WriteString (_T("\tmaxVertexShaderVersion=0\n"));
						outfile.WriteString (_T("\tmaxPixelShaderVersion=0\n"));
					}
		}

	//-- SharedFile
	if (ms_disableAsynchronousLoader)
		outfile.WriteString (_T("\n[SharedFile]\n\tenableAsynchronousLoader=0\n"));

	//-- SharedUtility
	outfile.WriteString (_T("\n[SharedUtility]\n"));
	
		if (ms_disableFileCaching)
			outfile.WriteString (_T("\tdisableFileCaching=1\n"));

		if (ClientMachine::getPhysicalMemorySize () < 256)
			outfile.WriteString (_T("\tcache=misc/cache_small.iff\n"));
		else
			if (ClientMachine::getPhysicalMemorySize () < 512)
				outfile.WriteString (_T("\tcache=misc/cache_medium.iff\n"));
			else
				outfile.WriteString (_T("\tcache=misc/cache_large.iff\n"));

	//-- SharedGame
	if(ms_localeManuallyChanged)
	{	
		outfile.WriteString (_T("\n[SharedGame]\n"));

		outfile.WriteString (_T("\tdefaultLocale="));
		outfile.WriteString (ms_localeCode);
		outfile.WriteString (_T("\n"));
		outfile.WriteString (_T("\tfontLocale="));
		outfile.WriteString (ms_fontLocaleCode);
		outfile.WriteString (_T("\n"));
	}

	return true;
}

// ======================================================================

int Options::getScreenWidth ()
{
	return ms_screenWidth;
}

// ----------------------------------------------------------------------

void Options::setScreenWidth (const int screenWidth)
{
	ms_screenWidth = screenWidth;
}

// ----------------------------------------------------------------------

int Options::getScreenHeight ()
{
	return ms_screenHeight;
}

// ----------------------------------------------------------------------

void Options::setScreenHeight (const int screenHeight)
{
	ms_screenHeight = screenHeight;
}

// ----------------------------------------------------------------------

int Options::getFullScreenRefreshRate ()
{
	return ms_fullScreenRefreshRate;
}

// ----------------------------------------------------------------------

void Options::setFullScreenRefreshRate (int fullScreenRefreshRate)
{
	ms_fullScreenRefreshRate = fullScreenRefreshRate;
}

// ----------------------------------------------------------------------

bool Options::getDisableHardwareMouseCursor ()
{
	return ms_disableHardwareMouseCursor;
}

// ----------------------------------------------------------------------

void Options::setDisableHardwareMouseCursor (const bool disableHardwareMouseCursor)
{
	ms_disableHardwareMouseCursor = disableHardwareMouseCursor;
}

// ----------------------------------------------------------------------

bool Options::getDisableBumpMapping ()
{
	return ms_disableBumpMapping;
}

// ----------------------------------------------------------------------

void Options::setDisableBumpMapping (const bool disableBumpMapping)
{
	ms_disableBumpMapping = disableBumpMapping;
}

// ----------------------------------------------------------------------

int Options::getPixelShaderMajorVersion ()
{
	return ms_pixelShaderMajorVersion;
}

// ----------------------------------------------------------------------

int Options::getPixelShaderMinorVersion ()
{
	return ms_pixelShaderMinorVersion;
}

// ----------------------------------------------------------------------

void Options::setPixelShaderVersion (int pixelShaderMajorVersion, int pixelShaderMinorVersion)
{
	ms_pixelShaderMajorVersion = pixelShaderMajorVersion;
	ms_pixelShaderMinorVersion = pixelShaderMinorVersion;
}

// ----------------------------------------------------------------------

bool Options::getDiscardHighestMipMapLevels ()
{
	return ms_discardHighestMipMapLevels;
}

// ----------------------------------------------------------------------

void Options::setDiscardHighestMipMapLevels (const bool discardHighestMipMapLevels)
{
	ms_discardHighestMipMapLevels = discardHighestMipMapLevels;
}

// ----------------------------------------------------------------------

bool Options::getDiscardHighestNormalMipMapLevels ()
{
	return ms_discardHighestNormalMipMapLevels;
}

// ----------------------------------------------------------------------

void Options::setDiscardHighestNormalMipMapLevels (const bool discardHighestNormalMipMapLevels)
{
	ms_discardHighestNormalMipMapLevels = discardHighestNormalMipMapLevels;
}

// ----------------------------------------------------------------------

bool Options::getWindowed ()
{
	return ms_windowed;
}

// ----------------------------------------------------------------------

void Options::setWindowed (const bool windowed)
{
	ms_windowed = windowed;
}

// ----------------------------------------------------------------------

bool Options::getBorderlessWindow ()
{
	return ms_borderlessWindow;
}

// ----------------------------------------------------------------------

void Options::setBorderlessWindow (const bool borderlessWindow)
{
	ms_borderlessWindow = borderlessWindow;
}

// ----------------------------------------------------------------------

bool Options::getDisableMultipassRendering ()
{
	return ms_disableMultipassRendering;
}

// ----------------------------------------------------------------------

void Options::setDisableMultipassRendering (const bool disableMultipassRendering)
{
	ms_disableMultipassRendering = disableMultipassRendering;
}

// ----------------------------------------------------------------------

bool Options::getAllowTearing ()
{
	return ms_allowTearing;
}

// ----------------------------------------------------------------------

void Options::setAllowTearing (const bool allowTearing)
{
	ms_allowTearing = allowTearing;
}

// ----------------------------------------------------------------------

bool Options::getDisableFastMouseCursor ()
{
	return ms_disableFastMouseCursor;
}

// ----------------------------------------------------------------------

void Options::setDisableFastMouseCursor (bool const disableFastMouseCursor)
{
	ms_disableFastMouseCursor = disableFastMouseCursor;
}

// ----------------------------------------------------------------------

bool Options::getUseSafeRenderer ()
{
	return ms_useSafeRenderer;
}

// ----------------------------------------------------------------------

void Options::setUseSafeRenderer (bool const useSafeRenderer)
{
	ms_useSafeRenderer = useSafeRenderer;
}

// ----------------------------------------------------------------------

bool Options::getDisableAudio ()
{
	return ms_disableAudio;
}

// ----------------------------------------------------------------------

void Options::setDisableAudio (const bool disableAudio)
{
	ms_disableAudio = disableAudio;
}

// ----------------------------------------------------------------------

const TCHAR* Options::getSoundProvider ()
{
	return ms_soundProvider.c_str ();
}

// ----------------------------------------------------------------------

void Options::setSoundProvider (const TCHAR* const soundProvider)
{
	ms_soundProvider = soundProvider;
}

// ----------------------------------------------------------------------

bool Options::getDisableWorldPreloading ()
{
	return ms_disableWorldPreloading;
}

// ----------------------------------------------------------------------

void Options::setDisableWorldPreloading (const bool disableWorldPreloading)
{
	ms_disableWorldPreloading = disableWorldPreloading;
}

// ----------------------------------------------------------------------

bool Options::getSkipL0Characters ()
{
	return ms_skipL0Characters;
}

// ----------------------------------------------------------------------

void Options::setSkipL0Characters (const bool skipL0Characters)
{
	ms_skipL0Characters = skipL0Characters;
}

// ----------------------------------------------------------------------

bool Options::getSkipL0Meshes ()
{
	return ms_skipL0Meshes;
}

// ----------------------------------------------------------------------

void Options::setSkipL0Meshes (const bool skipL0Meshes)
{
	ms_skipL0Meshes = skipL0Meshes;
}

// ----------------------------------------------------------------------

bool Options::getDisableFileCaching ()
{
	return ms_disableFileCaching;
}

// ----------------------------------------------------------------------

void Options::setDisableFileCaching (bool const disableFileCaching)
{
	ms_disableFileCaching = disableFileCaching;
}

// ----------------------------------------------------------------------

bool Options::getDisableAsynchronousLoader ()
{
	return ms_disableAsynchronousLoader;
}

// ----------------------------------------------------------------------

void Options::setDisableAsynchronousLoader (bool const disableAsynchronousLoader)
{
	ms_disableAsynchronousLoader = disableAsynchronousLoader;
}

// ----------------------------------------------------------------------

bool Options::getDisableTextureBaking ()
{
	return ms_disableTextureBaking;
}

// ----------------------------------------------------------------------

void Options::setDisableTextureBaking (bool const disableTextureBaking)
{
	ms_disableTextureBaking = disableTextureBaking;
}

// ----------------------------------------------------------------------

bool Options::getDisableCharacterLodManager ()
{
	return ms_disableCharacterLodManager;
}

// ----------------------------------------------------------------------

void Options::setDisableCharacterLodManager (bool const disableCharacterLodManager)
{
	ms_disableCharacterLodManager = disableCharacterLodManager;
}

// ----------------------------------------------------------------------

bool Options::getSkipIntro ()
{
	return ms_skipIntro;
}

// ----------------------------------------------------------------------

void Options::setSkipIntro (bool const skipIntro)
{
	ms_skipIntro = skipIntro;
}

// ----------------------------------------------------------------------

bool Options::getUseJoysticks()
{
	return ms_useJoysticks;
}

// ----------------------------------------------------------------------

void Options::setUseJoysticks(bool useJoystick)
{
	ms_useJoysticks = useJoystick;
}

// ----------------------------------------------------------------------

bool Options::getUseForceFeedback()
{
	return ms_enableForceFeedback;
}

// ----------------------------------------------------------------------

void Options::setUseForceFeedback(bool useForceFeedback)
{
	ms_enableForceFeedback = useForceFeedback;
}

// ----------------------------------------------------------------------

int Options::getUseJoystick()
{
	return ms_useJoystick;
}

// ----------------------------------------------------------------------

void Options::setUseJoystick(int joystick)
{
	ms_useJoystick = joystick;
}

// ----------------------------------------------------------------------

bool Options::getSendStationId ()
{
	return ms_sendStationId;
}

// ----------------------------------------------------------------------

void Options::setSendStationId (bool const sendStationId)
{
	ms_sendStationId = sendStationId;
}

// ----------------------------------------------------------------------

bool Options::getAllowCustomerContact ()
{
	return ms_allowCustomerContact;
}

// ----------------------------------------------------------------------

void Options::setAllowCustomerContact (bool const allowCustomerContact)
{
	ms_allowCustomerContact = allowCustomerContact;
}

// ----------------------------------------------------------------------

void Options::setLocaleCode(CString const & localeCode)
{
	ms_localeManuallyChanged = true;
	ms_localeCode = localeCode;
}

// ----------------------------------------------------------------------

const TCHAR * Options::getLocaleCode()
{
	return ms_localeCode;
}

// ----------------------------------------------------------------------

void Options::setFontLocaleCode(CString const & fontLocaleCode)
{	
	ms_localeManuallyChanged = true;
	ms_fontLocaleCode = fontLocaleCode;
}

// ----------------------------------------------------------------------

const TCHAR * Options::getFontLocaleCode()
{
	return ms_fontLocaleCode;
}

// ======================================================================

