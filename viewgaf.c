//
//  GAF Viewer Code
//

#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "resource.h"
#include "hpiview.h"
#include "splitstuf.h"
#include "viewstuf.h"
#include "palette.h"

#pragma pack(1)

typedef struct _VIEWOPTION {
	int option;
	int Length;
  int Version;
	char *Name;
	char *buff;
  GAFHEADER *Header;
  GAFENTRY *CurEntry;
  GAFFRAMEENTRY *FrameEntry;
  GAFFRAMEDATA **gfd;
  HPALETTE hpal;
	HDC hdc;
  int CurFrame;
	int xofs;
	int yofs;
} VIEWOPTION;

#pragma pack()

const char ViewGafBmpWindowClass[] = "HPIViewGafBmp";
int ViewGafClassRegistered = FALSE;

void FreeFrameStuff(VIEWOPTION *vo)
{
  if (vo->gfd) {
    FreeMem(vo->gfd);
	  vo->gfd = NULL;
	}
}

void FrameToHDC(HDC hdc, VIEWOPTION *vo, GAFFRAMEDATA *gfd)
{
  unsigned char *frame;
  int bcount;
  int count;
  int repeat;
  unsigned char mask;
	int xcount;
	int ycount;
  int y;
  COLORREF *color;
  int xofs = vo->xofs - gfd->XPos;
  int yofs = vo->yofs - gfd->YPos;

  frame = vo->buff + gfd->PtrFrameData;
  switch (vo->Version) {
    case HPI_V1 :
      color = TAColor;
      break;
    case HPI_V2 :
      color = TAKColor;
      break;
    default :
      color = TAColor;
  }
	if (gfd->Compressed) {
    for (ycount = 0; ycount < gfd->Height; ycount++) {
      bcount = *((short *) frame);
      frame += sizeof(short);
      y = yofs + ycount;
      xcount = xofs;
      count = 0;
      while (count < bcount) {
        mask = (unsigned char) frame[count++];
        if ((mask & 0x01) == 0x01) {
          // transparent
          xcount += (mask >> 1);
        }
        else if ((mask & 0x02) == 0x02) {
          // repeat next byte
          repeat = (mask >> 2) + 1;
          while (repeat--) {
            SetPixelV(hdc, xcount++, y, color[frame[count]]);
           
          }
          count++;
				}
        else {
          repeat = (mask >> 2) + 1;
          while (repeat--) {
            SetPixelV(hdc, xcount++, y, color[frame[count++]]);
					}
				}
			}
      frame += bcount;
		}
	}
	else {
    for (ycount = 0; ycount < gfd->Height; ycount++) {
			for (xcount = 0; xcount < gfd->Width; xcount++) {
        SetPixelV(hdc, xcount+xofs, ycount+yofs, color[*frame++]);
			}
		}
	}
}

void ShowFrameData(HWND hwndBmp, HDC hdc, VIEWOPTION *vo, GAFFRAMEDATA *gfd)
{
  int i;
  int *SubFrame;
  GAFFRAMEDATA *gfdSub;

  if (gfd->FramePointers) {
    SubFrame = (void *) (vo->buff + gfd->PtrFrameData);
    for (i = 0; i < gfd->FramePointers; i++) {
      gfdSub = (void *) (vo->buff + *SubFrame);
      FrameToHDC(hdc, vo, gfdSub);
      SubFrame++;
    }
  }
  else
    FrameToHDC(hdc, vo, gfd);
}

void SetCurrentFrame(HWND hwndBmp, VIEWOPTION *vo, int FrameNo)
{
  GAFFRAMEDATA *gfd;
  HWND hwnd = GetParent(hwndBmp);

  vo->CurFrame = FrameNo;
  SetDlgItemInt(hwnd, IDC_CURFRAME, FrameNo, FALSE);
  gfd = vo->gfd[FrameNo];
  SetDlgItemInt(hwnd, IDC_WIDTH, gfd->Width, TRUE);
  SetDlgItemInt(hwnd, IDC_HEIGHT, gfd->Height, TRUE);
  SetDlgItemInt(hwnd, IDC_XPOS, gfd->XPos, TRUE);
  SetDlgItemInt(hwnd, IDC_YPOS, gfd->YPos, TRUE);
  SetDlgItemInt(hwnd, IDC_SUBFRAMES, gfd->FramePointers, TRUE);

  InvalidateRect(hwndBmp, NULL, TRUE);
  //UpdateWindow(hwndBmp);
}

