//
//  Cob Viewer Code
//
// I never finished this.  Right now, it does nothing.
//
// If you want to make it decompile the COB files,
// the place to add the code is in the WndProcViewCobCreate function.
// See the notes.


#include <windows.h>
//#include <commctrl.h>
#include <richedit.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "resource.h"
#include "hpiview.h"
#include "viewstuf.h"

#define IDC_VIEWCOB 1

typedef struct _VIEWOPTION {
	int option;
	int Length;
	char *Name;
	char *buff;
} VIEWOPTION;

const char ViewCobWindowClass[] = "HPIViewCob";
int ViewCobClassRegistered = FALSE;

LRESULT WndProcViewCobCreate(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwndChild;
	int x;
  LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
	int CobLength = 0;
	LPSTR CobBuff = NULL;

	VIEWOPTION *vo = (void *) (lpcs->lpCreateParams);

  SetWindowLong(hwnd, GWL_USERDATA, (long) vo);


  // Decompile HERE.
	// vo->buff points to the COB code to decompile.
	// vo->Length is the number of bytes to decode.
	// 
  // Decompile to a big text buffer.
  // Delimit the lines with \r\n.
	// Set CobBuff to point to this big text buffer.
	// Set CobLength to the length of this big text buffer.
	
	if (hRichEd) {
    hwndChild = CreateWindowEx(0, "RichEdit", NULL,
      (WS_CHILD | ES_LEFT | WS_VISIBLE | ES_AUTOHSCROLL
       | WS_HSCROLL | WS_VSCROLL | ES_READONLY | ES_MULTILINE),
       0, 0, 0, 0,
       hwnd, (HMENU) IDC_VIEWCOB, hThisInstance, NULL);

    SendMessage(hwndChild, EM_EXLIMITTEXT, 0, CobLength+1024);
  }
  else {
    hwndChild = CreateWindowEx(0, "EDIT", NULL,
      (WS_CHILD | ES_LEFT | WS_VISIBLE
       | WS_HSCROLL | WS_VSCROLL | ES_READONLY | ES_MULTILINE),
       0, 0, 0, 0,
       hwnd, (HMENU) IDC_VIEWCOB, hThisInstance, NULL);

    SendMessage(hwndChild, EM_SETLIMITTEXT, 65500, 0);

    x = SendMessage(hwndChild, EM_GETLIMITTEXT, 0, 0);

    if (CobLength < x)
	    x = CobLength;

    if (x > 64150) {
  	  x = 64150;
	    CobBuff[x+1] = 0;
		  strcat(CobBuff, "\r\n***REMAINDER TRUNCATED***");
    }
  }

  SendMessage(hwndChild, WM_SETFONT, (WPARAM) GetStockObject(SYSTEM_FIXED_FONT), TRUE);

  if (!SendMessage(hwndChild, WM_SETTEXT, 0, (LPARAM) CobBuff))
	  SendMessage(hwndChild, WM_SETTEXT, 0, (LPARAM) "Too much data!");

	FreeMem(CobBuff);

	FreeMem(vo->buff);
	vo->buff = NULL;

	return 0;
}

LRESULT WndProcViewCobSize(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
// wm_size
{
	int fwSizeType = wParam;      // resizing flag 
  int nWidth = LOWORD(lParam);  // width of client area 
  int nHeight = HIWORD(lParam); // height of client area

	if (fwSizeType == SIZE_MINIMIZED)
		return FALSE;

  MoveWindow(GetDlgItem(hwnd, IDC_VIEWCOB), 0, 0, nWidth, nHeight, TRUE);

	return FALSE;
}

LRESULT WndProcViewCobClose(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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

LRESULT CALLBACK WndProcViewCob(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
    case WM_CREATE :
      return WndProcViewCobCreate(hwnd, message, wParam, lParam);
		case WM_SIZE :
			return WndProcViewCobSize(hwnd, message, wParam, lParam);
    case WM_CLOSE :
      return WndProcViewCobClose(hwnd, message, wParam, lParam);
  }   
  return DefWindowProc(hwnd, message, wParam, lParam);
}

void RegisterViewCobClasses(void)
{
  WNDCLASSEX  wndclass;

	if (ViewCobClassRegistered)
		return;

  //Main Window
  wndclass.cbSize        = sizeof(wndclass);
  wndclass.style         = 0; //CS_OWNDC;
  wndclass.lpfnWndProc   = WndProcViewCob;
  wndclass.cbClsExtra    = 0;
  wndclass.cbWndExtra    = sizeof(int);
  wndclass.hInstance     = hThisInstance;
  wndclass.hIcon         = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_MAIN));
  wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
  wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
  wndclass.lpszMenuName  = NULL;
  wndclass.lpszClassName = ViewCobWindowClass;
  wndclass.hIconSm       = LoadImage(hThisInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
  RegisterClassEx(&wndclass);

	ViewCobClassRegistered = TRUE;
}

int ViewCobFile(char *buff, int Length, char *Name, int option, HPIITEM *hi)
{
	HWND hwnd;
	VIEWOPTION *vo;

	vo = GetMem(sizeof(VIEWOPTION), TRUE);

	vo->Length = Length;
	vo->buff = buff;
	vo->Name = DupString(Name);

	RegisterViewCobClasses();

  hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, ViewCobWindowClass, Name,
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    HWND_DESKTOP, NULL, hThisInstance, (LPVOID) vo);

  ShowWindow(hwnd, SW_SHOWNORMAL);

	// buff is freed when the window is created
	return TRUE;
}

