// \addtodoc
// MBCS review (14Feb01, Jens)

/*! \file       NxNConstants.h
 *
 *  \brief      Collection of all NxN Integrator SDK specific constants and commands.
 *
 *  \author     Axel Pfeuffer, Helmut Klinger
 *
 *  \version    1.00
 *
 *  \date       2000
 *
 */

#if !defined(INC_NXN_CONSTANTS_H)
#define INC_NXN_CONSTANTS_H

/*! \enum NxNConst_Flags_GetProp
 *      These flags affect the retrieval of a property via CNxNNode::GetProperty().
 */
/*! \var NxNConst_Flags_GetProp NXNCONST_GETPROP_DIRECT
 *      Get the value of a property directly from the given object.
 */
/*! \var NxNConst_Flags_GetProp NXNCONST_GETPROP_INHERIT
 *      Get the value of a property from the parent object if the property is not defined for the specified item.
 */
/*! \var NxNConst_Flags_GetProp NXNCONST_GETPROP_NO_MIME
 *      Do NOT get the value of a property from the mime type if the property is not defined for the specified item.
 */
/*  \var NxNConst_Flags_GetProp NXNCONST_GETPROP_NO_LINK
 *      Currently unsupported.
 */
/*! \var NxNConst_Flags_GetProp NXNCONST_GETPROP_UNCACHED
 *      Get the value of a property directly from the database object
 *      and avoids using the local cache.
 */
/*! \var NxNConst_Flags_GetProp NXNCONST_GETPROP_ALL
 *      Get all available properties.
 */
/*! \var NxNConst_Flags_GetProp NXNCONST_GETPROP_DONT_INHERIT
 *      Do not return an inherited value, even if it is the default for a property.
 */
/*  \var NxNConst_Flags_GetProp NXNCONST_GETPROP_MULTIPLE
 *      Get the value of a property from a multiple selection of objects.
 */
/*! \var NxNConst_Flags_GetProp NXNCONST_GETPROP_RESOLVE
 *      Resolve all macros specified with the property name.
 */
/*! \var NxNConst_Flags_GetProp NXNCONST_GETPROP_INHERIT_ONLY
 *      Get only values of inherited properties.
 */
/*! \var NxNConst_Flags_GetProp NXNCONST_GETPROP_FIRST
 *      Get first property of a property sequence.
 */
/*! \var NxNConst_Flags_GetProp NXNCONST_GETPROP_NEXT
 *      Get next property of a property sequence.
 */
/*! \var NxNConst_Flags_GetProp NXNCONST_GETPROP_USER
 *      Get the value of a property from the current user object.
 */
/*! \var NxNConst_Flags_GetProp NXNCONST_GETPROP_PER_OBJECT
 *      Get the value an object-specific property.
 */
/*! \var NxNConst_Flags_GetProp NXNCONST_GETPROP_HANDLE_TO_PATH
 *      Get the value of a property, interpret it as a database handle (list) and convert it to a path (list).
 */
/*! \var NxNConst_Flags_GetProp NXNCONST_GETPROP_INHERITED
 *      The returned property value was retrieved from a parent object.
 */
/*! \var NxNConst_Flags_GetProp NXNCONST_GETPROP_FROM_MIME
 *      The returned property value was retrieved from the mime type.
 */
/*! \var NxNConst_Flags_GetProp NXNCONST_GETPROP_FROM_LINK
 *      The returned property value was retrieved from a link.
 */
/*! \var NxNConst_Flags_GetProp NXNCONST_GETPROP_FROM_CACHE
 *      The returned property value was found in the client-side cache.
 */
enum NxNConst_Flags_GetProp
{   
        NXNCONST_GETPROP_DIRECT         = 0,
        NXNCONST_GETPROP_INHERIT        = 1,
        NXNCONST_GETPROP_NO_MIME        = 2,
        NXNCONST_GETPROP_NO_LINK        = 4,
        NXNCONST_GETPROP_UNCACHED       = 8,
        NXNCONST_GETPROP_ALL            = 16,
        NXNCONST_GETPROP_DONT_INHERIT   = 32,
        NXNCONST_GETPROP_MULTIPLE       = 64,
        NXNCONST_GETPROP_RESOLVE        = 128,
        NXNCONST_GETPROP_INHERIT_ONLY   = 256,
        NXNCONST_GETPROP_FIRST          = 0x200,
        NXNCONST_GETPROP_NEXT           = 0x400,
        NXNCONST_GETPROP_USER           = 0x800,
        NXNCONST_GETPROP_PER_OBJECT     = 0x1000,
        NXNCONST_GETPROP_HANDLE_TO_PATH = 0x2000,
        NXNCONST_GETPROP_RESERVED14     = 0x4000,
        NXNCONST_GETPROP_RESERVED15     = 0x8000,
        NXNCONST_GETPROP_INHERITED      = 0x10000,
        NXNCONST_GETPROP_FROM_MIME      = 0x20000,
        NXNCONST_GETPROP_FROM_LINK      = 0x40000,
        NXNCONST_GETPROP_FROM_CACHE     = 0x80000
};


/*! \enum NxNConst_Flags_SetProp
 *      These flags affect the setting of a property via CNxNNode::SetProperty().
 */
/*! \var NxNConst_Flags_SetProp NXNCONST_SETPROP_DIRECT 
 *      Set the value of a property directly for the specified object.
 */
/*  \var NxNConst_Flags_SetProp NXNCONST_SETPROP_MULTIPLE
 *      Set multiple properties (property lists).
 */
/*! \var NxNConst_Flags_SetProp NXNCONST_SETPROP_USER   
 *      Set the value of a property for the current user object.
 */
/*! \var NxNConst_Flags_SetProp NXNCONST_SETPROP_PER_OBJECT
 *      Set the value of a property as an object-specific value.
 */
/*! \var NxNConst_Flags_SetProp NXNCONST_SETPROP_PATH_TO_HANDLE
 *      Convert the property value from a path (list) to a database handle (list) before setting the property.
 */
/*! \var NxNConst_Flags_SetProp NXNCONST_SETPROP_CLIENT_CACHE
 *      Set the property only in the client-side cache.
 */
