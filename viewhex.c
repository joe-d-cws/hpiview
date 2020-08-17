//
//  Hex Viewer Code
//

#include <windows.h>
//#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "resource.h"
#include "hpiview.h"
#include "viewstuf.h"

#pragma pack(1)

typedef struct _VIEWOPTION {
	int option;
	int Length;
	char *Name;
	char *buff;
} VIEWOPTION;

#pragma pack()

char LegendLine[] = "Address    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  ---ASCII Data---";
const char ViewHexWindowClass[] = "HPIViewHex";
int ViewHexClassRegistered = FALSE;

int ViewHexLinesPerPage(HWND hwnd)
{
	HDC hdc;
	TEXTMETRIC tm;
	RECT rc;

	GetClientRect(hwnd, &rc);
	hdc = GetDC(hwnd);
	SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT)); 

	GetTextMetrics(hdc, &tm);
	ReleaseDC(hwnd, hdc);

  //cxChar = tm.tmAveCharWidth;
  return (rc.bottom - rc.top) / (tm.tmHeight + tm.tmExternalLeading);
}

void ViewHexAdjustScroll(HWND hwnd, int nWidth, int nHeight)
{
  SCROLLINFO si;
	int h;
  
  h = ViewHexLinesPerPage(hwnd);

  si.cbSize = sizeof(si);
  si.fMask = SIF_ALL;
	GetScrollInfo(hwnd, SB_VERT, &si);
  
  if ((si.nPos+h) > si.nMax) {
    si.nPos = si.nMax - h + 1;
		InvalidateRect(hwnd, NULL, FALSE);
	}

  si.cbSize = sizeof(si);
  si.fMask = SIF_PAGE | SIF_POS;
  si.nPage = h;
  SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
}

void PaintViewHex(HWND hwnd, VIEWOPTION *vo, PAINTSTRUCT *ps)
{
	SCROLLINFO si;
	TEXTMETRIC tm;
	RECT rc;
	int cyChar;
	int cyClient;
	int cxClient;
  int fp;
  char line[100];
  char hexline[100];
  char ascline[100];
  char num[25];
  int count;
  int bcount;
  int sline;
	int ScreenLines;
	int fpMax;

/*
xxxxxxxx  xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx  xxxxxxxxxxxxxxxx
*/
	SelectObject(ps->hdc, GetStockObject(ANSI_FIXED_FONT)); 
	GetTextMetrics(ps->hdc, &tm);
  //cxChar = tm.tmAveCharWidth;
  cyChar = tm.tmHeight + tm.tmExternalLeading;

	GetClientRect(hwnd, &rc);
  cyClient = rc.bottom - rc.top;
	cxClient = rc.right - rc.left;

  ScreenLines = (cyClient / cyChar) - 1;
  if (((ScreenLines+1) * cyChar) != cyClient)
    ScreenLines++;

	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo(hwnd, SB_VERT, &si);

	fp = si.nPos * 16;

  sline = cyChar;
  ascline[0] = 0;
  TextOut(ps->hdc, 0, 0, LegendLine, strlen(LegendLine));
  for (count = 0; count < ScreenLines; count++) {
		if (fp > vo->Length) {
			PatBlt(ps->hdc, 0, sline, cxClient, sline+cyChar-1, PATCOPY); 
      //break;
		}
    else if ((sline + cyChar) >= ps->rcPaint.top) {
      sprintf(line, "%08X ", fp);
      hexline[0] = 0;
			fpMax = fp+16;
      for (bcount = fp; bcount < fpMax; bcount++)
				if (bcount < vo->Length) {
          if (vo->buff[bcount] > 127)
            ascline[bcount-fp] = '.';  //(ShowHi ? buff[bcount] : '.');
          else if (vo->buff[bcount] < ' ')
            ascline[bcount-fp] = '.';  //(ShowLo ? (buff[bcount] ? buff[bcount] : ' ') : '.');
          else
            ascline[bcount-fp] = vo->buff[bcount];
          sprintf(num, " %02X", (DWORD) (unsigned char) vo->buff[bcount]);
          strcat(hexline, num);
        }
        else {
          strcat(hexline, "   ");
          ascline[bcount-fp] = ' ';
        }
      ascline[16] = 0;
      strcat(line, hexline);
      strcat(line, "  ");
      strcat(line, ascline);
      TextOut(ps->hdc, 0, sline, line, strlen(line));
    }
    sline += cyChar;
    fp += 16;
  }
}

