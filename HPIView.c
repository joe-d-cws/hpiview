//
//  HPI File Viewer
//

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/utime.h>

#include "resource.h"
#include "hpiview.h"

#include "splitstuf.h"
#include "liststuf.h"
#include "viewstuf.h"
#include "palette.h"

//#define ZLIB_DLL
//#define WINDOWS

#include "zlib.h"

#define HEX_HAPI 0x49504148
// 'BANK'
#define HEX_BANK 0x4B4E4142
// 'SQSH'
#define HEX_SQSH 0x48535153

#define TADIR_REG_ENTRY "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Total Annihilation"

#define MAXCOLUMN 3
#define UM_INIT2 (WM_USER+1)

const char szAppName[] = "HPIView";
const char szAppTitle[] = "HPI File Viewer";
const char HPIFilter[] = "HPI Files (*.hpi,*.ufo,*.kmp,*.gp?,*.ccx)\0*.hpi;*.ufo;*.kmp;*.gp?;*.ccx\0All Files (*.*)\0*.*\0";
//const char TileFilter[] = "Annihilator Tileset (*.tat)\0*.tat\0";
//const char AnimFilter[] = "Annihilator Resources (*.res)\0*.res\0";
const char szSaveFilter[] = "All Files (*.*)\0*.*\0";

HANDLE hThisInstance = NULL;
HINSTANCE hRichEd = NULL;
HWND hwndMain = NULL;
HWND hwndSplit = NULL;
HIMAGELIST FileIcons = NULL;
HMENU hmPopNormal = NULL;
int ExtractCount;
int TotalCount;
FILE *DumpFile;
FILE *OutFile;
HANDLE ProgHeap = NULL;

HTREEITEM CurrentHPI = NULL;
HPIFILEDATA *HPIInfo = NULL;

int SortColumn = 0;
int SortOrder = FALSE;
char CurrentDirectory[MAX_PATH] = "";
char CurrentSaveDirectory[MAX_PATH] = "";
char ININame[MAX_PATH];

#define IDC_SPLIT 1

int IconOpen;
int IconClosed;
int IconList[MAX_VIEW_TYPE];

LPVOID GetMem(int size, int zero)
{
  LPVOID result;

  result = HeapAlloc(ProgHeap, (zero ? HEAP_ZERO_MEMORY : 0), size); 
  //result = (LPVOID) GlobalAlloc((zero ? GPTR : GMEM_FIXED), size);
  return result; 
}

LPVOID ReallocMem(LPVOID mem, int size)
{ 
  LPVOID result;

  result = HeapReAlloc(ProgHeap, 0, mem, size);
  //result = (LPVOID) GlobalReAlloc((HGLOBAL) mem, size, GMEM_MOVEABLE);
  return result;
}

void FreeMem(LPVOID x)
{
  HeapFree(ProgHeap, 0, x);
  //GlobalFree(x);
}

LPSTR DupString(LPSTR x)
{
  LPSTR s = GetMem(strlen(x)+1, FALSE);
  strcpy(s, x);
  return s;
}

void MsgPrintf(HWND hwnd, LPSTR fmt, ...)
{
  va_list argptr;
  char tstr[1024];
  
  va_start(argptr, fmt);
  wvsprintf(tstr, fmt, argptr);
  va_end(argptr);
  MessageBox(hwnd, tstr, szAppTitle, MB_OK);
}

int RoundUp(int num, int divisor)
{
  int result;

  if (!divisor)
    return num;
  result = num / divisor;
  result *= divisor;
  if (result != num)
    result += divisor;
  return result;
}

void EnableMenus(int option)
{
	EnableMenuItem(GetMenu(hwndMain), IDM_FILEVIEW, option);
	EnableMenuItem(GetMenu(hwndMain), IDM_FILEEXTRACT, option);
	EnableMenuItem(GetMenu(hwndMain), IDM_FILEEXTRACTALL, option);
}

int IsTextFile(char *ext)
{
	int result;

	result = stricmp(ext, ".bat");
	if (result <= 0)
		return !result;
	result = stricmp(ext, ".bos");
	if (result <= 0)
		return !result;
	result = stricmp(ext, ".c");
	if (result <= 0)
		return !result;
	result = stricmp(ext, ".fbi");
	if (result <= 0)
		return !result;
	result = stricmp(ext, ".gui");
	if (result <= 0)
		return !result;
	result = stricmp(ext, ".h");
	if (result <= 0)
		return !result;
	result = stricmp(ext, ".ota");
	if (result <= 0)
		return !result;
	result = stricmp(ext, ".pl");
	if (result <= 0)
		return !result;
	result = stricmp(ext, ".taf");
	if (result <= 0)
		return !result;
	result = stricmp(ext, ".tdf");
	if (result <= 0)
		return !result;
	result = stricmp(ext, ".txt");
	if (result <= 0)
		return !result;
	result = stricmp(ext, ".xls");
	if (result <= 0)
		return !result;
	return FALSE;
}

int FindFileType(char *Name)
{
	char *ext;

	ext = strrchr(Name, '.');
	if (!ext)
		return VIEW_HEX;
	if (IsTextFile(ext))
		return VIEW_TEXT;
	if (stricmp(ext, ".wav") == 0)
		return VIEW_WAV;
	if (stricmp(ext, ".bmp") == 0)
		return VIEW_BMP;
	if (stricmp(ext, ".pcx") == 0)
		return VIEW_PCX;
	if (stricmp(ext, ".jpg") == 0)
		return VIEW_JPG;
	if (stricmp(ext, ".tnt") == 0)
		return VIEW_TNT;
	if (stricmp(ext, ".gaf") == 0)
		return VIEW_GAF;
	if (stricmp(ext, ".sct") == 0)
		return VIEW_SCT;
	if (stricmp(ext, ".cob") == 0)
		return VIEW_COB;
	if (stricmp(ext, ".3do") == 0)
		return VIEW_3DO;
	return VIEW_HEX;
}
	

int StrIEqual(LPSTR Str1, LPSTR Str2)
{ // compares the entire length of Str1 against the first characters of Str2.
  // not case sensitive
  // it it matches, return 0.
  // else return -1 or 1 like strcmp.

  int c1;
  int c2;

  while ((*Str1) && (*Str2)) {
    c1 = tolower(*Str1);
    c2 = tolower(*Str2);
    if (c1 < c2)
      return -1;
    else if (c1 > c2)
      return 1;
    else {
      Str1++;
      Str2++;
    }
  }
  if (!*Str1)
    return 0;
  return 1;
}

int SelectDirectory(LPSTR DName)
{
	BROWSEINFO bi;
	LPITEMIDLIST iid;
  LPMALLOC g_pMalloc;
	char DispName[MAX_PATH];

  bi.hwndOwner = hwndMain;
  bi.pidlRoot = NULL;
  bi.pszDisplayName = DispName;
  bi.lpszTitle = "Select destination directory:";
  bi.ulFlags = BIF_RETURNONLYFSDIRS; 
  bi.lpfn = NULL;
  bi.lParam = 0;
  bi.iImage = 0;

	iid = SHBrowseForFolder(&bi);

	if (!iid)
		return FALSE;

	
	SHGetPathFromIDList(iid, DName);

  SHGetMalloc(&g_pMalloc);         
  g_pMalloc->lpVtbl->Free(g_pMalloc, iid);
  g_pMalloc->lpVtbl->Release(g_pMalloc);     	

  return TRUE;
}

HPIITEM *FindSelectedEntry(int smask)
{
	LV_ITEM lvi;
	HWND hwnd = GetDlgItem(hwndMain, IDC_HPILIST);
	int item;
	int ItemCount;

	ItemCount = SendMessage(hwnd, LVM_GETITEMCOUNT, 0, 0) ;
	if (ItemCount == 0)
		return NULL;

	if (SendMessage(hwnd, LVM_GETSELECTEDCOUNT, 0, 0) == 0)
		return NULL;

	for (item = 0; item < ItemCount; item++) {
		lvi.iItem = item;
		lvi.iSubItem = 0;
		lvi.mask = LVIF_PARAM | LVIF_STATE;
		lvi.stateMask = smask;
		SendMessage(hwnd, LVM_GETITEM, 0, (LPARAM) &lvi);
		if (lvi.state & smask) {
			return (HPIITEM *) lvi.lParam;
		}
	}
	return NULL;
}

LPSTR GetTADirectory(LPSTR TADir)
{
	HKEY hk;
	int size = MAX_PATH;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TADIR_REG_ENTRY, 0, KEY_READ, &hk)) {
		*TADir = 0;
	  return TADir;
	}
	if (RegQueryValueEx(hk, "Dir", 0, NULL, TADir, &size)) {
		*TADir = 0;
	}
	RegCloseKey(hk);
	return TADir;
}

