// \addtodoc

#ifndef INC_NXN_PATH_H_
#define INC_NXN_PATH_H_

#include "NxNString.h"
#include <vector>

//disable warnings on extern before template instantiation
#pragma warning (disable : 4231)
    
extern NXNINTEGRATORSDK_API wchar_t FSLASH, BSLASH; 

// "file://NT5/c:/dir/file.txt"  only UNC paths have forward slashes
//  protocol=file host=NT5 drive=c Directory=dir Filetitle=file Extension=txt Filename=file.txt
typedef enum {	P_PROTOCOL,			P_PROTOCOL_COLON,	P_UNC_SLASH,	P_HOST,			P_HOST_SLASH,	
				P_DRIVE,			P_DRIVE_COLON, 		P_DIR_SLASH1,	P_DIR_NAME,		P_DIR_SLASH2,	
				P_FILETITLE,		P_EXT_DOT,			P_EXTENSION,	P_NULL,			P_NUM } ePointer;

NXNINTEGRATORSDK_EXPIMP_TEMPLATE template struct NXNINTEGRATORSDK_API std::pair<ePointer, ePointer>;

class NXNINTEGRATORSDK_API CNxNPath : public CNxNString
{
	public:
		//-----------------------------------------------------------------------
		//	construction
		//-----------------------------------------------------------------------

		/*!	\fn			CNxNPath()
		 *	
		 *				Default constructor
		 *	
		 *	\param		none
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		CNxNPath() : CNxNString()												{ Init(); }
		
		/*!	\fn			CNxNPath(const CNxNPath& src)
		 *	
		 *				Copy constructor.
		 *	
		 *	\param		src The Path to be copied.
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		CNxNPath(const CNxNPath& src)			: CNxNString( (const CNxNString&) src ) { Init(); }

		/*!	\fn			CNxNPath(const CNxNString& src)
		 *	
		 *				
		 *	
		 *	\param		src The string to be copied.
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		CNxNPath(const CNxNString& src)			: CNxNString( src )			{ Init(); }

		/*!	\fn			CNxNPath(LPCSTR psz)
		 *	
		 *				Constructor from null terminated ansi character string.
		 *	
		 *	\param		psz The null terminated ansi character string to be copied.
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		CNxNPath(LPCSTR psz)					: CNxNString( psz )			{ Init(); }

		/*!	\fn			CNxNPath(LPCSTR psz, size_type nChar)
		 *	
		 *				Constructor from ansi character string and length.
		 *	
		 *	\param		psz The string to be copied.	
		 *	\param		nChar The number of characters to copy.
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		CNxNPath(LPCSTR psz, size_type nChar)	: CNxNString( psz, nChar )	{ Init(); }

		/*!	\fn			CNxNPath(LPCWSTR psz)
		 *	
		 *				Constructor from wide character string
		 *	
		 *	\param		psz The null terminated string to be copied.
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		CNxNPath(LPCWSTR psz)					: CNxNString( psz )			{ Init(); }

		/*!	\fn			CNxNPath(LPCWSTR psz, bool IsBSTR)
		 *	
		 *				Constructor from null terminated wide character string or real BSTR with flag signalling the type of the parameter string.
		 *	
		 *	\param		psz A null terminated wide character string or BSTR.
		 *	\param		IsBSTR true, when the given string is a real BSTR.
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		CNxNPath(LPCWSTR psz, bool IsBSTR)		: CNxNString( psz, IsBSTR )	{ Init(); }

		/*!	\fn			CNxNPath(LPCWSTR psz, size_type nChar)
		 *	
		 *				Constructor from wide character string and string length.
		 *	
		 *	\param		psz The string to be copied.	
		 *	\param		nChar The number of characters to copy.
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		CNxNPath(LPCWSTR psz, size_type nChar)	: CNxNString( psz, nChar )	{ Init(); }
	#ifdef _MFC_VER

		/*!	\fn			CNxNPath(const CString& src)
		 *	
		 *				Constructor from a CString object.
		 *	
		 *	\param		src The CString object to be copied.
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		CNxNPath(const CString& src)			: CNxNString( src )			{ Init(); }
	#endif

		//-----------------------------------------------------------------------
		//	cast operators
		//-----------------------------------------------------------------------
		//
		/*!	\fn			inline operator LPCTSTR () const
		 *	
		 *				Cast operator to temporary ansi character string.
		 *	
		 *	\param		pointer to temporary buffer, holding an ansi string version of path.
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		inline operator LPCTSTR () const;

		/*!	\fn			inline operator LPCWSTR () const
		 *	
		 *				Cast operator to constant wide character string.
		 *	
		 *	\param		none
		 *	
		 *	\return		internal BSTR
		 *	
		 *	\note		
		 */
		inline operator LPCWSTR () const;

