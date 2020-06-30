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
   Copyright (c) 1997-2000 by Tom Torfs, all rights reserved */

#define SOUPGATE_C
#include "soupgate.h"

/* PUBLIC DATA */

/*...sconstants:0:*/
const char osstr[]
#if defined(__OS2__)
   = "OS/2";
#elif defined(__NT__)
   = "Win32";
#elif defined (__linux__)
   = "Linux";
#elif defined (__unix__)
   = "Unix";
#else
   = "DOS";
#endif

const char weekday[][4]
   = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

const char month[][4]
   = {"Jan","Feb","Mar","Apr","May","Jun",
      "Jul","Aug","Sep","Oct","Nov","Dec"};

const char binkmailext[BINK_ALLTYPES][5]
   = {".out",".cut",".dut",".hut"};

const char binkflowext[BINK_ALLTYPES][5]
   = {".flo",".clo",".dlo",".hlo"};

const char packetext[5]
   = ".pkt";

const char messageext[5]
   = ".msg";

const char soupmsgext[5]
   = ".msg";

const char attachname[][8] = {"ignored","decoded","killed"};

const char encodename[][5] = {"MIME","UU"};

const char junkname[][8]
   = {"no junk","from","subject","body","group","from=to","imp/exp"};

const char TYPENAME[2][5] = {"mail","news"};
const char FORMATTYPE[2] = {'b','B'};

const char SCANDATEFILE[] = "soupgate.dat";

const char AREAS[] = "AREAS";
const char REPLIES[] = "REPLIES";

const char NETMAILAREANAME[] = "NETMAIL";
const char EMAILAREANAME[] = "Email";
const char JUNKAREANAME[] = "JUNKMAIL";
const char ARCHIVESAREANAME[] = "ARCHIVES";

const char RNEWSHEADER[] = "#! rnews ";

const char UUCP[] = "UUCP";

const char MIMEBOUNDARY[] = "+=SoupGate=+";

const char MIMEALPHABET[]
   = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const signed char MIMEDECODE[]
   = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,63,-1,63,
      52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
      -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
      15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
      -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
      41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1};

const int ENCBYTESPERLINE[2]
   = {57,45};
/*...e*/
/*...sglobal variables:0:*/
FILE *logf = NULL;

char configfile[FILENAME_MAX] = "soupgate.cfg";
char packetdir[FILENAME_MAX] = "";

TEXTBUF defcharset = "us-ascii";

char *replyfile[MAILTYPES];
unsigned replyfileno[MAILTYPES];
long newmsg[MAILTYPES];

int mode,testmode,verbose,globalstats;

int msgformat;
int msgtype;
int isfattach;
long pktmsgcount;
long messagecount,emailcount,newscount,junkcount,ownorgcount,maillistcount,
     personalcount,filecount,ignoretextcount,
     listmsgcount, listcmdcount, listreplycount;
long messagesize;
long parts,partno,partbytes;
TIMESTRUC msgtime;
FIDOADDR fromaddr,destaddr;
char *msgfromname,*msgfromemail;
char *msgtoname,*msgtoemail;
char *msgreplyname,*msgreplyemail;
char *quotename,*quoteemail;
char *sender,*xfrom;
FULLEMAIL *emaillist;
int emaillistcount;
char **xpost;
int xpostcount;
char *msgsubj,*msgid,*replyid;
int validmsgid,validreplyid,validfidoorgaddr;
FIDOADDR msgidaddr,replyidaddr,fidoorgaddr;
char *msgdate;
char *msgorganization;
char *areaname,*fidoareaname;
char *fidocharset,*rfccharset;
char *mimetype,*mimecte,*tzinfo;
unsigned long fidomsgid,fidoreplyid;
int routing,routepwdfailed,routeno;
char *routemailpwd, *routemailsubj;

