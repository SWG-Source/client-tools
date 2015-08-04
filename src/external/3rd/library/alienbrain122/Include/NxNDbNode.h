// \addtodoc

#ifndef INC_NXN_DBNODE_H
#define INC_NXN_DBNODE_H

/*!	\file		NxNDbNode.h
 *	
 *	\brief		This file contains the definition for the class CNxNDbNode and its associated types.
 *	
 *	\author		Axel Pfeuffer, Helmut Klinger
 *	
 *	\version	1.00
 *	
 *	\date		2000
 *	
 *	\mod
 *	\endmod
 */

/* \class       CNxNDbNode
 *
 *  \file       NxNDbNode.h
 *
 *  \brief      The class CNxNDbNode defines the basis for all objects that are contained in the project database.
 *
 *  \author     Axel Pfeuffer, Helmut Klinger
 *
 *  \version    1.00
 *
 *  \date       2000
 *
 *  \mod
 *      [ap]-02-May-2000 file created.
 *      [ap]-21-Sep-2000 BUGFIX: Most methods are now virtual.
 *      [gs]-12-Oct-2000 added 'bSilent' parameter to GetLatest() 
 *      [gs]-11-Nov-2000 renamed eCheckIn_CreateVersionPolicy enumeration
 *      [gs]-11-Nov-2000 replaced GetLatest() 'bKeepUnchanged' (obsolete!) parameter with 'bForceUpdate' 
 *      [gs]-11-Nov-2000 added 'bOnlyExisting', 'bForceUpdate', 'nOWP', 'nOCP', 'nFTP' parameters to GetLatest()
 *  \endmod
 */

//---------------------------------------------------------------------------
//  check in - create version flags
//---------------------------------------------------------------------------
/*! \enum eCheckIn_CreateVersionPolicy
 *      These enumeration values define different version creation policies for the check-in command.
 */
/*! \var eCheckIn_CreateVersionPolicy NXN_CHECKIN_CREATE_VERSION_DEFAULT 
 *      This value specifies that the default policy will be used. Same as NXN_CHECKIN_CREATE_VERSION_AUTO.
 */
/*! \var eCheckIn_CreateVersionPolicy NXN_CHECKIN_CREATE_VERSION_AUTO    
 *      This value specifies that a new version will be created if the checked-in file changed.
 */
/*! \var eCheckIn_CreateVersionPolicy NXN_CHECKIN_CREATE_VERSION_NO
 *      This value specifies that no new version will be created.
 */
/*! \var eCheckIn_CreateVersionPolicy NXN_CHECKIN_CREATE_VERSION_YES
 *      This value specifies that a new version will be created even if the checked-in file did not change.
 */
enum eCheckIn_CreateVersionPolicy
{
    NXN_CHECKIN_CREATE_VERSION_DEFAULT  = 0,
    NXN_CHECKIN_CREATE_VERSION_AUTO     = 0,
    NXN_CHECKIN_CREATE_VERSION_NO       = 1,
    NXN_CHECKIN_CREATE_VERSION_YES      = 2,
    NXN_CHECKIN_CREATE_VERSION_MAX
};

/*! \enum eGetLatest_OverwriteWritablePolicy
 *      These enumeration values define different overwrite policies for existing files
 *      that are writeable when calling GetLatest().
 */
/*! \var eGetLatest_OverwriteWritablePolicy NXN_GETLATEST_OVERWRITE_WRITABLE_DEFAULT 
 *      This value specifies that the default policy will be used. Same as NXN_GETLATEST_OVERWRITE_WRITABLE_ASK.
 */
/*! \var eGetLatest_OverwriteWritablePolicy NXN_GETLATEST_OVERWRITE_WRITABLE_ASK    
 *      This value specifies that a dialog box comes up asking the user if the existing (writeable) file should be kept or overwritten.
 */
/*! \var eGetLatest_OverwriteWritablePolicy NXN_GETLATEST_OVERWRITE_WRITABLE_SKIP
 *      This value specifies that a writeable file should be skipped and not overwritten.
 */
/*! \var eGetLatest_OverwriteWritablePolicy NXN_GETLATEST_OVERWRITE_WRITABLE_REPLACE
 *      This value specifies that a writeable file should be replaced by the server version.
 */
enum eGetLatest_OverwriteWritablePolicy
{
    NXN_GETLATEST_OVERWRITE_WRITABLE_DEFAULT    = 0,
    NXN_GETLATEST_OVERWRITE_WRITABLE_ASK        = 0,
    NXN_GETLATEST_OVERWRITE_WRITABLE_SKIP       = 1,
    NXN_GETLATEST_OVERWRITE_WRITABLE_REPLACE    = 2
};

