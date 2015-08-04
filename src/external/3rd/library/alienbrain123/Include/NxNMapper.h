// \addtodoc

#ifndef INC_NXN_MAPPER_H
#define INC_NXN_MAPPER_H

/*!	\file		NxNMapper.h
 *	
 *	\brief		This file contains the definition for the class CNxNMapper and its associated types.
 *	
 *	\author		Gregor vom Scheidt
 *	
 *	\version	1.00
 *	
 *	\date		2000
 *	
 *	\mod
 *	\endmod
 */

/* \class       CNxNMapper
 *
 *	\file		NxNMapper.h
 *
 *  \brief      The CNxNMapper class implements a "front door"
 *              to the Integrator SDK that is useful for all
 *              applications that want to use the SDK to access
 *              and manage files stored in the system in an
 *              intelligent way.
 *
 *  \author     Gregor vom Scheidt
 *
 *  \version    1.00
 *
 *  \date       2000
 *
 *  \mod
 *      [gs]-26-Aug-2000 file created.
 *      [gs]-12-Sep-2000 Added doxygen tags
 *      [gs]-12-Oct-2000 Added GetManagedPaths() and RefreshManagedPaths()
 *      [gs]-12-Nov-2000 Extended to handle lower-level working paths correctly.
 *      [ap]-30-Apr-2001 Enum eInit_ConnectPolicy introduced.
 *      [dp]-22-Jun-2001 Added GetConnectPolicy()
 *  \endmod
 */

//---------------------------------------------------------------------------
//  hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNMapperData* tNxNMapperData;

class CNxNIntegrator;
class CNxNNode;
class CNxNFile;
class CNxNWorkspace;
class CNxNString;

#include "./NxNArray.h"     // template CNxNArray<ItemType>


struct NXNINTEGRATORSDK_API CNxNMapperInfo
{
    CNxNString  m_sProjectName;     // the project name
    CNxNString  m_sWorkingPath;     // the managed path for the currently active branch and the last logged on user.
    CNxNString  m_sLogonName;       // the user's log-on name
    CNxNString  m_sServerName;      // the server's computer name
    bool        m_bUnloaded;        // if 'true', project is unloaded
};

/*! \enum eInit_ConnectPolicy
 *      These enumeration values are used with the CNxNMapper::InitInstance() and CNxNSmartIntegrator::InitInstance()
 *      methods to give the user control over the "work offline" dialog.
 */
/*! \var eInit_ConnectPolicy NXN_CONNECT_DEFAULT    
 *      This value indicates that the default settings of the "connect or work offline?" dialog will be used.
 *      This means when the hide checkbox in the dialog is ticked, the dialog will never appear again as long as the
 *      SHIFT key is not pressed.
 */
/*! \var eInit_ConnectPolicy NXN_CONNECT_SHOW_DIALOG_ALWAYS 
 *      This value forces the "connect or work offline?" dialog to pop up everytime.
 */
/*! \var eInit_ConnectPolicy NXN_CONNECT_HIDE_DIALOG_AND_CONNECT    
 *      This value hides the "connect or work offline?" dialog and automatically establishes a connection.
 */
/*! \var eInit_ConnectPolicy NXN_CONNECT_SHOW_DIALOG_WHEN_OFFLINE
 *      This value shows the "connect or work offline?" dialog if the user chose to work offline last time.
 */
enum eInit_ConnectPolicy
{
    NXN_CONNECT_DEFAULT                     = 0,
    NXN_CONNECT_SHOW_DIALOG_ALWAYS          = 1,
    NXN_CONNECT_HIDE_DIALOG_AND_CONNECT     = 2,
    NXN_CONNECT_SHOW_DIALOG_WHEN_OFFLINE    = 3
};


class NXNINTEGRATORSDK_API CNxNMapper : public CNxNObject
{
    NXN_DECLARE_DYNCREATE(CNxNMapper);

    //--------------------------------------------------------------------
    // Member variables
    //--------------------------------------------------------------------
    private:
        tNxNMapperData  m_pData;

    //--------------------------------------------------------------------
    // construction / destruction
    //--------------------------------------------------------------------
    public:
        CNxNMapper();
        virtual ~CNxNMapper();

    //--------------------------------------------------------------------
    // initialization/deinitialization of smart workspace
    //--------------------------------------------------------------------
    public:
        bool InitInstance(CNxNIntegrator* pIntegrator = NULL,
                          eInit_ConnectPolicy eConnectPolicy = NXN_CONNECT_DEFAULT);
        bool InitInstance(CNxNIntegrator* pIntegrator, bool bShowDialog);
        bool ExitInstance();

        //--------------------------------------------------------------------
        // for changing the question text in the connection dialog
        //--------------------------------------------------------------------
        void SetConnectionText(const CNxNString& strConnectionText);

    //--------------------------------------------------------------------
    // access to CNxNIntegrator interface
    //--------------------------------------------------------------------
    public:
        CNxNIntegrator* GetIntegrator(eInit_ConnectPolicy eConnectPolicy = NXN_CONNECT_DEFAULT);

        //--------------------------------------------------------------------
        // return the connection policy set with InitInstance
        //--------------------------------------------------------------------
        eInit_ConnectPolicy GetConnectPolicy();

    //--------------------------------------------------------------------
    // map managed paths
    //--------------------------------------------------------------------
    public:
        bool MapManagedPath(const CNxNString& sLocalPath,
                            CNxNString& sNamespacePath);
        CNxNNode* GetManagedNode(const CNxNString& sNamespacePath);
        CNxNFile* MapAndGetManagedFile(const CNxNString& sLocalPath);

    //--------------------------------------------------------------------
    // diagnostics and user information
    //--------------------------------------------------------------------
    public:
        int GetManagedPaths(CNxNArray<CNxNMapperInfo>& aMapperInfo);
        bool RefreshManagedPaths(void);

    //--------------------------------------------------------------------
    // access to window handle's
    //--------------------------------------------------------------------
    public:
        void SetParentWindow(long hWnd);

};

#endif // INC_NXN_MAPPER_H