FIDOADDR gateaddr = {0,0,0,0};
int addrmaps = 0;
ADDRMAP **addrmap;
int mapfidoorg = 0;
int transfidoorg = 0;
char *nospamtext;
char *packetpwd;
unsigned long importlimit = 0;
unsigned long maxpktsize = 0;
int separatemsg = 1;
unsigned short souplinewidth = 72;
char *origin;
char *organization;
int ownorgaction = ACTION_IGNORE;
char *ownorgarea;
int junkaction = ACTION_IGNORE;
char *junkarea;
int junkfroms = 0;
JUNKDEF *junkfrom;
int junksubjs = 0;
JUNKDEF *junksubj;
int junkstrs = 0;
JUNKDEF *junkstr;
int junkgroups = 0;
JUNKDEF *junkgroup;
int junkfromto = 0;
unsigned short exportmaxto = 0, exportmaxxpost = 0;
char *emailarea;
int areamaps = 0;
AREAMAP **areamap;
int stripkludges[MSGCATEGORIES] = {STRIP_NONE, STRIP_NONE, STRIP_NONE};
int strictfidomsgid = 0;
int killorigin = KILL_NONE;
char *logfile;
char *timezonestr;
int quickscan = 0;
int mailertype = MAILER_NONE;
int exportallmail = 0;
int decodedirall = 0;
char *indir;
char *outdir;
char *netdir;
char *decodedir;
char *soupdir;
char *tempdir;
unsigned short basezone = 0;
int attach[MAILTYPES] = {ATTACH_DECODE,ATTACH_IGNORE};
int encodeformat = ENCODE_MIME;
int useqp = 1;
int quoteto = 0;
int quotechars = 0;
QUOTECHAR *quotechar;
int quotemasks = 0;
char **quotemask;
int routes = 0;
ROUTE **route;
int aliases = 0;
ALIAS **alias;
int maillists = 0;
MAILLIST **maillist;
int ignoretexts = 0;
char **ignoretext;
int lists = 0;
LISTCONFIG *listconfig;
int flowfiles = 0;
FLOWFILE *flowfilelist;
int fidodomains = 0;
FIDODOMAIN *fidodomain;
int charsetmaps = 0;
CHARSETMAP *charsetmap;
char *junkmailto;
int infoheader = INFO_NAME;
int infoinsert = INFO_NONE;
/*...e*/

/* PUBLIC FUNCTIONS */

/*...smemalloc:0:*/
/* allocate memory, check for NULL, and initialize to 0 */
void *memalloc(unsigned size)
{
   void *ptr;

   ptr = calloc(size,1);
   if (ptr==NULL)
   {
      fprintf(stderr,"Error: not enough memory\n");
      exit(255);
   }

   return ptr;
}
/*...e*/

/*...scrc32:0:*/
/* Crc - 32 BIT ANSI X3.66 CRC checksum */

#define UPDC32(octet,crc) (crc_32_tab[((crc)^((unsigned char)octet))&0xff]^((crc)>>8))

/**********************************************************************\
|* Demonstration program to compute the 32-bit CRC used as the frame  *|
|* check sequence in ADCCP (ANSI X3.66, also known as FIPS PUB 71     *|
|* and FED-STD-1003, the U.S. versions of CCITT's X.25 link-level     *|
|* protocol).  The 32-bit FCS was added via the Federal Register,     *|
|* 1 June 1982, p.23798.  I presume but don't know for certain that   *|
|* this polynomial is or will be included in CCITT V.41, which        *|
|* defines the 16-bit CRC (often called CRC-CCITT) polynomial.  FIPS  *|
|* PUB 78 says that the 32-bit FCS reduces otherwise undetected       *|
|* errors by a factor of 10^-5 over 16-bit FCS.                       *|
\**********************************************************************/

/* Copyright (C) 1986 Gary S. Brown.  You may use this program, or
   code or tables extracted from it, as desired without restriction.*/

/* First, the polynomial itself and its table of feedback terms.  The  */
/* polynomial is                                                       */
/* X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0 */
/* Note that we take it "backwards" and put the highest-order term in  */
/* the lowest-order bit.  The X^32 term is "implied"; the LSB is the   */
/* X^31 term, etc.  The X^0 term (usually shown as "+1") results in    */
/* the MSB being 1.                                                    */

/* Note that the usual hardware shift register implementation, which   */
/* is what we're using (we're merely optimizing it by doing eight-bit  */
/* chunks at a time) shifts bits into the lowest-order term.  In our   */
/* implementation, that means shifting towards the right.  Why do we   */
/* do it this way?  Because the calculated CRC must be transmitted in  */
/* order from highest-order term to lowest-order term.  UARTs transmit */
/* characters in order from LSB to MSB.  By storing the CRC this way,  */
/* we hand it to the UART in the order low-byte to high-byte; the UART */
/* sends each low-bit to hight-bit; and the result is transmission bit */
/* by bit from highest- to lowest-order term without requiring any bit */
/* shuffling on our part.  Reception works similarly.                  */

