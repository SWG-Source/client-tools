// \addtodoc

#ifndef INC_NXN_VIRTUALNODE_H
#define INC_NXN_VIRTUALNODE_H

/* \class		CNxNVirtualNode
 *
 *	\file		NxNVirtualNode.h
 *
 *  \brief		This is the base class for all "virtual nodes".
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.0
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-02-May-2000 file created.
 *	\endmod
 */

class NXNINTEGRATORSDK_API CNxNVirtualNode : public CNxNNode
{
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
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
		{
			return CNxNType(NXN_TYPE_VIRTUALNODE);
		}

	private:
};

#endif // INC_NXN_VIRTUALNODE_H
