// \addtodoc

#ifndef INC_NXN_DBNODE_H
#define INC_NXN_DBNODE_H

/* \class		CNxNDbNode
 *
 *	\file		NxNDbNode.h
 *
 *  \brief		CNxNDbNode encapsulates all objects that are contained by a server's database.
 *				Usually those objects are of type CNxNFile, CNxNFolder or CNxNProject. There
 *				may be some more objects in a future version of the SDK. This interface
 *				provides methods for operation with those database nodes and which are inherited
 *				to all types of objects.
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-05-02-2000 file created.
 *		[ap]-09-21-2000 BUGFIX: Most methods are now virtual.
 *		[gs]-10-12-2000 added 'bSilent' parameter to GetLatest() 
 *		[gs]-11-11-2000 renamed eCheckIn_CreateVersionPolicy enumeration
 *		[gs]-11-11-2000 replaced GetLatest() 'bKeepUnchanged' (obsolete!) parameter with 'bForceUpdate' 
 *		[gs]-11-11-2000 added 'bOnlyExisting', 'bForceUpdate', 'nOWP', 'nOCP', 'nFTP' parameters to GetLatest()
 *	\endmod
 */

//---------------------------------------------------------------------------
//	check in - create version flags
//---------------------------------------------------------------------------
enum eCheckIn_CreateVersionPolicy {
	NXN_CHECKIN_CREATE_VERSION_DEFAULT	= 0,
	NXN_CHECKIN_CREATE_VERSION_AUTO		= 0,
	NXN_CHECKIN_CREATE_VERSION_NO		= 1,
	NXN_CHECKIN_CREATE_VERSION_YES		= 2,
	NXN_CHECKIN_CREATE_VERSION_MAX
};

enum eGetLatest_OverwriteWritablePolicy {
	NXN_GETLATEST_OVERWRITE_WRITABLE_DEFAULT	= 0,
	NXN_GETLATEST_OVERWRITE_WRITABLE_ASK		= 0,
	NXN_GETLATEST_OVERWRITE_WRITABLE_SKIP		= 1,
	NXN_GETLATEST_OVERWRITE_WRITABLE_REPLACE	= 2
};

enum eGetLatest_OverwriteCheckedOutPolicy {
	NXN_GETLATEST_OVERWRITE_CHECKEDOUT_DEFAULT	= 0,
	NXN_GETLATEST_OVERWRITE_CHECKEDOUT_ASK		= 0,
	NXN_GETLATEST_OVERWRITE_CHECKEDOUT_SKIP		= 1,
	NXN_GETLATEST_OVERWRITE_CHECKEDOUT_REPLACE	= 2
};

enum eGetLatest_FileTimePolicy {
	NXN_GETLATEST_FILETIME_DEFAULT		= 0,
	NXN_GETLATEST_FILETIME_MODIFICATION = 0,
	NXN_GETLATEST_FILETIME_CHECKIN		= 1,
	NXN_GETLATEST_FILETIME_CURRENT		= 2
};


//---------------------------------------------------------------------------
//	dependency types
//---------------------------------------------------------------------------
/*!	\enum eDependencyType
 *		These enumerated dependency types are for the only use with the methods
 *		DependencyAdd(), DependencyRemove(), DependencyClear(), GetFirstDependency() and
 *		GetNextDependency().
 */
enum eDependencyType {
	/*!	\var eDependencyType DET_INTERNAL	
	 *		Internal Dependency.
	 */
	DET_INTERNAL = 0,				// this type is the default

	/*!	\var eDependencyType DET_EXTERNAL
	 *		External (unresolved) dependency
	 */
	DET_EXTERNAL,

	/*!	\var eDependencyType DET_MANUAL	
	 *		Manual dependency.
	 */
	DET_MANUAL,

	/*!	\var eDependencyType MAX_DEPENDENCY_TYPES
	 *		There are two type of separators. If there's an opcode specified in a
	 *		separator menu item, then you must treat this menu item as a "category"
	 */
	MAX_DEPENDENCY_TYPES
};

//---------------------------------------------------------------------------
//	hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNDbNodeData*	tNxNDbNodeData;

class NXNINTEGRATORSDK_API CNxNDbNode : public CNxNNode {
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
		bool Import(	const CNxNString& strFileName, 
						bool bShowDialog = true, 
						const CNxNString& strComment = _STR(""), 
						const CNxNString& strKeywords = _STR(""), 
						bool bDeleteLocal = false, 
						bool bDontCheckForExisting = true, 
						bool bRunSilent = false);