void GafListSelect(HWND hwnd, HWND hwndList, VIEWOPTION *vo)
{
  int index;
  int i;
  HWND hwndBmp = GetDlgItem(hwnd, IDC_PICTURE);

  index = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
  
  if (index == LB_ERR)
    return;

  FreeFrameStuff(vo);

  vo->CurEntry = (void *) SendMessage(hwndList, LB_GETITEMDATA, index, 0);
  vo->FrameEntry = (void *) (vo->CurEntry+1);

  SetDlgItemInt(hwnd, IDC_FRAMES, vo->CurEntry->Frames, FALSE);

  SendDlgItemMessage(hwnd, IDC_SLIDE, TBM_SETRANGE, TRUE, MAKELONG(0, vo->CurEntry->Frames - 1)); 
  SendDlgItemMessage(hwnd, IDC_SLIDE, TBM_SETPOS, TRUE, 0); 

  SetDlgItemText(hwnd, IDC_CURFRAME, "");
  SetDlgItemText(hwnd, IDC_WIDTH, "");
  SetDlgItemText(hwnd, IDC_HEIGHT, "");
  SetDlgItemText(hwnd, IDC_XPOS, "");
  SetDlgItemText(hwnd, IDC_YPOS, "");
  SetDlgItemText(hwnd, IDC_SUBFRAMES, "");

  if (!vo->CurEntry->Frames) {
    InvalidateRect(hwndBmp, NULL, TRUE);
		return;
  }

  vo->gfd = GetMem(vo->CurEntry->Frames * sizeof(GAFFRAMEDATA *), TRUE);

  for (i = 0; i < vo->CurEntry->Frames; i++) {
		vo->gfd[i] = (void *) (vo->buff + vo->FrameEntry[i].PtrFrameTable);
  }

  SetCurrentFrame(hwndBmp, vo, 0);
}

int LoadViewGaf(HWND hwnd, VIEWOPTION *vo)
{
	GAFHEADER *Gaf;
  long *EntryPtr;
  int EntryNo;
  int Index;
  GAFENTRY *Entry;
  HWND hwndList;
	
	Gaf = (void *) vo->buff;

  EntryPtr = (long *) (vo->buff + sizeof(GAFHEADER));

  SendDlgItemMessage(hwnd, IDC_GAFLIST, LB_RESETCONTENT, 0, 0);

  for (EntryNo = 0; EntryNo < Gaf->Entries; EntryNo++) {
    Entry = (GAFENTRY *) (vo->buff + *EntryPtr);
    Index = SendDlgItemMessage(hwnd, IDC_GAFLIST, LB_ADDSTRING, 0, (LPARAM) Entry->Name);
    SendDlgItemMessage(hwnd, IDC_GAFLIST, LB_SETITEMDATA, Index, (LPARAM) Entry);

    EntryPtr++;
  }
  if (EntryNo) {
    hwndList = GetDlgItem(hwnd, IDC_GAFLIST);
    SendMessage(hwndList, LB_SETCURSEL, 0, 0);
    GafListSelect(hwnd, hwndList, vo);
  }

  return TRUE;
}

long WndProcViewGafInit(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//wm_initdialog
  HICON hiSmall;
  HICON hiLarge;
	RECT rc;
	int x;
	int y;
	int inset;

  VIEWOPTION *vo = (void *) (lParam);

  SetWindowLong(hwnd, GWL_USERDATA, (long) vo);

  hiSmall = LoadImage(hThisInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
  hiLarge = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_MAIN));

  SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM) hiLarge);
  SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM) hiSmall);

  SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM) vo->Name); 
  
	GetWindowRect(GetDlgItem(hwnd, IDC_GAFLIST), &rc);
  ConvertToClient(hwnd, &rc);
	inset = rc.left;
	x = rc.right + inset;

	GetWindowRect(GetDlgItem(hwnd, IDC_YPOS), &rc);
  ConvertToClient(hwnd, &rc);
	y = rc.bottom + inset;

  CreateWindowEx(WS_EX_CLIENTEDGE, ViewGafBmpWindowClass, "",
    WS_CHILD | WS_VISIBLE,
    x, y, 100, 100,
    hwnd, (HMENU) IDC_PICTURE, hThisInstance, NULL);

	if (!LoadViewGaf(hwnd, vo))
    PostMessage(hwnd, WM_CLOSE, 0, 0);

	return 0;
}

