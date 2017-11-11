#include "FirstUiBuilder.h"

#include "resource.h"
#include "UIImageFrame.h"
#include "UIImageStyle.h"
#include "UITypes.h"
#include "UIUtils.h"

#include <commctrl.h>
#include <ddraw.h>
#include <list>
#include <math.h>
#include <shlwapi.h>
#include <stdio.h>

typedef std::list<UIRect>		 FragmentRectangleList;

static bool									 gGenerateFragments				= true;
static bool									 gCompressFragments				= true;
static bool									 gUseWholeImage						= true;
static UIRect								 gSubImageRect;
static int									 gTileMaxIndex						= 3;
static int									 gTileMinIndex						= 1;
																											
static long									 gTileSizes[]							= {32, 64, 128, 256, 512, 1024, 2048};

static char                  gSourceFile[_MAX_PATH];
static char                  gAlphaFile[_MAX_PATH];
static long									 gCompressionMethodIndex	= 0;
static char                 *gCompressionMethods[]		= {"DXT1", "DXT2", "DXT3", "DXT4", "DXT5"};
static char                  gParentPath[_MAX_PATH];

static bool									 gFileIsValid							= false;
static bool                  gFileIsBitmap						= false;
static bool									 gAlphaFileIsValid				= false;
static long									 gFileWidth								= 0;
static long									 gFileHeight							= 0;
static FragmentRectangleList gFragmentRectangles;

void SaveBitmapToFile( HDC hdc, HBITMAP hbmp, const char *Filename )
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

