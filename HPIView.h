#define HPI_V1 0x00010000
#define HPI_V2 0x00020000

#pragma pack(1)

//typedef struct _HPIHEADER {
//  long HPIMarker;              /* 'HAPI' */
//  long SaveMarker;             /* 'BANK' if savegame */
//  long DirectorySize;                /* Directory size */
//  long Key;                    /* decode key */
//  long Start;                  /* offset of directory */
//} HPIHEADER;

typedef struct _HPIVERSION {
  long HPIMarker;              /* 'HAPI' */
  long Version;                /* 'BANK' if savegame, 0x00010000, or 0x000200 */
} HPIVERSION;

typedef struct _HPIHEADER1 {
  long DirectorySize;                /* Directory size */
  long Key;                    /* decode key */
  long Start;                  /* offset of directory */
} HPIHEADER1;

typedef struct _HPIENTRY1 {
  int NameOffset;
  int CountOffset;
  char Flag;
} HPIENTRY1;

typedef struct _HPICHUNK {
  long Marker;            /* always 0x48535153 (SQSH) */
  char Unknown1;          /* I have no idea what these mean */
	char CompMethod;				/* 1 = lz77, 2 = zlib */
	char Encrypt;           /* Is the chunk encrypted? */
  long CompressedSize;    /* the length of the compressed data */
  long DecompressedSize;  /* the length of the decompressed data */
  long Checksum;          /* check sum */
} HPICHUNK;

typedef struct _HPIHEADER2 {
  long DirBlock;
  long DirSize;
  long NameBlock;
  long NameSize;
  long Data;
  long Last78;
} HPIHEADER2;

typedef struct _HPIDIR2 {
  long NamePtr;
  long FirstSubDir;
  long SubCount;
  long FirstFile;
  long FileCount;
} HPIDIR2;

typedef struct _HPIENTRY2 {
  long NamePtr;
  long Start;
  long DecompressedSize;
  long CompressedSize; /* 0 = no compression */
  long Date;  /* date in time_t format */
  long Checksum;
} HPIENTRY2;

typedef struct _HPIFILEDATA {
  int Version;
	HPIHEADER1 H1;
  HPIHEADER2 H2;
	int Key;
	char HPIFileName[MAX_PATH];
  char *Names;
	char *Directory;
	FILE *HPIFile;
} HPIFILEDATA;

typedef struct _HPIITEM {
  HPIFILEDATA *hfd;
  int IsDir;
  char *Name;
  int Size;
  HPIENTRY1 *E1;
  HPIDIR2 *D2;
	HPIENTRY2 *E2;
} HPIITEM;

typedef struct _TNTHEADER {
  long IDVersion;
  long Width;
  long Height;
  long PtrMapData;
  long PtrMapAttr;
  long PtrTileGfx;
  long Tiles;
  long TileAnims;
  long PtrTileAnims;
  long SeaLevel;
  long PtrMiniMap;
  long unknown1;
  long pad1;
	long pad2;
	long pad3;
	long pad4;
} TNTHEADER;

typedef struct _TNTTILE {
	unsigned char data[1024];
} TNTTILE;

typedef struct _TNTANIM {
	long Sequence;
	char Name[128];
} TNTANIM;

typedef struct _GAFHEADER {
  long IDVersion;
  long Entries;
  long Unknown1;
} GAFHEADER;

typedef struct _GAFENTRY {
  short Frames;
  short Unknown1;
  long Unknown2;
  char Name[32];
} GAFENTRY;

typedef struct _GAFFRAMEENTRY {
  long PtrFrameTable;
  long Unknown1;
} GAFFRAMEENTRY;

typedef struct _GAFFRAMEDATA {
  short Width;
  short Height;
  short XPos;
  short YPos;
  char Unknown1;
  char Compressed;
  short FramePointers;
  long Unknown2;
  long PtrFrameData;
  long Unknown3;
} GAFFRAMEDATA;

typedef struct _SCTHEADER {
  long Version;
  long PtrMini;
  long NumTiles;
  long PtrTiles;
  long SectionWidth;
  long SectionHeight;
  long PtrSectionData;
} SCTHEADER;

#pragma pack()

extern HANDLE hThisInstance;
extern HINSTANCE hRichEd;
extern const char szAppName[];
extern const char szAppTitle[];
extern HWND hwndMain;

LPVOID GetMem(int size, int zero);

LPVOID ReallocMem(LPVOID mem, int size);

void FreeMem(LPVOID x);

LPSTR DupString(LPSTR x);

int RoundUp(int num, int divisor);
