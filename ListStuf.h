//
//  Routines to draw highlight bar across a list view
//

//BOOL CalcStringEllipsis(HDC hdc, LPTSTR lpszString, int cchMax, int uColWidth);

//void DrawItemColumn(HDC hdc, LPTSTR lpsz, LPRECT prcClip, int LVCAlign);

void DrawListViewItem(LPDRAWITEMSTRUCT lpDrawItem);

LRESULT WndProcMainDrawItem(HWND hwnd, UINT message, WPARAM wParam, LONG lParam);

LRESULT WndProcMainMeasureItem(HWND hwnd, UINT message, WPARAM wParam, LONG lParam);
