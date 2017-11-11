#ifndef INC_NXN_ITEMLIST_H
#define INC_NXN_ITEMLIST_H

#include <map>


class NXNINTEGRATORSDK_API CNxNItemList: public CNxNItemCollectionImpl
{
public:
    int GetCount() const;

	bool AddItem(const CNxNItem& Item);
	bool RemoveItem(const CNxNItem& Item);

    bool Clear();

	// Item enumeration
	bool GetItem(const CNxNString& sPath, CNxNItem& Item) const;

	bool GetFirstItem(CNxNItem& Item) const;
	bool GetNextItem(CNxNItem& Item) const;

    bool RunCommand(CNxNVarCommand& Command, HWND hWnd);

private:
    typedef std::map<CNxNString, CNxNItem, CNxNString::PredCompareNoCaseLess> TItemMap;

    TItemMap m_mapItems;
};

#endif // INC_NXN_ITEMLIST_H
