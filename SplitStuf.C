#include <windows.h>
#include "resource.h"
#include "splitstuf.h"

/* begin split stuff */

#define SPLIT_DRAGGING 0
#define SPLIT_STARTX (sizeof(int))
#define SPLIT_STARTY (2 * sizeof(int))
#define SPLIT_TYPE (3 * sizeof(int))
#define SPLIT_ID (4 * sizeof(int))

int SplitRegistered = FALSE;
const char szSplitVClass[] = "SplitVWindow";
const char szSplitHClass[] = "SplitHWindow";

void ConvertToClient(HWND hwnd, RECT *rc)
{
	POINT pt;

	pt.x = rc->left;
	pt.y = rc->top;
	ScreenToClient(hwnd, &pt);
	rc->left = pt.x;
	rc->top = pt.y;

	pt.x = rc->right;
	pt.y = rc->bottom;
	ScreenToClient(hwnd, &pt);
	rc->right = pt.x;
	rc->bottom = pt.y;

}

LRESULT CALLBACK WndProcSplit(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
  int wmax;
  int hmax;
	int xPos;
	int yPos;

  switch (message) {
		case WM_LBUTTONDOWN :
      xPos = (short) LOWORD(lParam);  // horizontal position of cursor 
      yPos = (short) HIWORD(lParam);  // vertical position of cursor
			SetWindowLong(hwnd, SPLIT_DRAGGING, TRUE);
			SetWindowLong(hwnd, SPLIT_STARTX, xPos);
			SetWindowLong(hwnd, SPLIT_STARTY, yPos);
			SetCapture(hwnd);
			return 0;
		case WM_MOUSEMOVE :
			if (!GetWindowLong(hwnd, SPLIT_DRAGGING))
  			return 0;
      GetClientRect(GetParent(hwnd), &rc);
      wmax = rc.right - rc.left - 10 - SPLITWIDTH;
      hmax = rc.bottom - rc.top - 10 - SPLITWIDTH;
			GetWindowRect(hwnd, &rc);
			if (GetWindowLong(hwnd, SPLIT_TYPE)) {
        xPos = (short) LOWORD(lParam);
			  rc.left += (xPos - GetWindowLong(hwnd, SPLIT_STARTX));
    	  ConvertToClient(GetParent(hwnd), &rc);
        if (rc.left < 10)
          rc.left = 10;
        else if (rc.left > wmax)
          rc.left = wmax;
        MoveWindow(hwnd, rc.left, rc.top, SPLITWIDTH, rc.bottom-rc.top, FALSE); 
			}
			else {
        yPos = (short) HIWORD(lParam);
			  rc.top += (yPos - GetWindowLong(hwnd, SPLIT_STARTY));
    	  ConvertToClient(GetParent(hwnd), &rc);
        if (rc.top < 10)
          rc.top = 10;
        else if (rc.top > wmax)
          rc.top = wmax;
        MoveWindow(hwnd, rc.left, rc.top, rc.right-rc.left, SPLITWIDTH, FALSE); 
			}
    	SendMessage(GetParent(hwnd), WM_COMMAND, GetWindowLong(hwnd, SPLIT_ID), (LPARAM) hwnd);
			return 0;

		case WM_LBUTTONUP :
			if (GetWindowLong(hwnd, SPLIT_DRAGGING)) {
			  SetWindowLong(hwnd, SPLIT_DRAGGING, FALSE);
			  ReleaseCapture();
			}
			return 0;
  }  
  return DefWindowProc(hwnd, message, wParam, lParam);
}

void RegisterSplitClasses(HINSTANCE hinst)
{
  WNDCLASSEX wc;

  if (SplitRegistered)
    return;

	wc.cbSize = sizeof(wc);
  wc.style         = 0;
  wc.lpfnWndProc   = WndProcSplit;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 5 * sizeof(int);
  wc.hInstance     = hinst;
  wc.hIcon         = NULL;
  wc.hCursor       = LoadCursor(hinst, MAKEINTRESOURCE(IDC_VSPLIT));
  wc.hbrBackground = GetStockObject(NULL_BRUSH);
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = szSplitVClass;
	wc.hIconSm = NULL;
  RegisterClassEx(&wc);

	wc.cbSize = sizeof(wc);
  wc.style         = 0;
  wc.lpfnWndProc   = WndProcSplit;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 5 * sizeof(int);
  wc.hInstance     = hinst;
  wc.hIcon         = NULL;
  wc.hCursor       = LoadCursor(hinst, MAKEINTRESOURCE(IDC_HSPLIT));
  wc.hbrBackground = GetStockObject(NULL_BRUSH);
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = szSplitHClass;
	wc.hIconSm = NULL;
  RegisterClassEx(&wc);

  SplitRegistered = TRUE;
}

HWND CreateSplit(HINSTANCE hinst, HWND hwndParent, HWND hwndSibling, int SplitType, int id)
{
	HWND hwnd;
  RECT rc;
  int x;
  int y;
  int size;

  GetWindowRect(hwndSibling, &rc);
  ConvertToClient(hwndParent, &rc);

  if (!SplitRegistered)
    RegisterSplitClasses(hinst);

  if (SplitType) {
    x = rc.right + 1;
    y = rc.top;
    size = rc.bottom - rc.top;
	  hwnd = CreateWindowEx(WS_EX_TRANSPARENT, szSplitVClass, "", WS_CHILD | WS_VISIBLE, 
		          x, y, SPLITWIDTH, size, hwndParent, (HMENU) id, hinst, NULL);
	}
	else {
    x = rc.left;
    y = rc.bottom + 1;
    size = rc.right - rc.left;
	  hwnd = CreateWindowEx(WS_EX_TRANSPARENT, szSplitHClass, "", WS_CHILD | WS_VISIBLE, 
		          x, y, size, SPLITWIDTH, hwndParent, (HMENU) id, hinst, NULL);
	}
	SetWindowLong(hwnd, SPLIT_TYPE, SplitType);
	SetWindowLong(hwnd, SPLIT_ID, id);
	return hwnd;
}

void AdjustHSplit(HWND hwndCtl, HWND hwndRight, HWND hwndLeft)
{
  RECT rc;
  int x;
  int w;
  int h;

  GetWindowRect(hwndCtl, &rc);
  ConvertToClient(GetParent(hwndCtl), &rc);
  w = rc.left - 1;

  GetWindowRect(hwndRight, &rc);
  ConvertToClient(GetParent(hwndCtl), &rc);
  h = rc.bottom - rc.top;
  MoveWindow(hwndRight, 0, 0, w, h, TRUE);

  GetWindowRect(hwndLeft, &rc);
  ConvertToClient(GetParent(hwndCtl), &rc);
  x = w + SPLITWIDTH;
  h = rc.bottom - rc.top;
  w = rc.right - x;
  MoveWindow(hwndLeft, x, 0, w, h, TRUE);
}

/* end split stuff */
