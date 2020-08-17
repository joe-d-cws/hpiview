
#define VIEW_UNKNOWN 0
#define VIEW_TEXT 1
#define VIEW_BMP 2
#define VIEW_PCX 3
#define VIEW_WAV 4
#define VIEW_TNT 5
#define VIEW_GAF 6
#define VIEW_HEX 7
#define VIEW_SCT 8
#define VIEW_COB 9
#define VIEW_3DO 10
#define VIEW_JPG 11
#define MAX_VIEW_TYPE 12


int ViewTextFile(char *buff, int Length, char *Name, int option, HPIITEM *hi);

int ViewBmpFile(char *buff, int Length, char *Name, int option, HPIITEM *hi);

int ViewHexFile(char *buff, int Length, char *Name, int option, HPIITEM *hi);

int ViewTntFile(char *buff, int Length, char *Name, int option, HPIITEM *hi);

int ViewGafFile(char *buff, int Length, char *Name, int option, HPIITEM *hi);

int ViewCobFile(char *buff, int Length, char *Name, int option, HPIITEM *hi);

//void ViewTheFile(char *buff, int Length, char *Name, int option, HPIITEM *hi);