static void SliceImagesToFiles( HWND hwndDlg )
{
	if( gFileIsValid && gFileIsBitmap && gUseWholeImage )
	{
		HBITMAP hSourceBitmap = 0;
		HBITMAP hAlphaBitmap  = 0;
		int     CurrentFragmentNumber = 1;

		SendDlgItemMessage( hwndDlg, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM( 1, gFragmentRectangles.size() ) );

		hSourceBitmap = (HBITMAP)LoadImage( GetModuleHandle(0), gSourceFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );

		if( gAlphaFileIsValid )
			hAlphaBitmap = (HBITMAP)LoadImage( GetModuleHandle(0), gAlphaFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );

		HDC ScreenDC  = GetDC( NULL );
		HDC SourceDC  = CreateCompatibleDC( NULL );
		HDC AlphaDC   = CreateCompatibleDC( NULL );
		HDC ScratchDC = CreateCompatibleDC( NULL );

		SelectObject( SourceDC, hSourceBitmap );

		if( hAlphaBitmap )
			SelectObject( AlphaDC, hAlphaBitmap );

		SelectObject( ScratchDC, GetStockObject( BLACK_BRUSH ) );
		SelectObject( ScratchDC, GetStockObject( BLACK_PEN ) );

		char *pDot = strrchr( gSourceFile, '.' );

		if( pDot )
			*pDot = '\0';

		for( FragmentRectangleList::iterator i = gFragmentRectangles.begin(); i != gFragmentRectangles.end(); ++i )
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

				sprintf( Buffer, "Processing fragment %d of %d...", CurrentFragmentNumber, gFragmentRectangles.size() );
				SetDlgItemText( hwndDlg, IDC_STATUS, Buffer );
				SendDlgItemMessage( hwndDlg, IDC_PROGRESS, PBM_SETPOS, CurrentFragmentNumber, 0 );
				
				Rectangle( ScratchDC, 0, 0, TileSize, TileSize );

				UIRect rc = *i;
				BitBlt( ScratchDC, 0, 0, i->Width(), i->Height(), SourceDC, i->left, i->top, SRCCOPY );

				sprintf( Buffer, "%s_%02d.bmp", gSourceFile, CurrentFragmentNumber-1 );
				SaveBitmapToFile( ScratchDC, Scratch, Buffer );

				if( hAlphaBitmap )
				{
					Rectangle( ScratchDC, 0, 0, TileSize, TileSize );
					BitBlt( ScratchDC, 0, 0, i->Width(), i->Height(), AlphaDC, i->left, i->top, SRCCOPY );

					sprintf( Buffer, "%s_%02d_a.bmp", gSourceFile, CurrentFragmentNumber-1 );
					SaveBitmapToFile( ScratchDC, Scratch, Buffer );
				}

				if( gCompressFragments )
				{
					sprintf( Buffer, "dxtex.exe \"%s_%02d.bmp\" %s \"%s_%02d.dds", gSourceFile, CurrentFragmentNumber - 1,
						gCompressionMethods[gCompressionMethodIndex], gSourceFile, CurrentFragmentNumber - 1 );

					STARTUPINFO si = {sizeof(si)};
					PROCESS_INFORMATION pi;

					GetStartupInfo( &si );
					si.wShowWindow = SW_HIDE;

					if( !CreateProcess( 0, Buffer, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi ) )
					{
						MessageBox(0, "Operation aborted because the texture compressor could not be run.\nCheck that dxtex.exe is on the path and retry.", 0, MB_OK );

						sprintf( Buffer, "%s_%02d.bmp", gSourceFile, CurrentFragmentNumber-1 );
						DeleteFile( Buffer );

						if( hAlphaBitmap )
						{
							sprintf( Buffer, "%s_%02d_a.bmp", gSourceFile, CurrentFragmentNumber-1 );
							DeleteFile( Buffer );
						}

						SelectObject( ScratchDC, OldBMP );
						DeleteObject( Scratch );
						DeleteDC( ScratchDC );
						DeleteDC( AlphaDC );
						DeleteDC( SourceDC );
						ReleaseDC( 0, ScreenDC );
						return;
					}

					WaitForSingleObject( pi.hProcess, INFINITE );
					CloseHandle( pi.hProcess );
					CloseHandle( pi.hThread );

					sprintf( Buffer, "%s_%02d.bmp", gSourceFile, CurrentFragmentNumber-1 );
					DeleteFile( Buffer );

					if( hAlphaBitmap )
					{
						sprintf( Buffer, "%s_%02d_a.bmp", gSourceFile, CurrentFragmentNumber-1 );
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
		ReleaseDC( 0, ScreenDC );
	}
}

static UIImageFrame *GenerateImageFrameFromData( void )
{
	UIImageFrame *theFrame = new UIImageFrame;

	if( gFragmentRectangles.size() == 1 )
	{
		char Buffer[_MAX_PATH+1];

		if( gFileIsBitmap )
			sprintf( Buffer, "%s_00", gSourceFile );
		else
		{
			strcpy( Buffer, gSourceFile );

			char *ext = strrchr( Buffer, '.' );

			if( ext )
				*ext = '\0';
		}
		
		theFrame->SetSourceResource( UIUnicode::narrowToWide (Buffer) );
		theFrame->SetSourceRect( gFragmentRectangles.front() );
	}
	else
	{
		int CurrentFragmentNumber = 0;

		for( FragmentRectangleList::iterator i = gFragmentRectangles.begin(); i != gFragmentRectangles.end(); ++i )
		{
			UIImageFragment *theFragment = new UIImageFragment;
			char             Buffer[_MAX_PATH+1];

			sprintf( Buffer, "%s_%02d", gSourceFile, CurrentFragmentNumber );
			
			theFragment->SetSourceResource( UIUnicode::narrowToWide (Buffer) );
			theFragment->SetSourceRect( UIRect( 0, 0, i->Width(), i->Height() ) );
			theFragment->SetOffset( i->left, i->top );

			theFrame->AddChild( theFragment );

			++CurrentFragmentNumber;
		}
	}
	return theFrame;
}

static void EnableControls( HWND hwndDlg )
{
	if( gFileIsValid )
	{	
		EnableWindow( GetDlgItem( hwndDlg, IDC_USESUBIMAGE ), TRUE );
		EnableWindow( GetDlgItem( hwndDlg, IDC_SUBIMAGERECT ), !gUseWholeImage );
		EnableWindow( GetDlgItem( hwndDlg, IDC_VIEWIMAGE ), !gUseWholeImage );

		EnableWindow( GetDlgItem( hwndDlg, IDC_USEWHOLEIMAGE ), TRUE );
		
		EnableWindow( GetDlgItem( hwndDlg, IDC_GENERATEFRAGMENTS ), gFileIsBitmap && gUseWholeImage );
		EnableWindow( GetDlgItem( hwndDlg, IDC_TILEMAX ), gFileIsBitmap && gUseWholeImage && gGenerateFragments );
		EnableWindow( GetDlgItem( hwndDlg, IDC_TILEMIN ), gFileIsBitmap && gUseWholeImage && gGenerateFragments );

		EnableWindow( GetDlgItem( hwndDlg, IDC_COMPRESSFRAGMENTS ), gFileIsBitmap && gUseWholeImage && gGenerateFragments );
		EnableWindow( GetDlgItem( hwndDlg, IDC_COMPRESSIONFORMAT ), gFileIsBitmap && gUseWholeImage && gGenerateFragments && gCompressFragments );
	}
	else
	{
		EnableWindow( GetDlgItem( hwndDlg, IDC_USESUBIMAGE ), FALSE );
		EnableWindow( GetDlgItem( hwndDlg, IDC_SUBIMAGERECT ), FALSE );
		EnableWindow( GetDlgItem( hwndDlg, IDC_VIEWIMAGE ), FALSE );

		EnableWindow( GetDlgItem( hwndDlg, IDC_USEWHOLEIMAGE ), FALSE );
		EnableWindow( GetDlgItem( hwndDlg, IDC_GENERATEFRAGMENTS ), FALSE );
		EnableWindow( GetDlgItem( hwndDlg, IDC_TILEMAX ), FALSE );
		EnableWindow( GetDlgItem( hwndDlg, IDC_TILEMIN ), FALSE );

		EnableWindow( GetDlgItem( hwndDlg, IDC_COMPRESSFRAGMENTS ), FALSE );
		EnableWindow( GetDlgItem( hwndDlg, IDC_COMPRESSIONFORMAT ), FALSE );
	}
}

static void CalculateRectangleSlices( const UIRect &r, long TileSize )
{
	if( !gFileIsBitmap )
	{
		gFragmentRectangles.push_back( r );
		return;
	}

	if( TileSize == gTileSizes[gTileMinIndex] )
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

				gFragmentRectangles.push_back( UIRect( x, y, right, bottom ) );
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
				gFragmentRectangles.push_back( UIRect( x, y, x + TileSize, y + TileSize ) );
			}
		}

		// Right edge
		CalculateRectangleSlices( UIRect( r.left + TilesAcross * TileSize, r.top, r.right, r.top + TilesHigh * TileSize ), TileSize / 2 );

		// Bottom edge and corner
		CalculateRectangleSlices( UIRect( r.left, r.top + TilesHigh * TileSize, r.right, r.bottom ), TileSize / 2 );
	}
	else
	{
		// Generate only smaller tiles
		CalculateRectangleSlices( r, TileSize / 2 );
	}
}

