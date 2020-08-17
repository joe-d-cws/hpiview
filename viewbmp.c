//
//  Bmp Viewer Code
//

#include <windows.h>
//#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "resource.h"
#include "hpiview.h"
#include "viewstuf.h"
#include "jpeglib.h"
#include "palette.h"

#pragma pack(1)

typedef struct _VIEWOPTION {
	int option;
	int Length;
	char *Name;
	char *buff;
	HPALETTE hpal;
	HDC hdc;
} VIEWOPTION;

typedef struct _PCXHEADER {
  char Manufacturer;  // Constant Flag  10 = ZSoft .PCX
	char Version;      //	0 = Version 2.5, 2 = Version 2.8 w/palette information
				             // 3 = Version 2.8 w/o palette information  5 = Version 3.0
  char Encoding;  //	1 = .PCX run length encoding
	char BPP;  //	Bits per pixel
	short XMin;
	short YMin;
	short XMax;
	short YMax;  // picture dimensions
	short HRes;  //	Horizontal Resolution of creating device
	short VRes;  //	Vertical Resolution of creating device
  unsigned char ColorMap[16][3];  //	Color palette setting
  char Res1;   // Reserved
	char NPlanes; //	Number of color planes
	short BytesPerLine; //Number of bytes per scan line per color plane (always even for .PCX files)
	short PaletteInfo;  //Palette Info	2	How to interpret palette - 1 = color/BW,2 = grayscale
  char  filler[58];
} PCXHEADER;

#pragma pack()

const char ViewBmpWindowClass[] = "HPIViewBmp";
int ViewBmpClassRegistered = FALSE;

void ViewBmpAdjustScroll(HWND hwnd, int nWidth, int nHeight)
{
  RECT rc;
  SCROLLINFO si;
	int h;
	int w;
	int hp;
	int vp;
  
  GetClientRect(hwnd, &rc);
  
  w = rc.right - rc.left;
  h = rc.bottom - rc.top;

  si.cbSize = sizeof(si);
  si.fMask = SIF_ALL;
	GetScrollInfo(hwnd, SB_VERT, &si);
  
  if (h >= si.nMax)
    si.nPos = 0;
  else
    si.nPos = min(si.nMax-si.nPos, si.nPos);

	vp = si.nPos;
  si.cbSize = sizeof(si);
  si.fMask = SIF_PAGE | SIF_POS;
  si.nPage = h;
  SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

  si.cbSize = sizeof(si);
  si.fMask = SIF_ALL;
	GetScrollInfo(hwnd, SB_HORZ, &si);

  if (w >= si.nMax)
    si.nPos = 0;
  else
    si.nPos = min(si.nMax-si.nPos, si.nPos);

	hp = si.nPos;
  si.cbSize = sizeof(si);
  si.fMask = SIF_PAGE | SIF_POS;
  si.nPage = w;
  SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

}