int CreateImages(void)
{
  HICON hi;

  FileIcons = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
                                TRUE, 0, 5);
  if (!FileIcons) {
    MessageBox(hwndMain, "Unable to create image list", szAppTitle, MB_OK);
    return FALSE;
  }

	hi = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_CLOSED));
  IconClosed = ImageList_AddIcon(FileIcons, hi);
	hi = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_OPEN));
  IconOpen = ImageList_AddIcon(FileIcons, hi);
	hi = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_BLANK));
  IconList[VIEW_HEX] = 
    IconList[VIEW_COB] = 
      IconList[VIEW_3DO] = ImageList_AddIcon(FileIcons, hi);
	hi = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_TEXT));
  IconList[VIEW_TEXT] = ImageList_AddIcon(FileIcons, hi);
	hi = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_WAV));
  IconList[VIEW_WAV] = ImageList_AddIcon(FileIcons, hi);
	hi = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_BMP));
  IconList[VIEW_PCX] = 
    IconList[VIEW_BMP] = 
      IconList[VIEW_JPG] = ImageList_AddIcon(FileIcons, hi);
	hi = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_TNT));
  IconList[VIEW_TNT] = ImageList_AddIcon(FileIcons, hi);
	hi = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_GAF));
  IconList[VIEW_GAF] = ImageList_AddIcon(FileIcons, hi);
	hi = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_SCT));
  IconList[VIEW_SCT] = ImageList_AddIcon(FileIcons, hi);

  SendDlgItemMessage(hwndMain, IDC_HPILIST, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM) FileIcons); 
  SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM) FileIcons); 

  return TRUE;

}

void InitList(HWND hwnd)
{
	LV_COLUMN lvc;
	int col = 0;

  lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT;
  lvc.fmt = LVCFMT_LEFT;
  lvc.pszText = "File Name";
  lvc.iSubItem = col;
  SendMessage(hwnd, LVM_INSERTCOLUMN, col, (LPARAM) &lvc);

	col++;
  lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT;
  lvc.fmt = LVCFMT_LEFT;
  lvc.pszText = "Type";
  lvc.iSubItem = col;
  SendMessage(hwnd, LVM_INSERTCOLUMN, col, (LPARAM) &lvc);

	col++;
  lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT;
  lvc.fmt = LVCFMT_RIGHT;
  lvc.pszText = "Size";
  lvc.iSubItem = col;
  SendMessage(hwnd, LVM_INSERTCOLUMN, col, (LPARAM) &lvc);

	for (col = 0; col < MAXCOLUMN; col++)
	  SendMessage(hwnd, LVM_SETCOLUMNWIDTH, col, LVSCW_AUTOSIZE_USEHEADER);
}

int CALLBACK ListSortFunc(HPIITEM *i1, HPIITEM *i2, int item)
{
	LPSTR Name1;
	LPSTR Name2;
  int result;

	switch (SortColumn) {
		case 0 :
			if (i1->IsDir == i2->IsDir)
        result = stricmp(i1->Name, i2->Name);
			else
				result = i2->IsDir - i1->IsDir;
			break;
		case 1 :
			if (i1->IsDir == i2->IsDir) {
				if (i1->IsDir) {
          result = stricmp(i1->Name, i2->Name);
					break;
				}

        Name1 = strrchr(i1->Name, '.');
        Name2 = strrchr(i2->Name, '.');
				if (!Name1) {
					if (!Name2)
						result = 0;
					else
						result = -1;
				}
				else if (!Name2)
					result = 1;
				else {
					Name1++;
					Name2++;
          result = stricmp(Name1, Name2);
					if (result == 0) {
            result = stricmp(i1->Name, i2->Name);
					}
				}
			}
			else
				result = i2->IsDir - i1->IsDir;
			break;
		case 2 :
      result = i1->Size - i2->Size;
			break;
		default :
			result = 0;
	}

  if (SortOrder)
    result = -result;
  return result;
}

int CheckCalc(long *cs, char *buff, long size)
{
  int count;
  unsigned int c;
  unsigned char *check = (unsigned char *) cs;

  for (count = 0; count < size; count++) {
    c = (unsigned char) buff[count];
    check[0] += c;
    check[1] ^= c;
    check[2] += (c ^ ((unsigned char) (count & 0x000000FF)));
    check[3] ^= (c + ((unsigned char) (count & 0x000000FF)));
  }
  return *cs;  
}

int ReadAndDecrypt(int fpos, char *buff, int buffsize)
{
	int count;
	int tkey;
	int result;
	
	fseek(HPIInfo->HPIFile, fpos, SEEK_SET);
	result = fread(buff, buffsize, 1, HPIInfo->HPIFile);
  if (HPIInfo->Key)
	  for (count = 0; count < buffsize; count++) {
		  tkey = (fpos + count) ^ HPIInfo->Key;
      buff[count] = tkey ^ ~buff[count];
    }
	return result;
}

void SetCurrentHPI(HTREEITEM hItem)
{
	HPIITEM *hi;
	TV_ITEM tvi;

	if (hItem == CurrentHPI)
		return;
/*  tvi.hItem = hItem;
	tvi.mask = TVIF_PARAM;
 	SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_GETITEM, 0, (LPARAM) &tvi);
	hi = (void *) tvi.lParam;
  CurrentHPI = hItem;
	HPIInfo = hi->hfd;
  return;*/

	while (hItem) {
		tvi.hItem = hItem;
		tvi.mask = TVIF_PARAM;
 	  SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_GETITEM, 0, (LPARAM) &tvi);
		hi = (void *) tvi.lParam;
		if (hi->Name == NULL) {
		  CurrentHPI = hItem;
		  HPIInfo = hi->hfd;
		  return;
		}
  	hItem = (HTREEITEM) SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM) hItem);
	}
	CurrentHPI = NULL;
	HPIInfo = NULL;
}

void ShowFileList1(HWND hwndList, HPIITEM *hi, HTREEITEM Parent)
{
	int *Entries;
	HPIENTRY1 *Base;
	HPIENTRY1 *Entry;
	int count;
	char *Name;
	int *FileCount;
	int *FileLength;
	char *FileFlag;
	int *EntryOffset;
	LV_ITEM lvi;
	char buff[256];
	char *ext;
  HPIITEM *li;

	Base = hi->E1;
  if (Base)
  	Entries = (int *) (HPIInfo->Directory + Base->CountOffset);
  else
    Entries = (int *) (HPIInfo->Directory + hi->hfd->H1.Start);

	EntryOffset = Entries + 1;
	Entry = (HPIENTRY1 *) (HPIInfo->Directory + *EntryOffset);

  for (count = 0; count < *Entries; count++) {
		Name = HPIInfo->Directory + Entry->NameOffset;
		FileCount = (int *) (HPIInfo->Directory + Entry->CountOffset);
	  FileLength = FileCount + 1;
    
    li = GetMem(sizeof(HPIITEM), TRUE);
    li->hfd = hi->hfd;
    li->Name = Name;
    li->E1 = Entry;

    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE; 
		lvi.iItem = count;
		lvi.iSubItem = 0;
		lvi.lParam = (LPARAM) li;
		lvi.pszText = Name;
    if (Entry->Flag == 1) {
      li->Size = 0;
      li->IsDir = 1;
			lvi.iImage = IconClosed;
    }
    else {
      li->Size = *FileLength;
      li->IsDir = 0;
			lvi.iImage = IconList[FindFileType(Name)];
    }
	  SendMessage(hwndList, LVM_INSERTITEM, 0, (LPARAM) &lvi);

		if (Entry->Flag == 1)	{
			// directory
    	lvi.mask = LVIF_TEXT; 
	    lvi.iItem = count;
		  lvi.iSubItem = 1;
		  lvi.pszText = "Directory";
	    SendMessage(hwndList, LVM_SETITEM, 0, (LPARAM) &lvi);
		}
		else {
			ext = strrchr(Name, '.');
			if (ext) {
				ext++;
				strcpy(buff, ext);
				strupr(buff);
				strcat(buff, " file");
    		lvi.mask = LVIF_TEXT; 
	    	lvi.iItem = count;
		    lvi.iSubItem = 1;
		    lvi.pszText = buff;
	      SendMessage(hwndList, LVM_SETITEM, 0, (LPARAM) &lvi);
			}
			itoa(*FileLength, buff, 10);
			FileFlag = (char *)	(FileLength + 1);
  		lvi.mask = LVIF_TEXT; 
	  	lvi.iItem = count;
		  lvi.iSubItem = 2;
		  lvi.pszText = buff;
	    SendMessage(hwndList, LVM_SETITEM, 0, (LPARAM) &lvi);
		}	
		Entry++;
	}
}

