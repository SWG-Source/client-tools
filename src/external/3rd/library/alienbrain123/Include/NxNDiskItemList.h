// \addtodoc
#ifndef INC_NXN_DISKITEMLIST_H
#define INC_NXN_DISKITEMLIST_H

/*  \class      CNxNDiskItemList NxNDiskItemList.h
 *
 *  \brief      This is a container class for disk item objects (CNxNDiskItem). 
 *
 *  \author     Axel Pfeuffer, Helmut Klinger
 *
 *  \version    1.00
 *
 *  \date       2000
 *
 *  \mod
 *      [ap]-13-Feb-2000 file created.
 *  \endmod
 */


class NXNINTEGRATORSDK_API CNxNDiskItemList : public CNxNDiskItem
{
    NXN_DECLARE_DYNCREATE(CNxNDiskItemList);

    public:
        //---------------------------------------------------------------------------
        //  construction/destruction
        //---------------------------------------------------------------------------
        CNxNDiskItemList();
        CNxNDiskItemList(const CNxNDiskItemList& srcDiskItemList);

        virtual ~CNxNDiskItemList();

        //---------------------------------------------------------------------------
        // operators
        //---------------------------------------------------------------------------
        const CNxNDiskItemList& operator =(const CNxNDiskItemList& srcArray);

        CNxNDiskItem*& operator [](int nIndex);
        CNxNDiskItem* operator [](int nIndex) const;

        //---------------------------------------------------------------------------
        // access methods
        //---------------------------------------------------------------------------
        bool Add(CNxNDiskItem* pDiskItem);

        bool SetAt(long nIndex, CNxNDiskItem* pDiskItem);
        void RemoveAt(long nIndex);

        //---------------------------------------------------------------------------
        // information retrieval methods
        //---------------------------------------------------------------------------
        CNxNDiskItem* GetAt(long nIndex) const;
        CNxNDiskItem*& ElementAt(long nIndex);

        long GetCount() const;
        long GetSize() const;

        bool IsEmpty() const;

        void RemoveAll();

        //---------------------------------------------------------------------------
        // overloaded methods to achieve command execution for disk item lists
        //---------------------------------------------------------------------------
        /*! \fn         inline CNxNString CNxNDiskItemList::GetName() const
         *  
         *              This method does not return a valid name, because
         *              node lists do not have valid names.
         *  
         *  \param      none
         *  
         *  \return     empty string
         *  
         *  \note       
         */
        inline CNxNString GetName() const
        {
            return L"";
        };

        virtual CNxNString GetNamespacePath() const;
        virtual CNxNString GetLocalPath() const;

        virtual bool Flush();
        virtual bool FlushProperties();

        virtual bool IsValid() const;

        //---------------------------------------------------------------------------
        //  overloaded property methods
        //---------------------------------------------------------------------------
        virtual bool SetProperty(CNxNProperty& propProperty);

        //---------------------------------------------------------------------------
        //  static object type information
        //---------------------------------------------------------------------------
        static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_DISKITEMLIST);
        }


    private:
        typedef class NXNINTEGRATORSDK_API CNxNArray<CNxNDiskItem*> tDiskItemList;

        tDiskItemList   m_aDiskItems;
};

#endif // INC_NXN_DISKITEM_H
