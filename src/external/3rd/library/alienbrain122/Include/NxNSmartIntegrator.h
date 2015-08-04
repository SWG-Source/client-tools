// \addtodoc
#ifndef INC_NXN_SMARTINTEGRATOR_H
#define INC_NXN_SMARTINTEGRATOR_H

/* \class		CNxNSmartIntegrator NxNSmartIntegrator.h
 *
 *  \brief      CNxNSmartIntegartor is a helper class that provides a simple and convenient 
 *				way to integrate the functionality of alienbrain into your application.
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.0
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-05-Sep-2000 file created.
 *	\endmod
 */

class NXNINTEGRATORSDK_API CNxNSmartIntegrator : public CNxNObject
{
	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNSmartIntegrator();
		virtual ~CNxNSmartIntegrator();

		//---------------------------------------------------------------------------
		//	for initialization/deinitialization
		//---------------------------------------------------------------------------
		bool InitInstance(bool bShowDialog, const CNxNString& strConnectionText = _STR(""));
		bool InitInstance(eInit_ConnectPolicy eConnectPolicy = NXN_CONNECT_DEFAULT,
                          const CNxNString& strConnectionText = _STR(""));
		bool ExitInstance();

		//---------------------------------------------------------------------------
		//	for availability checks
		//---------------------------------------------------------------------------
		bool ExistsInDatabase(const CNxNString& strLocalFilePath);
		bool FileExistsInDatabase(const CNxNString& strLocalFilePath);
		bool FileIsManageable(const CNxNString& strLocalFilePath);

		bool FileExistsOnLocalHardDisk(const CNxNString& strLocalFilePath);

		//---------------------------------------------------------------------------
		//	for database checks
		//---------------------------------------------------------------------------
		bool FileCheckedOutByUser(const CNxNString& strLocalFilePath);
		bool FileCheckedOutByAnotherUser(const CNxNString& strLocalFilePath);
		bool FileLockedOnUserMachine( const CNxNString& strLocalFilePath);

		//---------------------------------------------------------------------------
		//	for database checks relating versioning
		//---------------------------------------------------------------------------
		bool NewerFileOnServer(const CNxNString& strLocalFilePath);

		//---------------------------------------------------------------------------
		//	node retrieval functions
		//---------------------------------------------------------------------------
		CNxNNode* GetManagedNode(const CNxNString& strNamespacePath, const CNxNType& tType);

		//---------------------------------------------------------------------------
		//	file time specific operations
		//---------------------------------------------------------------------------
		CNxNString GetModificationTimeString(const CNxNString& strLocalFilePath);
		DWORD GetFileSize(const CNxNString& strLocalFilePath);

		//---------------------------------------------------------------------------
		//	filename specific operations
		//---------------------------------------------------------------------------
		bool MapManagedPath(const CNxNString& strLocalFilePath, CNxNString& strNamespacePath) const;

		CNxNString GetFileNameOnly(const CNxNString& strLocalFilePath) const;
		CNxNString GetPathOnly(const CNxNString& strLocalFilePath) const;

		//---------------------------------------------------------------------------
		//	direct object access
		//---------------------------------------------------------------------------
		CNxNMapper*	GetMapper() const
		{
			return m_pMapper;
		}

		CNxNIntegrator* GetIntegrator(eInit_ConnectPolicy eConnectPolicy = NXN_CONNECT_DEFAULT) const;
		
		//---------------------------------------------------------------------------
		//	for timed object data flushing
		//---------------------------------------------------------------------------
		bool FlushIfRequired() const;

		/*!	\fn			void CNxNSmartIntegrator::SetParentWindow(long hWnd)
		 *	
		 *				Sets the handle for the parent window.
		 *	
		 *	\param		hWnd    long value containing the handle of the parent window
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		inline void SetParentWindow(long hWnd)
		{
			m_hWnd = hWnd;
		}

	private:
		CNxNMapper*			m_pMapper;
		CNxNIntegrator*		m_pIntegrator;
		long				m_hWnd;
};

#endif // INC_NXN_SMARTINTEGRATOR_H
