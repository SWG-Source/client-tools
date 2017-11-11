#ifndef INC_NXN_NXNSTRING_H_
#define INC_NXN_NXNSTRING_H_


//
// This file was changed during internationalization on 12/18/2000 by Jens.


#include "NxNExport.h"
#include <atlbase.h> // A2WBSTR
#include <functional> // binary_function
#include "macros.h"

/* \fn CNxNString CNxNPath::GetNamespacePart(int xPart, bool bWithPreceedingSlash) const
	\param xPart 0 based part of the Namespace path
	\param bWithPreceedingSlash the result will have an preceeding backlash (default = false)
	\return part text or empty string

  	<dl compact><dt><b>Example:</b>
	<dd>
	\code
	"\\A\\B".GetNamespacePart(0,true) "\\A";
	"\\A\\B".GetNamespacePart(1) returns "B";
	"\\A\\B".GetNamespacePart(2) returns "";
	\endcode
	</dl>

	<dl compact><dt><b>Note:</b>
	This method works only on namespace paths, which have a preceeding slash and all
	slashes are backslashes
	</dl>		
*/



/*! \class CNxNString NxNBSTR.h
\brief This is a string class combining the advantages of BSTR and CString classes.
See the implementation for details.

<dl compact><dt><b>Note:</b>
<dd>
	Some information about different types of strings:
<ul>
<li>BSTR: wide char string with leading length field (size_type) and by default terminated with '\0'
<ul><li>allocation:		
	<ul><li>dynamic: different versions of ::SysAllocString (also appends a terminating L'\0') only
		<li>static:  -
	</ul>
	<li>deallocation:	
	<ul><li>dynamic: ::SysFreeString()
	</ul>
</ul>

<li>LPWSTR: wide-character string (UNICODE 2 bytes per character) terminated with '\0'
<ul><li>allocation:		
	<ul><li>dynamic: new wchar[], malloc() 
		<li>static:  L"this is a static wide-character string"
	</ul>
	<li>deallocation:		
	<ul><li>dynamic: delete [], free()
	</ul>
</ul>

<li>LPSTR: single byte char string (ANSI 1 byte per character) terminated with '\0'
<ul><li>allocation:		
	<ul><li>dynamic: new char[], malloc() 
		<li>static:  "this is a static single character string"
  	</ul>
 	<ul><li>dynamic: delete [], free()
	</ul>
</ul>

<li>LPTSTR: single byte or wide-character string, depending on build options (default ANSI, optional UNICODE)
<ul><li>allocation:		
	<ul><li>dynamic: new TCHAR[], malloc() 
		<li>static:  _T("this is a static wide character string")
	</ul>
   	<ul><li>dynamic: delete [], free()
	</ul>
</ul>

<li>LPOLESTR: from 16-Bit days, now same as LPWSTR
</ul>
<br>
Besides conversion and copy methods, the interface does not support single byte character strings.

Differentiate between LPWSTR and BSTR:
   LPWSTR and BSTR are of the same C++ type: w_char*
   <br>This has an unpleasant consequence: <br>
   The compiler cannot differentiate between methods that have LPWSTR/BSTR as the same parameter
   We are using the following rule for methods to compensate for this:
   <ul>
   <li>Methods with read-access on the parameter like Copy(), Append(), ... are assuming a LPWSTR and
   methods with write-accesss like CopyTo() are assuming a BSTR.
   <li>To access the non-default read-functions, there are functions with an additional BSTR in the name, 
   like CopyBSTR(), AppendBSTR().
   <li>To access the non-default write-functions, there is a function with an additional WSTR in the name,
   like CopyToWSTR().
   </ul>

</dl>
*/
class CNxNStringRef;
class CNxNVariant;


class CNxNBSTR;

// This is only a proxy class for CNxNBSTR
class NXNINTEGRATORSDK_API CNxNString
{
private:
    CNxNBSTR* psData;  // This is the real data for the proxy.

public:
	typedef unsigned int size_type;

public:
	//--------------------------------------------------------------------
	// construction / destruction
	//--------------------------------------------------------------------
	CNxNString();							    // default constructor (m_str == NULL)
	explicit CNxNString(size_type nSize);		// explicit, so this constructor isn't called when declaring a variable like this: CNxNString b('a') <- char == int 
	// string copy constructors
	CNxNString(LPCWSTR pSrc);					
	CNxNString(LPCWSTR pSrc, size_type nSize);	
	CNxNString(LPCWSTR pSrc, bool IsBSTR);		// we need the 2nd parameter only to differentiate between BSTR and LPWSTR
	CNxNString(LPCSTR pSrc);
	CNxNString(LPCSTR pSrc, size_type nSize);
	CNxNString(const CNxNString& src);
	CNxNString(const GUID& src);
	// character copy constructors
	explicit CNxNString(char c);
	explicit CNxNString(wchar_t c);

