// \addtodoc

#ifndef INC_NXN_DBNODELIST_H
#define INC_NXN_DBNODELIST_H

/*	\class		CNxNDbNodeList
 *	
 *	\file		NxNDbNodeList.h
 *	
 *  \brief		A CNxNDbNodeList class is a compoud of CNxNDbNode objects. This class
 *				is derived from CNxNDbNode to allow all derived operations on compound
 *				lists too.
 *	
 *	\author		Axel Pfeuffer, Helmut Klinger
 *	
 *	\version	1.00
 *	
 *	\date		2000
 *	
 *	\mod
 *		[ap]-05-19-2000 file created.
 *		[ap]-09-21-2000 BUGFIX: Most methods are now virtual.
 *	\endmod
 */

//---------------------------------------------------------------------------
//	hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNDbNodeListData*	tNxNDbNodeListData;

class NXNINTEGRATORSDK_API CNxNDbNodeList : public CNxNDbNode {
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
		/*!	\fn			inline CNxNString CNxNDbNodeList::GetName() const
		 *	
		 *				This method does not return a valid name, because
		 *				dbnode lists does not have a valid name.
		 *	
		 *	\param		none
		 *	
		 *	\return		Always an empty string.
		 *	
		 *	\note		
		 */
		virtual inline CNxNString GetName() const { return _STR(""); };

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
		//	overloaded property methods
		//---------------------------------------------------------------------------
		virtual bool SetProperty(CNxNProperty& propProperty);

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType() { return CNxNType(NXN_TYPE_DBNODELIST); };

		//---------------------------------------------------------------------------
		//	direct access to node object data
		//---------------------------------------------------------------------------
		inline tNxNDbNodeListData GetDbNodeListData() const { return m_pDbNodeListData; };

	// methods for private use only
	private:
		virtual CNxNDbNode* GetAt(int nIndex) const;
		bool RemoveAt(int nIndex);

	private:
		tNxNDbNodeListData	m_pDbNodeListData;
};

#endif // INC_NXN_DBNODELIST_H
