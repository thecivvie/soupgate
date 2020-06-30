/*
        SoupGate Open-Source License

        Copyright (c) 1999-2000 by Tom Torfs
        
        The SoupGate software and its documentation may freely be distributed
        and used for all purposes, provided no fee is charged other than to
        cover administration and distribution costs, in other words it may
        not be sold for profit.

        The SoupGate software may freely be modified. Source code need
        not be made available for modified versions or derived programs,
        but if it is not, at least a copy of this license must be included
        in the program or its documentation.

        Modified source code may be made available, provided this license
        remains included, intact and unmodified, and the fact that changes
        were made must clearly be identified in both the source code and
        documentation, and a reference must be provided as to where the
        original, unmodified version can be obtained.

        DISCLAIMER:  THE AUTHOR EXCLUDES ANY AND ALL IMPLIED
        WARRANTIES,  INCLUDING WARRANTIES OF MERCHANTABILITY
        AND FITNESS FOR A PARTICULAR PURPOSE.     THE AUTHOR
        MAKES NO WARRANTY OR REPRESENTATION,  EITHER EXPRESS
        OR IMPLIED,   WITH RESPECT TO THIS SOFTWARE,     ITS
        QUALITY,  PERFORMANCE,  MERCHANTABILITY,  OR FITNESS
        FOR A PARTICULAR PURPOSE.   THE AUTHOR SHALL HAVE NO
        LIABILITY FOR SPECIAL,  INCIDENTAL, OR CONSEQUENTIAL
        DAMAGES ARISING OUT OF  OR RESULTING FROM THE USE OR
        MODIFICATION OF THIS SOFTWARE.

	End of License
*/

/* SoupGate - Fidonet-Internet Gateway Software
   common header file */

/*...sversion info:0:*/
#define MAJORVERSION 1
#define MINORVERSION 5

//#define BUGFIX

#define BETA
//#define GAMMA
/*...e*/

/*...sinclude files:0:*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>

#ifdef __unix__

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#define DEFDIRSEP "/"

#define _fsopen(name,mode,flags) fopen(name,mode)
#define stricmp(a,b) strcasecmp(a,b)

static int inline memicmp(const void *a, const void *b, size_t n)
{
   const unsigned char *p1=a, *p2=b;
   int c1, c2;
   for (; n>0; n--)
   {
      c1 = tolower(*(p1++));
      c2 = tolower(*(p2++));
      if (c1!=c2)
         return c1 - c2;
   }
   return 0;
}

static void inline strupr(char *s)
{
   for (; *s; s++)
      *s = toupper(*s);
}

static void inline strlwr(char *s)
{
   for (; *s; s++)
      *s = tolower(*s);
}

#else

#ifdef __WATCOMC__
#include <direct.h>
#else
#include <dirent.h>
#endif
#include <sys\stat.h>

#include <share.h>
#include <io.h>

#define DEFDIRSEP "\\"

#endif

#define MDEBUG_AUTOCHECK
#include "mdebug.h"
#include "fdebug.h"
/*...e*/

/*...senums:0:*/
enum {MODE_IMPORT, MODE_EXPORT, MODE_PACK, MODE_CHECK, MODE_NONE};

enum {FORMAT_USENET, FORMAT_MAILBOX, FORMAT_MMDF, FORMAT_BINARY,
      FORMAT_UNKNOWN};

enum {EMAIL, NEWS, MAILTYPES};

enum {CAT_EMAIL, CAT_NEWS, CAT_LIST, MSGCATEGORIES};

enum {GATE_BOTH,GATE_IMPORT,GATE_EXPORT};

enum {ACTION_IGNORE, ACTION_KILL, ACTION_MOVE, ACTION_JUNK};

enum {MAILER_NONE, MAILER_ARCMAIL, MAILER_BINKLEY};

enum {ATTACH_IGNORE, ATTACH_DECODE, ATTACH_KILL};

enum {ENCODE_MIME, ENCODE_UU, ENCODE_NONE};

enum {BINK_NORMAL, BINK_CRASH, BINK_DIRECT,
      BINK_HOLD,
      BINK_ALLTYPES, BINK_NORMTYPES=BINK_HOLD};

