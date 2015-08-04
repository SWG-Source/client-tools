#ifndef INC_NXN_PTR_STRING
#define INC_NXN_PTR_STRING

#include "NxNString.h"

class CNxNPtrString
{
public:
	// stl-sort predicate
	struct PredCollateNoCaseLess : std::binary_function<CNxNPtrString, CNxNPtrString, bool>
	{
		inline bool operator() (const CNxNPtrString& s1, const CNxNPtrString& s2) const
		{
			return s1.CollateNoCase(s2)<0;
		}
	};

	// constrution
	CNxNPtrString() : pBegin(NULL), pEnd(NULL) {}
	CNxNPtrString(LPCWSTR pBegin, CNxNString::size_type Len) : pBegin(LPWSTR(pBegin)), pEnd(LPWSTR(pBegin)+Len) {}
	CNxNPtrString(LPCWSTR pBegin, LPCWSTR pEnd) : pBegin(LPWSTR(pBegin)), pEnd(LPWSTR(pEnd)) {}
	CNxNPtrString(const CNxNString& s) : pBegin(s.GetBSTR()), pEnd(s.GetBSTR()+s.GetLength()) {}
	CNxNPtrString(const CNxNPtrString& s) : pBegin(s.pBegin), pEnd(s.pEnd) {}

	// pointer access
	inline void Set(LPWSTR pBegin, LPWSTR pEnd)
	{
		this->pBegin = pBegin;
		this->pEnd = pEnd;
	}
	inline LPWSTR& Begin() { return pBegin; };
	inline LPWSTR& End() { return pEnd; };

	// string information
	inline int GetLength() const { return pEnd-pBegin; };
	inline bool IsEmpty() const { return GetLength()==0; };

	// returns 0 when a larger string equals a shorter string in the length of the shorter string
	int CollateNoCase(const CNxNPtrString& s2) const
	{
		if (s2.pBegin == pBegin || pBegin == NULL || s2.pBegin == NULL) return 0; //equal than one string is empty is ok
		return _wcsncoll(pBegin,s2.pBegin,min(GetLength(),s2.GetLength()));
	}

	void Clear()
	{
		pBegin = pEnd = NULL;
	}

	bool operator == (LPCWSTR pCompare)
	{
		if (pCompare == NULL) return pBegin == NULL;
		if (pBegin == NULL) return false;

		for (LPWSTR p=pBegin; p != pEnd; p++, pCompare++)
		{
			if (*pCompare == L'\0') return p == pEnd; 
			if (*p != *pCompare) return false;
		}
		return *pCompare == L'\0';
	}

	bool operator != (LPCWSTR pCompare)
	{
		return ! ((*this) == pCompare);
	}
	inline void operator ++() { if (pBegin!=pEnd) pBegin++; };
	inline void CopyTo(CNxNString& rs) { rs.Copy(pBegin,CNxNString::size_type(GetLength())); };

	LPWSTR GetPtrTo(WCHAR c, int nTimes=1)
	{
		NXN_ASSERT(nTimes > 0);
		LPWSTR p = pBegin;
		while (p != pEnd)
		{
			if (*p == c) nTimes --;
			if (nTimes == 0) return p;
			p++;
		}
		return NULL;
	}

	bool AdvanceTo(WCHAR c, int nTimes=1)
	{
		NXN_ASSERT(nTimes > 0);
		while (pBegin != pEnd)
		{
			if (*pBegin == c) nTimes --;
			if (nTimes == 0) return true;
			pBegin++;
		}
		return false;
	}
	bool AdvanceToOneOf(LPCWSTR pChars, int nTimes=1)
	{
		NXN_ASSERT(pChars != NULL);
		LPCWSTR pChar;
		while (pBegin != pEnd)
		{
			pChar = pChars;
			while (*pChar++ != NULL)
			{
				if (*pBegin == *pChar)
				{
					nTimes --;
					break;
				}
			}
			if (nTimes == 0) return true;
			pBegin++;
		}
		return false;
	}
	/*bool AdvanceTo(WCHAR c, int nTimes=1)
	{
		NXN_ASSERT(nTimes > 0);
		while (pBegin != pEnd)
		{
			if (*pBegin == c) nTimes --;
			if (nTimes == 0) return true;
			pBegin++;
		}
		return false;
	}*/
private:
	LPWSTR pBegin;
	LPWSTR pEnd;
};





#endif // INC_NXN_PTR_STRING
