// ======================================================================
//
// DxDiagnostic.h
// vthakkar
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DXDIAGNOSTIC_H
#define INCLUDED_DXDIAGNOSTIC_H


// ======================================================================

#include <dxdiag.h>

// ======================================================================

class DxDiagnostic
{
public:
	enum PropertyType
	{
		PROP_TYPE_BOOL          = 1,
		PROP_TYPE_INT           = 2,
		PROP_TYPE_UINT          = 3,
		PROP_TYPE_STR_OR_64INT  = 4,
		PROP_TYPE_UNKNOWN       = 5
	};

	DxDiagnostic();
	~DxDiagnostic();

	bool initialize( bool bAllowWHQLChecks );
	void destroy();

	bool resetToRootContainer();
	bool getNumberOfContainers( unsigned long& pnumContainers );
	bool enumContainerNames( unsigned int index, LPWSTR containerName, unsigned int containerSize );
	bool changeToChildContainer( LPWSTR containerName );

	bool getNumberOfProperties( unsigned long& pnumProperties );
	bool enumPropertyNames( unsigned int index, LPWSTR propertyName, unsigned int propertySize );

	bool getPropertyType( LPWSTR propertyName, PropertyType& type);
	bool getBooleanProperty( LPWSTR propertyName, bool& value );
	bool getIntegerProperty( LPWSTR propertyName, int& value );
	bool getUnsignedIntegerProperty( LPWSTR propertyName, unsigned int& value );
	bool getStringProperty( LPWSTR propertyName, char* outputString, int value );
	bool getUnsignedInteger64Property( LPWSTR propertyName, _int64& pulinteger );

private:

	IDxDiagProvider*    m_pDxDiagProvider;
	IDxDiagContainer*   m_pDxDiagContainerRoot;
	IDxDiagContainer*   m_pDxDiagContainerCurrent;
	bool                m_bRemoveCOM;
};

// ======================================================================

#endif