enum NxNConst_Flags_SetProp
{   
    NXNCONST_SETPROP_DIRECT         = 0,
    NXNCONST_SETPROP_MULTIPLE       = 1,
    NXNCONST_SETPROP_USER           = 2,
    NXNCONST_SETPROP_PER_OBJECT     = 4,
    NXNCONST_SETPROP_PATH_TO_HANDLE = 8,
    NXNCONST_SETPROP_CLIENT_CACHE   = 0x80000000
};



/*! \enum NxNConst_Menu_Types
 *      These enumerated menu types are for use with CNxNMenu objects. 
 */
/*! \var NxNConst_Menu_Types NXNCONST_MENU_ROOT 
 *      The menu item is the root item. Roots do not contain any menu item relevant data.
 *      They just provide an entry point for recursive menu creation.
 */
/*! \var NxNConst_Menu_Types NXNCONST_MENU_COMMAND
 *      The menu item is a menu command.
 */
/*! \var NxNConst_Menu_Types NXNCONST_MENU_SUBMENU  
 *      This menu item has a sub-menu.
 */
/*! \var NxNConst_Menu_Types NXNCONST_MENU_SEPARATOR
 *      There are two type of separators. If there is an opcode specified in a
 *      separator menu item, then you must treat this menu item as a "category"
 */
/*! \var NxNConst_Menu_Types NXNCONST_MENU_IGNORE
 *      This menu item will be ignored.
 */
enum NxNConst_Menu_Types
{   
        NXNCONST_MENU_ROOT          = 0,
        NXNCONST_MENU_COMMAND       = 1,
        NXNCONST_MENU_SUBMENU       = 2,
        NXNCONST_MENU_SEPARATOR     = 3,
        NXNCONST_MENU_INSERT_POS    = 4,
        NXNCONST_MENU_IGNORE        = 0xffffffff
};


/*! \enum NxNConst_Flags_StatusFlags
 *      These flags define the status of an item are for use with the property "WkspcFlags". 
 *      This property is managed by the system automatically and is available for each object.
 */
/*! \var NxNConst_Flags_StatusFlags NXNCONST_NYNDB_FOLDER   
 *      This flag is only a shortcut to "not NXNCONST_NYNDB_FILE".
 */
/*! \var NxNConst_Flags_StatusFlags NXNCONST_NYNDB_FILE
 *      Object is a file.
 */
/*! \var NxNConst_Flags_StatusFlags NXNCONST_NYNDB_LINK 
 *      Object is a link.
 */
/*! \var NxNConst_Flags_StatusFlags NXNCONST_NYNDB_SYSTEM
 *      Object is a system object.
 */
/*! \var NxNConst_Flags_StatusFlags NXNCONST_NYNDB_READONLY
 *      Object is read only.
 */
/*! \var NxNConst_Flags_StatusFlags NXNCONST_NYNDB_LOCKED
 *      Object is locked by someone else.
 */
/*! \var NxNConst_Flags_StatusFlags NXNCONST_NYNDB_CHECKED_OUT
 *      Object is checked out by the requesting user.
 */
/*! \var NxNConst_Flags_StatusFlags NXNCONST_NYNDB_MOUNTED
 *      This flag is obsolete and should not be used.
 */
/*! \var NxNConst_Flags_StatusFlags NXNCONST_NYNDB_LOCKED_MULTIPLE
 *      Object is locked by several people.
 */
/*! \var NxNConst_Flags_StatusFlags NXNCONST_NYNDB_LOCKED_EXCLUSIVE
 *      Object is exclusivley locked and subsequent multiple check-outs have to be avoided.
 */
enum NxNConst_Flags_StatusFlags
{   
    NXNCONST_NYNDB_FOLDER           =   0,      //  only as a shortcut to "not NYNDB_FILE"
    NXNCONST_NYNDB_FILE             =   1,      //  bit 0
    NXNCONST_NYNDB_LINK             =   2,      //  bit 1
    NXNCONST_NYNDB_SYSTEM           =   4,      //  bit 2
    NXNCONST_NYNDB_READONLY         =   8,      //  bit 3
    NXNCONST_NYNDB_LOCKED           =   16,     //  bit 4,  locked = locked by someone else!
    NXNCONST_NYNDB_CHECKED_OUT      =   32,     //  bit 5,  checked out = checked out by requesting user
    NXNCONST_NYNDB_MOUNTED          =   64,     //  bit 6,  mount point
    NXNCONST_NYNDB_LOCKED_MULTIPLE  =   128,    //  bit 7,  locked by several people
    NXNCONST_NYNDB_LOCKED_EXCLUSIVE =   256,    //  bit 8,  locked, avoid subsequent multiple check-outs
};


#if !defined(_NXN_CONSTANTS_SERVER_DEPENDENCIES_H_) 