void LoadViewBmp(HWND hwnd, VIEWOPTION *vo)
{
	BITMAPINFO *bmi;
	int nColorData;
	LOGPALETTE *lp;
	int i;																		
	char *bits;
	SCROLLINFO si;
	//RECT rc;

	HBITMAP hbmp;

	HDC hdc;
	
	bmi = (void *) (vo->buff+sizeof(BITMAPFILEHEADER));

	/*
	GetClientRect(hwnd, &rc);
	rc.right = bmi->bmiHeader.biWidth;
	rc.bottom = bmi->bmiHeader.biHeight;
	AdjustWindowRectEx(&rc, GetWindowLong(hwnd, GWL_STYLE), FALSE, GetWindowLong(hwnd, GWL_EXSTYLE));

	SetWindowPos(hwnd, 0, 0, 0, rc.right-rc.left, rc.bottom-rc.top, 
		SWP_NOMOVE | SWP_NOZORDER);
	*/

	si.cbSize = sizeof(si);
  si.fMask = SIF_POS | SIF_RANGE;
  si.nPos = 0;
	si.nMin = 0;
	si.nMax = bmi->bmiHeader.biHeight;
  SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

  si.cbSize = sizeof(si);
  si.fMask = SIF_POS | SIF_RANGE;
  si.nPos = 0;
	si.nMin = 0;
	si.nMax = bmi->bmiHeader.biWidth;
  SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

	if (bmi->bmiHeader.biClrUsed != 0)
    nColorData = bmi->bmiHeader.biClrUsed;
  else
    switch(bmi->bmiHeader.biBitCount) {
      case 1  : nColorData = 2;   break; // Monochrome
      case 4  : nColorData = 16;  break; // VGA
      case 8  : nColorData = 256; break; // SVGA
      case 24 : nColorData = 0;   break; // True Color
    }

  // Allocate memory for color palette.
  lp = GetMem(sizeof(LOGPALETTE)+(nColorData * sizeof(PALETTEENTRY)), FALSE);

  lp->palVersion    = 0x300;      
  lp->palNumEntries = nColorData;

  // Load each color into the palette.
  for (i = 0; i < nColorData; i++ ) {
    lp->palPalEntry[i].peRed   = bmi->bmiColors[i].rgbRed;
    lp->palPalEntry[i].peGreen = bmi->bmiColors[i].rgbGreen;
    lp->palPalEntry[i].peBlue  = bmi->bmiColors[i].rgbBlue;
  }

  // Create the Palette.
  vo->hpal = CreatePalette(lp);

	// point to bitmap bits
  bits = (char *) bmi;
  bits += bmi->bmiHeader.biSize + (nColorData * sizeof(RGBQUAD));

	// get memory DC to hold bitmap
	hdc = GetDC(hwnd);
	vo->hdc = CreateCompatibleDC(hdc);

	// create bitmap object and select it into memory dc
	hbmp = CreateDIBitmap(hdc, &bmi->bmiHeader, CBM_INIT, bits, bmi, DIB_RGB_COLORS);
	SelectObject(vo->hdc, hbmp);

	// cleanup
	DeleteObject(hbmp);
	ReleaseDC(hwnd, hdc);
  FreeMem(lp);
	FreeMem(vo->buff);
	vo->buff = NULL;

}

int LoadViewPcx(HWND hwnd, VIEWOPTION *vo)
{
	BITMAPINFO *bmi;
	int nColorData;
	LOGPALETTE *lp;
	int i;																		
	char *bits;
	SCROLLINFO si;
	PCXHEADER *pcx;
	char *PcxPal;
	char *PcxBits;
	int XSize;
	int FudgeX;
	int YSize;
	int ImgSize;
	int rep;
	char b;
	char *outbits;
	int x;
	int y;
	//RECT rc;

	HBITMAP hbmp;

	HDC hdc;
	
	pcx = (void *) vo->buff;

	// only do 256 color PCX files - sanity checks here
	if (pcx->Manufacturer != 10)
		return FALSE;
	if (pcx->Version != 5)
		return FALSE;
	if (pcx->Encoding != 1)
		return FALSE;
	if (pcx->BPP != 8)
		return FALSE;
	if (pcx->NPlanes != 1)
		return FALSE;

	PcxPal = vo->buff + vo->Length - 769;
	
	if (*PcxPal != 12)
		return FALSE;

	PcxPal++;
	PcxBits = vo->buff + sizeof(PCXHEADER);



	XSize = pcx->XMax - pcx->XMin + 1;
	YSize = pcx->YMax - pcx->YMin + 1;

  /*if (XSize == 1)
    XSize = pcx->HRes;
  if (YSize == 1)
    YSize = pcx->VRes;*/

	/*
	GetClientRect(hwnd, &rc);
	rc.right = XSize - 1;
	rc.bottom = YSize - 1;
	AdjustWindowRectEx(&rc, GetWindowLong(hwnd, GWL_STYLE), FALSE, GetWindowLong(hwnd, GWL_EXSTYLE));

	SetWindowPos(hwnd, 0, 0, 0, rc.right-rc.left, rc.bottom-rc.top, 
		SWP_NOMOVE | SWP_NOZORDER);
	*/

	si.cbSize = sizeof(si);
  si.fMask = SIF_POS | SIF_RANGE;
  si.nPos = 0;
	si.nMin = 0;
	si.nMax = YSize - 1;
  SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

  si.cbSize = sizeof(si);
  si.fMask = SIF_POS | SIF_RANGE;
  si.nPos = 0;
	si.nMin = 0;
	si.nMax = XSize - 1;
  SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

	i = sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD));
	bmi = GetMem(i, TRUE); 

	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi->bmiHeader.biWidth = XSize;
	bmi->bmiHeader.biHeight = -YSize;
	bmi->bmiHeader.biPlanes = 1; 
  bmi->bmiHeader.biBitCount = 8;
  bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biSizeImage = 0; 

  nColorData = 256;

  // Allocate memory for color palette.
  lp = GetMem(sizeof(LOGPALETTE)+(nColorData * sizeof(PALETTEENTRY)), FALSE);

  lp->palVersion    = 0x300;      
  lp->palNumEntries = nColorData;

  // Load each color into the palette.
  for (i = 0; i < nColorData; i++ ) {
    lp->palPalEntry[i].peRed   = bmi->bmiColors[i].rgbRed = *PcxPal++;
    lp->palPalEntry[i].peGreen = bmi->bmiColors[i].rgbGreen = *PcxPal++;
    lp->palPalEntry[i].peBlue  = bmi->bmiColors[i].rgbBlue = *PcxPal++;
  }

  // Create the Palette.
  vo->hpal = CreatePalette(lp);



	// load bitmap bits
	FudgeX = RoundUp(XSize, 4);
	ImgSize = FudgeX * YSize;
  bits = GetMem(ImgSize, TRUE);

	outbits = bits;
	for (y = 0; y < YSize; y++) {
		outbits = bits + (y * FudgeX);
		x = 0;
		while (x < XSize) {
  		b = *PcxBits++;
	  	if ((b & 0xC0) == 0xC0)	{
		  	rep = b & 0x3F;
			  b = *PcxBits++;
			  while (rep--)
				  outbits[x++] = b;
			}
		  else {
			  outbits[x++] = b;
			}
		}
	}

	// get memory DC to hold bitmap
	hdc = GetDC(hwnd);
	vo->hdc = CreateCompatibleDC(hdc);

	// create bitmap object and select it into memory dc
	hbmp = CreateDIBitmap(hdc, &bmi->bmiHeader, CBM_INIT, bits, bmi, DIB_RGB_COLORS);
	SelectObject(vo->hdc, hbmp);

	// cleanup
	DeleteObject(hbmp);
	ReleaseDC(hwnd, hdc);
	FreeMem(bits);
	FreeMem(bmi);
  FreeMem(lp);
	FreeMem(vo->buff);
	vo->buff = NULL;
	return TRUE;
}

