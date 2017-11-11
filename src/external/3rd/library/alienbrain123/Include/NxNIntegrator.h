// \addtodoc

#ifndef INC_NXN_INTEGRATOR_H
#define INC_NXN_INTEGRATOR_H

/*! \file       NxNIntegrator.h
 *  
 *  \brief      This file contains the definition for the class CNxNIntegrator and its associated types.
 *  
 *  \author     Axel Pfeuffer, Helmut Klinger
 *  
 *  \version    1.00
 *  
 *  \date       2000
 *  
 *  \mod
 *  \endmod
 */

#include <CommCtrl.h>

/*  \class      CNxNIntegrator NxNIntegrator.h
 *
 *  \brief      This class is the main entrance point for your application to use the SDK.
 *
 *  \author     Axel Pfeuffer, Helmut Klinger
 *
 *  \version    1.00
 *
 *  \date       2000
 *
 *  \mod
 *      [ap]-05-Feb-2000 file created.
 *      [dp]-31-Aug-2001 SetupWorkspace() removed because no implementation existed.
 *  \endmod
 */

/*! \enum eNxNImageListType
 *      These enumeration values define different types of image list. See CNxNIntegrator::GetImageList()
 *      and CNxNNode::GetIcon() for more information.
 */
/*! \var eNxNImageListType NXN_IMAGELIST_NORMAL 
 *      This value specifies the image list used for "normal" icons.
 */
/*! \var eNxNImageListType NXN_IMAGELIST_STATE
 *      This value specifies the image list used for state icons.
 */
/*! \var eNxNImageListType NXN_IMAGELIST_HEADER
 *      This value specifies the image list used for header icons.
 */
enum eNxNImageListType
{
    NXN_IMAGELIST_NORMAL,
    NXN_IMAGELIST_STATE,
    NXN_IMAGELIST_HEADER,
};

class CNxNItem;
class CNxNItemCollection;

class NXNINTEGRATORSDK_API CNxNIntegrator : public CNxNNode
{
    NXN_DECLARE_DYNCREATE(CNxNIntegrator);

    public:
        //--------------------------------------------------------------------
        // construction / destruction
        //--------------------------------------------------------------------
        CNxNIntegrator();
        virtual ~CNxNIntegrator();

        //--------------------------------------------------------------------
        // CNxNNode methods
        //--------------------------------------------------------------------
        virtual CNxNType GetNamespaceType() const;

        //--------------------------------------------------------------------
        // initialization/deinitialization of integrator environment
        //--------------------------------------------------------------------
        bool InitInstance();
        bool ExitInstance();

        void SetAutoCOMInit(bool bAutoCOMInit);
        bool SetupInitInstance(bool bShowInsertProjectDialog = true);

        //--------------------------------------------------------------------
        // workspace specific methods
        //--------------------------------------------------------------------

        CNxNWorkspace* CreateWorkspace(const CNxNString& strFileName = _STR(L""));
        CNxNWorkspace* OpenWorkspace(const CNxNString& strFileName = _STR(L""));

        bool SaveWorkspace(CNxNWorkspace* pWorkspace, const CNxNString& strFileName = _STR(L""));
        bool CloseWorkspace(CNxNWorkspace*& pWorkspace);

        CNxNWorkspace* GetWorkspace() const;

        bool GetItem(const CNxNString& sPath, CNxNItem& Item) const;
        bool GetGlobalSelection(CNxNItemCollection& Selection) const;

        //--------------------------------------------------------------------
        // version data
        //--------------------------------------------------------------------
        bool GetDllVersion(DWORD& dwMajorVersion, DWORD& dwMinorVersion, DWORD& dwBuildNumber);

        /*! \fn         bool CNxNIntegrator::GetNeededVersion(DWORD& dwMajorVersion, DWORD& dwMinorVersion, DWORD& dwBuildNumber)
         *  
         *              This method returns the necessary version of the NxN Integrator SDK dll file.
         *  
         *  \param      dwMajorVersion  reference to a dword receiving the major version number
         *  \param      dwMinorVersion  reference to a dword receiving the minor version number
         *  \param      dwBuildNumber   reference to a dword receiving the build number
         *  
         *  \return     boolean value indicating if the version number was successfully determined
         *  
         *  \note       
         */
        inline bool GetNeededVersion(DWORD& dwMajorVersion, DWORD& dwMinorVersion, DWORD& dwBuildNumber)
        {
            dwMajorVersion = VERSION_MAJOR;
            dwMinorVersion = VERSION_MINOR;
            dwBuildNumber = VERSION_BUILDNUMBER;

            return true;
        }

        //--------------------------------------------------------------------
        // data retrieval
        //--------------------------------------------------------------------
        CNxNString GetInstalledAppName();

        virtual bool IsValid() const;

        //--------------------------------------------------------------------
        // global selection specific methods
        //--------------------------------------------------------------------
        CNxNGlobalSelection* CreateGlobalSelection();
        CNxNGlobalSelection* GetGlobalSelection() const;
        bool DestroyGlobalSelection();
        bool HasGlobalSelection() const;

        //---------------------------------------------------------------------------
        //  event manager specific methods
        //---------------------------------------------------------------------------
        CNxNEventManager* GetEventManager() const;

        //---------------------------------------------------------------------------
        //  global window handle
        //---------------------------------------------------------------------------
        void SetGlobalHwnd(long hWnd);

        //---------------------------------------------------------------------------
        //  global namespace image lists
        //---------------------------------------------------------------------------
        HIMAGELIST GetImageList(eNxNImageListType listType);

        //---------------------------------------------------------------------------
        //  static object type information
        //---------------------------------------------------------------------------
        static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_INTEGRATOR);
        };

        //---------------------------------------------------------------------------
        //  DEBUGGING!!!!!
        //---------------------------------------------------------------------------
        void PrintTree()
        {
            _OutputNode(this, 0);
            printf("------------------------\n\n");
        }

        inline void _OutputNode(CNxNNode* pNode, int nLevel)
        {
            for (int i = 0; i < nLevel; i++)
            {
                printf ("\t");
            }
            
            // output node name
            printf("%s\n", (LPCSTR)(pNode->GetName()));

            // child has nodes?
            if (pNode->HasInternalChildren())
            {
                int nSize = pNode->GetInternalChildCount();

                for (int nCount = 0; nCount < nSize; nCount++)
                {
                    _OutputNode(pNode->GetInternalChild(nCount), nLevel+1);
                }
            }
        }


    private:
        bool                    m_bInitialized;
        bool                    m_bAutoCOMInit;

        CNxNWorkspace*          m_pCurrentWorkspace;
        CNxNGlobalSelection*    m_pGlobalSelection;

        int                     m_eUseApi;

        bool                    m_bShowInsertProjectDialog;

        CNxNEventManager*       m_pEventManager;
};

#endif // INC_NXN_INTEGRATOR_H