enum {FILE_KEEP, FILE_KILL, FILE_TRUNCATE};

enum {KILL_NONE, KILL_ALL, KILL_EMAIL};

enum {NO_JUNK,JUNK_FROM,JUNK_SUBJ,JUNK_STR,JUNK_GROUP,JUNK_FROMTO,
      JUNK_IMPEX};

enum {JUNK_EMAIL, JUNK_NEWS, JUNK_ALL, JUNK_NONE};

enum {FLOW_PKT, FLOW_FILE, FLOW_USED};

enum {EMAIL_FROM, EMAIL_TO, EMAIL_CC, EMAIL_BCC, EMAIL_REPLY, EMAIL_QUOTE};

enum {INFO_NAME, INFO_EMAIL, INFO_NONE, INFO_BOTH};

enum {STATUS_ADMIN, STATUS_NORMAL, STATUS_READONLY, STATUS_BLACKLIST};

enum {COMMAND_NONE, COMMAND_SUBSCRIBE, COMMAND_UNSUBSCRIBE, COMMAND_CHANGEPASS};

enum {SOURCE_FIDO, SOURCE_INTERNET};

enum {STRIP_NONE, STRIP_ALL, STRIP_SOME};
/*...e*/
/*...s\35\defines:0:*/
#define EMAILBUFSIZE    128
#define NOSPAMBUFSIZE    64
#define PKTPWDBUFSIZE     9
#define ROUTEPWDBUFSIZE  64
#define ORIGINBUFSIZE    64
#define ORGANBUFSIZE    128
#define AREABUFSIZE     128
#define TZBUFSIZE        32
#define TEXTBUFSIZE     128
#define MSGDATESIZE      21
#define BUFSIZE         256
#define FILEBUFSIZE     768
#define LINEBUFSIZE   32767

#define FIDOBODYLINESIZE   255
#define FIDOKLUDGELINESIZE  78
#define MINSOUPLINESIZE     20      /* should be <= BUFSIZE */
#define MAXSOUPLINESIZE    199      /* should be <= BUFSIZE */
#define TEMPLATELINESIZE   128      /* should be < BUFSIZE */

#define MAXFLOWFILES   2048

#define MAXADDRMAPS    1024
#define MAXROUTES      1024
#define MAXAREAMAPS    2048

#define MAXALIASES     1024
#define MAXMAILLISTS   1024
#define MAXIGNORETEXTS  512
#define MAXLISTS         64

#define MAXJUNKFROMS   4096
#define MAXJUNKSUBJS   2048
#define MAXJUNKSTRS    2048
#define MAXJUNKGROUPS   512

#define MAXQUOTECHARS   256
#define MAXQUOTEMASKS   256

#define MAXPASTQCHARS    72

#define MAXEMAIL       1024
#define MAXXPOST       1024

#define MAXFIDODOMAINS  256
#define MAXCHARSETMAPS  256
/*...e*/
/*...sstructs \38\ typedefs:0:*/

/* disable structure alignment */
#if defined(__GNUC__)
   #define PACKED __attribute__((packed))
#else
   #define PACKED
   #if defined(_MSC_VER) || defined(_QC) || defined(__WATCOMC__)
      #pragma pack(1)
   #elif defined(__ZTC__)
      #pragma ZTC align 1
   #elif defined(__TURBOC__) && (__TURBOC__ > 0x202)
      #pragma option -a-
   #endif
#endif

/* FSC-39 (Type 2+) packet definition */

struct PACKETHEADER {
   unsigned short orignode,destnode,
                  year,month,day,
                  hour,minute,second,
                  baudrate,
                  version,
                  orignet,destnet;
   unsigned char  pcodelow,prevmajor,
                  password[8];
   unsigned short qmorigzone,qmdestzone,
                  auxnet,cwvalidate;
   unsigned char  pcodehigh,prevminor;
   unsigned short capword,
                  origzone,destzone,
                  origpoint,destpoint;
   unsigned long  extrainfo;
  } PACKED;

struct PACKETMSGHEADER {
   unsigned short version,
                  orignode,destnode,
                  orignet,destnet,
                  attrib,
                  cost;
  } PACKED;

