// \addtodoc

#ifndef INC_NXNPATHCOLLECTION_H
#define INC_NXNPATHCOLLECTION_H

/* \class		CNxNPathCollection NxNPathCollection.h
 *
 *  \brief		This class handles collections of path names.
 *				
 *  \author		Axel Pfeuffer, (c) 1999-2001 by NxN Software AG
 *
 *  \version	1.0
 *
 *  \date		2001
 *
 * 	\mod
 *		[ap]-27-Apr-2001 file created.
 *	\endmod
 */

#include "NxNSTLAllocator.h"


//---------------------------------------------------------------------------
//	hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNPathCollectionData*	tNxNPathCollectionData;
typedef DWORD                           HNxNPathItem;

//---------------------------------------------------------------------------
//	enumeration iterator
//---------------------------------------------------------------------------
typedef struct NXNINTEGRATORSDK_API CNxNPathIterator : public CNxNIterator
{
    HNxNPathItem   m_hCurrentPath;            // handle to the current path item

    bool IsEnd() const;
} TNxNPathIterator;


class NXNINTEGRATORSDK_API CNxNPathCollection : public CNxNObject
{
	public:
        //---------------------------------------------------------------------------
        //	construction/destruction
        //---------------------------------------------------------------------------
		CNxNPathCollection();
		virtual ~CNxNPathCollection();

        //---------------------------------------------------------------------------
        //	collection manipulating methods
        //---------------------------------------------------------------------------
        HNxNPathItem Add(const CNxNString& sPathItem, DWORD dwUserData = 0);
        bool Remove(HNxNPathItem hItem);
        bool Remove(const CNxNString& sPathItem);
        void RemoveAll();

        bool SetUserData(HNxNPathItem hItem, DWORD dwUserData);

        //---------------------------------------------------------------------------
        //	information retrieval methods
        //---------------------------------------------------------------------------
        bool Contains(const CNxNString& sPathItem) const;
        DWORD GetUserData(HNxNPathItem hItem) const;
        CNxNString GetPath(HNxNPathItem hItem) const;

        HNxNPathItem Get(const CNxNString& sPathItem) const;

        HNxNPathItem GetFirst(TNxNPathIterator* pPathIterator) const;
        HNxNPathItem GetNext(TNxNPathIterator* pPathIterator) const;


        /*!	\fn			bool CNxNPathCollection::IsEmpty() const
         *	
         *			    Returns a boolean expression if the collection is empty.
         *	
         *	\param		none
         *	
         *	\return		boolean expression indication if colleciton is empty.
         *	
         *	\note		
         */
        inline bool IsEmpty() const
        {
            return GetCount() == 0;
        }
        
        /*!	\fn			inline bool HasItems() const
         *	
         *				Returns a boolean expression if the collection is NOT empty.
         *	
         *	\param		none
         *	
         *	\return		boolean expression indication if colleciton is empty.
         *	
         *	\note		
         */
        inline bool HasItems() const
        {
            return GetCount() != 0;
        }

        long GetCount() const;

        CNxNString GetPathsAsString(const CNxNString& sSeparator = L"|") const;


    private:
        struct CNxNPathItem
        {
            CNxNPath   m_pathItem;             // full local path of the item
            DWORD      m_dwUserData;           // item specific user data
        };

        // comparison function for the internal map
        struct __ltItem
        {
	        bool operator()(CNxNPath strKey1, CNxNPath strKey2) const
	        {
		        return strKey1.IsLess(strKey2);
	        }
        };

        typedef std::map< CNxNPath,
                          CNxNPathItem*,
                          __ltItem,
                          CNxNSTLAllocator< CNxNPathItem* > > TPathCollectionMap;

	    TPathCollectionMap*	  m_pMapPathItems;
};

#endif // INC_NXNPATHCOLLECTION_H
