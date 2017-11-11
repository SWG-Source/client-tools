// \addtodoc

#ifndef INC_NXN_ITEMCOLLECTION_H
#define INC_NXN_ITEMCOLLECTION_H


class CNxNItemCollectionImpl;

class NXNINTEGRATORSDK_API CNxNItemCollection: public CNxNObject
{
public:
    CNxNItemCollection();
    virtual ~CNxNItemCollection();

    void Init(CNxNItemCollectionImpl* pImpl);

	int GetCount() const;

	bool AddItem(const CNxNItem& Item);
	bool RemoveItem(const CNxNItem& Item);

    bool Clear();

	// Item enumeration
	bool GetItem(const CNxNString& sPath, CNxNItem& Item) const;

	bool GetFirstItem(CNxNItem& Item) const;
	bool GetNextItem(CNxNItem& Item) const;

	// Basic item information
	bool GetPath(CNxNString& sPath) const;
	bool GetLocalPath(CNxNString& sLocalPath) const;
    CNxNString GetPath() const;
    CNxNString GetType() const;

    eNxNBasicNamespaceType GetBasicType() const;

    bool IsFileFolder() const;
    bool IsFile() const;
    bool IsFolder() const;
    bool IsProject() const;
    bool IsDiskItem() const;
    bool IsDiskFile() const;
    bool IsDiskFolder() const;

    bool RunCommand(CNxNVarCommand& Command, HWND hWnd = NXN_HWND_DETECT);

private:
    CNxNItemCollectionImpl* m_pImpl;
};

#endif // INC_NXN_ITEMCOLLECTION_H