void ShowFileList2(HWND hwndList, HPIITEM *hi, HTREEITEM Parent)
{
	HPIDIR2 *Base;
	HPIENTRY2 *Entry;
  HPIDIR2 *DirEntry;
	int count;
	char *Name;
	LV_ITEM lvi;
	char buff[256];
	char *ext;
  HPIITEM *li;

	Base = hi->D2;

  if (Base->SubCount) 
	  DirEntry = (HPIDIR2 *) (HPIInfo->Directory + Base->FirstSubDir);

  for (count = 0; count < Base->SubCount; count++) {
		Name = HPIInfo->Names + DirEntry[count].NamePtr;

    li = GetMem(sizeof(HPIITEM), TRUE);
    li->hfd = hi->hfd;
    li->Name = Name;
    li->Size = 0;
    li->IsDir = 1;
    li->D2 = DirEntry+count;

		lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE; 
		lvi.iItem = count;
		lvi.iSubItem = 0;
		lvi.lParam = (LPARAM) li;
		lvi.pszText = Name;
  	lvi.iImage = IconClosed;
    SendMessage(hwndList, LVM_INSERTITEM, 0, (LPARAM) &lvi);

   	lvi.mask = LVIF_TEXT; 
    lvi.iItem = count;
	  lvi.iSubItem = 1;
	  lvi.pszText = "Directory";
    SendMessage(hwndList, LVM_SETITEM, 0, (LPARAM) &lvi);
	}

  if (Base->FileCount) 
	  Entry = (HPIENTRY2 *) (HPIInfo->Directory + Base->FirstFile);

  for (count = 0; count < Base->FileCount; count++) {
		Name = HPIInfo->Names + Entry[count].NamePtr;

    li = GetMem(sizeof(HPIITEM), TRUE);
    li->hfd = hi->hfd;
    li->Name = Name;
    li->Size = Entry[count].DecompressedSize;
    li->IsDir = 0;
    li->E2 = Entry+count;

		lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE; 
		lvi.iItem = count;
		lvi.iSubItem = 0;
		lvi.lParam = (LPARAM) li;
		lvi.pszText = Name;
		lvi.iImage = IconList[FindFileType(Name)];
    SendMessage(hwndList, LVM_INSERTITEM, 0, (LPARAM) &lvi);

		ext = strrchr(Name, '.');
		if (ext) {
			ext++;
			strcpy(buff, ext);
			strupr(buff);
			strcat(buff, " file");
   		lvi.mask = LVIF_TEXT; 
    	lvi.iItem = count;
	    lvi.iSubItem = 1;
	    lvi.pszText = buff;
      SendMessage(hwndList, LVM_SETITEM, 0, (LPARAM) &lvi);
		}
		itoa(Entry[count].DecompressedSize, buff, 10);
 		lvi.mask = LVIF_TEXT; 
  	lvi.iItem = count;
	  lvi.iSubItem = 2;
	  lvi.pszText = buff;
    SendMessage(hwndList, LVM_SETITEM, 0, (LPARAM) &lvi);
	}
}

void ShowFileList(NM_TREEVIEW *pnmtv)
{
  HPIITEM *hi;
	HTREEITEM Parent;
	HWND hwndList;
  int count;

	SetCurrentHPI(pnmtv->itemNew.hItem);

	if (!HPIInfo)
		return;

	if (!HPIInfo->Directory)
		return;

	hwndList = GetDlgItem(hwndMain, IDC_HPILIST);
		
	Parent = pnmtv->itemNew.hItem;

 	hi = (HPIITEM *) (pnmtv->itemNew.lParam);

  SendMessage(hwndList, WM_SETREDRAW, FALSE, 0);

	SendMessage(hwndList, LVM_DELETEALLITEMS, 0, 0);

  switch (hi->hfd->Version) {
    case HPI_V1 :
      ShowFileList1(hwndList, hi, Parent);
      break;
    case HPI_V2 :
      ShowFileList2(hwndList, hi, Parent);
      break;
  }

	for (count = 0; count < MAXCOLUMN; count++)
	  SendMessage(hwndList, LVM_SETCOLUMNWIDTH, count, LVSCW_AUTOSIZE_USEHEADER);
  SendDlgItemMessage(hwndMain, IDC_HPILIST, LVM_SORTITEMS, 0, (LPARAM) ListSortFunc);

  SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);
}

int ZLibDecompress(char *out, char *in, HPICHUNK *Chunk)
{
  z_stream zs;
  int result;

  zs.next_in = in;
  zs.avail_in = Chunk->CompressedSize;
  zs.total_in = 0;

  zs.next_out = out;
  zs.avail_out = Chunk->DecompressedSize;
  zs.total_out = 0;

  zs.msg = NULL;
  zs.state = NULL;
  zs.zalloc = Z_NULL;
  zs.zfree = Z_NULL;
  zs.opaque = NULL;

  zs.data_type = Z_BINARY;
  zs.adler = 0;
  zs.reserved = 0;

  result = inflateInit(&zs);
  if (result != Z_OK) {
    //printf("Error on inflateInit %d\nMessage: %s\n", result, zs.msg);
    return 0;
  }

  result = inflate(&zs, Z_FINISH);
  if (result != Z_STREAM_END) {
    //printf("Error on inflate %d\nMessage: %s\n", result, zs.msg);
    zs.total_out = 0;
  }

  result = inflateEnd(&zs);
  if (result != Z_OK) {
    //printf("Error on inflateEnd %d\nMessage: %s\n", result, zs.msg);
    return 0;
  }

	return zs.total_out;
}

int LZ77Decompress(char *out, char *in, HPICHUNK *Chunk)
{
	int x;
	int work1;
	int work2;
	int work3;
	int inptr;
	int outptr;
	int count;
	int done;
	char DBuff[4096];
	int DPtr;

	done = FALSE;

  inptr = 0;
	outptr = 0;
	work1 = 1;
	work2 = 1;
	work3 = in[inptr++];
	
	while (!done) {
	  if ((work2 & work3) == 0) {
  		out[outptr++] = in[inptr];
		  DBuff[work1] = in[inptr];
		  work1 = (work1 + 1) & 0xFFF;
		  inptr++;
		}
	  else {
  		count = *((unsigned short *) (in+inptr));
			inptr += 2;
			DPtr = count >> 4;
			if (DPtr == 0) {
				return outptr;
			}
			else {
				count = (count & 0x0f) + 2;
				if (count >= 0) {
					for (x = 0; x < count; x++) {
						out[outptr++] = DBuff[DPtr];
						DBuff[work1] = DBuff[DPtr];
						DPtr = (DPtr + 1) & 0xFFF;
		        work1 = (work1 + 1) & 0xFFF;
					}

				}
			}
		}
		work2 *= 2;
		if (work2 & 0x0100) {
			work2 = 1;
			work3 = in[inptr++];
		}
	}

	return outptr;

}

int Decompress(char *out, char *in, HPICHUNK *Chunk)
{
	int x;
	int Checksum;

	Checksum = 0;
	for (x = 0; x < Chunk->CompressedSize; x++) {
		Checksum += (unsigned char) in[x];
		if (Chunk->Encrypt)
		  in[x] = (in[x] - x) ^ x;
	}

	if (Chunk->Checksum != Checksum) {
		MsgPrintf(hwndMain, "Checksum error! Calculated: 0x%X  Actual: 0x%X\n", Checksum, Chunk->Checksum);
		return 0;
	}

	switch (Chunk->CompMethod) {
		case 1 : return LZ77Decompress(out, in, Chunk);
		case 2 : return ZLibDecompress(out, in, Chunk);
		default : return 0;
	}
}

LPSTR DecodeFileToMem1(HPIITEM *hi)
{
  HPICHUNK *Chunk;
	long *DeSize;
	int DeCount;
	int DeLen;
	int x;
	char *DeBuff;
	char *WriteBuff;
	int WriteSize;
	int WritePtr;
  HPIENTRY1 *Entry;

	int Offset;
	int Length;
	char FileFlag;

	if (!hi)
		return NULL;

  Entry = hi->E1;

	Offset = *((int *) (HPIInfo->Directory + Entry->CountOffset));
	Length = *((int *) (HPIInfo->Directory + Entry->CountOffset + 4));
	FileFlag = *(HPIInfo->Directory + Entry->CountOffset + 8);

	WriteBuff = GetMem(Length+1, FALSE);
	if (!WriteBuff) {
		MessageBox(hwndMain, "Unable to allocate extract buffer", szAppTitle, MB_OK);
		return NULL;
	}
	WriteBuff[Length] = 0;

	if (FileFlag) {
  	DeCount = Length / 65536;
  	if (Length % 65536)
		  DeCount++;
	  DeLen = DeCount * sizeof(int);

	  DeSize = GetMem(DeLen, FALSE);

  	ReadAndDecrypt(Offset, (char *) DeSize, DeLen);

	  Offset += DeLen;
	
	  WritePtr = 0;

	  for (x = 0; x < DeCount; x++) {
		  Chunk = GetMem(DeSize[x], FALSE);
		  ReadAndDecrypt(Offset, (char *) Chunk, DeSize[x]);
		  Offset += DeSize[x];

		  DeBuff = (char *) (Chunk+1);

		  WriteSize = Decompress(WriteBuff+WritePtr, DeBuff, Chunk);
		  WritePtr += WriteSize;

		  FreeMem(Chunk);
		}
	  FreeMem(DeSize);
	}
	else {
		// file not compressed
  	ReadAndDecrypt(Offset, WriteBuff, Length);
	}

	return WriteBuff;
}