#define NXN_PROPERTY_DBPATH ( "DbPath" )
#define NXN_PROPERTY_NAME   ( "Name" )
#define NXN_PROPERTY_HANDLE ( "Handle" )
#define NXN_PROPERTY_CHILDCOUNT ( "ChildCount" )
#define NXN_PROPERTY_SERVERNAME ( "ServerName" )
#define NXN_PROPERTY_FULL_DBPATH    ( "FullDbPath" )
#define NXN_PROPERTY_PATH   ( "Path" )
#define NXN_PROPERTY_RELPATH    ( "RelPath" )
#define NXN_PROPERTY_PREVIEW_URL    ( "PreviewURL" )
#define NXN_PROPERTY_PREVIEW_TEMPLATE   ( "PreviewTemplate" )
#define NXN_PROPERTY_PREVIEW_PATH   ( "PreviewPath" )
#define NXN_PROPERTY_LOCALPATH  ( "LocalPath" )
#define NXN_PROPERTY_WORKINGPATH    ( "WorkingPath" )
#define NXN_PROPERTY_BUILDPATH  ( "BuildPath" )
#define NXN_PROPERTY_INTERMEDIATEPATH   ( "IntermediatePath" )
#define NXN_PROPERTY_FLATTEN_LOCALPATH  ( "FlattenLocalPath" )
#define NXN_PROPERTY_FLATTEN_BUILDPATH  ( "FlattenBuildPath" )
#define NXN_PROPERTY_SERVERPATH ( "ServerPath" )
#define NXN_PROPERTY_MOUNTPATH  ( "MountPath" )
#define NXN_PROPERTY_MOUNTED    ( "Mounted" )
#define NXN_PROPERTY_MIMETYPE   ( "Mime Type" )
#define NXN_PROPERTY_MIMEPATH   ( "Mime Path" )
#define NXN_PROPERTY_MIME_EXTENSION ( "Mime Extension" )
#define NXN_PROPERTY_APPLIES_TO ( "Applies To" )
#define NXN_PROPERTY_PROPERTY_TYPE  ( "Property Type" )
#define NXN_PROPERTY_PROPERTY_INFO  ( "Property Info" )
#define NXN_PROPERTY_INHERIT_FROM   ( "Inherit From" )
#define NXN_PROPERTY_TEMPLATE_NAME  ( "TemplateName" )
#define NXN_PROPERTY_FILE_SIZE  ( "FileSize" )
#define NXN_PROPERTY_FILE_TIME_SERVER   ( "FileTimeServer" )
#define NXN_PROPERTY_FILE_SIZE_SERVER   ( "FileSizeServer" )
#define NXN_PROPERTY_CREATED_AT ( "Created At" )
#define NXN_PROPERTY_CREATED_BY ( "Created By" )
#define NXN_PROPERTY_COMMENT    ( "Comment" )
#define NXN_PROPERTY_KEYWORDS   ( "Keywords" )
#define NXN_PROPERTY_AUTHOR ( "Author" )
#define NXN_PROPERTY_DEFAULT_IMPORT_FOLDER  ( "DefaultImportFolder" )
#define NXN_PROPERTY_IMPORT_IMPLICIT_DEPENDENCIES   ( "ImportImplicitDependencies" )
#define NXN_PROPERTY_IMPLICIT_DEPENDENCY_EXTENSION_LIST ( "ImplicitDependencyExtensionList" )
#define NXN_PROPERTY_IMPORT_IGNORE_EXTENSION_LIST   ( "ImportIgnoreExtensionList" )
#define NXN_PROPERTY_LOCKED_BY  ( "Locked By" )
#define NXN_PROPERTY_LOCKED_AT  ( "Locked At" )
#define NXN_PROPERTY_LOCKED_ON  ( "Locked On" )
#define NXN_PROPERTY_CHECKOUT_PATH  ( "CheckOutPath" )
#define NXN_PROPERTY_CHECKOUT_COMMENT   ( "CheckOutComment" )
#define NXN_PROPERTY_CHECKOUT_DEPENDENCIES  ( "CheckOutDependencies" )
#define NXN_PROPERTY_CHECKIN_COMMENT    ( "CheckInComment" )
#define NXN_PROPERTY_SHARED_ON_SERVER   ( "Shared On Server" )
#define NXN_PROPERTY_FILE_CHECKSUM  ( "FileCheckSum" )
#define NXN_PROPERTY_CHANGED_AT ( "Changed At" )
#define NXN_PROPERTY_CHANGED_BY ( "Changed By" )
#define NXN_PROPERTY_ACTION ( "Action" )
#define NXN_PROPERTY_ACTION_UNDEFINED   ( "0" )
#define NXN_PROPERTY_ACTION_IMPORT  ( "1" )
#define NXN_PROPERTY_ACTION_CHECK_IN    ( "2" )
#define NXN_PROPERTY_ACTION_ROLLBACK    ( "3" )
#define NXN_PROPERTY_ACTION_EXPLICIT    ( "4" )
#define NXN_PROPERTY_ACTION_NEW_FILE    ( "5" )
#define NXN_PROPERTY_ACTION_TEXT_UNDEFINED  ( "Undefined" )
#define NXN_PROPERTY_ACTION_TEXT_IMPORT ( "Imported" )
#define NXN_PROPERTY_ACTION_TEXT_CHECK_IN   ( "Checked in" )
#define NXN_PROPERTY_ACTION_TEXT_ROLLBACK   ( "Rollback" )
#define NXN_PROPERTY_ACTION_TEXT_EXPLICIT   ( "Manual" )
#define NXN_PROPERTY_ACTION_TEXT_NEW_FILE   ( "Created" )
#define NXN_PROPERTY_PASSWORD   ( "Password" )
#define NXN_PROPERTY_REGISTRATION_KEY   ( "RegistrationKey" )
#define NXN_PROPERTY_SERVER_REGISTERED  ( "ServerRegistered" )
#define NXN_PROPERTY_USER_FOLDER    ( "UserFolder" )
#define NXN_PROPERTY_USER_SESSION   ( "UserSession" )
#define NXN_PROPERTY_USER_MACHINE   ( "UserMachine" )
#define NXN_PROPERTY_USER_NAME  ( "UserName" )
#define NXN_PROPERTY_UNIQUE_CHILDREN    ( "UniqueChildren" )
#define NXN_PROPERTY_SCRIPT_PRE_COMPILE ( "PreScript_Compile" )
#define NXN_PROPERTY_SCRIPTHOST_PRE_COMPILE ( "PreScriptHost_Compile" )
#define NXN_PROPERTY_SCRIPT_PRE_COMPILE_FOLDER  ( "PreScript_Compile_Folder" )
#define NXN_PROPERTY_SCRIPTHOST_PRE_COMPILE_FOLDER  ( "PreScriptHost_Compile_Folder" )
#define NXN_PROPERTY_SCRIPT_POST_COMPILE    ( "PostScript_Compile" )
#define NXN_PROPERTY_SCRIPTHOST_POST_COMPILE    ( "PostScriptHost_Compile" )
#define NXN_PROPERTY_SCRIPT_POST_COMPILE_FOLDER ( "PostScript_Compile_Folder" )
#define NXN_PROPERTY_SCRIPTHOST_POST_COMPILE_FOLDER ( "PostScriptHost_Compile_Folder" )
#define NXN_PROPERTY_SCRIPT_PRE_EDIT    ( "PreScript_Edit" )
#define NXN_PROPERTY_SCRIPTHOST_PRE_EDIT    ( "PreScriptHost_Edit" )
#define NXN_PROPERTY_SCRIPT_POST_EDIT   ( "PostScript_Edit" )
#define NXN_PROPERTY_SCRIPTHOST_POST_EDIT   ( "PostScriptHost_Edit" )
#define NXN_PROPERTY_SCRIPT_FILE_CHECKED_IN ( "Script_File_CheckedIn" )
#define NXN_PROPERTY_SCRIPTHOST_FILE_CHECKED_IN ( "ScriptHost_File_CheckedIn" )
#define NXN_PROPERTY_SCRIPT_FILE_CHECKED_OUT    ( "Script_File_CheckedOut" )
#define NXN_PROPERTY_SCRIPTHOST_FILE_CHECKED_OUT    ( "ScriptHost_File_CheckedOut" )
#define NXN_PROPERTY_SCRIPT_FOLDER_CHECKED_OUT  ( "Script_Folder_CheckedOut" )
#define NXN_PROPERTY_SCRIPTHOST_FOLDER_CHECKED_OUT  ( "ScriptHost_Folder_CheckedOut" )
#define NXN_PROPERTY_SCRIPT_FILE_UNDONE_CHECKOUT    ( "Script_File_UndoneCheckOut" )
#define NXN_PROPERTY_SCRIPTHOST_FILE_UNDONE_CHECKOUT    ( "ScriptHost_File_UndoneCheckOut" )
#define NXN_PROPERTY_SCRIPT_FILE_LOCALLY_DELETED    ( "Script_File_LocallyDeleted" )
#define NXN_PROPERTY_SCRIPTHOST_FILE_LOCALLY_DELETED    ( "ScriptHost_File_LocallyDeleted" )
#define NXN_PROPERTY_SCRIPT_FOLDER_LOCALLY_DELETED  ( "Script_Folder_LocallyDeleted" )
#define NXN_PROPERTY_SCRIPTHOST_FOLDER_LOCALLY_DELETED  ( "ScriptHost_Folder_LocallyDeleted" )
#define NXN_PROPERTY_SCRIPT_FILE_GOT_LATEST ( "Script_File_GotLatest" )
#define NXN_PROPERTY_SCRIPTHOST_FILE_GOT_LATEST ( "ScriptHost_File_GotLatest" )
#define NXN_PROPERTY_SCRIPT_FOLDER_GOT_LATEST   ( "Script_Folder_GotLatest" )
#define NXN_PROPERTY_SCRIPTHOST_FOLDER_GOT_LATEST   ( "ScriptHost_Folder_GotLatest" )
#define NXN_PROPERTY_SCRIPT_FILE_IMPORTED   ( "Script_File_Imported" )
#define NXN_PROPERTY_SCRIPTHOST_FILE_IMPORTED   ( "ScriptHost_File_Imported" )
#define NXN_PROPERTY_SCRIPT_FOLDER_IMPORTED ( "Script_Folder_Imported" )
#define NXN_PROPERTY_SCRIPTHOST_FOLDER_IMPORTED ( "ScriptHost_Folder_Imported" )
#define NXN_PROPERTY_SCRIPT_FILE_MOUNTED    ( "Script_File_Mounted" )
#define NXN_PROPERTY_SCRIPTHOST_FILE_MOUNTED    ( "ScriptHost_File_Mounted" )
#define NXN_PROPERTY_SCRIPT_FOLDER_MOUNTED  ( "Script_Folder_Mounted" )
#define NXN_PROPERTY_SCRIPTHOST_FOLDER_MOUNTED  ( "ScriptHost_Folder_Mounted" )
#define NXN_PROPERTY_SCRIPT_FILE_NEW    ( "Script_File_New" )
#define NXN_PROPERTY_SCRIPTHOST_FILE_NEW    ( "ScriptHost_File_New" )
#define NXN_PROPERTY_SCRIPT_FOLDER_NEW  ( "Script_Folder_New" )
#define NXN_PROPERTY_SCRIPTHOST_FOLDER_NEW  ( "ScriptHost_Folder_New" )
#define NXN_PROPERTY_SCRIPT_ITEM_MOVED  ( "Script_Item_Moved" )
#define NXN_PROPERTY_SCRIPTHOST_ITEM_MOVED  ( "ScriptHost_Item_Moved" )
#define NXN_PROPERTY_SCRIPT_PROJECT_LOADED  ( "Script_Project_Loaded" )
#define NXN_PROPERTY_SCRIPTHOST_PROJECT_LOADED  ( "ScriptHost_Project_Loaded" )
#define NXN_PROPERTY_SCRIPT_PROJECT_UNLOADING   ( "Script_Project_Unloading" )
#define NXN_PROPERTY_SCRIPTHOST_PROJECT_UNLOADING   ( "ScriptHost_Project_Unloading" )
#define NXN_PROPERTY_DEPENDENCY_LIST    ( "DependencyList" )
#define NXN_PROPERTY_EXTERNAL_DEPENDENCY_LIST   ( "ExternalDependencyList" )
#define NXN_PROPERTY_MANUAL_DEPENDENCY_LIST ( "ManualDependencyList" )
#define NXN_PROPERTY_REFERENCE_LIST ( "ReferenceList" )
#define NXN_PROPERTY_ALLOW_RECURSION_OPCODES    ( "AllowRecursion_OpCodes" )
#define NXN_PROPERTY_ALLOW_RECURSION_MANUAL ( "AllowRecursion_Manual" )
#define NXN_PROPERTY_ALLOW_RECURSION_INTERNAL   ( "AllowRecursion_Internal" )
#define NXN_PROPERTY_DEPENDENCY_DO_SEARCH_FOLDERS   ( "DependencyDoSearchFolders" )
#define NXN_PROPERTY_DEPENDENCY_SEARCH_FOLDER_LIST  ( "DependencySearchFolderList" )
#define NXN_PROPERTY_DEPENDENCY_SEARCH_NEIGHBOURS   ( "DependencySearchNeighbours" )
#define NXN_PROPERTY_DEPENDENCY_SEARCH_RELATIVE ( "DependencySearchRelative" )
#define NXN_PROPERTY_DEPENDENCY_CHECK_FOR_EXISTING  ( "DependencyCheckForExisting" )

