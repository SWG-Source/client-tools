// \addtodoc

#ifndef INC_NXN_NAMESPACEITEM_H
#define INC_NXN_NAMESPACEITEM_H



class NXNINTEGRATORSDK_API CNxNItem: public CNxNObject
{
    friend class CNxNSelection;
    friend class CNxNIntegrator;

public:
	// Construction / destruction
	CNxNItem();
	virtual ~CNxNItem();

	// Validity
	bool IsValid() const;

	// Item enumeration
	int GetChildCount() const;
	bool GetChild(const CNxNString& sName, CNxNItem& Child) const;

	bool GetFirstChild(CNxNItem& Child) const;
	bool GetNextChild(CNxNItem& Child) const;

	bool GetParent(CNxNItem& Parent) const;

	// Basic item information
	CNxNString GetName() const;
	CNxNString GetType() const;
	CNxNString GetPath() const;
	CNxNString GetLocalPath() const;
	bool GetName(CNxNString& sName) const;
	bool GetType(CNxNString& sType) const;
	bool GetPath(CNxNString& sPath) const;
	bool GetLocalPath(CNxNString& sLocalPath) const;

    eNxNBasicNamespaceType GetBasicType() const;

    bool IsFileFolder() const;
    bool IsFile() const;
    bool IsFolder() const;
    bool IsProject() const;
    bool IsDiskItem() const;
    bool IsDiskFile() const;
    bool IsDiskFolder() const;

	long GetFlags() const;

	// Commands
	bool RunCommand(CNxNVarCommand& Command, HWND hWnd = NXN_HWND_DETECT);
	bool RunCommand(CNxNCommand& Command);

	bool Rename(const CNxNString& sNewName);
	bool Delete(bool bDeleteLocalCopy, bool bDeleteServerCopy);
	bool Copy(const CNxNString& sDestPath, const CNxNString& sOtherParams = L"");
	bool Move(const CNxNString& sDestPath, const CNxNString& sOtherParams = L"");
	bool GetLatest(const CNxNString& sParams = L"");
	bool CheckOut(const CNxNString& sParams = L"");
	bool UndoCheckOut(const CNxNString& sParams = L"");
	bool CheckIn(const CNxNString& sParams = L"");
	bool Import(const CNxNString& sParams = L"");
	bool Import(const CNxNString& sLocalPath, const CNxNString& sOtherParams = L"");
	bool CreateFile(const CNxNString& sName, const CNxNString& sOtherParams = L"");
	bool CreateFolder(const CNxNString& sName, const CNxNString& sOtherParams = L"");
	bool SetWorkingPath(const CNxNString& sWorkingPath, const CNxNString& sOtherParams = L"");

    // Properties
	CNxNString GetProperty(const CNxNString& sPropName, long* pFlags = NULL) const;
	bool GetProperty(const CNxNString& sPropName, CNxNString& sPropValue, long* pFlags = NULL) const;
	bool SetProperty(const CNxNString& sPropName, const CNxNString& sPropValue, long* pFlags = NULL);
    bool GetProperty(CNxNProperty& Property) const;
    bool SetProperty(const CNxNProperty& Property);

    bool GetProperties(int iPropCount,
                       const CNxNString aPropNames[],
                       long lPropFlags,
                       CNxNString aPropValues[]);
	bool GetProperties(CNxNPropertyCollection& PropCollection, long lPropFlags);

public:
    static bool GetItem(const CNxNString& sPath, CNxNItem& Item);

private:
    void Reset();
    void Init(const CNxNString& sPath);

private:
    CNxNString    m_sPath;
};

#endif // INC_NXN_NAMESPACEITEM_H