/*! \enum eGetLatest_OverwriteCheckedOutPolicy
 *      These enumeration values define different overwrite policies for existing files
 *      that are checked-out when calling GetLatest().
 */
/*! \var eGetLatest_OverwriteCheckedOutPolicy NXN_GETLATEST_OVERWRITE_CHECKEDOUT_DEFAULT 
 *      This value specifies that the default policy will be used. Same as NXN_GETLATEST_OVERWRITE_CHECKEDOUT_ASK.
 */
/*! \var eGetLatest_OverwriteCheckedOutPolicy NXN_GETLATEST_OVERWRITE_CHECKEDOUT_ASK    
 *      This value specifies that a dialog box comes up asking the user if the existing (checked-out) file should be kept or overwritten.
 */
/*! \var eGetLatest_OverwriteCheckedOutPolicy NXN_GETLATEST_OVERWRITE_CHECKEDOUT_SKIP
 *      This value specifies that a writeable file should be skipped and not overwritten.
 */
/*! \var eGetLatest_OverwriteCheckedOutPolicy NXN_GETLATEST_OVERWRITE_CHECKEDOUT_REPLACE
 *      This value specifies that a writeable file should be replaced by the server version.
 */
enum eGetLatest_OverwriteCheckedOutPolicy
{
    NXN_GETLATEST_OVERWRITE_CHECKEDOUT_DEFAULT  = 0,
    NXN_GETLATEST_OVERWRITE_CHECKEDOUT_ASK      = 0,
    NXN_GETLATEST_OVERWRITE_CHECKEDOUT_SKIP     = 1,
    NXN_GETLATEST_OVERWRITE_CHECKEDOUT_REPLACE  = 2
};

/*! \enum eGetLatest_FileTimePolicy
 *      These enumeration values specify the file modification time that the local copy of a file will receive when calling GetLatest().
 */
/*! \var eGetLatest_FileTimePolicy NXN_GETLATEST_FILETIME_DEFAULT 
 *      This value specifies that the default policy will be used. The default policy is the currently persistent setting in alienbrain.
 */
/*! \var eGetLatest_FileTimePolicy NXN_GETLATEST_FILETIME_MODIFICATION    
 *      This value specifies that the modification time of the file to be retrieved will be used.
 */
/*! \var eGetLatest_FileTimePolicy NXN_GETLATEST_FILETIME_CHECKIN
 *      This value specifies that the check-in time of the file to be retrieved will be used.
 */
/*! \var eGetLatest_FileTimePolicy NXN_GETLATEST_FILETIME_CURRENT
 *      This value specifies that the current system time will be used for the file to be retrieved.
 */
enum eGetLatest_FileTimePolicy
{
    NXN_GETLATEST_FILETIME_DEFAULT      = -1,
    NXN_GETLATEST_FILETIME_MODIFICATION = 0,
    NXN_GETLATEST_FILETIME_CHECKIN      = 1,
    NXN_GETLATEST_FILETIME_CURRENT      = 2
};


/*! \enum eUndoCheckOut_UpdatePolicy
 *      These enumeration values specify the action that is performed with the local copy if an UndoCheckOut is done on a file.
 */
/*! \var eUndoCheckOut_UpdatePolicy NXN_UNDOCHECKOUT_UPDATE_POLICY_DEFAULT
 *      This value specifies that the default update policy will be used. The default policy is the currently persistent setting in alienbrain.
 */
/*! \var eUndoCheckOut_UpdatePolicy NXN_UNDOCHECKOUT_UPDATE_POLICY_ASK
 *      This value specifies that the user should be asked.
 */
/*! \var eUndoCheckOut_UpdatePolicy NXN_UNDOCHECKOUT_UPDATE_POLICY_SKIP
 *      This value specifies that the local copy should not be changed.
 */
/*! \var eUndoCheckOut_UpdatePolicy NXN_UNDOCHECKOUT_UPDATE_POLICY_REVERT
 *      This value specifies that the local copy should be reverted to the latest version on the server.
 */
enum eUndoCheckOut_UpdatePolicy
{
    NXN_UNDOCHECKOUT_UPDATE_POLICY_DEFAULT     = -1,
    NXN_UNDOCHECKOUT_UPDATE_POLICY_ASK         = 0,
    NXN_UNDOCHECKOUT_UPDATE_POLICY_SKIP        = 1,
    NXN_UNDOCHECKOUT_UPDATE_POLICY_REVERT      = 2
};


