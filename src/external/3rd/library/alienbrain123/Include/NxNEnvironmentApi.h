#ifndef INC_NXN_ENVIRONMENTAPI_H
#define INC_NXN_ENVIRONMENTAPI_H

/* \class		CNxNEnvironmentApi NxNEnvironmentApi.h
 *
 *  \brief		All stuff that needs to be done before any
 *				namespace is instantiated goes into this class.
 *				Example: Query the user for a default working path
 *				and set a drive mapping to it.
 *
 *  \author		Andreas Kahler [ak]
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ak]-14-Sep-2000 file created.
 *		[ak]-22-May-2001 
 *	\endmod
 */

class NXNINTEGRATORSDK_API CNxNEnvironmentApi
{
	public:

		// Recreate drive mapping, run package installer etc.
        // If the user has not run alienbrain/medializer before, query some user settings
		static bool SetupUserEnvironment();

		//	Show the settings dialog only, do nothing else (like package installer)
		static bool DisplaySettings();

        // Skin helper methods
        static CNxNString GetSkinName();
        static CNxNString GetSkinPath();
        static COLORREF   GetSkinBackgroundColor();
        static COLORREF   GetSkinReadOnlyBackgroundColor();

    private:
        
        //	Show the settings dialog (if needed) and apply the settings
        static bool ApplyUserSettings(bool bForceShowDialog);

        // create a shortcut in the startup group
        static HRESULT CreateUserStartMenuShortcut(const CNxNString& sShortcutToFile,
                                                   const CNxNString& sArguments, 
                                                   const CNxNString& sShortcutName);

        // delete a shortcut in the startup group
        static HRESULT DeleteUserStartMenuShortcut(const CNxNString& sShortcutName);

        // internal helper function
        static HRESULT CreateOrDeleteUserStartMenuShortcut(bool bCreate,
                                                           const CNxNString& sShortcutToFile,
                                                           const CNxNString& sArguments, 
                                                           const CNxNString& sShortcutName);

        // run "subst"
        static int MapDrive(const char* pMapDrive, const char* pParam);

        // run the package installer
        static bool RunPackageInstaller();
		
		// run the Online Updater
        static bool RunOnlineUpdate();

};

#endif // INC_NXN_ENVIRONMENTAPI_H