/* FTS-1 / FrontDoor message definition */
struct MESSAGEHEADER {
   char from[36],
        to[36],
        subj[72],
        date[20];
   unsigned short timesread,
                  destnode,
                  orignode,
                  cost,
                  orignet,
                  destnet;
   unsigned long  datewritten,
                  datearrived;
   unsigned short replyto,
                  attrib,
                  replynext;
  } PACKED;

/* typedefs */

typedef struct {unsigned short zone,net,node,point;} FIDOADDR;

typedef char EMAILADDR[EMAILBUFSIZE];

typedef char TEXTBUF[TEXTBUFSIZE];

typedef char PASSWORD[ROUTEPWDBUFSIZE];

typedef char AREA[AREABUFSIZE];

typedef struct {AREA echomail; AREA usenet; int direction;} AREAMAP;

typedef struct {FIDOADDR fido; EMAILADDR email; EMAILADDR replyto;
                TEXTBUF name;} ADDRMAP;

typedef struct {FIDOADDR fido; EMAILADDR email; PASSWORD pwd;
                TEXTBUF mailsubj;} ROUTE;

typedef struct {unsigned short day,month,year;
                unsigned short hour,min,sec;} TIMESTRUC;

typedef struct {TEXTBUF alias; EMAILADDR email;} ALIAS;

typedef struct {AREA area; EMAILADDR postaddr; EMAILADDR fromaddr;
                TEXTBUF substr; TEXTBUF truncate; TEXTBUF truncend;
                TEXTBUF truncsubj;
                int authorinfo;
               } MAILLIST;

typedef struct {unsigned char type; char *file;} FLOWFILE;

typedef struct {char type; char *addr; char *name;} FULLEMAIL;

typedef struct {char qchar; unsigned char mincol,maxcol;
                            unsigned char minid,maxid;} QUOTECHAR;

typedef struct {unsigned char category; char *text;} JUNKDEF;

typedef struct {unsigned short minzone, maxzone;
                char *domain;} FIDODOMAIN;

typedef struct {char *fidocharset; char *rfccharset; int dir;} CHARSETMAP;

typedef struct userconfig {unsigned long crc32; /* of email */
                           unsigned long ofs;
                           struct userconfig *next;} USERCONFIG;

typedef struct userdata {EMAILADDR email;
                         TEXTBUF name;
                         int status;
                         TEXTBUF password;} USERDATA;

typedef struct {char *configname;
                char *listname;
                char *listaddress;
                char *listarea;
                char *password;
                unsigned char newstatus;
                char *keepername;
                char *keeperaddress;
                char *subscribeinfo;
                char *welcome;
                char *goodbye;
                char *passwordinfo;
                char *newpassword;
                char *readonly;
                char *blacklist;
                unsigned long numusers;
                /* hash table based on first letter of e-mail address
                   (1=a/A..26=z/Z; 0 = non-alphabetic first character) */
                USERCONFIG *users[27];
               } LISTCONFIG;

/*...e*/

/* SOUPGATE.C */

#ifndef SOUPGATE_C
/*...sconstants:0:*/
extern const char osstr[];

extern const char weekday[][4];
extern const char month[][4];

extern const char binkmailext[BINK_ALLTYPES][5];
extern const char binkflowext[BINK_ALLTYPES][5];
extern const char packetext[5];
extern const char messageext[5];

extern const char attachname[][8];
extern const char encodename[][5];

extern const char junkname[][8];

extern const char TYPENAME[2][5];
extern const char FORMATTYPE[2];

extern const char SCANDATEFILE[];

extern char AREAS[];
extern char REPLIES[];

extern const char NETMAILAREANAME[];
extern const char EMAILAREANAME[];
extern const char JUNKAREANAME[];
extern const char ARCHIVESAREANAME[];

extern const char RNEWSHEADER[];

extern const char UUCP[];

extern const char MIMEBOUNDARY[];
extern const char MIMEALPHABET[];
extern const signed char MIMEDECODE[];

extern const int ENCBYTESPERLINE[2];
/*...e*/
/*...sglobal variables:0:*/
extern FILE *logf;

extern char configfile[FILENAME_MAX];
extern char packetdir[FILENAME_MAX];

extern TEXTBUF defcharset;