	~CNxNString();

	//--------------------------------------------------------------------
	// buffer information
	//--------------------------------------------------------------------
	size_type GetLength() const;				// return m_str == NULL || SysStringLen(m_str) == 0;
	bool IsEmpty() const;					// return GetLength()==0;
	bool IsNull() const;						// return m_str == NULL 
	BSTR GetBSTR() const;					// return m_str;
	BSTR* GetBSTRPtr();						// return &m_str;

	//--------------------------------------------------------------------
	// direct buffer manipulation
	//--------------------------------------------------------------------
	CNxNString& Attach(BSTR src);			// m_str = src;
	BSTR Detach();							// m_str = NULL;
	HRESULT Alloc(size_type len);			// free current string and allocate empty string with given length
	void Free();								// SysFreeString(m_str); m_str = NULL;

	//--------------------------------------------------------------------
	// copy other strings from given buffer
	//--------------------------------------------------------------------
	HRESULT Copy (LPCWSTR str, size_type len);
	HRESULT Copy (const VARIANT &Variant);
	HRESULT Copy (LPCWSTR str);
	HRESULT Copy (LPCWSTR str, bool IsBSTR);	// need IsBSTR to signal that we have a real BSTR and not a LPWSTR
	HRESULT Copy (const CNxNString& s);

	HRESULT Copy (LPCSTR str, int len);
	HRESULT Copy (LPCSTR str);
	
	//--------------------------------------------------------------------
	// create copy 
	//--------------------------------------------------------------------
	BSTR GetCopy() const;
	LPWSTR GetCopyWSTR() const;
	HRESULT CopyTo(CNxNString& src) const;
	HRESULT CopyTo(BSTR* pbstr) const;
	HRESULT CopyTo(char** ppstr) const;
	HRESULT CopyToTSTR(LPTSTR ppstr, size_type max_len) const;
	HRESULT CopyToWSTR(LPWSTR pbstr, size_type max_len) const;
	HRESULT CopyToWSTR(LPWSTR* pbstr) const;

	//--------------------------------------------------------------------
	// string collating
	//--------------------------------------------------------------------
	HRESULT Append(LPCWSTR lpsz, size_type nLen);
	HRESULT Append (LPCWSTR lpsz);
	HRESULT Append (LPCWSTR p, bool IsBSTR);			// need IsBSTR to signal that we have a real BSTR and not a LPWSTR
	HRESULT Append (const CNxNString& bstrSrc);
	HRESULT Append (LPCSTR lpsz, size_type nLen=-1);
	HRESULT Append (char c);
	HRESULT Append (wchar_t c);


	//--------------------------------------------------------------------
	// string search
	//--------------------------------------------------------------------
	int Find (wchar_t ch, int nStart=0) const;			// find a single character
	int Find (LPCWSTR lpszSub, int nStart=0) const;		// find a sub-string (like strstr)
	int FindOneOf (LPCWSTR lpszCharSet) const;			// find one of many characters
	int ReverseFind(WCHAR ch) const;					// find a single character, starting from the end of the string

	//--------------------------------------------------------------------
	// compare
	//--------------------------------------------------------------------
	int Compare(LPCWSTR lpsz, int nLen=-1) const;			
	int CompareNoCase(LPCWSTR lpsz, int nLen=-1) const; 

	// Collate is often slower than Compare but 
	// locale-sensitive with respect to sort order.
	int Collate(LPCWSTR lpsz, int nLen=-1) const;		
	int CollateNoCase(LPCWSTR lpsz, int nLen=-1) const;	

    bool BeginsWith(LPCWSTR pszStr) const;
    bool BeginsWithNoCase(LPCWSTR pszStr) const;
    bool EndsWith(LPCWSTR pszStr) const;
    bool EndsWithNoCase(LPCWSTR pszStr) const;