#define NXN_DEPENDENCY_TYPE_INTERNAL    ( "internal" )
#define NXN_DEPENDENCY_TYPE_MANUAL  ( "manual" )
#define NXN_DEPENDENCY_TYPE_EXTERNAL    ( "external" )
#define NXN_DEPENDENCY_TYPE_IMPLICIT    ( "implicit" )
#define NXN_UM_IS_ACTIVE    ( "UM_UserManagement" )
#define NXN_UM_IS_ON    ( "On" )
#define NXN_UM_IS_OFF   ( "Off" )
#define NXN_UM_IS_IGNORE    ( "Ignore" )
#define NXN_UM_GET_PROPERTIES   ( "UM_Get_Properties" )
#define NXN_UM_SET_PROPERTIES   ( "UM_Set_Properties" )
#define NXN_UM_FOR_MIMETYPES    ( "UM_For_MimeTypes" )
#define NXN_UM_FOR_USERS    ( "UM_For_Users" )
#define NXN_UM_FOR_FOLDERS  ( "UM_For_Folders" )
#define NXN_UM_HIDEMENU ( "UM_HideMenu" )
#define NXN_UM_VERBOSEERRORS    ( "UM_VerboseErrors" )
#define NXN_UM_VERBOSE_MODE ( "UM_Verbose" )
#define NXN_UM_PROPERTY_DEFAULT_EXECUTION   ( "UM_Default_Execution" )
#define NXN_UM_PROPERTY_VALUE_ALLOW_STATIC  ( "Allow" )
#define NXN_UM_PROPERTY_VALUE_DENY_STATIC   ( "Deny" )

