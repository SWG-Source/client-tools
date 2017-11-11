// \addtodoc

#ifndef INC_NXN_MAPPER_H
#define INC_NXN_MAPPER_H

/* \class		CNxNMapper CNxNMapper.h
 *
 *  \brief		The CNxNMapper class imlpements a "front door"
 *				to the integrator SDK that is useful for all
 *				applications that want to use the SDK to access
 *				and manage files stored in the system in an
 *				intelligent way.
 *
 *				The class uses the list of local working paths
 *				cached in the project list in the registry to 
 *				determine if a file or folder is potentially
 *				"managed" (i.e. under version control).
 *				It then helps you map the local path of that
 *				file to an NxN Namespace path (which does not
 *				yet mean that the file is actually in the 
 *				database, but rather that it could be because
 *				it resides inside the database's local working
 *				folder.
 *				Using this NxN Namespace path, you can then try
 *				to retrieve a CNxNNode or CNxNFile instance of
 *				the managed object to see if it is actually in
 *				the database. This requires establishing a 
 *				connection to the server and will (only on the
 *				first attempt) prompt the user "Do you want to
 *				connect or work off-line?". If the user chooses
 *				"Work Off-line", no connection is 
 *				established.
 *				The advantage of using the CNxNMapper as a "front
 *				door" to the NxN Integrator SDK is that this approach
 *				speeds up the loading time of your application
 *				(because the NxN Namespace and Plug-in DLLs are
 *				loaded only when they are actually needed) and
 *				that it allows the user to choose to work off-line
 *				if a network connection is down and the application
 *				would otherwise block every time it tried to
 *				access the database.				
 *
 *  \author		Gregor vom Scheidt
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[gs]-08-26-2000 file created.
 *		[gs]-09-12-2000 Added doxygen tags
 *		[gs]-10-12-2000 Added GetManagedPaths() and RefreshManagedPaths()
 *		[gs]-11-12-2000 Extended to handle lower-level working paths correctly.
 *	\endmod
 */

//---------------------------------------------------------------------------
//	hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNMapperData*	tNxNMapperData;
class CNxNIntegrator;
class CNxNNode;
class CNxNFile;
class CNxNWorkspace;
class CNxNString;

#include "./NxNArray.h"		// template CNxNArray<ItemType>


struct NXNINTEGRATORSDK_API CNxNMapperInfo {
	CNxNString	m_sProjectName;		// the project name
	CNxNString	m_sWorkingPath;		// the managed path
	CNxNString	m_sLogonName;		// the user's log-on name
	CNxNString	m_sServerName;		// the server's computer name
	bool		m_bUnloaded;		// if 'true', project is unloaded
};

class NXNINTEGRATORSDK_API CNxNMapper : public CNxNObject {
	NXN_DECLARE_DYNCREATE(CNxNMapper);

	//--------------------------------------------------------------------
	// Member variables
	//--------------------------------------------------------------------
	private:
		tNxNMapperData	m_pData;

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
		bool InitInstance(CNxNIntegrator* pIntegrator = 0, bool bShowDialog = true);
		bool ExitInstance();

	//--------------------------------------------------------------------
	// for changing the question text in the connection dialog
	//--------------------------------------------------------------------
	void SetConnectionText(const CNxNString& strConnectionText);

	//--------------------------------------------------------------------
	// access to CNxNIntegrator interface
	//--------------------------------------------------------------------
	public:
		CNxNIntegrator* GetIntegrator();

	//--------------------------------------------------------------------
	// map managed paths
	//--------------------------------------------------------------------
	public:
		bool MapManagedPath( const CNxNString& sLocalPath,
							 CNxNString& sNamespacePath );
		CNxNNode* GetManagedNode( const CNxNString& sNamespacePath );
		CNxNFile* MapAndGetManagedFile( const CNxNString& sLocalPath );

	//--------------------------------------------------------------------
	// diagnostics and user information
	//--------------------------------------------------------------------
	public:
		int	GetManagedPaths( CNxNArray<CNxNMapperInfo>& InfoArray );
		bool RefreshManagedPaths( void );

	//--------------------------------------------------------------------
	// access to window handle's
	//--------------------------------------------------------------------
	public:
		void SetParentWindow(long hWnd);

};

#endif // INC_NXN_MAPPER_H
