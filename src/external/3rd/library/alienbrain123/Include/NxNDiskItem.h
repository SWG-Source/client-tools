// \addtodoc

#ifndef INC_NXN_DISKITEM_H
#define INC_NXN_DISKITEM_H

/*	\class		CNxNDiskItem NxNDiskItem.h
 *
 *  \brief		This class represents items on local disks.
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-13-Feb-2000 file created.
 *	\endmod
 */

class NXNINTEGRATORSDK_API CNxNDiskItem : public CNxNVirtualNode
{
	NXN_DECLARE_DYNCREATE(CNxNDiskItem);

	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNDiskItem(CNxNNode* pParent = NULL);
		virtual ~CNxNDiskItem();

		//-------------------------------------------------------------------
		// node related operations
		//--------------------------------------------------------------------
        bool Activate();

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_DISKITEM);
        }

	private:
};

#endif // INC_NXN_DISKITEM_H
