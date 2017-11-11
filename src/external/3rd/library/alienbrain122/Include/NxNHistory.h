// \addtodoc

#ifndef INC_NXN_HISTORY_H
#define INC_NXN_HISTORY_H

/*	\class		CNxNHistory NxNHistory.h
 *
 *  \brief		A history object contains all versions and labels defined for a file
 *              that is stored in the database.
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2001
 *
 *	\mod
 *		[ap]-13-Feb-2001 file created.
 *	\endmod
 */

//---------------------------------------------------------------------------
//	history specific defines
//---------------------------------------------------------------------------
#define NXN_HISTORY_FOLDER_NAME     L"History"

class NXNINTEGRATORSDK_API CNxNHistory : public CNxNVersionControlItem
{
	NXN_DECLARE_DYNCREATE(CNxNHistory);

	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNHistory(CNxNFile* pParent = NULL);
		virtual ~CNxNHistory();

		//---------------------------------------------------------------------------
		//	command methods concnerning labels
		//---------------------------------------------------------------------------
		bool AddLabel(const CNxNString& strLabelName, 
					  bool bShowDialog = true, 
					  const CNxNString& strLabelDescription = _STR(""), 
                      const CNxNTime& tTimeStamp = CNxNTime(),
                      bool bIsLocalTime = false,
					  bool bRunSilent = false);

        bool RemoveLabel(CNxNHistoryLabel*& pHistoryLabel,
                         bool bShowDialog = true,
                         bool bRunSilent = false);

        CNxNHistoryLabel* GetFirstLabel(TNxNNodeIterator* pNodeIterator);
        CNxNHistoryLabel* GetNextLabel(TNxNNodeIterator* pNodeIterator);

		//---------------------------------------------------------------------------
		//	command methods concerning file versions
        //---------------------------------------------------------------------------
        bool GetVersion(CNxNHistoryVersion* pVersionToGet,
                        bool bShowDialog = true,
                        const CNxNString& sDestDir = _STR(""),
                        eNxNVersionOverwritePolicy eOverwritePolicy = NXN_VC_OVERWRITE_POLICY_ASK,
                        eNxNVersionOverwritePolicy eOverwritePolicyWriteable = NXN_VC_OVERWRITE_POLICY_ASK,
                        bool bRunSilent = false);
        bool ViewVersion(CNxNHistoryVersion* pVersionToView);
        bool RollbackToVersion(CNxNHistoryVersion* pRollbackToVersion,
                               bool bShowDialog = true,
                               const CNxNString& sComment = _STR(L""),
                               bool bGetLocalCopy = true,
                               bool bRestoreProps = true,
                               bool bRunSilent = false);

        CNxNHistoryVersion* GetFirstVersion(TNxNNodeIterator* pNodeIterator);
        CNxNHistoryVersion* GetNextVersion(TNxNNodeIterator* pNodeIterator);

		//---------------------------------------------------------------------------
		//	general history command methods
		//---------------------------------------------------------------------------
        bool ShowHistoryDialog();
        
        //---------------------------------------------------------------------------
		//	information retrieval
		//---------------------------------------------------------------------------
		long GetLabelCount() const;
		long GetVersionCount() const;

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_HISTORY);
        }

	private:
};

#endif // INC_NXN_HISTORY_H