static void UpdateFragmentData( HWND hwndDlg )
{
	gFragmentRectangles.clear();

	if( gFileIsValid )
	{
		UIRect SmallestFragment;
		UIRect LargestFragment;
		long   TotalFragmentArea;
		char	 buffer[1024];

		if( gUseWholeImage )
			CalculateRectangleSlices( UIRect( 0, 0, gFileWidth, gFileHeight ), gTileSizes[gTileMaxIndex] );	
		else
		{			
			long  BufferLength = SendDlgItemMessage( hwndDlg, IDC_SUBIMAGERECT, WM_GETTEXTLENGTH, 0, 0 ) + 1;
			char *Buffer			 = new char [ BufferLength ];

			SendDlgItemMessage( hwndDlg, IDC_SUBIMAGERECT, WM_GETTEXT, BufferLength, (LPARAM) Buffer );

			UIRect rc;

			if( UIUtils::ParseRect( Buffer, rc ) )
				gFragmentRectangles.push_back( rc );

			delete Buffer;
		}

		sprintf( buffer, "%d", gFragmentRectangles.size() );
		SetDlgItemText( hwndDlg, IDC_NUMFRAGMENTS, buffer );

		if( gFragmentRectangles.size() > 0 )
		{
			FragmentRectangleList::iterator i = gFragmentRectangles.begin();

			SmallestFragment  = *i;
			LargestFragment   = *i;
			TotalFragmentArea = i->Width() * i->Height();

			for( ++i; i != gFragmentRectangles.end(); ++i )
			{
				if( i->Width() < SmallestFragment.Width() )
					SmallestFragment = *i;

				if( i->Width() > LargestFragment.Width() )
					LargestFragment = *i;

				TotalFragmentArea += i->Width() * i->Height();
			}

			sprintf( buffer, "%dx%d", LargestFragment.Width(), LargestFragment.Width() );
			SetDlgItemText( hwndDlg, IDC_LARGESTFRAGMENT, buffer );

			sprintf( buffer, "%dx%d", SmallestFragment.Width(), SmallestFragment.Width() );
			SetDlgItemText( hwndDlg, IDC_SMALLESTFRAGMENT, buffer );

			float memoryused   = float(TotalFragmentArea * 2) / 4.0f + gFragmentRectangles.size() * ( sizeof(DWORD) + sizeof( DDSURFACEDESC2 ) );
			float memorywasted = memoryused - float(gFileWidth * gFileHeight * 2) / 4.0f;

			sprintf( buffer, "%.1f KB", memoryused / 1024.0f );
			SetDlgItemText( hwndDlg, IDC_TOTALMEMORY, buffer );
			
			sprintf( buffer, "%.1f KB", memorywasted / 1024.0f );
			SetDlgItemText( hwndDlg, IDC_MEMORYWASTED, buffer );
		}
		else
		{
			SetDlgItemText( hwndDlg, IDC_LARGESTFRAGMENT, "" );
			SetDlgItemText( hwndDlg, IDC_SMALLESTFRAGMENT, "" );
			SetDlgItemText( hwndDlg, IDC_TOTALMEMORY, "" );
			SetDlgItemText( hwndDlg, IDC_MEMORYWASTED, "" );
		}
	}
	else
	{
		SetDlgItemText( hwndDlg, IDC_NUMFRAGMENTS, "0" );
		SetDlgItemText( hwndDlg, IDC_LARGESTFRAGMENT, "" );
		SetDlgItemText( hwndDlg, IDC_SMALLESTFRAGMENT, "" );
		SetDlgItemText( hwndDlg, IDC_TOTALMEMORY, "" );
		SetDlgItemText( hwndDlg, IDC_MEMORYWASTED, "" );
	}
}

