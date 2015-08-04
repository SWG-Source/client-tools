// \addtodoc

#ifndef INC_NXN_DISKFILE_H
#define INC_NXN_DISKFILE_H

/*	\class		CNxNDiskFile NxNDiskFile.h
 *
 *  \brief		This class represents files on local disks.
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

class NXNINTEGRATORSDK_API CNxNDiskFile : public CNxNDiskItem
{
	NXN_DECLARE_DYNCREATE(CNxNDiskFile);

	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNDiskFile(CNxNFolder* pParent = NULL);
		CNxNDiskFile(CNxNDiskFolder* pParent);
		virtual ~CNxNDiskFile();

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_DISKITEM_FILE);
        }

	private:
};

#endif // INC_NXN_DISKFILE_H