/*! \enum eUndoCheckOut_RevertUpdatePolicy
 *      These enumeration values specify the action that is performed with the local copy if an UndoCheckOut is done 
 *      on a changed file. The options only aply if the update policy is set to NXN_UNDOCHECKOUT_UPDATE_POLICY_REVERT.
 */
/*! \var eUndoCheckOut_RevertUpdatePolicy NXN_UNDOCHECKOUT_REVERT_UDPATE_POLICY_DEFAULT
 *      This value specifies that the default revert policy will be used. The default policy is the currently persistent setting in alienbrain.
 */
/*! \var eUndoCheckOut_RevertUpdatePolicy NXN_UNDOCHECKOUT_REVERT_UPDATE_POLICY_ASK
 *      This value specifies that the user should be asked.
 */
/*! \var eUndoCheckOut_RevertUpdatePolicy NXN_UNDOCHECKOUT_REVERT_UPDATE_POLICY_SKIP
 *      This value specifies that the local copy should not be reverted.
 */
/*! \var eUndoCheckOut_RevertUpdatePolicy NXN_UNDOCHECKOUT_REVERT_UPDATE_POLICY_REVERT
 *      This value specifies that the local copy should be reverted to the latest version on the server.
 */
enum eUndoCheckOut_RevertUpdatePolicy
{
    NXN_UNDOCHECKOUT_REVERT_UDPATE_POLICY_DEFAULT     = -1,
    NXN_UNDOCHECKOUT_REVERT_UPDATE_POLICY_ASK         = 0,
    NXN_UNDOCHECKOUT_REVERT_UPDATE_POLICY_SKIP        = 1,
    NXN_UNDOCHECKOUT_REVERT_UPDATE_POLICY_REVERT      = 2
};


//---------------------------------------------------------------------------
//  dependency types
//---------------------------------------------------------------------------
/*! \enum eDependencyType
 *      These enumerated dependency types are only for use with the methods
 *      DependencyAdd(), DependencyRemove(), DependencyClear(), GetFirstDependency() and
 *      GetNextDependency().
 */
enum eDependencyType
{
    /*! \var eDependencyType DET_INTERNAL   
     *      Internal Dependency.
     */
    DET_INTERNAL = 0,               // this type is the default

    /*! \var eDependencyType DET_EXTERNAL
     *      External (unresolved) dependency
     */
    DET_EXTERNAL,

    /*! \var eDependencyType DET_MANUAL 
     *      Manual dependency.
     */
    DET_MANUAL,

    /*! \var eDependencyType MAX_DEPENDENCY_TYPES
     *      There are two type of separators. If there's an opcode specified in a
     *      separator menu item, then you must treat this menu item as a "category"
     */
    MAX_DEPENDENCY_TYPES
};

//---------------------------------------------------------------------------
//  iterator object for dependencies
//---------------------------------------------------------------------------
typedef struct NXNINTEGRATORSDK_API CNxNDependencyIterator : public CNxNIterator
{
    long            m_lCurrentHandle;
    eDependencyType m_eType;

    virtual bool IsEnd() const;
} TNxNDependencyIterator;


//---------------------------------------------------------------------------
//  structure for references
//---------------------------------------------------------------------------
struct sNxNReference
{
    CNxNString  sUniqueIdentifier;
    CNxNString  sType;
    CNxNString  sName;
    CNxNString  sComment;
};

typedef CNxNArray<sNxNReference> TNxNReferenceList;


//---------------------------------------------------------------------------
//  hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNDbNodeData* tNxNDbNodeData;

class NXNINTEGRATORSDK_API CNxNDbNode : public CNxNNode
{
    NXN_DECLARE_DYNCREATE(CNxNDbNode);

    friend CNxNDbNodeList;
    friend CNxNGlobalSelection;

    public:
        //--------------------------------------------------------------------
        // construction / destruction
        //--------------------------------------------------------------------
        CNxNDbNode(CNxNNode* pParent = NULL);
        virtual ~CNxNDbNode();

        //--------------------------------------------------------------------
        // database node commands
        //--------------------------------------------------------------------
        bool Import(const CNxNString& strFileName, 
                    bool bShowDialog = true, 
                    const CNxNString& strComment = _STR(""), 
                    const CNxNString& strKeywords = _STR(""), 
                    bool bDeleteLocal = false, 
                    bool bDontCheckForExisting = true, 
                    bool bRunSilent = false,
                    const CNxNString& strImportDescription = _STR(""),
                    bool bCheckOut = false,
                    bool bGetLocalCopy = true,
                    bool bChangeSignOffStatus = false);

