//
//  Routines to draw highlight bar across a list view
//

#include <windows.h>
#include <commctrl.h>

#include "liststuf.h"

//
//  FUNCTION:   CalcStringEllipsis(HDC, LPTSTR, int, UINT)
//
//  PURPOSE:    Determines whether the specified string is too wide to fit in
//              an allotted space, and if not truncates the string and adds some
//              points of ellipsis to the end of the string.
//
//  PARAMETERS:
//      hdc        - Handle of the DC the string will be drawn on.
//      lpszString - Pointer to the string to verify
//      cchMax     - Maximum size of the lpszString buffer.
//      uColWidth  - Width of the space in pixels to fit the string into.
//
//  RETURN VALUE:
//      Returns TRUE if the string needed to be truncated, or FALSE if it fit
//      into uColWidth.
//

BOOL CalcStringEllipsis(HDC hdc, LPTSTR lpszString, int cchMax, int uColWidth)
{
  const char szEllipsis[] = "...";
  int    cbEllipsis = 3;   // lstrlen(szEllipsis);
  SIZE   sizeString;
  SIZE   sizeEllipsis;
  int    cbString;
  LPSTR  lpszTemp;
  BOOL   fSuccess = FALSE;

  // Adjust the column width to take into account the edges
  uColWidth -= 4;

  // Allocate a string for us to work with.  This way we can mangle the
  // string and still preserve the return value
  lpszTemp = GlobalAlloc(GPTR, cchMax);
  if (!lpszTemp)
    return FALSE;

  strcpy(lpszTemp, lpszString);

  // Get the width of the string in pixels
  cbString = strlen(lpszTemp);
  GetTextExtentPoint32(hdc, lpszTemp, cbString, &sizeString);

  // If the width of the string is greater than the column width shave
  // the string and add the ellipsis
  if (sizeString.cx > uColWidth) {
    GetTextExtentPoint32(hdc, szEllipsis, cbEllipsis, &sizeEllipsis);

    while (cbString > 0) {
      lpszTemp[--cbString] = 0;
      GetTextExtentPoint32(hdc, lpszTemp, cbString, &sizeString);

      if ((sizeString.cx + sizeEllipsis.cx) <= uColWidth) {
        // The string with the ellipsis finally fits, now make sure
        // there is enough room in the string for the ellipsis
        if (cchMax >= (cbString + cbEllipsis)) {
          // Concatenate the two strings and break out of the loop
          strcat(lpszTemp, szEllipsis);
          strcpy(lpszString, lpszTemp);
          fSuccess = TRUE;
          break;
        }
      }
    }
  }
  else {
    // No need to do anything, everything fits great.
    fSuccess = TRUE;
  }
  // Free the memory
  GlobalFree(lpszTemp);
  return (fSuccess);
}

//
//  FUNCTION:   DrawItemColumn(HDC, LPTSTR, LPRECT)
//
//  PURPOSE:    Draws the text for one of the columns in the list view.
//
//  PARAMETERS:
//      hdc     - Handle of the DC to draw the text into.
//      lpsz    - String to draw.
//      prcClip - Rectangle to clip the string to.
//

void DrawItemColumn(HDC hdc, LPTSTR lpsz, int texth, LPRECT prcClip, int LVCAlign)
{
  char szString[256];
  int x;
  int y;
  int w;
	int h;

  // Check to see if the string fits in the clip rect.  If not, truncate
  // the string and add "...".
  strcpy(szString, lpsz);
  CalcStringEllipsis(hdc, szString, sizeof(szString), prcClip->right - prcClip->left);

  // adjust for alignment
  if (LVCAlign & LVCFMT_CENTER) {
    SetTextAlign(hdc, TA_CENTER);
    w = (prcClip->right - prcClip->left) / 2;
    x = prcClip->left + w;
  }
  else if (LVCAlign & LVCFMT_RIGHT) {
    SetTextAlign(hdc, TA_RIGHT);
    x = prcClip->right - 6;
  }
  else { // left
    x = prcClip->left + 2;
  }
	h = (prcClip->bottom - prcClip->top - texth) / 2;
  y = prcClip->top + h + 1;

  // print the text
  ExtTextOut(hdc, x, y, ETO_CLIPPED | ETO_OPAQUE,
             prcClip, szString, strlen(szString), NULL);
  if (!(LVCAlign & LVCFMT_LEFT))
    SetTextAlign(hdc, TA_LEFT); 

}

