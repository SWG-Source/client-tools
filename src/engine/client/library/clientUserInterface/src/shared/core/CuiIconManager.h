//======================================================================
//
// CuiIconManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiIconManager_H
#define INCLUDED_CuiIconManager_H

//======================================================================

class ClientObject;
class CuiDragInfo;
class CuiIconManagerCallback;
class CuiWidget3dObjectListViewer;
class Object;
class UIBaseObject;
class UIButtonStyle;
class UIImageStyle;
class UIRectangleStyle;
class UILowerString;
class UIPage;
struct UIColor;


//----------------------------------------------------------------------

class CuiIconManager
{
public:
	
	struct Properties
	{
		static const UILowerString ModifyBackgroundOpacity;
	};

	static void install ();
	static void remove  ();

	static std::string                   findIconPath       (const CuiDragInfo & info);
	static UIImageStyle *                findIconImageStyle (const CuiDragInfo & info );
	static UIImageStyle *                findIconImageStyle ( const std::string &inputPath , bool recurse );
	static UIImageStyle *                findIcon           (const std::string & styleName);

	static const std::string &           getSocialCommand   ();
	static const std::string &           getMoodCommand     ();
	static UIImageStyle *                getFallback        ();

	static std::string                   findButtonPath     (const CuiDragInfo & info);
	static UIButtonStyle *               findButtonStyle    (const CuiDragInfo & info);
	static UIButtonStyle *               getFallbackButton  ();

	static CuiWidget3dObjectListViewer * createObjectIcon       (ClientObject & obj, const char * const dragType);
	static CuiWidget3dObjectListViewer * createObjectIcon       (Object & obj, const char * const dragType);
	static void                          registerObjectIcon     (CuiWidget3dObjectListViewer & viewer, CuiIconManagerCallback * callback, bool minimalTooltip = true);
	static void                          unregisterObjectIcon   (CuiWidget3dObjectListViewer & viewer);
	static void                          unregisterIconsForPage (UIPage & page);

	static void                          update               (float deltaTimeSecs);

	static const std::string &           getFontPathLarge     ();
	static const std::string &           getFontPathSmall     ();

	static const UIColor &               getTextColorMagic    ();
	static const UIColor &               getTextColorNormal   ();

	static void setEntangleResistance(bool enabled);
	static bool getEntangleResistance();
	static void setColdResist(bool enabled);
	static bool getColdResist();
	static void setConductivity(bool enabled);
	static bool getConductivity();
	static void setDecayResist(bool enabled);
	static bool getDecayResist();
	static void setFlavor(bool enabled);
	static bool getFlavor();
	static void setHeatResist(bool enabled);
	static bool getHeatResist();
	static void setMalleability(bool enabled);
	static bool getMalleability();
	static void setPotentialEnergy(bool enabled);
	static bool getPotentialEnergy();
	static void setOverallQuality(bool enabled);
	static bool getOverallQuality();
	static void setShockResistance(bool enabled);
	static bool getShockResistance();
	static void setToughness(bool enabled);
	static bool getToughness();

	static void setActiveStyleMap(int index);
	static int getActiveStyleMapIndex();

private:
	static void                          updateTooltipForObject (ClientObject & obj, bool requestServerUpdate);
	static void                          updateColors         ();

	static const std::string             ms_fontPathLarge;
	static const std::string             ms_fontPathSmall;
	static UIColor                       ms_colorTextMagic;
	static UIColor                       ms_colorTextNormal;
	static int ms_activeStyleMap;
};

//----------------------------------------------------------------------

inline const std::string & CuiIconManager::getFontPathLarge     ()
{
	return ms_fontPathLarge;
}

//----------------------------------------------------------------------

inline const std::string & CuiIconManager::getFontPathSmall     ()
{
	return ms_fontPathSmall;
}

//----------------------------------------------------------------------

inline const UIColor & CuiIconManager::getTextColorMagic    ()
{
	return ms_colorTextMagic;
}

//----------------------------------------------------------------------

inline const UIColor & CuiIconManager::getTextColorNormal   ()
{
	return ms_colorTextNormal;
}

//======================================================================

#endif
