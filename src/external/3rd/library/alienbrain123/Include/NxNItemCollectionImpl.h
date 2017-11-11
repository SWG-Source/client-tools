#ifndef INC_NXN_ITEMCOLLECTIONIMPL_H
#define INC_NXN_ITEMCOLLECTIONIMPL_H


class NXNINTEGRATORSDK_API CNxNItemCollectionImpl: public CNxNObject
{
public:
	virtual int GetCount() const = 0;

	virtual bool AddItem(const CNxNItem& Item) = 0;
	virtual bool RemoveItem(const CNxNItem& Item) = 0;

    virtual bool Clear() = 0;

	// Item enumeration
	virtual bool GetItem(const CNxNString& sPath, CNxNItem& Item) const = 0;

	virtual bool GetFirstItem(CNxNItem& Item) const = 0;
	virtual bool GetNextItem(CNxNItem& Item) const = 0;

    //virtual eNxNBasicNamespaceType GetBasicType() const = 0;
    virtual bool RunCommand(CNxNVarCommand& Command, HWND hWnd) = 0;
};

#endif // INC_NXN_ITEMCOLLECTIONIMPL_H
