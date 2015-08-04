// \addtodoc

#ifndef INC_NXN_GLOBALSELECTION_H
#define INC_NXN_GLOBALSELECTION_H

/*  \class      CNxNGlobalSelection
 *  
 *  \file       NxNGlobalSelection.h
 *  
 *  \brief      This class gives you access to the global namespace selection.
 *
 *  \author     Axel Pfeuffer, Helmut Klinger
 *  
 *  \version    1.00
 *  
 *  \date       2000
 *  
 *  \mod
 *      [ap]-14-Jul-2000 file created.
 *  \endmod
 */


class NXNINTEGRATORSDK_API CNxNGlobalSelection : public CNxNDbNode
{
    public:
        //---------------------------------------------------------------------------
        // construction/destruction
        //---------------------------------------------------------------------------
        CNxNGlobalSelection(CNxNIntegrator* pIntegrator);
        virtual ~CNxNGlobalSelection();

        //---------------------------------------------------------------------------
        // list concerning methods
        //---------------------------------------------------------------------------
        virtual bool Add(CNxNNode* pNode);
        virtual bool Remove(CNxNNode* pNode);

        virtual bool RemoveAll();

        /*! \fn         inline bool CNxNGlobalSelection::Clear()
         *  
         *              This method clears the global selection list.
         *              Its effect is identical to CNxNGlobalSelection::RemoveAll()
         *  
         *  \param      none
         *  
         *  \return     boolean value indicating the success
         *  
         *  \note       
         */
        virtual inline bool Clear()
        {
            return RemoveAll();
        }

        virtual CNxNNode* GetFirst() const;
        virtual CNxNNode* GetNext() const;

        //---------------------------------------------------------------------------
        // operators for easy access
        //---------------------------------------------------------------------------
        inline CNxNGlobalSelection& operator += (CNxNNode& nodeNode)
        {
            Add(&nodeNode);
            return *this;
        }

        inline CNxNGlobalSelection& operator -= (CNxNNode& nodeNode)
        {
            Remove(&nodeNode);
            return *this;
        }

        inline CNxNNode* operator [](int nIndex) const
        {
            return GetAt(nIndex);
        }
        

        //---------------------------------------------------------------------------
        // list information retrieval
        //---------------------------------------------------------------------------
        virtual long GetObjectCount() const;
        virtual bool HasObjects() const;

        /*! \fn         inline CNxNString CNxNGlobalSelection::GetName() const
         *  
         *              This method does not return a valid name, because
         *              global selection lists can not have a valid name.
         *  
         *  \param      none
         *  
         *  \return     empty string
         *  
         *  \note       
         */
        virtual inline CNxNString GetName() const
        {
            return L"";
        }

        virtual CNxNString GetNamespacePath() const;
        virtual CNxNString GetLocalPath();
        virtual CNxNString GetServerPath();
        virtual CNxNString GetDbPath();

        virtual bool IsOfType(const CNxNType& tType) const;

        virtual CNxNNode* BrowseForObject(CNxNBrowseFilter& bfFilter, const CNxNString& strTitle, bool bOnlyBrowsing = true);

        virtual bool Flush(bool bDeleteChildren = false, bool bFlushNamespaceObjs = true);
        virtual bool FlushProperties();

        virtual bool IsValid();

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
            return CNxNType(NXN_TYPE_GLOBALSELECTION);
        }


    // methods for private use only
    private:
        virtual CNxNNode* GetAt(int nIndex) const;
        bool RemoveAt(int nIndex);


    private:
        CNxNString       m_strCurrentNode;
        CNxNIntegrator*  m_pIntegrator;
};

#endif // INC_NXN_GLOBALSELECTION_H
