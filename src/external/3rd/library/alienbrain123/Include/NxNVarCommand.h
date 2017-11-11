// \addtodoc

#ifndef INC_NXN_VARCOMMAND_H
#define INC_NXN_VARCOMMAND_H


class CNxNItem;
class CNxNParamXML;
typedef void TNxNEventNode;

class NXNINTEGRATORSDK_API CNxNCommandEvent: public CNxNObject
{
public:
    CNxNCommandEvent();
    virtual ~CNxNCommandEvent();

    void SetCurrent(const CNxNString& sPath, TNxNEventNode* pEventNode);

    bool IsError();
    bool IsWarning();
    bool IsNotification();

    CNxNString GetSeverity();
    long       GetCode();
    CNxNString GetPath();
    CNxNString GetMessage();

    bool GetItem(CNxNItem& EventItem);

    bool GetAttribute(const CNxNString& sAttrName, CNxNString& sAttrValue);

private:
    CNxNParamXML*   m_pParamXML;
    TNxNEventNode*  m_pEventNode;
    CNxNString      m_sPath;
};


class NXNINTEGRATORSDK_API CNxNVarCommand: public CNxNObject
{
public:
    // Verbose levels
    enum eVerboseLevel
    {
        VERBOSE_NOTHING = 0,
        VERBOSE_ERRORS,
        VERBOSE_WARNINGS,
        VERBOSE_NOTIFICATIONS,
        VERBOSE_OKNOTES
    };

    // Progress dialog setting
    enum eProgressType
    {
        PROGRESS_SHOW_WITH_CANCEL = 0,
        PROGRESS_HIDE_DIALOG,
        PROGRESS_HIDE_BAR,
        PROGRESS_HIDE_CANCEL
    };

    // Response default values
    enum eResponseType
    {
        RESPONSE_YES = 1,
        RESPONSE_NO,
        RESPONSE_OK
    };

public:
    CNxNVarCommand(const CNxNString& sCommand, long lFlags = 0, const CNxNString& sParams = L"");
    virtual ~CNxNVarCommand();

    void Reset();

    // basic command information
    bool       SetOpcode(const CNxNString& sCommand);
    CNxNString GetOpcode() const;

    void SetFlags(long lFlags);
    long GetFlags() const;

    // access to parameters
    bool SetInParams(const CNxNString& sParams);

    bool SetIn(const CNxNString& sParamName, const CNxNString& sParamValue);
    bool SetIn(const CNxNString& sParamName, long lParamValue);
    bool SetIn(const CNxNString& sParamName, bool bParamValue);

    bool       GetOut(const CNxNString& sParamName, CNxNString& sParamValue);
    CNxNString GetOutString(const CNxNString& sParamName, const CNxNString& sDefaultValue = _STR(L""));
    long       GetOutLong(const CNxNString& sParamName, long lDefaultValue = 0);
    bool       GetOutBool(const CNxNString& sParamName, bool bDefaultValue);
    int        GetOutCount();
    bool       GetOutAt(int iIndex, CNxNString& sParamName, CNxNString& sParamValue);

    // verbose level
	bool SetVerboseLevel(eVerboseLevel level);

    // access to events occurred during command execution
	bool WasSuccessful();
    int  GetEventCount();
	bool GetEventAt(int iIndex, CNxNCommandEvent& event);

    // customize progress bar
    bool SetProgressDialog(const CNxNString& sTitle, eProgressType type);

    // modify response behaviour
    bool HideResponseDialog(const CNxNString& sName, eResponseType defaultResponse);

    // only for internal usage
    void       SetPath(const CNxNString& sPath);

    bool       SetResult(const CNxNString& sResult);
    CNxNString GetParams() const;

private:
    bool LoadResult();

private:
    CNxNString        m_sCommand;
    long              m_lFlags;
    CNxNParamXML*     m_pParamXML;
    CNxNString        m_sCmdResult;
    CNxNString        m_sPath;
};


#endif // INC_NXN_VARCOMMAND_H
