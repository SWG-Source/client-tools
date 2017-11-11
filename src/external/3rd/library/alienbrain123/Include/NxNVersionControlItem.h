// \addtodoc

#ifndef INC_NXN_VERSIONCONTROLITEM_H
#define INC_NXN_VERSIONCONTROLITEM_H

/*	\class		CNxNVersionControlItem NxNVersionControlItem.h
 *
 *  \brief		This is the base class for all item types specificly related to the
 *				version control system.
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

class NXNINTEGRATORSDK_API CNxNVersionControlItem : public CNxNVirtualNode
{
	NXN_DECLARE_DYNCREATE(CNxNVersionControlItem);

	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNVersionControlItem(CNxNNode* pParent = NULL);
		virtual ~CNxNVersionControlItem();

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_VERSIONCONTROLITEM);
        }

	private:
};

#endif // INC_NXN_VERSIONCONTROLITEM_H