        bool ImportDbNode(const CNxNString& strFileName, 
                          bool bShowDialog = true, 
                          const CNxNString& strComment = _STR(""), 
                          const CNxNString& strKeywords = _STR(""), 
                          bool bDeleteLocal = false, 
                          bool bDontCheckForExisting = true, 
                          bool bRunSilent = false,
                          const CNxNString& strImportDescription = _STR(""),
                          bool bCheckOut = false,
                          bool bGetLocalCopy = true,
                          bool bChangeSignOffStatus = false);

        bool ImportCollection(CNxNPathCollection& pcCollection, 
                              bool bShowDialog = true, 
                              const CNxNString& strComment = _STR(""), 
                              const CNxNString& strKeywords = _STR(""), 
                              bool bDeleteLocal = false, 
                              bool bDontCheckForExisting = true, 
                              bool bRunSilent = false,
                              const CNxNString& strImportDescription = _STR(""),
                              bool bCheckOut = false,
                              bool bGetLocalCopy = true,
                              bool bChangeSignOffStatus = false);

        CNxNDbNode* ImportDiskItem(CNxNDiskItem*& pDiskItem, 
                                   bool bShowDialog = true, 
                                   const CNxNString& strComment = _STR(""), 
                                   const CNxNString& strKeywords = _STR(""), 
                                   bool bDeleteLocal = false, 
                                   bool bDontCheckForExisting = true, 
                                   bool bRunSilent = false,
                                   const CNxNString& strImportDescription = _STR(""),
                                   bool bCheckOut = false,
                                   bool bGetLocalCopy = true,
                                   bool bChangeSignOffStatus = false);

        bool ImportDiskItems(CNxNDiskItemList& dlDiskItems, 
                             bool bShowDialog = true, 
                             const CNxNString& strComment = _STR(""), 
                             const CNxNString& strKeywords = _STR(""), 
                             bool bDeleteLocal = false, 
                             bool bDontCheckForExisting = true, 
                             bool bRunSilent = false,
                             const CNxNString& strImportDescription = _STR(""),
                             bool bCheckOut = false,
                             bool bGetLocalCopy = true,
                             bool bChangeSignOffStatus = false);

        bool CheckIn(bool bShowDialog = true, 
                     const CNxNString& strFileName = _STR(""), 
                     const CNxNString& strComment = _STR(""), 
                     bool bKeepCheckedOut = false, 
                     bool bDeleteLocal = false,
                     bool bAskForEachObject = false, 
                     bool bApplyToReferences = false, 
                     eCheckIn_CreateVersionPolicy eCreateVersion = NXN_CHECKIN_CREATE_VERSION_DEFAULT, 
                     bool bRunSilent = false,
                     bool bChangeSignOffStatus = false);

        bool CheckOut(bool bShowDialog = true, 
                      const CNxNString& strComment = _STR(""), 
                      bool bDontGetLocal = false, 
                      bool bAskForEachObject = false, 
                      bool bApplyToReferences = false, 
                      const CNxNString& strCheckOutPath = _STR(""), 
                      bool bRunSilent = false,
                      bool bAllowMultipleCheckOut = true,
                      enum eGetLatest_FileTimePolicy eFTP = NXN_GETLATEST_FILETIME_DEFAULT);

        bool GetLatest(bool bShowDialog = true, 
                       bool bForceUpdate = false, 
                       bool bCreateDirStructureOnly = false, 
                       bool bAskForEachObject = false, 
                       bool bApplyToReferences = false, 
                       const CNxNString& strDestinationPath = _STR(""), 
                       bool bRunSilent = false,
                       bool bOnlyExisting = false,
                       enum eGetLatest_OverwriteWritablePolicy eOWP = NXN_GETLATEST_OVERWRITE_WRITABLE_DEFAULT,
                       enum eGetLatest_OverwriteCheckedOutPolicy eOCP = NXN_GETLATEST_OVERWRITE_CHECKEDOUT_DEFAULT,
                       enum eGetLatest_FileTimePolicy eFTP = NXN_GETLATEST_FILETIME_DEFAULT);

        bool SignOff(bool bShowDialog = true,
                     const CNxNString& sComment = _STR(""),
                     const CNxNString& sStatusSelection = _STR(""),
                     bool bAskForEachObject = false);

        bool Activate();

        bool Edit();

        bool Compile();

        bool Delete();

        bool DeleteLocalCopy();

        bool Rename(const CNxNString& strNewName, bool bRenameLocal = true, bool bRunSilent = false);

        bool UndoCheckOut(bool bShowDialog = true, 
                          bool bRunSilent = false,
                          enum eUndoCheckOut_UpdatePolicy eUpdatePolicy = NXN_UNDOCHECKOUT_UPDATE_POLICY_DEFAULT,
                          enum eUndoCheckOut_RevertUpdatePolicy eRUP = NXN_UNDOCHECKOUT_REVERT_UDPATE_POLICY_DEFAULT,
                          enum eGetLatest_FileTimePolicy eFTP = NXN_GETLATEST_FILETIME_DEFAULT);

//      bool Destroy();