		/*!	\fn			inline CNxNPath& operator = (const VARIANT& Src)
		 *	
		 *				Assignment operator for variants
		 *	
		 *	\param		none
		 *	
		 *	\return		itself
		 *	
		 *	\note		
		 */
		inline CNxNPath& operator = ( const VARIANT& Src );

		//-----------------------------------------------------------------------
		// validation
		//-----------------------------------------------------------------------

		/*!	\fn			inline static bool IsValidFilename(const CNxNString& s, bool bCanBeEmpty=false)
		 *	
		 *				Checks the given string for invalid filename characters "\\/:*?<>|"
		 *	
		 *	\param		s String to test
		 *	\param		bCanBeEmpty true, when an empty string should be valid
		 *	
		 *	\return		true, when there are no invalid characters
		 *	
		 *	\note		
		 */
		inline static bool NXNINTEGRATORSDK_API_CALL IsValidFilename(const CNxNString& s, bool bCanBeEmpty=false);

		/*!	\fn			inline static bool IsValidDrivename(const CNxNString& s)
		 *	
		 *				Checks the given string for invalid drivename characters (all others than 'a'-'z')
		 *	
		 *	\param		s String to test
		 *	
		 *	\return		true, when the string has only one character between 'a' and 'z' (upper or lowercase)
		 *	
		 *	\note		
		 */
		inline static bool NXNINTEGRATORSDK_API_CALL IsValidDrivename(const CNxNString& s);

		//-----------------------------------------------------------------------
		// High-level element information
		//-----------------------------------------------------------------------
		/*!	\fn			inline CNxNString GetProtocol() const
		 *	
		 *				Returns the protocol part of the shell path, if any.
		 *	
		 *	\param		none
		 *	
		 *	\return		The protocol string
		 *	
		 *	\note		
		 */
		inline CNxNString CNxNPath::GetProtocol() const;

		/*!	\fn			inline CNxNString GetComputerName() const
		 *	
		 *				Returns the computer name part of the shell path, if any.
		 *	
		 *	\param		none
		 *	
		 *	\return		The computer name
		 *	
		 *	\note		
		 */
		inline CNxNString GetComputerName() const;

		/*!	\fn			CNxNPath GetShellPath(bool bWithSlash = true) const
		 *	
		 *				Returns the complete path without a filename
		 *	
		 *	\param		none
		 *	
		 *	\return		The complete path without filename
		 *	
		 *	\note		
		 */
		CNxNPath GetShellPath(bool bWithSlash = true) const;		

		/*!	\fn			CNxNPath GetShellDirectory(bool bWithSlash = true) const
		 *	
		 *				Returns only the directory part of the path, without UNC-part, drive or filename
		 *	
		 *	\param		none
		 *	
		 *	\return		The directory part
		 *	
		 *	\note		
		 */
		CNxNPath GetShellDirectory(bool bWithSlash = true) const;

		/*!	\fn			HRESULT AppendToDirectory(const CNxNString& sNewPart)
		 *	
		 *				Appends the given string at the end of itself and takes care of double slashes.
		 *	
		 *	\param		none
		 *	
		 *	\return		S_OK or E_OUTOFMEMORY
		 *	
		 *	\note		
		 */
		HRESULT AppendToDirectory(const CNxNString& sNewPart);

		/*!	\fn			CNxNString GetName() const
		 *	
		 *				Returns either the filename with extension or last directory name with extension.
		 *	
		 *	\param		none
		 *	
		 *	\return		file- or directoryname with extension
		 *	
		 *	\note		
		 */
		CNxNString GetName() const;					// returns filename with extension or last directory name with extension

		/*!	\fn			HRESULT SetName(const CNxNString& sName)
		 *	
		 *				Replaces either the filename with extension or last directory name with extension.
		 *	
		 *	\param		none
		 *	
		 *	\return		S_OK or E_OUTOFMEMORY
		 *	
		 *	\note		
		 */
		HRESULT SetName(const CNxNString& sName);

