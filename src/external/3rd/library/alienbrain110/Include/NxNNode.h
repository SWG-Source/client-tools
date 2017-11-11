// \addtodoc

#ifndef INC_NXN_NODE_H
#define INC_NXN_NODE_H

/*	\class		CNxNNode NxNNode.h
 *
 *  \brief		All namespace objects in alienbrain are nodes and can be organized as a
 *				complex tree known from windows explorer. 
 *
 *				All interactive objects are derived from this class, even the main class 
 *				CNxnIntegrator. Each (derived) CNxNNode
 *				instance provides some basic functionality. Using this interface you can operate
 *				on each node with the same function calls, anyway it's a CNxNIntegrator, a
 *				CNxNProject or a CNxNWorkspace object. These methods are also available in
 *				all derived classes.
 *				It is very necessary to know that there are some macros defined in the header file
 *				<windowsx.h> using the names GetFirstChild and GetNextChild. After the includation
 *				of this header file these macros are out of order!
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
 *	\endmod
 */

//---------------------------------------------------------------------------
//	There are some ugly macros defined in windowsx.h, undefine them!
//---------------------------------------------------------------------------
#ifdef GetFirstChild
#	undef GetFirstChild
#endif
#ifdef GetNextChild
#	undef GetNextChild
#endif

//---------------------------------------------------------------------------
//	hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNNodeData*	tNxNNodeData;


typedef enum
{
    NXN_BASICTYPE_WORKSPACE,
    NXN_BASICTYPE_PROJECT,
    NXN_BASICTYPE_FILE,
    NXN_BASICTYPE_FOLDER,
    NXN_BASICTYPE_DISKFILE,
    NXN_BASICTYPE_DISKFOLDER,
    NXN_BASICTYPE_UNKNOWN
} eNxNBasicNamespaceType;

typedef struct
{
    CNxNString              sName;
    CNxNString              sNamespacePath;
    CNxNString              sLocalPath;
    long                    lFlags;
    eNxNBasicNamespaceType  basicType;
} sNxNBasicNodeInfo;


class NXNINTEGRATORSDK_API CNxNNode : public CNxNObject {
	NXN_DECLARE_DYNCREATE(CNxNNode);

	friend CNxNNode;
	friend CNxNDbNode;
	friend CNxNProject;
	friend CNxNFile;
	friend CNxNFolder;
	friend CNxNIntegrator;
	friend CNxNWorkspace;
	friend CNxNGlobalSelection;

	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNNode(CNxNNode* pParent = NULL);
		virtual ~CNxNNode();

		//---------------------------------------------------------------------------
		//	command execution
		//---------------------------------------------------------------------------
		bool RunCommand(CNxNCommand& comCommandToRun);

		//---------------------------------------------------------------------------
		//	object hierarchy information access
		//---------------------------------------------------------------------------
		CNxNNode* GetParent() const;
		CNxNNode* GetRoot() const;

		inline bool HasParent() const { return (GetParent() != NULL); };

		CNxNNode* GetFirstChild();
		CNxNNode* GetNextChild();

        int GetNextNodeInfos( bool bStart, int iNbOfNodes, sNxNBasicNodeInfo* pBasicInfo ) const;

		long GetChildCount();

		CNxNNode* GetNode(const CNxNString& strName);
		CNxNNode* FindNode(const CNxNString& strNamespacePath);
		
		CNxNNode* CreateNode(	const CNxNString& strFileName, 
								const CNxNString& strMimeType, 
								bool bShowDialog = true, 
								const CNxNString& strComment = _STR(""), 
								const CNxNString& strKeywords = _STR(""), 
								const CNxNString& strLocalPath = _STR(""), 
								bool bFailIfExists = false, 
								bool bUseTemplate = false);

		bool HasChilds();

		//---------------------------------------------------------------------------
		//	flush for object and property data!
		//---------------------------------------------------------------------------
		virtual bool Flush(bool bDeleteChildren = false);
		virtual bool FlushProperties();

		//---------------------------------------------------------------------------
		//	property access
		//---------------------------------------------------------------------------
		virtual bool SetProperty(const CNxNProperty& propProperty);
		virtual bool GetProperty(CNxNProperty& propProperty) const;

		//---------------------------------------------------------------------------
		//	object information access
		//---------------------------------------------------------------------------
		virtual CNxNString GetName() const;
		virtual CNxNString GetNamespacePath() const;
		virtual CNxNType GetNamespaceType() const;
		virtual CNxNString GetLocalPath() const;
		virtual CNxNString GetServerPath() const;
		virtual CNxNString GetDbPath() const;
		bool IsOfType(const CNxNType& tType) const;

		virtual bool IsValid() const;

		virtual bool GetNodeInfo( sNxNBasicNodeInfo* pBasicInfo ) const;

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType() { return CNxNType(NXN_TYPE_NODE); };

		//---------------------------------------------------------------------------
		//	direct access to node object data
		//---------------------------------------------------------------------------
		inline tNxNNodeData GetNodeData() const { return m_pNodeData; };

		//---------------------------------------------------------------------------
		//	special browse method
		//---------------------------------------------------------------------------
		virtual CNxNNode* BrowseForObject(CNxNBrowseFilter& bfFilter, const CNxNString& strTitle, bool bOnlyBrowsing = true);

		//---------------------------------------------------------------------------
		//	access to object specific menu
		//---------------------------------------------------------------------------
		CNxNMenu* GetMenu();
		void ReleaseMenu();

	protected:
		//---------------------------------------------------------------------------
		//	class factory
		//---------------------------------------------------------------------------
		CNxNNode* CreateChildObject(const CNxNType& typeObject);
		CNxNNode* GetOrCreateChildObject(const CNxNString& strChildName, const CNxNString& strNamespaceType);

		void Reset();

		void SetName(const CNxNString& strName);
		virtual void SetParent(CNxNNode* pNewParent);

		//---------------------------------------------------------------------------
		//	internal hierarchical data
		//---------------------------------------------------------------------------
		bool AddInternalChild(CNxNNode* pChild);
		bool RemoveInternalChild(CNxNNode* pChild);
		void DeleteInternalChilds();

		CNxNNode* FindInternalChild(const CNxNString& strChildName) const;

		long GetInternalChildCount() const;
		bool HasInternalChilds() const;

		CNxNNode* GetInternalChild(int i);

		CNxNNode* GetInternalChild_Recursive(const CNxNString& strPath);
		CNxNNode* CreateInternalChild_Recursive(const CNxNString& strPath);
	private:
		tNxNNodeData	m_pNodeData;
};

#endif // INC_NXN_NODE_H