#define NXN_PROPERTY_SCAN_DEPENDENCIES  ( "ScanDependencies" )
#define NXN_PROPERTY_SCAN_USE_EXTERNAL  ( "ScanUseExternal" )
#define NXN_PROPERTY_SCAN_EXECUTABLE    ( "ScanExecutable" )
#define NXN_PROPERTY_SCAN_EXECUTABLE_COMMANDLINE    ( "ScanExecutableCommandLine" )
#define NXN_PROPERTY_SCAN_ACCEPT_EXTENSIONS ( "ScanAcceptExtensions" )
#define NXN_PROPERTY_SCAN_REJECT_EXTENSIONS ( "ScanRejectExtensions" )
#define NXN_PROPERTY_SCAN_THIS_FILE ( "ScanThisFile" )
#define NXN_PROPERTY_SCAN_SURROGATE_FILE    ( "ScanSurrogateFile" )
#define NXN_PROPERTY_SCAN_AUTO_IMPORT_REFERENCES    ( "ScanAutoImport" )
#define NXN_PROPERTY_SCAN_IMPORT_EXTENSION_LIST ( "ScanImportExtensionList" )

#define NXN_PROPERTY_OBJECTCOUNT_TOTAL  ( "ObjectCountTotal" )
#define NXN_PROPERTY_DBLEVEL    ( "DbLevel" )
#define NXN_PROPERTY_CHANGE_TIME    ( "ChangeTime" )
#define NXN_PROPERTY_CHANGE_TIME_CHILD  ( "ChangeTimeChild" )

#define NXN_PROPERTY_DBSERVER_CLIENTCOUNT   ( "_ClientCount" )
#define NXN_PROPERTY_DBSERVER_PROJECTCOUNT  ( "_ProjectCount" )
#define NXN_PROPERTY_DBSERVER_CONNECTIONCOUNT   ( "_ConnectionCount" )
#define NXN_PROPERTY_DBSERVER_PROJECTLIST   ( "_ProjectList" )
#define NXN_PROPERTY_DBSERVER_RECENTPROJECTLIST ( "_RecentProjectList" )
#define NXN_PROPERTY_DBSERVER_RECENTPROJECTLIST_FULL    ( "_RecentProjectListFull" )
#define NXN_PROPERTY_DBSERVER_MAXRECENTPROJECTS ( "_MaxRecentProjects" )
#define NXN_PROPERTY_DBSERVER_PROJECTTRANSACTIONCOUNT   ( "_ProjectTransactionCount" )
#define NXN_PROPERTY_DBSERVER_PROJECTHISTORY    ( "_ProjectHistory" )
#define NXN_PROPERTY_DBSERVER_TOTALTRANSACTIONCOUNT ( "_TotalTransactionCount" )
#define NXN_PROPERTY_DBSERVER_AVGTRANSACTIONDURATION    ( "_AvgTransactionDuration" )
#define NXN_PROPERTY_DBSERVER_MINTRANSACTIONDURATION    ( "_MinTransactionDuration" )
#define NXN_PROPERTY_DBSERVER_MAXTRANSACTIONDURATION    ( "_MaxTransactionDuration" )
#define NXN_PROPERTY_DBSERVER_LOGMESSAGECOUNT   ( "_LogMessageCount" )
#define NXN_PROPERTY_DBSERVER_LOGMESSAGES   ( "_LogMessages" )
#define NXN_PROPERTY_DBSERVER_LOGFILENAME   ( "_LogFilename" )
#define NXN_PROPERTY_DBSERVER_LOGFILENAME_PROJECT   ( "_LogFilenameProject" )
#define NXN_PROPERTY_DBSERVER_AUTOSAVE  ( "_AutoSave" )
#define NXN_PROPERTY_DBSERVER_VERSION   ( "_Version" )
#define NXN_PROPERTY_DBSERVER_PATH  ( "_Path" )
#define NXN_PROPERTY_DBSERVER_LOGGEDON_USERS    ( "_LoggedOnUsers" )
#define NXN_PROPERTY_DBSERVER_LICENSE_INFO  ( "_LicenseInfo" )
#define NXN_PROPERTY_DBSERVER_LIC_CLIENTS   ( "_Clients" )
#define NXN_PROPERTY_DBSERVER_LIC_PROJECTS  ( "_Projects" )
#define NXN_PROPERTY_DBSERVER_LIC_ASSETS    ( "_Assets" )
#define NXN_PROPERTY_DBSERVER_LIC_USED  ( "_UsedLicenses" )
#define NXN_PROPERTY_DBSERVER_LAST_ACTIVITY ( "_LastActivity" )
#define NXN_PROPERTY_DBSERVER_LAST_SAVE ( "_LastSave" )
#define NXN_PROPERTY_DBSERVER_REMAINING_EVAL_DAYS   ( "_RemainingEvalDays" )