void jpg_init(j_decompress_ptr cinfo)
{
  // no need to do anything here
}

boolean jpg_fill(j_decompress_ptr cinfo)
{
  // input buffer already filled
  // should never be called.
  return TRUE;
}

void jpg_skip(j_decompress_ptr cinfo, long num_bytes)
{
    cinfo->src->next_input_byte += (size_t) num_bytes;
    cinfo->src->bytes_in_buffer -= (size_t) num_bytes;
}

void jpg_term(j_decompress_ptr cinfo)
{
}

/*int LoadViewJpg8(HWND hwnd, VIEWOPTION *vo)
{
	BITMAPINFO *bmi;
	int nColorData;
	LOGPALETTE *lp;
	int i;																		
	char *bits;
	SCROLLINFO si;
	int XSize;
	int YSize;
	int ImgSize;
  int count;
  JSAMPARRAY p;
	int y;
	//RECT rc;
  int LinesRead;
  int LineSize;

	HBITMAP hbmp;

	HDC hdc;
	
  struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
  struct jpeg_source_mgr source;
  JSAMPARRAY jsa;
  int components;

  cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

  source.next_input_byte = vo->buff;
  source.bytes_in_buffer = vo->Length;

  source.init_source = jpg_init;
  source.fill_input_buffer = jpg_fill;
  source.skip_input_data = jpg_skip;
  source.resync_to_restart = jpeg_resync_to_restart;
  source.term_source = jpg_term;

  cinfo.src = &source;

  jpeg_read_header(&cinfo, TRUE);

  cinfo.quantize_colors = TRUE;
  cinfo.desired_number_of_colors = 256;
  
  jpeg_start_decompress(&cinfo);

  //components = 1;
  components = cinfo.output_components;

	XSize = cinfo.output_width;
	YSize = cinfo.output_height;

  LineSize = XSize * components * sizeof(JSAMPLE);

 	ImgSize = YSize * LineSize;
  bits = GetMem(ImgSize, TRUE);

  jsa = GetMem(YSize * sizeof(JSAMPROW), FALSE);
 
  for (y = 0; y < YSize; y++)
    jsa[y] = (JSAMPROW) (bits + (y * LineSize));

  LinesRead = 0;
  while (cinfo.output_scanline < cinfo.output_height) {
    LinesRead += jpeg_read_scanlines(&cinfo, jsa+cinfo.output_scanline, cinfo.output_height-cinfo.output_scanline);
  }

  nColorData = cinfo.actual_number_of_colors;

  i = sizeof(BITMAPINFOHEADER) + (nColorData * sizeof(RGBQUAD));
	bmi = GetMem(i, TRUE); 

	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi->bmiHeader.biWidth = XSize;
	bmi->bmiHeader.biHeight = -YSize;
	bmi->bmiHeader.biPlanes = 1; 
  bmi->bmiHeader.biBitCount = 8;
  bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biSizeImage = 0; //ImgSize; 
  
  // Allocate memory for color palette.
  lp = GetMem(sizeof(LOGPALETTE)+(nColorData * sizeof(PALETTEENTRY)), FALSE);

  lp->palVersion    = 0x300;      
  lp->palNumEntries = nColorData;
  p = cinfo.colormap;
  for (count = 0; count < nColorData; count++) {
    lp->palPalEntry[count].peRed = bmi->bmiColors[count].rgbRed = p[0][count];
    lp->palPalEntry[count].peGreen = bmi->bmiColors[count].rgbGreen = p[1][count];
    lp->palPalEntry[count].peBlue = bmi->bmiColors[count].rgbBlue = p[2][count];
    lp->palPalEntry[count].peFlags = 0;
  }
  // Create the Palette.
  vo->hpal = CreatePalette(lp);

  jpeg_finish_decompress(&cinfo);

  jpeg_destroy_decompress(&cinfo);

	si.cbSize = sizeof(si);
  si.fMask = SIF_POS | SIF_RANGE;
  si.nPos = 0;
	si.nMin = 0;
	si.nMax = YSize - 1;
  SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

  si.cbSize = sizeof(si);
  si.fMask = SIF_POS | SIF_RANGE;
  si.nPos = 0;
	si.nMin = 0;
	si.nMax = XSize - 1;
  SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

	// get memory DC to hold bitmap
	hdc = GetDC(hwnd);
	vo->hdc = CreateCompatibleDC(hdc);

	// create bitmap object and select it into memory dc
	hbmp = CreateDIBitmap(hdc, &bmi->bmiHeader, CBM_INIT, bits, bmi, DIB_RGB_COLORS);
  //hbmp = CreateBitmap(XSize, YSize, 1, 24, bits);
	SelectObject(vo->hdc, hbmp);

	// cleanup
	DeleteObject(hbmp);
	ReleaseDC(hwnd, hdc);
	FreeMem(bits);
  FreeMem(jsa);
	FreeMem(bmi);
  FreeMem(lp);
	FreeMem(vo->buff);
	vo->buff = NULL;
	return TRUE;
}
*/
int LoadViewJpg(HWND hwnd, VIEWOPTION *vo)
{
	BITMAPINFO *bmi;
	int nColorData;
	int i;																		
	char *bits;
	SCROLLINFO si;
	int XSize;
	int YSize;
	int ImgSize;
	int y;
  int LinesRead;
  int LineSize;

	HBITMAP hbmp;

	HDC hdc;
	
  struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
  struct jpeg_source_mgr source;
  JSAMPARRAY jsa;
  int components;

  cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

  source.next_input_byte = vo->buff;
  source.bytes_in_buffer = vo->Length;

  source.init_source = jpg_init;
  source.fill_input_buffer = jpg_fill;
  source.skip_input_data = jpg_skip;
  source.resync_to_restart = jpeg_resync_to_restart;
  source.term_source = jpg_term;

  cinfo.src = &source;

  jpeg_read_header(&cinfo, TRUE);

  cinfo.quantize_colors = FALSE;
  
  jpeg_start_decompress(&cinfo);

  components = cinfo.output_components;

	XSize = cinfo.output_width;
	YSize = cinfo.output_height;

  LineSize = XSize * components * sizeof(JSAMPLE);

 	ImgSize = YSize * LineSize;
  bits = GetMem(ImgSize, TRUE);

  jsa = GetMem(YSize * sizeof(JSAMPROW), FALSE);
 
  for (y = 0; y < YSize; y++)
    jsa[y] = (JSAMPROW) (bits + (y * LineSize));

  LinesRead = 0;
  while (cinfo.output_scanline < cinfo.output_height) {
    LinesRead += jpeg_read_scanlines(&cinfo, jsa+cinfo.output_scanline, cinfo.output_height-cinfo.output_scanline);
  }

  nColorData = 0;

  i = sizeof(BITMAPINFOHEADER) + (nColorData * sizeof(RGBQUAD));
	bmi = GetMem(i, TRUE); 

	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi->bmiHeader.biWidth = XSize;
	bmi->bmiHeader.biHeight = -YSize;
	bmi->bmiHeader.biPlanes = 1; 
  bmi->bmiHeader.biBitCount = 24;
  bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biSizeImage = 0;

  vo->hpal = NULL;

  jpeg_finish_decompress(&cinfo);

  jpeg_destroy_decompress(&cinfo);

	si.cbSize = sizeof(si);
  si.fMask = SIF_POS | SIF_RANGE;
  si.nPos = 0;
	si.nMin = 0;
	si.nMax = YSize - 1;
  SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

  si.cbSize = sizeof(si);
  si.fMask = SIF_POS | SIF_RANGE;
  si.nPos = 0;
	si.nMin = 0;
	si.nMax = XSize - 1;
  SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

	// get memory DC to hold bitmap
	hdc = GetDC(hwnd);
	vo->hdc = CreateCompatibleDC(hdc);

	// create bitmap object and select it into memory dc
	hbmp = CreateDIBitmap(hdc, &bmi->bmiHeader, CBM_INIT, bits, bmi, 0); //DIB_RGB_COLORS);
  //hbmp = CreateBitmap(XSize, YSize, 1, 24, bits);
	SelectObject(vo->hdc, hbmp);

	// cleanup
	DeleteObject(hbmp);
	ReleaseDC(hwnd, hdc);
	FreeMem(bits);
  FreeMem(jsa);
	FreeMem(bmi);
	FreeMem(vo->buff);
	vo->buff = NULL;
	return TRUE;
}

