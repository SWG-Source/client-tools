// ImportImageDialog.cpp : implementation file
//

#include "FirstUiBuilder.h"
#include "ImportImageDialog.h"
#include "ProgressDialog.h"

#include "UIImageFrame.h"
#include "UIUtils.h"

#include <commctrl.h>
#include <shlwapi.h>
#include <ddraw.h>
#include <math.h>

char *ImportImageDialog::s_compressionMethods[] = {"DXT1", "DXT2", "DXT3", "DXT4", "DXT5"};
long	ImportImageDialog::s_tileSizes[]							= {32, 64, 128, 256, 512, 1024, 2048};

/////////////////////////////////////////////////////////////////////////////
// ImportImageDialog dialog


ImportImageDialog::ImportImageDialog(CWnd* pParent /*=NULL*/)
	: CDialog(ImportImageDialog::IDD, pParent)
	,m_compressionMethodIndex(0)
	,m_tileMaxIndex(3)
	,m_tileMinIndex(1)
	,m_fileWidth(0)
	,m_fileHeight(0)
	,m_fileIsValid(false)
	,m_alphaFileIsValid(false)
	,m_fileIsBitmap(false)
	,m_useWholeImage(false)
	,m_generateFragments(false)
	,m_compressFragments(false)
{
	//{{AFX_DATA_INIT(ImportImageDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void ImportImageDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ImportImageDialog)
	DDX_Control(pDX, IDC_TILEMIN, m_tileMin);
	DDX_Control(pDX, IDC_TILEMAX, m_tileMax);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////

void ImportImageDialog::_saveBitmapToFile( HDC hdc, HBITMAP hbmp, const char *Filename )
{
	BITMAPFILEHEADER Header;
	BITMAPINFO		   bmi;
	BITMAP				   bm;
	long             datasize;

	GetObject( hbmp, sizeof( bm ), &bm );

	ZeroMemory( &bmi, sizeof( bmi ) );
	bmi.bmiHeader.biSize        =  sizeof( bmi.bmiHeader );
	bmi.bmiHeader.biWidth       =  bm.bmWidth;
	bmi.bmiHeader.biHeight      =  bm.bmHeight;
	bmi.bmiHeader.biPlanes      =  1;
	bmi.bmiHeader.biBitCount    =  24;
	bmi.bmiHeader.biCompression =  BI_RGB;	

	datasize = bm.bmWidth * bm.bmHeight * 3;

	char *mem = new char[datasize];
	GetDIBits( hdc, hbmp, 0, bm.bmHeight, mem, &bmi, DIB_RGB_COLORS );

	ZeroMemory( &Header, sizeof( Header ) );
	Header.bfType		 = 'MB';		// BM reversed due to byte ordering
	Header.bfSize		 = sizeof( Header ) + sizeof( bmi.bmiHeader ) + datasize;
	Header.bfOffBits = sizeof( Header ) + sizeof( bmi.bmiHeader );

	FILE *fp = fopen( Filename, "wb" );

	if( fp )
	{
		fwrite( &Header, sizeof( Header ), 1, fp );
		fwrite( &bmi.bmiHeader, sizeof( bmi.bmiHeader ), 1, fp );
		fwrite( mem, datasize, 1, fp );
		fclose( fp );
	}

	delete mem;
}

// =======================================================================

void ImportImageDialog::_sliceImagesToFiles(ProgressDialog *progress)
{
	if( m_fileIsValid && m_fileIsBitmap && m_useWholeImage )
	{
		HBITMAP hSourceBitmap = 0;
		HBITMAP hAlphaBitmap  = 0;
		int     CurrentFragmentNumber = 1;

		progress->SendDlgItemMessage(IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM( 1, m_fragmentRectangles.size() ) );

		hSourceBitmap = (HBITMAP)LoadImage( GetModuleHandle(0), m_sourceFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );

		if( m_alphaFileIsValid )
			hAlphaBitmap = (HBITMAP)LoadImage( GetModuleHandle(0), m_alphaFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );

		HDC ScreenDC  = ::GetDC( NULL );
		HDC SourceDC  = ::CreateCompatibleDC( NULL );
		HDC AlphaDC   = ::CreateCompatibleDC( NULL );
		HDC ScratchDC = ::CreateCompatibleDC( NULL );

		SelectObject( SourceDC, hSourceBitmap );

		if( hAlphaBitmap )
			SelectObject( AlphaDC, hAlphaBitmap );

		SelectObject( ScratchDC, GetStockObject( BLACK_BRUSH ) );
		SelectObject( ScratchDC, GetStockObject( BLACK_PEN ) );

		char *pDot = strrchr( m_sourceFile, '.' );

		if( pDot )
			*pDot = '\0';

		for( FragmentRectangleList::iterator i = m_fragmentRectangles.begin(); i != m_fragmentRectangles.end(); ++i )
		{
			HBITMAP Scratch;
			long    TileSize;

			TileSize = i->Width();

			if( TileSize < i->Height() )
				TileSize = i->Height();

			TileSize = (long)pow( 2.0, ceil( log( (double)TileSize ) / log( 2.0 ) ) );

			Scratch = CreateCompatibleBitmap( ScreenDC, TileSize, TileSize );
			
			if( Scratch )
			{
				HBITMAP OldBMP  = (HBITMAP)SelectObject( ScratchDC, Scratch );
				char    Buffer[_MAX_PATH+1];

				sprintf( Buffer, "Processing fragment %d of %d...", CurrentFragmentNumber, m_fragmentRectangles.size() );
				progress->SetDlgItemText(IDC_STATUS, Buffer);
				progress->SendDlgItemMessage(IDC_PROGRESS, PBM_SETPOS, CurrentFragmentNumber, 0 );
				
				Rectangle( ScratchDC, 0, 0, TileSize, TileSize );

				UIRect rc = *i;
				BitBlt( ScratchDC, 0, 0, i->Width(), i->Height(), SourceDC, i->left, i->top, SRCCOPY );

				sprintf( Buffer, "%s_%02d.bmp", m_sourceFile, CurrentFragmentNumber-1 );
				_saveBitmapToFile( ScratchDC, Scratch, Buffer );

				if( hAlphaBitmap )
				{
					Rectangle( ScratchDC, 0, 0, TileSize, TileSize );
					BitBlt( ScratchDC, 0, 0, i->Width(), i->Height(), AlphaDC, i->left, i->top, SRCCOPY );

					sprintf( Buffer, "%s_%02d_a.bmp", m_sourceFile, CurrentFragmentNumber-1 );
					_saveBitmapToFile( ScratchDC, Scratch, Buffer );
				}

				if( m_compressFragments )
				{
					sprintf( Buffer, "dxtex.exe \"%s_%02d.bmp\" %s \"%s_%02d.dds", m_sourceFile, CurrentFragmentNumber - 1,
						s_compressionMethods[m_compressionMethodIndex], m_sourceFile, CurrentFragmentNumber - 1 );

					STARTUPINFO si = {sizeof(si)};
					PROCESS_INFORMATION pi;

					GetStartupInfo( &si );
					si.wShowWindow = SW_HIDE;

					if( !CreateProcess( 0, Buffer, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi ) )
					{
						::MessageBox(0, "Operation aborted because the texture compressor could not be run.\nCheck that dxtex.exe is on the path and retry.", 0, MB_OK );

						sprintf( Buffer, "%s_%02d.bmp", m_sourceFile, CurrentFragmentNumber-1 );
						DeleteFile( Buffer );

						if( hAlphaBitmap )
						{
							sprintf( Buffer, "%s_%02d_a.bmp", m_sourceFile, CurrentFragmentNumber-1 );
							DeleteFile( Buffer );
						}

						SelectObject( ScratchDC, OldBMP );
						DeleteObject( Scratch );
						DeleteDC( ScratchDC );
						DeleteDC( AlphaDC );
						DeleteDC( SourceDC );
						::ReleaseDC( 0, ScreenDC );
						return;
					}

					WaitForSingleObject( pi.hProcess, INFINITE );
					CloseHandle( pi.hProcess );
					CloseHandle( pi.hThread );

					sprintf( Buffer, "%s_%02d.bmp", m_sourceFile, CurrentFragmentNumber-1 );
					DeleteFile( Buffer );

					if( hAlphaBitmap )
					{
						sprintf( Buffer, "%s_%02d_a.bmp", m_sourceFile, CurrentFragmentNumber-1 );
						DeleteFile( Buffer );
					}
				}

				++CurrentFragmentNumber;
				SelectObject( ScratchDC, OldBMP );
				DeleteObject( Scratch );
			}
		}

		DeleteDC( ScratchDC );
		DeleteDC( AlphaDC );
		DeleteDC( SourceDC );
		::ReleaseDC( 0, ScreenDC );
	}
}

UIImageFrame *ImportImageDialog::_generateImageFrameFromData()
{
	UIImageFrame *theFrame = new UIImageFrame;

	if( m_fragmentRectangles.size() == 1 )
	{
		char Buffer[_MAX_PATH+1];

		if (m_fileIsBitmap)
			sprintf( Buffer, "%s_00", m_sourceFile );
		else
		{
			strcpy( Buffer, m_sourceFile );

			char *ext = strrchr( Buffer, '.' );

			if( ext )
				*ext = '\0';
		}
		
		theFrame->SetSourceResource( UIUnicode::narrowToWide (Buffer) );
		theFrame->SetSourceRect(m_fragmentRectangles.front() );
	}
	else
	{
		int CurrentFragmentNumber = 0;

		for( FragmentRectangleList::iterator i = m_fragmentRectangles.begin(); i != m_fragmentRectangles.end(); ++i )
		{
			UIImageFragment *theFragment = new UIImageFragment;
			char             Buffer[_MAX_PATH+1];

			sprintf( Buffer, "%s_%02d", m_sourceFile, CurrentFragmentNumber );
			
			theFragment->SetSourceResource( UIUnicode::narrowToWide (Buffer) );
			theFragment->SetSourceRect( UIRect( 0, 0, i->Width(), i->Height() ) );
			theFragment->SetOffset( i->left, i->top );

			theFrame->AddChild( theFragment );

			++CurrentFragmentNumber;
		}
	}
	return theFrame;
}

void ImportImageDialog::_enableControls()
{
	if( m_fileIsValid )
	{	
		GetDlgItem(IDC_USESUBIMAGE )->EnableWindow(TRUE);
		GetDlgItem(IDC_SUBIMAGERECT )->EnableWindow(!m_useWholeImage);
		GetDlgItem(IDC_VIEWIMAGE )->EnableWindow(!m_useWholeImage);

		GetDlgItem(IDC_USEWHOLEIMAGE )->EnableWindow(TRUE);
		
		GetDlgItem(IDC_GENERATEFRAGMENTS )->EnableWindow(m_fileIsBitmap && m_useWholeImage);

		m_tileMax.EnableWindow(m_fileIsBitmap && m_useWholeImage && m_generateFragments );
		m_tileMin.EnableWindow(m_fileIsBitmap && m_useWholeImage && m_generateFragments );

		GetDlgItem(IDC_COMPRESSFRAGMENTS )->EnableWindow(m_fileIsBitmap && m_useWholeImage && m_generateFragments );
		GetDlgItem(IDC_COMPRESSIONFORMAT )->EnableWindow(m_fileIsBitmap && m_useWholeImage && m_generateFragments && m_compressFragments );
	}
	else
	{
		GetDlgItem(IDC_USESUBIMAGE )->EnableWindow(FALSE );
		GetDlgItem(IDC_SUBIMAGERECT )->EnableWindow(FALSE );
		GetDlgItem(IDC_VIEWIMAGE )->EnableWindow(FALSE );

		GetDlgItem(IDC_USEWHOLEIMAGE )->EnableWindow(FALSE );
		GetDlgItem(IDC_GENERATEFRAGMENTS )->EnableWindow(FALSE );

		m_tileMax.EnableWindow(FALSE);
		m_tileMin.EnableWindow(FALSE);

		GetDlgItem(IDC_COMPRESSFRAGMENTS )->EnableWindow(FALSE);
		GetDlgItem(IDC_COMPRESSIONFORMAT )->EnableWindow(FALSE);
	}
}

void ImportImageDialog::_calculateRectangleSlices( const UIRect &r, long TileSize )
{
	if( !m_fileIsBitmap )
	{
		m_fragmentRectangles.push_back( r );
		return;
	}

	if( TileSize == s_tileSizes[m_tileMinIndex] )
	{
		// Generate only full size, clipped tiles
		int x, y;

		for( y = r.top; y < r.bottom; y += TileSize )
		{
			for( x = r.left; x < r.right; x += TileSize )
			{
				long right  = x + TileSize;
				long bottom = y + TileSize;

				if( right > r.right )
					right = r.right;
				
				if( bottom > r.bottom )
					bottom = r.bottom;

				m_fragmentRectangles.push_back( UIRect( x, y, right, bottom ) );
			}
		}			
	}
	else if( ((r.right - r.left) > (TileSize / 2)) && ((r.bottom - r.top) > (TileSize / 2)) )
	{
		int TilesHigh   = 0;
		int TilesAcross = 0;
		int x, y;

		// Generate some full size unclipped tiles
		for( y = r.top; y <= r.bottom - TileSize; y += TileSize )
		{
			++TilesHigh;
			TilesAcross = 0;

			for( x = r.left; x <= r.right - TileSize; x += TileSize )
			{
				++TilesAcross;
				m_fragmentRectangles.push_back( UIRect( x, y, x + TileSize, y + TileSize ) );
			}
		}

		// Right edge
		_calculateRectangleSlices( UIRect( r.left + TilesAcross * TileSize, r.top, r.right, r.top + TilesHigh * TileSize ), TileSize / 2 );

		// Bottom edge and corner
		_calculateRectangleSlices( UIRect( r.left, r.top + TilesHigh * TileSize, r.right, r.bottom ), TileSize / 2 );
	}
	else
	{
		// Generate only smaller tiles
		_calculateRectangleSlices( r, TileSize / 2 );
	}
}

void ImportImageDialog::_updateFragmentData()
{
	m_fragmentRectangles.clear();

	if( m_fileIsValid )
	{
		UIRect SmallestFragment;
		UIRect LargestFragment;
		long   TotalFragmentArea;
		char	 buffer[1024];

		if( m_useWholeImage )
			_calculateRectangleSlices( UIRect( 0, 0, m_fileWidth, m_fileHeight ), s_tileSizes[m_tileMaxIndex] );	
		else
		{			
			long  BufferLength = SendDlgItemMessage(IDC_SUBIMAGERECT, WM_GETTEXTLENGTH, 0, 0 ) + 1;
			char *Buffer			 = new char [ BufferLength ];

			SendDlgItemMessage(IDC_SUBIMAGERECT, WM_GETTEXT, BufferLength, (LPARAM) Buffer );

			UIRect rc;

			if( UIUtils::ParseRect( Buffer, rc ) )
				m_fragmentRectangles.push_back( rc );

			delete Buffer;
		}

		sprintf( buffer, "%d", m_fragmentRectangles.size() );
		SetDlgItemText(IDC_NUMFRAGMENTS, buffer );

		if( m_fragmentRectangles.size() > 0 )
		{
			FragmentRectangleList::iterator i = m_fragmentRectangles.begin();

			SmallestFragment  = *i;
			LargestFragment   = *i;
			TotalFragmentArea = i->Width() * i->Height();

			for( ++i; i != m_fragmentRectangles.end(); ++i )
			{
				if( i->Width() < SmallestFragment.Width() )
					SmallestFragment = *i;

				if( i->Width() > LargestFragment.Width() )
					LargestFragment = *i;

				TotalFragmentArea += i->Width() * i->Height();
			}

			sprintf( buffer, "%dx%d", LargestFragment.Width(), LargestFragment.Width() );
			SetDlgItemText(IDC_LARGESTFRAGMENT, buffer );

			sprintf( buffer, "%dx%d", SmallestFragment.Width(), SmallestFragment.Width() );
			SetDlgItemText(IDC_SMALLESTFRAGMENT, buffer );

			float memoryused   = float(TotalFragmentArea * 2) / 4.0f + m_fragmentRectangles.size() * ( sizeof(DWORD) + sizeof( DDSURFACEDESC2 ) );
			float memorywasted = memoryused - float(m_fileWidth * m_fileHeight * 2) / 4.0f;

			sprintf( buffer, "%.1f KB", memoryused / 1024.0f );
			SetDlgItemText(IDC_TOTALMEMORY, buffer );
			
			sprintf( buffer, "%.1f KB", memorywasted / 1024.0f );
			SetDlgItemText(IDC_MEMORYWASTED, buffer );
		}
		else
		{
			SetDlgItemText(IDC_LARGESTFRAGMENT, "" );
			SetDlgItemText(IDC_SMALLESTFRAGMENT, "" );
			SetDlgItemText(IDC_TOTALMEMORY, "" );
			SetDlgItemText(IDC_MEMORYWASTED, "" );
		}
	}
	else
	{
		SetDlgItemText(IDC_NUMFRAGMENTS, "0" );
		SetDlgItemText(IDC_LARGESTFRAGMENT, "" );
		SetDlgItemText(IDC_SMALLESTFRAGMENT, "" );
		SetDlgItemText(IDC_TOTALMEMORY, "" );
		SetDlgItemText(IDC_MEMORYWASTED, "" );
	}
}

void ImportImageDialog::_loadData()
{
	GetCurrentDirectory( sizeof( m_parentPath ), m_parentPath );

	m_fileIsValid      = false;
	m_fileIsBitmap     = false;
	m_alphaFileIsValid = false;

	SendDlgItemMessage(IDC_BROWSESOURCE, BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_FOLDER), IMAGE_ICON, 16, 16, 0 ) );

	SendDlgItemMessage(IDC_VIEWIMAGE, BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_VIEWIMAGE), IMAGE_ICON, 16, 16, 0 ) );

	if( m_useWholeImage )
		CheckRadioButton(IDC_USEWHOLEIMAGE, IDC_USESUBIMAGE, IDC_USEWHOLEIMAGE );
	else
		CheckRadioButton(IDC_USEWHOLEIMAGE, IDC_USESUBIMAGE, IDC_USESUBIMAGE );

	if( m_generateFragments )
		CheckDlgButton(IDC_GENERATEFRAGMENTS, BST_CHECKED );
	else
		CheckDlgButton(IDC_GENERATEFRAGMENTS, BST_UNCHECKED );

	if( m_compressFragments )
		CheckDlgButton(IDC_COMPRESSFRAGMENTS, BST_CHECKED );
	else
		CheckDlgButton(IDC_COMPRESSFRAGMENTS, BST_UNCHECKED );

	m_tileMax.SetRange(1, 6);
	m_tileMax.SetPos(m_tileMaxIndex);

	m_tileMin.SetRange(0, 5);
	m_tileMin.SetPos(m_tileMinIndex);

	for( int i = 0; i < sizeof( s_compressionMethods ) / sizeof( *s_compressionMethods ); ++i )
	{
		int ItemID = SendDlgItemMessage(IDC_COMPRESSIONFORMAT, CB_ADDSTRING, 0, (LPARAM)s_compressionMethods[i] );
		SendDlgItemMessage(IDC_COMPRESSIONFORMAT, CB_SETITEMDATA, ItemID, i );
	}

	for( i = 0; i < sizeof( s_compressionMethods ) / sizeof( *s_compressionMethods ); ++i )
	{
		if(m_compressionMethodIndex == SendDlgItemMessage(IDC_COMPRESSIONFORMAT, CB_GETITEMDATA, i, 0 ) )
		{
			SendDlgItemMessage(IDC_COMPRESSIONFORMAT, CB_SETCURSEL, i, 0 );
			break;
		}
	}

	_enableControls();
}