		bool ImportDbNode( const CNxNString& strFileName, 
						bool bShowDialog = true, 
						const CNxNString& strComment = _STR(""), 
						const CNxNString& strKeywords = _STR(""), 
						bool bDeleteLocal = false, 
						bool bDontCheckForExisting = true, 
						bool bRunSilent = false);

		bool CheckIn(	bool bShowDialog = true, 
						const CNxNString& strFileName = _STR(""), 
						const CNxNString& strComment = _STR(""), 
						bool bKeepCheckedOut = false, 
						bool bDeleteLocal = false,
						bool bAskForEachObject = false, 
						bool bApplyToReferences = false, 
						eCheckIn_CreateVersionPolicy eCreateVersion = NXN_CHECKIN_CREATE_VERSION_DEFAULT, 
						bool bRunSilent = false);

		bool CheckOut(	bool bShowDialog = true, 
						const CNxNString& strComment = _STR(""), 
						bool bDontGetLocal = false,	
						bool bAskForEachObject = false, 
						bool bApplyToReferences = false, 
						const CNxNString& strCheckOutPath = _STR(""), 
						bool bRunSilent = false);

		bool GetLatest(	bool bShowDialog = true, 
						bool bForceUpdate = false, 
						bool bCreateDirStructureOnly = false, 
						bool bAskForEachObject = false, 
						bool bApplyToReferences = false, 
						const CNxNString& strDestinationPath = _STR(""), 
						bool bRunSilent = false,
						bool bOnlyExisting = false,
						enum eGetLatest_OverwriteWritablePolicy nOWP = NXN_GETLATEST_OVERWRITE_WRITABLE_DEFAULT,
						enum eGetLatest_OverwriteCheckedOutPolicy nOCP = NXN_GETLATEST_OVERWRITE_CHECKEDOUT_DEFAULT,
						enum eGetLatest_FileTimePolicy nFTP = NXN_GETLATEST_FILETIME_DEFAULT);

		bool Activate();

		bool Edit();

		bool Compile();

		bool Delete();

		bool DeleteLocalCopy();

		bool Rename(const CNxNString& strNewName);

		bool UndoCheckOut(bool bShowDialog = true, bool bRunSilent = false);

//		bool Destroy();

		bool UndoMove(CNxNCommand& comCommand);
	
		bool DependencyAdd(CNxNDbNode& nodeReference, eDependencyType eType = DET_INTERNAL);

		bool DependencyClear(eDependencyType eType = DET_INTERNAL);

		bool DependencyRemove(CNxNDbNode& nodeReference, eDependencyType eType = DET_INTERNAL);

		//--------------------------------------------------------------------
		// database node information retrieval
		//--------------------------------------------------------------------
		virtual CNxNString GetKeywords() const;
		virtual CNxNString GetComment() const;
		virtual CNxNString GetMimeType() const;
		virtual long GetHandle() const;

		virtual CNxNString GetWorkingPath() const;
		virtual bool SetWorkingPath(const CNxNString& strNewWorkingPath);

		virtual bool IsCheckedOut() const;

		virtual bool IsValid() const;

		virtual CNxNDbNode* GetFirstDependency(eDependencyType eType = DET_INTERNAL) const;
		virtual CNxNDbNode* GetNextDependency(eDependencyType eType = DET_INTERNAL) const;

		//---------------------------------------------------------------------------
		//	project retrieval (parent)
		//---------------------------------------------------------------------------
		CNxNProject* GetProject();

		//---------------------------------------------------------------------------
		//	dbnode retrieval using the project specific handle
		//---------------------------------------------------------------------------
		CNxNDbNode* GetDbNodeFromHandle(long lHandle) const;

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType() { return CNxNType(NXN_TYPE_DBNODE); };

		//---------------------------------------------------------------------------
		//	direct access to node object data
		//---------------------------------------------------------------------------
		inline tNxNDbNodeData GetDbNodeData() const { return m_pDbNodeData; };

	protected:
		void Reset();
		void SetDbNodeList(CNxNDbNodeList* pDbNodeList);
		bool IsInDbNodeList();

	private:
		tNxNDbNodeData	m_pDbNodeData;
};


#endif // INC_NXN_DBNODE_H