int LoadViewSct(HWND hwnd, VIEWOPTION *vo)
{
	BITMAPINFO *bmi;
	int nColorData;
	LOGPALETTE *lp;
	int i;																		
	char *bits;
	char *outbits;
	char *TileBits;
	SCROLLINFO si;
	SCTHEADER *sct;
	int XSize;
	int YSize;
	int ImgSize;
	int x;
	int y;
	int dx;
	int dy;
	TNTTILE *Tile;
	unsigned short *TileNo;
	int bmisize;
	//RECT rc;

	HBITMAP hbmp;

	HDC hdc;
	
	sct = (void *) vo->buff;
	Tile = (TNTTILE *) (vo->buff + sct->PtrTiles);
	TileNo = (void *) (vo->buff + sct->PtrSectionData);

	XSize = sct->SectionWidth * 32;
	YSize = sct->SectionHeight * 32;

	si.cbSize = sizeof(si);
  si.fMask = SIF_POS | SIF_RANGE;
  si.nPos = 0;
	si.nMin = 0;
	si.nMax = YSize - 1;
  SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

  si.cbSize = sizeof(si);
  si.fMask = SIF_POS | SIF_RANGE;
  si.nPos = 0;
	si.nMin = 0;
	si.nMax = XSize - 1;
  SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

	bmisize = sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD));
	bmi = GetMem(bmisize, TRUE); 

	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi->bmiHeader.biWidth = XSize;
	bmi->bmiHeader.biHeight = -YSize;
	bmi->bmiHeader.biPlanes = 1; 
  bmi->bmiHeader.biBitCount = 8;
  bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biSizeImage = 0; 

  nColorData = 256;

  // Allocate memory for color palette.
  lp = GetMem(sizeof(LOGPALETTE)+(nColorData * sizeof(PALETTEENTRY)), TRUE);

  lp->palVersion    = 0x300;      
  lp->palNumEntries = nColorData;

  // Load each color into the palette.
  for (i = 0; i < nColorData; i++ ) {
    lp->palPalEntry[i].peRed   = bmi->bmiColors[i].rgbRed = TAPalette[i].peRed;
    lp->palPalEntry[i].peGreen = bmi->bmiColors[i].rgbGreen = TAPalette[i].peGreen;
    lp->palPalEntry[i].peBlue  = bmi->bmiColors[i].rgbBlue = TAPalette[i].peBlue;
    lp->palPalEntry[i].peFlags = TAPalette[i].peFlags;
  }

  // Create the Palette.
  vo->hpal = CreatePalette(lp);



	// load bitmap bits
	ImgSize = XSize * YSize;
  bits = GetMem(ImgSize, TRUE);

	for (y = 0; y < sct->SectionHeight; y++) {
		for (x = 0; x < sct->SectionWidth; x++) {
			TileBits = (char *) (Tile + *TileNo);
			for (dy = 0; dy < 32; dy++) {
				outbits = bits + (((y * 32) + dy) * XSize) + (x * 32);
				for (dx = 0; dx < 32; dx++) {
					*outbits++ = *TileBits++;
				}
			}
			TileNo++;
		}
	}

	// get memory DC to hold bitmap
	hdc = GetDC(hwnd);
	vo->hdc = CreateCompatibleDC(hdc);

	// create bitmap object
	hbmp = CreateDIBitmap(hdc, &bmi->bmiHeader, CBM_INIT, bits, bmi, DIB_RGB_COLORS);

	// select the bitmap into the memory DC
	SelectObject(vo->hdc, hbmp);

	// cleanup
	DeleteObject(hbmp);
	ReleaseDC(hwnd, hdc);
	FreeMem(bits);
	FreeMem(bmi);
  FreeMem(lp);
	FreeMem(vo->buff);
	vo->buff = NULL;
	return TRUE;
}

