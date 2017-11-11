// \addtodoc

#ifndef INC_NXN_DBFOLDER_H
#define INC_NXN_DBFOLDER_H

/*! \file       NxNDbFolder.h
 *  
 *  \brief      This file contains the definition for the class CNxNDbFolder and its associated types.
 *  
 *  \author     Axel Pfeuffer, Helmut Klinger
 *  
 *  \version    1.00
 *  
 *  \date       2001
 *  
 *  \mod
 *  \endmod
 */

/* \class       CNxNDbFolder NxNDbFolder.h
 *
 *  \brief      This is the base class for all folders contained in the project database. 
 *
 *              A database folder can typically be a CNxNFolder or a CNxNProject object.
 *
 *  \author     Axel Pfeuffer, Helmut Klinger
 *
 *  \version    1.0
 *
 *  \date       2001
 *
 *  \mod
 *      [ap]-19-Apr-2001 file created.
 *  \endmod
 */

// overwrite policies for GetVersionByTime
/*! \enum eNxNVersionOverwritePolicy
 *      These enumeration values define different overwrite policies for existing files when calling GetVersionByTime()
 */
/*! \var eNxNVersionOverwritePolicy NXN_VC_OVERWRITE_POLICY_ASK 
 *      This value specifies that a dialog box comes up asking the user if the existing file should be kept or overwritten.
 */
/*! \var eNxNVersionOverwritePolicy NXN_VC_OVERWRITE_POLICY_ALWAYS    
 *      This value specifies that existing files are always overwritten.
 */
/*! \var eNxNVersionOverwritePolicy NXN_VC_OVERWRITE_POLICY_NEVER
 *      This value specifies that existing files are always kept and never overwritten.
 */
enum eNxNVersionOverwritePolicy
{
    NXN_VC_OVERWRITE_POLICY_ASK = 0,
    NXN_VC_OVERWRITE_POLICY_ALWAYS,
    NXN_VC_OVERWRITE_POLICY_NEVER
};


//---------------------------------------------------------------------------
//  class pre-definitions
//---------------------------------------------------------------------------
class CNxNFolder;

class NXNINTEGRATORSDK_API CNxNDbFolder : public CNxNDbNode
{
    NXN_DECLARE_DYNCREATE(CNxNDbFolder);

    public:
        //---------------------------------------------------------------------------
        //  construction/destruction
        //---------------------------------------------------------------------------
        CNxNDbFolder(CNxNNode* pParent = NULL);
        virtual ~CNxNDbFolder();

        //---------------------------------------------------------------------------
        //  folder & file browsing
        //---------------------------------------------------------------------------
        CNxNFolder* BrowseForFolder();
        CNxNFile* BrowseForFile();

        //---------------------------------------------------------------------------
        //  folder & file access
        //---------------------------------------------------------------------------
        CNxNFolder* GetFolder(const CNxNString& strName);
        CNxNFile* GetFile(const CNxNString& strName);

        CNxNFolder* CreateFolder(const CNxNString& strFolderName, 
                                 bool bShowDialog = true,
                                 const CNxNString& strComment = _STR(L""),
                                 const CNxNString& strKeywords = _STR(L""),
                                 bool bFailIfExists = true,
                                 const CNxNString& strDescription = _STR(L""),
                                 bool bGetLocalCopy = true);


        /*! \fn         inline CNxNFile* CNxNDbFolder::CreateFile(const CNxNString& strFileName,
                                                                  const CNxNString& strComment = _STR(L""),
                                                                  const CNxNString& strKeywords = _STR(L""),
                                                                  const CNxNString& strLocalPath = _STR(L""))
         *  
         *              Creates a file in the database and returns a CNxNFile pointer to the created child object.
         *  
         *  \param      strFileName     string containing the name of the file to create
         *  \param      strComment      string containing the comment for the new file
         *  \param      strKeywords     string containing a list of keywords for the file
         *  \param      strLocalPath    string containing the local path of the file to create
         *  
         *  \return     pointer to a CNxNFile object.
         *              If the returned value is NULL, the specified file could not be created. 
         *              For more detailed information use GetLastError() or GetLastErrorMessage().
         *  
         *  \note       
         */
        inline CNxNFile* CreateFile(const CNxNString& strFileName, 
                                    const CNxNString& strComment = _STR(L""),
                                    const CNxNString& strKeywords = _STR(L""),
                                    const CNxNString& strLocalPath = _STR(L""))
        {
            return (CNxNFile*)CreateNode(strFileName,
                                         CNxNFile::GetType(),
                                         true,
                                         strComment,
                                         strKeywords,
                                         strLocalPath,
                                         false,
                                         false);
        }

        bool Delete(CNxNDbNode*& rpDbNode, bool bRunSilent = false);
        bool RemoveFromControl(CNxNDbNode*& rpDbNode, bool bRunSilent = false);

        bool GetVersionByTime(const CNxNTime& tTimeStamp,
                              bool bIsLocalTime = true,
                              bool bShowDialog = true,
                              const CNxNString& sDestDir = _STR(L""),
                              eNxNVersionOverwritePolicy eOverwritePolicy = NXN_VC_OVERWRITE_POLICY_ASK,
                              eNxNVersionOverwritePolicy eOverwritePolicyWriteable = NXN_VC_OVERWRITE_POLICY_ASK,
                              bool bRunSilent = false);

        bool RollbackByTime(const CNxNTime& tTimeStamp,
                            bool bIsLocalTime = true,
                            bool bShowDialog = true,
                            const CNxNString& sComment = _STR(L""),
                            bool bGetLocalCopy = true,
                            bool bRestoreProps = true,
                            bool bRunSilent = false);

        //---------------------------------------------------------------------------
        //  static object type information
        //---------------------------------------------------------------------------
        static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_DBFOLDER);
        }

    private:
};

#endif // INC_NXN_DBFOLDER_H
