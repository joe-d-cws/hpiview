#define SPLITWIDTH 3
#define SPLIT_TYPE_VERT 1
#define SPLIT_TYPE_HORZ 0

void ConvertToClient(HWND hwnd, RECT *rc);

HWND CreateSplit(HINSTANCE hinst, HWND hwndParent, HWND hwndSibling, int SplitType, int id);

void AdjustHSplit(HWND hwndCtl, HWND hwndRight, HWND hwndLeft);