LPSTR DecodeFileToMem2(HPIITEM *hi)
{
  HPICHUNK Chunk;
  char *block;
  char *sqsh;
  long outlen;
  long outsize;
  HPIENTRY2 *Entry;
  long dcheck;

  if (!hi)
		return NULL;

  if (hi->IsDir == 1)
    return NULL;

  Entry = hi->E2;

  block = GetMem(Entry->DecompressedSize+1, FALSE);
  block[Entry->DecompressedSize] = 0;
  fseek(hi->hfd->HPIFile, Entry->Start, SEEK_SET);

  dcheck = 0;
  if (Entry->CompressedSize) {
    outlen = 0;
    while (outlen < Entry->DecompressedSize) {
      fread(&Chunk, sizeof(Chunk), 1, hi->hfd->HPIFile); 

      sqsh = GetMem(Chunk.CompressedSize, FALSE);

      fread(sqsh, Chunk.CompressedSize, 1, hi->hfd->HPIFile); 
      outsize = Decompress(block+outlen, sqsh, &Chunk);

      if (outsize != Chunk.DecompressedSize) {
        MessageBox(hwndMain, "** error decompressing!", NULL, 0);
        break;
      }
      else {
        outlen += Chunk.DecompressedSize;
      }

      CheckCalc(&dcheck, block, outsize);
      FreeMem(sqsh);
    }
  }
  else {
    fread(block, Entry->DecompressedSize, 1, hi->hfd->HPIFile); 
    CheckCalc(&dcheck, block, Entry->DecompressedSize);
  }

  if (Entry->Checksum != dcheck)
    MsgPrintf(hwndMain, "Checksum error! Actual: 0x%08X  Calculated: 0x%08X\n", Entry->Checksum, dcheck);

  return block;
}

LPSTR DecodeFileToMem(HPIITEM *hi)
{
	HCURSOR SaveCursor;
  LPSTR result = NULL;

  if (!hi)
		return NULL;

	SaveCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

  switch (hi->hfd->Version) {
    case HPI_V1 :
      result = DecodeFileToMem1(hi);
      break;
    case HPI_V2 :
      result = DecodeFileToMem2(hi);
      break;
  }
	SetCursor(SaveCursor);

  return result;
}

void DecodeFile1(LPSTR FName, HPIITEM *hi)
{
  HPICHUNK *Chunk;
	long *DeSize;
	int DeCount;
	int DeLen;
	int x;
	char *DeBuff;
	char *WriteBuff;
	int WriteSize;
	FILE *Sub;
	int Offset;
	int Length;
	char FileFlag;
  HPIENTRY1 *Entry;

	if (!hi)
		return;
  Entry = hi->E1;

	Offset = *((int *) (HPIInfo->Directory + Entry->CountOffset));
	Length = *((int *) (HPIInfo->Directory + Entry->CountOffset + 4));
	FileFlag = *(HPIInfo->Directory + Entry->CountOffset + 8);

	Sub = fopen(FName, "wb");
	if (!Sub) {
		MessageBox(hwndMain, "Unable to open output file", szAppTitle, MB_OK);
		return;
	}

	if (FileFlag) {
	  DeCount = Length / 65536;
	  if (Length % 65536)
  		DeCount++;
  	DeLen = DeCount * sizeof(int);

	  DeSize = GetMem(DeLen, FALSE);

	  ReadAndDecrypt(Offset, (char *) DeSize, DeLen);

	  Offset += DeLen;
	
	  WriteBuff = GetMem(65536, FALSE);

	  for (x = 0; x < DeCount; x++) {
		  Chunk = GetMem(DeSize[x], FALSE);
		  ReadAndDecrypt(Offset, (char *) Chunk, DeSize[x]);
		  Offset += DeSize[x];

  		DeBuff = (char *) (Chunk+1);

  		WriteSize = Decompress(WriteBuff, DeBuff, Chunk);
	    fwrite(WriteBuff, 1, WriteSize, Sub);

		  FreeMem(Chunk);
		}
	  FreeMem(WriteBuff);
	  FreeMem(DeSize);
	}
	else {
		WriteBuff = GetMem(Length, FALSE);
		ReadAndDecrypt(Offset, WriteBuff, Length);
    fwrite(WriteBuff, 1, Length, Sub);
		FreeMem(WriteBuff);
	}
	fclose(Sub);
}

void DecodeFile2(LPSTR FName, HPIITEM *hi)
{
  HPICHUNK Chunk;
  HPIENTRY2 *Entry;
  FILE *Sub;
  char *block;
  char *sqsh;
  long outlen;
  long outsize;
  struct _utimbuf utb;
  long dcheck;

	if (!hi)
		return;
  Entry = hi->E2;

	Sub = fopen(FName, "wb");
	if (!Sub) {
		MessageBox(hwndMain, "Unable to open output file", FName, MB_OK);
		return;
	}

  fseek(hi->hfd->HPIFile, Entry->Start, SEEK_SET);

  dcheck = 0;
  if (Entry->CompressedSize) {
    outlen = 0;
    while (outlen < Entry->DecompressedSize) {
      fread(&Chunk, sizeof(Chunk), 1, hi->hfd->HPIFile); 

      block = GetMem(Chunk.DecompressedSize, FALSE);
      sqsh = GetMem(Chunk.CompressedSize, FALSE);

      fread(sqsh, Chunk.CompressedSize, 1, hi->hfd->HPIFile); 
      outsize = Decompress(block, sqsh, &Chunk);

      if (outsize != Chunk.DecompressedSize) {
        MessageBox(hwndMain, "** error decompressing!", NULL, 0);
        break;
      }
      else {
        fwrite(block, Chunk.DecompressedSize, 1, Sub);
        outlen += Chunk.DecompressedSize;
      }
      CheckCalc(&dcheck, block, outsize);
      FreeMem(block);
      FreeMem(sqsh);
    }
  }
  else {
    block = malloc(Entry->DecompressedSize);
    fread(block, Entry->DecompressedSize, 1, hi->hfd->HPIFile); 
    fwrite(block, Entry->DecompressedSize, 1, Sub);
    CheckCalc(&dcheck, block, Entry->DecompressedSize);
    FreeMem(block);
  }
  if (Entry->Checksum != dcheck)
    MsgPrintf(hwndMain, "Checksum error! Actual: 0x%08X  Calculated: 0x%08X\n", Entry->Checksum, dcheck);
	fclose(Sub);
  utb.actime = Entry->Date;
  utb.modtime = Entry->Date;

  _utime(FName, &utb);
}

void DecodeFile(LPSTR FName, HPIITEM *hi)
{
	HCURSOR SaveCursor;

  if (!hi)
		return;

	SaveCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

  switch (hi->hfd->Version) {
    case HPI_V1 :
      DecodeFile1(FName, hi);
      return;
    case HPI_V2 :
      DecodeFile2(FName, hi);
      return;
  }
	SetCursor(SaveCursor);
}

void CloseTheFile(HPIFILEDATA *hfd)
{
	TV_ITEM tvi;
	HTREEITEM hItem;
  HPIITEM *hi;

	if (!hfd)
		return;

  SendDlgItemMessage(hwndMain, IDC_HPITREE, WM_SETREDRAW, FALSE, 0);

  hItem = (HTREEITEM) SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_GETNEXTITEM, TVGN_ROOT, (LPARAM) 0);

	while (hItem) {
		tvi.hItem = hItem;
		tvi.mask = TVIF_PARAM;
	  SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_GETITEM, 0, (LPARAM) &tvi);
    hi = (HPIITEM *) tvi.lParam;
		if (hi->hfd == hfd) {
			if (hItem == CurrentHPI)
      	SendDlgItemMessage(hwndMain, IDC_HPILIST, LVM_DELETEALLITEMS, 0, 0);
	    SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_DELETEITEM, 0, (LPARAM) hItem);
			if (hfd == HPIInfo) {
				HPIInfo = NULL;
				CurrentHPI = NULL;
			}
			break;
		}
  	hItem = (HTREEITEM) SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM) hItem);
	}

  if (hfd->Directory) {
 	  FreeMem(hfd->Directory);
    hfd->Directory = NULL;
  }
  if (hfd->Names) {
    FreeMem(hfd->Names);
    hfd->Names = NULL;
  }
  if (hfd->HPIFile)
    fclose(hfd->HPIFile);
  hfd->HPIFile = NULL;
	FreeMem(hfd);

  SendDlgItemMessage(hwndMain, IDC_HPITREE, WM_SETREDRAW, TRUE, 0);
}

void CloseAllFiles(void)
{
	TV_ITEM tvi;
	HTREEITEM hItem;
	HPIFILEDATA *hfd;
  HPIITEM *hi;

  HPIInfo = NULL;
	CurrentHPI = NULL;

  SendDlgItemMessage(hwndMain, IDC_HPITREE, WM_SETREDRAW, FALSE, 0);

	while (hItem = (HTREEITEM) SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_GETNEXTITEM, TVGN_ROOT, (LPARAM) 0)) {
		tvi.hItem = hItem;
		tvi.mask = TVIF_PARAM;
	  SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_GETITEM, 0, (LPARAM) &tvi);
		hi = (void *) tvi.lParam;
    hfd = hi->hfd;
    SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_DELETEITEM, 0, (LPARAM) hItem);
    if (hfd->Directory)
 	    FreeMem(hfd->Directory);
    hfd->Directory = NULL;
    if (hfd->Names)
      FreeMem(hfd->Names);
    hfd->Names = NULL;
    if (hfd->HPIFile)
      fclose(hfd->HPIFile);
    hfd->HPIFile = NULL;
	  FreeMem(hfd);
	}

  SendDlgItemMessage(hwndMain, IDC_HPITREE, WM_SETREDRAW, TRUE, 0);
 	SendDlgItemMessage(hwndMain, IDC_HPILIST, LVM_DELETEALLITEMS, 0, 0);
}