void ImportImageDialog::_saveData()
{
	// All data items are updated from the controls when they change so we don't need
	// to get them here	
	ShowWindow(SW_HIDE);
	ProgressDialog *progress = new ProgressDialog();
	_sliceImagesToFiles(progress);
	progress->DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(ImportImageDialog, CDialog)
	//{{AFX_MSG_MAP(ImportImageDialog)
	ON_WM_CLOSE()
	ON_EN_CHANGE(IDC_SUBIMAGERECT, OnChangeSubimagerect)
	ON_EN_CHANGE(IDC_SOURCEFILE, OnChangeSourcefile)
	ON_BN_CLICKED(IDC_BROWSESOURCE, OnBrowsesource)
	ON_BN_CLICKED(IDC_USESUBIMAGE, OnUsesubimage)
	ON_BN_CLICKED(IDC_USEWHOLEIMAGE, OnUsewholeimage)
	ON_BN_CLICKED(IDC_COMPRESSFRAGMENTS, OnCompressfragments)
	ON_BN_CLICKED(IDC_GENERATEFRAGMENTS, OnGeneratefragments)
	ON_CBN_SELCHANGE(IDC_COMPRESSIONFORMAT, OnSelchangeCompressionformat)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ImportImageDialog message handlers

BOOL ImportImageDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	_loadData();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void ImportImageDialog::OnClose() 
{
	// Equivalent to pressing cancel
	EndDialog(0);
}

void ImportImageDialog::OnOK() 
{
	_saveData();
	UIImageFrame *result = _generateImageFrameFromData();
	EndDialog((long)result);

}

void ImportImageDialog::OnCancel() 
{
	EndDialog(0);
}

void ImportImageDialog::OnChangeSubimagerect() 
{
	_updateFragmentData();
}

void ImportImageDialog::OnChangeSourcefile() 
{
	char Buffer[_MAX_PATH + 1];

	CString sourceFile;
	GetDlgItemText(IDC_SOURCEFILE, sourceFile);
	strcpy(Buffer, sourceFile);
	strcpy(m_sourceFile, Buffer );

	char *ext = strrchr( Buffer, '.' );

	if( ext && !strcmp( ext, ".dds" ) )
	{
		FILE *fp = fopen( Buffer, "r" );

		m_fileIsValid = false;

		if( fp )
		{
			DDSURFACEDESC2 ddsd2;
			char           hdr[4];

			m_fileIsValid      = true;
			m_fileIsBitmap     = false;
			m_alphaFileIsValid = false;

			if( (fread( &hdr, sizeof( hdr ), 1, fp ) == 1) && !memcmp( hdr, "DDS ", 4 ) )
			{
				if( fread( &ddsd2, sizeof( ddsd2 ), 1, fp ) == 1 )
				{
					m_fileWidth  = ddsd2.dwWidth;
					m_fileHeight = ddsd2.dwHeight;

					m_fileIsValid = true;
				}
			}

			fclose( fp );
		}	
	}
	else
	{
		FILE *fp = fopen( Buffer, "r" );

		if( fp )
		{
			BITMAPFILEHEADER	 FileHeader;
			BITMAPINFOHEADER	 BitmapInfoHeader;
			
			if( !fread( &FileHeader, sizeof( FileHeader ), 1, fp ) )
				m_fileIsValid = false;
			else
			{
				if( !fread( &BitmapInfoHeader, sizeof( BitmapInfoHeader ), 1, fp ) )
					m_fileIsValid = false;
				else
				{
					if( FileHeader.bfType != 'MB' )
						m_fileIsValid = false;
					else
					{
						m_fileWidth    = BitmapInfoHeader.biWidth;
						m_fileHeight   = BitmapInfoHeader.biHeight;
						m_fileIsValid  = true;
						m_fileIsBitmap = true;
					}
				}
			}

			fclose( fp );
		}

		if( ext )
			*ext = '\0';

		strcat( Buffer, "_a.bmp" );
		strcpy( m_alphaFile, Buffer );

		m_alphaFileIsValid = false;
		fp = fopen( Buffer, "r" );					

		if( fp )
		{
			BITMAPFILEHEADER	 FileHeader;
			BITMAPINFOHEADER	 BitmapInfoHeader;
			
			if( fread( &FileHeader, sizeof( FileHeader ), 1, fp ) )
			{
				if( fread( &BitmapInfoHeader, sizeof( BitmapInfoHeader ), 1, fp ) )
				{
					if( FileHeader.bfType == 'MB' &&
							(m_fileWidth == BitmapInfoHeader.biWidth) &&
							(m_fileHeight == BitmapInfoHeader.biHeight) )
					{
						m_alphaFileIsValid = true;
					}
				}
			}

			fclose( fp );
		}
	}

	_enableControls();
	_updateFragmentData();
}

void ImportImageDialog::OnBrowsesource() 
{
	OPENFILENAME ofn = {sizeof(ofn)};
	char         Buffer[_MAX_PATH+1];

	Buffer[0] = '\0';

	ofn.lpstrInitialDir = ".";
	ofn.hwndOwner				= m_hWnd;
	ofn.lpstrFilter     = "24 Bit Windows Bitmap Image Files (.bmp)\0*.bmp\0S3 Compressed Textures (.dds)\0*.dds\024 Bit Targa Files (.tga)\0*.tga\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile				= Buffer;
	ofn.nMaxFile				= sizeof( Buffer );
	ofn.lpstrTitle			= "Select Image to Import";		
	ofn.Flags						= OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

	if( GetOpenFileName( &ofn ) )
	{
		char RelPath[_MAX_PATH+1];
		PathRelativePathTo( RelPath, m_parentPath, FILE_ATTRIBUTE_DIRECTORY, Buffer, 0 );

		if( (RelPath[0] == '.') && (RelPath[1] == '\\') )
			memmove( RelPath, RelPath + 2, strlen( RelPath + 2 ) + 1 );

		SendDlgItemMessage(IDC_SOURCEFILE, WM_SETTEXT, 0, (LPARAM)RelPath );
	}
}

void ImportImageDialog::OnUsesubimage() 
{
	OnUsewholeimage();
	/*
	if( IsDlgButtonChecked( hwndDlg, IDC_USEWHOLEIMAGE ) )
		m_useWholeImage = true;
	else
		m_useWholeImage = false;

	EnableControls( hwndDlg );
	UpdateFragmentData( hwndDlg );
	*/
}

void ImportImageDialog::OnUsewholeimage() 
{
	if( IsDlgButtonChecked(IDC_USEWHOLEIMAGE ) )
		m_useWholeImage = true;
	else
		m_useWholeImage = false;

	_enableControls();
	_updateFragmentData();
}

void ImportImageDialog::OnCompressfragments() 
{
	m_compressFragments = IsDlgButtonChecked(IDC_COMPRESSFRAGMENTS ) != 0;
	_enableControls();
}

void ImportImageDialog::OnGeneratefragments() 
{
	m_generateFragments = IsDlgButtonChecked(IDC_GENERATEFRAGMENTS ) != 0;
	_enableControls();
}

void ImportImageDialog::OnSelchangeCompressionformat() 
{
	int SelectedItem = SendDlgItemMessage(IDC_COMPRESSIONFORMAT, CB_GETCURSEL, 0, 0 );
	m_compressionMethodIndex = SendDlgItemMessage(IDC_COMPRESSIONFORMAT, CB_GETITEMDATA, SelectedItem, 0 );
	_enableControls();
}

void ImportImageDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// We get this from the track bars
	if (pScrollBar->m_hWnd == m_tileMax.m_hWnd)
	{
		m_tileMaxIndex = m_tileMax.GetPos();
		
		if (m_tileMaxIndex < m_tileMinIndex)
		{
			m_tileMinIndex = m_tileMaxIndex;
			m_tileMin.SetPos(m_tileMinIndex);
		}

		_updateFragmentData();
	}
	else
	{
		m_tileMinIndex = m_tileMin.GetPos();

		if (m_tileMaxIndex<m_tileMinIndex)
		{
			m_tileMaxIndex = m_tileMinIndex;
			m_tileMax.SetPos(m_tileMaxIndex);
		}

		_updateFragmentData();
	}
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}