extern long msgpos,bodypos,endpos,fpos;
extern int addsubj,isjunk,isownorg;

extern char *replyfile[MAILTYPES];
extern unsigned replyfileno[MAILTYPES];
extern long newmsg[MAILTYPES];

extern int mode,testmode,verbose,globalstats;

extern int msgformat;
extern int msgtype;
extern int isfattach;
extern long pktmsgcount;
extern long messagecount,emailcount,newscount,junkcount,ownorgcount,
       maillistcount,personalcount,filecount,ignoretextcount,
       listmsgcount, listcmdcount, listreplycount;
extern long messagesize;
extern long parts,partno,partbytes;
extern TIMESTRUC msgtime;
extern FIDOADDR fromaddr,destaddr;
extern char *msgfromname,*msgfromemail;
extern char *msgtoname,*msgtoemail;
extern char *msgreplyname,*msgreplyemail;
extern char *quotename,*quoteemail;
extern char *sender,*xfrom;
extern FULLEMAIL *emaillist;
extern int emaillistcount;
extern char **xpost;
extern int xpostcount;
extern char *msgsubj,*msgid,*replyid;
extern int validmsgid,validreplyid,validfidoorgaddr;
extern FIDOADDR msgidaddr,replyidaddr,fidoorgaddr;
extern char *msgdate;
extern char *msgorganization;
extern char *areaname,*fidoareaname;
extern char *fidocharset,*rfccharset;
extern char *mimetype, *mimecte, *tzinfo;
extern unsigned long fidomsgid,fidoreplyid;
extern int routing,routepwdfailed,routeno;
extern char *routemailpwd, *routemailsubj;

extern FIDOADDR gateaddr;
extern int addrmaps;
extern ADDRMAP **addrmap;
extern int mapfidoorg;
extern int transfidoorg;
extern char *nospamtext;
extern char *packetpwd;
extern unsigned long importlimit;
extern unsigned long maxpktsize;
extern int separatemsg;
extern unsigned short souplinewidth;
extern char *origin;
extern char *organization;
extern int ownorgaction;
extern char *ownorgarea;
extern int junkaction;
extern char *junkarea;
extern int junkfroms;
extern JUNKDEF *junkfrom;
extern int junksubjs;
extern JUNKDEF *junksubj;
extern int junkstrs;
extern JUNKDEF *junkstr;
extern int junkgroups;
extern JUNKDEF *junkgroup;
extern int junkfromto;
extern unsigned short exportmaxto, exportmaxxpost;
extern char *emailarea;
extern int areamaps;
extern AREAMAP **areamap;
extern int stripkludges[MSGCATEGORIES];
extern int strictfidomsgid;
extern int killorigin;
extern char *logfile;
extern char *timezonestr;
extern int quickscan;
extern int mailertype;
extern int exportallmail;
extern int decodedirall;
extern char *indir;
extern char *outdir;
extern char *netdir;
extern char *decodedir;
extern char *soupdir;
extern char *tempdir;
extern unsigned short basezone;
extern int attach[MAILTYPES];
extern int encodeformat;
extern int useqp;
extern int quoteto;
extern int quotechars;
extern QUOTECHAR *quotechar;
extern int quotemasks;
extern char **quotemask;
extern int routes;
extern ROUTE **route;
extern int aliases;
extern ALIAS **alias;
extern int maillists;
extern MAILLIST **maillist;
extern int ignoretexts;
extern char **ignoretext;
extern int lists;
extern LISTCONFIG *listconfig;
extern int flowfiles;
extern FLOWFILE *flowfilelist;
extern int fidodomains;
extern FIDODOMAIN *fidodomain;
extern int charsetmaps;
extern CHARSETMAP *charsetmap;
extern char *junkmailto;
extern int infoheader;
extern int infoinsert;
/*...e*/
#endif

/*...smemory functions:0:*/
/* allocate memory, check for NULL, and initialize to 0 */
void *memalloc(unsigned size);
/*...e*/
/*...smath functions:0:*/
/* calculate CRC-32 */
unsigned long crc32(unsigned char *buf, size_t len);

