// \addtodoc

#ifndef INC_NXN_NODELIST_H
#define INC_NXN_NODELIST_H

/*	\class		CNxNNodeList
 *	
 *	\file		NxNNodeList.h
 *	
 *  \brief		A CNxNNodeList class is a compoud of CNxNNode objects. This class
 *				is derived from CNxNNode to allow all derived operations on compound
 *				lists too.
 *	
 *	\author		Axel Pfeuffer, Helmut Klinger
 *	
 *	\version	1.00
 *	
 *	\date		2000
 *	
 *	\mod
 *		[ap]-09-19-2000 file created.
 *	\endmod
 */

//---------------------------------------------------------------------------
//	hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNNodeListData*	tNxNNodeListData;

class NXNINTEGRATORSDK_API CNxNNodeList : public CNxNNode {
	NXN_DECLARE_DYNCREATE(CNxNNodeList);

	public:
		//---------------------------------------------------------------------------
		// construction/destruction
		//---------------------------------------------------------------------------
		CNxNNodeList();
		virtual ~CNxNNodeList();

		//---------------------------------------------------------------------------
		// list concerning methods
		//---------------------------------------------------------------------------
		bool Add(CNxNNode* pNode);
		bool Remove(CNxNNode* pNode);

		bool RemoveAll();

		CNxNNode* GetFirst() const;
		CNxNNode* GetNext() const;

		//---------------------------------------------------------------------------
		// operators for easy access
		//---------------------------------------------------------------------------
		inline CNxNNodeList& operator += (CNxNNode& nodeNode)
		{
			Add(&nodeNode);
			return *this;
		}

		inline CNxNNodeList& operator -= (CNxNNode& nodeNode)
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
		long GetObjectCount() const;
		bool HasObjects() const;

		//---------------------------------------------------------------------------
		// overloaded methods to achieve command execution for node lists
		//---------------------------------------------------------------------------
		/*!	\fn			inline CNxNString CNxNNodeList::GetName() const
		 *	
		 *				This method does not return a valid name, because
		 *				node lists does not have a valid name.
		 *	
		 *	\param		none
		 *	
		 *	\return		Always an empty string.
		 *	
		 *	\note		
		 */
		inline CNxNString GetName() const { return _STR(""); };

		CNxNString GetNamespacePath() const;

		bool Flush();
		bool FlushProperties();

		//---------------------------------------------------------------------------
		//	overloaded property methods
		//---------------------------------------------------------------------------
		bool SetProperty(CNxNProperty& propProperty);

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType() { return CNxNType(NXN_TYPE_NODELIST); };

		//---------------------------------------------------------------------------
		//	direct access to node object data
		//---------------------------------------------------------------------------
		inline tNxNNodeListData GetNodeListData() const { return m_pNodeListData; };

	// methods for private use only
	private:
		CNxNNode* GetAt(int nIndex) const;
		bool RemoveAt(int nIndex);

	private:
		tNxNNodeListData	m_pNodeListData;
};

#endif // INC_NXN_NODELIST_H
