// \addtodoc

#ifndef INC_NXN_WORKSPACE_H
#define INC_NXN_WORKSPACE_H
/*!	\file		NxNWorkspace.h
 *	
 *	\brief		This file contains the class definition for CNxNWorkspace.
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

/* \class       CNxNWorkspace
 *
 *  \file       NxNWorkspace.h
 *
 *  \brief      CNxNWorkspace represents the global workspace. 
 *
 *  \author     Axel Pfeuffer, Helmut Klinger
 *
 *  \version    1.00
 *
 *  \date       2000
 *
 *  \mod
 *      [ap]-02-May-2000 file created.
 *      [jr]-18-Dec-2000 Internationalization
 *  \endmod
 */

//
// This file was changed during internationalization on 12/18/2000 by Jens.


/*!	\enum eNxNTimeSyncPolicy
 *		These values describe different time synchronization policies used when connecting to a server. 
 */
/*!	\var eNxNTimeSyncPolicy NXN_TIMESYNC_POLICY_NEVER	
 *		Do not synchronize with server time.
 */
/*!	\var eNxNTimeSyncPolicy NXN_TIMESYNC_POLICY_ASK	
 *		You will be asked if you want to synchronize when the time difference is greater than 15 seconds.
 */
/*!	\var eNxNTimeSyncPolicy NXN_TIMESYNC_POLICY_MAXDELTA	
 *		The times will be synchronized if the difference is greater then 15 seconds.
 */
/*!	\var eNxNTimeSyncPolicy NXN_TIMESYNC_POLICY_ALWAYS	
 *		The times will always be synchronized.
 */
enum eNxNTimeSyncPolicy
{
    NXN_TIMESYNC_POLICY_NEVER,
    NXN_TIMESYNC_POLICY_ASK,
    NXN_TIMESYNC_POLICY_MAXDELTA,
    NXN_TIMESYNC_POLICY_ALWAYS
};

//---------------------------------------------------------------------------
//  hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNWorkspaceData*  tNxNWorkspaceData;

class NXNINTEGRATORSDK_API CNxNWorkspace : public CNxNVirtualNode
{
    NXN_DECLARE_DYNCREATE(CNxNWorkspace);

    public:
        //--------------------------------------------------------------------
        // construction / destruction
        //--------------------------------------------------------------------
        CNxNWorkspace(CNxNIntegrator* pNxNIntegrator = NULL);
        virtual ~CNxNWorkspace();

        //--------------------------------------------------------------------
        // project specific methods
        //--------------------------------------------------------------------
        CNxNProject* NewProject();

        CNxNProject* NewProjectEx(const CNxNString& strProjectName,
                                  const CNxNString& strPath, 
                                  const CNxNString& strUserName,
                                  const CNxNString& strPassword, 
                                  const CNxNString& strHostName,
                                  const CNxNString& strMountPath = _STR(""),
                                  const CNxNString& strBuildPath = _STR(""),
                                  const CNxNString& strWorkingPath = _STR(""));

        CNxNProject* InsertProject(const CNxNString& strFileName = _STR(""),
                                   const CNxNString& strUserName = _STR(""), 
                                   const CNxNString& strPassword = _STR(""),
								   const CNxNString& strHostName = _STR(""));

        CNxNProject* InsertProjectEx(const CNxNString& strFileName,
									 const CNxNString& strUserName,
									 const CNxNString& strPassword, 
                                     const CNxNString& strHostName);

        CNxNProject* LoadProject(const CNxNString& strProjectName);

        CNxNProject* LoadProjectAs(const CNxNString& strProjectName);

        CNxNProject* LoadProjectEx(const CNxNString& strProjectName,
								   const CNxNString& strUserName,
								   const CNxNString& strPassword, 
                                   const CNxNString& strHostName,
								   bool bAllowDialogs = true,
								   eNxNTimeSyncPolicy eTimeSyncPolicy = NXN_TIMESYNC_POLICY_ASK);

        bool UnloadProject(CNxNProject*& pProject); 


        bool RemoveProject(CNxNProject*& pProject, bool bShowDialog = true);

        CNxNProject* GetProject(const CNxNString& strProjectName);

        //--------------------------------------------------------------------
        // workspace information access methods
        //--------------------------------------------------------------------
        CNxNString GetFileName() const;

        //---------------------------------------------------------------------------
        //  project browsing
        //---------------------------------------------------------------------------
        CNxNProject* BrowseForProject();

        //--------------------------------------------------------------------
        // data retrieval
        //--------------------------------------------------------------------
        bool IsOpened() const;

        //---------------------------------------------------------------------------
        //  static object type information
        //---------------------------------------------------------------------------
        static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
		{ 
			return CNxNType(NXN_TYPE_WORKSPACE);
		}

        //--------------------------------------------------------------------
        // direct access to workspace data
        //--------------------------------------------------------------------
        inline tNxNWorkspaceData GetWorkspaceData()
		{
			return m_pWorkspaceData;
		}

    protected:
        //--------------------------------------------------------------------
        // overloaded methods
        //--------------------------------------------------------------------
        void SetParent(CNxNNode* pNewParent);


    private:
        tNxNWorkspaceData   m_pWorkspaceData;
};

#endif // INC_NXN_WORKSPACE_H