/* 0 = Sunday; 1 <= m <= 12, y > 1752 or so */
int dow(int y, int m, int d);
/*...e*/
/*...sconversion functions:0:*/
int mimevalue(int c);

int uuenc(int value);

/* classifies users based on first character
   1..26: alphabetic character a/A..z/Z
   0: non-alphabetic character
   this routine assumes contiguous letters */
int classifyuser(char *c);

int tohexdigit(int c);

int hexdigit(char c);

int hexbyte(char *value);
/*...e*/
/*...scomparison functions:0:*/
int emailcmp(char *mask, char *addr);

int substrcmp(char *substr, char *str);

char *wordfind(char *substr, char *str);

void trim(char *s);

int ismail(char *name, int len);

void replacemacro(char *buf, int maxlen, char *macro, char *value);
/*...e*/
/*...sfile functions:0:*/
int fncmp(char *f1, char *f2);

int exist(char *name);
long fcopy(char *source, char *dest, char *buffer, int bufsize);

unsigned long genfname(const char *path, const char *ext, char *filename);
unsigned short genfnamebase(const char *path, const char *base,
                            const char *ext, char *filename);
void renfname(char *filename);
int starname(char *fullpath, char *nameonly);

/* call once to initialize
   call after every MSG has been written */
void genreplyfile(int mailtype);

/* read a character from a file, and return '\0' on EOF */
char zgetc(FILE *fp);

/* read a nul-terminated string from a file */
size_t readasciiz(FILE *fp, char *s, size_t maxlen);

/* read a nul or CR-terminated string from a file
   returns 0 if end of ASCIIZ string reached,
   1 if \r reached
   2 if maxlen reached) */
int readasciicr(FILE *fp, char *s, size_t maxlen);

/* checks if directory specified in path exists, aborts otherwise */
void checkdir(const char *name, char *dir);

void logprintf(char *s, ...);
/*...e*/

/* CONFIG.C */

/*...sconfig I\47\O functions:0:*/
void configwarn(const char *text, long lineno, const char *fname);

int readconfigfile(char *configfile, char *argv0);
/*...e*/

/* LISTS.C */

/*...slist I\47\O functions:0:*/
int readlist(char *listconfigfile, long lineno, char *origname, char *argv0);

void packlists(void);
/*...e*/
/*...slist user functions:0:*/
int getuserdata(int listno, USERCONFIG *config, USERDATA *data);

/* triple pointers... fun --- feel free to pass NULL for the last 2 params */
int finduserdata(int listno, char *email, USERDATA *data,
                 USERCONFIG ***configptr, USERCONFIG **config);

void createuserdata(int listno, USERDATA *data);
/*...e*/
/*...slist process functions:0:*/
void sendlisttemplate(int listno, USERDATA *userdata, char *template);

/* only from MSG -> MSG; PKT -> MSG handled in exportmessage()
   returns 1 if message may be processed further; 0 if it shouldn't */
int sendlistmessage(int listno, FILE *sfp,
                    long bodypos, long endpos);

void processlistcommand(int listno, int listcommand);
/*...e*/

/* PKT.C */

/*...spkt write functions:0:*/
void writepktheader(FILE *fpkt);
void writepktmsgheader(int msgcategory, FILE *fpkt, char *fname,
                       int isjunk, int isownorg);
void writemsgbottom(int msgcategory, FILE *fpkt);
void writefileinfo(char *decfpath, char *decfname, int frenamed,
                   int decodefile, int dofile, long decodebytes,
                   int filesonly, FILE *fpkt);
void closepkt(FILE *fpkt);
/*...e*/

/* IMPORT.C */

/*...simport functions:0:*/

void import(void);
/*...e*/

/* EXPORT.C */

/*...sexport functions:0:*/
void msgidprint(FILE *fp, char *msgid,
                unsigned long fidomsgid, FIDOADDR *msgidaddr);

void exportmessage(FILE *msgf, int flow);

int exportpkt(FILE *fpkt, int flow);
FILE *createfattach(FIDOADDR *destfido, EMAILADDR destemail,
                    char *pktname, char *filename);

void binkexport(FIDOADDR *destfido, EMAILADDR destemail, int isroute);
void msgexport(void);
void pktexport(void);

void export(void);
/*...e*/
