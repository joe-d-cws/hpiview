//
//  Text Viewer Code
//

#include <windows.h>
//#include <commctrl.h>
#include <richedit.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "resource.h"
#include "hpiview.h"
#include "viewstuf.h"

#define IDC_VIEWTEXT 1

typedef struct _VIEWOPTION {
	int option;
	int Length;
	char *Name;
	char *buff;
} VIEWOPTION;

const char ViewTextWindowClass[] = "HPIViewText";
int ViewTextClassRegistered = FALSE;

LRESULT WndProcViewTextCreate(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwndChild;
	int x;
  LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;

	VIEWOPTION *vo = (void *) (lpcs->lpCreateParams);

  SetWindowLong(hwnd, GWL_USERDATA, (long) vo);

  if (hRichEd) {
    hwndChild = CreateWindowEx(0, "RichEdit", NULL,
      (WS_CHILD | ES_LEFT | WS_VISIBLE | ES_AUTOHSCROLL
       | WS_HSCROLL | WS_VSCROLL | ES_READONLY | ES_MULTILINE),
       0, 0, 0, 0,
       hwnd, (HMENU) IDC_VIEWTEXT, hThisInstance, NULL);

    SendMessage(hwndChild, EM_EXLIMITTEXT, 0, vo->Length+1024);
  }
  else {
    hwndChild = CreateWindowEx(0, "EDIT", NULL,
      (WS_CHILD | ES_LEFT | WS_VISIBLE
       | WS_HSCROLL | WS_VSCROLL | ES_READONLY | ES_MULTILINE),
       0, 0, 0, 0,
       hwnd, (HMENU) IDC_VIEWTEXT, hThisInstance, NULL);

    SendMessage(hwndChild, EM_SETLIMITTEXT, 65500, 0);

    x = SendMessage(hwndChild, EM_GETLIMITTEXT, 0, 0);

    if (vo->Length < x)
	    x = vo->Length;

    if (x > 64150) {
  	  x = 64150;
	    vo->buff[x+1] = 0;
		  strcat(vo->buff, "\r\n***REMAINDER TRUNCATED***");
    }
  }

  SendMessage(hwndChild, WM_SETFONT, (WPARAM) GetStockObject(SYSTEM_FIXED_FONT), TRUE);

  if (!SendMessage(hwndChild, WM_SETTEXT, 0, (LPARAM) vo->buff))
	  SendMessage(hwndChild, WM_SETTEXT, 0, (LPARAM) "Too much data!");

	FreeMem(vo->buff);
	vo->buff = NULL;

	return 0;
}

LRESULT WndProcViewTextSize(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
// wm_size
{
	int fwSizeType = wParam;      // resizing flag 
  int nWidth = LOWORD(lParam);  // width of client area 
  int nHeight = HIWORD(lParam); // height of client area

	if (fwSizeType == SIZE_MINIMIZED)
		return FALSE;

  MoveWindow(GetDlgItem(hwnd, IDC_VIEWTEXT), 0, 0, nWidth, nHeight, TRUE);

	return FALSE;
}

LRESULT WndProcViewTextClose(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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

LRESULT CALLBACK WndProcViewText(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
    case WM_CREATE :
      return WndProcViewTextCreate(hwnd, message, wParam, lParam);
		case WM_SIZE :
			return WndProcViewTextSize(hwnd, message, wParam, lParam);
    case WM_CLOSE :
      return WndProcViewTextClose(hwnd, message, wParam, lParam);
  }   
  return DefWindowProc(hwnd, message, wParam, lParam);
}

void RegisterViewTextClasses(void)
{
  WNDCLASSEX  wndclass;

	if (ViewTextClassRegistered)
		return;

  //Main Window
  wndclass.cbSize        = sizeof(wndclass);
  wndclass.style         = 0; //CS_OWNDC;
  wndclass.lpfnWndProc   = WndProcViewText;
  wndclass.cbClsExtra    = 0;
  wndclass.cbWndExtra    = sizeof(int);
  wndclass.hInstance     = hThisInstance;
  wndclass.hIcon         = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_MAIN));
  wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
  wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
  wndclass.lpszMenuName  = NULL;
  wndclass.lpszClassName = ViewTextWindowClass;
  wndclass.hIconSm       = LoadImage(hThisInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
  RegisterClassEx(&wndclass);

	ViewTextClassRegistered = TRUE;
}

int ViewTextFile(char *buff, int Length, char *Name, int option, HPIITEM *hi)
{
	HWND hwnd;
	VIEWOPTION *vo;

	vo = GetMem(sizeof(VIEWOPTION), TRUE);

	vo->Length = Length;
	vo->buff = buff;
	vo->Name = DupString(Name);

	RegisterViewTextClasses();

  hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, ViewTextWindowClass, Name,
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    HWND_DESKTOP, NULL, hThisInstance, (LPVOID) vo);

  ShowWindow(hwnd, SW_SHOWNORMAL);

	// buff is freed when the window is created
	return TRUE;
}

