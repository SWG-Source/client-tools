// \addtodoc
#ifndef INC_NXN_SMARTINTEGRATOR_H
#define INC_NXN_SMARTINTEGRATOR_H

/* \class		CNxNSmartIntegrator NxNSmartIntegrator.h
 *
 *  \brief		This contains a plug-in independent code base which can be used
 *				to implement a basic functionality for almost all plug-in types.
 *				This class is meant to be used internally only!
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.0
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-09-05-2000 file created.
 *	\endmod
 */

class NXNINTEGRATORSDK_API CNxNSmartIntegrator : public CNxNObject {
	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNSmartIntegrator();
		virtual ~CNxNSmartIntegrator();

		//---------------------------------------------------------------------------
		//	for initialization/deinitialization
		//---------------------------------------------------------------------------
		bool InitInstance(bool bShowDialog = true, const CNxNString& strConnectionText = _STR(""));
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
		CNxNMapper*	GetMapper() const { return m_pMapper; };
		CNxNIntegrator* GetIntegrator() const;
		
		//---------------------------------------------------------------------------
		//	for timed object data flushing
		//---------------------------------------------------------------------------
		bool FlushIfRequired() const;

		/*!	\fn			void CNxNSmartIntegrator::SetParentWindow(long hWnd)
		 *	
		 *				Sets the handle for the parent window.
		 *	
		 *	\param		hWnd is a long containin the window handle to the parent.
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		inline void SetParentWindow(long hWnd) { m_hWnd = hWnd; };

	private:
		CNxNMapper*			m_pMapper;
		CNxNIntegrator*		m_pIntegrator;
		long				m_hWnd;
};

#endif // INC_NXN_SMARTINTEGRATOR_H
