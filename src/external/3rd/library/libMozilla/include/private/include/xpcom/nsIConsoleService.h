/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/xpcom/base/nsIConsoleService.idl
 */

#ifndef __gen_nsIConsoleService_h__
#define __gen_nsIConsoleService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsIConsoleListener_h__
#include "nsIConsoleListener.h"
#endif

#ifndef __gen_nsIConsoleMessage_h__
#include "nsIConsoleMessage.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIConsoleService */
#define NS_ICONSOLESERVICE_IID_STR "a647f184-1dd1-11b2-a9d1-8537b201161b"

#define NS_ICONSOLESERVICE_IID \
  {0xa647f184, 0x1dd1, 0x11b2, \
    { 0xa9, 0xd1, 0x85, 0x37, 0xb2, 0x01, 0x16, 0x1b }}

class NS_NO_VTABLE nsIConsoleService : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICONSOLESERVICE_IID)

  /* void logMessage (in nsIConsoleMessage message); */
  NS_IMETHOD LogMessage(nsIConsoleMessage *message) = 0;

  /**
     * Convenience method for logging simple messages.
     */
  /* void logStringMessage (in wstring message); */
  NS_IMETHOD LogStringMessage(const PRUnichar *message) = 0;

  /**
     * Get an array of all the messages logged so far.  If no messages
     * are logged, this function will return a count of 0, but still
     * will allocate one word for messages, so as to show up as a
     * 0-length array when called from script.
     */
  /* void getMessageArray ([array, size_is (count)] out nsIConsoleMessage messages, out PRUint32 count); */
  NS_IMETHOD GetMessageArray(nsIConsoleMessage ***messages, PRUint32 *count) = 0;

  /**
     * To guard against stack overflows from listeners that could log
     * messages (it's easy to do this inadvertently from listeners
     * implemented in JavaScript), we don't call any listeners when
     * another error is already being logged.
     */
  /* void registerListener (in nsIConsoleListener listener); */
  NS_IMETHOD RegisterListener(nsIConsoleListener *listener) = 0;

  /**
     * Each registered listener should also be unregistered.
     */
  /* void unregisterListener (in nsIConsoleListener listener); */
  NS_IMETHOD UnregisterListener(nsIConsoleListener *listener) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSICONSOLESERVICE \
  NS_IMETHOD LogMessage(nsIConsoleMessage *message); \
  NS_IMETHOD LogStringMessage(const PRUnichar *message); \
  NS_IMETHOD GetMessageArray(nsIConsoleMessage ***messages, PRUint32 *count); \
  NS_IMETHOD RegisterListener(nsIConsoleListener *listener); \
  NS_IMETHOD UnregisterListener(nsIConsoleListener *listener); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSICONSOLESERVICE(_to) \
  NS_IMETHOD LogMessage(nsIConsoleMessage *message) { return _to LogMessage(message); } \
  NS_IMETHOD LogStringMessage(const PRUnichar *message) { return _to LogStringMessage(message); } \
  NS_IMETHOD GetMessageArray(nsIConsoleMessage ***messages, PRUint32 *count) { return _to GetMessageArray(messages, count); } \
  NS_IMETHOD RegisterListener(nsIConsoleListener *listener) { return _to RegisterListener(listener); } \
  NS_IMETHOD UnregisterListener(nsIConsoleListener *listener) { return _to UnregisterListener(listener); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSICONSOLESERVICE(_to) \
  NS_IMETHOD LogMessage(nsIConsoleMessage *message) { return !_to ? NS_ERROR_NULL_POINTER : _to->LogMessage(message); } \
  NS_IMETHOD LogStringMessage(const PRUnichar *message) { return !_to ? NS_ERROR_NULL_POINTER : _to->LogStringMessage(message); } \
  NS_IMETHOD GetMessageArray(nsIConsoleMessage ***messages, PRUint32 *count) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMessageArray(messages, count); } \
  NS_IMETHOD RegisterListener(nsIConsoleListener *listener) { return !_to ? NS_ERROR_NULL_POINTER : _to->RegisterListener(listener); } \
  NS_IMETHOD UnregisterListener(nsIConsoleListener *listener) { return !_to ? NS_ERROR_NULL_POINTER : _to->UnregisterListener(listener); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsConsoleService : public nsIConsoleService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICONSOLESERVICE

  nsConsoleService();

private:
  ~nsConsoleService();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsConsoleService, nsIConsoleService)

nsConsoleService::nsConsoleService()
{
  /* member initializers and constructor code */
}

nsConsoleService::~nsConsoleService()
{
  /* destructor code */
}

/* void logMessage (in nsIConsoleMessage message); */
NS_IMETHODIMP nsConsoleService::LogMessage(nsIConsoleMessage *message)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void logStringMessage (in wstring message); */
NS_IMETHODIMP nsConsoleService::LogStringMessage(const PRUnichar *message)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getMessageArray ([array, size_is (count)] out nsIConsoleMessage messages, out PRUint32 count); */
NS_IMETHODIMP nsConsoleService::GetMessageArray(nsIConsoleMessage ***messages, PRUint32 *count)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void registerListener (in nsIConsoleListener listener); */
NS_IMETHODIMP nsConsoleService::RegisterListener(nsIConsoleListener *listener)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void unregisterListener (in nsIConsoleListener listener); */
NS_IMETHODIMP nsConsoleService::UnregisterListener(nsIConsoleListener *listener)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

#define NS_CONSOLESERVICE_CLASSNAME "Console Service"
#define NS_CONSOLESERVICE_CID \
{ 0x7e3ff85c, 0x1dd2, 0x11b2, { 0x8d, 0x4b, 0xeb, 0x45, 0x2c, 0xb0, 0xff, 0x40 }}
#define NS_CONSOLESERVICE_CONTRACTID "@mozilla.org/consoleservice;1"

#endif /* __gen_nsIConsoleService_h__ */