long WndProcViewBmpCreate(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//wm_create
  LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;

	VIEWOPTION *vo = (void *) (lpcs->lpCreateParams);

  SetWindowLong(hwnd, GWL_USERDATA, (long) vo);

	switch (vo->option) {
		case VIEW_BMP :
  		LoadViewBmp(hwnd, vo);
			break;
		case VIEW_SCT :
  		return LoadViewSct(hwnd, vo);
    case VIEW_JPG :
      return LoadViewJpg(hwnd, vo);
		case VIEW_PCX :
			if (LoadViewPcx(hwnd, vo))
			  break;
			MessageBox(hwnd, "PCX format error", szAppTitle, MB_OK);
			return -1;
	}
	return 0;
}

long WndProcViewBmpSize(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
// wm_size
{
	int fwSizeType = wParam;      // resizing flag 
  int nWidth = LOWORD(lParam);  // width of client area 
  int nHeight = HIWORD(lParam); // height of client area

	if (fwSizeType == SIZE_MINIMIZED)
		return FALSE;

	ViewBmpAdjustScroll(hwnd, nWidth, nHeight);

	return FALSE;
}

long WndProcViewBmpPaint(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
// wm_paint
{
	VIEWOPTION *vo;
  PAINTSTRUCT  ps;
	int h;
	int w;
	SCROLLINFO si;
	int x;
	int y;

	vo = (void *) GetWindowLong(hwnd, GWL_USERDATA);

	BeginPaint(hwnd, &ps);

	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo(hwnd, SB_HORZ, &si);
	x = si.nPos;

	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo(hwnd, SB_VERT, &si);
	y = si.nPos;

  // Select palette and realize it.
  if (vo->hpal) {
    SelectPalette(ps.hdc, vo->hpal, FALSE );
  }
  RealizePalette(ps.hdc);

	// get width and height of repaint region
	w = ps.rcPaint.right - ps.rcPaint.left;
	h = ps.rcPaint.bottom - ps.rcPaint.top;

	// repaint
	BitBlt(ps.hdc, ps.rcPaint.left, ps.rcPaint.top, w, h,
		     vo->hdc, x+ps.rcPaint.left, y+ps.rcPaint.top, SRCCOPY);

	EndPaint(hwnd, &ps);
	return 0;
}

void ViewBmpScroll(HWND hwnd, int ScrollType, int nScrollCode)
{ 
  SCROLLINFO si;
	RECT rc;
	int PageLen;
	int OldPos;
	int ScrollStep = 1;

	GetClientRect(hwnd, &rc);

  if (ScrollType == SB_VERT)
 		PageLen = rc.bottom;
  else
    PageLen = rc.right;
  ScrollStep = 32;

  si.cbSize = sizeof(si);
  si.fMask = SIF_ALL;
  GetScrollInfo(hwnd, ScrollType, &si);
	OldPos = si.nPos;

  switch (nScrollCode) {
    case SB_LINEUP : si.nPos -= ScrollStep; break;
    case SB_LINEDOWN : si.nPos += ScrollStep; break;
    case SB_PAGEUP : si.nPos -= PageLen; break;
    case SB_PAGEDOWN : si.nPos += PageLen; break;
    case SB_TOP : si.nPos = 0; break;
    case SB_BOTTOM : si.nPos = si.nMax; break;
    case SB_THUMBTRACK : // fall thru
    case SB_THUMBPOSITION : si.nPos = si.nTrackPos; break;
		default :
			return;
  }
	if (si.nPos < si.nMin)
		si.nPos = si.nMin;
	else if ((si.nPos+PageLen-1) > si.nMax)
		si.nPos = si.nMax-PageLen+1;
  
	if (si.nPos == OldPos)
		return;

  si.cbSize = sizeof(si);
  si.fMask = SIF_POS;
  SetScrollInfo(hwnd, ScrollType, &si, TRUE);

	InvalidateRect(hwnd, NULL, FALSE);
}  

long WndProcViewBmpVScroll(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{ 
	ViewBmpScroll(hwnd, SB_VERT, LOWORD(wParam));
  return 0;
}  

long WndProcViewBmpHScroll(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
	ViewBmpScroll(hwnd, SB_HORZ, LOWORD(wParam));
	return 0;
}  

long WndProcViewBmpClose(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	VIEWOPTION *vo;
	vo = (void *) GetWindowLong(hwnd, GWL_USERDATA);

	if (vo->buff)
	  FreeMem(vo->buff);
	FreeMem(vo->Name);
	if (vo->hpal)
		DeleteObject(vo->hpal);
	if (vo->hdc)
		DeleteDC(vo->hdc);
	FreeMem(vo);

	DestroyWindow(hwnd);
  return FALSE;
}

LRESULT CALLBACK WndProcViewBmp(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
    case WM_CREATE :
      return WndProcViewBmpCreate(hwnd, message, wParam, lParam);
    case WM_VSCROLL : 
			return WndProcViewBmpVScroll(hwnd, message, wParam, lParam);
    case WM_HSCROLL : 
			return WndProcViewBmpHScroll(hwnd, message, wParam, lParam);
		case WM_SIZE :
			return WndProcViewBmpSize(hwnd, message, wParam, lParam);
		case WM_PAINT :
			return WndProcViewBmpPaint(hwnd, message, wParam, lParam);
    case WM_CLOSE :
      return WndProcViewBmpClose(hwnd, message, wParam, lParam);
  }   
  return DefWindowProc(hwnd, message, wParam, lParam);
}

void RegisterViewBmpClasses(void)
{
  WNDCLASSEX  wndclass;

	if (ViewBmpClassRegistered)
		return;

  //Main Window
  wndclass.cbSize        = sizeof(wndclass);
  wndclass.style         = 0; //CS_OWNDC;
  wndclass.lpfnWndProc   = WndProcViewBmp;
  wndclass.cbClsExtra    = 0;
  wndclass.cbWndExtra    = sizeof(int);
  wndclass.hInstance     = hThisInstance;
  wndclass.hIcon         = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_MAIN));
  wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
  wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
  wndclass.lpszMenuName  = NULL;
  wndclass.lpszClassName = ViewBmpWindowClass;
  wndclass.hIconSm       = LoadImage(hThisInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
  RegisterClassEx(&wndclass);

	ViewBmpClassRegistered = TRUE;
}

int ViewBmpFile(char *buff, int Length, char *Name, int option, HPIITEM *hi)
{
	HWND hwnd;
	VIEWOPTION *vo;
	HCURSOR SaveCursor;

	SaveCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

  vo = GetMem(sizeof(VIEWOPTION), TRUE);

	vo->Length = Length;
	vo->buff = buff;
	vo->Name = DupString(Name);
	vo->hpal = NULL;
	vo->hdc = NULL;
	vo->option = option;

	RegisterViewBmpClasses();

  hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, ViewBmpWindowClass, Name,
    WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    HWND_DESKTOP, NULL, hThisInstance, (LPVOID) vo);

	if (!hwnd)
		return FALSE;

  ShowWindow(hwnd, SW_SHOWNORMAL);

  SetCursor(SaveCursor);

	// buff is freed when the window closes
	return TRUE;
}