	//--------------------------------------------------------------------
	// character / substring retrieval
	//--------------------------------------------------------------------
	WCHAR GetAt(UINT nIndex) const;
	HRESULT SetAt(UINT nIndex, WCHAR ch);
	CNxNString Mid (UINT nFirst, UINT nCount) const;
	CNxNString Mid(UINT nFirst) const;
	CNxNString Left	(UINT nCount) const;
	CNxNString Right	(UINT nCount) const;

	//	checks for strings of the type "HELLO1" and returns the number
	int GetSuffixNumber( size_type& PrefixLength ) const;

	//--------------------------------------------------------------------
	// type conversion
	//--------------------------------------------------------------------
	CNxNString& FromUInt	(unsigned int v, int radix=10);
	CNxNString& FromULong	(unsigned long v, int radix=10);
	CNxNString& FromUInt64	(unsigned _int64 v, int radix=10);
	CNxNString& FromChar	(WCHAR c);
	CNxNString& FromInt	(int v, int radix=10);
	CNxNString& FromLong	(long v, int radix=10);
	CNxNString& FromInt64	(_int64 v, int radix=10);
	CNxNString& FromBool	(bool b);
	CNxNString& FromDouble	(double d);
	int		            AsInt(int base=10)	const;
	long		        AsLong(int base=10)	const;
	__int64	            AsInt64()			const;
	unsigned int		AsUInt(int base=10)	const;
	unsigned long	    AsULong(int base=10)const;
	unsigned __int64	AsUInt64()			const;
	float	            AsFloat()	const;
	double	            AsDouble()	const;
	bool	            AsBool(bool bDefault = false) const;

	//--------------------------------------------------------------------
	// make lower or upper case
	//--------------------------------------------------------------------
	HRESULT MakeLower();
	HRESULT MakeUpper();

	//--------------------------------------------------------------------
	// Escaping and resolving escapes
	//--------------------------------------------------------------------
    HRESULT EscapeStdCodes( CNxNString& sEscapedString );
    HRESULT ResolveStdCodeEscapes( CNxNString& sResolvedString );

    CNxNString EscapeStdCodes();
    CNxNString ResolveStdCodeEscapes();

	//--------------------------------------------------------------------
	// wsprintf like formatting
	//--------------------------------------------------------------------
	// FormatV should only be used if you exactly know what you are doing
	// take a look at Format if you want to know how to use it
	HRESULT FormatV(LPCWSTR lpszFormat, va_list argList); 
	CNxNString& __cdecl Format(LPCWSTR lpszFormat, ...);

	//--------------------------------------------------------------------
	// delete given characters or space from left or right side of string
	//--------------------------------------------------------------------
public:
	void TrimLeft	();
	void TrimLeft	(wchar_t chTarget);
	void TrimLeft	(LPCWSTR lpszTargets);
	void TrimRight	();
	void TrimRight	(wchar_t chTarget);
	void TrimRight	(LPCWSTR lpszTargetList);

	//--------------------------------------------------------------------
	// remove, replace, insert or delete characters and strings
	//--------------------------------------------------------------------
	int Remove	(WCHAR chRemove);
	int Remove  (const CNxNString& String, bool bIgnoreCase = false);
	int RemoveAll (const CNxNString& Chars);
	int Replace	(LPCWSTR lpszOld, LPCWSTR lpszNew);
	int Replace	(WCHAR chOld, WCHAR chNew);
	int Insert(int nIndex, LPWSTR pstr);
	int Insert(int nIndex, WCHAR ch);
	int Delete(int nIndex, int nCharsToDelete = 1);

	template<class STLContainer>
	STLContainer::size_type Split( STLContainer& PartList, wchar_t chSeparator ) const
	{
		STLContainer::size_type nPartsAtStart = PartList.size();

		LPWSTR pBegin = GetBSTR();
		LPWSTR pEnd	  = pBegin+GetLength(); 

		for (LPWSTR p=pBegin; p!=pEnd; p++) 
		{
			if (*p == chSeparator)
			{
				CNxNString::size_type nLen = CNxNString::size_type(p-pBegin);
				if (nLen > 0)
					PartList.push_back ( STLContainer::value_type(pBegin,nLen) );
				pBegin = p+1;
			}
		}
		if (pBegin != pEnd)
		{
			PartList.push_back ( STLContainer::value_type(pBegin,CNxNString::size_type(p-pBegin)) );
		}

		return PartList.size()-nPartsAtStart;
	}