		/*!	\fn			CNxNString GetTitle() const
		 *	
		 *				Returns either filename without extension or last directory name without extension.
		 *	
		 *	\param		none
		 *	
		 *	\return		file- or directoryname without extension
		 *	 
		 *		<dl compact><dt><b>Example:</b>
		 *		<dd>
		 *		\code
		 *			"\\Dir\\File"         ->   "File"
		 *			"\\Dir\\File.txt"     ->   "File"
		 *			"\\Dir\\File.txt.a"   ->   "File.txt"
		 *			"File.txt"            ->   "File"
		 *			".txt"                ->   ""
		 *			"\\Dir\\"             ->   "Dir"
		 *			"\\a\\Dir\\"          ->   "Dir"
		 *			"\\a\\Dir.ext\\"      ->   "Dir"
		 *			"\\a\\Dir.ext.\\"     ->   "Dir.ext"
		 *		\endcode
		 *		</dl>
		 *
		 *	\note		
		 */
		CNxNString GetTitle() const;					

		/*!	\fn			CNxNString GetExtension(bool bWithDot = false) const
		 *	
		 *				Returns the extension of the filename or last directory.
		 *	
		 *	\param		bWithDot true, when the return string should include a dot
		 *	
		 *	\return		extension of last filename or directory, if any
		 *	
		 *	\note		
		 */
		CNxNString GetExtension(bool bWithDot = false) const;	// returns extension with or without dot

		/*!	\fn			HRESULT SetExtension(const CNxNString& sExt)
		 *	
		 *				Replaces the extension of the filename or last directory.
		 *	
		 *	\param		none
		 *	
		 *	\return		S_OK or E_OUTOFMEMORY
		 *	
		 *	\note		
		 */
		HRESULT SetExtension(const CNxNString& sExt);	// assume it is a file and adds/replaces extension

		/*!	\fn			CNxNString GetNamespacePart(int xPart, bool bWithPreceedingSlash) const
		 *	
		 *				Returns the x-th element of the directory part.
		 *	
		 *	\param		true, when the return string should include a preceeding slash
		 *	
		 *	\return		the x-th element of the directory part or an empty string
		 *	
		 *	\note		
		 */
		CNxNString GetNamespacePart(int xPart, bool bWithPreceedingSlash) const;

		/*!	\fn			template<class STLContainer> STLContainer::size_type Split(STLContainer& PartList, const CNxNString& Separators = L"\\/") const
		 *	
		 *				
		 *	
		 *	\param		none
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		template<class STLContainer> STLContainer::size_type Split(STLContainer& PartList, const CNxNString& Separators = L"\\/") const
		{
			return CNxNString::Split(PartList,Separators);
		}

		/*!	\fn			inline bool HasTrailingSlash (bool bCountSingleSlash=true) const
		 *	
		 *				Checks for trailing slash
		 *	
		 *	\param		none
		 *	
		 *	\return		true, when the string has a trailing slash
		 *	
		 *	\note		
		 */
		inline bool HasTrailingSlash (bool bCountSingleSlash=true) const; // true if path has trailing forward or backward slash

		/*!	\fn			inline bool HasLeadingSlash	(bool bCountSingleSlash=true) const
		 *	
		 *				Checks for leading slash
		 *	
		 *	\param		none
		 *	
		 *	\return		true, when the string has a leading slash
		 *	
		 *	\note		
		 */
		inline bool HasLeadingSlash	(bool bCountSingleSlash=true) const; // true if path has trailing forward or backward slash

		/*!	\fn			inline HRESULT AddLeadingSlash	(bool bBackSlash = true)
		 *	
		 *				Places a slash before the string if there is none.
		 *	
		 *	\param		none
		 *	
		 *	\return		S_OK or E_OUTOFMEMORY
		 *	
		 *	\note		
		 */
		inline HRESULT AddLeadingSlash	(bool bBackSlash = true);

		/*!	\fn			inline void RemoveLeadingSlash	(bool bKeepSingleSlash = true)
		 *	
		 *				Removes the leading slash, if any.
		 *	
		 *	\param		none
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		inline void RemoveLeadingSlash	(bool bKeepSingleSlash = true); // removes trailing slash

		/*!	\fn			inline HRESULT AddTrailingSlash	(bool bBackSlash = true)
		 *	
		 *				Places a slash after the string if there is none.
		 *	
		 *	\param		none
		 *	
		 *	\return		S_OK or E_OUTOFMEMORY
		 *	
		 *	\note		
		 */
		inline HRESULT AddTrailingSlash	(bool bBackSlash = true);

