// \addtodoc

#ifndef INC_NXN_VIRTUALNODE_H
#define INC_NXN_VIRTUALNODE_H

/* \class		CNxNVirtualNode
 *
 *	\file		NxNVirtualNode.h
 *
 *  \brief		This is the base class for all "virtual nodes". A virtual node is a
 *				specific kind of node which does not rely in the server's database,
 *				but they're still visible when browsing through the namespace tree.
 *				All node's or items that rely in the server's database are dervied from
 *				the class CNxNDbNode.
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.0
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-05-02-2000 file created.
 *	\endmod
 */

class NXNINTEGRATORSDK_API CNxNVirtualNode : public CNxNNode {
	NXN_DECLARE_DYNCREATE(CNxNVirtualNode);

	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNVirtualNode(CNxNNode* pParent = NULL);
		virtual ~CNxNVirtualNode();

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType() { return CNxNType(NXN_TYPE_VIRTUALNODE); };

	private:
};

#endif // INC_NXN_VIRTUALNODE_H