//
//  FUNCTION:   DrawListViewItem(const LPDRAWITEMSTRUCT)
//
//  PURPOSE:    Draws one item in the listview control.
//
//  PARAMETERS:
//      lpDrawItem - Pointer to the information needed to draw the item.  The
//                   item number is in the itemID member.
//


void DrawListViewItem(LPDRAWITEMSTRUCT lpDrawItem)
{
  HIMAGELIST himl;
  LV_ITEM lvi;
  LV_COLUMN lvc;
  int cxImage = 0, cyImage = 0;
  RECT rcClip;
  int iColumn = 1;
  UINT uiFlags = ILD_TRANSPARENT;
  char buff[255];
  int col;
	int defheight;
	TEXTMETRIC tm;

  // Get the item image to be displayed
  lvi.mask = LVIF_IMAGE | LVIF_STATE | LVIF_TEXT;
  lvi.iItem = lpDrawItem->itemID;
  lvi.iSubItem = 0;
  lvi.pszText = buff;
  lvi.cchTextMax = sizeof(buff);
  SendMessage(lpDrawItem->hwndItem, LVM_GETITEM, 0, (LPARAM) &lvi);

  // Check to see if this item is selected
  if (lpDrawItem->itemState & ODS_SELECTED) {
    // Set the text background and foreground colors
    SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
    SetBkColor(lpDrawItem->hDC, GetSysColor(COLOR_HIGHLIGHT));

  	// Also add the ILD_BLEND50 so the images come out selected
	  uiFlags |= ILD_BLEND50;
  }
  else {
    // Set the text background and foreground colors to the standard window colors
    SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_WINDOWTEXT));
    SetBkColor(lpDrawItem->hDC, GetSysColor(COLOR_WINDOW));
  }

  // Get the image list and draw the image
  himl = ListView_GetImageList(lpDrawItem->hwndItem, LVSIL_SMALL);
  if (himl) {
    ImageList_Draw(himl, lvi.iImage, lpDrawItem->hDC,
                   lpDrawItem->rcItem.left, lpDrawItem->rcItem.top,
                   uiFlags);

    // Find out how big the image we just drew was
    ImageList_GetIconSize(himl, &cxImage, &cyImage);
  }
  else {
    cxImage = 0;
    cyImage = 0;
  }

  // Calculate the width of the first column after the image width.  If
  // There was no image, then cxImage will be zero.

  // Set up the new clipping rect for the first column text and draw it
  lvc.mask = LVCF_FMT | LVCF_WIDTH;
  SendMessage(lpDrawItem->hwndItem, LVM_GETCOLUMN, 0, (LPARAM) &lvc);
  rcClip.left = lpDrawItem->rcItem.left + cxImage;
  rcClip.right = lpDrawItem->rcItem.left + lvc.cx;
  rcClip.top = lpDrawItem->rcItem.top;
  rcClip.bottom = lpDrawItem->rcItem.bottom;

  GetTextMetrics(lpDrawItem->hDC, &tm);

  defheight = tm.tmHeight + 2;

  DrawItemColumn(lpDrawItem->hDC, buff, defheight, &rcClip, lvc.fmt);

	col = 1;
    // Update the clip rect to the remaining columns
	while (lvc.mask = LVCF_FMT | LVCF_WIDTH,
         SendMessage(lpDrawItem->hwndItem, LVM_GETCOLUMN, col, (LPARAM) &lvc)) {
    lvi.mask = LVIF_TEXT;
    lvi.iItem = lpDrawItem->itemID;
    lvi.iSubItem = col;
    lvi.pszText = buff;
    lvi.cchTextMax = sizeof(buff);
    SendMessage(lpDrawItem->hwndItem, LVM_GETITEM, 0, (LPARAM) &lvi);
    rcClip.left = rcClip.right;
    rcClip.right = rcClip.left + lvc.cx;
    DrawItemColumn(lpDrawItem->hDC, buff, defheight, &rcClip, lvc.fmt);
  	col++;
  }
    
  // If we changed the colors for the selected item, undo it
  if (lpDrawItem->itemState & ODS_SELECTED) {
    // Set the text background and foreground colors
    SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_WINDOWTEXT));
    SetBkColor(lpDrawItem->hDC, GetSysColor(COLOR_WINDOW));
  }

  // If the item is focused, now draw a focus rect around the entire row
  if (lpDrawItem->itemState & ODS_FOCUS) {
    // Adjust the left edge to exclude the image
    rcClip = lpDrawItem->rcItem;
    rcClip.left += cxImage;

    // Draw the focus rect
    DrawFocusRect(lpDrawItem->hDC, &rcClip);
  }
}