void CloseCurrentFile(void)
{
	if (HPIInfo)
		CloseTheFile(HPIInfo);
}

int AlreadyOpen(char *fname)
{

	HTREEITEM hItem;
	TV_ITEM tvi;
	char buff[1024];
	
  hItem = (HTREEITEM) SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_GETNEXTITEM, TVGN_ROOT, (LPARAM) 0);

	while (hItem) {
		tvi.hItem = hItem;
		tvi.mask = TVIF_TEXT;
		tvi.pszText = buff;
		tvi.cchTextMax = sizeof(buff);

	  SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_GETITEM, 0, (LPARAM) &tvi);
		if (stricmp(fname, buff) == 0) {
      SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_SELECTITEM, TVGN_CARET, (LPARAM) hItem);
			return TRUE;
		}
		hItem = (HTREEITEM) SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM) hItem);
	}

	return FALSE;
}

void ProcessDirectory1(HPIFILEDATA *hfd, char *StartPath, int offset, HTREEITEM Parent)
{
	int *Entries;
	HPIENTRY1 *Entry;
	int count;
	char *Name;
	int *FileCount;
	int *EntryOffset;
	char MyPath[255];
	TV_INSERTSTRUCT tvis;
	HTREEITEM ThisItem;
  HPIITEM *hi;

	Entries = (int *) (hfd->Directory + offset);
	EntryOffset = Entries + 1;
	Entry = (HPIENTRY1 *) (hfd->Directory + *EntryOffset);

  for (count = 0; count < *Entries; count++) {
		Name = hfd->Directory + Entry->NameOffset;
		FileCount = (int *) (hfd->Directory + Entry->CountOffset);
		if (Entry->Flag == 1)	{
    	strcpy(MyPath, StartPath);
			if (*StartPath)
			  strcat(MyPath, "\\");
			strcat(MyPath, Name);
			//_mkdir(MyPath);

      hi = GetMem(sizeof(HPIITEM), TRUE);

      hi->hfd = hfd;
      hi->IsDir = 1;
      hi->Size = 0;
      hi->Name = Name;
      hi->E1 = Entry;

			tvis.hParent = Parent;
			tvis.hInsertAfter = TVI_SORT;
			tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
			tvis.item.pszText = Name;
			tvis.item.cchTextMax = strlen(Name);
			tvis.item.lParam = (LPARAM) hi;

			ThisItem = (HTREEITEM) SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_INSERTITEM, 0, (LPARAM) &tvis);

			ProcessDirectory1(hfd, MyPath, Entry->CountOffset, ThisItem);
		}
		Entry++;
	}
}

void BuildDirectoryTree1(HPIFILEDATA *hfd)
{

	HTREEITEM ThisItem;
	TV_INSERTSTRUCT tvis;
	int start;
	int size;
  HPIITEM *hi;

	if (!hfd)
		return;

	start = hfd->H1.Start;
	size = hfd->H1.DirectorySize;

	hfd->Directory = GetMem(size, TRUE);

	HPIInfo = hfd;
	CurrentHPI = NULL;

	ReadAndDecrypt(start, hfd->Directory + start, size - start);

  SendDlgItemMessage(hwndMain, IDC_HPITREE, WM_SETREDRAW, FALSE, 0);

  hi = GetMem(sizeof(HPIITEM), TRUE);

  hi->hfd = hfd;
  hi->IsDir = 1;
  hi->Size = 0;
  hi->Name = NULL;
  hi->E1 = NULL;

  tvis.hParent = NULL;
  tvis.hInsertAfter = TVI_SORT;
	tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
	tvis.item.pszText = hfd->HPIFileName;
	tvis.item.cchTextMax = strlen(hfd->HPIFileName);
	tvis.item.lParam = (LPARAM) hi;

	ThisItem = (HTREEITEM) SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_INSERTITEM, 0, (LPARAM) &tvis);
  SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_SELECTITEM, TVGN_CARET, (LPARAM) ThisItem);

  ProcessDirectory1(hfd, "", start, ThisItem);
  
	SendDlgItemMessage(hwndMain, IDC_HPITREE, WM_SETREDRAW, TRUE, 0);
}

void ProcessDirectory2(HPIFILEDATA *hfd, char *StartPath, HPIDIR2 *hd, HTREEITEM Parent)
{
  HPIDIR2 *Sub;
  int count;
	TV_INSERTSTRUCT tvis;
	HTREEITEM ThisItem;
	char MyPath[255];
  char *Name;
  HPIITEM *hi;

  if (hd->SubCount) {
    Sub = (HPIDIR2 *) (hfd->Directory + hd->FirstSubDir);
    for (count = 0; count < hd->SubCount; count++) {
      Name = hfd->Names+Sub[count].NamePtr;

      strcpy(MyPath, StartPath);
			if (*StartPath)
			  strcat(MyPath, "\\");
			strcat(MyPath, Name);
			//_mkdir(MyPath);

      hi = GetMem(sizeof(HPIITEM), TRUE);

      hi->hfd = hfd;
      hi->IsDir = 1;
      hi->Size = 0;
      hi->Name = Name;
      hi->D2 = (HPIDIR2 *) Sub+count;

      tvis.hParent = Parent;
			tvis.hInsertAfter = TVI_SORT;
			tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
			tvis.item.pszText = Name;
			tvis.item.cchTextMax = strlen(Name);
			tvis.item.lParam = (LPARAM) hi;

			ThisItem = (HTREEITEM) SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_INSERTITEM, 0, (LPARAM) &tvis);
      ProcessDirectory2(hfd, MyPath, Sub+count, ThisItem);
    }
  }
  /*if (hd->FileCount) {
    Entry = (HPIENTRY2 *) (hfd->H2.Directory + hd->FirstFile); 
    for (count = 0; count < hd->FileCount; count++) {
      //ProcessFile2(NewPath, Entry+count, Dir, Name);
    }
  }*/
}

void BuildDirectoryTree2(HPIFILEDATA *hfd)
{
	HTREEITEM ThisItem;
	TV_INSERTSTRUCT tvis;
  char *block;
  HPICHUNK *Chunk;
  int outsize;
  HPIITEM *hi;

	if (!hfd)
		return;

  block = GetMem(hfd->H2.DirSize, FALSE);
  
  fseek(hfd->HPIFile, hfd->H2.DirBlock, SEEK_SET);
  fread(block, hfd->H2.DirSize, 1, hfd->HPIFile);

  Chunk = (void *) block;
  if (Chunk->Marker == HEX_SQSH) {
    hfd->Directory = GetMem(Chunk->DecompressedSize, FALSE);
    outsize = Decompress(hfd->Directory, block+sizeof(HPICHUNK), Chunk);
    if (outsize != Chunk->DecompressedSize) {
      MessageBox(hwndMain, "Error decompressing dir block", NULL, MB_OK);
      return;
    }
    FreeMem(block);
  }
  else {
    hfd->Directory = block;
  }

  block = GetMem(hfd->H2.NameSize, FALSE);
  
  fseek(hfd->HPIFile, hfd->H2.NameBlock, SEEK_SET);
  fread(block, hfd->H2.NameSize, 1, hfd->HPIFile);
  
  Chunk = (void *) block;
  if (Chunk->Marker == HEX_SQSH) {
    hfd->Names = GetMem(Chunk->DecompressedSize, FALSE);
    outsize = Decompress(hfd->Names, block+sizeof(HPICHUNK), Chunk);
    if (outsize != Chunk->DecompressedSize) {
      MessageBox(hwndMain, "Error decompressing name block.", NULL, MB_OK);
      return;
    }
    FreeMem(block);
  }
  else {
    hfd->Names = block;
  }
 
	HPIInfo = hfd;
	CurrentHPI = NULL;

  SendDlgItemMessage(hwndMain, IDC_HPITREE, WM_SETREDRAW, FALSE, 0);

  hi = GetMem(sizeof(HPIITEM), TRUE);

  hi->hfd = hfd;
  hi->IsDir = 1;
  hi->Size = 0;
  hi->Name = NULL;
  hi->D2 = (HPIDIR2 *) hfd->Directory;

	tvis.hParent = NULL;
  tvis.hInsertAfter = TVI_SORT;
	tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
	tvis.item.pszText = hfd->HPIFileName;
	tvis.item.cchTextMax = strlen(hfd->HPIFileName);
	tvis.item.lParam = (LPARAM) hi;

	ThisItem = (HTREEITEM) SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_INSERTITEM, 0, (LPARAM) &tvis);
  SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_SELECTITEM, TVGN_CARET, (LPARAM) ThisItem);

  ProcessDirectory2(hfd, "", (HPIDIR2 *) hfd->Directory, ThisItem);
  
	SendDlgItemMessage(hwndMain, IDC_HPITREE, WM_SETREDRAW, TRUE, 0);
}

