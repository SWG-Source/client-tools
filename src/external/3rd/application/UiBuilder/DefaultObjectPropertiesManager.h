#ifndef __DEFAULTPROPERTYSETTINGS_H__
#define __DEFAULTPROPERTYSETTINGS_H__

#include "UIBaseObject.h"

#include <map>
#include <unordered_map>
#include <list>

class DefaultObjectPropertiesManager
{
public:
	
	typedef std::unordered_map<UINarrowString, UIString> StringMap;
	
	struct DefaultObjectProperties
	{
		UITypeID        TypeID;
		UINarrowString  TypeName;
		StringMap       Properties;
	};
	
	typedef std::list<DefaultObjectProperties> DefaultObjectPropertiesList;


	 DefaultObjectPropertiesManager();			
	 void ApplyDefaultPropertiesToObject( UIBaseObject * ) const;
	 void DisplayEditDialog( HWND hParent );
	 
	 void SaveTo( FILE * );
	 void LoadFrom( FILE * );

private:

	void LoadDataToDialog( HWND );
	void LoadDefaultsToListbox( HWND );

	void HandleOwnerDraw( HWND hwndDlg, WPARAM wParam, LPARAM lParam );
	void AddNewProperty( HWND hwndDlg );
	void RemoveSelectedProperty( HWND hwndDlg );

	static BOOL CALLBACK DefaultObjectPropertiesManagerStaticWindowProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	BOOL DefaultObjectPropertiesManagerWindowProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

	DefaultObjectPropertiesList mDefaultObjectProperties;
	DefaultObjectPropertiesList mWorkingObjectProperties;
	StringMap                  *mSelectedPropertyMap;
	HWND                        mPropertyListbox;
};

#endif /* __DEFAULTPROPERTYSETTINGS_H__ */