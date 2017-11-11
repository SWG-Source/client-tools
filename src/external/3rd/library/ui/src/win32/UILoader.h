#ifndef __UILOADER_H__
#define __UILOADER_H__

#include "UIBaseObject.h"
#include "UILoaderExtension.h"
#include "UIOutputStream.h"
#include "UITypes.h"

#include <list>
#include <map>
#include <stack>
#include <vector>
#include <unordered_set>

struct UILoaderToken;
class UICanvas;
class UIPage;

// Loader
// =====================================

class UILoader
{
public:

	typedef std::list<UIBaseObject *> UIObjectList;

	           UILoader  ();
	virtual   ~UILoader  ();

	void           AddToken     (const char * const name , const UILoaderExtension *Constructor );
	UIBaseObject * CreateObject (const char * const name);

	virtual bool  LoadFromResource       (const std::string &, UIObjectList &, bool SetSourceFileProperty = false);
	virtual bool  LoadFromString         (const std::string &, UIObjectList &, bool SetSourceFileProperty = false);
	virtual bool  LoadFromToken          (const UILoaderToken &, UIBaseObject *&, bool SetSourceFileProperty = false);
	virtual bool  LoadStringFromResource (const std::string &, std::string &);

	UIPage       *LoadRootPage           (const std::string &);

	void          Lint                   () const;

private:

	virtual	bool AddToCurrentContainer   (const UILoaderToken &T, UIBaseObject *NewObject );

	typedef std::map<const char * const, UILoaderExtension const *, CompareNoCase> UIConstructorMap;
	UIConstructorMap mConstructorMap;


	typedef std::vector <std::string> UINarrowStringVector;
	UINarrowStringVector      mIncludes;

	typedef std::unordered_set<std::string> UINarrowStringHashSet;
	UINarrowStringHashSet mIncludeHistory;

	typedef std::vector<UIBaseObject *> ObjectVector;
	ObjectVector           mContainerStack;

	typedef std::stack<std::string, std::vector<std::string> > NarrowStringStack;
	NarrowStringStack     mContainerTypeStack;
	NarrowStringStack     mContainerNameStack;

public:

};

//----------------------------------------------------------------------

#endif // __UILOADER_H__
