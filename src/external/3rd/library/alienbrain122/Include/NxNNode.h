// \addtodoc

#ifndef INC_NXN_NODE_H
#define INC_NXN_NODE_H

/*!	\file		NxNNode.h
 *	
 *	\brief		This file contains the definition for the class CNxNNode and its associated types.
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

/*  \class      CNxNNode NxNNode.h
 *
 *  \brief      This class is the basic building block of the SDK class library.
 *              CNxNNode is the root class for all items that are available through the Namespace and
 *              defines the common interface for the different kinds of these items.
 *
 *  \author     Axel Pfeuffer, Helmut Klinger
 *
 *  \version    1.00
 *
 *  \date       2000
 *
 *  \mod
 *      [ap]-02-May-2000 file created.
 *      [ap]-21-Sep-2000 BUGFIX: Most methods are now virtual.
 *      [dp]-27-Sep-2001 Added sign-off parameter flags to the import and check-in commands.
 *  \endmod
 */

//---------------------------------------------------------------------------
//  There are some ugly macros defined in windowsx.h, undefine them!
//---------------------------------------------------------------------------
#ifdef GetFirstChild
#   undef GetFirstChild
#endif
#ifdef GetNextChild
#   undef GetNextChild
#endif

//---------------------------------------------------------------------------
//  hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNNodeData*   tNxNNodeData;

//---------------------------------------------------------------------------
//  iterator object
//---------------------------------------------------------------------------
typedef struct NXNINTEGRATORSDK_API CNxNNodeIterator : public CNxNIterator
{
    CNxNString  m_strCurrentChild;

    virtual bool IsEnd() const;
} TNxNNodeIterator;

/*  \enum eNxNBasicNamespaceType
 *      This enumeration type is used is conjunction with GetNodeInfo() and defines
 *      values that identify the different types of nodes. 
 */
/*  \var eNxNBasicNamespaceType NXN_BASICTYPE_WORKSPACE
 *      value denoting a workspace node
 */
/*  \var eNxNBasicNamespaceType NXN_BASICTYPE_PROJECT
 *      value denoting a project node
 */
/*  \var eNxNBasicNamespaceType NXN_BASICTYPE_FILE
 *      value denoting a file node
 */
/*  \var eNxNBasicNamespaceType NXN_BASICTYPE_FOLDER
 *      value denoting a folder node
 */
/*  \var eNxNBasicNamespaceType NXN_BASICTYPE_DISKFILE
 *      value denoting a disk file node
 */
/*  \var eNxNBasicNamespaceType NXN_BASICTYPE_DISKFOLDER
 *      value denoting a disk folder node
 */
/*  \var eNxNBasicNamespaceType NXN_BASICTYPE_UNKNOWN
 *      value denoting an unknown node
 */
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

class NXNINTEGRATORSDK_API CNxNNode : public CNxNObject
{
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
        //  construction/destruction
        //---------------------------------------------------------------------------
        CNxNNode(CNxNNode* pParent = NULL);
        virtual ~CNxNNode();

        //---------------------------------------------------------------------------
        //  command execution
        //---------------------------------------------------------------------------
        bool RunCommand(CNxNCommand& comCommandToRun);

        //---------------------------------------------------------------------------
        //  object hierarchy information access
        //---------------------------------------------------------------------------
        CNxNNode* GetParent() const;
        CNxNNode* GetRoot() const;

        inline bool HasParent() const { return (GetParent() != NULL); };
        inline bool HasRoot() const { return (GetRoot() != NULL); };

        virtual CNxNNode* GetFirstChild(TNxNNodeIterator* pNodeIterator = NULL);
        virtual CNxNNode* GetNextChild(TNxNNodeIterator* pNodeIterator = NULL);

        virtual int GetNextNodeInfos( bool bStart, int iNbOfNodes, sNxNBasicNodeInfo* pBasicInfo ) const;

        virtual long GetChildCount() const;

        virtual CNxNNode* GetNode(const CNxNString& strName);
        virtual CNxNNode* GetTypedNode(const CNxNString& strNamespacePath, const CNxNType& tType);

        virtual CNxNNode* FindNode(const CNxNString& strNamespacePath);
        virtual CNxNNode* FindTypedNode(const CNxNString& strNamespacePath, const CNxNType& tType);

        
        virtual CNxNNode* CreateNode(const CNxNString& strFileName, 
                                     const CNxNString& strMimeType, 
                                     bool bShowDialog = true, 
                                     const CNxNString& strComment = _STR(""), 
                                     const CNxNString& strKeywords = _STR(""), 
                                     const CNxNString& strLocalPath = _STR(""), 
                                     bool bFailIfExists = false, 
                                     bool bUseTemplate = false);