LRESULT WndProcMainDrawItem(HWND hwnd, UINT message, WPARAM wParam, LONG lParam)
{
  int idCtl = (UINT) wParam; // control identifier 
  LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT) lParam; // item-drawing information
  
  // Make sure the control is the listview control
  if (lpDrawItem->CtlType != ODT_LISTVIEW)
      return FALSE;

  // There are three types of drawing that can be requested.  First, to draw
  // the entire contents of the item specified.  Second, to update the focus
  // rect as the focus changed, and third to update the selection as the
  // selection changes.
  //
  // NOTE: An artifact of the implementation of the listview control is that
  // it doesn't send the ODA_FOCUS or ODA_SELECT action items.  All updates
  // sent as ODA_DRAWENTIRE and the lpDrawItem->itemState flags contain the
  // selected and focused information.

  switch (lpDrawItem->itemAction) {
     // Just in case the implementation of the control changes in the
     // future, let's handle the other itemAction types too.
      case ODA_DRAWENTIRE:
      case ODA_FOCUS:
      case ODA_SELECT:
        DrawListViewItem(lpDrawItem);
        break;
  }

  return TRUE;
}

LRESULT WndProcMainMeasureItem(HWND hwnd, UINT message, WPARAM wParam, LONG lParam)
{
  int idCtl = (UINT) wParam;                // control identifier 
  LPMEASUREITEMSTRUCT lpMeasureItem = (LPMEASUREITEMSTRUCT) lParam; // item-size information 
  TEXTMETRIC tm;
  HDC hdc;
	HFONT hFont;
	HWND hwndLV;
  HIMAGELIST himl;
	UINT cxImage;
	UINT cyImage;

  // WM_MEASUREITEM
  // Make sure the control is the listview control
  if (lpMeasureItem->CtlType != ODT_LISTVIEW)
    return FALSE;

	// Get the handle of the ListView control we're using
	hwndLV = GetDlgItem(hwnd, lpMeasureItem->CtlID);

	// Get the font the control is currently using
	hFont = (HFONT)(DWORD) SendMessage(hwndLV, WM_GETFONT, 0, 0L);

	// Set the font of the DC to the same font the control is using
  hdc = GetDC(hwndLV);
	SelectObject(hdc, hFont);

  // Get the height of the font used by the control
  if (!GetTextMetrics(hdc, &tm))
    return FALSE;

  // Add a little extra space between items
  lpMeasureItem->itemHeight = tm.tmHeight + 2;

  // Make sure there is enough room for the images which are cyImage high
  // Get the image list and measure
  himl = ListView_GetImageList(hwndLV, LVSIL_SMALL);
  if (himl) {
    ImageList_GetIconSize(himl, &cxImage, &cyImage);
		cyImage++;
    if (lpMeasureItem->itemHeight < cyImage)
  	  lpMeasureItem->itemHeight = cyImage;
	}
	// Clean up
	ReleaseDC(hwndLV, hdc);
  return TRUE;
}

