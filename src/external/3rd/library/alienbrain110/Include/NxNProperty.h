// \addtodoc

#ifndef INC_NXN_PROPERTY_H
#define INC_NXN_PROPERTY_H

/* \class		CNxNProperty
 *
 *	\file		NxNProperty.h
 *
 *  \brief		This class is meant to set/get properties for objects using the methods
 *				SetProperty() or GetProperty(). CNxNProperty contains all necessary informations
 *				about the property itself. All informations can be modified at runtime so that
 *				a property object can be reused several times.
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-05-02-2000 file created.
 *	\endmod
 */

//---------------------------------------------------------------------------
//	hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNPropertyData*	tNxNPropertyData;

class NXNINTEGRATORSDK_API CNxNProperty : public CNxNObject {
	NXN_DECLARE_DYNCREATE(CNxNProperty);

	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNProperty();
		CNxNProperty(const CNxNString& strPropName, const CNxNString& strValue, long lFlags);
		CNxNProperty(const CNxNString& strPropName, long lValue, long lFlags);
		CNxNProperty(const CNxNString& strPropName, bool bValue, long lFlags);
		CNxNProperty(const CNxNString& strPropName, long lFlags);

		virtual ~CNxNProperty();

		//---------------------------------------------------------------------------
		//	data modification methods
		//---------------------------------------------------------------------------
		void SetName(const CNxNString& strPropName);
		void SetValue(const CNxNString& strValue);
		void SetValue(long lValue);
		void SetBoolValue(bool bValue);

		void SetFlags(long lFlags);
		void AddFlags(long lFlags);
		void RemoveFlags(long lFlags);

		//---------------------------------------------------------------------------
		//	data retrieval methods
		//---------------------------------------------------------------------------
		CNxNString GetName() const;
		CNxNString GetString() const;
		int GetInt() const;
		long GetLong() const;
		bool GetBool(bool bDefault = false) const;
		long GetFlags() const;

		//---------------------------------------------------------------------------
		//	direct access to property data
		//---------------------------------------------------------------------------
		inline tNxNPropertyData GetPropertyData() const { return m_pPropertyData; };

	private:
		tNxNPropertyData	m_pPropertyData;
};

#endif // INC_NXN_PROPERTY_H