#define NXN_COMMAND_DBSERVER_RECENT_PROJECT_CHANGE  ( "_RecentProjectChange" )
#define NXN_COMMAND_DBSERVER_RECENT_PROJECT_ADD ( "_RecentProjectAdd" )
#define NXN_COMMAND_DBSERVER_RECENT_PROJECT_REMOVE  ( "_RecentProjectRemove" )
#define NXN_COMMAND_DBSERVER_TEST_EVENT_MANAGER ( "_TestEventManager" )

#define NXN_PROPERTYTYPE_DIRECTORY  ( "Directory" )
#define NXN_PROPERTYTYPE_DIRECTORY_LIST ( "DirectoryList" )
#define NXN_PROPERTYTYPE_FILENAME   ( "Filename" )
#define NXN_PROPERTYTYPE_FILENAME_LIST  ( "FilenameList" )
#define NXN_PROPERTYTYPE_COMPUTER   ( "Computer" )
#define NXN_PROPERTYTYPE_COMPUTER_LIST  ( "ComputerList" )
#define NXN_PROPERTYTYPE_ENUM   ( "Enum" )
#define NXN_PROPERTYTYPE_ENUMLIST   ( "EnumList" )
#define NXN_PROPERTYTYPE_BOOL   ( "Bool" )
#define NXN_PROPERTYTYPE_BOOLLIST   ( "BoolList" )
#define NXN_PROPERTYTYPE_STRING ( "String" )
#define NXN_PROPERTYTYPE_STRINGLIST ( "StringList" )
#define NXN_PROPERTYTYPE_SCRIPT ( "Script" )
#define NXN_PROPERTYTYPE_SCRIPTLIST ( "ScriptList" )
#define NXN_PROPERTYTYPE_INTEGER    ( "Integer" )
#define NXN_PROPERTYTYPE_INTEGERLIST    ( "IntegerList" )
#define NXN_PROPERTYTYPE_NAMESPACEOBJ   ( "NamespaceObj" )
#define NXN_PROPERTYTYPE_NAMESPACEOBJLIST   ( "NamespaceObjList" )

#define NYNDB_FOLDER_ROOT   ( "Root" )
#define NYNDB_FOLDER_SYSTEM ( "System" )
#define NYNDB_FOLDER_ARCHIVES   ( "Archives" )
#define NYNDB_FOLDER_CONFIGURATIONS ( "Configurations" )
#define NYNDB_FOLDER_MIMETYPES  ( "Mime Types" )
#define NYNDB_FOLDER_PROPERTIES ( "Properties" )
#define NYNDB_FOLDER_TOOLS  ( "Tools" )
#define NYNDB_FOLDER_USERS  ( "Users" )
#define NYNDB_FOLDER_USER   ( "User" )
#define NYNDB_FOLDER_MACHINE    ( "Machine" )
#define NYNDB_FOLDER_SESSION    ( "Session" )

#define NXN_COMMAND_CHECK_OUT   ( "CheckOut" )
#define NXN_COMMAND_CHECK_OUT_PREPARE   ( "CheckOutPrepare" )
#define NXN_COMMAND_CHECK_IN    ( "CheckIn" )
#define NXN_COMMAND_CHECK_IN_PREPARE    ( "CheckInPrepare" )
#define NXN_COMMAND_UNDO_CHECK_OUT  ( "UndoCheckOut" )
#define NXN_COMMAND_UNDO_CHECK_OUT_PREPARE  ( "UndoCheckOutPrepare" )
#define NXN_COMMAND_GET_LATEST  ( "GetLatest" )
#define NXN_COMMAND_IMPORT  ( "Import" )
#define NXN_COMMAND_IMPORT_PREPARE  ( "ImportPrepare" )
#define NXN_COMMAND_MOUNT   ( "ImportMount" )
#define NXN_COMMAND_GET_PRIORITY    ( "GetPriority" )
#define NXN_COMMAND_LOGON   ( "Logon" )
#define NXN_COMMAND_PRELOGON    ( "PreLogon" )
#define NXN_COMMAND_POSTLOGON   ( "PostLogon" )
#define NXN_COMMAND_LOGOFF  ( "Logoff" )
#define NXN_COMMAND_SHUTDOWN    ( "Shutdown" )
#define NXN_COMMAND_SAVE    ( "Save" )
#define NXN_COMMAND_CHANGE_PASSWORD ( "ChangePassword" )
#define NXN_COMMAND_GETPROP ( "GetProp" )
#define NXN_COMMAND_SETPROP ( "SetProp" )
#define NXN_COMMAND_NEW_FILE    ( "NewFile" )
#define NXN_COMMAND_NEW_FOLDER  ( "NewFolder" )
#define NXN_COMMAND_NEW_ITEM_PREPARE    ( "NewItemPrepare" )
#define NXN_COMMAND_DELETE_TREE ( "DeleteTree" )
#define NXN_COMMAND_DELETE_WITH_UNDO    ( "Delete" )
#define NXN_COMMAND_MOVE_WITH_UNDO  ( "MoveWithUndo" )
#define NXN_COMMAND_UNDO_MOVE   ( "UndoMove" )
#define NXN_COMMAND_CREATE_FOLDERS  ( "CreateFolders" )
#define NXN_COMMAND_MOVE    ( "Move" )
#define NXN_COMMAND_SET_TEMPLATE    ( "SetTemplate" )
#define NXN_COMMAND_CREATE_PREPARE  ( "CreatePrepare" )
#define NXN_COMMAND_EDIT    ( "Edit" )
#define NXN_COMMAND_COMPILE ( "Compile" )
#define NXN_COMMAND_LINK    ( "Link" )
#define NXN_COMMAND_COPY    ( "Copy" )
#define NXN_COMMAND_FIND    ( "Find" )
#define NXN_COMMAND_DEPENDENCY_ADD  ( "DependencyAdd" )
#define NXN_COMMAND_DEPENDENCY_REMOVE   ( "DependencyRemove" )
#define NXN_COMMAND_DEPENDENCY_CLEAR    ( "DependencyClear" )
#define NXN_COMMAND_FIND_EXISTING   ( "FindExisting" )
#define NXN_COMMAND_GET_NEXT_CHILD  ( "GetNextChild" )
#define NXN_COMMAND_GET_NEXT_CHILDREN   ( "GetNextChildren" )
#define NXN_COMMAND_GET_NEXT_FOLDERS    ( "GetNextFolders" )
#define NXN_COMMAND_GET_PROPERTY_INFO   ( "GetPropertyInfo" )
#define NXN_COMMAND_GET_CHANGE_INFO ( "GetChangeInfo" )
#define NXN_COMMAND_UPDATE_MASTER_FILE  ( "UpdateMasterFile" )
#define NXN_COMMAND_SET_MOUNT_ROOT  ( "SetMountRoot" )
#define NXN_COMMAND_CREATE_INSTANCE ( "CreateInstance" )
#define NXN_COMMAND_FREE_INSTANCE   ( "FreeInstance" )

