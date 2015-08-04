// \addtodoc

#ifndef INC_NXN_TYPE_H
#define INC_NXN_TYPE_H

/* \class       CNxNType
 *
 *  \file       NxNType.h
 *
 *  \brief      CNxNType is a class representing node types. The type information
 *              stored in objects of this class are strings containing Namespace types.
 *
 *  \author     Axel Pfeuffer, Helmut Klinger
 *
 *  \version    1.00
 *
 *  \date       2000
 *
 *  \mod
 *      [ap]-02-May-2000 file created.
 *      [ap]-20-Apr-2001 NXN_TYPE_DISKITEMLIST introduced.
 *  \endmod
 */

//---------------------------------------------------------------------------
//  public type constants
//---------------------------------------------------------------------------

// node objects (derived from CNxNObject)
#define NXN_TYPE_NODE               L""
#define NXN_TYPE_NODELIST           L"NodeList"
#define NXN_TYPE_MENU               L"Menu"

// extended nodes (separated into categories, derived from CNxNNode)
#define NXN_TYPE_INTEGRATOR         L"Integrator"
#define NXN_TYPE_EVENTMANAGER       L"EventManager"
#define NXN_TYPE_DBNODE             L"\\Workspace\\DbItem\\"
#define NXN_TYPE_VIRTUALNODE        L""

// db nodes (derived from CNxNDbNode)
#define NXN_TYPE_DBNODELIST         L"DbNodeList"
#define NXN_TYPE_DBFOLDER           L"\\Workspace\\DbItem\\FileFolder\\Folder\\"
#define NXN_TYPE_FILE               L"\\Workspace\\DbItem\\FileFolder\\File\\Asset\\"

// (derived from CNxNDbFolder)
#define NXN_TYPE_PROJECT            L"\\Workspace\\DbItem\\FileFolder\\Folder\\Project\\"
#define NXN_TYPE_FOLDER             L"\\Workspace\\DbItem\\FileFolder\\Folder\\Asset\\"

// user management nodes (derived from CNxNDbNode)
#define NXN_TYPE_USERMANAGEMENTITEM L"\\Workspace\\DbItem\\FileFolder\\System\\Folder\\System\\User\\"
#define NXN_TYPE_USERGROUP          NXN_TYPE_USERMANAGEMENTITEM
#define NXN_TYPE_USER               L"\\Workspace\\DbItem\\FileFolder\\System\\Folder\\System\\User\\UserEntry"

// virtual nodes (derived from CNxNVirtualNode)
#define NXN_TYPE_WORKSPACE          L"\\Workspace\\Workspace\\"
#define NXN_TYPE_DISKITEM           L"\\Workspace\\DiskItem\\"
#define NXN_TYPE_DISKITEMLIST       L"DiskItemList"
#define NXN_TYPE_VERSIONCONTROLITEM L"\\VersionControl\\"

// version control stuff - history items, admin (derived from CNxNVersionControlItem)
#define NXN_TYPE_HISTORY            L"\\VersionControl\\History\\"
#define NXN_TYPE_HISTORYLABEL       L"\\VersionControl\\Label\\"
#define NXN_TYPE_HISTORYVERSION     L"\\VersionControl\\Version\\"
// version control - admin (derived from CNxNVersionControlItem)
#define NXN_TYPE_ADMIN_ROOT         L"\\VersionControl\\Admin\\"
// version control - admin - bucket folder 
#define NXN_TYPE_BUCKET_FOLDER      L"\\VersionControl\\Buckets\\"
// version control - bucket
#define NXN_TYPE_BUCKET             L"\\VersionControl\\Buckets\\Bucket\\"

// disk item nodes (derived from CNxNDiskItem)
#define NXN_TYPE_DISKITEM_FOLDER    L"\\Workspace\\DiskItem\\Folder\\"
#define NXN_TYPE_DISKITEM_FILE      L"\\Workspace\\DiskItem\\File\\"

#define NXN_TYPE_GLOBALSELECTION    L"GlobalSelection"

//---------------------------------------------------------------------------
//  public constants
//---------------------------------------------------------------------------
#define NXN_TYPE_NAMESPACE_ROOT     L"\\"	// Namespace types for the root object 

//---------------------------------------------------------------------------
//  hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNTypeData*   tNxNTypeData;

class NXNINTEGRATORSDK_API CNxNType
{
    public:
        //---------------------------------------------------------------------------
        //  construction/destruction
        //---------------------------------------------------------------------------
        CNxNType();
        CNxNType(const CNxNType& tType);
        CNxNType(const CNxNString& strType);
        CNxNType(const CNxNString& strParentType, const CNxNString& strType);
        virtual ~CNxNType();

        //---------------------------------------------------------------------------
        //  overloaded operators
        //---------------------------------------------------------------------------
        operator=(const CNxNType& tType);
        operator=(const CNxNString& strString);
        operator const CNxNString& () const;
        operator const CNxNPath& () const;
    

        //---------------------------------------------------------------------------
        //  type information retrieval
        //---------------------------------------------------------------------------
        long GetLength() const;
        bool IsEmpty() const;
        bool IsValid(const CNxNString& strString) const;
        virtual bool InheritsFrom(const CNxNType& tType) const;
        virtual bool InheritsFrom(const CNxNString& strType) const;
        virtual bool IsSpecializationOf(const CNxNType& tType) const;
        virtual bool IsSpecializationOf(const CNxNString& strType) const;

        virtual bool DerivesFrom(const CNxNType& tType) const;
        virtual bool DerivesFrom(const CNxNString& strType) const;
        virtual bool IsGeneralizationOf(const CNxNType& tType) const;
        virtual bool IsGeneralizationOf(const CNxNString& strType) const;

        //---------------------------------------------------------------------------
        //  direct access to type object data
        //---------------------------------------------------------------------------
        inline tNxNTypeData GetTypeData() const
        {
            return m_pTypeData;
        }

    private:
        tNxNTypeData        m_pTypeData;
};

#endif // INC_NXN_TYPE_H