HPIFILEDATA *OpenSingleFile(LPSTR buff)
{
	HPIFILEDATA *hfd;
  HPIVERSION hv;

  if (AlreadyOpen(buff))
		return NULL;

 	hfd = GetMem(sizeof(HPIFILEDATA), TRUE);
  if (!hfd) {
    MessageBox(hwndMain, "Unable to allocate memory", buff, MB_OK);
    return NULL;
  }
  strcpy(hfd->HPIFileName, buff);

	hfd->HPIFile = fopen(hfd->HPIFileName, "rb");
  if (!hfd->HPIFile) {
    MessageBox(hwndMain, "Error opening file", buff, MB_OK);
	  CloseTheFile(hfd);
    FreeMem(hfd);
		return NULL;
	}
			
  fread(&hv, sizeof(HPIVERSION), 1, hfd->HPIFile);

  if (hv.HPIMarker != HEX_HAPI) {  /* 'HAPI' */
  	MessageBox(hwndMain, "Not an HPI-format file.", buff, MB_OK);
    CloseTheFile(hfd);
    FreeMem(hfd);
    return NULL;
	}
  switch (hv.Version) {
    case HEX_BANK :  /* 'BANK' */
      MessageBox(hwndMain, "That is a saved game.", buff, MB_OK);
      CloseTheFile(hfd);
      FreeMem(hfd);
	    return NULL;
    case HPI_V1 : /* TA */
      hfd->Version = hv.Version;
      fread(&hfd->H1, sizeof(HPIHEADER1), 1, hfd->HPIFile);
      if (hfd->H1.Key)
        hfd->Key = ~((hfd->H1.Key * 4)	| (hfd->H1.Key >> 6));
      else
        hfd->Key = 0;
      BuildDirectoryTree1(hfd);
      EnableMenus(MF_ENABLED);
      break;
    case HPI_V2 : /* TAK */
      hfd->Version = hv.Version;
      fread(&hfd->H2, sizeof(HPIHEADER2), 1, hfd->HPIFile);
      BuildDirectoryTree2(hfd);
      EnableMenus(MF_ENABLED);
      break;
    default :
    	MessageBox(hwndMain, "Unknown HPI format.", buff, MB_OK);
      CloseTheFile(hfd);
      FreeMem(hfd);
      return NULL;
  }
	return hfd;
}

void OpenTheFile(void)
{
  OPENFILENAME ofn;
  char buff[1024];
	char NameList[10240];
	char *CurFile;
	int ofs;

	NameList[0] = 0;
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hwndMain;
  ofn.lpstrFilter = HPIFilter;
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = NameList;
	ofn.lpstrInitialDir = CurrentDirectory;
  ofn.nMaxFile = sizeof(NameList);
  ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | 
              OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NODEREFERENCELINKS;

  if (!GetOpenFileName(&ofn))
    return;

	UpdateWindow(hwndMain);

	ofs = ofn.nFileOffset;
	if (ofs > 0) {
		if (NameList[ofs-1]) {
		  NameList[ofs-1] = 0;
			// Make sure there are 2 nulls at end
			CurFile = NameList+ofs;
			while (*CurFile++);
			*CurFile = 0;
		}
	}

	CurFile = NameList+ofs;
	ofs = strlen(NameList)-1;
  if (NameList[ofs] == '\\') {
		NameList[ofs] = 0;
  }

	while (*CurFile) {
		strcpy(buff, NameList);
		strcat(buff, "\\");
		strcat(buff, CurFile);
		if (OpenSingleFile(buff)) {
	    strcpy(CurrentDirectory, buff);
	    CurrentDirectory[ofn.nFileOffset] = 0;
		}
		while (*CurFile++);
	}

	EnableMenus(MF_ENABLED);
}

int OpenSaveFile(char *Name, const char *filter, char *defext)
{
  OPENFILENAME ofn;

  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hwndMain;
  ofn.lpstrFilter = filter;
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = Name;
	ofn.lpstrInitialDir = CurrentSaveDirectory;
  ofn.nMaxFile = MAX_PATH;
  ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | 
              OFN_HIDEREADONLY | OFN_NOTESTFILECREATE;
	ofn.lpstrDefExt = defext;

  if (!GetSaveFileName(&ofn))
    return FALSE;

	UpdateWindow(hwndMain);

	strcpy(CurrentSaveDirectory, Name);
	CurrentSaveDirectory[ofn.nFileOffset] = 0;

	return TRUE;
}

void ExtractFile(HPIITEM *hi)
{
	char SaveName[MAX_PATH];

	if (!hi)
		return;

	strcpy(SaveName, hi->Name);
	if (!OpenSaveFile(SaveName, szSaveFilter, NULL))
		return;

	DecodeFile(SaveName, hi);
}

void ViewFile(HPIITEM *hi, int defoption)
{
	char *buff;
	char *Name;
	int option;
	int Length;
	int result = FALSE;
	HCURSOR hcSave;

	if (!hi)
		return;

	if (defoption == -1)
	  option = FindFileType(hi->Name);
	else
		option = defoption;

	buff = DecodeFileToMem(hi);

	if (!buff)
	  return;

  Name = hi->Name;
	Length = hi->Size;

	if (option == VIEW_WAV) {
		hcSave = SetCursor(LoadCursor(NULL, IDC_WAIT));
		result = PlaySound(buff, NULL, SND_MEMORY);
		SetCursor(hcSave);
		if (result) {
  		FreeMem(buff);
			return;
		}
		MessageBox(hwndMain, "Unable to play sound", Name, MB_OK);
		UpdateWindow(hwndMain);
		option = VIEW_HEX;
	}

	switch (option) {
		case VIEW_TEXT :
			result = ViewTextFile(buff, Length, Name, option, hi);
			break;
		case VIEW_SCT :
		case VIEW_BMP :
    case VIEW_JPG :
		case VIEW_PCX :
			result = ViewBmpFile(buff, Length, Name, option, hi);
			break;
		case VIEW_TNT :
			result = ViewTntFile(buff, Length, Name, option, hi);
			break;
		case VIEW_GAF :
			result = ViewGafFile(buff, Length, Name, option, hi);
			break;
		case VIEW_COB :
		//	result = ViewCobFile(buff, Length, Name, option, hi);
			break;
		case VIEW_3DO :
		//	result = View3DOFile(buff, Length, Name, option, hi);
			break;
		default :
			result = FALSE;
	}
	if (!result)
		result = ViewHexFile(buff, Length, Name, option, hi);

}

void ExtractOrView(int idItem)
{
	HPIITEM *hi;
	LV_ITEM lvi;
	HWND hwnd = GetDlgItem(hwndMain, IDC_HPILIST);
	int item;
	int ItemCount;

	ItemCount = SendMessage(hwnd, LVM_GETITEMCOUNT, 0, 0) ;
	if (ItemCount == 0)
		return;

	if (SendMessage(hwnd, LVM_GETSELECTEDCOUNT, 0, 0) == 0)
		return;

	for (item = 0; item < ItemCount; item++) {
		lvi.iItem = item;
		lvi.iSubItem = 0;
		lvi.mask = LVIF_PARAM | LVIF_STATE;
		lvi.stateMask = LVIS_SELECTED;
		SendMessage(hwnd, LVM_GETITEM, 0, (LPARAM) &lvi);
		if (lvi.state & LVIS_SELECTED) {
			hi = (HPIITEM *) lvi.lParam;
    	if (hi->IsDir == 0) {
				switch (idItem) {
  				case IDM_FILEEXTRACT :
	  	      ExtractFile(hi);
						break;
					case IDM_FILEVIEW :
  			    ViewFile(hi, -1);
						break;
					case IDM_HEXDUMP :
						ViewFile(hi, VIEW_HEX);
						break;
					case IDM_VIEWTEXT :
						ViewFile(hi, VIEW_TEXT);
						break;
				}
			}
	    else {
		    //directory
			}
		}
	}
}


void ExtractAllDirectories1(char *StartPath, HPIFILEDATA *hfd, int offset)
{
	int *Entries;
	HPIENTRY1 *Entry;
	int count;
	char *Name;
	int *FileCount;
	int *FileLength;
	char *FileFlag;
	int *EntryOffset;
	char MyPath[255];
  HPIITEM hi;

	Entries = (int *) (hfd->Directory + offset);
	EntryOffset = Entries + 1;
	Entry = (HPIENTRY1 *) (hfd->Directory + *EntryOffset);

  hi.hfd = hfd;
  hi.D2 = NULL;
  hi.E2 = NULL;

  for (count = 0; count < *Entries; count++) {
		Name = hfd->Directory + Entry->NameOffset;
		FileCount = (int *) (hfd->Directory + Entry->CountOffset);
  	strcpy(MyPath, StartPath);
		if (*StartPath)
		  strcat(MyPath, "\\");
		strcat(MyPath, Name);
		if (Entry->Flag == 1)	{
    	//printf("Directory %s SubEntries %d Flag %d\n", Name, *FileCount, Entry->Flag);
			CreateDirectory(MyPath, NULL);
		  ExtractAllDirectories1(MyPath, hfd, Entry->CountOffset);
		}
		else {
		  FileLength = FileCount + 1;
			FileFlag = (char *)	(FileLength + 1);
  		//printf("File %s Data Offset %d Length %d Flag %d FileFlag %d\n", Name, *FileCount, *FileLength, Entry->Flag, *FileFlag);
      hi.IsDir = 0;
      hi.Name = Name;
      hi.Size = *FileLength;
      hi.E1 = Entry;

	    DecodeFile1(MyPath, &hi);
		}
		Entry++;
	}
}

