#ifndef INC_NXN_BSTR_H_
#define INC_NXN_BSTR_H_


#include "NxNExport.h"
#include <atlbase.h> // A2WBSTR
#include <functional> // binary_function
#include "macros.h"
/*
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
*/

/*! \fn CNxNBSTR CNxNPath::GetNamespacePart(int xPart, bool bWithPreceedingSlash) const
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



/*! \class CNxNBSTR NxNBSTR.h
\brief This is a string class combining the advantages of BSTR and CString classes.
See the implementation for details.

<dl compact><dt><b>Note:</b>
<dd>
	NDR Some information about strings
<ul>
<li>BSTR: wide char string with leading length field (size_type) and by default terminated with 0
<ul><li>allocation:		
	<ul><li>dynamic: different versions of ::SysAllocString (also appends a terminating L'\0') only
		<li>static:  -
	</ul>
	<li>deallocation:	
	<ul><li>dynamic: SysFreeString
	</ul>
</ul>

<li>LPWSTR: wide char string (UNICODE 2 bytes per character) terminated with 0
<ul><li>allocation:		
	<ul><li>dynamic: new wchar[], malloc() 
		<li>static:  L"this is a static wide character string"
	</ul>
	<li>deallocation:		
	<ul><li>dynamic: delete [], free()
	</ul>
</ul>

<li>LPSTR: single byte char string (ANSI 1 byte per character) terminated with 0
<ul><li>allocation:		
	<ul><li>dynamic: new char[], malloc() 
		<li>static:  "this is a static single character string"
  	</ul>
 	<ul><li>dynamic: delete [], free()
	</ul>
</ul>

<li>LPTSTR: single byte or wide char string, depending on build options (default ANSI, optional UNICODE)
<ul><li>allocation:		
	<ul><li>dynamic: new TCHAR[], malloc() 
		<li>static:  _T("this is a static wide character string")
	</ul>
   	<ul><li>dynamic: delete [], free()
	</ul>
</ul>

<li>LPOLESTR: from 16-Bit days, now same as LPWSTR
</ul>
Besides conversion and copy methods, the interface does not support single byte character strings.

Differentiate between LPWSTR and BSTR:
   LPWSTR and BSTR are of the same CPP-type: w_char*
   This has an unpleasant consequence: 
   The compiler can't differentiate between methods which have LPWSTR/BSTR as the same parameter
   We are using the following rule for methods to compensate for this:
   Methods with read-access on the parameter like Copy, Append,... are assuming a LPWSTR and
   methods with write-accesss like CopyTo assuming BSTR.
   To access the non-default read-functions, there are functions with an additional BSTR in the name, 
   like CopyBSTR, AppendBSTR
   To access the non-default write-functions, there is a function with an additional WSTR in the name,
   like CopyToWSTR

Get... is used, when the result is a copy on the stack or the result of some calculation
when a member variable is returned, Get is not used
</dl>
*/
class CNxNBSTRRef;
class CNxNVariant;

#define CNxNBSTR CNxNString
#define MAX_STATIC_BUFFER		256

class NXNINTEGRATORSDK_API CNxNBSTR
{
public:
	typedef unsigned int size_type;
private:
	inline void Construct(LPCWSTR str, size_type len=-1);
public:

	//--------------------------------------------------------------------
	// construction / destruction
	//--------------------------------------------------------------------
	inline CNxNBSTR();								// default constructor (m_str == NULL)
	inline explicit CNxNBSTR(size_type nSize);		// explicit, so this constructor isn't called when declaring a variable like this: CNxNBSTR b('a') <- char == int 
	// string copy constructors
	inline CNxNBSTR(LPCWSTR pSrc);					
	inline CNxNBSTR(LPCWSTR pSrc, size_type nSize);	
	inline CNxNBSTR(LPCWSTR pSrc, bool IsBSTR);		// we need the 2nd parameter only to differentiate between BSTR and LPWSTR
	inline CNxNBSTR(LPCSTR pSrc);
	inline CNxNBSTR(LPCSTR pSrc, size_type nSize);
	inline CNxNBSTR(const CNxNBSTR& src);
	inline CNxNBSTR(const GUID& src);
	// character copy constructors
	inline explicit CNxNBSTR(char c);
	inline explicit CNxNBSTR(wchar_t c);

//	inline virtual ~CNxNBSTR()
	inline ~CNxNBSTR()
	{
		_Free ();
	}
//	virtual ~CNxNBSTR();

	//--------------------------------------------------------------------
	// buffer information
	//--------------------------------------------------------------------
	inline size_type GetLength() const;				// return m_str == NULL || SysStringLen(m_str) == 0;
	inline bool IsEmpty() const;					// return GetLength()==0;
	inline bool IsNull() const;						// return m_str == NULL 
	inline BSTR GetBSTR() const;					// return m_str;
	inline BSTR* GetBSTRPtr();						// return &m_str;

	//--------------------------------------------------------------------
	// direct buffer manipulation
	//--------------------------------------------------------------------
	inline CNxNBSTR& Attach(BSTR src);				// m_str = src;
	inline BSTR Detach();							// m_str = NULL;
	inline HRESULT Alloc(size_type len);			// free current string and allocate empty string with given length
	inline void Free();								// SysFreeString(m_str); m_str = NULL;

	//--------------------------------------------------------------------
	// copy other strings from given buffer
	//--------------------------------------------------------------------
	HRESULT Copy (LPCWSTR str, size_type len);
	HRESULT Copy (const VARIANT &Variant);
	inline HRESULT Copy (LPCWSTR str);
	inline HRESULT Copy (LPCWSTR str, bool IsBSTR);	// need IsBSTR to signal that we have a real BSTR and not a LPWSTR
	inline HRESULT Copy (const CNxNBSTR& s);

	HRESULT Copy (LPCSTR str, int len);
	inline HRESULT Copy (LPCSTR str) { return Copy(str, str ? strlen(str) : 0); };
	
