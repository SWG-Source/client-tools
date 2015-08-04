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

		/*!	\fn			CNxNPath::CNxNPath()
		 *	
		 *				Default constructor.
		 *	
		 *	\param		none
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		CNxNPath() : CNxNString()												{ Init(); }
		
		/*!	\fn			CNxNPath::CNxNPath(const CNxNPath& src)
		 *	
		 *				Copy constructor.
		 *	
		 *	\param		src     path to be copied
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		CNxNPath(const CNxNPath& src)			: CNxNString( (const CNxNString&) src ) { Init(); }

		/*!	\fn			CNxNPath::CNxNPath(const CNxNString& src)
		 *	
		 *				Copy constructor.
		 *	
		 *	\param		src     string containing the initial path
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		CNxNPath(const CNxNString& src)			: CNxNString( src )			{ Init(); }

		/*!	\fn			CNxNPath::CNxNPath(LPCSTR psz)
		 *	
		 *				Constructor from a '\0' terminated ANSI character string.
		 *	
		 *	\param		psz     pointer to a string with terminating '\0' that contains the initial path
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		CNxNPath(LPCSTR psz)					: CNxNString( psz )			{ Init(); }

		/*!	\fn			CNxNPath::CNxNPath(LPCSTR psz, size_type nChar)
		 *	
		 *				Constructor from a ANSI character string and length.
		 *	
		 *	\param		psz     pointer to a string containing the initial path
		 *	\param		nChar   number of characters to copy
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		CNxNPath(LPCSTR psz, size_type nChar)	: CNxNString( psz, nChar )	{ Init(); }

		/*!	\fn			CNxNPath::CNxNPath(LPCWSTR psz)
		 *	
		 *				Constructor from a wide character string.
		 *	
		 *	\param		psz     wide character string with terminating '\0' containing the initial path
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		CNxNPath(LPCWSTR psz)					: CNxNString( psz )			{ Init(); }

		/*!	\fn			CNxNPath::CNxNPath(LPCWSTR psz, bool bIsBSTR)
		 *	
		 *				Constructor from a '\0' terminated wide character string or a real BSTR
         *              with flag signalling the type of the parameter string.
		 *	
		 *	\param		psz     wide character string with terminating '\0' or BSTR
		 *	\param		bIsBSTR flag indicating whether the given string is a BSTR
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		CNxNPath(LPCWSTR psz, bool IsBSTR)		: CNxNString( psz, IsBSTR )	{ Init(); }

		/*!	\fn			CNxNPath::CNxNPath(LPCWSTR psz, size_type nChar)
		 *	
		 *				Constructor from a wide character string and string length.
		 *	
		 *	\param		psz     wide character string containing the initial path
		 *	\param		nChar   number of characters to copy from psz
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		CNxNPath(LPCWSTR psz, size_type nChar)	: CNxNString( psz, nChar )	{ Init(); }
	#ifdef _MFC_VER

		/*!	\fn			CNxNPath::CNxNPath(const CString& src)
		 *	
		 *				Constructor from a CString object.
		 *	
		 *	\param		src     CString object containing the initial path
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
		/*!	\fn			inline CNxNPath::operator LPCSTR () const
		 *	
		 *				Cast operator to a temporary ANSI character string.
		 *	
		 *	\param		none
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
        // \param was:  pointer to temporary buffer, holding an ANDI string version of path.
		inline operator LPCSTR () const;

		/*!	\fn			inline CNxNPath::operator LPCWSTR () const
		 *	
		 *				Cast operator to a constant wide character string.
		 *	
		 *	\param		none
		 *	
		 *	\return		internal BSTR
		 *	
		 *	\note		
		 */
		inline operator LPCWSTR () const;

		/*!	\fn			inline CNxNPath& CNxNPath::operator = (const VARIANT& Src)
		 *	
		 *				Assignment operator for variants.
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

		/*!	\fn			inline static bool CNxNPath::IsValidFilename(const CNxNString& sName, bool bCanBeEmpty = false)
		 *	
		 *				Checks the given string for invalid filename characters like "\\/:*?<>|"
		 *	
		 *	\param		sName       string containing the filename to test
		 *	\param		bCanBeEmpty flag that indicates whether an empty string should be considered as valid
		 *	
		 *	\return		boolean value indicating whether the given filename is valid
		 *	
		 *	\note		
		 */
		inline static bool NXNINTEGRATORSDK_API_CALL IsValidFilename(const CNxNString& sName, bool bCanBeEmpty = false);

		/*!	\fn			inline static bool CNxNPath::IsValidDrivename(const CNxNString& sName)
		 *	
		 *				Checks the given string for invalid drivename characters 
		 *				(different from 'a'-'z')
		 *	
		 *	\param		sName   string containing the drivename to test
		 *	
		 *	\return		boolean value. If true, the string has only 
		 *				one character between 'a' and 'z' (upper or lowercase)
		 *	
		 *	\note		
		 */
		inline static bool NXNINTEGRATORSDK_API_CALL IsValidDrivename(const CNxNString& sName);

		//-----------------------------------------------------------------------
		// High-level element information
		//-----------------------------------------------------------------------
		/*!	\fn			inline CNxNString CNxNPath::GetProtocol() const
		 *	
		 *				Returns the protocol part of the shell path, if any.
		 *	
		 *	\param		none
		 *	
		 *	\return		string containing the protocol
		 *	
		 *	\note		
		 */
		inline CNxNString GetProtocol() const;

		/*!	\fn			inline CNxNString CNxNPath::GetComputerName() const
		 *	
		 *				Returns the computer name part of the shell path, if any.
		 *	
		 *	\param		none
		 *	
		 *	\return		string containing the computer name.
		 *	
		 *	\note		
		 */
		inline CNxNString GetComputerName() const;

		/*!	\fn			CNxNPath CNxNPath::GetShellPath(bool bWithSlash = true) const
		 *	
		 *				Returns the complete path without the filename.
		 *	
		 *	\param		none
		 *	
		 *	\return		path object containing the complete path without filename.
		 *	
		 *	\note		
		 */
		CNxNPath GetShellPath(bool bWithSlash = true) const;		

		/*!	\fn			CNxNPath CNxNPath::GetShellDirectory(bool bWithSlash = true) const
		 *	
		 *				Returns only the directory part of the path, without UNC-part, drive or filename part.
		 *	
		 *	\param		none
		 *	
		 *	\return		path object containing the directory part.
		 *	
		 *	\note		
		 */
		CNxNPath GetShellDirectory(bool bWithSlash = true) const;

		/*!	\fn			HRESULT CNxNPath::AppendToDirectory(const CNxNString& sNewPart)
		 *	
		 *				Appends the given string at the end of itself and 
		 *				removes any double slashes.
		 *	
		 *	\param		none
		 *	
		 *	\return		S_OK for success or E_OUTOFMEMORY in case of an error
		 *	
		 *	\note		
		 */
		HRESULT AppendToDirectory(const CNxNString& sNewPart);

		/*!	\fn			CNxNString CNxNPath::GetName() const
		 *	
		 *				Returns either the filename with extension or the last directory name 
		 *				with extension.
		 *	
		 *	\param		none
		 *	
		 *	\return		string containing the filename or directory name with extension
		 *	
		 *	\note		
		 */
		CNxNString GetName() const;					// returns filename with extension or last directory name with extension

		/*!	\fn			HRESULT CNxNPath::SetName(const CNxNString& sName)
		 *	
		 *				Replaces either the filename with extension or the last directory 
		 *				name with extension.
		 *	
		 *	\param		none
		 *	
		 *	\return		S_OK for success or E_OUTOFMEMORY in case of an error
		 *	
		 *	\note		
		 */
		HRESULT SetName(const CNxNString& sName);

		/*!	\fn			CNxNString CNxNPath::GetTitle() const
		 *	
		 *				Returns either the filename without extension or 
		 *				the last directory name without extension.
		 *	
		 *	\param		none
		 *	
		 *	\return		string containing the filename or directory name without extension
		 *	 
		 *		<br><b>Example:</b><br>
		 		\code
		 			"\\Dir\\File"         ->   "File"
		 			"\\Dir\\File.txt"     ->   "File"
		 			"\\Dir\\File.txt.a"   ->   "File.txt"
		 			"File.txt"            ->   "File"
		 			".txt"                ->   ""
		 			"\\Dir\\"             ->   "Dir"
		 			"\\a\\Dir\\"          ->   "Dir"
		 			"\\a\\Dir.ext\\"      ->   "Dir"
		 			"\\a\\Dir.ext.\\"     ->   "Dir.ext"
		 		\endcode
		 *
		 *	\note		
		 */
		CNxNString GetTitle() const;					

		/*!	\fn			CNxNString CNxNPath::GetExtension(bool bWithDot = false) const
		 *	
		 *				Returns the extension of the filename or the last directory.
		 *	
		 *	\param		bWithDot    flag that indicates whether the returned string should include a dot
		 *	
		 *	\return		string containing the extension of the last filename or directory, if any.
		 *	
		 *	\note		
		 */
		CNxNString GetExtension(bool bWithDot = false) const;	// returns extension with or without dot

		/*!	\fn			HRESULT CNxNPath::SetExtension(const CNxNString& sExt)
		 *	
		 *				Replaces the extension of the filename or the last directory.
		 *	
		 *	\param		none
		 *	
		 *	\return		S_OK for success or E_OUTOFMEMORY in case of an error
		 *	
		 *	\note		
		 */
		HRESULT SetExtension(const CNxNString& sExt);	// assume it is a file and adds/replaces extension

		/*!	\fn			CNxNString CNxNPath::GetNamespacePart(int iPart, bool bWithPreceedingSlash) const
		 *	
		 *				Returns the iPart-th element of the directory part.
		 *	
		 *	\param		iPart                  integer containing the part number to extract
         *  \param      bWithPreceedingSlash   flag that indicates whether the returned string should include a preceeding slash
		 *	
		 *	\return		string containing the iPart-th element of the directory part or an empty string
		 *	
		 *	\note		
		 */
		CNxNString GetNamespacePart(int iPart, bool bWithPreceedingSlash) const;

		/*!	\fn			template<class STLContainer> STLContainer::size_type Split(STLContainer& PartList, const CNxNString& sSeparators = L"\\/") const
		 *	
		 *				Splits the path into its parts and puts them into the given STL container. The container is not cleared.
		 *	
		 *	\param		PartList        STL-container object to put the parts into
         *  \param      sSeparators     string containing the characters used as separators (e.g. "/\")
		 *	
		 *	\return		number of parts added to the container
		 *	
		 *	\note		
		 */
		template<class STLContainer> STLContainer::size_type Split(STLContainer& PartList, const CNxNString& Separators = L"\\/") const
		{
			return CNxNString::Split(PartList, Separators);
		}

		/*!	\fn			inline bool CNxNPath::HasTrailingSlash(bool bCountSingleSlash = true) const
		 *	
		 *				Checks for a trailing slash.
		 *	
		 *	\param		none
		 *	
		 *	\return		boolean value indicating if the string has a trailing slash
		 *	
		 *	\note		
		 */
		inline bool HasTrailingSlash (bool bCountSingleSlash=true) const; // true if path has trailing forward or backward slash

		/*!	\fn			inline bool CNxNPath::HasLeadingSlash	(bool bCountSingleSlash=true) const
		 *	
		 *				Checks for a leading slash.
		 *	
		 *	\param		none
		 *	
		 *	\return		boolean value indicating if the string has a leading slash
		 *	
		 *	\note		
		 */
		inline bool HasLeadingSlash	(bool bCountSingleSlash = true) const; // true if path has trailing forward or backward slash

		/*!	\fn			inline HRESULT CNxNPath::AddLeadingSlash(bool bBackSlash = true)
		 *	
		 *				Places a leading slash in front of the string if there is currently none.
		 *	
		 *	\param		none
		 *	
		 *	\return		S_OK for success or E_OUTOFMEMORY in case of an error
		 *	
		 *	\note		
		 */
		inline HRESULT AddLeadingSlash(bool bBackSlash = true);

		/*!	\fn			inline void CNxNPath::RemoveLeadingSlash(bool bKeepSingleSlash = true)
		 *	
		 *				Removes the leading slash, if any.
		 *	
		 *	\param		none
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		inline void RemoveLeadingSlash(bool bKeepSingleSlash = true); // removes trailing slash

		/*!	\fn			inline HRESULT CNxNPath::AddTrailingSlash	(bool bBackSlash = true)
		 *	
		 *				Places a trailing slash behind the string if there is currently none.
		 *	
		 *	\param		none
		 *	
		 *	\return		S_OK for success or E_OUTOFMEMORY in case of an error
		 *	
		 *	\note		
		 */
		inline HRESULT AddTrailingSlash(bool bBackSlash = true);

		/*!	\fn			inline void CNxNPath::RemoveTrailingSlash(bool bKeepSingleSlash = true)
		 *	
		 *				Removes the trailing slash, if any.
		 *	
		 *	\param		none
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		inline void RemoveTrailingSlash(bool bKeepSingleSlash = true); // removes trailing slash

		/*!	\fn			void CNxNPath::StandardizeAsFile()
		 *	
		 *				Removes a trailing slash and makes all forward slashes backslashes.
		 *	
		 *	\param		none
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		void StandardizeAsFile();		

		/*!	\fn			void CNxNPath::StandardizeAsFolder()
		 *	
		 *				Adds a trailing slash and makes all forward slashes backslashes.
		 *	
		 *	\param		none
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		void StandardizeAsFolder();

		/*!	\fn			void CNxNPath::StandardizeForNamespace()
		 *	
		 *				Adds a leading slash, removes a trailing slash and makes all forward slashes backslashes.
		 *	
		 *	\param		none
		 *	
		 *	\return		none
		 *	
		 *	\note		
		 */
		void StandardizeForNamespace();	

		/*!	\fn			bool CNxNPath::IsAbsolute() const
		 *	
		 *				Checks whether the path represented by this object appears to be absolute or relative. 
		 *				It is assumed to be absolute if it starts with a slash or if it contains a colon in the	first three letters.
		 *				It returns false for empty strings.
		 *	
		 *	\param		none
		 *	
		 *	\return		boolean value indicating if the one of the conditions from above are fulfilled
		 *	
		 *	\note		
		 */
		bool IsAbsolute () const;

		/*!	\fn			inline bool CNxNPath::IsRelative() const
		 *	
		 *				Checks whether the path represented by this object appears to be absolute or relative. 
		 *				It is assumed to be relative if it does not start with a slash or if it does not contain a colon in the first three letters.
		 *				It returns true for empty strings.
		 *	
		 *	\param		none
		 *	
		 *	\return		boolean value indicating if the one of the conditions from above are fulfilled
		 *	
		 *	\note		
		 */
		inline bool IsRelative() const;

		/*!	\fn			inline bool CNxNPath::IsDots() const
		 *	
		 *				Checks for dot-only strings.
		 *	
		 *	\param		none
		 *	
		 *	\return		boolean value indicating if the string equals "." or ".."
		 *	
		 *	\note		
		 */
		inline bool IsDots() const;

		/*!	\fn			inline bool CNxNPath::IsURL() const
		 *	
		 *				Checks for a protocol component in the path.
		 *	
		 *	\param		none
		 *	
		 *	\return		boolean value indicating if the path contains a protocol component
		 *	
		 *	\note		
		 */
		inline bool IsURL() const;

		/*!	\fn			inline static bool CNxNPath::IsSlash(wchar_t c)
		 *	
		 *				Checks the character for being either a backslash or a forward slash.
		 *	
		 *	\param		none
		 *	
		 *	\return		boolean value indicating if the character is a backslash or a forward slash
		 *	
		 *	\note		
		 */
		inline static bool NXNINTEGRATORSDK_API_CALL IsSlash(wchar_t c);

		/*!	\fn			int	CNxNPath::MakeForwardSlashes()
		 *	
		 *				Changes all backward slashes into forward slashes.
		 *	
		 *	\param		none
		 *	
		 *	\return		number of replaced backward slashes
		 *	
		 *	\note		
		 */
		int	MakeForwardSlashes();

		/*!	\fn			int	CNxNPath::MakeBackwardSlashes()
		 *	
		 *				Changes all forward slashes into backward slashes.
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

		/*!	\fn			bool CNxNPath::IsSame(const CNxNPath& Path, int iMaxCompareLength = 0, bool bIgnoreLeadingSlash = true, bool bIgnoreTrailingSlash = true) const
		 *	
		 *				Case-, slash- and termination-insensitive compare method.
		 *	
		 *	\param		Path                 path object containing the path to test against *this*
		 *	\param		iMaxCompareLength    integer specifying the number of characters to compare
		 *	\param		bIgnoreLeadingSlash  flag that indicates whether to ignore an existing leading slash of both strings
		 *	\param		bIgnoreTrailingSlash flag that indicates whether to ignore an existing trailing slash of both strings
		 *
		 *	\return		boolean value indicating if the paths are equal, ignoring case, different slash styles, trailing slash and leading slash.
		 *	
		 *	\note		
		 */
		bool IsSame(const CNxNPath& Path, int iMaxCompareLength = 0, bool bIgnoreLeadingSlash = true, bool bIgnoreTrailingSlash = true) const;  
		
		/*!	\fn			CNxNString CNxNPath::GetShortName(int iMaxLength) const
		 *	
		 *				Get a short version of the path for displaying.
		 *	
		 *	\param		iMaxLength  integer specifying the maximum length of the returned string
		 *	
		 *	\return		string containing the short name
		 *	
		 *	\note		
		 */
		CNxNString GetShortName(int iMaxLength) const;

        /*!	\fn			bool IsFile() const;
         *	
         *				Checks if this path name denotes an existing file.
         *	
         *	\param		none
         *	
         *	\return		boolean value indicating if this path denotes file
         *	
         *	\note		
         */
        bool IsFile() const;

        /*!	\fn			bool IsFolder() const;
         *	
         *				Checks if this path name denotes an existing folder.
         *	
         *	\param		none
         *	
         *	\return		boolean value indicating if this path denotes folder
         *	
         *	\note		
         */
        bool IsFolder() const;

		//-----------------------------------------------------------------------
		// Hierarchy information
		//-----------------------------------------------------------------------
		/*!	\fn			CNxNPath CNxNPath::GetParent(bool bWithTerminatingSlash = true) const
		 *	
		 *				Returns the whole path minus the last part, when the last part is a filename or a folder.
         *              If there is no parent path, this method returns an empty path.
		 *	
		 *	\param		bWithTerminatingSlash   flag indicating if the parent path should be returned with trailing '\'
		 *	
		 *	\return		path object containing parent folder of the current path.
         *              Returned path is empty if no parent exists, i.e. if this path denotes a file.
		 *	
		 *	\note		
		 */
		CNxNPath GetParent(bool bWithTerminatingSlash = true) const;

		/*!	\fn			bool CNxNPath::IsAbove (const CNxNPath& path) const
		 *	
		 *				Analyzes the parent/child relationship between different paths.
		 *	
		 *	\param		path    path object to test against *this*
		 *	
		 *	\return		boolean value indicating if the path is a parent of the given path
		 *
		   				<br><b>Example:</b><br>
						\code
							"/A/B".IsAbove("/A/B/C/D") -> true
						\endcode
		 *	\note		The method is
							<ul>
								<li>case insensitive:			'A' == 'a'</li>
								<li>slash insensitive:			'\' == '/'</li>
								<li>termination insensitive:    'a:\b\ == a:\b'</li>
							</ul>
		 */
		bool IsAbove(const CNxNPath&) const;

		/*!	\fn			bool CNxNPath::IsBelow(const CNxNPath& path) const
		 *	
		 *				Analyzes the parent/child relationship between different paths.
		 *	
		 *	\param		path    path object to test against *this*
		 *	
		 *	\return		boolean value indicating the path is a child of the given path
		 *
		   				<br><b>Example:</b><br>
						\code
							"/A/B".IsBelow("/A/B/C/D") -> false
						\endcode

		 *	\note		The method is
							<ul>
								<li>case insensitive:			'A' == 'a'</li>
								<li>slash insensitive:			'\' == '/'</li>
								<li>termination insensitive:    'a:\b\' == a:\b'</li>
							</ul>
		 */
		bool IsBelow(const CNxNPath& path) const;

		/*!	\fn			inline bool CNxNPath::IsBelowOrSame(const CNxNPath& path) const
		 *	
		 *				Short for IsBelow() OR'ed with IsSame().
		 *	
		 *	\param		none
		 *	
		 *	\return		boolean value indicating if the specified path is below or the same
		 *	
		 *	\note		
		 */
		inline bool IsBelowOrSame(const CNxNPath& path) const;

		/*!	\fn			inline bool CNxNPath::IsAboveOrSame(const CNxNPath&) const
		 *	
		 *				Short for IsAbove() OR'ed with IsSame().
		 *	
		 *	\param		none
		 *	
		 *	\return		boolean value indicating if the specified path is below or the same.
		 *	
		 *	\note		
		 */
		inline bool IsAboveOrSame(const CNxNPath& path) const;

		/*!	\fn			int CNxNPath::CountComponents() const
		 *	
		 *				Counts the number of parts the path consists of. 
		 *				All parts are separated by slashes.
		 *	
		 *	\param		none
		 *	
		 *	\return		integer containing the number of parts
		 *	
		 *	\note		
		 */
		int CountComponents() const;

		/*!	\fn			HRESULT CNxNPath::MakeAbsolute()
		 *	
		 *				Call this function to convert an absolute path that contains path 
		 *				traversal tokens like '.' and '..' into a cleaned up absolute equivalent. 
		 *	
		 *	\param		none
		 *	
		 *	\return		S_OK or E_OUTOFMEMORY
		 *
		   				<br><b>Example:</b><br>
						\code
							"C:\temp\abs\..\d.bmp" -> "C:\temp\d.bmp"
						\endcode
		 *	
		 *	\note		
		 */
		HRESULT MakeAbsolute();

		//-----------------------------------------------------------------------
		//	Disk file related methods
		//-----------------------------------------------------------------------
		/*!	\fn			CNxNString CNxNPath::GetShortPathName()
		 *	
		 *				Call this function to get the Windows short path name of the 
		 *				path stored in this object.
		 *	
		 *	\param		none
		 *	
		 *	\return		string containing the Windows short path name of the path
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

inline CNxNPath::operator LPCSTR () const 
{ 
	return CNxNString::operator LPCSTR(); 
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