long WndProcViewGafSize(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
// wm_size
{
	RECT rc;
	HWND hwndChild;
	int fwSizeType = wParam;      // resizing flag 
  int nWidth = LOWORD(lParam);  // width of client area 
  int nHeight = HIWORD(lParam); // height of client area
	int x;

	if (fwSizeType == SIZE_MINIMIZED)
		return FALSE;

	hwndChild = GetDlgItem(hwnd, IDC_GAFLIST);
	GetWindowRect(hwndChild, &rc);
	MoveWindow(hwndChild, 2, 2, rc.right-rc.left, nHeight - 4, TRUE);
	x = rc.right - rc.left + 6;
	hwndChild = GetDlgItem(hwnd, IDC_PICTURE);
	GetWindowRect(hwndChild, &rc);
  ConvertToClient(hwnd, &rc);
	MoveWindow(hwndChild, x, rc.top, nWidth - x - 2, nHeight-rc.top-2, TRUE);
	return FALSE;
}

LRESULT WndProcViewGafCommand(HWND hwnd, UINT message, WPARAM wParam, LONG lParam)
{
	VIEWOPTION *vo;
  UINT idItem = LOWORD(wParam);
  UINT wNotifyCode = HIWORD(wParam);
  HWND hwndCtl = (HWND) lParam;

	vo = (void *) GetWindowLong(hwnd, GWL_USERDATA);

  switch (idItem) {
	  case IDC_GAFLIST :
      if (wNotifyCode == LBN_SELCHANGE) 
        GafListSelect(hwnd, hwndCtl, vo);
      return 0;
  }
  return 0;
}

long WndProcViewGafScroll(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{ 
	// wm_hscroll - from slider
  int wNotifyCode = LOWORD(wParam); // scroll bar value
	VIEWOPTION *vo;
  int NewPos;

	vo = (void *) GetWindowLong(hwnd, GWL_USERDATA);

  if (!vo->CurEntry->Frames)
    return 0;
  
  NewPos = SendMessage((HWND) lParam, TBM_GETPOS, 0, 0);
  SetCurrentFrame(GetDlgItem(hwnd, IDC_PICTURE), vo, NewPos);
  return 0;
}  

long WndProcViewGafClose(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	VIEWOPTION *vo;
	vo = (void *) GetWindowLong(hwnd, GWL_USERDATA);

  FreeFrameStuff(vo);
	if (vo->Name)
	  FreeMem(vo->Name);
  if (vo->hpal)
    DeleteObject(vo->hpal);
	if (vo->buff)
	  FreeMem(vo->buff);
	FreeMem(vo);

	DestroyWindow(hwnd);
  return FALSE;
}

LRESULT CALLBACK WndProcViewGaf(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
		case WM_COMMAND :
			return WndProcViewGafCommand(hwnd, message, wParam, lParam);
		case WM_SIZE :
			return WndProcViewGafSize(hwnd, message, wParam, lParam);
		case WM_HSCROLL :
			return WndProcViewGafScroll(hwnd, message, wParam, lParam);
    case WM_INITDIALOG :
      return WndProcViewGafInit(hwnd, message, wParam, lParam);
    case WM_CLOSE :
      return WndProcViewGafClose(hwnd, message, wParam, lParam);
  }   
  return FALSE;
}

long WndProcViewBmpGafPaint(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
// wm_paint
{
  PAINTSTRUCT  ps;
	VIEWOPTION *vo;
  GAFFRAMEDATA *gfd;
  RECT rc;
  HWND hwndParent = GetParent(hwnd);

  BeginPaint(hwnd, &ps);

  vo = (void *) GetWindowLong(hwndParent, GWL_USERDATA);
  if (vo->gfd) {
    // Select palette and realize it.
    SelectPalette(ps.hdc, vo->hpal, FALSE );
    RealizePalette(ps.hdc);

    gfd = (void *) (vo->buff + vo->FrameEntry[vo->CurFrame].PtrFrameTable);
    GetClientRect(hwnd, &rc);

	  vo->xofs = (rc.right / 2) + vo->gfd[0]->XPos - (vo->gfd[0]->Width / 2);
	  vo->yofs = (rc.bottom / 2) + vo->gfd[0]->YPos - (vo->gfd[0]->Height / 2);
	
    ShowFrameData(hwnd, ps.hdc, vo, gfd);
  }
	EndPaint(hwnd, &ps);
	return 0;
}

LRESULT CALLBACK WndProcViewGafBmp(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
		case WM_PAINT :
			return WndProcViewBmpGafPaint(hwnd, message, wParam, lParam);
  }   
  return DefWindowProc(hwnd, message, wParam, lParam);
}

void RegisterViewGafClasses(void)
{
  WNDCLASSEX  wndclass;

	if (ViewGafClassRegistered)
		return;

  //Main Window
  wndclass.cbSize        = sizeof(wndclass);
  wndclass.style         = CS_HREDRAW | CS_VREDRAW; //CS_OWNDC;
  wndclass.lpfnWndProc   = WndProcViewGafBmp;
  wndclass.cbClsExtra    = 0;
  wndclass.cbWndExtra    = 0;
  wndclass.hInstance     = hThisInstance;
  wndclass.hIcon         = NULL;
  wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
  wndclass.lpszMenuName  = NULL;
  wndclass.lpszClassName = ViewGafBmpWindowClass;
  wndclass.hIconSm       = NULL;
  RegisterClassEx(&wndclass);

	ViewGafClassRegistered = TRUE;
}

void DumpFrameData(FILE *f, char *buff, int PtrFrameTable, int FrameNo, int level)
{
  GAFFRAMEDATA *gfd;
  int *SubFrame;
  int i;
  char spaces[100];
  int bytes;
  char *data;
  int count;
  int mask;
  int x;
  int repeat;

  FillMemory(spaces, sizeof(spaces), ' ');
  spaces[level * 3] = 0;
  gfd = (void *) (buff + PtrFrameTable);
  fprintf(f, "%sW: %d  H: %d  X: %d  Y: %d\n", spaces, gfd->Width, gfd->Height, gfd->XPos, gfd->YPos);
  fprintf(f, "%sU1: %d  Compressed: %d  FramePointers: %d  U2: %d  U3: 0x%08X\n",
    spaces, gfd->Unknown1, gfd->Compressed, gfd->FramePointers, gfd->Unknown2, gfd->Unknown3);
  fprintf(f, "%sPtrFrameData: 0x%08X\n", spaces, gfd->PtrFrameData);
  if (gfd->FramePointers) {
    SubFrame = (void *) (buff + gfd->PtrFrameData);
    for (i = 0; i < gfd->FramePointers; i++) {
      DumpFrameData(f, buff, *SubFrame, i, level+1);
      SubFrame++;
    }
  }
  else {
    data = buff + gfd->PtrFrameData;
    for (i = 0; i < gfd->Height; i++) {
      fprintf(f, "%sLine %3d", spaces, i);
      bytes = *((short *) data);
      data += sizeof(short);
      count = 0;
      while (count < bytes) {
        mask = (unsigned char) data[count++];
        if ((mask & 0x01) == 0x01) {
          // transparent
          for (x = 0; x < (mask >> 1); x++)
            fprintf(f, "   ");
        }
        else if ((mask & 0x02) == 0x02) {
          // repeat next byte
          repeat = (mask >> 2) + 1;
          while (repeat--)
            fprintf(f, " %02X", (unsigned char) data[count]);
          count++;
        }
        else {
          repeat = (mask >> 2) + 1;
          while (repeat--)
            fprintf(f, " %02X", (unsigned char) data[count++]);
        }
      }
      data += bytes;
      fprintf(f, "*\n");
    }
    fprintf(f, "\n");
    data = buff + gfd->PtrFrameData;
    for (i = 0; i < gfd->Height; i++) {
      fprintf(f, "%sLine %3d", spaces, i);
      bytes = *((short *) data);
      data += sizeof(short);
      for (count = 0; count < bytes; count++) {
        fprintf(f, " %02X", (unsigned char) data[count]);
      }
      fprintf(f, "\n");
      data += bytes;
    }
  }
}

void DumpGAF(char *buff, char *Name)
{
  FILE *f;
  char fname[MAX_PATH];
	GAFHEADER *Gaf;
  long *EntryPtr;
  int EntryNo;
  GAFENTRY *Entry;
  GAFFRAMEENTRY *FrameEntry;
  int i;

  strcpy(fname, Name);
  strcat(fname, ".dmp");
  f = fopen(fname, "wtc");

	
	Gaf = (void *) buff;

  EntryPtr = (long *) (buff + sizeof(GAFHEADER));

  for (EntryNo = 0; EntryNo < Gaf->Entries; EntryNo++) {
    Entry = (GAFENTRY *) (buff + *EntryPtr);
    fprintf(f, "\nEntryPtr 0x%08X\nName %s  Frames %d  U1: %d  U2: %d\n", 
      *EntryPtr, Entry->Name, Entry->Frames, Entry->Unknown1, Entry->Unknown2);
    FrameEntry = (void *) (Entry+1);

    if (Entry->Frames) {
      for (i = 0; i < Entry->Frames; i++) {
        fprintf(f, "Frame %d\n", i);
        fprintf(f, "PtrFrameTable: 0x%08X\n", FrameEntry[i].PtrFrameTable);
        fprintf(f, "FrameEntry.Unknown1: 0x%08X\n", FrameEntry[i].Unknown1);
        DumpFrameData(f, buff, FrameEntry[i].PtrFrameTable, i, 0);
      }
    }
    EntryPtr++;
  }
  fclose(f);
}

void InitVOStuff(VIEWOPTION *vo)
{
  LOGPALETTE *lp;
  PALETTEENTRY *pal;
  int i;

  // Allocate memory for color palette.
  lp = GetMem(sizeof(LOGPALETTE)+(256 * sizeof(PALETTEENTRY)), FALSE);

  lp->palVersion    = 0x300;      
  lp->palNumEntries = 256;

  // Load each color into the palette.
  switch (vo->Version) {
    case HPI_V1 :
      pal = TAPalette;
      break;
    case HPI_V2 :
      pal = TAKPalette;
      break;
    default :
      pal = TAPalette;
  }
  for (i = 0; i < 256; i++ ) {
    lp->palPalEntry[i].peRed = pal[i].peRed;
    lp->palPalEntry[i].peGreen = pal[i].peGreen;
    lp->palPalEntry[i].peBlue = pal[i].peBlue;
    lp->palPalEntry[i].peFlags = pal[i].peFlags;
  }

  // Create the Palette.
  vo->hpal = CreatePalette(lp);
  FreeMem(lp);
}

int ViewGafFile(char *buff, int Length, char *Name, int option, HPIITEM *hi)
{
	HWND hwnd;
	VIEWOPTION *vo;

  //DumpGAF(buff, Name);
  //FreeMem(buff);
  //return TRUE;

  RegisterViewGafClasses();
  
	vo = GetMem(sizeof(VIEWOPTION), TRUE);

  vo->Version = hi->hfd->Version;
	vo->Length = Length;
	vo->buff = buff;
	vo->Name = DupString(Name);
	vo->option = option;

	InitVOStuff(vo);

  hwnd = CreateDialogParam(hThisInstance, MAKEINTRESOURCE(IDD_GAFVIEW), 
         NULL, WndProcViewGaf, (LPARAM) vo);

	if (!hwnd)
		return FALSE;

  ShowWindow(hwnd, SW_SHOWNORMAL);

	// buff is freed when the window closes
	return TRUE;
}

