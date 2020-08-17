//
//  TNT Viewer Code
//

#include <windows.h>
//#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "resource.h"
#include "hpiview.h"
#include "viewstuf.h"
#include "palette.h"

#pragma pack(1)

typedef struct _VIEWOPTION {
	int option;
	int Length;
	char *Name;
	char *buff;
	unsigned short *MapData;
	int *MapAttr;
	int Tiles;
	char **Tile;
	BITMAPINFO *bmi;
	HPALETTE hpal;
	int XSize;
	int YSize;
} VIEWOPTION;

#pragma pack()

const char ViewTntWindowClass[] = "HPIViewTnt";
int ViewTntClassRegistered = FALSE;

void ViewTntAdjustScroll(HWND hwnd, int nWidth, int nHeight)
{
  RECT rc;
  SCROLLINFO si;
	int h;
	int w;
  
  GetClientRect(hwnd, &rc);
  
  w = rc.right - rc.left;
	if (w % 32)
		w = (w / 32) + 1;
	else
		w = w / 32;

  h = rc.bottom - rc.top;
	if (h % 32)
		h = (h / 32) + 1;
	else
		h = h / 32;

  si.cbSize = sizeof(si);
  si.fMask = SIF_PAGE;
  si.nPage = h;
  SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

  si.cbSize = sizeof(si);
  si.fMask = SIF_PAGE;
  si.nPage = w;
  SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

	InvalidateRect(hwnd, NULL, FALSE);

}

int LoadViewTnt(HWND hwnd, VIEWOPTION *vo)
{
	BITMAPINFO *bmi;
	int nColorData;
	LOGPALETTE *lp;
	int i;																		
	SCROLLINFO si;
	int XSize;
	int YSize;
	TNTHEADER *tnt;
	int bmisize;
	char *MapTiles;
	
	tnt = (void *) vo->buff;
	if (tnt->IDVersion != 0x00002000)
		return FALSE;

	XSize = tnt->Width / 2;
	YSize = tnt->Height / 2;
	vo->XSize = XSize;
	vo->YSize = YSize;

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

	vo->MapData = (void *) (vo->buff+tnt->PtrMapData);
	vo->MapAttr = (void *) (vo->buff+tnt->PtrMapAttr);
	MapTiles = (void *) (vo->buff+tnt->PtrTileGfx);
	vo->Tiles = tnt->Tiles;
	vo->Tile = GetMem(vo->Tiles * sizeof(char *), TRUE);
	
	bmisize = sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD));
	bmi = GetMem(bmisize, TRUE); 

	vo->bmi = bmi;

	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi->bmiHeader.biWidth = 32;
	bmi->bmiHeader.biHeight = -32;
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

  for (i = 0; i < tnt->Tiles; i++) {
	  vo->Tile[i] = MapTiles;
		MapTiles += 1024;
	}

  FreeMem(lp);
  return TRUE;
}

long WndProcViewTntCreate(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//wm_create
  LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;

	VIEWOPTION *vo = (void *) (lpcs->lpCreateParams);

  SetWindowLong(hwnd, GWL_USERDATA, (long) vo);

	if (!LoadViewTnt(hwnd, vo))
		return -1;

	return 0;
}