#define NYNDB_EMPTY ( "-empty-" )
#define NYNDB_USERNAME_ADMINISTRATOR    ( "Administrator" )
#define NYNDB_USERGROUP_ADMINISTRATORS  ( "Administrators" )
#define NYNDB_DEFAULT_LOCALHOST ( "LOCALHOST" )
#define NYNDB_DEFAULT_EXTENSION ( "ndb" )

#define NXN_SCRIPT_LANGUAGE ( "NxN Script Language" )
#define NXN_SCRIPT_NAME_PRE ( "PreScript_" )
#define NXN_SCRIPT_NAME_POST    ( "PostScript_" )
#define NXN_SCRIPT_HOST_PRE ( "PreScriptHost_" )
#define NXN_SCRIPT_HOST_POST    ( "PostScriptHost_" )

#define NYNDB_TRUE  ( "1" )
#define NYNDB_FALSE ( "0" )
#define NXN_WORKFLOW_PROPSUFFIX_MOVED_ITEMS ( "_Items" )
#define NXN_WORKFLOW_PROPSUFFIX_MOVED_FROM  ( "_From" )
#define NXN_WORKFLOW_PROPSUFFIX_MOVED_AT    ( "_At" )
#define NXN_WORKFLOW_PROPSUFFIX_MOVED_BY    ( "_By" )
#define NXN_WORKFLOW_PROPSUFFIX_MOVE_COMMENT    ( "_Comment" )
#define NXN_WORKFLOW_PROPSUFFIX_MOVE_OLD_NAME   ( "_OldName" )
#define NXN_WORKFLOW_PROPSUFFIX_MOVE_OLD_DBPATH ( "_OldDbPath" )

#define NXN_TOPLEVEL_DISPLAYNAME_DELETED_ITEMS  ( "Recycle Bin" )
#define NXN_TOPLEVEL_NAME_DELETED_ITEMS ( "Deleted Items" )
#define NXN_OPERATION_DELETE    ( "Delete" )
#define NXN_TOPLEVEL_NAME_FORWARDED ( "Forwarded Items" )
#define NXN_OPERATION_FORWARD   ( "Forward" )
#define NXN_NAMESPACE_CB_HINT_PROJECT_LOADED    ( "ProjectLoaded" )
#define NXN_NAMESPACE_CB_HINT_PROJECT_UNLOADED  ( "ProjectUnloaded" )

#define NYNDB_DIRNAME_MASTER_TREE   ( "_Master" )
#define NYNDB_DIRNAME_HANDOVER_CHECKIN  ( "_CheckIn" )
#define NYNDB_DIRNAME_HANDOVER_IMPORT   ( "_Import" )
#define NYNDB_DIRNAME_CLIENT_CHECKOUT   ( "CheckOut" )
#define NYNDB_DIRNAME_CLIENT_BUILD  ( "Build" )

#ifndef __NXN_BLIND_HWND_DEFINED
#define __NXN_BLIND_HWND_DEFINED

#define NXN_BLIND_HWND  ( 0xffffffff )

#endif // __NXN_BLIND_HWND_DEFINED


#define NXN_PARAM_APPLY_TO_REFERENCES   ( "ApplyToReferences" )
#define NXN_PARAM_ASK_FOR_EACH_OBJECT   ( "AskForEachObject" )
#define NXN_PARAM_ARGUMENT_LIST ( "ArgumentList" )
#define NXN_PARAM_BROWSER_DIALOG    ( "BrowserDialog" )
#define NXN_PARAM_BUILD_PATH    ( "BuildPath" )
#define NXN_PARAM_CHECKSUM  ( "Checksum" )
#define NXN_PARAM_CHECKOUT_PATH ( "CheckoutPath" )
#define NXN_PARAM_COMMENT   ( "Comment" )
#define NXN_PARAM_CREATE_DIRECTORY_STRUCTURE_ONLY   ( "CreateDirectoryStructureOnly" )
#define NXN_PARAM_CREATE_VERSION    ( "CreateVersion" )
#define NXN_PARAM_DBPATH    ( "DbPath" )
#define NXN_PARAM_DEFAULT_FOLDER    ( "DefaultFolder" )
#define NXN_PARAM_DELETE_LOCAL  ( "DeleteLocal" )
#define NXN_PARAM_DEPENDENCY_TYPE   ( "DependencyType" )
#define NXN_PARAM_DESTINATION_PATH  ( "DestinationPath" )
#define NXN_PARAM_DONT_CHECK_FOR_EXISTING   ( "DontCheckForExisting" )
#define NXN_PARAM_FAIL_IF_EXISTS    ( "FailIfExists" )
#define NXN_PARAM_FILE_MODIFICATION_TIME    ( "FileModificationTime" )
#define NXN_PARAM_FILENAME  ( "Filename" )
#define NXN_PARAM_FILTER    ( "Filter" )
#define NXN_PARAM_FILE_SIZE ( "FileSize" )
#define NXN_PARAM_GET_LOCAL ( "GetLocal" )
#define NXN_PARAM_GET_PROPERTY  ( "GetProperty" )
#define NXN_PARAM_HANDLE    ( "Handle" )
#define NXN_PARAM_HEADER_PATH   ( "HeaderPath" )
#define NXN_PARAM_HOSTNAME  ( "Hostname" )
#define NXN_PARAM_KEEP_CHECKED_OUT  ( "KeepCheckedOut" )
#define NXN_PARAM_KEEP_UNCHANGED    ( "KeepUnchanged" )
#define NXN_PARAM_KEYWORDS  ( "Keywords" )
#define NXN_PARAM_LOCALPATH ( "LocalPath" )
#define NXN_PARAM_LOCALPATH_SCANNED_FILE    ( "LocalPathScannedFile" )
#define NXN_PARAM_LOCALPATH_SERVER  ( "LocalPathServer" )
#define NXN_PARAM_MERGE ( "Merge" )
#define NXN_PARAM_MIME_TYPE ( "MimeType" )
#define NXN_PARAM_MOUNT_PATH    ( "MountPath" )
#define NXN_PARAM_NAME  ( "Name" )
#define NXN_PARAM_PATH  ( "Path" )
#define NXN_PARAM_PASSWORD  ( "Password" )
#define NXN_PARAM_RECURSIVE ( "AutoImport_Recursive" )
#define NXN_PARAM_REFERENCED_PATHS  ( "ReferencedPaths" )
#define NXN_PARAM_QUERY_EXPRESSION  ( "QueryExpression" )
#define NXN_PARAM_SCAN_ONLY_WRITEABLE   ( "ScanOnlyWriteable" )
#define NXN_PARAM_SCAN_PATH ( "ScanPath" )
#define NXN_PARAM_SERVER_MASTER_PATH    ( "ServerMasterPath" )
#define NXN_PARAM_SERVERPATH    ( "ServerPath" )
#define NXN_PARAM_SERVER_TRANSFER_PATH  ( "ServerTransferPath" )
#define NXN_PARAM_SOURCE_PATH   ( "SourcePath" )
#define NXN_PARAM_TARGET_PATH   ( "TargetPath" )
#define NXN_PARAM_TITLE ( "Title" )
#define NXN_PARAM_USERNAME  ( "Username" )
#define NXN_PARAM_USE_TEMPLATE  ( "UseTemplate" )
#define NXN_PARAM_UNDO_MOVE_OPERATION   ( "UndoMoveOperation" )
#define NXN_PARAM_VERBOSE_LEVEL ( "VerboseLevel" )
#define NXN_PARAM_WINDOMAIN ( "WinDomain" )
#define NXN_PARAM_WINPASSWORD   ( "WinPassword" )
#define NXN_PARAM_WINUSERNAME   ( "WinUsername" )
#define NXN_PARAM_WORKING_PATH  ( "WorkingPath" )

