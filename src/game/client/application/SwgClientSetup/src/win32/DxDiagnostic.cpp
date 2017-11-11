// ======================================================================
//
// DxDiagnostic.cpp
// vthakkar
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "DxDiagnostic.h"

// ======================================================================

DxDiagnostic::DxDiagnostic()
{
	m_pDxDiagProvider           = NULL;
    m_pDxDiagContainerRoot      = NULL;
	m_pDxDiagContainerCurrent   = NULL;
	m_bRemoveCOM                = false;
}

// ----------------------------------------------------------------------

DxDiagnostic::~DxDiagnostic()
{
	DxDiagnostic::destroy();
}

// ----------------------------------------------------------------------

// method: initialize
// effect: creates DxDiag COM object
// arguments: boolean whether to allow WHQL checks or not
// return: true on success, false on failure
bool DxDiagnostic::initialize( bool bAllowWHQLChecks )
{
	// don't allow initialize to be called twice without a destroy() being called first
	if(m_bRemoveCOM)
		return false;

	unsigned long lresult;

    lresult = CoInitialize(NULL);
    m_bRemoveCOM = SUCCEEDED(lresult);

	// Create the COM object
    lresult = CoCreateInstance(CLSID_DxDiagProvider, NULL, CLSCTX_INPROC_SERVER, IID_IDxDiagProvider, (LPVOID*) &m_pDxDiagProvider);
    
	// Verify success or exit
	if(FAILED(lresult) || m_pDxDiagProvider == NULL)
        return false;

    // Fill out a DXDIAG_INIT_PARAMS struct and pass it to IDxDiagContainer::Initialize
	// Passing in true for bAllowWHQL checks updates the WHQL database from the internet
	// and normally runs much slower
    DXDIAG_INIT_PARAMS dxDiagInitializeParameter;
    ZeroMemory(&dxDiagInitializeParameter, sizeof(DXDIAG_INIT_PARAMS));

    dxDiagInitializeParameter.dwSize                  = sizeof(DXDIAG_INIT_PARAMS);
    dxDiagInitializeParameter.dwDxDiagHeaderVersion   = DXDIAG_DX9_SDK_VERSION;
    dxDiagInitializeParameter.bAllowWHQLChecks        = bAllowWHQLChecks;
    dxDiagInitializeParameter.pReserved               = NULL;

    lresult = m_pDxDiagProvider->Initialize(&dxDiagInitializeParameter);

    if(FAILED(lresult))
        return false;

    lresult = m_pDxDiagProvider->GetRootContainer(&m_pDxDiagContainerRoot);
    
	if(SUCCEEDED(lresult))
		m_pDxDiagContainerCurrent = m_pDxDiagContainerRoot;

    return SUCCEEDED(lresult);
}

// ----------------------------------------------------------------------

// method: destroy
// effect: destroys DxDiag COM object
// arguments: none
// return: none
void DxDiagnostic::destroy()
{
	if(m_pDxDiagProvider)			
	{ 
		m_pDxDiagProvider->Release();				
		m_pDxDiagProvider = NULL;			
	}

	if(m_pDxDiagContainerRoot)		
	{ 
		m_pDxDiagContainerRoot->Release();		
		
		if(m_pDxDiagContainerRoot == m_pDxDiagContainerCurrent)
			m_pDxDiagContainerCurrent = NULL;

		m_pDxDiagContainerRoot = NULL;		
	}

	if(m_pDxDiagContainerCurrent)	
	{ 
		m_pDxDiagContainerCurrent->Release();		
		m_pDxDiagContainerCurrent = NULL;	
	}

	if(m_bRemoveCOM)				
	{ 
		CoUninitialize();	
		m_bRemoveCOM = false;
	}
}

// ----------------------------------------------------------------------

// method: resetToRootContainer
// effect: reset the current container to the root of DxDiag
// arguments: none
// return: true on success, false on failure
bool DxDiagnostic::resetToRootContainer()
{
    if(m_pDxDiagProvider == NULL)
        false;

	if(m_pDxDiagContainerCurrent != NULL && m_pDxDiagContainerCurrent != m_pDxDiagContainerRoot)	
		m_pDxDiagContainerCurrent->Release();		

	m_pDxDiagContainerCurrent = m_pDxDiagContainerRoot;

	return true;
}

// ----------------------------------------------------------------------

// method: getNumberOfContainers
// effect: stores number of containers inside current container in referenced unsigned long
// arguments: reference to unsigned long (out)
// return: true on success, false on failure
bool DxDiagnostic::getNumberOfContainers( unsigned long& numContainers )
{
    if(m_pDxDiagProvider == NULL)
        return false;

    return SUCCEEDED(m_pDxDiagContainerCurrent->GetNumberOfChildContainers(&numContainers));
}

// ----------------------------------------------------------------------

// method: enumContainerNames
// effect: get name of container based on index under current container in LWPSTR argument
// arguments: index to enumerate, container name buffer (out), size of container name buffer
// return: true on success, false on failure
bool DxDiagnostic::enumContainerNames( unsigned int index, LPWSTR containerName, unsigned int containerNameSize )
{
    if(m_pDxDiagProvider == NULL)
        return false;

	return SUCCEEDED(m_pDxDiagContainerCurrent->EnumChildContainerNames(index, containerName, containerNameSize));
}

// ----------------------------------------------------------------------

