// \addtodoc

#ifndef INC_NXN_GLOBALSELECTION_H
#define INC_NXN_GLOBALSELECTION_H

/*	\class		CNxNGlobalSelection
 *	
 *	\file		NxNGlobalSelection.h
 *	
 *  \brief		A CNxNGlobalSelection class generally has the same functionality as CNxNNodeList. 
 *				The difference between these both classes is that CNxNNodeList is only
 *				for internal CNxNNode compoud lists. You can use CNxNGlobalSelection objects
 *				to comunicate with the java script object NxNNamespaceCollection. It is possible
 *				to retrieve contained objects contained or to set objects.
 *	
 *	\author		Axel Pfeuffer, Helmut Klinger
 *	
 *	\version	1.00
 *	
 *	\date		2000
 *	
 *	\mod
 *		[ap]-07-14-2000 file created.
 *	\endmod
 */

//---------------------------------------------------------------------------
//	hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNGlobalSelectionData*	tNxNGlobalSelectionData;

class NXNINTEGRATORSDK_API CNxNGlobalSelection : public CNxNDbNode {
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

		/*!	\fn			inline bool CNxNGlobalSelection::Clear()
		 *	
		 *				This method clears the complete selection list. This
		 *				method behaves like CNxNGlobalSelection::RemoveAll()
		 *	
		 *	\param		none
		 *	
		 *	\return		boolean expression indicating success.
		 *	
		 *	\note		
		 */
		virtual inline bool Clear() { return RemoveAll(); };

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

		/*!	\fn			inline CNxNString CNxNDbNodeList::GetName() const
		 *	
		 *				This method does not return a valid name, because
		 *				global selections lists can not have a valid name.
		 *				They are just meant to be used as a node container.
		 *	
		 *	\param		none
		 *	
		 *	\return		Always an empty string.
		 *	
		 *	\note		
		 */
		virtual inline CNxNString GetName() const { return _STR(""); };

		virtual CNxNString GetNamespacePath() const;
		virtual CNxNString GetLocalPath();
		virtual CNxNString GetServerPath();
		virtual CNxNString GetDbPath();

		virtual bool IsOfType(const CNxNType& tType) const;

		virtual CNxNNode* BrowseForObject(CNxNBrowseFilter& bfFilter, const CNxNString& strTitle, bool bOnlyBrowsing = true);

		virtual bool Flush();
		virtual bool FlushProperties();

		virtual bool IsValid();

		virtual bool Rename(const CNxNString& strNewName);

		//---------------------------------------------------------------------------
		//	overloaded property methods
		//---------------------------------------------------------------------------
		virtual bool SetProperty(CNxNProperty& propProperty);

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType() { return CNxNType(NXN_TYPE_GLOBALSELECTION); };

		//---------------------------------------------------------------------------
		//	direct access to global selection data
		//---------------------------------------------------------------------------
		inline tNxNGlobalSelectionData GetGlobalSelectionData() const { return m_pGlobalSelectionData; };

	// methods for private use only
	private:
		virtual CNxNNode* GetAt(int nIndex) const;
		bool RemoveAt(int nIndex);

	private:
		tNxNGlobalSelectionData	m_pGlobalSelectionData;
};

#endif // INC_NXN_GLOBALSELECTION_H
