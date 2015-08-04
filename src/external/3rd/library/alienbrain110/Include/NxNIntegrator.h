// \addtodoc

#ifndef INC_NXN_INTEGRATOR_H
#define INC_NXN_INTEGRATOR_H

/*  \class		CNxNIntegrator NxNIntegrator.h
 *
 *  \brief		This class is the main important class for developing plug-ins and stand-alone
 *				applications using alienbrain functionality. It is the very main entry point and
 *				the connection to alienbrain. You can not use the IntegratorSDK without having
 *				a CNxNIntegrator object correctly instantiated and initialized.
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-05-02-2000 file created.
 *	\endmod
 */

//---------------------------------------------------------------------------
//	hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNIntegratorData*	tNxNIntegratorData;

class NXNINTEGRATORSDK_API CNxNIntegrator : public CNxNNode {
	NXN_DECLARE_DYNCREATE(CNxNIntegrator);

	public:
		//--------------------------------------------------------------------
		// construction / destruction
		//--------------------------------------------------------------------
		CNxNIntegrator();
		virtual ~CNxNIntegrator();

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
		bool SetupWorkspace(bool bShowDialog = true);

		CNxNWorkspace* CreateWorkspace(const CNxNString& strFileName);
		CNxNWorkspace* OpenWorkspace(const CNxNString& strFileName);

		bool SaveWorkspace(CNxNWorkspace* pWorkspace, const CNxNString& strFileName = _STR(""));
		bool CloseWorkspace(CNxNWorkspace*& pWorkspace);

		CNxNWorkspace* GetWorkspace() const;

		//--------------------------------------------------------------------
		// version data
		//--------------------------------------------------------------------
		bool GetDllVersion(DWORD& dwMajorVersion, DWORD& dwMinorVersion, DWORD& dwBuildNumber);

		/*!	\fn			bool CNxNIntegrator::GetNeededVersion(DWORD& dwMajorVersion, DWORD& dwMinorVersion, DWORD& dwBuildNumber)
		 *	
		 *				This method returns the needed version of the NxNIntegratorSDK.dll file.
		 *	
		 *	\param		dwMajorVersion a reference to the major version number.
		 *	\param		dwMinorVersion a reference to the minor version number.
		 *	\param		dwBuildNumber a reference to the build number
		 *	
		 *	\return		none
		 *	
		 *	\note		
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

		//--------------------------------------------------------------------
		// global selection specific methods
		//--------------------------------------------------------------------
		CNxNGlobalSelection* CreateGlobalSelection();
		CNxNGlobalSelection* GetGlobalSelection() const;
		bool DestroyGlobalSelection();
		bool HasGlobalSelection() const;

		//---------------------------------------------------------------------------
		//	event manager specific methods
		//---------------------------------------------------------------------------
		CNxNEventManager* GetEventManager() const;

		//---------------------------------------------------------------------------
		//	Global window handle
		//---------------------------------------------------------------------------
        void SetGlobalHwnd(long hWnd);

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType() { return CNxNType(NXN_TYPE_INTEGRATOR); };

		//---------------------------------------------------------------------------
		//	DEBUGGING!!!!!
		//---------------------------------------------------------------------------
		void PrintTree() { _OutputNode(this, 0); printf("------------------------\n\n"); };
		inline void _OutputNode(CNxNNode* pNode, int nLevel)
		{
			for (int i=0; i<nLevel; i++) {
				printf ("\t");
			}
			
			// output node name
			printf("%s\n", (LPCSTR)(pNode->GetName()));

			// child has nodes?
			if (pNode->HasInternalChilds()) {
				int nSize = pNode->GetInternalChildCount();

				for (int nCount=0; nCount<nSize; nCount++) {
					_OutputNode(pNode->GetInternalChild(nCount), nLevel+1);
				}
			}
		}

		//--------------------------------------------------------------------
		// access internal integrator data
		//--------------------------------------------------------------------
		inline tNxNIntegratorData GetIntegratorData() const { return m_pIntegratorData; };

	private:
		tNxNIntegratorData	m_pIntegratorData;
};

#endif // INC_NXN_INTEGRATOR_H