// method: changeToChildContainer
// effect: change the current container to given child container
// arguments: container name to change to 
// return: true on success, false on failure
bool DxDiagnostic::changeToChildContainer( LPWSTR containerName )
{
	// Can change to nested containers - seperated by a '.'
	// eg. L"DxDiag_DirectSound.DxDiag_SoundDevices"
    if(m_pDxDiagProvider == NULL)
        return false;

	unsigned long lresult;
	IDxDiagContainer* newContainer;

	if(SUCCEEDED(lresult = m_pDxDiagContainerCurrent->GetChildContainer(containerName, &newContainer)))
		m_pDxDiagContainerCurrent = newContainer;

	return SUCCEEDED(lresult);
}

// ----------------------------------------------------------------------

// method: getNumberOfProperties
// effect: get number of properties in current container
// arguments: unsigned long reference to get number of properties
// return: true on success, false on failure
bool DxDiagnostic::getNumberOfProperties( unsigned long& numProperties )
{
    if(m_pDxDiagProvider == NULL)
        return false;

    return SUCCEEDED(m_pDxDiagContainerCurrent->GetNumberOfProps(&numProperties));	
}

// ----------------------------------------------------------------------

// method: enumPropertyNames
// effect: get name of property based on index under current container in LWPSTR argument
// arguments: index to enumerate, container name buffer (out), size of container name buffer
// return: true on success, false on failure
bool DxDiagnostic::enumPropertyNames( unsigned int index, LPWSTR propertyName, unsigned int propertyNameSize )
{
    if(m_pDxDiagProvider == NULL)
        return false;

	return SUCCEEDED(m_pDxDiagContainerCurrent->EnumPropNames(index, propertyName, propertyNameSize));
}

// ----------------------------------------------------------------------

// method: getPropertyType
// effect: get the type of the property specified by property name
// arguments: property name, reference to a PropertyType (out)
// return: true on success, false on failure
bool DxDiagnostic::getPropertyType( LPWSTR propertyName, PropertyType& type)
{
    if(m_pDxDiagProvider == NULL)
        false;

    VARIANT var;
    VariantInit(&var);

    if(FAILED(m_pDxDiagContainerCurrent->GetProp(propertyName, &var)))
        return false;

    if(var.vt == VT_BOOL)
        type = PROP_TYPE_BOOL;
	else if(var.vt == VT_I4)
		type = PROP_TYPE_INT;
	else if(var.vt == VT_UI4)
		type = PROP_TYPE_UINT;
	else if(var.vt == VT_BSTR)
		type = PROP_TYPE_STR_OR_64INT;
	else
		type = PROP_TYPE_UNKNOWN;

	return true;
}

// ----------------------------------------------------------------------

// method: getBooleanProperty
// effect: get the the boolean property specified by property name
// arguments: property name, reference to a bool (out)
// return: true on success, false on failure
bool DxDiagnostic::getBooleanProperty( LPWSTR propertyName, bool& value )
{
    if(m_pDxDiagProvider == NULL)
        false;

    VARIANT var;
    VariantInit(&var);

    if(FAILED(m_pDxDiagContainerCurrent->GetProp(propertyName, &var)) || var.vt != VT_BOOL)
        return false;

	value = (var.boolVal != 0);
    VariantClear(&var);

    return true;
}

// ----------------------------------------------------------------------

// method: getIntegerProperty
// effect: get the the int property specified by property name
// arguments: property name, reference to an int (out)
// return: true on success, false on failure
bool DxDiagnostic::getIntegerProperty( LPWSTR propertyName, int& value )
{
    if(m_pDxDiagProvider == NULL)
        false;

    VARIANT var;
    VariantInit(&var);

    if(FAILED(m_pDxDiagContainerCurrent->GetProp(propertyName, &var)) || var.vt != VT_I4)
        return false;

    value = var.lVal;
    VariantClear(&var);

    return true;
}

// ----------------------------------------------------------------------

// method: getUnsignedIntegerProperty
// effect: get the the unsigned int property specified by property name
// arguments: property name, reference to an unsigned int (out)
// return: true on success, false on failure
bool DxDiagnostic::getUnsignedIntegerProperty( LPWSTR propertyName, unsigned int& value )
{
    if(m_pDxDiagProvider == NULL)
        false;

    VARIANT var;
    VariantInit(&var);

    if(FAILED(m_pDxDiagContainerCurrent->GetProp(propertyName, &var)) || var.vt != VT_UI4)
        return false;

    value = var.ulVal;
    VariantClear(&var);

    return true;
}

// ----------------------------------------------------------------------

// method: getStringProperty
// effect: get the the string property specified by property name
// arguments: property name, pointer to a character string (out), size of character buffer argument
// return: true on success, false on failure
bool DxDiagnostic::getStringProperty( LPWSTR propertyName, char* outputString, int bufferSize )
{
    if(m_pDxDiagProvider == NULL)
        false;

    VARIANT var;
    VariantInit(&var);

    if(FAILED(m_pDxDiagContainerCurrent->GetProp(propertyName, &var)) || var.vt != VT_BSTR)
        return false;

    wcstombs(outputString, var.bstrVal, bufferSize);   
    outputString[bufferSize - 1] = TEXT('\0');

    VariantClear(&var);

    return true;
}

// ----------------------------------------------------------------------

// method: getUnsignedInteger64Property
// effect: get the the unsigned 64 bit int property specified by property name
// arguments: property name, referemce to a 64 bit int (out)
// return: true on success, false on failure
bool DxDiagnostic::getUnsignedInteger64Property( LPWSTR propertyName, _int64& value )
{
    if(m_pDxDiagProvider == NULL)
        false;

    VARIANT var;
    VariantInit(&var);

    // 64-bit values are stored as strings in BSTRs
	if(FAILED(m_pDxDiagContainerCurrent->GetProp(propertyName, &var)) || var.vt != VT_BSTR)
        return false;

    value = _wtoi64(var.bstrVal);    
	VariantClear(&var);

    return true;
}