        bool HasChilds() const;
        bool HasChildren() const;

        //---------------------------------------------------------------------------
        //  flush for object and property data!
        //---------------------------------------------------------------------------
        virtual bool Flush(bool bDeleteChildren = false, bool bFlushNamespaceObjs = true);
        virtual bool FlushProperties();

        //---------------------------------------------------------------------------
        //  single property access
        //---------------------------------------------------------------------------
        virtual bool SetProperty(const CNxNProperty& propProperty);
        virtual bool GetProperty(CNxNProperty& propProperty) const;

        virtual CNxNString GetStringProperty(const CNxNString& strPropertyName, long lFlags = NXNCONST_GETPROP_DIRECT) const;

        //---------------------------------------------------------------------------
        //  property block access
        //---------------------------------------------------------------------------
        virtual bool GetProperties(CNxNPropertyCollection& aProperties, long lPropertyFlags = NXNCONST_GETPROP_DIRECT);
        virtual bool PrefetchProperties(const CNxNPropertyCollection& aProperties);

        //---------------------------------------------------------------------------
        //  object information access
        //---------------------------------------------------------------------------
        virtual CNxNString GetName() const;
        virtual CNxNString GetNamespacePath() const;
        virtual CNxNType GetNamespaceType() const;
        virtual CNxNString GetLocalPath() const;
        virtual CNxNString GetServerPath() const;
        virtual CNxNString GetDbPath() const;
        bool IsOfType(const CNxNType& tType) const;

        virtual bool IsValid() const;

        virtual bool GetNodeInfo(sNxNBasicNodeInfo* pBasicInfo) const;

        //--------------------------------------------------------------------
        // unique identifiers
        //--------------------------------------------------------------------
        virtual CNxNNode* ResolveUniqueIdentifier(const CNxNString& sUniqueIdentifier,
                                                  bool  bConnect = false,
                                                  const CNxNString& sUsername = _STR(L""),
                                                  const CNxNString& sPassword = _STR(L""));

        //---------------------------------------------------------------------------
        //  static object type information
        //---------------------------------------------------------------------------
        /*! \fn         static CNxNType CNxNNode::GetType()
         *  
         *              This static method returns the type of the interface. Generally
         *              all classes provide this method, except for the base class CNxNObject.
         *  
         *  \param      none
         *  
         *  \return     A CNxNType object containing the type as a string.
         *  
         *  \note       
         */
        static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_NODE);
        }

        //---------------------------------------------------------------------------
        //  direct access to node object data
        //---------------------------------------------------------------------------
        inline tNxNNodeData GetNodeData() const
        {
            return m_pNodeData;
        }

        //---------------------------------------------------------------------------
        //  special browse method
        //---------------------------------------------------------------------------
        virtual CNxNNode* BrowseForObject(CNxNBrowseFilter& bfFilter, const CNxNString& strTitle, bool bOnlyBrowsing = true);

        //---------------------------------------------------------------------------
        //  access to object specific menu and icon
        //---------------------------------------------------------------------------
        CNxNMenu* GetMenu();
        void ReleaseMenu();

        bool GetIcon(long& lNormal, long& lSelected, long& lMask, long& lState);

    public:
        //---------------------------------------------------------------------------
        //  class factory
        //---------------------------------------------------------------------------
        CNxNNode* GetOrCreateChildObject(const CNxNString& strChildName, const CNxNString& strNamespaceType);

        void Reset();

        void SetName(const CNxNString& strName);
        virtual void SetParent(CNxNNode* pNewParent);

    protected:
        //---------------------------------------------------------------------------
        //  internal hierarchical data
        //---------------------------------------------------------------------------
        bool AddInternalChild(CNxNNode* pChild);
        bool RemoveInternalChild(CNxNNode* pChild);
        void DeleteInternalChildren();

        CNxNNode* FindInternalChild(const CNxNString& strChildName) const;

        long GetInternalChildCount() const;
        bool HasInternalChildren() const;

        CNxNNode* GetInternalChild(int i) const;

        CNxNNode* GetInternalChild_Recursive(const CNxNString& strPath);
        CNxNNode* CreateInternalChild_Recursive(const CNxNString& strPath);

    private:
        CNxNNode* CreateChildObject(const CNxNType& typeObject);

    private:
        tNxNNodeData    m_pNodeData;
};

#endif // INC_NXN_NODE_H
