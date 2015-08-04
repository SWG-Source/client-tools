#ifndef INCLUDED_ObjectBrowser_H
#define INCLUDED_ObjectBrowser_H

class UIBaseObject;
class ObjectBrowser
{
public:

   // lock the dialog to temporarily prevent redraws during periods where numerous events will be generated.
   virtual void lock()=0; 

   // unlock the dialog to allow redraws. lock and unlock are reference-count based to each call to lock
   // must have a corresponding call to unlock.
   virtual void unlock()=0;

	virtual void onObjectChange(int action, UIBaseObject *object, uint32 param)=0;
};

#endif
