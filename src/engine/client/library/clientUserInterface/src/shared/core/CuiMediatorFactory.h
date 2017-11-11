// ======================================================================
//
// CuiMediatorFactory.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiMediatorFactory_H
#define INCLUDED_CuiMediatorFactory_H

class CuiMediator;
class UIPage;

// ======================================================================

class CuiMediatorFactory
{
public:

	class ConstructorBase;
	template <typename T> class Constructor;

	static void                      install                  ();
	static void                      remove                   ();

	static ConstructorBase *         findConstructor          (const char * namePtr);
	static CuiMediator *             create                   (const char * namePtr, UIPage & page);
	static CuiMediator *             get                      (const char * namePtr, bool create = true);

	template <typename T> static T * getType  (const char * namePtr)
	{
		return dynamic_cast<T *> (get (namePtr));
	}

	static bool                      isValidName               (const char * namePtr);
	static CuiMediator *             activate                  (const char * namePtr, const char * previousMediatorNamePtr = 0, bool create = true);
	static bool                      deactivate                (const char * namePtr, bool create = true);
	static bool                      toggle                    (const char * namePtr);
	static void                      deactivateAll             ();
	static bool                      attemptRelease            (const char * namePtr);
	static void                      addConstructor            (const char * namePtr, ConstructorBase * constructor);

	static void                      getMediatorNames          (stdvector<std::string>::fwd & result);
	static void                      getMediators              (stdvector<CuiMediator *>::fwd & result);

	static bool                      findMediatorCanonicalName (const std::string & name, std::string & canonicalName);

	static CuiMediator *             activateInWorkspace       (const char * namePtr, bool ignoreExisting = false, bool settingsAutoSizeLocation = true);
	static CuiMediator *             deactivateInWorkspace     (const char * namePtr);
	static CuiMediator *             toggleInWorkspace         (const char * namePtr);
	static CuiMediator *             getInWorkspace            (const char * namePtr, bool create = false, bool ignoreExisting = false, bool settingsAutoSizeLocation = true);

	static bool                      test                      (std::string & result);

	static bool                      isInstalled               ();

	static void                      clearActivatedThisFrame   ();

private:

	CuiMediatorFactory (const CuiMediatorFactory & rhs);
	CuiMediatorFactory & operator= (const CuiMediatorFactory & rhs);

	static void          deleteMediators ();
	static bool          ms_installed;
};

//----------------------------------------------------------------------

inline bool CuiMediatorFactory::isInstalled ()
{
	return ms_installed;
}

// ======================================================================

#endif
