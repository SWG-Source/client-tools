// \addtodoc

#ifndef INC_NXN_FINDER_H
#define INC_NXN_FINDER_H
/*	\class		CNxNFinder NxNFinder.h
 *
 *	\brief		CNxNFinder allows to do searches within the namespace.
 *
 *  \author		Jens Riemschneider
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[jr]-04-Sep-2000 file created.
 *	\endmod
 */


// The internal data structure for the found nodes is a list. We use the STL 
// list template for this.
#include <list>
using namespace std;

// These are the types for the list of nodes (based on the STL list template).
typedef list<CNxNNode*> NODEPTRLIST;
typedef NODEPTRLIST* LPNODEPTRLIST;
typedef const NODEPTRLIST* LPCNODEPTRLIST;

class NXNINTEGRATORSDK_API CNxNFinder : public CNxNObject
{
	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNFinder();
		CNxNFinder( CNxNNode*         pNodeRootOfSearch, 
					const CNxNString& strSearchExpr );

		virtual ~CNxNFinder();

		//---------------------------------------------------------------------------
		//	object information access
		//---------------------------------------------------------------------------
		CNxNNode*   GetRootNode();
		void        SetRootNode( CNxNNode* pNodeRootOfSearch );

		CNxNString  GetSearchExpr();
		void        SetSearchExpr( const CNxNString& strSearchExpr );

		//---------------------------------------------------------------------------
		//	Starting the search
		//---------------------------------------------------------------------------
		bool        StartSearch();
		bool        StartSearch( CNxNNode*         pNodeRootOfSearch, 
								 const CNxNString& strSearchExpr );

		//---------------------------------------------------------------------------
		//	Retrieving of found nodes
		//---------------------------------------------------------------------------
		CNxNNode*   GetFirstFoundNode();
		CNxNNode*   GetNextFoundNode();

	private:
		CNxNNode*               m_pNodeRootOfSearch;      // The root node.
		CNxNString              m_strSearchExpr;          // The search expression.
		NODEPTRLIST             m_listSearchResult;       // The list of found nodes.
		NODEPTRLIST::iterator   m_iterCurrentRetrivalPos; // The current position 
														  // for found node retrieval.

	private:
		//---------------------------------------------------------------------------
		//	Clean up helpers.
		//---------------------------------------------------------------------------
		void DeleteOldListElements();
};


#endif // INC_NXN_FINDER_H



//---------------------------------------------------------------------------
//	(c) 2000 by NxN Software
//---------------------------------------------------------------------------