		/*!	\fn			inline void RemoveTrailingSlash	(bool bKeepSingleSlash = true)
		 *	
		 *				Removes the trailing slash, if any.
		 *	
		 *	\param		none
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		inline void RemoveTrailingSlash	(bool bKeepSingleSlash = true); // removes trailing slash

		/*!	\fn			void StandardizeAsFile()
		 *	
		 *				Removes trailing slash and makes all slashes backslashes.
		 *	
		 *	\param		none
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		void StandardizeAsFile();		

		/*!	\fn			void StandardizeAsFolder()
		 *	
		 *				Adds trailing slash and makes all slashes backslashes.
		 *	
		 *	\param		none
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		void StandardizeAsFolder();		

		/*!	\fn			void StandardizeForNamespace()
		 *	
		 *				Adds leading slash, removes trailing slash and makes all slashes backslashes.
		 *	
		 *	\param		none
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		void StandardizeForNamespace();	

		/*!	\fn			bool IsAbsolute () const
		 *	
		 *				Checks whether the path represented by this object appears to be absolute or relative. 
		 *				It is assumed to be absolute if it starts with a slash or if it contains a colon in the	first three letters.
		 *				It returns false for empty strings.
		 *	
		 *	\param		none
		 *	
		 *	\return		true, when the one of the conditions from above are fullfilled
		 *	
		 *	\note		
		 */
		bool IsAbsolute () const;

		/*!	\fn			inline bool IsRelative () const
		 *	
		 *				Checks whether the path represented by this object appears to be absolute or relative. 
		 *				It is assumed to be absolute if it does not start with a slash or if it does not contain a colon in the first three letters.
		 *				It returns true for empty strings.
		 *	
		 *	\param		none
		 *	
		 *	\return		true, when the one of the conditions from above are fullfilled
		 *	
		 *	\note		
		 */
		inline bool IsRelative () const;

		/*!	\fn			inline bool IsDots() const
		 *	
		 *				Checks for dot only strings
		 *	
		 *	\param		none
		 *	
		 *	\return		true, when string equals "." or ".."
		 *	
		 *	\note		
		 */
		inline bool IsDots() const;

		/*!	\fn			inline bool IsURL() const
		 *	
		 *				Checks for a protocol component in the path.
		 *	
		 *	\param		none
		 *	
		 *	\return		true, when the path contains a protocol
		 *	
		 *	\note		
		 */
		inline bool IsURL() const;

		/*!	\fn			inline static bool IsSlash(wchar_t c)
		 *	
		 *				Checks the character for being either a backslash or a forward slash.
		 *	
		 *	\param		none
		 *	
		 *	\return		true, when the character is a backslash or a forward slash
		 *	
		 *	\note		
		 */
		inline static bool NXNINTEGRATORSDK_API_CALL IsSlash(wchar_t c);

		/*!	\fn			int	MakeForwardSlashes()
		 *	
		 *				Changes all backward slashes into forward slashes
		 *	
		 *	\param		none
		 *	
		 *	\return		number of replaced backward slashes
		 *	
		 *	\note		
		 */
		int	MakeForwardSlashes();

		/*!	\fn			int	MakeBackwardSlashes()
		 *	
		 *				Changes all forward slashes into backward slashes
		 *	
		 *	\param		none
		 *	
		 *	\return		number of replaced forward slashes
		 *	
		 *	\note		
		 */
		int	MakeBackwardSlashes();

		// compare method used only in map compare methods, they have nothing to do with alphabetical order
		bool IsLess(const CNxNPath&) const;  

		/*!	\fn			bool IsSame(const CNxNPath& Path, int MaxCompareLength=0, bool IgnoreLeadingSlash=true, bool IgnoreTrailingSlash=true) const
		 *	
		 *				case, slash and termination insensitive compare method
		 *	
		 *	\param		Path the Path to test against *this*.
		 *	\param		MaxCompareLength specifiy number of characteres, if you want only a part of the string compared
		 *	\param		IgnoreLeadingSlash  ignore existing or nonexisting leading slash of both strings
		 *	\param		IgnoreTrailingSlash ignore existing or nonexisting trailing slash of both strings
		 *
		 *	\return		true when paths are no different, ignoring case, different slash styles, trailing slash and leading slash
		 *	
		 *	\note		
		 */
		bool IsSame(const CNxNPath& Path, int MaxCompareLength=0, bool IgnoreLeadingSlash=true, bool IgnoreTrailingSlash=true) const;  
		