static void LoadData( HWND hwndDlg )
{
	GetCurrentDirectory( sizeof( gParentPath ), gParentPath );

	gFileIsValid      = false;
	gFileIsBitmap     = false;
	gAlphaFileIsValid = false;

	SendDlgItemMessage( hwndDlg, IDC_BROWSESOURCE, BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_FOLDER), IMAGE_ICON, 16, 16, 0 ) );

	SendDlgItemMessage( hwndDlg, IDC_VIEWIMAGE, BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_VIEWIMAGE), IMAGE_ICON, 16, 16, 0 ) );

	if( gUseWholeImage )
		CheckRadioButton( hwndDlg, IDC_USEWHOLEIMAGE, IDC_USESUBIMAGE, IDC_USEWHOLEIMAGE );
	else
		CheckRadioButton( hwndDlg, IDC_USEWHOLEIMAGE, IDC_USESUBIMAGE, IDC_USESUBIMAGE );

	if( gGenerateFragments )
		CheckDlgButton( hwndDlg, IDC_GENERATEFRAGMENTS, BST_CHECKED );
	else
		CheckDlgButton( hwndDlg, IDC_GENERATEFRAGMENTS, BST_UNCHECKED );

	if( gCompressFragments )
		CheckDlgButton( hwndDlg, IDC_COMPRESSFRAGMENTS, BST_CHECKED );
	else
		CheckDlgButton( hwndDlg, IDC_COMPRESSFRAGMENTS, BST_UNCHECKED );

	SendDlgItemMessage( hwndDlg, IDC_TILEMAX, TBM_SETRANGE, FALSE, MAKELONG( 1, 6 ) );
	SendDlgItemMessage( hwndDlg, IDC_TILEMAX, TBM_SETPOS, TRUE, gTileMaxIndex );

	SendDlgItemMessage( hwndDlg, IDC_TILEMIN, TBM_SETRANGE, FALSE, MAKELONG( 0, 5 ) );
	SendDlgItemMessage( hwndDlg, IDC_TILEMIN, TBM_SETPOS, TRUE, gTileMinIndex );

	for( int i = 0; i < sizeof( gCompressionMethods ) / sizeof( *gCompressionMethods ); ++i )
	{
		int ItemID = SendDlgItemMessage( hwndDlg, IDC_COMPRESSIONFORMAT, CB_ADDSTRING, 0, (LPARAM)gCompressionMethods[i] );
		SendDlgItemMessage( hwndDlg, IDC_COMPRESSIONFORMAT, CB_SETITEMDATA, ItemID, i );
	}

	for( int i = 0; i < sizeof( gCompressionMethods ) / sizeof( *gCompressionMethods ); ++i )
	{
		if( gCompressionMethodIndex == SendDlgItemMessage( hwndDlg, IDC_COMPRESSIONFORMAT, CB_GETITEMDATA, i, 0 ) )
		{
			SendDlgItemMessage( hwndDlg, IDC_COMPRESSIONFORMAT, CB_SETCURSEL, i, 0 );
			break;
		}
	}

	EnableControls( hwndDlg );
}

