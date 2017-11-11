// \addtodoc

#ifndef INC_NXN_HISTORYLABEL_H
#define INC_NXN_HISTORYLABEL_H

/*	\class		CNxNHistoryLabel NxNHistoryLabel.h
 *
 *  \brief		This class represents a history label for a file.
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

class NXNINTEGRATORSDK_API CNxNHistoryLabel : public CNxNVersionControlItem
{
	NXN_DECLARE_DYNCREATE(CNxNHistoryLabel);

	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNHistoryLabel(CNxNHistory* pParent = NULL);
		virtual ~CNxNHistoryLabel();

		//---------------------------------------------------------------------------
		//	information retrieval
		//---------------------------------------------------------------------------
		long GetHandle() const;

        CNxNTime GetCreationTime() const;

		CNxNString GetLabelName() const;
		virtual CNxNString GetComment() const;
		CNxNString GetCreator() const;

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_HISTORYLABEL);
        }

	private:
};

#endif // INC_NXN_HISTORYLABEL_H