    template<class STLContainer>
	int Split( STLContainer& PartList, const CNxNString& sSeparators ) const
	{
		int nPartsAtStart = PartList.size();

		LPWSTR pBegin = GetBSTR();
		LPWSTR pEnd	  = pBegin+GetLength(); 
		LPWSTR pSepBegin = sSeparators.GetBSTR();
		LPWSTR pSepEnd	 = pSepBegin + sSeparators.GetLength();

		for (LPWSTR p=pBegin; p!=pEnd; p++) 
		{
			for (LPWSTR pSep = pSepBegin; pSep != pSepEnd; pSep++)
			{
				if (*p == *pSep)
				{
					CNxNString::size_type nLen = CNxNString::size_type(p-pBegin);
					if (nLen > 0)
						PartList.push_back ( STLContainer::value_type(pBegin,nLen) );
					pBegin = p+1;
					break;
				}
			}
		}
		if (pBegin != pEnd)
		{
			PartList.push_back ( STLContainer::value_type(pBegin,CNxNString::size_type(p-pBegin)) );
		}

		return PartList.size()-nPartsAtStart;
	}

    template<class STLContainer>
	int SplitStringStructure(STLContainer& StringList, WCHAR chSeparator = L'|') const
	{
		int Pos = Find(chSeparator);

		if(Pos < 0) return 0;

		STLContainer::size_type nPartsAtStart = StringList.size();

		// seperate comma seperated list of string sizes from the list
		// of strings.
		CNxNString sLengthList;
		CNxNString sStringList;

		sLengthList.Attach(Left(Pos).Detach());
		sStringList.Attach(Mid(Pos+1).Detach());

		typedef std::vector<CNxNString> TStrings;
		typedef TStrings::iterator ItStrings;

		// seperate list of sizes into its elements.
		TStrings LengthList;
		sLengthList.Split(LengthList, L',');

		int PosInStrings = 0;
		for (ItStrings s = LengthList.begin(); s != LengthList.end(); ++s)
		{
			int Length = s->AsInt();

			// store current string in StringList
			StringList.push_back(sStringList.Mid(PosInStrings, Length));

			PosInStrings += Length;
		}

		return StringList.size()-nPartsAtStart;
	}

	template<class STLContainer>
	CNxNString& BuildString(STLContainer& StringList, WCHAR Separator = L'|', bool bEmptyFirst=true)
	{
		if (bEmptyFirst)
			*this = L"";

		if (StringList.size() == 0) 
			return *this;

		if (!IsEmpty())
			*this += Separator;

		STLContainer::const_iterator iLast = StringList.end();
		--iLast;
		
		// iterate through string list
		for( STLContainer::iterator i = StringList.begin(); i != StringList.end(); ++i )
		{
			*this += *i;
			if ( i != iLast )
				*this += Separator;
		}

		return *this;
	}

	template<class STLContainer>
	CNxNString& BuildStringStructure(STLContainer& StringList, WCHAR Separator = L'|')
	{
		CNxNString sLengthList;		// first part of the string, storing the sizes
		CNxNString sStringList;		// secnd part of the string, storing all strings

		// iterate through string list
		for( STLContainer::iterator i = StringList.begin(); i != StringList.end(); ++i )
		{
			// build length list
			if (!sLengthList.IsEmpty()) 
				sLengthList += L',';
			CNxNString s;	s.FromLong(i->GetLength());
			sLengthList += s;

			// build string list
			sStringList += *i;
		}

		// combine both parts.
		operator = (sLengthList + Separator + sStringList);

		return *this;
	}


	//--------------------------------------------------------------------
	// stream & resource I/O
	//--------------------------------------------------------------------
	HRESULT WriteToStream(IStream* pStream);
	HRESULT ReadFromStream(IStream* pStream);


	//--------------------------------------------------------------------
	// operators
	//--------------------------------------------------------------------
	
	// operator !
	bool operator!() const;				// return (m_str == NULL);

	// cast operators
	operator BSTR() const;				// return m_str;
	operator LPCWSTR() const;			// return m_str;
	operator LPCSTR () const;
	//operator bool() const; 
	//we are not defining an bool operator by intention because it could either mean p != NULL or GetLength() != 0 
	//and also it would enable int n = s; which could be misleading