		/*!	\fn			CNxNString GetShortName(int MaxLength) const
		 *	
		 *				Get a short version of the path for displaying
		 *	
		 *	\param		MaxLength the maximum length of the returned string
		 *	
		 *	\return		the short string
		 *	
		 *	\note		
		 */
		CNxNString GetShortName(int MaxLength) const;

		//-----------------------------------------------------------------------
		// Hierarchy information
		//-----------------------------------------------------------------------
		/*!	\fn			CNxNPath GetParent (bool WithTerminatingSlash=true) const
		 *	
		 *				Returns the whole path minus the last part, when the last part is a filename or a folder.
		 *	
		 *	\param		none
		 *	
		 *	\return		the parent folder of the current path
		 *	
		 *	\note		
		 */
		CNxNPath GetParent (bool WithTerminatingSlash=true) const;

		/*!	\fn			bool IsAbove (const CNxNPath&) const
		 *	
		 *				Analyzes the parent/child relationship between different paths.
		 *	
		 *	\param		Path the Path to test against *this*.
		 *	
		 *	\return		true, when the path is a parent of the given path
		 *
		   				<dl compact><dt><b>Example:</b>
							<dd>
							\code
								"/A/B".IsAbove("/A/B/C/D") returns true;
							\endcode
						</dl>
						<dl compact><dt><b>Note:</b>
							<dd>The method is
							<ul>
								<li>case insensitive:			'A' == 'a'
								<li>slash insensitive:			'\' == FSLASH
								<li>termination insensitive:    'a:\b\ == a:\b'
							</ul>
						</dl>		
		 *	
		 *	\note		
		 */
		bool IsAbove (const CNxNPath&) const;

		/*!	\fn			bool IsBelow (const CNxNPath&) const
		 *	
		 *				Analyzes the parent/child relationship between different paths.
		 *	
		 *	\param		Path the Path to test against *this*.
		 *	
		 *	\return		true, when the path is a child of the given path
		 *
						<dl compact><dt><b>Example:</b>
							<dd>
							\code
								"/A/B".IsBelow("/A/B/C/D") returns false;
							\endcode
						</dl>
						<dl compact><dt><b>Note:</b>
							<dd>The method is
							<ul>
								<li>case insensitive:			'A' == 'a'
								<li>slash insensitive:			'\' == FSLASH
								<li>termination insensitive:    'a:\b\' == a:\b'
							</ul>
						</dl>		
		 *	
		 *	\note		
		 */
		bool IsBelow (const CNxNPath&) const;

		/*!	\fn			inline bool IsBelowOrSame (const CNxNPath&) const
		 *	
		 *				Short for IsBelow() OR'ed with IsSame().
		 *	
		 *	\param		none
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		inline bool IsBelowOrSame (const CNxNPath&) const;

		/*!	\fn			inline bool IsAboveOrSame (const CNxNPath&) const
		 *	
		 *				Short for IsAbove() OR'ed with IsSame().
		 *	
		 *	\param		none
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		inline bool IsAboveOrSame (const CNxNPath&) const;

		/*!	\fn			int CountComponents() const
		 *	
		 *				Counts the number of parts, the path consits of, that is, the number of strings, that
		 *				are separated by slashes.
		 *	
		 *	\param		none
		 *	
		 *	\return		the number of parts 
		 *	
		 *	\note		
		 */
		int CountComponents() const;

		/*!	\fn			HRESULT MakeAbsolute()
		 *	
		 *				Call this function to convert an absolute path that contains path traversal tokens like '.' and '..' 
		 *				into a cleaned up absolute version. 
		 *	
		 *	\param		none
		 *	
		 *	\return		S_OK or E_OUTOFMEMORY
		 *
  						<dl compact><dt><b>Example:</b>
							<dd>
							\code
								"C:\temp\abs\..\d.bmp" becomes "C:\temp\d.bmp"
							\endcode
						</dl>
		 *	
		 *	\note		
		 */
		HRESULT MakeAbsolute();

		//-----------------------------------------------------------------------
		//	Disk file related methods
		//-----------------------------------------------------------------------
		/*!	\fn			CNxNString GetShortPathName()
		 *	
		 *				Call this function to get the Windows short path name of the path stored in this object.
		 *	
		 *	\param		none
		 *	
		 *	\return		The Windows short path name version of the path.
		 *	
		 *	\note		
		 */
		CNxNString GetShortPathName();