/* The feedback terms table consists of 256, 32-bit entries.  Notes:   */
/*                                                                     */
/*  1. The table can be generated at runtime if desired; code to do so */
/*     is shown later.  It might not be obvious, but the feedback      */
/*     terms simply represent the results of eight shift/xor opera-    */
/*     tions for all combinations of data and CRC register values.     */
/*                                                                     */
/*  2. The CRC accumulation logic is the same for all CRC polynomials, */
/*     be they sixteen or thirty-two bits wide.  You simply choose the */
/*     appropriate table.  Alternatively, because the table can be     */
/*     generated at runtime, you can start by generating the table for */
/*     the polynomial in question and use exactly the same "updcrc",   */
/*     if your application needn't simultaneously handle two CRC       */
/*     polynomials.  (Note, however, that XMODEM is strange.)          */
/*                                                                     */
/*  3. For 16-bit CRCs, the table entries need be only 16 bits wide;   */
/*     of course, 32-bit entries work OK if the high 16 bits are zero. */
/*                                                                     */
/*  4. The values must be right-shifted by eight bits by the "updcrc"  */
/*     logic; the shift must be unsigned (bring in zeroes).  On some   */
/*     hardware you could probably optimize the shift in assembler by  */
/*     using byte-swap instructions.                                   */

