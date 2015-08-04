#ifndef INC_NXN_SELECTION_H
#define INC_NXN_SELECTION_H


class NXNINTEGRATORSDK_API CNxNSelection: public CNxNItemCollectionImpl
{
public:
    CNxNSelection();
    virtual ~CNxNSelection();

	int GetCount() const;

	bool AddItem(const CNxNItem& Item);
	bool RemoveItem(const CNxNItem& Item);

    bool Clear();

	// Item enumeration
	bool GetItem(const CNxNString& sPath, CNxNItem& Item) const;

	bool GetFirstItem(CNxNItem& Item) const;
	bool GetNextItem(CNxNItem& Item) const;

    //eNxNBasicNamespaceType GetType() const;
    bool RunCommand(CNxNVarCommand& Command, HWND hWnd);
};

#endif // INC_NXN_SELECTION_H