	//--------------------------------------------------------------------
	// create copy 
	//--------------------------------------------------------------------
	inline BSTR GetCopy() const;
	inline LPWSTR GetCopyWSTR() const;
	inline HRESULT CopyTo(CNxNBSTR& src) const;
	inline HRESULT CopyTo(BSTR* pbstr) const;
	inline HRESULT CopyTo(char** ppstr) const;
	inline HRESULT CopyToTSTR(LPTSTR ppstr, size_type max_len) const;
	inline HRESULT CopyToWSTR(LPWSTR pbstr, size_type max_len) const;
	inline HRESULT CopyToWSTR(LPWSTR* pbstr) const;
private:
	inline HRESULT AllocCopy(CNxNBSTR& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;

	template <class T>
	class CMallocPointer
	{
		T * p;
		size_t len;
		inline void InternalFree(T *pFree) { if (pFree) ::free(pFree); }
	public:
		inline CMallocPointer()					: p(NULL), len(size_t(-1))	{}
		inline CMallocPointer(size_t nBytes)	: p(NULL), len(size_t(-1))	{ p = (T*)malloc(nBytes); len = nBytes; }
		inline CMallocPointer(T * p)			: p(NULL), len(size_t(-1))	{ *this = other.p; }
		virtual ~CMallocPointer()											{ InternalFree(p); }

		inline size_t GetLength() const { NXN_ASSERT(len != size_t(-1)); return len; }
		inline T* Alloc ( size_t nBytes ) { if(p) InternalFree(p); p = (T*)malloc(nBytes); len = nBytes; return p; }

		inline operator T * ()		{ return p; }
		inline operator bool ()		{ return p != NULL; }
		inline operator ! ()		{ return p == NULL; }

		inline T &	operator * ()	{ NXN_ASSERT(p != NULL); return *p; }
		inline T **	operator & ()	{ return &p; }
		inline bool	operator == (int i)			const { NXN_ASSERT(i==0); return p == NULL; }
		inline bool	operator == (const T *other)const { return p == other; }

		inline const T * operator = (const T * other) { InternalFree(p); p = const_cast<T *>(other); return p; }

		inline T *	Detach() { T *oldp = p; p = NULL; return oldp; }
	};

public:

	//--------------------------------------------------------------------
	// string collating
	//--------------------------------------------------------------------
	HRESULT Append(LPCWSTR lpsz, size_type nLen);
	inline HRESULT Append (LPCWSTR lpsz);
	inline HRESULT Append (LPCWSTR p, bool IsBSTR);			// need IsBSTR to signal that we have a real BSTR and not a LPWSTR
	inline HRESULT Append (const CNxNBSTR& bstrSrc);
	inline HRESULT Append (LPCSTR lpsz, size_type nLen=-1);
	inline HRESULT Append (char c);
	inline HRESULT Append (wchar_t c);


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
	inline int Compare(LPCWSTR lpsz, int nLen=-1) const;			
	inline int CompareNoCase(LPCWSTR lpsz, int nLen=-1) const; 
	// Collate is often slower than Compare but 
	// locale-sensitive with respect to sort order.
	inline int Collate(LPCWSTR lpsz, int nLen=-1) const;		
	inline int CollateNoCase(LPCWSTR lpsz, int nLen=-1) const;	

	//--------------------------------------------------------------------
	// character / substring retrieval
	//--------------------------------------------------------------------
	inline WCHAR GetAt(UINT nIndex) const;
	inline HRESULT SetAt(UINT nIndex, WCHAR ch);
	CNxNBSTR Mid (UINT nFirst, UINT nCount) const;
	inline CNxNBSTR Mid(UINT nFirst) const;
	CNxNBSTR Left	(UINT nCount) const;
	CNxNBSTR Right	(UINT nCount) const;

	//	checks for strings of the type "HELLO1" and returns the number
	int GetSuffixNumber( size_type& PrefixLength ) const;

	//--------------------------------------------------------------------
	// type conversion
	//--------------------------------------------------------------------
	inline CNxNBSTR& FromUInt	(unsigned int v, int radix=10);
	inline CNxNBSTR& FromULong	(unsigned long v, int radix=10);
	inline CNxNBSTR& FromUInt64	(unsigned _int64 v, int radix=10);
	inline CNxNBSTR& FromChar	(WCHAR c);
	inline CNxNBSTR& FromInt	(int v, int radix=10);
	inline CNxNBSTR& FromLong	(long v, int radix=10);
	inline CNxNBSTR& FromInt64	(_int64 v, int radix=10);
	inline CNxNBSTR& FromBool	(bool b);
	inline CNxNBSTR& FromDouble	(double d);
	inline int		AsInt(int base=10)	const;
	inline long		AsLong(int base=10)	const;
	inline __int64	AsInt64()			const;
	inline unsigned int		AsUInt(int base=10)	const;
	inline unsigned long	AsULong(int base=10)const;
	inline unsigned __int64	AsUInt64()			const;
	inline float	AsFloat()	const;
	inline double	AsDouble()	const;
	inline bool		AsBool()	const;

	//--------------------------------------------------------------------
	// make lower or upper case
	//--------------------------------------------------------------------
	inline HRESULT MakeLower();
	inline HRESULT MakeUpper();

	//--------------------------------------------------------------------
	// wsprintf like formatting
	//--------------------------------------------------------------------
	// FormatV should only be used if you exactly know what you are doing
	// take a look at Format if you want to know how to use it
	HRESULT FormatV(LPCWSTR lpszFormat, va_list argList); 
	HRESULT __cdecl Format(LPCWSTR lpszFormat, ...);

	//--------------------------------------------------------------------
	// delete given characters or space from left or right side of string
	//--------------------------------------------------------------------
private:
	void _TrimLeft	(LPCWSTR lpsz);
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
	int Replace	(LPCWSTR lpszOld, LPCWSTR lpszNew);
	int Replace	(WCHAR chOld, WCHAR chNew);
	int Insert(int nIndex, LPWSTR pstr);
	int Insert(int nIndex, WCHAR ch);
	int Delete(int nIndex, int nCharsToDelete = 1);

	/*! Splits the strint into its subparts and stores the parts into a STL-container.
		The routine does not clear the container.
		\param STL-container to store the parts into.
		\param Separator the character which separates the subparts
		\return the number of parts added to the container
	*/
	template<class STLContainer>
	STLContainer::size_type Split( STLContainer& PartList, wchar_t Separator ) const
	{
		STLContainer::size_type nPartsAtStart = PartList.size();

		LPWSTR pBegin = GetBSTR();
		LPWSTR pEnd	  = pBegin+GetLength(); 

		for (LPWSTR p=pBegin; p!=pEnd; p++) 
		{
			if (*p == Separator)
			{
				CNxNBSTR::size_type nLen = CNxNBSTR::size_type(p-pBegin);
				if (nLen > 0)
					PartList.push_back ( STLContainer::value_type(pBegin,nLen) );
				pBegin = p+1;
			}
		}
		if (pBegin != pEnd)
		{
			PartList.push_back ( STLContainer::value_type(pBegin,CNxNBSTR::size_type(p-pBegin)) );
		}

		return PartList.size()-nPartsAtStart;
	}
	/*! Splits the strint into its subparts and stores the parts into a STL-container.
		The routine does not clear the container.
		\param STL-container to store the parts into.
		\param Separators list of separator characters
		\return the number of parts added to the container
	*/
	template<class STLContainer>
	int Split( STLContainer& PartList, const CNxNBSTR& Separators ) const
	{
		int nPartsAtStart = PartList.size();

		LPWSTR pBegin = GetBSTR();
		LPWSTR pEnd	  = pBegin+GetLength(); 
		LPWSTR pSepBegin = Separators.GetBSTR();
		LPWSTR pSepEnd	 = pSepBegin + Separators.GetLength();

		for (LPWSTR p=pBegin; p!=pEnd; p++) 
		{
			for (LPWSTR pSep = pSepBegin; pSep != pSepEnd; pSep++)
			{
				if (*p == *pSep)
				{
					CNxNBSTR::size_type nLen = CNxNBSTR::size_type(p-pBegin);
					if (nLen > 0)
						PartList.push_back ( STLContainer::value_type(pBegin,nLen) );
					pBegin = p+1;
					break;
				}
			}
		}
		if (pBegin != pEnd)
		{
			PartList.push_back ( STLContainer::value_type(pBegin,CNxNBSTR::size_type(p-pBegin)) );
		}

		return PartList.size()-nPartsAtStart;
	}
	/*! Splits the string structure into its subparts and stores the parts into a STL-container.
		The routine does not clear the container.
		\param STL-container to store the parts into.
		\param Separators list of separator characters
		\return the number of parts added to the container
	*/
	template<class STLContainer>
	int SplitStringStructure(STLContainer& StringList, WCHAR Separator = L'|') const
	{
		int Pos = Find(Separator);

		if(Pos < 0) return 0;

		STLContainer::size_type nPartsAtStart = StringList.size();

		// seperate comma seperated list of string sizes from the list
		// of strings.
		CNxNBSTR sLengthList;
		CNxNBSTR sStringList;

		sLengthList.Attach(Left(Pos).Detach());
		sStringList.Attach(Mid(Pos+1).Detach());

		typedef std::vector<CNxNBSTR> TStrings;
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
	CNxNBSTR& BuildStringStructure(STLContainer& StringList, WCHAR Separator = L'|')
	{
		CNxNBSTR sLengthList;		// first part of the string, storing the sizes
		CNxNBSTR sStringList;		// secnd part of the string, storing all strings

		// iterate through string list
		for( STLContainer::iterator i = StringList.begin(); i != StringList.end(); ++i )
		{
			// build length list
			if (!sLengthList.IsEmpty()) 
				sLengthList += L',';
			CNxNBSTR s;	s.FromLong(i->GetLength());
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
	bool LoadString(HINSTANCE hInst, UINT nID);
	bool LoadString(UINT nID);
	HRESULT WriteToStream(IStream* pStream);
	HRESULT ReadFromStream(IStream* pStream);


	//--------------------------------------------------------------------
	// operators
	//--------------------------------------------------------------------
	
	// operator !
	inline bool operator!() const;				// return (m_str == NULL);

	// cast operators
	inline operator BSTR() const;				// return m_str;
	inline operator LPCWSTR() const;			// return m_str;
	inline operator LPCSTR () const;
	//inline operator bool() const; 
	//we are not defining an bool operator by intention because it could either mean p != NULL or GetLength() != 0 
	//and also it would enable int n = s; which could be misleading

	// inline BSTR* operator&() !! can't overload & operator, since STL uses &CNxNBSTR in allocator class 
	//inline operator BSTR*();						// replacement for & operator, not very nice (only alternative would be an explicit method)
	inline BSTR* operator & ()	
	{ 
		_Free ();		// free content, before receiving new string
		return &m_str;
	}

	// operator []
	inline wchar_t	operator [] (int nChar) const;
	inline wchar_t& operator [] (int nChar);
	inline wchar_t	operator [] (size_type nChar) const;
	inline wchar_t& operator [] (size_type nChar);

	// operator = 
	inline CNxNBSTR& operator=(const CNxNBSTR& src);
	inline CNxNBSTR& operator=(const CNxNBSTRRef& src);
	template <typename T>
	inline CNxNBSTR& operator= (T p)
	{
		Copy (p);
		return *this;
	}

	// operator +=
	inline CNxNBSTR& operator += (const CNxNBSTR& bstrSrc);
	template<typename T>
	inline CNxNBSTR& operator += (T v)
	{
		Append(v);
		return *this;
	}

	//--------------------------------------------------------------------
	// helper methods for comparison
	//--------------------------------------------------------------------
	inline static bool NXNINTEGRATORSDK_API_CALL IsEqual (LPCWSTR pc1, LPCWSTR pc2);
	inline static bool NXNINTEGRATORSDK_API_CALL IsLess (LPCWSTR p1, LPCWSTR p2);
	inline static bool NXNINTEGRATORSDK_API_CALL IsGreater (LPCWSTR p1, LPCWSTR p2);


//--------------------------------------------------------------------
// CString support
//--------------------------------------------------------------------
#ifdef _MFC_VER 

	// static members
	inline static bool IsEqual (const CNxNBSTR& bs, const CString& s)
	{
		return (int)bs.GetLength()==s.GetLength() && (bs.GetLength()==0 || s == (LPCSTR)bs);
	}
	inline static CString MakeString(BSTR bs)
	{
		return bs ? CString(bs) : CString(_T(""));
	}

	inline CNxNBSTR::CNxNBSTR(const CString& s)
	{
		m_str = NULL;
		Copy(s);
	}

	// conversion
	inline CString CNxNBSTR::AsCString() const
	{
		return m_str ? CString(m_str) : CString(_T(""));
	}

	inline HRESULT Copy(const CString& s)
	{
		return Copy((LPCTSTR)s,s.GetLength());
	}
	inline UINT CopyTo(CString& rs) const
	{
		if (m_str==NULL) 
			rs = _T("");
		else 
			rs = CString(m_str, GetLength());
		return GetLength();
	}
	inline HRESULT Append (const CString& s)
	{
		return Append((LPCTSTR)s,(size_type)s.GetLength());
	}
	inline operator const CString& () const
	{
		static CString as[MAX_STATIC_BUFFER];
		static int x=0;
		CString &s=as[x++];
		if (x==20) x = 0;
		CopyTo(s);
		return s;
	}
	/*inline operator CString () const // seems not work, don't know why
	{
		static CString as[MAX_STATIC_BUFFER];
		static int x=0;
		CString &s=as[x++];
		if (x==20) x = 0;
		CopyTo(s);
		return s;
	}*/
	inline CNxNBSTR& operator=(const CString& s)
	{
		Copy(s);
		return *this;
	}
	inline CNxNBSTR& operator+=(const CString& s)
	{
		Append(s);
		return *this;
	}

	//--------------------------------------------------------------------
	// time string conversion routines
	//--------------------------------------------------------------------
	time_t GetTimeFromCTime() const
	{
		if (GetLength() != 24) return 0;

		WCHAR szWDay[40]	= L"";
		WCHAR szMonth[40]	= L""; 
		int Month=-1, Day=0, Hour=0, Min=0, Sec=0, Year=0;

		if (swscanf(m_str, L"%s %s %d %d:%d:%d %d\n", &szWDay[0], &szMonth[0], &Day, &Hour, &Min, &Sec, &Year) != 7)
			return 0;

		static WCHAR *szMonthTable[12] = { L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec" };
		for (int i=0; i < 12; i++)
		{
			if (wcscmp(szMonth,szMonthTable[i])==0)
			{
				Month = i+1;
				break;
			}
		}
		if (Month == -1)
			return 0;

		return CTime(Year, Month, Day, Hour, Min, Sec).GetTime();
	}

#endif // #ifdef _MFC_VER


//====================================================================
// implementation
//====================================================================
private:
	//--------------------------------------------------------------------
	// internal buffer
	//--------------------------------------------------------------------
	BSTR m_str;

	//--------------------------------------------------------------------
	// reference counting
	//--------------------------------------------------------------------
	inline void _Free()
	{
		SysFreeString(m_str);
		m_str = NULL;
	}

protected:
	//--------------------------------------------------------------------
	// notifications about changed content
	//--------------------------------------------------------------------
/*	virtual void _Changed()											{;};	// made it upper or lowercase
	virtual void _Replaced(LPCWSTR fromPos=NULL, LPCWSTR toPos=NULL){;};	// replaced part or everything
	virtual void _Deleted (LPCWSTR fromPos=NULL, LPCWSTR toPos=NULL){;};	// deleted part or everything
	virtual void _Appended(LPCWSTR fromPos, size_type nChars)		{;};	// appended string
*/
	// not used at the moment, so we eliminate the calling overhead by declaring them as non virtual and inline
	inline void _Changed()											{;};	// made it upper or lowercase
	inline void _Replaced(LPCWSTR fromPos=NULL, LPCWSTR toPos=NULL) { fromPos=NULL; toPos = NULL; };	// replaced part or everything
	inline void _Deleted (LPCWSTR fromPos=NULL, LPCWSTR toPos=NULL) { fromPos=NULL; toPos = NULL; };	// deleted part or everything
	inline void _Appended(LPCWSTR fromPos, size_type nChars)		{ fromPos=NULL; nChars = 0; };	// appended string

	void SetLength(size_type nNewLen)
	{
		NXN_ASSERT(nNewLen < GetLength());
		size_type *p=(size_type*)m_str;
		--p;
		*p = nNewLen << 1; // length is stored in Byte units, not WCHAR units
	}

	//	helper method for ASCII to Wide character conversion
	inline BSTR NXNA2WBSTR(LPCSTR lp, int nLen) // nLen includes the 0 terminator
	{
		USES_CONVERSION; // from ATLCONV.h
		BSTR str = NULL;
		int nWideChars = MultiByteToWideChar(_acp, 0, lp, nLen, NULL, 0);
		str = ::SysAllocStringLen(NULL, nWideChars-1); //TECH: SysAllocStringLen allocates nWideChars+1 (1 for 0 character)
		if (str != NULL)
		{
			MultiByteToWideChar(_acp, 0, lp, nLen, str, nWideChars);
			str[nWideChars-1] = L'\0';  // terminate, MultiByteToWideChar does not do it
		}
		return str;
	}

public:
	struct PredCollateNoCaseLess : std::binary_function<CNxNBSTR, CNxNBSTR, bool>
	{
		inline bool operator() (const CNxNBSTR& s1, const CNxNBSTR& s2) const
		{
			return s1.CollateNoCase(s2)<0;
		}
	};
	struct PredCollateByLength : std::binary_function<CNxNBSTR, CNxNBSTR, bool>
	{
		inline bool operator() (const CNxNBSTR& s1, const CNxNBSTR& s2) const
		{
			return (s1.GetLength() - s2.GetLength() < 0) || (s1.Collate(s2) < 0);
		}
	};
	struct PredCollateByLengthNoCase : std::binary_function<CNxNBSTR, CNxNBSTR, bool>
	{
		inline bool operator() (const CNxNBSTR& s1, const CNxNBSTR& s2) const
		{
			return (s1.GetLength() - s2.GetLength() < 0) || (s1.CollateNoCase(s2) < 0);
		}
	};
};

extern CNxNBSTR NULLBSTR;
extern CNxNBSTR NULLSTR;

inline bool operator == (const CNxNBSTR&, const CNxNBSTR&);
inline bool operator == (const CNxNBSTR&, LPCSTR p);
inline bool operator == (const CNxNBSTR&, LPCWSTR p);
inline bool operator == (LPCSTR p, const CNxNBSTR&);
inline bool operator == (LPCWSTR p, const CNxNBSTR&);

inline bool operator != (const CNxNBSTR&, const CNxNBSTR&);
inline bool operator != (LPCSTR p, const CNxNBSTR&);
inline bool operator != (LPCWSTR p, const CNxNBSTR&);
inline bool operator != (const CNxNBSTR&, LPCSTR p);
inline bool operator != (const CNxNBSTR&, LPCWSTR p);

inline CNxNBSTR operator + (const CNxNBSTR&, const CNxNBSTR&);
inline CNxNBSTR operator + (LPCSTR p, const CNxNBSTR&);
inline CNxNBSTR operator + (LPCWSTR p, const CNxNBSTR&);
inline CNxNBSTR operator + (wchar_t c, const CNxNBSTR&);
inline CNxNBSTR operator + (char c, const CNxNBSTR&);
inline CNxNBSTR operator + (const CNxNBSTR&, LPCSTR p);
inline CNxNBSTR operator + (const CNxNBSTR&, LPCWSTR p);
inline CNxNBSTR operator + (const CNxNBSTR&, wchar_t c);
inline CNxNBSTR operator + (const CNxNBSTR&, char c);

//--------------------------------------------------------------------
// CString support
//--------------------------------------------------------------------
// 
#ifdef _MFC_VER

	inline bool operator == (const CNxNBSTR& bs, const CString& s)
	{
		return CNxNBSTR::IsEqual(bs, s);
	}
	inline bool operator == (const CString& s, const CNxNBSTR& bs)
	{
		return CNxNBSTR::IsEqual(bs, s);
	}

	inline bool operator != (const CNxNBSTR& bs, const CString& s)
	{
		return !CNxNBSTR::IsEqual(bs, s);
	}
	inline bool operator != (const CString& s, const CNxNBSTR& bs)
	{
		return !CNxNBSTR::IsEqual(bs, s);
	}

	inline CNxNBSTR operator + (const CNxNBSTR& bs, const CString& s)
	{
		CNxNBSTR v(bs);
		v+=s;
		return v;
	}
	inline CNxNBSTR operator + (const CString& s, const CNxNBSTR& bs)
	{
		CNxNBSTR v(s);
		v+=bs;
		return v;
	}

	inline bool operator < (const CNxNBSTR& s1, const CString& s2)
	{
		return CNxNBSTR::IsLess(s1, CNxNBSTR(s2));
	}
	inline bool operator < (const CString& s1, const CNxNBSTR& s2)
	{
		return CNxNBSTR::IsLess(CNxNBSTR(s1), s2);
	}
	inline bool operator > (const CNxNBSTR& s1, const CString& s2)
	{
		return CNxNBSTR::IsGreater(s1, CNxNBSTR(s2));
	}
	inline bool operator > (const CString& s1, const CNxNBSTR& s2)
	{
		return CNxNBSTR::IsGreater(CNxNBSTR(s1), s2);
	}

#endif // _MFC_VER


class CNxNBSTRRef : public CNxNBSTR
{
public:
	inline CNxNBSTRRef(BSTR bs)
	: CNxNBSTR ()
	{
		Attach(bs);
	}
	inline ~CNxNBSTRRef()
	{
		Detach();	// do not delete BSTR since its only a reference
	}
};


CNxNBSTR NXNINTEGRATORSDK_API NxNFormatString(HINSTANCE hInst, UINT nID, ...);
//CNxNBSTR NXNINTEGRATORSDK_API NxNFormatString(const CNxNBSTR& strFormat, ...);
CNxNBSTR NXNINTEGRATORSDK_API NxNFormatString(LPCWSTR lpszFormat, ...);






//-----------------------------------------------------------------------
//	implementation of CNxNBSTR inline members
//-----------------------------------------------------------------------


//-----------------------------------------------------------------------
//	constructors
//-----------------------------------------------------------------------

/*! \fn CNxNBSTR::CNxNBSTR() 
*/
inline CNxNBSTR::CNxNBSTR()
{
	m_str = NULL;
}
/*! \fn CNxNBSTR::CNxNBSTR(size_type nChars) 
	\param nChars number of characters to allocate. A null character is placed afterwards, 
	allocating a total of nChars plus one characters. 
	<dl compact><dt><b>Comments</b>
	Besides the null character at the end of the string, the string is not initialized.
	</dl>		
*/
inline CNxNBSTR::CNxNBSTR(size_type nChars) 
{
	m_str = ::SysAllocStringLen(NULL, nChars);
	if (m_str)
		m_str[nChars] = L'\0';
}
/*! \fn CNxNBSTR::CNxNBSTR(LPCWSTR sz)
	\param  sz The string to be copied.
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR::CNxNBSTR(LPCWSTR sz)
{
	Construct(sz);
}
/*! \fn CNxNBSTR::CNxNBSTR(LPCWSTR sz, size_type nChars)
	\param  sz Wide character string to be copied.
	\param  nChars The number of characters to be copied from sz.
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR::CNxNBSTR(LPCWSTR sz, size_type nChars)
{
	Construct(sz, nChars);
}
/*! \fn CNxNBSTR::CNxNBSTR(LPCWSTR sz, bool IsBSTR)
	\param  sz Wide character string to be copied.
	\param  IsBSTR true, when sz is a BSTR (the string was allocated by one of the SysAlloc... methods)
	<dl compact><dt><b>Comments</b>
	Constants like L"IsBSTR" is no BSTR.
	</dl>		
*/
inline CNxNBSTR::CNxNBSTR(LPCWSTR sz, bool IsBSTR)
{
	if (IsBSTR) 
		Construct(sz, ::SysStringLen((BSTR)sz));
	else 
		Construct(sz);
}

/*! \fn CNxNBSTR::CNxNBSTR(LPCSTR sz)
	\param sz ANSI character string to be copied.
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR::CNxNBSTR(LPCSTR sz)
{
	if (sz==NULL) 
	{
		m_str = NULL;
		return;
	}
	m_str = NXNA2WBSTR(sz, strlen(sz)+1); // strlen+1 : A2WBSTR length includes '\0' character
}
/*! \fn CNxNBSTR::CNxNBSTR(LPCSTR sz, size_type nSize)
	\param sz ANSI character string to be copied.
	\param nSize The number of characters to be copied from sz.
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR::CNxNBSTR(LPCSTR sz, size_type nSize)
{
	if (nSize==0 || sz==NULL) 
	{
		m_str = NULL;
		return;
	}
	if (nSize==size_type(-1))
	{
		nSize = strlen(sz);
	}
	m_str = NXNA2WBSTR(sz, nSize+1); // nSize+1 : A2WBSTR length includes '\0' character
}
/*! \fn CNxNBSTR::CNxNBSTR(const CNxNBSTR& src)
	\param src CNxNBSTR string to be copied
	<dl compact><dt><b>Comments</b>
	Copy constructor.
	</dl>		
*/
inline CNxNBSTR::CNxNBSTR(const CNxNBSTR& src)
{
	m_str = src.GetCopy();
}

/*! \fn CNxNBSTR::CNxNBSTR(const GUID& guid)
	\param guid GUID to be copied.
	<dl compact><dt><b>Comments</b>
	This constructor uses the method StringFromCLSID to create a string from a GUID.
	</dl>		
*/
inline CNxNBSTR::CNxNBSTR(const GUID& guid)
{
	LPOLESTR szGuid;
	StringFromCLSID(guid, &szGuid);
	m_str = ::SysAllocString(szGuid);
	CoTaskMemFree(szGuid);
}

/*! \fn CNxNBSTR::CNxNBSTR(char c)
	\param c ANSI character to be copied.
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR::CNxNBSTR(char c)
{
	m_str = NULL;
	static char sz[2]="\0";
	sz[0] = c;
	Copy (&sz[0],1);
}
/*! \fn CNxNBSTR::CNxNBSTR(wchar_t c)
	\param c Wide character to be copied.
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR::CNxNBSTR(wchar_t c)
{
	static wchar_t sz[2]=L"\0";
	sz[0] = c;
	m_str = ::SysAllocStringLen(&sz[0],1);
}

/*! \fn CNxNBSTR::Construct(LPCWSTR str, size_type len)
	<dl compact><dt><b>Comments</b>
	Special method which is called only from constructors
	</dl>		
*/
inline void CNxNBSTR::Construct(LPCWSTR str, size_type len)
{
	if (str == NULL || *str == L'\0')
	{
		m_str = NULL; 
		return;
	}

	if (len==size_type(-1))
	{
		len = str ? wcslen(str) : 0;
	}

	m_str = SysAllocStringLen(str, len);
}




//--------------------------------------------------------------------
// buffer information
//--------------------------------------------------------------------

/*! \fn CNxNBSTR::size_type CNxNBSTR::GetLength() const
	\return Number of characters in string.
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR::size_type CNxNBSTR::GetLength() const
{
	return (m_str == NULL) ? 0 : SysStringLen(m_str);
	//return (m_str == NULL) ? 0 : ( (*(((size_type*)m_str)-1)) >> 1);//SysStringLen(m_str);
}
/*! \fn bool CNxNBSTR::IsEmpty() const
	\return True, when number of characters is 0.
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool CNxNBSTR::IsEmpty() const 
{ 
	return GetLength()==0; 
}
/*! \fn inline bool CNxNBSTR::IsNull() const
	\return True, when m_str == NULL
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool CNxNBSTR::IsNull() const
{ 
	return m_str==NULL; 
}


//--------------------------------------------------------------------
// copy other strings
//--------------------------------------------------------------------

/*! \fn inline HRESULT CNxNBSTR::Copy(const CNxNBSTR& str)
	\param	str Wide character string to copy
	\return	 S_OK or E_OUTOFMEMORY
	<dl compact><dt><b>Commnts</b>
	Copies the given CNxNBSTR
	</dl>		
*/
inline HRESULT CNxNBSTR::Copy(const CNxNBSTR& str)
{
	return Copy(str.GetBSTR(), str.GetLength());
}
/*! \fn inline HRESULT CNxNBSTR::Copy (LPCWSTR str)
	\param	str Null terminated wide character string to copy
	\return	 S_OK or E_OUTOFMEMORY
	<dl compact><dt><b>Comments</b>
	Copies the given wide character string.
	The given string must be null terminated.
	</dl>		
*/
inline HRESULT CNxNBSTR::Copy (LPCWSTR str)
{
	return Copy(str, (size_type)-1);
}
/*! \fn inline HRESULT CNxNBSTR::Copy (LPCWSTR str, bool IsBSTR)
	\param	str Null terminated wide character string or real BSTR.
	\param	IsBSTR true, when str is a real BSTR
	\return	 S_OK or E_OUTOFMEMORY
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline HRESULT CNxNBSTR::Copy (LPCWSTR str, bool IsBSTR)		// need IsBSTR to signal that we have a real BSTR and not a LPWSTR
{
	return str ? (IsBSTR ? Copy((LPCWSTR)str, ::SysStringLen((BSTR)str)) : Copy((LPCWSTR)str,wcslen(str))) : Copy((LPCWSTR)NULL, (size_type)0);
}

//--------------------------------------------------------------------
// direct buffer manipulation
//--------------------------------------------------------------------

/*! \fn inline HRESULT CNxNBSTR::Alloc(size_type len)
	\param	len Number of characters to allocate
	\return	 S_OK or E_OUTOFMEMORY
	<dl compact><dt><b>Comments</b>
	Frees current string and allocates empty, null terminated string with given length.
	</dl>		
*/
inline HRESULT CNxNBSTR::Alloc(size_type len)
{
	bool bDeleted = m_str != NULL;
	::SysFreeString(m_str);
	m_str = ::SysAllocStringLen(0,len);
	if (m_str)
		m_str[len] = L'\0';

	// notification about string changes
	if (bDeleted)
		_Deleted();

	return m_str ? S_OK : E_OUTOFMEMORY;
}

/*! \fn inline void CNxNBSTR::Free()
	<dl compact><dt><b>Comments</b>
	Frees internal buffer.
	</dl>		
*/
inline void CNxNBSTR::Free()
{
	_Free();
	// notification about string changes
	_Deleted();
}

/*! \fn inline CNxNBSTR& CNxNBSTR::Attach(BSTR src)
	\param	src	Real BSTR
	\return	itself 
	<dl compact><dt><b>Comments</b>
	Frees the current buffer and makes the given buffer its internal buffer.
	This function does not make a copy of the string, so do not delete the buffer by yourself.
	</dl>		
*/
inline CNxNBSTR& CNxNBSTR::Attach(BSTR src)
{
	::SysFreeString(m_str);
	m_str = src;
	
	// notification about string changes
	_Replaced();

	return *this;
}
/*! \fn inline BSTR CNxNBSTR::Detach()
	\return	 Buffer pointer
	<dl compact><dt><b>Comments</b>
	Sets its internal buffer to null and returns the original buffer.
	</dl>		
*/
inline BSTR CNxNBSTR::Detach()
{
	BSTR s = m_str;
	m_str = NULL;

	// notification about string changes
	_Deleted();

	return s;
}

/*! \fn inline BSTR CNxNBSTR::GetBSTR() const
	\return	 Returns the internal buffer
	<dl compact><dt><b>Comments</b>
	Do not free the returned buffer. 
	</dl>		
*/
inline BSTR CNxNBSTR::GetBSTR() const
{
	return m_str;
}
/*! \fn inline BSTR* CNxNBSTR::GetBSTRPtr()
	\return	 Returns a pointer to the internal buffer.
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline BSTR* CNxNBSTR::GetBSTRPtr()
{
	return &m_str;
}


//--------------------------------------------------------------------
// buffer manipulation
//--------------------------------------------------------------------

/*! \fn inline HRESULT CNxNBSTR::Append(LPCWSTR psz)
	\param	psz Null terminated wide character string.
	\return	S_OK or E_OUTOFMEMORY 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline HRESULT CNxNBSTR::Append(LPCWSTR p)
{
	return Append((LPCWSTR)p, (size_type)-1);
}
/*! \fn inline HRESULT CNxNBSTR::Append(LPCWSTR psz, bool IsBSTR)
	\param	psz Null terminated wide character string or real BSTR.
	\param	IsBSTR true, when str is a real BSTR
	\return	S_OK or E_OUTOFMEMORY  
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline HRESULT CNxNBSTR::Append(LPCWSTR p, bool IsBSTR)
{
	return IsBSTR ? Append((LPCWSTR)p, SysStringLen((BSTR)p)) : Append((LPCWSTR)p, wcslen(p));
}
/*! \fn inline HRESULT CNxNBSTR::Append(const CNxNBSTR& bstrSrc)
	\param	src	CNxNBSTR to append
	\return	 S_OK or E_OUTOFMEMORY
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline HRESULT CNxNBSTR::Append(const CNxNBSTR& bstrSrc)
{
	return Append((LPCWSTR)bstrSrc, true);
}
/*! \fn inline HRESULT CNxNBSTR::Append(LPCSTR psz, size_type len)
	\param	psz	Ansi character string to append.
	\param	len Number of characters to append.
	\return	 S_OK or E_OUTOFMEMORY
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline HRESULT CNxNBSTR::Append(LPCSTR lpsz, size_type len)
{
	return Append(CNxNBSTR(lpsz,len));
}
/*! \fn inline HRESULT CNxNBSTR::Append(char c)
	\param	c Ansi character to append
	\return	 S_OK, E_OUTOFMEMORY or E_INAVALIDARG when c is null character
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline HRESULT CNxNBSTR::Append(char c)
{
	if (c=='\0') return E_INVALIDARG;
	return Append(CNxNBSTR(c));
}
/*! \fn inline HRESULT CNxNBSTR::Append(wchar_t c)
	\param	c Wide character to append
	\return	 S_OK, E_OUTOFMEMORY or E_INAVALIDARG when c is null character
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline HRESULT CNxNBSTR::Append(wchar_t c)
{
	if (c==L'\0') return E_INVALIDARG;
	return Append(&c, size_type(1));
}


//--------------------------------------------------------------------
// buffer copy
//--------------------------------------------------------------------

/*! \fn inline BSTR CNxNBSTR::GetCopy() const
	\return	 A copy of the internal buffer or NULL
	<dl compact><dt><b>Comments</b>
	The copy is allocated with 'SysAllocStringLen', so you have to free the copy with 'SysFreeString'
	</dl>		
*/
inline BSTR CNxNBSTR::GetCopy() const
{
	if (m_str==NULL) return NULL;
	return ::SysAllocStringLen(m_str, GetLength());
}

/*! \fn inline LPWSTR CNxNBSTR::GetCopyWSTR() const
	\return	 A copy of the internal buffer or NULL
	<dl compact><dt><b>Comments</b>
	The copy is allocated with 'malloc', so you have to free the copy with 'free'
	</dl>		
*/
inline LPWSTR CNxNBSTR::GetCopyWSTR() const
{
	if (m_str==NULL) return NULL;
	LPWSTR p=NULL;
	CopyToWSTR (&p);
	return p;
}

/*! \fn inline HRESULT CNxNBSTR::AllocCopy(CNxNBSTR& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const
	\param	dest CNxNBSTR to receive the new string
	\param	nCopyLen Number of characters to copy from original string
	\param	nCopyIndex Start character to copy original string
	\param	nExtraLen Number of extra characters to allocate after the copy
	\return	 S_OK or E_OUTOFMEMORY
	<dl compact><dt><b>Comments</b>
	Will clone the data attached to this string allocating 'nExtraLen' characters
	Places results in uninitialized string 'dest'
	Will copy the part or all of original data to start of new string
	A null character is placed afterwards, allocating a total of nCopyLen plus nExtraLen plus one characters. 
	</dl>		
*/
inline HRESULT CNxNBSTR::AllocCopy(CNxNBSTR& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const
{
	// will clone the data attached to this string allocating 'nExtraLen' characters
	// Places results in uninitialized string 'dest'
	// Will copy the part or all of original data to start of new string
	int nNewLen = nCopyLen + nExtraLen;
	if (nNewLen != 0)
	{
		dest.m_str = ::SysAllocStringLen(m_str+nCopyIndex, nNewLen);
		if (dest.m_str) dest.m_str[nNewLen] = L'\0';
		return dest.m_str == NULL ? E_OUTOFMEMORY : S_OK;
	}
	return S_OK;
}

/*! \fn inline HRESULT CNxNBSTR::CopyTo(BSTR* pbstr) const
	\param	pbstr Pointer to a BSTR
	\return	 S_OK, E_OUTOFMEMORY or E_INVALIDARG when pbstr is null or pbstr points to internal buffer
	<dl compact><dt><b>Comments</b>
	It does not free the given BSTR before allocating a new buffer.
	</dl>		
*/
inline HRESULT CNxNBSTR::CopyTo(BSTR* pbstr) const
{
	ASSERT_OR_RETURN(pbstr != NULL, E_INVALIDARG);
	RETURN_IF (m_str == NULL, S_OK);
	ASSERT_OR_RETURN(*pbstr != m_str, E_INVALIDARG);
	*pbstr = ::SysAllocStringLen(m_str, ::SysStringLen(m_str));
	return (*pbstr == NULL) ?  E_OUTOFMEMORY : S_OK;
}

/*! \fn inline HRESULT CNxNBSTR::CopyTo(CNxNBSTR& src) const
	\param	src CNxNBSTR which will receive the copy
	\return	 S_OK or E_OUTOFMEMORY
	<dl compact><dt><b>Comments</b>
	The routine frees the given string before allocating a new buffer.
	</dl>		
*/
inline HRESULT CNxNBSTR::CopyTo(CNxNBSTR& src) const
{
	if (m_str != src.m_str)
	{
		::SysFreeString(src.m_str);
		src.m_str = GetCopy();
	}
	return S_OK;
}

#define AB_MIN(a,b) ((a) < (b) ? (a) : (b))

/*! \fn inline HRESULT CNxNBSTR::CopyToTSTR(LPTSTR ptstr, size_type max_len) const
	\param	ptstr Pointer to ansi or wide character string buffer
	\param  max_len Number of characters which will fit into the given buffer
	\return	 S_OK, E_OUTOFMEMORY or E_INVALIDARG when max_len < 1 or ptstr == NULL
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline HRESULT CNxNBSTR::CopyToTSTR(LPTSTR ptstr, size_type max_len) const
{
	ASSERT_OR_RETURN(ptstr != NULL, E_INVALIDARG);
	ASSERT_OR_RETURN(max_len > 0, E_INVALIDARG);
	size_type Len = GetLength();
	if (!Len)
	{
		*ptstr = _T('\0');
		return S_OK;
	}

	if (sizeof(TCHAR)==sizeof(WCHAR))
		memcpy (ptstr, m_str, (AB_MIN (Len+1,max_len))*2);
	else
		AtlW2AHelper(ptstr,m_str,AB_MIN (Len+1,max_len));


	if (max_len < Len+1)
	{
		ptstr[max_len-1] = _T('\0');
	}
	return S_OK;
}

/*! \fn inline HRESULT CNxNBSTR::CopyToWSTR(LPWSTR* pwstr) const
	\param	pwstr Pointer to a wide character string
	\return	 S_OK, E_OUTOFMEMORY or E_INVALIDARG when pwstr != NULL or *pwstr == m_str
	<dl compact><dt><b>Comments</b>
	The buffer will be allocated with 'malloc', so you have to free it with 'free'.
	</dl>		
*/
inline HRESULT CNxNBSTR::CopyToWSTR(LPWSTR* pwstr) const
{
	ASSERT_OR_RETURN(pwstr != NULL, E_INVALIDARG);
	ASSERT_OR_RETURN(*pwstr != m_str, E_INVALIDARG);
	size_type Len = GetLength();
	if (!Len)
	{
		*pwstr = NULL;
		return S_OK;
	}
	*pwstr = (LPWSTR)malloc(::SysStringLen(m_str));
	if (*pwstr == NULL ) return E_OUTOFMEMORY;
	memcpy (*pwstr, m_str, (Len+1)*2);
	return S_OK;
}

/*! \fn inline HRESULT CNxNBSTR::CopyToWSTR(LPWSTR pwstr, size_type max_len) const
	\param	pwstr Pointer to wide character string buffer
	\param  max_len Number of characters which will fit into the given buffer
	\return	 S_OK, E_OUTOFMEMORY or E_INVALIDARG when max_len < 1 or pwstr == NULL
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline HRESULT CNxNBSTR::CopyToWSTR(LPWSTR pwstr, size_type max_len) const
{
	ASSERT_OR_RETURN(pwstr != NULL, E_INVALIDARG);
	ASSERT_OR_RETURN(max_len > 0, E_INVALIDARG);
	size_type Len = GetLength();
	if (!Len)
	{
		*pwstr = L'\0';
		return S_OK;
	}
	memcpy (pwstr, m_str, (AB_MIN(Len+1,max_len))*2);
	if (max_len < Len+1)
	{
		pwstr[max_len-1] = L'\0';
	}
	return S_OK;
}

#undef AB_MIN 

//--------------------------------------------------------------------
// character / substring retrieval
//--------------------------------------------------------------------

/*! \fn inline WCHAR CNxNBSTR::GetAt(UINT nIndex) const
	\param	nIndex of character to return
	\return	 The character from given index or null character
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline WCHAR CNxNBSTR::GetAt(UINT nIndex) const
{
	ASSERT_OR_RETURN(nIndex < GetLength(), L'\0');
	return m_str[nIndex];
}
/*! \fn inline HRESULT CNxNBSTR::SetAt(UINT nIndex, WCHAR c)
	\param	nIndex of character to set
	\param  c Character to set at given index
	\return	 S_OK or E_INVALIDARG when index is out of range
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline HRESULT CNxNBSTR::SetAt(UINT nIndex, WCHAR ch)
{
	ASSERT_OR_RETURN(nIndex < GetLength(), E_INVALIDARG);
	m_str[nIndex] = ch;
	return S_OK;
}
/*! \fn inline CNxNBSTR CNxNBSTR::Mid(UINT nFirst) const
	\param	nFirst Index of first character to return
	\return	 All characters starting from nFirst to the end
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR CNxNBSTR::Mid(UINT nFirst) const
{
	return Mid(nFirst, GetLength() - nFirst);
}


//--------------------------------------------------------------------
// type conversion
//--------------------------------------------------------------------

/*! \fn inline CNxNBSTR& CNxNBSTR::FromChar(WCHAR c)
	\param	c Character from which to build the string
	\return	 itself
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR& CNxNBSTR::FromChar(WCHAR c)
{
	Alloc(1);
	if (m_str) *m_str = c;

	_Replaced();

	return *this;
}
/*! \fn inline CNxNBSTR& CNxNBSTR::FromInt(int v, int radix)
	\param	v Integer from which to build the string
	\param radix The radix to build the string with
	\return	 itself
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR& CNxNBSTR::FromInt(int v, int radix)
{
	static wchar_t sDummy[(sizeof(int) << 3) + 1];
	*this = _itow(v, sDummy, radix);
	
	_Replaced();

	return *this;
}
/*! \fn inline CNxNBSTR& CNxNBSTR::FromLong(long v, int radix)
	\param	v Integer from which to build the string
	\param radix The radix to build the string with
	\return	 itself
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR& CNxNBSTR::FromLong(long v, int radix)
{
	static wchar_t sDummy[(sizeof(long) << 3) + 1];
	*this = _ltow(v, sDummy, radix);

	_Replaced();

	return *this;
}
/*! \fn inline CNxNBSTR& CNxNBSTR::FromInt64(_int64 v, int radix)
	\param	v The 64 bit Integer from which to build the string
	\param radix The radix to build the string with
	\return	 itself
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR& CNxNBSTR::FromInt64(_int64 v, int radix)
{
	static wchar_t sDummy[(sizeof(_int64) << 3) + 1];
	*this = _i64tow(v, sDummy, radix);

	_Replaced();

	return *this;
}

/*! \fn inline CNxNBSTR& CNxNBSTR::FromUInt(unsigned int v, unsigned int radix)
	\param	v Integer FromU which to build the string
	\param radix The radix to build the string with
	\return	 itself
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR& CNxNBSTR::FromUInt(unsigned int v, int radix)
{
	static wchar_t sDummy[(sizeof(unsigned long) << 3) + 1];
	*this = _ultow(v, sDummy, radix);

	_Replaced();

	return *this;
}
/*! \fn inline CNxNBSTR& CNxNBSTR::FromULong(unsigned long v, unsigned int radix)
	\param	v Integer FromU which to build the string
	\param radix The radix to build the string with
	\return	 itself
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR& CNxNBSTR::FromULong(unsigned long v, int radix)
{
	static wchar_t sDummy[(sizeof(unsigned long) << 3) + 1];
	*this = _ultow(v, sDummy, radix);

	_Replaced();

	return *this;
}
/*! \fn inline CNxNBSTR& CNxNBSTR::FromUInt64(unsigned _int64 v, unsigned int radix)
	\param	v The 64 bit Integer FromU which to build the string
	\param radix The radix to build the string with
	\return	 itself
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR& CNxNBSTR::FromUInt64(unsigned _int64 v, int radix)
{
	static wchar_t sDummy[(sizeof(unsigned _int64) << 3) + 1];
	*this = _ui64tow(v, sDummy, radix);

	_Replaced();

	return *this;
}
/*! \fn inline CNxNBSTR& CNxNBSTR::FromBool(bool b)
	\param	b Boolean value true or false
	\return	 itself
	<dl compact><dt><b>Comments</b>
	The string will be "1" or "0"
	</dl>		
*/
inline CNxNBSTR& CNxNBSTR::FromBool(bool b)
{
	Alloc(1);

	if (m_str) *m_str = b ? L'1' : L'0';

	_Replaced();

	return *this;
}

/*! \fn inline int CNxNBSTR::AsInt(int base) const
	\return	 the integer value of the string or 0
	<dl compact><dt><b>Comments</b>
	The method uses wtoi for transformation.
	</dl>		
*/
inline int CNxNBSTR::AsInt(int base) const
{
	if (GetLength()==0) return 0;
	return wcstol(m_str, NULL, base);
}
/*! \fn inline long CNxNBSTR::AsLong(int base) const
	\return	 the integer value of the string or 0
	<dl compact><dt><b>Comments</b>
	The method uses wtol for transformation.
	</dl>		
*/
inline long CNxNBSTR::AsLong(int base) const
{
	if (GetLength()==0) return 0;
	return wcstol(m_str, NULL, base);
}
/*! \fn inline __int64 CNxNBSTR::AsInt64() const
	\return	 the integer value of the string or 0
	<dl compact><dt><b>Comments</b>
	The method uses wtoi64 for transformation.
	</dl>		
*/
inline __int64 CNxNBSTR::AsInt64() const
{
	if (GetLength()==0) return 0;
	return _wtoi64(m_str);
}
/*! \fn inline int CNxNBSTR::AsUInt(int base) const
	\return	 the integer value of the string or 0
	<dl compact><dt><b>Comments</b>
	The method uses wtoi for transformation.
	</dl>		
*/
inline unsigned int CNxNBSTR::AsUInt(int base) const
{
	if (GetLength()==0) return 0;
	return (unsigned int)wcstoul(m_str, NULL, base);
}
/*! \fn inline long CNxNBSTR::AsULong(int base) const
	\return	 the integer value of the string or 0
	<dl compact><dt><b>Comments</b>
	The method uses wtol for transformation.
	</dl>		
*/
inline unsigned long CNxNBSTR::AsULong(int base) const
{
	if (GetLength()==0) return 0;
	return wcstoul(m_str, NULL, base);
}
/*! \fn inline __int64 CNxNBSTR::AsUInt64() const
	\return	 the integer value of the string or 0
	<dl compact><dt><b>Comments</b>
	The method uses wtoi64 for transformation.
	</dl>		
*/
inline unsigned __int64 CNxNBSTR::AsUInt64() const
{
	if (GetLength()==0) return 0;
	return (unsigned __int64) _wtoi64(m_str);
}
/*! \fn inline float CNxNBSTR::AsFloat() const
	\return	 the float value of the string or .0
	<dl compact><dt><b>Comments</b>
	The routine uses atof for transformation.
	</dl>		
*/
inline float CNxNBSTR::AsFloat() const
{
	return (float)AsDouble();
}
/*! \fn inline double CNxNBSTR::AsDouble() const
	\return	 the float value of the string or .0
	<dl compact><dt><b>Comments</b>
	The routine uses atof for transformation.
	</dl>		
*/
inline double CNxNBSTR::AsDouble() const
{
	if (GetLength()==0) return .0;
	return wcstod(m_str,NULL);
}
/*! \fn inline bool CNxNBSTR::AsBool() const
	\return	 the bool value of the string or false
	<dl compact><dt><b>Comments</b>
	The routine returns true, if the string is "1" or "TRUE" (case insensitive) otherwise false.
	</dl>		
*/
inline bool CNxNBSTR::AsBool() const
{
	if (GetLength()==0) return false;
	return (GetLength()==1 && *m_str == L'1') || (GetLength()==4 && CompareNoCase(L"TRUE")==0);
}



//--------------------------------------------------------------------
// helper methods for comparison
//--------------------------------------------------------------------

/*! \fn inline bool CNxNBSTR::IsEqual (LPCWSTR psz1, LPCWSTR psz2)
	\param	psz1 Null terminated wide character string
	\param	psz2 Null terminated wide character string to compare
	\return	 true when every character of string one is the same as the character of string two.
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool CNxNBSTR::IsEqual (LPCWSTR pc1, LPCWSTR pc2)
{
	if (pc1 == pc2) return true;
	if (pc1 == NULL) return *pc2==L'\0';
	if (pc2 == NULL) return *pc1==L'\0';

	LPWSTR p1=(LPWSTR)pc1;
	LPWSTR p2=(LPWSTR)pc2;

	while (*p1==*p2)
	{
		if (*p1==L'\0') return true;
		p1++; p2++;
	}
	return false;
}
/*! \fn inline bool CNxNBSTR::IsLess (LPCWSTR psz1, LPCWSTR psz2)
	\param	psz1 Null terminated wide character string
	\param	psz2 Null terminated wide character string to compare
	\return	 true when psz1 is less than psz2
	<dl compact><dt><b>Comments</b>
	The routine uses wcscmp to compare the strings
	</dl>		
*/
inline bool CNxNBSTR::IsLess (LPCWSTR p1, LPCWSTR p2)
{
	if (p1==p2) return false;
	if (p1==NULL) return true;
	if (p2==NULL) return false;
	return wcscmp(p1,p2) < 0;
}
/*! \fn inline bool CNxNBSTR::IsGreater (LPCWSTR psz1, LPCWSTR psz2)
	\param	psz1 Null terminated wide character string
	\param	psz2 Null terminated wide character string to compare
	\return	 true when psz1 is greater than psz2
	<dl compact><dt><b>Comments</b>
	The routine uses wcscmp to compare the strings
	</dl>		
*/
inline bool CNxNBSTR::IsGreater (LPCWSTR p1, LPCWSTR p2)
{
	if (p1==p2) return false;
	if (p2==NULL) return true;
	if (p1==NULL) return false;
	return wcscmp(p1,p2) > 0;
}


//--------------------------------------------------------------------
// comparison
//--------------------------------------------------------------------

/*! \fn inline int CNxNBSTR::Compare(LPCWSTR psz, int nLen) const
	\param	psz Null terminated wide character string to compare with
	\param	nLen Optional parameter to specify the number of characters to be compared
	\return	 0 when both strings are equal, -1 when CNxNBSTR is less than given string, 1 when CNxNBSTR is greater than given string
	<dl compact><dt><b>Comments</b>
	The routine uses wcscmp or wcsncmp to compare the strings
	This routine is case sensitive.
	</dl>		
*/
inline int CNxNBSTR::Compare(LPCWSTR lpsz, int nLen) const
{ 
	if (m_str==lpsz) return 0;
	if (m_str==NULL) return -1;
	if (lpsz==NULL)  return 1;
	return m_str ? ((nLen < 0) ? wcscmp(m_str, lpsz) : wcsncmp(m_str, lpsz, nLen)) : (lpsz ? -1 : 0); 
}   
/*! \fn inline int CNxNBSTR::CompareNoCase(LPCWSTR psz, int nLen) const
	\param	psz Null terminated wide character string to compare with
	\param	nLen Optional parameter to specify the number of characters to be compared
	\return	 0 when both strings are equal, -1 when CNxNBSTR is less than given string, 1 when CNxNBSTR is greater than given string
	<dl compact><dt><b>Comments</b>
	The routine uses wcsicmp or wcsnicmp to compare the strings
	This routine is case insensitive.
	</dl>		
*/
inline int CNxNBSTR::CompareNoCase(LPCWSTR lpsz, int nLen) const
{ 
	if (m_str==lpsz) return 0;
	if (m_str==NULL) return -1;
	if (lpsz==NULL)  return 1;
	return m_str ? ((nLen < 0) ? _wcsicmp(m_str, lpsz) : _wcsnicmp(m_str, lpsz, nLen)) : (lpsz ? -1 : 0); 
}  
/*! \fn inline int CNxNBSTR::Collate(LPCWSTR psz, int nLen) const
	\param	psz Null terminated wide character string to compare with
	\param	nLen Optional parameter to specify the number of characters to be compared
	\return	 0 when both strings are equal, -1 when CNxNBSTR is less than given string, 1 when CNxNBSTR is greater than given string
	<dl compact><dt><b>Comments</b>
	The routine uses wcscoll or wcsncoll to compare the strings
	This routine is case sensitive and locale sensitive with respect to sort order.
	</dl>		
*/
inline int CNxNBSTR::Collate(LPCWSTR lpsz, int nLen) const
{ 
	if (m_str==lpsz) return 0;
	if (m_str==NULL) return -1;
	if (lpsz==NULL)  return 1;
	return m_str ? ((nLen < 0) ? wcscoll(m_str, lpsz) : _wcsncoll(m_str, lpsz, nLen)) : (lpsz ? -1 : 0);  
}   
/*! \fn inline int CNxNBSTR::CollateNoCase(LPCWSTR psz, int nLen) const
	\param	psz Null terminated wide character string to compare with
	\param	nLen Optional parameter to specify the number of characters to be compared
	\return	 0 when both strings are equal, -1 when CNxNBSTR is less than given string, 1 when CNxNBSTR is greater than given string
	<dl compact><dt><b>Comments</b>
	The routine uses wcsicoll or wcsnicoll to compare the strings
	This routine is case insensitive and locale sensitive with respect to sort order.
	</dl>		
*/
inline int CNxNBSTR::CollateNoCase(LPCWSTR lpsz, int nLen) const
{ 
	if (m_str==lpsz) return 0;
	if (m_str==NULL) return -1;
	if (lpsz==NULL)  return 1;
	return m_str ? ((nLen < 0) ? wcsicoll(m_str, lpsz) : _wcsnicoll(m_str, lpsz, nLen)) : (lpsz ? -1 : 0); 
}   





//--------------------------------------------------------------------
// operator implementation
//--------------------------------------------------------------------

/*! \fn inline bool CNxNBSTR::operator!() const
	\return	 true when internal buffer pointer is NULL
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool CNxNBSTR::operator!() const
{
	return (m_str == NULL);
}

/*! \fn inline CNxNBSTR::operator BSTR() const
	\return	 internal buffer
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR::operator BSTR() const		// == LPWSTR
{
	return m_str;
}
/*! \fn inline CNxNBSTR::operator LPCWSTR() const
	\return	 internal buffer
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR::operator LPCWSTR() const
{
	return m_str;
}
/*! \fn inline CNxNBSTR::operator LPCSTR () const
	\return	 Ansi version of internal buffer
	<dl compact><dt><b>Comments</b>
	The routine is using a static array of 20 buffers to hold the converted strings.
	After the 20th call, the routine reuses the first buffer.
	The result of the method should only be used temporarely.
	</dl>		
*/
inline CNxNBSTR::operator LPCSTR () const
{ 
	// return value is created on the fly and only valid until the next 20th call to this method in any CNxNBSTR variable
	//NXN_ASSERT(!"use ""char* CopyTo(char** pNewStr) instead of (const char*)""");
	if (m_str==NULL) return NULL;
	
	//we are using 20 buffers which are circulary used, so the current result is valid until the 20th call of this method
	static CMallocPointer<char> buffers[MAX_STATIC_BUFFER];
	static int x=0;
	CMallocPointer<char> &buffer=buffers[x++];
	if(x==20) x = 0;

	buffer.Alloc ((GetLength()+1)*2);
	ATLW2AHELPER((char*)buffer, m_str, buffer.GetLength());

	return buffer;
}

/*! \fn inline CNxNBSTR::operator BSTR*() 
	\return	 Pointer to internal buffer
	<dl compact><dt><b>Comments</b>
	Before the pointer to its internal buffer is returned, the routine frees the internal buffer.
	This operator is normally used to receive new content from outside.
	</dl>		
*/
/*
inline CNxNBSTR::operator BSTR*() // replacement for & CNxNBSTR::operator, i am not sure that this idea is any good
{
	_Free ();		// free content, before receiving new string
	return &m_str;
}
*/
/*! \fn inline wchar_t CNxNBSTR::operator [] (int nChar) const
	\param	nChar The index of the requested character.
	\return	 The character from the nChar position in the buffer or '\0' when the index is out of range.
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline wchar_t CNxNBSTR::operator [] (int nChar) const
{
	return GetAt(nChar);
}
/*! \fn inline wchar_t& CNxNBSTR::operator [] (int nChar)
	\param	nChar The index of the requested character.
	\return	 A reference to the character from the nChar position in the buffer or a reference to a '\0' character when the index is out of range.
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline wchar_t& CNxNBSTR::operator [] (int nChar)
{
	static wchar_t dummy=L'\0';
	ASSERT_OR_RETURN((size_type)nChar < GetLength(), dummy);
	return m_str[nChar];
}
/*! \fn inline wchar_t CNxNBSTR::operator [] (size_type nChar) const
	\param	nChar The index of the requested character.
	\return	 The character from the nChar position in the buffer or '\0' when the index is out of range.
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline wchar_t CNxNBSTR::operator [] (size_type nChar) const
{
	return GetAt(nChar);
}
/*! \fn inline wchar_t& CNxNBSTR::operator [] (size_type nChar)
	\param	nChar The index of the requested character.
	\return	 A reference to the character from the nChar position in the buffer or a reference to a '\0' character when the index is out of range.
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline wchar_t& CNxNBSTR::operator [] (size_type nChar)
{
	static wchar_t dummy=L'\0';
	ASSERT_OR_RETURN(nChar < GetLength(), dummy);
	return m_str[nChar];
}
/*! \fn inline CNxNBSTR& CNxNBSTR::operator=(const CNxNBSTR& src)
	\param	src The CNxNBSTR to be copied
	\return	 reference to itself
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR& CNxNBSTR::operator=(const CNxNBSTR& src)
{
	if (m_str != src.m_str)
	{
		::SysFreeString(m_str);
		m_str = src.GetCopy();
	}
	return *this;
}
/*! \fn inline CNxNBSTR& CNxNBSTR::operator=(const CNxNBSTRRef& refsrc)
	\param	src The CNxNBSTRRef to be copied
	\return	 reference to itself
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR& CNxNBSTR::operator=(const CNxNBSTRRef& refsrc)
{
	return this->operator = ((CNxNBSTR&)refsrc);
}
/*! \fn inline CNxNBSTR& CNxNBSTR::operator += (const CNxNBSTR& bstrSrc)
	\param	src The CNxNBSTR to be appended
	\return	 reference to itself
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR& CNxNBSTR::operator += (const CNxNBSTR& bstrSrc)
{
	Append(bstrSrc.m_str,true);
	return *this;
}


/////////////////////////////////////////////////////////////////////////
// operator +

/*! \fn inline CNxNBSTR operator + (const CNxNBSTR& s1, const CNxNBSTR& s2)
	\param	s1 first part of result string
	\param	s2 second part of result string
	\return	 CNxNBSTR object with complete string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR operator + (const CNxNBSTR& s1, const CNxNBSTR& s2)
{
	return CNxNBSTR(s1)+=s2;
}
/*! \fn inline CNxNBSTR operator + (LPCSTR psz, const CNxNBSTR& s)
	\param	psz first part of result string
	\param	s second part of result string
	\return	 CNxNBSTR object with complete string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR operator + (LPCSTR p, const CNxNBSTR& s)
{
	return CNxNBSTR(p)+=s;
}
/*! \fn inline CNxNBSTR operator + (LPCWSTR psz, const CNxNBSTR& s)
	\param	psz first part of result string
	\param	s second part of result string
	\return	 CNxNBSTR object with complete string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR operator + (LPCWSTR p, const CNxNBSTR& s)
{
	return CNxNBSTR(p)+=s;
}
/*! \fn inline CNxNBSTR operator + (wchar_t c, const CNxNBSTR& s)
	\param	c first part of result string
	\param	s second part of result string
	\return	 CNxNBSTR object with complete string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR operator + (wchar_t c, const CNxNBSTR& s)
{
	return CNxNBSTR(c)+=s;
}
/*! \fn inline CNxNBSTR operator + (char c, const CNxNBSTR& s)
	\param	c first part of result string
	\param	s second part of result string
	\return	 CNxNBSTR object with complete string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR operator + (char c, const CNxNBSTR& s)
{
	return CNxNBSTR(c)+=s;
}
/*! \fn inline CNxNBSTR operator + (const CNxNBSTR& s, LPCSTR psz)
	\param	s first part of result string
	\param	psz Null terminated ansi character string, serving as second part of result string
	\return	 CNxNBSTR object with complete string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR operator + (const CNxNBSTR& s, LPCSTR p)
{
	return CNxNBSTR(s)+=p;
}
/*! \fn inline CNxNBSTR operator + (const CNxNBSTR& s, LPCWSTR psz)
	\param	s first part of result string
	\param	psz Null terminated wide character string, serving as second part of result string
	\return	 CNxNBSTR object with complete string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR operator + (const CNxNBSTR& s, LPCWSTR p)
{
	return CNxNBSTR(s)+=p;
}
/*! \fn inline CNxNBSTR operator + (const CNxNBSTR& s, wchar_t c)
	\param	s first part of result string
	\param	c second part of result string
	\return	 CNxNBSTR object with complete string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR operator + (const CNxNBSTR& s, wchar_t c)
{
	return CNxNBSTR(s)+=c;
}
/*! \fn inline CNxNBSTR operator + (const CNxNBSTR& s, char c)
	\param	s first part of result string
	\param	c second part of result string
	\return	 CNxNBSTR object with complete string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline CNxNBSTR operator + (const CNxNBSTR& s, char c)
{
	return CNxNBSTR(s)+=c;
}


/////////////////////////////////////////////////////////////////////////
// operator ==

/*! \fn inline bool operator == (const CNxNBSTR& s1, const CNxNBSTR& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator == (const CNxNBSTR& s1, const CNxNBSTR& s2)
{
	return (s1.GetLength() == s2.GetLength() && CNxNBSTR::IsEqual((LPCWSTR)s1,(LPCWSTR)s2));
}
/*! \fn inline bool operator == (LPCSTR s1, const CNxNBSTR& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator == (LPCSTR p, const CNxNBSTR& s)
{
	return CNxNBSTR::IsEqual(s, CNxNBSTR(p));
}
/*! \fn inline bool operator == (LPCWSTR s1, const CNxNBSTR& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator == (LPCWSTR p, const CNxNBSTR& s)
{
	return CNxNBSTR::IsEqual((LPCWSTR)s, p);
}
/*! \fn inline bool operator == (const CNxNBSTR& s1, LPCSTR s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator == (const CNxNBSTR& s, LPCSTR p)
{
	return CNxNBSTR::IsEqual((LPCWSTR)s, CNxNBSTR(p));
}
/*! \fn inline bool operator == (const CNxNBSTR& s1, LPCWSTR s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator == (const CNxNBSTR& s, LPCWSTR p)
{
	return CNxNBSTR::IsEqual((LPCWSTR)s, p);
}

/////////////////////////////////////////////////////////////////////////
// operator !=

/*! \fn inline bool operator != (const CNxNBSTR& s1, const CNxNBSTR& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are not equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator != (const CNxNBSTR& s1, const CNxNBSTR& s2)
{
	return (s1.GetLength() != s2.GetLength() || !CNxNBSTR::IsEqual((LPCWSTR)s1,(LPCWSTR)s2));
}
/*! \fn inline bool operator != (LPCSTR s1, const CNxNBSTR& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are not equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator != (LPCSTR p, const CNxNBSTR& s)
{
	return !CNxNBSTR::IsEqual((LPCWSTR)s, (LPCWSTR)CNxNBSTR(p));
}
/*! \fn inline bool operator != (LPCWSTR s1, const CNxNBSTR& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are not equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator != (LPCWSTR p, const CNxNBSTR& s)
{
	return !CNxNBSTR::IsEqual((LPCWSTR)s, p);
}
/*! \fn inline bool operator != (const CNxNBSTR& s1, LPCSTR s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are not equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator != (const CNxNBSTR& s, LPCSTR p)
{
	return !CNxNBSTR::IsEqual((LPCWSTR)s, (LPCWSTR)CNxNBSTR(p));
}
/*! \fn inline bool operator != (const CNxNBSTR& s1, LPCWSTR s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when both strings are not equal 
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator != (const CNxNBSTR& s, LPCWSTR p)
{
	return !CNxNBSTR::IsEqual((LPCWSTR)s, p);
}

/////////////////////////////////////////////////////////////////////////
// operator <

/*! \fn inline bool operator < (const CNxNBSTR& s1, const CNxNBSTR& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is less then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator < (const CNxNBSTR& s1, const CNxNBSTR& s2)
{
	return CNxNBSTR::IsLess(s1,s2);
}
/*! \fn inline bool operator < (const CNxNBSTR& s1, LPCWSTR s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is less then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator < (const CNxNBSTR& s1, LPCWSTR s2)
{
	return CNxNBSTR::IsLess(s1,s2);
}
/*! \fn inline bool operator < (LPCWSTR s1,	 const CNxNBSTR& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is less then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator < (LPCWSTR s1,	 const CNxNBSTR& s2)
{
	return CNxNBSTR::IsLess(s1,s2);
}
/*! \fn inline bool operator < (const CNxNBSTR& s1, LPCSTR s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is less then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator < (const CNxNBSTR& s1, LPCSTR s2)
{
	return CNxNBSTR::IsLess(s1,CNxNBSTR(s2));
}
/*! \fn inline bool operator < (LPCSTR s1,	 const CNxNBSTR& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is less then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator < (LPCSTR s1,	 const CNxNBSTR& s2)
{
	return CNxNBSTR::IsLess(CNxNBSTR(s1),s2);
}

/////////////////////////////////////////////////////////////////////////
// operator >

/*! \fn inline bool operator > (const CNxNBSTR& s1, const CNxNBSTR& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is greater then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator > (const CNxNBSTR& s1, const CNxNBSTR& s2)
{
	return CNxNBSTR::IsGreater(s1,s2);
}
/*! \fn inline bool operator > (const CNxNBSTR& s1, LPCWSTR s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is greater then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator > (const CNxNBSTR& s1, LPCWSTR s2)
{
	return CNxNBSTR::IsGreater(s1,s2);
}
/*! \fn inline bool operator > (const CNxNBSTR& s1, LPCSTR s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is greater then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator > (const CNxNBSTR& s1, LPCSTR s2)
{
	return CNxNBSTR::IsGreater(s1,CNxNBSTR(s2));
}
/*! \fn inline bool operator > (LPCWSTR s1,	 const CNxNBSTR& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is greater then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator > (LPCWSTR s1,	 const CNxNBSTR& s2)
{
	return CNxNBSTR::IsGreater(s1,s2);
}
/*! \fn inline bool operator > (LPCSTR s1,	 const CNxNBSTR& s2)
	\param	s1 First string to compare
	\param	s2 Second string to compare
	\return	 true when first string is greater then second string
	<dl compact><dt><b>Comments</b>
	</dl>		
*/
inline bool operator > (LPCSTR s1,	 const CNxNBSTR& s2)
{
	return CNxNBSTR::IsGreater(CNxNBSTR(s1),s2);
}

#endif // INC_NXN_BSTR_H_
