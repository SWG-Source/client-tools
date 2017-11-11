#ifndef INC_NXN_NAMESPACE_H
#define INC_NXN_NAMESPACE_H

/* \class		CNxNNamespaceApi NxNNamespaceApi.h
 *
 *  \brief		This class encapsulates all medializer interface specific calls and
 *				contains static methods only!
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-29-Aug-2000 file created.
 *	\endmod
 */

#include "NxNApi.h"

class CNxNNamespaceApi {
	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNNamespaceApi();
		virtual ~CNxNNamespaceApi();

		//---------------------------------------------------------------------------
		//	informations about api
		//---------------------------------------------------------------------------
		static const char* GetApiName();

		//---------------------------------------------------------------------------
		//	Namespace api install check
		//---------------------------------------------------------------------------
		static bool Installed();

		//---------------------------------------------------------------------------
		//	namespace creation methods
		//---------------------------------------------------------------------------
		static HRESULT RequestNamespace(INxNNamespace*& pNamespace, INxNNamespace_Internal*& pNamespaceInternal);
		static HRESULT ReleaseNamespace(INxNNamespace*& pNamespace, INxNNamespace_Internal*& pNamespaceInternal);

		//---------------------------------------------------------------------------
		//	response object creation methods
		//---------------------------------------------------------------------------
		static HRESULT RequestResponseObject(INxNResponseObject*& pResponseObject);
		static HRESULT ReleaseResponseObject(INxNResponseObject*& pResponseObject);

		//---------------------------------------------------------------------------
		//	class id's
		//---------------------------------------------------------------------------
		static const IID IID_INxNNamespacePluginHelper;
		static const CLSID CLSID_NxNNamespacePluginHelper;
};

#endif // INC_NXN_NAMESPACEAPI_H
