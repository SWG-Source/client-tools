// \addtodoc

#ifndef INC_NXNPROPERTYCOLLECTION_H
#define INC_NXNPROPERTYCOLLECTION_H

/* \class       CNxNPropertyCollection NxNPropertyCollection.h
 *
 *  \brief      This class handles collections of properties.
 *
 *  \author     Axel Pfeuffer, (c) 1999-2001 by NxN Software AG
 *
 *  \version    1.0
 *
 *  \date       2001
 *
 *  \mod
 *      [ap]-27-Apr-2001 file created.
 *  \endmod
 */

#include <map>

#include "NxNSTLAllocator.h"


//---------------------------------------------------------------------------
//  enumeration iterator
//---------------------------------------------------------------------------
typedef struct NXNINTEGRATORSDK_API CNxNPropertyIterator : public CNxNIterator
{
    DWORD   m_hCurrentProperty;            // handle to the current property

    bool IsEnd() const;
} TNxNPropertyIterator;


class NXNINTEGRATORSDK_API CNxNPropertyCollection : public CNxNObject
{
    public:
        //---------------------------------------------------------------------------
        //  construction/destruction
        //---------------------------------------------------------------------------
        CNxNPropertyCollection();
        virtual ~CNxNPropertyCollection();

        //---------------------------------------------------------------------------
        //  collection manipulating methods
        //---------------------------------------------------------------------------
        bool Add(const CNxNProperty& propToAdd);
        bool Add(const CNxNString& strPropertyName, long lPropertyFlags = NXNCONST_GETPROP_DIRECT);
        bool Remove(TNxNPropertyIterator* pCurrentPropertyIt);
        bool Remove(const CNxNString& sPropertyToRemove);
        void RemoveAll();

        //---------------------------------------------------------------------------
        //  information retrieval methods
        //---------------------------------------------------------------------------
        bool Contains(const CNxNString& sProperty) const;

        CNxNProperty Get(const CNxNString& sPropertyName) const;
        CNxNProperty& Element(const CNxNString& sPropertyName);

        CNxNProperty* GetFirst(TNxNPropertyIterator* pCurrentPropertyIt) const;
        CNxNProperty* GetNext(TNxNPropertyIterator* pCurrentPropertyIt) const;

        /*! \fn         bool CNxNPropertyCollection::IsEmpty() const
         *  
         *              Returns a boolean value indicating if the collection is empty.
         *  
         *  \param      none
         *  
         *  \return     boolean value indicating if colleciton is empty
         *  
         *  \note       
         */
        inline bool IsEmpty() const
        {
            return GetCount() == 0;
        }
        
        /*! \fn         inline bool HasItems() const
         *  
         *              Returns a boolean value indicating if the collection is not empty.
         *  
         *  \param      none
         *  
         *  \return     boolean value indicating if collection is non-empty
         *  
         *  \note       
         */
        inline bool HasItems() const
        {
            return GetCount() != 0;
        }

        long GetCount() const;

        //--------------------------------------------------------------------
        // operators for easier access
        //--------------------------------------------------------------------
        inline CNxNProperty& operator[](LPCWSTR szPropertyName)
        {
            return Element(szPropertyName);
        }

        inline CNxNProperty operator[](LPCWSTR szPropertyName) const
        {
            return Get(szPropertyName);
        }

        inline CNxNProperty& operator[](const char* szPropertyName)
        {
            return Element(CNxNString(szPropertyName));
        }

        inline CNxNProperty operator[](const char* szPropertyName) const
        {
            return Get(CNxNString(szPropertyName));
        }


    private:
        struct __ltItem
        {
            bool operator () (CNxNString strKey1, CNxNString strKey2) const
            {
                return strKey1.CompareNoCase(strKey2) < 0;
            }
        };

        typedef std::map< CNxNString,
                          CNxNProperty,
                          __ltItem,
                          CNxNSTLAllocator< CNxNProperty > > TPropertyCollectionMap;

        TPropertyCollectionMap*  m_pMapProperties;                    // containing all properties
};

#endif // INC_NXNPROPERTYCOLLECTION_H