LRESULT WndProcViewHexCreate(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO si;
  LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;

	VIEWOPTION *vo = (void *) (lpcs->lpCreateParams);

  SetWindowLong(hwnd, GWL_USERDATA, (long) vo);

	si.cbSize = sizeof(si);
  si.fMask = SIF_POS | SIF_RANGE;
  si.nPos = 0;
	si.nMin = 0;
	if (vo->Length & 0x0F)
	  si.nMax = (vo->Length / 16) + 1;
	else
		si.nMax = (vo->Length / 16);
  SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

	return 0;
}

LRESULT WndProcViewHexSize(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
// wm_size
{
	int fwSizeType = wParam;      // resizing flag 
  int nWidth = LOWORD(lParam);  // width of client area 
  int nHeight = HIWORD(lParam); // height of client area

	if (fwSizeType == SIZE_MINIMIZED)
		return FALSE;

  ViewHexAdjustScroll(hwnd, nWidth, nHeight);

	return FALSE;
}

LRESULT WndProcViewHexPaint(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
// wm_paint
{
	VIEWOPTION *vo;
  PAINTSTRUCT  ps;

	vo = (void *) GetWindowLong(hwnd, GWL_USERDATA);

	BeginPaint(hwnd, &ps);

	PaintViewHex(hwnd, vo, &ps);

	EndPaint(hwnd, &ps);
	return 0;
}

void ViewHexScroll(HWND hwnd, int ScrollType, int nScrollCode)
{ 
  SCROLLINFO si;
	RECT rc;
	int PageLen;
	int OldPos;
	int ScrollStep = 1;

	GetClientRect(hwnd, &rc);

	ScrollStep = 1;
 	PageLen = ViewHexLinesPerPage(hwnd);

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

long WndProcViewHexVScroll(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{ 
	ViewHexScroll(hwnd, SB_VERT, LOWORD(wParam));
  return 0;
}  

LRESULT WndProcViewHexClose(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	VIEWOPTION *vo;
	vo = (void *) GetWindowLong(hwnd, GWL_USERDATA);

	if (vo->buff)
	  FreeMem(vo->buff);
	FreeMem(vo->Name);
	FreeMem(vo);

	DestroyWindow(hwnd);
  return FALSE;
}

LRESULT CALLBACK WndProcViewHex(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
    case WM_CREATE :
      return WndProcViewHexCreate(hwnd, message, wParam, lParam);
    case WM_VSCROLL : 
			return WndProcViewHexVScroll(hwnd, message, wParam, lParam);
		case WM_SIZE :
			return WndProcViewHexSize(hwnd, message, wParam, lParam);
		case WM_PAINT :
			return WndProcViewHexPaint(hwnd, message, wParam, lParam);
    case WM_CLOSE :
      return WndProcViewHexClose(hwnd, message, wParam, lParam);
  }   
  return DefWindowProc(hwnd, message, wParam, lParam);
}

void RegisterViewHexClasses(void)
{
  WNDCLASSEX  wndclass;

	if (ViewHexClassRegistered)
		return;

  //Main Window
  wndclass.cbSize        = sizeof(wndclass);
  wndclass.style         = 0; //CS_OWNDC;
  wndclass.lpfnWndProc   = WndProcViewHex;
  wndclass.cbClsExtra    = 0;
  wndclass.cbWndExtra    = sizeof(int);
  wndclass.hInstance     = hThisInstance;
  wndclass.hIcon         = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_MAIN));
  wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
  wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
  wndclass.lpszMenuName  = NULL;
  wndclass.lpszClassName = ViewHexWindowClass;
  wndclass.hIconSm       = LoadImage(hThisInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
  RegisterClassEx(&wndclass);

	ViewHexClassRegistered = TRUE;
}

int ViewHexFile(char *buff, int Length, char *Name, int option, HPIITEM *hi)
{
	HWND hwnd;
	VIEWOPTION *vo;

	vo = GetMem(sizeof(VIEWOPTION), TRUE);

	vo->Length = Length;
	vo->buff = buff;
	vo->Name = DupString(Name);
	vo->option = option;

	RegisterViewHexClasses();

  hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, ViewHexWindowClass, Name,
    WS_OVERLAPPEDWINDOW | WS_VSCROLL,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    HWND_DESKTOP, NULL, hThisInstance, (LPVOID) vo);

  ShowWindow(hwnd, SW_SHOWNORMAL);

	// buff is freed when the window closes
	return TRUE;
}

