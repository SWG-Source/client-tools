// \addtodoc

#ifndef INC_NXN_PROPERTY_H
#define INC_NXN_PROPERTY_H

/* \class		CNxNProperty NxNProperty.h
 *
 *  \brief		This class defines properties that can be set and retrieved for namespace and database items
 *              using CNxNNode::SetProperty() and CNxNNode::GetProperty().
 *
 *  \author		Axel Pfeuffer, Helmut Klinger, (c) 1999-2001 by NxN Software AG
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-02-May-2000 file created.
 *	\endmod
 */


//---------------------------------------------------------------------------
//	typedef for a property list
//---------------------------------------------------------------------------
class CNxNProperty;
class CNxNNamespaceProxy;
class CNxNMenuProxy;

typedef CNxNArray<CNxNProperty>           TNxNPropertyList;

class NXNINTEGRATORSDK_API CNxNProperty : public CNxNObject
{
	NXN_DECLARE_DYNCREATE(CNxNProperty);

    friend CNxNResponse;
    friend CNxNNamespaceProxy;
    friend CNxNMenuProxy;

	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNProperty();
        CNxNProperty(const CNxNProperty& propSrc);
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
		void SetHugeIntValue(TNxNHugeInt hiValue);
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
        TNxNHugeInt GetHugeInt() const;
		long GetFlags() const;

        long GetAsStringList(TNxNStringList& aStringList, char cSeparator = '|') const;

		//---------------------------------------------------------------------------
		//	assignment operators
		//---------------------------------------------------------------------------
  		const CNxNProperty& operator =(const CNxNProperty& propSourceToCopy);

        const CNxNProperty& operator =(const CNxNString& strValue);
  		const CNxNProperty& operator =(long lValue);
  		const CNxNProperty& operator =(bool bValue);
  		const CNxNProperty& operator =(TNxNHugeInt hiValue);

		//---------------------------------------------------------------------------
		//	special value cast operators
		//---------------------------------------------------------------------------
		operator const CNxNString&() const;

        /*!	\fn			inline operator int() const
         *	
         *				Returns the value of the property as an integer.
         *	
         *	\param		none
         *	
         *	\return		integer containing the property value
         *	
         *	\note		
         */
		inline operator int() const
        {
            return GetInt();
        }


        /*!	\fn			inline operator long() const
         *	
         *				Returns the value of the property as a long.
         *	
         *	\param		none
         *	
         *	\return		long containing the property value as a long
         *	
         *	\note		
         */
		inline operator long() const
        {
            return GetLong();
        }


        /*!	\fn			inline operator bool() const
         *	
         *				Returns the value of the property as a boolean.
         *	
         *	\param		none
         *	
         *	\return		boolean containing the property value
         *	
         *	\note		
         */
		inline operator bool() const
        {
            return GetBool();
        }

        /*!	\fn			inline operator TNxNHugeInt() const
         *	
         *				Returns the value of the property as a 64-bit integer.
         *	
         *	\param		none
         *	
         *	\return		64 bit integer containing the property value
         *	
         *	\note		
         */
		inline operator TNxNHugeInt() const
        {
            return GetHugeInt();
        }


	private:
	    CNxNString	m_strPropName;
	    CNxNString	m_strValue;
	    long		m_lFlags;
};

#endif // INC_NXN_PROPERTY_H