		//-----------------------------------------------------------------------
		// Low-level element information
		//-----------------------------------------------------------------------

		typedef std::pair<ePointer,ePointer> TPtrPair;
		typedef TPtrPair TElement;
		//! Protocol 
		static const TPtrPair PROTOCOL;			// P_PROTOCOL,	P_PROTOCOL_COLON+1 
		static const TPtrPair PROTOCOL_NAME;	// P_PROTOCOL,	P_PROTOCOL+1
		static const TPtrPair COMPUTERNAME;		// P_UNC_SLASH,	P_HOST_SLASH+1
		static const TPtrPair UNC_COMPUTERNAME;	// P_HOST,		P_HOST+1
		static const TPtrPair DRIVE;			// P_DRIVE,		P_DRIVE_COLON+1
		static const TPtrPair DRIVE_NAME;		// P_DRIVE,		P_DRIVE+1
		static const TPtrPair DIRECTORY;		// P_DRIVE		P_DIR_SLASH2+1
		static const TPtrPair DIRECTORY_NAME;	// P_DIR_SLASH1	P_DIR_SLASH2+1
		static const TPtrPair FILENAME;			// P_FILETITILE	P_EXTENSION+1
		static const TPtrPair FILETITLE;		// P_FILETITLE	P_FILETITLE+1
		static const TPtrPair EXTENSION;		// P_EXT_DOT	P_EXTENSION+1
		static const TPtrPair EXTENSION_NAME;	// P_EXTENSION	P_EXTENSION+1
		static const TPtrPair DISK_PATH;		// P_DRIVE		P_NULL
		static const TPtrPair NAMESPACE_PATH;	// P_DIR_SLASH1 P_NULL
		static const TPtrPair UNC_PATH;			// P_UNC_SLASH	P_NULL
		static const TPtrPair URL_PATH;			// P_PROTOCOL	P_NULL

		HRESULT Scan();

		CNxNPath Get	(TElement) const;
		CNxNPath Get	(ePointer x1, ePointer x2) const;
		HRESULT Delete	(TElement);
		HRESULT Replace	(TElement, const CNxNString&);
		bool IsA		(TElement) const;
		bool IsValid	(TElement) const;
		bool HasA		(TElement) const;
		bool HasA		(ePointer) const;

		HRESULT ShortenTo (TElement element);

	private:
		// substring pointers 
		TElement m_Elements;
		LPWSTR m_Pointers[P_NUM];
		HRESULT m_ScanResult;

		// notifications about string changes
		inline virtual void _Changed()											{m_ScanResult = S_FALSE;};
		inline virtual void _Replaced(LPCWSTR fromPos=NULL, LPCWSTR toPos=NULL) {m_ScanResult = S_FALSE; fromPos=NULL; toPos=NULL; };
		inline virtual void _Deleted (LPCWSTR fromPos=NULL, LPCWSTR toPos=NULL)	{m_ScanResult = S_FALSE; fromPos=NULL; toPos=NULL; };
		inline virtual void _Appended(LPCWSTR fromPos, size_type nChars)		{m_ScanResult = S_FALSE; fromPos=NULL; nChars = 0; };
		
		void Init()
		{
			memset(&m_Pointers[0],0,sizeof(m_Pointers));
			m_ScanResult = S_FALSE; // not yet scanned
		}

		// string scanning
		static HRESULT NXNINTEGRATORSDK_API_CALL Scan (LPWSTR p, LPWSTR *pPointers);
		inline HRESULT ScanOnDemand() { return m_ScanResult == S_OK ? S_OK : Scan(); }

		HRESULT GetElementPointer (TElement Pointers, LPWSTR& rpBegin, LPWSTR& rpEnd);

	public:
		// predicate structure used as compare method for stl or nxn container classes like map, CNxNStringMap
		// with CNxNPath as key
		struct PredLess : std::binary_function<CNxNPath, CNxNPath, bool>
		{
			inline bool operator() (const CNxNPath& s1, const CNxNPath& s2) const
			{
				return s1.IsLess(s2);
			}
		};
		struct PredCollateNoCaseLess : std::binary_function<CNxNPath, CNxNPath, bool>
		{
			inline bool operator() (const CNxNPath& s1, const CNxNPath& s2) const
			{
				return s1.CollateNoCase(s2)<0;
			}
		};
};

