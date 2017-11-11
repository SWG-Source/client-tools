// \addtodoc

#ifndef INC_NXN_PROJECT_H
#define INC_NXN_PROJECT_H

/* \class       CNxNProject NxNProject.h
 *
 *  \brief      The class CNxNProject represents projects that reside in the database on a server.
 *
 *  \author     Axel Pfeuffer, Helmut Klinger
 *
 *  \version    1.00
 *
 *  \date       2000
 *
 *  \mod
 *      [ap]-02-May-2000 file created.
 *      [ap]-13-Jun-2000 GetFile() implemented.
 *      [ap]-13-Jun-2000 GetFolder() implemented.
 *      [ap]-13-Jun-2000 CreateFile() implemented.
 *      [ap]-13-Jun-2000 CreateFolder() implemented.
 *      [ap]-08-Sep-2000 BUGFIX: CreateFolder() now works.
 *      [jr]-18-Dec-2000 Internationalization
 *      [ap]-19-Apr-2001 MODIFICATION: Base class changed to new CNxNDbFolder class.
 *      [ap]-20-Apr-2001 CreateFile() moved to CNxNDbFolder.
 *      [ap]-20-Apr-2001 CreateFolder() moved to CNxNDbFolder.
 *      [ap]-20-Apr-2001 BrowseForFolder() moved to CNxNDbFolder.
 *      [ap]-20-Apr-2001 BrowseForFile() moved to CNxNDbFolder.
 *      [ap]-20-Apr-2001 GetForFolder() moved to CNxNDbFolder.
 *      [ap]-20-Apr-2001 GetForFile() moved to CNxNDbFolder.
 *  \endmod
 */

//
// This file was changed during internationalization on 12/18/2000 by Jens.


//---------------------------------------------------------------------------
//  label info and list object types
//---------------------------------------------------------------------------
class CNxNLabelInfo
{
public:
    CNxNLabelInfo()
    {
        m_sName        = _STR("");
        m_sDescription = _STR("");
        m_sUser        = _STR("");
        m_tTimeStamp   = CNxNTime();
    }

    CNxNLabelInfo(const CNxNLabelInfo& other)
    {
        m_sName         = other.m_sName;
        m_sDescription  = other.m_sDescription;
        m_sUser         = other.m_sUser;
        m_tTimeStamp    = other.m_tTimeStamp;
    }

    virtual ~CNxNLabelInfo()
    {
    }

    CNxNString  m_sName;
    CNxNString  m_sDescription;
    CNxNString  m_sUser;
    CNxNTime    m_tTimeStamp;
};

typedef CNxNLabelInfo TNxNLabelInfo;

typedef CNxNArray<TNxNLabelInfo> TNxNLabelList;


//---------------------------------------------------------------------------
//  hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNProjectData*    tNxNProjectData;

class NXNINTEGRATORSDK_API CNxNProject : public CNxNDbFolder
{
    NXN_DECLARE_DYNCREATE(CNxNProject);

    public:
        //---------------------------------------------------------------------------
        //  construction/destruction
        //---------------------------------------------------------------------------
        CNxNProject(CNxNWorkspace* pParent = NULL);
        virtual ~CNxNProject();

        //---------------------------------------------------------------------------
        //  project information retrieval
        //---------------------------------------------------------------------------
        bool SetUserName(const CNxNString& strUserName);
        bool SetPassword(const CNxNString& strPassword);
        bool SetHostName(const CNxNString& strHostName);

        CNxNString GetUserName() const;
        CNxNString GetHostName() const;
        bool IsConnected() const;

        bool Connect(const CNxNString& strUserName = _STR(""), 
                     const CNxNString& strPassword = _STR(""), 
                     const CNxNString& strHostName = _STR(""),
                     bool bAllowDialogs = true,
                     eNxNTimeSyncPolicy eTimeSyncPolicy = NXN_TIMESYNC_POLICY_ASK);

        bool Disconnect();

        //---------------------------------------------------------------------------
        //  dbnode, file & folder retrieval using the corresponding project specific handle
        //---------------------------------------------------------------------------
        virtual CNxNNode* GetNodeFromHandle(long lHandle) const;
        CNxNDbNode* GetDbNodeFromHandle(long lHandle) const;
        CNxNFile* GetFileFromHandle(long lHandle) const;
        CNxNFolder* GetFolderFromHandle(long lHandle) const;


        //---------------------------------------------------------------------------
        //  version control: labels
        //---------------------------------------------------------------------------
        bool AddLabel(const CNxNString& sLabelName, 
                      bool bShowDialog = true,
                      const CNxNString& sLabelDescription = _STR(""), 
                      const CNxNTime& tTimeStamp = CNxNTime(),
                      bool bIsLocalTime = true,
                      bool bRunSilent = false);

        bool EnumLabels(TNxNLabelList& aLabelList);

        //---------------------------------------------------------------------------
        //  user management: get users for this project
        //---------------------------------------------------------------------------
        CNxNUserGroup* GetUsers();

        //---------------------------------------------------------------------------
        //  static object type information
        //---------------------------------------------------------------------------
        static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_PROJECT);
        }

        //---------------------------------------------------------------------------
        //  direct access to node object data
        //---------------------------------------------------------------------------
        inline tNxNProjectData GetProjectData() const
        {
            return m_pProjectData;
        }

    protected:
//      void Reset();

    private:
        tNxNProjectData m_pProjectData;
};


#endif // INC_NXN_PROJECT_H