void ExtractAllDirectories2(char *StartPath, HPIFILEDATA *hfd, HPIDIR2 *hd)
{
  HPIDIR2 *Sub;
  HPIENTRY2 *Entry;
  HPIITEM hi;
  int count;
  char NewPath[1024];
  char NewName[1024];

  strcpy(NewPath, StartPath);
  if (*(hfd->Names+hd->NamePtr)) {
    strcat(NewPath, "\\");
    strcat(NewPath, hfd->Names+hd->NamePtr);
  }


 	CreateDirectory(NewPath, NULL);

  if (hd->SubCount) {
    Sub = (HPIDIR2 *) (hfd->Directory + hd->FirstSubDir);
    for (count = 0; count < hd->SubCount; count++) {
      ExtractAllDirectories2(NewPath, hfd, Sub+count);
    }
  }
  if (hd->FileCount) {
    Entry = (HPIENTRY2 *) (hfd->Directory + hd->FirstFile); 
    hi.hfd = hfd;
    hi.D2 = NULL;
    hi.E1 = NULL;
    hi.IsDir = 0;
    for (count = 0; count < hd->FileCount; count++) {
      hi.Name = hfd->Names+Entry[count].NamePtr;
      hi.Size = Entry[count].DecompressedSize;
      hi.E2 = Entry+count;
      strcpy(NewName, NewPath);
      strcat(NewName, "\\");
      strcat(NewName, hi.Name);
      DecodeFile2(NewName, &hi);
    }
  }
}

void ExtractAllFiles(void)
{
	HCURSOR hc;

	if (SelectDirectory(CurrentSaveDirectory)) {
  	hc = SetCursor(LoadCursor(NULL, IDC_WAIT));
    switch (HPIInfo->Version) {
      case HPI_V1 :
	      ExtractAllDirectories1(CurrentSaveDirectory, HPIInfo, HPIInfo->H1.Start);
        break;
      case HPI_V2 :
	      ExtractAllDirectories2(CurrentSaveDirectory, HPIInfo, (HPIDIR2 *) HPIInfo->Directory);
        break;
    }
	  SetCursor(hc);
	}
}

void ProcessSort(NM_LISTVIEW *pnmlv)
{
  if (SortColumn == pnmlv->iSubItem)
    SortOrder = !SortOrder;
  else {
    SortColumn = pnmlv->iSubItem;
    SortOrder = 0;
  }
  SendDlgItemMessage(hwndMain, IDC_HPILIST, LVM_SORTITEMS, 0, (LPARAM) ListSortFunc);
}

void ShowExpandState(NM_TREEVIEW *pnmtv)
{
	TV_ITEM tvi;
	int NewImage = -1;

	switch (pnmtv->action) {
		case TVE_COLLAPSE :
			NewImage = IconClosed;
			break;
		case TVE_EXPAND :
			NewImage = IconOpen;
			break;
	}
	if (NewImage == -1)
		return;

	tvi.hItem = pnmtv->itemNew.hItem;
	tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvi.iImage = NewImage;
	tvi.iSelectedImage = NewImage;
	SendDlgItemMessage(hwndMain, IDC_HPITREE, TVM_SETITEM, 0, (LPARAM) &tvi); 
}

void DeleteTreeItem(NM_TREEVIEW *pnmtv)
{
  FreeMem((void *) pnmtv->itemOld.lParam);
}

// Handle WM_NOTIFY messages from treeview
LONG NotifyHPITree(HWND hwnd, UINT code, UINT idItem, LPNMHDR pnmh)
{
  NM_TREEVIEW *pnmtv;

  (LPNMHDR) pnmtv = pnmh;
	switch (code) {
		case TVN_SELCHANGED :
			ShowFileList(pnmtv);
			break;
		case TVN_ITEMEXPANDED :
			ShowExpandState(pnmtv);
			break;
    case TVN_DELETEITEM :
      DeleteTreeItem(pnmtv);
      break;
	}
  return 0;
}

void ViewOrExpand(NM_LISTVIEW *pnmlv)
{
	TV_ITEM tvi;
  HPIITEM *li;
  HPIITEM *ti;
	HWND hwndTree;
	HTREEITEM hti;

	li = FindSelectedEntry(LVIS_FOCUSED);
	if (!li)
		return;
	if (li->IsDir == 1) {
		hwndTree = GetDlgItem(hwndMain, IDC_HPITREE);

		hti = (HTREEITEM) SendMessage(hwndTree, TVM_GETNEXTITEM, TVGN_CARET, 0);
		hti = (HTREEITEM) SendMessage(hwndTree, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM) hti);
		while (hti) {
			tvi.hItem = hti;
			tvi.mask = TVIF_PARAM;
			SendMessage(hwndTree, TVM_GETITEM, 0, (LPARAM) &tvi);
      ti = (HPIITEM *) tvi.lParam;
			if (ti->Name == li->Name) {
				SendMessage(hwndTree, TVM_SELECTITEM, TVGN_CARET, (LPARAM) hti);
				return;
			}
			hti = (HTREEITEM) SendMessage(hwndTree, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM) hti);
		}
		//directory
	}
	else
		ViewFile(li, -1);
}

void PopUpListMenu(NM_LISTVIEW *pnmlv)
{
  
	LV_ITEM lvi;
	LV_HITTESTINFO lvh;
	HWND hwnd = GetDlgItem(hwndMain, IDC_HPILIST);
	int item;
	char FType[256];
	HPIITEM *hi;
	POINT pt;

	lvh.pt = pnmlv->ptAction;
	item = SendMessage(hwnd, LVM_HITTEST, 0, (LPARAM) &lvh) ;
	if (item < 0)
		return;

  lvi.iItem = item;
	lvi.iSubItem = 0;
  lvi.mask = LVIF_PARAM;
	SendMessage(hwnd, LVM_GETITEM, 0, (LPARAM) &lvi);
  hi = (HPIITEM *) lvi.lParam;
  if (hi->IsDir == 0) {
	  lvi.iItem = item;
	  lvi.iSubItem = 1;
	  lvi.mask = LVIF_TEXT;
	  lvi.pszText = FType;
	  lvi.cchTextMax = sizeof(FType);
	  SendMessage(hwnd, LVM_GETITEM, 0, (LPARAM) &lvi);
    pt = pnmlv->ptAction;
	  ClientToScreen(hwnd, &pt);
	  TrackPopupMenu(hmPopNormal, TPM_LEFTALIGN, pt.x, pt.y,	0, hwndMain, NULL);
  }
	return;
}

void DeleteListItem(HWND hwnd, NM_LISTVIEW *pnmlv)
{
  LV_ITEM lvi;
  HPIITEM *hi;

  hi = (HPIITEM *) pnmlv->lParam;
  if (!hi) {
    lvi.mask = LVIF_PARAM;
    lvi.iItem = pnmlv->iItem;
    lvi.iSubItem = 0;

    SendMessage(hwnd, LVM_GETITEM, 0, (LPARAM) &lvi);
    hi = (HPIITEM *) lvi.lParam;
  }

  FreeMem(hi);
}

// WM_NOTIFY messages from list
LONG NotifyHPIList(HWND hwnd, UINT code, UINT idItem, LPNMHDR pnmh)
{
  NM_LISTVIEW *pnmlv = (NM_LISTVIEW *) pnmh;

  switch (code) {
    case LVN_ITEMCHANGED :
      break;
    case LVN_COLUMNCLICK :
      ProcessSort(pnmlv);
      break;
		case NM_DBLCLK :
			ViewOrExpand(pnmlv);
			break;
		case NM_RCLICK :
			PopUpListMenu(pnmlv);
			break;
    case LVN_DELETEITEM :
      DeleteListItem(hwnd, pnmlv);
      break;
  }
  return 0;
}

LRESULT CALLBACK HelpAboutProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
  switch (message) {
    case WM_INITDIALOG : return TRUE;
    case WM_COMMAND :
      switch (LOWORD(wParam)) {
        case IDCANCEL :
        case IDOK : EndDialog(hDlg, 0);
                    return TRUE;
      }
                    
  }   
  return FALSE;
}