	// BSTR* operator&() !! can't overload & operator, since STL uses &CNxNString in allocator class 
	//operator BSTR*();						// replacement for & operator, not very nice (only alternative would be an explicit method)
	BSTR* operator & ();

	// replacement for the overloaded &-operator
	CNxNString* GetAddr();

	// operator []
	wchar_t	operator [] (int nChar) const;
	wchar_t& operator [] (int nChar);
	wchar_t	operator [] (size_type nChar) const;
	wchar_t& operator [] (size_type nChar);

	// operator = 
	CNxNString& operator=(const CNxNString& src);
	CNxNString& operator=(const CNxNStringRef& src);

	template <typename T>
	inline CNxNString& operator= (T p)
	{
		Copy(p);
		return *this;
	}

	// operator +=
	CNxNString& operator += (const CNxNString& bstrSrc);
	
    template<typename T>
	inline CNxNString& operator += (T v)
	{
		Append(v);
		return *this;
	}

	//--------------------------------------------------------------------
	// helper methods for comparison
	//--------------------------------------------------------------------
	static bool NXNINTEGRATORSDK_API_CALL IsEqual (LPCWSTR pc1, LPCWSTR pc2);
	static bool NXNINTEGRATORSDK_API_CALL IsLess (LPCWSTR p1, LPCWSTR p2);
	static bool NXNINTEGRATORSDK_API_CALL IsGreater (LPCWSTR p1, LPCWSTR p2);


//--------------------------------------------------------------------
// CString support
//--------------------------------------------------------------------
#ifdef _MFC_VER 

	// static members
	static bool IsEqual (const CNxNString& bs, const CString& s);
	static CString MakeString(BSTR bs);

	CNxNString(const CString& s);

	// conversion
	CString AsCString() const;

	HRESULT Copy(const CString& s);
	UINT CopyTo(CString& rs) const;
	HRESULT Append (const CString& s);
	operator const CString& () const;
	CNxNString& operator=(const CString& s);
	CNxNString& operator+=(const CString& s);

	//--------------------------------------------------------------------
	// time string conversion routines
	//--------------------------------------------------------------------
	time_t GetTimeFromCTime() const;

#endif // #ifdef _MFC_VER


//====================================================================
// implementation
//====================================================================
public:
	struct PredCompareNoCaseLess : std::binary_function<CNxNString, CNxNString, bool>
	{
		inline bool operator() (const CNxNString& s1, const CNxNString& s2) const
		{
			return s1.CompareNoCase(s2)<0;
		}
	};

    struct PredCollateNoCaseLess : std::binary_function<CNxNString, CNxNString, bool>
	{
		inline bool operator() (const CNxNString& s1, const CNxNString& s2) const
		{
			return s1.CollateNoCase(s2)<0;
		}
	};

	struct PredCollateByLength : std::binary_function<CNxNString, CNxNString, bool>
	{
		inline bool operator() (const CNxNString& s1, const CNxNString& s2) const
		{
			return (s1.GetLength() - s2.GetLength() < 0) || (s1.Collate(s2) < 0);
		}
	};

	struct PredCollateByLengthNoCase : std::binary_function<CNxNString, CNxNString, bool>
	{
		inline bool operator() (const CNxNString& s1, const CNxNString& s2) const
		{
			return (s1.GetLength() - s2.GetLength() < 0) || (s1.CollateNoCase(s2) < 0);
		}
	};

protected:  // Needed for sub classes
};


class CNxNStringRef : public CNxNString
{
public:
	inline CNxNStringRef(BSTR bs)
	: CNxNString ()
	{
		Attach(bs);
	}
	inline ~CNxNStringRef()
	{
		Detach();	// do not delete BSTR since its only a reference
	}
};


//extern CNxNString NULLBSTR;
extern CNxNString NULLSTR;

inline bool operator == (const CNxNString&, const CNxNString&);
inline bool operator == (const CNxNString&, LPCSTR p);
inline bool operator == (const CNxNString&, LPCWSTR p);
inline bool operator == (LPCSTR p, const CNxNString&);
inline bool operator == (LPCWSTR p, const CNxNString&);

inline bool operator != (const CNxNString&, const CNxNString&);
inline bool operator != (LPCSTR p, const CNxNString&);
inline bool operator != (LPCWSTR p, const CNxNString&);
inline bool operator != (const CNxNString&, LPCSTR p);
inline bool operator != (const CNxNString&, LPCWSTR p);