        bool UndoMove(CNxNCommand& comCommand);
    
        bool DependencyAdd(CNxNNode& nodeReference, eDependencyType eType = DET_INTERNAL);

        bool DependencyClear(eDependencyType eType = DET_INTERNAL);

        bool DependencyRemove(CNxNNode& nodeReference, eDependencyType eType = DET_INTERNAL);

        //--------------------------------------------------------------------
        // unique identifiers and references
        //--------------------------------------------------------------------
		bool GetUniqueIdentifier(CNxNString& sUniqueIdentifier);

        bool AddReference(const CNxNString& sUniqueIdentifier,
                          const CNxNString& sType = _STR(L""),
                          const CNxNString& sName = _STR(L""),
                          const CNxNString& sComment = _STR(L""));
        bool AddReference(CNxNNode*         pRefNode,
                          const CNxNString& sType = _STR(L""),
                          const CNxNString& sName = _STR(L""),
                          const CNxNString& sComment = _STR(L""));

        int  GetReferenceCount(const CNxNString& sType);
        int  GetReferences(const CNxNString& sType,
                           int               iMaxRefCount,
                           sNxNReference*    pReferences);
        bool GetReferences(const CNxNString&  sType,
                           TNxNReferenceList& aReferences);
        bool GetFirstReference(const CNxNString& sType,
                               sNxNReference&    refFirst,
                               CNxNNode*&        pRefNode);
        bool GetNextReference(const CNxNString&    sType,
                              const sNxNReference& refPrevious,
                              sNxNReference&       refNext,
                              CNxNNode*&           pRefNode);

        bool DeleteReference(const CNxNString& sUniqueIdentifier,
                             const CNxNString& sType = _STR(L""));
        bool DeleteAllReferences(const CNxNString& sType = _STR(L""));


        //--------------------------------------------------------------------
        // database node information retrieval
        //--------------------------------------------------------------------
        virtual bool IsValid() const;

        virtual long GetStatusFlags() const;
        virtual CNxNString GetKeywords() const;
        virtual CNxNString GetComment() const;
        virtual CNxNString GetMimeType() const;
        virtual long GetHandle() const;

        virtual CNxNString GetWorkingPath() const;
        virtual bool SetWorkingPath(const CNxNString& strNewWorkingPath);

        virtual bool IsCheckedOut() const;
        virtual bool IsCheckedOutExclusive() const;
        virtual bool IsCheckedOutMultiple() const;
        virtual bool IsMultipleCheckOutEnabled() const;

        virtual long GetCheckOutUserCount() const;

        virtual bool IsCheckedOutByUser(const CNxNString& strUserName = _STR("")) const;
        virtual bool IsCheckedOutExclusiveByUser(const CNxNString& strUserName = _STR("")) const;

        virtual CNxNString GetFirstCheckOutUser() const;
        virtual CNxNString GetNextCheckOutUser(const CNxNString& strCurrentUser) const;
        virtual bool GetAllCheckOutUsers(TNxNStringList& aUserList) const;
        //virtual bool GetAllCheckOutUsers(CNxNString* aUsers, int iMaxUsers) const;

        virtual CNxNNode* GetFirstDependency(eDependencyType eType = DET_INTERNAL, TNxNDependencyIterator* pDependencyIterator = NULL) const;
        virtual CNxNNode* GetNextDependency(eDependencyType eType = DET_INTERNAL, TNxNDependencyIterator* pDependencyIterator = NULL) const;
        virtual bool GetDependencyList(CNxNNodeList* pNodeList, eDependencyType eType = DET_INTERNAL) const;

        //---------------------------------------------------------------------------
        //  project retrieval (parent)
        //---------------------------------------------------------------------------
        CNxNProject* GetProject() const;

        //---------------------------------------------------------------------------
        //  static object type information
        //---------------------------------------------------------------------------
        static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_DBNODE);
        }

        //---------------------------------------------------------------------------
        //  direct access to node object data
        //---------------------------------------------------------------------------
        inline tNxNDbNodeData GetDbNodeData() const
        {
            return m_pDbNodeData;
        }

    protected:
        void Reset();
        void SetDbNodeList(CNxNDbNodeList* pDbNodeList);
        bool IsInDbNodeList();

    private:
        tNxNDbNodeData  m_pDbNodeData;
};


#endif // INC_NXN_DBNODE_H