static BOOL CALLBACK NullDialogProc( HWND, UINT, WPARAM, LPARAM )
{
	return FALSE;
}

static void SaveData( HWND hwndDlg )
{
	// All data items are updated from the controls when they change so we don't need
	// to get them here	
	ShowWindow( hwndDlg, SW_HIDE );
	HWND hwndProgress = CreateDialog( GetModuleHandle(0), MAKEINTRESOURCE( IDD_PROGRESS ), NULL, NullDialogProc );
	SliceImagesToFiles( hwndProgress );
	DestroyWindow( hwndProgress );
}

static void Cleanup( void )
{
}

BOOL CALLBACK ImportImageDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_INITDIALOG:
			LoadData( hwndDlg );
			return TRUE;

		case WM_CLOSE:
			// Equivalent to pressing cancel
			EndDialog( hwndDlg, 0 );
			Cleanup();
			return 0;

		case WM_COMMAND:
			if( LOWORD( wParam ) == IDOK )
			{
				SaveData( hwndDlg );
				EndDialog( hwndDlg, (long)GenerateImageFrameFromData() );
				Cleanup();
			}
			else if( LOWORD( wParam ) == IDCANCEL )
			{
				EndDialog( hwndDlg, 0 );
				Cleanup();
			}
			else if( LOWORD( wParam ) == IDC_SUBIMAGERECT )
			{
				if( HIWORD( wParam ) == EN_CHANGE )
					UpdateFragmentData( hwndDlg );
			}
			else if( LOWORD( wParam ) == IDC_SOURCEFILE )
			{
				if( HIWORD( wParam ) == EN_CHANGE )
				{
					char Buffer[_MAX_PATH + 1];

					SendMessage( (HWND)lParam, WM_GETTEXT, sizeof( Buffer ), (LPARAM)Buffer );
					strcpy( gSourceFile, Buffer );

					char *ext = strrchr( Buffer, '.' );

					if( ext && !strcmp( ext, ".dds" ) )
					{
						FILE *fp = fopen( Buffer, "r" );

						gFileIsValid = false;

						if( fp )
						{
							DDSURFACEDESC2 ddsd2;
							char           hdr[4];

							gFileIsValid      = true;
							gFileIsBitmap     = false;
							gAlphaFileIsValid = false;

							if( (fread( &hdr, sizeof( hdr ), 1, fp ) == 1) && !memcmp( hdr, "DDS ", 4 ) )
							{
								if( fread( &ddsd2, sizeof( ddsd2 ), 1, fp ) == 1 )
								{
									gFileWidth  = ddsd2.dwWidth;
									gFileHeight = ddsd2.dwHeight;

									gFileIsValid = true;
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
								gFileIsValid = false;
							else
							{
								if( !fread( &BitmapInfoHeader, sizeof( BitmapInfoHeader ), 1, fp ) )
									gFileIsValid = false;
								else
								{
									if( FileHeader.bfType != 'MB' )
										gFileIsValid = false;
									else
									{
										gFileWidth    = BitmapInfoHeader.biWidth;
										gFileHeight   = BitmapInfoHeader.biHeight;
										gFileIsValid  = true;
										gFileIsBitmap = true;
									}
								}
							}

							fclose( fp );
						}

						if( ext )
							*ext = '\0';

						strcat( Buffer, "_a.bmp" );
						strcpy( gAlphaFile, Buffer );

						gAlphaFileIsValid = false;
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
											(gFileWidth == BitmapInfoHeader.biWidth) &&
											(gFileHeight == BitmapInfoHeader.biHeight) )
									{
										gAlphaFileIsValid = true;
									}
								}
							}

							fclose( fp );
						}
					}

					EnableControls( hwndDlg );
					UpdateFragmentData( hwndDlg );
				}
			}
			else if( LOWORD( wParam ) == IDC_BROWSESOURCE )
			{
				if( HIWORD( wParam ) == BN_CLICKED )
				{
					OPENFILENAME ofn = {sizeof(ofn)};
					char         Buffer[_MAX_PATH+1];

					Buffer[0] = '\0';

					ofn.lpstrInitialDir = ".";
					ofn.hwndOwner				= hwndDlg;
					ofn.lpstrFilter     = "24 Bit Windows Bitmap Image Files (.bmp)\0*.bmp\0S3 Compressed Textures (.dds)\0*.dds\024 Bit Targa Files (.tga)\0*.tga\0";
					ofn.nFilterIndex    = 1;
					ofn.lpstrFile				= Buffer;
					ofn.nMaxFile				= sizeof( Buffer );
					ofn.lpstrTitle			= "Select Image to Import";		
					ofn.Flags						= OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

					if( GetOpenFileName( &ofn ) )
					{
						char RelPath[_MAX_PATH+1];
						PathRelativePathTo( RelPath, gParentPath, FILE_ATTRIBUTE_DIRECTORY, Buffer, 0 );

						if( (RelPath[0] == '.') && (RelPath[1] == '\\') )
							memmove( RelPath, RelPath + 2, strlen( RelPath + 2 ) + 1 );

						SendDlgItemMessage( hwndDlg, IDC_SOURCEFILE, WM_SETTEXT, 0, (LPARAM)RelPath );
					}
				}
			}
			else if( (LOWORD( wParam ) == IDC_USESUBIMAGE) || (LOWORD( wParam ) == IDC_USEWHOLEIMAGE) )
			{
				if( HIWORD( wParam ) == BN_CLICKED )
				{
					if( IsDlgButtonChecked( hwndDlg, IDC_USEWHOLEIMAGE ) )
						gUseWholeImage = true;
					else
						gUseWholeImage = false;

					EnableControls( hwndDlg );
					UpdateFragmentData( hwndDlg );
				}
			}
			else if( LOWORD( wParam ) == IDC_COMPRESSFRAGMENTS )
			{
				if( HIWORD( wParam ) == BN_CLICKED )
				{
					gCompressFragments = IsDlgButtonChecked( hwndDlg, IDC_COMPRESSFRAGMENTS ) != 0;
					EnableControls( hwndDlg );
				}
			}
			else if( LOWORD( wParam ) == IDC_GENERATEFRAGMENTS )
			{
				if( HIWORD( wParam ) == BN_CLICKED )
				{
					gGenerateFragments = IsDlgButtonChecked( hwndDlg, IDC_GENERATEFRAGMENTS ) != 0;
					EnableControls( hwndDlg );
				}				
			}
			else if( LOWORD( wParam ) == IDC_COMPRESSIONFORMAT )
			{
				if( HIWORD( wParam ) == CBN_SELCHANGE )
				{
					int SelectedItem = SendDlgItemMessage( hwndDlg, IDC_COMPRESSIONFORMAT, CB_GETCURSEL, 0, 0 );
					gCompressionMethodIndex = SendDlgItemMessage( hwndDlg, IDC_COMPRESSIONFORMAT, CB_GETITEMDATA, SelectedItem, 0 );
					EnableControls( hwndDlg );
				}
			}
			return 0;

		case WM_HSCROLL:
			// We get this from the track bars
			if( (HWND)lParam == GetDlgItem( hwndDlg, IDC_TILEMAX ) )
			{
				gTileMaxIndex = SendDlgItemMessage( hwndDlg, IDC_TILEMAX, TBM_GETPOS, 0, 0 );
				
				if( gTileMaxIndex < gTileMinIndex )
				{
					gTileMinIndex = gTileMaxIndex;
					SendDlgItemMessage( hwndDlg, IDC_TILEMIN, TBM_SETPOS, TRUE, gTileMinIndex );
				}

				UpdateFragmentData( hwndDlg );
			}
			else
			{
				gTileMinIndex = SendDlgItemMessage( hwndDlg, IDC_TILEMIN, TBM_GETPOS, 0, 0 );

				if( gTileMaxIndex < gTileMinIndex )
				{
					gTileMaxIndex = gTileMinIndex;
					SendDlgItemMessage( hwndDlg, IDC_TILEMAX, TBM_SETPOS, TRUE, gTileMaxIndex );
				}

				UpdateFragmentData( hwndDlg );
			}

			return 0;

		default:
			return 0;
	}
}