int InitPart2(void)
{
  char *c;
  HANDLE hi;
  int i;
  HWND hwndTree = GetDlgItem(hwndMain, IDC_HPITREE);

  GetModuleFileName(NULL, ININame, sizeof(ININame));
  c = strrchr(ININame, '.');
  if (c)
    *c = 0;
  strcat(ININame, ".INI");

	GetTADirectory(CurrentDirectory);
	strcpy(CurrentSaveDirectory, CurrentDirectory);

	GetPrivateProfileString(szAppName, "CurrentDirectory", CurrentDirectory, CurrentDirectory, sizeof(CurrentDirectory), ININame);
	GetPrivateProfileString(szAppName, "CurrentSaveDirectory", CurrentSaveDirectory, CurrentSaveDirectory, sizeof(CurrentSaveDirectory), ININame);

  hi = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_MAIN));
  SendMessage(hwndMain, WM_SETICON, ICON_BIG, (LPARAM) hi);

  hi = LoadImage(hThisInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
  SendMessage(hwndMain, WM_SETICON, ICON_SMALL, (LPARAM) hi);

  SendMessage(hwndMain, WM_SETTEXT, 0, (LPARAM) szAppTitle);

  for (i = 0; i < 256; i++) {
    TAColor[i] = PALETTERGB(TAPalette[i].peRed, TAPalette[i].peGreen, TAPalette[i].peBlue);
    TAKColor[i] = PALETTERGB(TAKPalette[i].peRed, TAKPalette[i].peGreen, TAKPalette[i].peBlue);
  }

	if (!CreateImages())
    return FALSE;

	InitList(GetDlgItem(hwndMain, IDC_HPILIST));

  hwndSplit = CreateSplit(hThisInstance, hwndMain, hwndTree, SPLIT_TYPE_VERT, IDC_SPLIT);

	hmPopNormal = LoadMenu(hThisInstance, MAKEINTRESOURCE(IDM_POPNORMAL));
	hmPopNormal = GetSubMenu(hmPopNormal, 0);

  hRichEd = LoadLibrary("RICHED32.DLL");
  return TRUE;
}

LRESULT WndProcMainInit2(HWND hwnd, UINT message, WPARAM wParam, LONG lParam)
{
	LPSTR CmdLine = (LPSTR) lParam;

  InitPart2();

	ShowWindow(hwndMain, SW_SHOWNORMAL);
	UpdateWindow(hwndMain);

	if (*CmdLine)
		OpenSingleFile(CmdLine);

  return FALSE;
}

LRESULT WndProcMainCreate(HWND hwnd, UINT message, WPARAM wParam, LONG lParam)
{
  HWND hwndChild;
  // WM_CREATE
  //hwndMain = hwnd;

  hwndChild = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, NULL,  
              (TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT |
               TVS_DISABLEDRAGDROP | TVS_SHOWSELALWAYS | WS_CHILD | WS_VISIBLE),
              2,2,240,192, hwnd, (HMENU) IDC_HPITREE, hThisInstance, NULL);
  hwndChild = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL,  
              (LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS |
              LVS_OWNERDRAWFIXED | WS_CHILD | WS_VISIBLE),
              152,2,240,194, hwnd, (HMENU) IDC_HPILIST, hThisInstance, NULL);

	return FALSE;
}

void AdjustSplit(HWND hwndCtl)
{
  HWND hwndTree = GetDlgItem(hwndMain, IDC_HPITREE);
  HWND hwndList = GetDlgItem(hwndMain, IDC_HPILIST);

	AdjustHSplit(hwndCtl, hwndTree, hwndList);
}

LRESULT WndProcMainCommand(HWND hwnd, UINT message, WPARAM wParam, LONG lParam)
{
  UINT idItem = LOWORD(wParam);
  UINT wNotifyCode = HIWORD(wParam);
  HWND hwndCtl = (HWND) lParam;

  switch (idItem) {
	  case IDM_FILEOPEN :
		  OpenTheFile();
      return FALSE;
		case IDM_FILECLOSE :
			CloseCurrentFile();
			return FALSE;
		case IDM_FILECLOSEALL :
			CloseAllFiles();
			return FALSE;
		case IDM_FILEEXIT :
      PostMessage(hwnd, WM_CLOSE, 0, 0);
      return 0;
		case IDM_VIEWTEXT :
		case IDM_HEXDUMP :
		case IDM_FILEVIEW :
		case IDM_FILEEXTRACT :
			ExtractOrView(idItem);
			return 0;
		case IDM_FILEEXTRACTALL :
			ExtractAllFiles();
			return 0;
		case IDM_HELPABOUT :
      DialogBox(hThisInstance, MAKEINTRESOURCE(IDD_ABOUT), hwndMain, HelpAboutProc);
      return 0;
    case IDC_SPLIT :
      AdjustSplit(hwndCtl);
      return 0;
  }
  return FALSE;
}

LRESULT WndProcMainNotify(HWND hwnd, UINT message, WPARAM wParam, LONG lParam)
{
  // Process WM_NOTIFY messages

  LPNMHDR pnmh = (LPNMHDR) lParam;
  UINT idItem = pnmh->idFrom;
  
  switch (idItem) {
    case IDC_HPITREE : 
      return NotifyHPITree(pnmh->hwndFrom, pnmh->code, idItem, pnmh);
    case IDC_HPILIST : 
      return NotifyHPIList(pnmh->hwndFrom, pnmh->code, idItem, pnmh);
  }
  return FALSE;
}

LRESULT WndProcMainSize(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
// wm_size
{
	int fwSizeType = wParam;      // resizing flag 
  int nWidth = LOWORD(lParam);  // width of client area 
  int nHeight = HIWORD(lParam); // height of client area 
	RECT rc;
  int w;
  int x;

  HWND hwndTree = GetDlgItem(hwnd, IDC_HPITREE);
  HWND hwndList = GetDlgItem(hwnd, IDC_HPILIST);

	if (fwSizeType == SIZE_MINIMIZED)
		return FALSE;
  GetWindowRect(hwndTree, &rc);
  //ConvertToClient(hwnd, &rc);
  w = rc.right - rc.left;
  x = w;
  if (w > nWidth)	{
	  MoveWindow(hwndTree, 0, 0, nWidth, nHeight, TRUE);
		return FALSE;
	}

//  GetWindowRect(hwndSplit, &rc);
//  ConvertToClient(hwnd, &rc);
//  w = rc.left-1;

	MoveWindow(hwndTree, 0, 0, w, nHeight, TRUE);
  MoveWindow(hwndSplit, x, 0, SPLITWIDTH, nHeight, TRUE);


  x += SPLITWIDTH;
  //GetWindowRect(hwndList, &rc);
  //ConvertToClient(hwnd, &rc);
  w = nWidth - x;
  MoveWindow(hwndList, x, 0, w, nHeight, TRUE);

	return FALSE;
}

LRESULT WndProcMainClose(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CloseAllFiles();
	WritePrivateProfileString(szAppName, "CurrentDirectory", CurrentDirectory, ININame);
	WritePrivateProfileString(szAppName, "CurrentSaveDirectory", CurrentSaveDirectory, ININame);
	DestroyWindow(hwnd);
  hwndMain = NULL;
  PostQuitMessage(0);
  return FALSE;
}

LRESULT CALLBACK WndProcMain(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
    case WM_COMMAND :
      return WndProcMainCommand(hwnd, message, wParam, lParam);
    case WM_NOTIFY : 
			return WndProcMainNotify(hwnd, message, wParam, lParam);
		case WM_SIZE :
			return WndProcMainSize(hwnd, message, wParam, lParam);
    case WM_DRAWITEM :
      return WndProcMainDrawItem(hwnd, message, wParam, lParam);
    case WM_MEASUREITEM :
      return WndProcMainMeasureItem(hwnd, message, wParam, lParam);
    case WM_CREATE :
      return WndProcMainCreate(hwnd, message, wParam, lParam);
    case WM_CLOSE :
      return WndProcMainClose(hwnd, message, wParam, lParam);
    case UM_INIT2 :
      return WndProcMainInit2(hwnd, message, wParam, lParam);
  }   
  return DefWindowProc(hwnd, message, wParam, lParam);
}

void RegisterClasses(void)
{
  WNDCLASSEX    wndclass;
  HICON hiSmall;
  HICON hiLarge;

  hiSmall = LoadImage(hThisInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
  hiLarge = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_MAIN));

  wndclass.cbSize = sizeof(wndclass);
  wndclass.style = CS_HREDRAW | CS_VREDRAW;
  wndclass.lpfnWndProc = WndProcMain;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = 0;
  wndclass.hInstance = hThisInstance;
  wndclass.hIcon = hiLarge;
  wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground = GetStockObject(LTGRAY_BRUSH);
  wndclass.lpszMenuName = MAKEINTRESOURCE(IDM_MAIN);
  wndclass.lpszClassName = szAppName;
  wndclass.hIconSm = hiSmall;
  if (!RegisterClassEx(&wndclass))
    MessageBox(NULL, "Error registering class", szAppTitle, MB_OK);

}

int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpszCmdLine, int nCmdShow)
{
  MSG	  msg;

	hThisInstance = hInstance;

  ProgHeap = HeapCreate(0, 250000, 0);
  if (!ProgHeap) {
    MessageBox(NULL, "Unable to allocate heap", szAppTitle, MB_OK);
    return 1;
  }

  RegisterClasses();

  InitCommonControls();
 
  hwndMain = CreateWindowEx(0, szAppName, szAppTitle, WS_OVERLAPPEDWINDOW, 
                CW_USEDEFAULT, CW_USEDEFAULT, 500, 400, 
                NULL, (HMENU) NULL, hThisInstance, NULL);

  if (!hwndMain) {
    MessageBox(NULL, "Unable to create window", szAppTitle, MB_OK);
    return 1;
  }

	PostMessage(hwndMain, UM_INIT2, 0, (LPARAM) lpszCmdLine);

  //ShowWindow(hwndMain, nCmdShow);
  //UpdateWindow(hwndMain);

  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return (msg.wParam);
}