extern NXNINTEGRATORSDK_API CNxNPath NULLPATH; 


inline CNxNString CNxNPath::GetComputerName() const
{
	if ( !HasA( COMPUTERNAME ) ) return L"";
	return Get( COMPUTERNAME );
}

inline CNxNString CNxNPath::GetProtocol() const
{
	if ( !HasA( PROTOCOL ) ) return L"";
	return Get( PROTOCOL );
}



inline bool CNxNPath::IsRelative () const 
{ 
	return !IsAbsolute(); 
}
inline bool CNxNPath::IsURL() const 
{ 
	return HasA(PROTOCOL); 
}
inline bool CNxNPath::IsSlash(wchar_t c) 
{ 
	return c==BSLASH || c==FSLASH; 
}

inline bool CNxNPath::IsDots() const
{ 
	size_type n = GetLength();
	if (n!=1 && n!=2) return false;

	BSTR p=GetBSTR();
	BSTR p1=p+1;
	return ( (*p == L'.' && n==1) || (*p1 == L'.') );
}

inline bool CNxNPath::IsBelowOrSame (const CNxNPath& Path2) const
{
	return IsSame( Path2 ) || IsBelow( Path2 );
}

inline bool CNxNPath::IsAboveOrSame (const CNxNPath& Path2) const
{
	return IsSame( Path2 ) || IsAbove( Path2 );
}

//-----------------------------------------------------------------------
//	validation
//-----------------------------------------------------------------------
//
inline bool CNxNPath::IsValidFilename(const CNxNString& s, bool bCanBeEmpty)	
{ 
	return (bCanBeEmpty || !s.IsEmpty()) && s.FindOneOf(L"\\/:*?<>|")==-1; 
}
inline bool CNxNPath::IsValidDrivename(const CNxNString& s)	
{ 
	return s.GetLength()==1 && towlower(s[0]) >= L'a' && towlower(s[0]) <= L'z'; 
}


//-----------------------------------------------------------------------
//	cast operators
//-----------------------------------------------------------------------
//

inline CNxNPath::operator LPCTSTR () const 
{ 
	return CNxNString::operator LPCTSTR(); 
}
inline CNxNPath::operator LPCWSTR () const 
{ 
	return GetBSTR(); 
}
inline CNxNPath& CNxNPath::operator= ( const VARIANT& Src ) 
{ 
	Copy(Src); 
	return *this; 
}



//-----------------------------------------------------------------------
// leading and traling slashes
//-----------------------------------------------------------------------
//
inline bool CNxNPath::HasTrailingSlash (bool bCountSingleSlash) const // true if path has trailing forward or backward slash
{
	if (IsEmpty()) return false;
	int nLen = GetLength();
	wchar_t c = (*this)[nLen-1];
	return IsSlash(c) && (nLen > 1 || bCountSingleSlash); 
}
inline bool CNxNPath::HasLeadingSlash (bool bCountSingleSlash) const // true if path has trailing forward or backward slash
{
	if (IsEmpty()) return false;
	int nLen = GetLength();
	wchar_t c = *GetBSTR();
	return IsSlash(c) && (nLen > 1 || bCountSingleSlash); 
}
inline HRESULT CNxNPath::AddLeadingSlash(bool bBackSlash)
{
	if (HasLeadingSlash()) return S_OK;
	Insert(0, bBackSlash ? BSLASH : FSLASH);
	return S_OK;
}
inline void CNxNPath::RemoveLeadingSlash(bool bKeepSingleSlash) // removes trailing slash
{
	if (!HasLeadingSlash()) return;
	if (GetLength() > 1 || !bKeepSingleSlash) // do not delete single slash
		CNxNString::Delete(0);
}
inline HRESULT CNxNPath::AddTrailingSlash(bool bBackSlash)
{
	if (HasTrailingSlash()) return S_OK;
	*this += bBackSlash ? BSLASH : FSLASH;
	return S_OK;
}
inline void CNxNPath::RemoveTrailingSlash(bool bKeepSingleSlash) // removes trailing slash
{
	if (!HasTrailingSlash()) return;
	if (GetLength() > 1 || !bKeepSingleSlash) // do not delete single slash
		CNxNString::Delete(GetLength()-1);
}



#endif // INC_NXN_PATH_H_