inline CNxNString operator + (const CNxNString&, const CNxNString&);
inline CNxNString operator + (LPCSTR p, const CNxNString&);
inline CNxNString operator + (LPCWSTR p, const CNxNString&);
inline CNxNString operator + (wchar_t c, const CNxNString&);
inline CNxNString operator + (char c, const CNxNString&);
inline CNxNString operator + (const CNxNString&, LPCSTR p);
inline CNxNString operator + (const CNxNString&, LPCWSTR p);
inline CNxNString operator + (const CNxNString&, wchar_t c);
inline CNxNString operator + (const CNxNString&, char c);

//--------------------------------------------------------------------
// CString support
//--------------------------------------------------------------------
// 
#ifdef _MFC_VER

	inline bool operator == (const CNxNString& bs, const CString& s)
	{
		return CNxNString::IsEqual(bs, s);
	}
	inline bool operator == (const CString& s, const CNxNString& bs)
	{
		return CNxNString::IsEqual(bs, s);
	}

	inline bool operator != (const CNxNString& bs, const CString& s)
	{
		return !CNxNString::IsEqual(bs, s);
	}
	inline bool operator != (const CString& s, const CNxNString& bs)
	{
		return !CNxNString::IsEqual(bs, s);
	}

	inline CNxNString operator + (const CNxNString& bs, const CString& s)
	{
		CNxNString v(bs);
		v+=s;
		return v;
	}
	inline CNxNString operator + (const CString& s, const CNxNString& bs)
	{
		CNxNString v(s);
		v+=bs;
		return v;
	}

	inline bool operator < (const CNxNString& s1, const CString& s2)
	{
		return CNxNString::IsLess(s1, CNxNString(s2));
	}
	inline bool operator < (const CString& s1, const CNxNString& s2)
	{
		return CNxNString::IsLess(CNxNString(s1), s2);
	}
	inline bool operator > (const CNxNString& s1, const CString& s2)
	{
		return CNxNString::IsGreater(s1, CNxNString(s2));
	}
	inline bool operator > (const CString& s1, const CNxNString& s2)
	{
		return CNxNString::IsGreater(CNxNString(s1), s2);
	}

#endif // _MFC_VER


CNxNString NXNINTEGRATORSDK_API NxNFormatString(LPCWSTR lpszFormat, ...);

// Implementations of inline operators:

/////////////////////////////////////////////////////////////////////////
// operator +

inline CNxNString operator + (const CNxNString& s1, const CNxNString& s2)
{
	return CNxNString(s1) += s2;
}

inline CNxNString operator + (LPCSTR p, const CNxNString& s)
{
	return CNxNString(p) += s;
}

inline CNxNString operator + (LPCWSTR p, const CNxNString& s)
{
	return CNxNString(p) += s;
}

inline CNxNString operator + (wchar_t ch, const CNxNString& s)
{
	return CNxNString(ch) += s;
}

inline CNxNString operator + (char ch, const CNxNString& s)
{
	return CNxNString(ch) += s;
}

inline CNxNString operator + (const CNxNString& s, LPCSTR p)
{
	return CNxNString(s) += p;
}

inline CNxNString operator + (const CNxNString& s, LPCWSTR p)
{
	return CNxNString(s) += p;
}

inline CNxNString operator + (const CNxNString& s, wchar_t ch)
{
	return CNxNString(s) += ch;
}

inline CNxNString operator + (const CNxNString& s, char ch)
{
	return CNxNString(s) += ch;
}


/////////////////////////////////////////////////////////////////////////
// operator ==

