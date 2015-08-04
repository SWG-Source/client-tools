// \addtodoc

#ifndef INC_NXN_DBNODELIST_H
#define INC_NXN_DBNODELIST_H

#include "NxNArray.h"


/*  \class      CNxNDbNodeList
 *  
 *  \file       NxNDbNodeList.h
 *  
 *  \brief      This is a container class for database objects (CNxNDbNode). 
 *
 *  \author     Axel Pfeuffer, Helmut Klinger
 *  
 *  \version    1.00
 *  
 *  \date       2000
 *  
 *  \mod
 *      [ap]-19-May-2000 file created.
 *      [ap]-21-Sep-2000 BUGFIX: Most methods are now virtual.
 *  \endmod
 */


class NXNINTEGRATORSDK_API CNxNDbNodeList : public CNxNDbNode
{
    NXN_DECLARE_DYNCREATE(CNxNDbNodeList);

    public:
        //---------------------------------------------------------------------------
        // construction/destruction
        //---------------------------------------------------------------------------
        CNxNDbNodeList();
        virtual ~CNxNDbNodeList();

        //---------------------------------------------------------------------------
        // list concerning methods
        //---------------------------------------------------------------------------
        virtual bool Add(CNxNDbNode* pDbNode);
        virtual bool Remove(CNxNDbNode* pDbNode);

        virtual bool RemoveAll();

        // TODO: EXCHANGE GetFirst() GetNext() WITH THREAD-SAFE-VERSIONS!
        virtual CNxNDbNode* GetFirst() const;
        virtual CNxNDbNode* GetNext() const;

        //---------------------------------------------------------------------------
        // operators for easy access
        //---------------------------------------------------------------------------
        inline CNxNDbNodeList& operator += (CNxNDbNode& nodeDbNode)
        {
            Add(&nodeDbNode);
            return *this;
        }

        inline CNxNDbNodeList& operator -= (CNxNDbNode& nodeDbNode)
        {
            Remove(&nodeDbNode);
            return *this;
        }

        inline CNxNDbNode* operator [](int nIndex) const
        {
            return GetAt(nIndex);
        }
        

        //---------------------------------------------------------------------------
        // list information retrieval
        //---------------------------------------------------------------------------
        virtual long GetObjectCount() const;
        virtual bool HasObjects() const;

        //---------------------------------------------------------------------------
        // overloaded methods to achieve command execution for dbnode lists
        //---------------------------------------------------------------------------
        /*! \fn         inline CNxNString CNxNDbNodeList::GetName() const
         *  
         *              This method does not return a valid name, because
         *              node lists do not have a name.
         *  
         *  \param      none
         *  
         *  \return     always an empty string
         *  
         *  \note       
         */
        virtual inline CNxNString GetName() const
        {
            return L"";
        }

        virtual CNxNString GetNamespacePath() const;
        virtual CNxNString GetLocalPath() const;
        virtual CNxNString GetServerPath() const;
        virtual CNxNString GetDbPath() const;

        virtual CNxNNode* BrowseForObject(CNxNBrowseFilter& bfFilter, const CNxNString& strTitle, bool bOnlyBrowsing = true);

        virtual bool Flush();
        virtual bool FlushProperties();

        virtual bool IsValid() const;

        virtual bool Rename(const CNxNString& strNewName);

        //---------------------------------------------------------------------------
        //  overloaded property methods
        //---------------------------------------------------------------------------
        virtual bool SetProperty(CNxNProperty& propProperty);

        //---------------------------------------------------------------------------
        //  static object type information
        //---------------------------------------------------------------------------
        static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_DBNODELIST);
        }

    // methods for private use only
    private:
        virtual CNxNDbNode* GetAt(int nIndex) const;
        bool RemoveAt(int nIndex);


    private:
        typedef class NXNINTEGRATORSDK_API CNxNArray<CNxNDbNode*> tDbNodeMap;

        tDbNodeMap  m_mapDbNodeList;
        int         m_nCurrentIndex;
};

#endif // INC_NXN_DBNODELIST_H