long WndProcViewTntSize(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
// wm_size
{
	int fwSizeType = wParam;      // resizing flag 
  int nWidth = LOWORD(lParam);  // width of client area 
  int nHeight = HIWORD(lParam); // height of client area

	if (fwSizeType == SIZE_MINIMIZED)
		return FALSE;

	ViewTntAdjustScroll(hwnd, nWidth, nHeight);

	return FALSE;
}

long WndProcViewTntPaint(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
// wm_paint
{
	VIEWOPTION *vo;
  PAINTSTRUCT  ps;
	SCROLLINFO si;
	int x;
	int y;
	int x1;
	int x2;
	int y1;
	int y2;
	int dx;
	int dy;
	unsigned short *TNo;

	vo = (void *) GetWindowLong(hwnd, GWL_USERDATA);

	BeginPaint(hwnd, &ps);

	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	GetScrollInfo(hwnd, SB_HORZ, &si);
	x1 = si.nPos;
	x2 = si.nPos + si.nPage; 

	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	GetScrollInfo(hwnd, SB_VERT, &si);
	y1 = si.nPos;
	y2 = si.nPos + si.nPage; 


  // Select palette and realize it.
  SelectPalette(ps.hdc, vo->hpal, FALSE );
  RealizePalette(ps.hdc);

	dy = 0;
	// repaint
	for (y = y1; y < y2; y++) {
		dx = 0;
		TNo = vo->MapData + (y * vo->XSize) + x1;
		for (x = x1; x < x2; x++) {
			SetDIBitsToDevice(ps.hdc, dx, dy, 32, 32, 
				0, 0, 0, 32, 
				vo->Tile[*TNo], vo->bmi, DIB_RGB_COLORS);

			dx += 32;
			TNo++;
		}
		dy += 32;
	}

	EndPaint(hwnd, &ps);
	return 0;
}

void ViewTntScroll(HWND hwnd, int ScrollType, int nScrollCode)
{ 
  SCROLLINFO si;
	int OldPos;
	int ScrollStep = 1;

  si.cbSize = sizeof(si);
  si.fMask = SIF_ALL;
  GetScrollInfo(hwnd, ScrollType, &si);
	OldPos = si.nPos;

  switch (nScrollCode) {
    case SB_LINEUP : si.nPos -= ScrollStep; break;
    case SB_LINEDOWN : si.nPos += ScrollStep; break;
    case SB_PAGEUP : si.nPos -= si.nPage; break;
    case SB_PAGEDOWN : si.nPos += si.nPage; break;
    case SB_TOP : si.nPos = 0; break;
    case SB_BOTTOM : si.nPos = si.nMax; break;
    case SB_THUMBTRACK : // fall thru
    case SB_THUMBPOSITION : si.nPos = si.nTrackPos; break;
		default :
			return;
  }
	if (si.nPos < si.nMin)
		si.nPos = si.nMin;
	else if ((si.nPos + (int) si.nPage - 1) > si.nMax)
		si.nPos = si.nMax-si.nPage+1;
  
	if (si.nPos == OldPos)
		return;

  si.cbSize = sizeof(si);
  si.fMask = SIF_POS;
  SetScrollInfo(hwnd, ScrollType, &si, TRUE);

	InvalidateRect(hwnd, NULL, FALSE);
}  

long WndProcViewTntVScroll(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{ 
	ViewTntScroll(hwnd, SB_VERT, LOWORD(wParam));
  return 0;
}  

long WndProcViewTntHScroll(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
	ViewTntScroll(hwnd, SB_HORZ, LOWORD(wParam));
	return 0;
}  

long WndProcViewTntClose(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	VIEWOPTION *vo;
	vo = (void *) GetWindowLong(hwnd, GWL_USERDATA);

	if (vo->Name)
	  FreeMem(vo->Name);
	if (vo->buff)
	  FreeMem(vo->buff);
	if (vo->hpal)
		DeleteObject(vo->hpal);
	if (vo->bmi)
		FreeMem(vo->bmi);
	if (vo->Tile)
		FreeMem(vo->Tile);
	FreeMem(vo);

	DestroyWindow(hwnd);
  return FALSE;
}

LRESULT CALLBACK WndProcViewTnt(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
    case WM_CREATE :
      return WndProcViewTntCreate(hwnd, message, wParam, lParam);
    case WM_VSCROLL : 
			return WndProcViewTntVScroll(hwnd, message, wParam, lParam);
    case WM_HSCROLL : 
			return WndProcViewTntHScroll(hwnd, message, wParam, lParam);
		case WM_SIZE :
			return WndProcViewTntSize(hwnd, message, wParam, lParam);
		case WM_PAINT :
			return WndProcViewTntPaint(hwnd, message, wParam, lParam);
    case WM_CLOSE :
      return WndProcViewTntClose(hwnd, message, wParam, lParam);
  }   
  return DefWindowProc(hwnd, message, wParam, lParam);
}

void RegisterViewTntClasses(void)
{
  WNDCLASSEX  wndclass;

	if (ViewTntClassRegistered)
		return;

  //Main Window
  wndclass.cbSize        = sizeof(wndclass);
  wndclass.style         = 0; //CS_OWNDC;
  wndclass.lpfnWndProc   = WndProcViewTnt;
  wndclass.cbClsExtra    = 0;
  wndclass.cbWndExtra    = sizeof(int);
  wndclass.hInstance     = hThisInstance;
  wndclass.hIcon         = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_MAIN));
  wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
  wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
  wndclass.lpszMenuName  = NULL;
  wndclass.lpszClassName = ViewTntWindowClass;
  wndclass.hIconSm       = LoadImage(hThisInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
  RegisterClassEx(&wndclass);

	ViewTntClassRegistered = TRUE;
}

int ViewTntFile(char *buff, int Length, char *Name, int option, HPIITEM *hi)
{
	HWND hwnd;
	VIEWOPTION *vo;

	vo = GetMem(sizeof(VIEWOPTION), TRUE);

	vo->Length = Length;
	vo->buff = buff;
	vo->Name = DupString(Name);
	vo->option = option;

	RegisterViewTntClasses();

  hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, ViewTntWindowClass, Name,
    WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    HWND_DESKTOP, NULL, hThisInstance, (LPVOID) vo);

	if (!hwnd)
		return FALSE;

  ShowWindow(hwnd, SW_SHOWNORMAL);

	// buff is freed when the window closes
	return TRUE;
}

