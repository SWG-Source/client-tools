// \addtodoc

#ifndef INC_NXN_NODELIST_H
#define INC_NXN_NODELIST_H

#include "NxNArray.h"


/*	\class		CNxNNodeList NxNNodeList.h
 *	
 *  \brief		A CNxNNodeList class is a compound of CNxNNode objects. 
 *
 *	\author		Axel Pfeuffer, Helmut Klinger
 *	
 *	\version	1.00
 *	
 *	\date		2000
 *	
 *	\mod
 *		[ap]-19-Sep-2000 file created.
 *	\endmod
 */


class NXNINTEGRATORSDK_API CNxNNodeList : public CNxNNode
{
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
		CNxNNode* GetNext()  const;

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
		 *				This method does not return a valid name, this is because
		 *				node lists do not have valid names.
		 *	
		 *	\param		none
		 *	
		 *	\return		Always an empty string.
		 *	
		 *	\note		
		 */
		virtual inline CNxNString GetName() const
        {
            return L"";
        }

		virtual CNxNString GetNamespacePath() const;

		virtual bool Flush();
		virtual bool FlushProperties();

		//---------------------------------------------------------------------------
		//	overloaded property methods
		//---------------------------------------------------------------------------
		virtual bool SetProperty(CNxNProperty& propProperty);

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_NODELIST);
        }


	// methods for private use only
	private:
		CNxNNode* GetAt(int nIndex) const;
		bool RemoveAt(int nIndex);


	private:
        typedef class NXNINTEGRATORSDK_API CNxNArray<CNxNNode*> tNodeList;

	    tNodeList	m_aNodeList;
	    int			m_nCurrentIndex;
};

#endif // INC_NXN_NODELIST_H