static unsigned long crc_32_tab[] = { /* CRC polynomial 0xedb88320 */
0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

unsigned long crc32(unsigned char *buf, size_t len)
{
   unsigned long oldcrc32;

   oldcrc32 = 0xFFFFFFFF;

   for (; len>0; buf++, len--)
      oldcrc32 = UPDC32(*buf, oldcrc32);

   return ~oldcrc32;
}
/*...e*/
/*...sdow:0:*/
/* 0 = Sunday; 1 <= m <= 12, y > 1752 or so
   (Copyright 1993, Tomohiko Sakamoto) */
int dow(int y, int m, int d)
{
   static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
   y -= m < 3;
   return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}

#ifdef ALTERNATE_DOW

/* sunday == 0 */
#define DOW_ADJ 6

/* 4-digit year, month base 1, day base 1 */
unsigned dow(unsigned y, unsigned m, unsigned d)
{
   if (m < 3)
   {
      m += 13;
      y--;
   }
   else
      m++;
   return (d + 26 * m / 10 + y + y / 4 - y / 100 + y / 400 + DOW_ADJ) % 7;
}

#endif
/*...e*/

/*...smimevalue:0:*/
int mimevalue(int c)
{
   if (c>=32 && c<=127)
      return MIMEDECODE[c-32];
   else
      return -1;
}
/*...e*/
/*...suuenc:0:*/
int uuenc(int value)
{
   return (value==0)?(96):(value+32);
}
/*...e*/
/*...sclassifyuser:0:*/
/* classifies users based on first character
   1..26: alphabetic character a/A..z/Z
   0: non-alphabetic character
   this routine assumes contiguous letters */
int classifyuser(char *c)
{
   if (*c>='a' && *c<='z')
      return *c - 'a' + 1;
   else if (*c>='A' && *c<='Z')
      return *c - 'A' + 1;
   else
      return 0;
}
/*...e*/

/*...stohexdigit:0:*/
int tohexdigit(int c)
{
   if (c<10)
      return c+'0';
   else
      return c-10+'A';
}
/*...e*/
/*...shexdigit:0:*/
int hexdigit(char c)
{
   if (c>='0' && c<='9')
      return c-'0';
   else if (c>='A' && c<='F')
      return c-'A'+10;
   else if (c>='a' && c<='f')
      return c-'a'+10;
   return 0;
}
/*...e*/
/*...shexbyte:0:*/
int hexbyte(char *value)
{
   return (hexdigit(value[0])<<4) | hexdigit(value[1]);
}
/*...e*/

/*...semailcmp:0:*/
int emailcmp(char *mask, char *addr)
{
   char *mat,*aat;
   int mlen1,mlen2,alen1,alen2;

   mat = strchr(mask,'@');

   aat = strchr(addr,'@');

   if ((mat==NULL && aat!=NULL)
    || (mat!=NULL && aat==NULL))
      return 0;

   if (mat==NULL)
   {
      mlen1 = strlen(mask);
      alen1 = strlen(addr);
   }
   else
   {
      mlen1 = mat - mask;
      alen1 = aat - addr;

      mat++;
      aat++;

      mlen2 = strlen(mat);
      alen2 = strlen(aat);
   }

   if (mask[0]=='*')
   {
      if (memicmp(addr+alen1-(mlen1-1),mask+1,mlen1-1)!=0)
         return 0;
   }
   else if (mask[mlen1-1]=='*')
   {
      if (memicmp(addr,mask,mlen1-1)!=0)
         return 0;
   }
   else
   {
      if (mlen1!=alen1 || memicmp(addr,mask,mlen1)!=0)
         return 0;
   }

   if (mat!=NULL)
   {
      if (mat[0]=='*')
      {
         if (memicmp(aat+alen2-(mlen2-1),mat+1,mlen2-1)!=0)
            return 0;
      }
      else if (mat[mlen2-1]=='*')
      {
         if (memicmp(aat,mat,mlen2-1)!=0)
            return 0;
      }
      else
      {
         if (mlen2!=alen2 || memicmp(aat,mat,mlen2)!=0)
            return 0;
      }
   }

   return 1;
}
/*...e*/
/*...ssubstrcmp:0:*/
int substrcmp(char *substr, char *str)
{
   int sslen = strlen(substr);
   if (sslen>0 && sslen<=strlen(str))
   {
      for (;*str!='\0';str++)
      {
         if (memicmp(str,substr,sslen)==0)
            return 1;
      }
   }
   return 0;
}
/*...e*/
/*...swordfind:0:*/
/* dot required to be part of word for newsgroup names */
static int isalnumdot(int c)
{
   return isalnum(c) || c=='.';
}

char *wordfind(char *substr, char *str)
{
   char *str2;
   int sslen = strlen(substr);
   if (sslen>0 && sslen<=strlen(str))
   {
      for (str2=str;*str2!='\0';str2++)
      {
         if (memicmp(str2,substr,sslen)==0
             && (str2==str || !isalnumdot(str2[-1]))
             && !isalnumdot(str2[sslen]))
            return str2;
      }
   }
   return NULL;
}
/*...e*/
/*...sismail:0:*/
int ismail(char *name, int len)
{
   int i;

   if (len>4 && name[len-4]=='.')
   {
      if (memicmp(name+len-4,packetext,3)==0)
         return 1;
      else if (isxdigit(name[len-1]))
      {
         if (strchr(name, '*')==NULL)
         {
            if (len<12)
               return 0;
            for (i=0; i<8; i++)
            {
               if (!isxdigit(name[len-5-i]))
                  return 0;
            }
         }
         for (i=0; i<7; i++)
         {
            if (memicmp(name+len-3, weekday[i], 2) == 0)
               return 1;
         }
      }
   }

   return 0;
}
/*...e*/
/*...strim:0:*/
void trim(char *s)
{
   int len;

   len = strlen(s);

   while (isspace(s[0]))
      memmove(s,s+1,len--);

   while (isspace(s[len-1]))
      len--;

   s[len] = '\0';
}
/*...e*/
/*...sreplacemacro:0:*/
void replacemacro(char *buf, int maxlen, char *macro, char *value)
{
   char *cp;
   int slen,mlen,vlen;

   if (value==NULL)
      value = "[unknown]";

   slen = strlen(buf);
   mlen = strlen(macro);
   vlen = strlen(value);

   cp = buf;
   while (slen+vlen-mlen<maxlen
          && (cp=strstr(cp,macro))!=NULL)
   {
      if (mlen!=vlen)
	 memmove(cp+vlen,cp+mlen,strlen(cp+mlen)+1);
      memcpy(cp,value,vlen);
      cp += vlen;
      slen += vlen - mlen;
   }
}
/*...e*/

/*...sfncmp:0:*/
int fncmp(char *f1, char *f2)
{
   char *c1,*c2;

#ifndef __unix__
   c1 = strrchr(f1,'\\');
   if (c1==NULL)
#endif
      c1 = strrchr(f1,'/');
   if (c1==NULL)
      c1 = f1;
   else
      c1++;
#ifndef __unix__
   c2 = strrchr(f2,'\\');
   if (c2==NULL)
#endif
      c2 = strrchr(f2,'/');
   if (c2==NULL)
      c2 = f2;
   else
      c2++;

   return stricmp(c1,c2);
}
/*...e*/

/*...sexist:0:*/
int exist(char *name)
{
   return access(name,0)==0;
}
/*...e*/
/*...sfcopy:0:*/
long fcopy(char *source, char *dest, char *buffer, int bufsize)
{
   FILE *d, *s;
   size_t incount;
   long totcount = 0L;

   s = _fsopen(source, "rb", SH_DENYNO);
   if (s == NULL)
      return -1L;

   d = _fsopen(dest, "wb", SH_DENYWR);
   if (d == NULL)
   {
      fclose(s);
      return -1L;
   }

   incount = fread(buffer, 1, bufsize, s);

   while (!feof(s))
   {
      totcount += (long) incount;
      fwrite(buffer, 1, bufsize, d);
      incount = fread(buffer, 1, bufsize, s);
   }

   totcount += (long) incount;
   fwrite(buffer, 1, incount, d);

   fclose(s);
   fclose(d);

   return totcount;
}
/*...e*/
/*...sgenfname:0:*/
unsigned long genfname(const char *path, const char *ext, char *filename)
{
   unsigned long fileno;

   for (fileno=0;;fileno++)
   {
      sprintf(filename,"%s%08lx%s",path,fileno,ext);
      if (!exist(filename))
         return fileno;
   }
}
/*...e*/
/*...sgenfnamebase:0:*/
unsigned short genfnamebase(const char *path, const char *base,
                            const char *ext, char *filename)
{
   unsigned short fileno;

   for (fileno=0;;fileno++)
   {
      sprintf(filename,"%s%s%04x%s",path,base,fileno,ext);
      if (!exist(filename))
         return fileno;
   }
}
/*...e*/
/*...srenfname:0:*/
void renfname(char *filename)
{
   char *cp;

   cp = filename + strlen(filename) - 1;

   while (exist(filename))
   {
      if (!isdigit(*cp))
         *cp = '0';
      else if (*cp!='9')
         (*cp)++;
      else
      {
         if (cp==filename)
            break;
         cp--;
         if (*cp=='.')
            cp--;
         if (
#ifndef __unix__
            *cp=='\\' || 
#endif
            *cp=='/')
            break;
      }
   }
}
/*...e*/
/*...sstarname:0:*/
int starname(char *fullpath, char *nameonly)
{
   char *cp;
   char ext[8];
   unsigned long fileno;

   if (strchr(nameonly, '*') == NULL)
      return 0;

   cp = strrchr(nameonly, '.');
   if (cp==NULL || strchr(cp+1, '*')!=NULL)
   {
      strcpy(ext, "ATT");
   }
   else
   {
      strncpy(ext, cp+1, sizeof ext - 1);
      ext[sizeof ext - 1] = '\0';
   }

   for (fileno=0;;fileno++)
   {
      sprintf(nameonly,"%08lx.%s",fileno,ext);
      if (!exist(fullpath))
	 return 1;
   }

   /* return 0; */
}
/*...e*/

/*...sgenreplyfile:0:*/
/* call once to initialize
   call after every MSG has been written */
void genreplyfile(int mailtype)
{
   FILE *fp;
   char fname[FILENAME_MAX];
   char tbuf[5];

   strcpy(tbuf, TYPENAME[mailtype]);
   strupr(tbuf);
 
   if (newmsg[mailtype])
   {
      strcpy(fname,soupdir);
      strcat(fname,REPLIES);
      fp = _fsopen(fname,"ab",SH_DENYWR);
      if (fp==NULL)
         logprintf("Error: can't append to %s\n",fname);
      else
      {
         fprintf(fp,"%s%04X\t%s\t%cn\n",
                    tbuf, replyfileno[mailtype],
                    TYPENAME[mailtype],
                    FORMATTYPE[mailtype]);
         fclose(fp);
      }
   }

   if (newmsg[mailtype] || replyfile[mailtype][0]=='\0')
   {
      replyfileno[mailtype] = genfnamebase(soupdir,tbuf,
                                           soupmsgext,replyfile[mailtype]);
      newmsg[mailtype] = 0;
   }
}
/*...e*/
   
/*...szgetc:0:*/
/* read a character from a file, and return '\0' on EOF */
char zgetc(FILE *fp)
{
   int ch=getc(fp);
   if (ch==EOF)
      ch = '\0';
   return (char)ch;
}
/*...e*/
/*...sreadasciiz:0:*/
/* read a nul-terminated string from a file */
size_t readasciiz(FILE *fp, char *s, size_t maxlen)
{
   size_t count;
   for (count=0; count<maxlen && (s[count]=zgetc(fp))!='\0'; count++) ;
   return count;
}
/*...e*/
/*...sreadasciicr:0:*/
/* read a nul or CR-terminated string from a file
   returns 0 if end of ASCIIZ string reached,
   1 if \r reached
   2 if maxlen reached)
   compensates for CRLF: if the first character of a
   line read is a newline (\n), it will be skipped
*/
int readasciicr(FILE *fp, char *s, size_t maxlen)
{
   size_t count;
   for (count=0; count<maxlen;)
   {
      s[count] = zgetc(fp);
      if (s[count]=='\r')
      {
         s[count] = '\0';
         return 1;
      }
      else if (s[count]=='\0')
      {
         return 0;
      }
      if (count>0 || s[count]!='\n')
         count++;
   }

   s[count] = '\0';
   return 2;
}
/*...e*/

/*...scheckdir:0:*/
void checkdir(const char *name, char *dir)
{
   char *cp;
   char origchar;

#ifndef __unix__
   cp = strrchr(dir, '\\');
   if (cp==NULL)
#endif
      cp = strrchr(dir, '/');
   if (cp!=NULL)
   {
      origchar = *cp;
      *cp = '\0';
   }

   if (!exist(dir))
   {
      logprintf("FATAL ERROR: subdirectory %s (%s) does not exist\n", dir, name);
      exit(EXIT_FAILURE);
   }

   if (cp!=NULL)
      *cp = origchar;
}
/*...e*/

/*...slogprintf:0:*/
void logprintf(char *s, ...)
{
   va_list ap;
   static char buf[200];

   va_start(ap,s);
   vsprintf(buf,s,ap);
   va_end(ap);

   fprintf(stderr,"%s",buf);
   if (logf!=NULL)
      fprintf(logf,"%s",buf);
}
/*...e*/

/* PRIVATE FUNCTIONS */

/*...susage:0:*/
static void usage(void)
{
   printf("Usage: (all commands & options can be abbreviated to their first letter)\n\n");

   printf("soupgate import      import mail from Internet -> Fidonet\n");
   printf("soupgate export      export mail from Fidonet -> Internet\n");
   printf("soupgate pack        pack mailinglist user files\n");
   printf("soupgate check       reads & checks all configuration files\n");

   printf("\nCommandline options:\n\n");

   printf("/pkt=<dir>           export *.pkt for gateway in directory\n");
   printf("/config=<filename>   use <filename> as configuration file\n");
   printf("/test                don't delete mail/files after processing\n");
   printf("/verbose             display from/to information for every mail\n");
   printf("/global              display global statistics rather than per file\n");

   exit(EXIT_FAILURE);
}
/*...e*/

int main(int argc, char *argv[])
{
/*...svariables:0:*/
   int i,len;
   char *cp;
/*...e*/

/*...stitle:0:*/
   printf("SoupGate-%s v%d.%02d - Fidonet/Internet gateway software\n",
           osstr,MAJORVERSION,MINORVERSION);
   printf("Copyright (c) 1997-2000 by Tom Torfs, all rights reserved\n\n");

#if defined(BETA)
   printf("Restricted beta version; compiled %s %s\n\n",__DATE__,__TIME__);
#elif defined(GAMMA)
   printf("Public gamma version; compiled %s %s\n\n",__DATE__,__TIME__);
#elif defined(BUGFIX)
   printf("Bugfix version; compiled %s %s\n\n",__DATE__,__TIME__);
#else
   printf("Release version; compiled %s %s\n\n",__DATE__,__TIME__);
#endif

   printf("This is FREEWARE. Read soupgate.doc for more information.\n\n");
/*...e*/
/*...scheck commandline parameters:0:*/
   mode = MODE_NONE;
   testmode = 0;
   verbose = 0;
   globalstats = 0;
   for (i=1; i<argc; i++)
   {
      if (stricmp(argv[i],"import")==0
          || stricmp(argv[i],"i")==0)
         mode = MODE_IMPORT;
      else if (stricmp(argv[i],"export")==0
          || stricmp(argv[i],"e")==0)
         mode = MODE_EXPORT;
      else if (stricmp(argv[i],"pack")==0
          || stricmp(argv[i],"p")==0)
         mode = MODE_PACK;
      else if (stricmp(argv[i],"check")==0
          || stricmp(argv[i],"c")==0)
         mode = MODE_CHECK;
      else if (argv[i][0]=='/' || argv[i][0]=='-')
      {
         if (stricmp(argv[i]+1,"test")==0
             || stricmp(argv[i]+1,"t")==0)
            testmode = 1;
         else if (stricmp(argv[i]+1,"verbose")==0
             || stricmp(argv[i]+1,"v")==0)
            verbose = 1;
         else if (stricmp(argv[i]+1,"global")==0
             || stricmp(argv[i]+1,"g")==0)
            globalstats = 1;
         else if (memicmp(argv[i]+1,"config=",7)==0)
         {
            strcpy(configfile,argv[i]+8);
         }
         else if (memicmp(argv[i]+1,"c=",2)==0)
         {
            strcpy(configfile,argv[i]+3);
         }
         else if (memicmp(argv[i]+1,"pkt=",4)==0)
         {
            strcpy(packetdir,argv[i]+5);
            len = strlen(packetdir);
            if (
#ifndef __unix__
                packetdir[len-1]!='\\' &&
#endif
                packetdir[len-1]!='/')
               strcat(packetdir,DEFDIRSEP);
         }
         else if (memicmp(argv[i]+1,"p=",2)==0)
         {
            strcpy(packetdir,argv[i]+3);
            len = strlen(packetdir);
             if (
#ifndef __unix__
                packetdir[len-1]!='\\' &&
#endif
                packetdir[len-1]!='/')
               strcat(packetdir,DEFDIRSEP);
         }
         else
            usage();
      }
      else
         usage();
   }

   if (mode==MODE_NONE
       || (packetdir[0]!='\0' && mode!=MODE_EXPORT))
      usage();

/*...e*/

/*...sallocate memory:0:*/
   for (i=0; i<MAILTYPES; i++)
      replyfile[i] = memalloc(FILENAME_MAX);
   msgfromname = memalloc(TEXTBUFSIZE);
   msgfromemail = memalloc(EMAILBUFSIZE);
   msgtoname = memalloc(TEXTBUFSIZE);
   msgtoemail = memalloc(EMAILBUFSIZE);
   msgreplyname = memalloc(TEXTBUFSIZE);
   msgreplyemail = memalloc(EMAILBUFSIZE);
   sender = memalloc(TEXTBUFSIZE);
   xfrom = memalloc(TEXTBUFSIZE);
   quotename = memalloc(TEXTBUFSIZE);
   quoteemail = memalloc(EMAILBUFSIZE);
   emaillist = memalloc(MAXEMAIL * sizeof(FULLEMAIL));
   xpost = memalloc(MAXXPOST * sizeof(char *));
   msgsubj = memalloc(TEXTBUFSIZE);
   msgid = memalloc(TEXTBUFSIZE);
   replyid = memalloc(TEXTBUFSIZE);
   msgdate = memalloc(MSGDATESIZE);
   msgorganization = memalloc(ORGANBUFSIZE);
   areaname = memalloc(AREABUFSIZE);
   fidoareaname = memalloc(AREABUFSIZE);
   routemailpwd = memalloc(ROUTEPWDBUFSIZE);
   routemailsubj = memalloc(TEXTBUFSIZE);
   addrmap = memalloc(MAXADDRMAPS * sizeof(ADDRMAP *));
   nospamtext = memalloc(NOSPAMBUFSIZE);
   packetpwd = memalloc(PKTPWDBUFSIZE);
   origin = memalloc(ORIGINBUFSIZE);
   organization = memalloc(ORGANBUFSIZE);
   ownorgarea = memalloc(AREABUFSIZE);
   junkarea = memalloc(AREABUFSIZE);
   junkfrom = memalloc(MAXJUNKFROMS * sizeof(JUNKDEF));
   junksubj = memalloc(MAXJUNKSUBJS * sizeof(JUNKDEF));
   junkstr = memalloc(MAXJUNKSTRS * sizeof(JUNKDEF));
   junkgroup = memalloc(MAXJUNKGROUPS * sizeof(JUNKDEF));
   emailarea = memalloc(AREABUFSIZE);
   areamap = memalloc(MAXAREAMAPS * sizeof(AREAMAP *));
   logfile = memalloc(FILENAME_MAX);
   timezonestr = memalloc(TZBUFSIZE);
   indir = memalloc(FILENAME_MAX);
   outdir = memalloc(FILENAME_MAX);
   netdir = memalloc(FILENAME_MAX);
   decodedir = memalloc(FILENAME_MAX);
   soupdir = memalloc(FILENAME_MAX);
   tempdir = memalloc(FILENAME_MAX);
   route = memalloc(MAXROUTES * sizeof(ROUTE *));
   alias = memalloc(MAXALIASES * sizeof(ALIAS *));
   maillist = memalloc(MAXMAILLISTS * sizeof(MAILLIST *));
   ignoretext = memalloc(MAXIGNORETEXTS * sizeof(char *));
   flowfilelist = memalloc(MAXFLOWFILES * sizeof(FLOWFILE));
   quotechar = memalloc(MAXQUOTECHARS * sizeof(QUOTECHAR));
   quotemask = memalloc(MAXQUOTEMASKS * sizeof(char *));
   fidodomain = memalloc(MAXFIDODOMAINS * sizeof(FIDODOMAIN));
   charsetmap = memalloc(MAXCHARSETMAPS * sizeof(CHARSETMAP));
   fidocharset = memalloc(TEXTBUFSIZE);
   rfccharset = memalloc(TEXTBUFSIZE);
   mimetype = memalloc(TEXTBUFSIZE);
   mimecte = memalloc(TEXTBUFSIZE);
   tzinfo = memalloc(TEXTBUFSIZE);
   junkmailto = memalloc(EMAILBUFSIZE);
   listconfig = memalloc(MAXLISTS * sizeof(LISTCONFIG));
/*...e*/

/*...sread configfile:0:*/
   cp = getenv("TZ");
   if (cp!=NULL)
      strncpy(timezonestr,cp,TZBUFSIZE);

   cp = getenv("TEMP");
   if (cp==NULL)
   {
#ifdef __unix__
      strcpy(tempdir, "./");
#else
      tempdir[0] = '\0';
#endif
   }
   else
   {
      strncpy(tempdir,cp,FILENAME_MAX);
      if (
#ifndef __unix__
          tempdir[strlen(tempdir)-1]!='\\' &&
#endif
          tempdir[strlen(tempdir)-1]!='/')
         strcat(tempdir, DEFDIRSEP);
   }

   strcpy(ownorgarea,ARCHIVESAREANAME);
   strcpy(junkarea,JUNKAREANAME);
   strcpy(emailarea,NETMAILAREANAME);

   if (readconfigfile(configfile,argv[0])!=0)
      return 1;
  
   if (gateaddr.zone==0)
   {
      fprintf(stderr,"Error: no valid gateway Fido-address defined in %s\n",configfile);
      return 1;
   }

   if (addrmaps<1)
   {
      fprintf(stderr,"Error: no address mappings defined in %s\n",configfile);
      return 1;
   }

   if (mailertype==MAILER_NONE)
   {
      fprintf(stderr,"Error: no valid mailer type defined in %s\n",configfile);
      return 1;
   }

   if (indir[0]=='\0'
       || (outdir[0]=='\0' && mailertype==MAILER_BINKLEY)
       || (netdir[0]=='\0' && mailertype==MAILER_ARCMAIL)
       || soupdir[0]=='\0')
   {
      fprintf(stderr,"Error: required directories not defined in %s\n",configfile);
      return 1;
   }

   if (decodedir[0]=='\0')
      strcpy(decodedir, indir);

   checkdir("TempDir",tempdir);
   checkdir("InboundDir",indir);
   if (mailertype==MAILER_BINKLEY)
      checkdir("OutboundDir",outdir);
   if (mailertype==MAILER_ARCMAIL)
      checkdir("NetmailDir",netdir);
   checkdir("SoupDir",soupdir);
   checkdir("DecodeDir",decodedir);
   if (exportmaxto==0 || exportmaxto>MAXEMAIL)
      exportmaxto = MAXEMAIL;

   if (exportmaxxpost==0 || exportmaxxpost>MAXXPOST)
      exportmaxxpost = MAXXPOST;

/*...e*/

/*...swrite log header:0:*/
   if (logfile[0]!='\0')
      logf = _fsopen(logfile,"a",SH_DENYWR);

   if (logf!=NULL)
   {
      time_t t;
      struct tm *tm;

      time(&t);
      tm = localtime(&t);
      if (ftell(logf)>0)
         fprintf(logf,"\n");
      fprintf(logf,"--- SoupGate-%s v%d.%02d  %s %02d-%-3s-%04d %02d:%02d:%02d\n",
                    osstr,MAJORVERSION,MINORVERSION,
                    weekday[tm->tm_wday],
                    tm->tm_mday,month[tm->tm_mon],1900+tm->tm_year,
                    tm->tm_hour,tm->tm_min,tm->tm_sec);
   }
/*...e*/

/*...sdo work:0:*/
   switch(mode)
   {
   case MODE_IMPORT:
      import();
      break;
   case MODE_EXPORT:
      export();
      break;
   case MODE_PACK:
      packlists();
      break;
   case MODE_CHECK:
      logprintf("Configuration checked. Nothing more to do.\n");
      break;
   }
/*...e*/

/*...sclean up \38\ exit:0:*/
   if (logf!=NULL)
      fclose(logf);

   return EXIT_SUCCESS;
/*...e*/
}