/*! \fn inline bool operator == (const CNxNString& s1, const CNxNString& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator == (const CNxNString& s1, const CNxNString& s2)
{
	return (s1.GetLength() == s2.GetLength() && CNxNString::IsEqual((LPCWSTR)s1,(LPCWSTR)s2));
}
/*! \fn inline bool operator == (LPCSTR s1, const CNxNString& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator == (LPCSTR p, const CNxNString& s)
{
	return CNxNString::IsEqual(s, CNxNString(p));
}
/*! \fn inline bool operator == (LPCWSTR s1, const CNxNString& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator == (LPCWSTR p, const CNxNString& s)
{
	return CNxNString::IsEqual((LPCWSTR)s, p);
}
/*! \fn inline bool operator == (const CNxNString& s1, LPCSTR s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator == (const CNxNString& s, LPCSTR p)
{
	return CNxNString::IsEqual((LPCWSTR)s, CNxNString(p));
}
/*! \fn inline bool operator == (const CNxNString& s1, LPCWSTR s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator == (const CNxNString& s, LPCWSTR p)
{
	return CNxNString::IsEqual((LPCWSTR)s, p);
}

/////////////////////////////////////////////////////////////////////////
// operator !=

/*! \fn inline bool operator != (const CNxNString& s1, const CNxNString& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are not equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator != (const CNxNString& s1, const CNxNString& s2)
{
	return (s1.GetLength() != s2.GetLength() || !CNxNString::IsEqual((LPCWSTR)s1,(LPCWSTR)s2));
}
/*! \fn inline bool operator != (LPCSTR s1, const CNxNString& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are not equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator != (LPCSTR p, const CNxNString& s)
{
	return !CNxNString::IsEqual((LPCWSTR)s, (LPCWSTR)CNxNString(p));
}
/*! \fn inline bool operator != (LPCWSTR s1, const CNxNString& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are not equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator != (LPCWSTR p, const CNxNString& s)
{
	return !CNxNString::IsEqual((LPCWSTR)s, p);
}
/*! \fn inline bool operator != (const CNxNString& s1, LPCSTR s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are not equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator != (const CNxNString& s, LPCSTR p)
{
	return !CNxNString::IsEqual((LPCWSTR)s, (LPCWSTR)CNxNString(p));
}
/*! \fn inline bool operator != (const CNxNString& s1, LPCWSTR s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are not equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator != (const CNxNString& s, LPCWSTR p)
{
	return !CNxNString::IsEqual((LPCWSTR)s, p);
}

/////////////////////////////////////////////////////////////////////////
// operator <

/*! \fn inline bool operator < (const CNxNString& s1, const CNxNString& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is less then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator < (const CNxNString& s1, const CNxNString& s2)
{
	return CNxNString::IsLess(s1,s2);
}
/*! \fn inline bool operator < (const CNxNString& s1, LPCWSTR s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is less then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator < (const CNxNString& s1, LPCWSTR s2)
{
	return CNxNString::IsLess(s1,s2);
}
/*! \fn inline bool operator < (LPCWSTR s1,	 const CNxNString& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is less then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator < (LPCWSTR s1,	 const CNxNString& s2)
{
	return CNxNString::IsLess(s1,s2);
}
/*! \fn inline bool operator < (const CNxNString& s1, LPCSTR s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is less then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator < (const CNxNString& s1, LPCSTR s2)
{
	return CNxNString::IsLess(s1,CNxNString(s2));
}
/*! \fn inline bool operator < (LPCSTR s1,	 const CNxNString& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is less then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator < (LPCSTR s1,	 const CNxNString& s2)
{
	return CNxNString::IsLess(CNxNString(s1),s2);
}

/////////////////////////////////////////////////////////////////////////
// operator >

/*! \fn inline bool operator > (const CNxNString& s1, const CNxNString& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is greater then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator > (const CNxNString& s1, const CNxNString& s2)
{
	return CNxNString::IsGreater(s1,s2);
}
/*! \fn inline bool operator > (const CNxNString& s1, LPCWSTR s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is greater then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator > (const CNxNString& s1, LPCWSTR s2)
{
	return CNxNString::IsGreater(s1,s2);
}
/*! \fn inline bool operator > (const CNxNString& s1, LPCSTR s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is greater then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator > (const CNxNString& s1, LPCSTR s2)
{
	return CNxNString::IsGreater(s1,CNxNString(s2));
}
/*! \fn inline bool operator > (LPCWSTR s1,	 const CNxNString& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is greater then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator > (LPCWSTR s1,	 const CNxNString& s2)
{
	return CNxNString::IsGreater(s1,s2);
}
/*! \fn inline bool operator > (LPCSTR s1,	 const CNxNString& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is greater then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator > (LPCSTR s1,	 const CNxNString& s2)
{
	return CNxNString::IsGreater(CNxNString(s1),s2);
}


#endif // INC_NXN_NXNSTRING_H_