#define NXN_COMMAND_WORKSPACE_OPEN  ( "WorkspaceOpen" )
#define NXN_COMMAND_WORKSPACE_CREATE    ( "WorkspaceCreate" )
#define NXN_COMMAND_WORKSPACE_CLOSE ( "WorkspaceClose" )
#define NXN_COMMAND_WORKSPACE_SAVE  ( "WorkspaceSave" )
#define NXN_COMMAND_WORKSPACE_ABOUT ( "WorkspaceAbout" )
#define NXN_COMMAND_PROJECT_INSERT  ( "ProjectInsert" )
#define NXN_COMMAND_PROJECT_NEW ( "ProjectNew" )
#define NXN_COMMAND_PROJECT_LOAD    ( "ProjectLoad" )
#define NXN_COMMAND_PROJECT_LOAD_AS ( "ProjectLoadAs" )
#define NXN_COMMAND_PROJECT_UNLOAD  ( "ProjectUnload" )
#define NXN_COMMAND_PROJECT_REMOVE  ( "ProjectRemove" )
#define NXN_COMMAND_PROJECT_NEW_EX  ( "ProjectNewEx" )
#define NXN_COMMAND_PROJECT_INSERT_EX   ( "ProjectInsertEx" )
#define NXN_COMMAND_PROJECT_LOAD_EX ( "ProjectLoadEx" )
#define NXN_COMMAND_PROJECT_UNLOAD_EX   ( "ProjectUnloadEx" )
#define NXN_COMMAND_PROJECT_REMOVE_EX   ( "ProjectRemoveEx" )
#define NXN_COMMAND_FOLDER_SET_WORKINGPATH  ( "SetWorkingPath" )
#define NXN_COMMAND_FILE_MIMETYPE_SETTINGS  ( "MimeTypeSettings" )
#define NXN_COMMAND_DELETE_LOCAL_COPY   ( "DeleteLocalCopy" )
#define NXN_COMMAND_ADVANCED_PROPERTIES ( "AdvancedProperties" )
#define NXN_COMMAND_BROWSE_REFERENCES   ( "BrowseReferences" )
#define NXN_COMMAND_GETSET_PROPERTY ( "GetSetProperty" )
#define NXN_COMMAND_ADMIN_USERMANAGER   ( "UserManager" )
#define NXN_COMMAND_ADMIN_PROPMANAGER   ( "PropertyManager" )
#define NXN_COMMAND_ADMIN_MIMEMANAGER   ( "MimeTypeManager" )

#define NXN_PROPERTY_WORKSPACE_HPROJECT ( "WkspcProjectHandle" )
#define NXN_PROPERTY_WORKSPACE_HOBJECT  ( "WkspcObjectHandle" )
#define NXN_PROPERTY_WORKSPACE_FLAGS    ( "WkspcFlags" )
#define NXN_PROPERTY_WORKSPACE_ITEM_STATUS  ( "WkspcItemStatus" )
#define NXN_PROPERTY_WORKSPACE_ITEM_TYPE    ( "WkspcItemType" )
#define NXN_PROPERTY_PROJECT_COUNT  ( "WkspcProjectCount" )
#define NXN_PROPERTY_PROJECT_FILENAME   ( "WkspcProjectFilename" )
#define NXN_PROPERTY_PROJECT_SERVER ( "WkspcProjectServer" )
#define NXN_PROPERTY_PROJECT_LOGON  ( "WkspcProjectLogon" )
#define NXN_OBJNAME_PROPERTIES  ( "Registered Properties" )
#define NXN_PROPERTY_FLAGS  ( "Flags" )
#define NXN_COMMAND_GET_CHILDREN    ( "GetChildren" )
#define NXN_COMMAND_GET_PROPERTIES  ( "GetProperties" )

#endif //_NXN_CONSTANTS_SERVER_DEPENDENCIES_H_
#endif // INC_NXN_CONSTANTS_H
