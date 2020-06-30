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

/* SoupGate import functions */

#include "soupgate.h"

/*...sisseparator:0:*/
static int isseparator(int c)
{
   return (c==',' || c==';' || c=='=');
}
/*...e*/
/*...sisrightsep:0:*/
static int isrightsep(int c)
{
   return (c=='>' || c==')');
}
/*...e*/
/*...sisdelimiter:0:*/
static int isdelimiter(int c)
{
   return (c=='\0' || c==' ' || c=='\t'
           || c=='<' || c=='>' || c=='(' || c==')');
}
/*...e*/

/*...sunescape:0:*/
static void unescape(char *s)
{
   for (;*s!='\0'; s++)
   {
      if (*s=='\\')
      {
         memmove(s,s+1,strlen(s+1)+1);
         if (*s==' ')
            *s = (char)255;
      }
   }
}
/*...e*/
/*...sunfixed:0:*/
static void unfixed(char *s)
{
   for (;*s!='\0'; s++)
   {
      if (*s==(char)255)
         *s = ' ';
   }
}
/*...e*/
/*...sunquote:0:*/
static void unquote(char *s)
{
   int len = strlen(s);

   /* remove quotes that are both left and right */
   while (len>=2
          && s[0]==s[len-1]
          && (s[0]=='\"' || s[0]=='\''))
   {
      if (len<2)
         break;
      memmove(s,s+1,len);
      len-=2;
      s[len] = '\0';
   }
}
/*...e*/

/*...sprocessemail:0:*/
static void processemail(char *s, char *addr, char *name, int type)
{
   char *cp,*cp2,*ep,*np;
   int len;
   int inlevel,qlevel;
   static char tempbuf[EMAILBUFSIZE];
   int storename, first;

   /* remove escaped characters */
   unescape(s);

   cp = s;
   first = 1;
   do
   {
      /* delimit current address */
      inlevel = 0;
      qlevel = 0;
      for (cp2=cp; *cp2!=',' || inlevel!=0 || qlevel!=0; cp2++)
      {
         if (*cp2=='\0')
         {
            cp2 = NULL;
            break;
         }
         if (*cp2=='\"' || *cp2=='\'')
            qlevel = !qlevel;
         else if (*cp2=='(' || *cp2=='<')
            inlevel++;
         else if (*cp2==')' || *cp2=='>')
            inlevel--;
      }
      if (cp2!=NULL)
        *(cp2++) = '\0';

      storename = -1;

      /* find at-sign of e-mail address */
      ep = strchr(cp,'@');
      if (ep!=NULL)
      {
         /* scan till left delimiter */
         while (ep>cp)
         {
            if (isdelimiter(ep[-1]))
               break;
            ep--;
         }

         /* blank out left delimiter */
         if (ep>cp)
            ep[-1] = ' ';

         /* copy until right delimiter & blank out */
         len = 0;
         while (!isdelimiter(*ep) && len<EMAILBUFSIZE-1)
         {
            tempbuf[len++] = *ep;
            *(ep++) = ' ';
         }
         tempbuf[len] = '\0';

         unquote(tempbuf);
         unfixed(tempbuf);

         /* primary address */
         if (first && addr!=NULL)
         {
            strncpy(addr,tempbuf,EMAILBUFSIZE);
         }
         /* additional To & CC addresses */
         else if ((type==EMAIL_TO || type==EMAIL_CC || type==EMAIL_BCC)
                  && emaillistcount<MAXEMAIL)
         {
            emaillist[emaillistcount].type = type;
            emaillist[emaillistcount].addr = malloc(strlen(tempbuf)+1);
            if (emaillist[emaillistcount].addr!=NULL)
               strcpy(emaillist[emaillistcount].addr,tempbuf);
            emaillist[emaillistcount].name = NULL;
            storename = emaillistcount;
            emaillistcount++;
         }

         /* blank out right delimiter */
         if (*ep!='\0')
            *ep = ' ';
      }

      /* what's left must be the name */

      np = NULL;

      if (ep!=NULL)
      {
         /* find first non-blank character right of e-mail */
         np = ep;
         while (isspace(*np))
            np++;
         if (*np=='\0')
            np = NULL;
      }

      if (np==NULL)
      {
         /* find first non-blank character from beginning */
         np = cp;
         while (isspace(*np))
            np++;
         if (*np=='\0')
            np = NULL;
      }

      if (np!=NULL)
      {
         if (ep==NULL)
            ep = cp + strlen(cp) - 1;
         else if (np>ep)
         {
            /* name after email-address */
            np = ep;
            ep = cp + strlen(cp) - 1;
         }

         /* skip left whitespace & delimiter */
         while (np<=ep && (isspace(*np) || isdelimiter(*np)))
            np++;

         /* skip right whitespace & delimiter */
         while (ep>=np && (isspace(*ep) || isdelimiter(*ep)))
            ep--;

         if (np<=ep)
         {
            /* copy name */
            len = ep - np + 1;
            if (len >= EMAILBUFSIZE)
               len = EMAILBUFSIZE - 1;
            memcpy(tempbuf,np,len);
            tempbuf[len] = '\0';

            unquote(tempbuf);
            unfixed(tempbuf);

            if (first && name!=NULL)
               strncpy(name,tempbuf,TEXTBUFSIZE);
            if (storename!=-1)
            {
               emaillist[storename].name = malloc(strlen(tempbuf)+1);
               if (emaillist[storename].name!=NULL)
                  strcpy(emaillist[storename].name,tempbuf);
            }
         }
      }
   
      cp = cp2;
      first = 0;
   }
   while (cp!=NULL);
}
/*...e*/
/*...sprocessmsgid:0:*/
static void processmsgid(char *s, char *msgid, int rightmost)
{
   char *cp;
   int len;

   /* remove escaped characters */
   unescape(s);

   /* find at-sign of last msgid address */
   if (rightmost)
      cp = strrchr(s,'@');
   else
      cp = strchr(s,'@');
   if (cp!=NULL)
   {
      /* scan till left delimiter: white space, <, (, \" */
      while (cp>s && cp[-1]!=' ' && cp[-1]!='\t' && cp[-1]!='<' && cp[-1]!='(' && cp[-1]!='\"')
         cp--;

      /* copy until right delimiter */
      len = 0;
      while (*cp!='\0' && *cp!=' ' && *cp!='\t' && *cp!='>' && *cp!=')' && *cp!='\"')
      {
         msgid[len++] = *cp;
         cp++;
      }
      msgid[len] = '\0';
      unfixed(msgid);
   }
}
/*...e*/
/*...sprocesstext:0:*/
static void processtext(char *s, char *text)
{
   char *ep;
   int len;

   /* skip left whitespace */
   while (*s==' ' || *s=='\t')
      s++;

   /* skip right whitespace */
   ep = s + strlen(s) - 1;
   while (ep>=s && (*ep==' ' || *ep=='\t'))
      ep--;

   if (s<=ep)
   {
      /* copy text */
      len = ep - s + 1;
      if (len>=TEXTBUFSIZE)
         len = TEXTBUFSIZE-1;
      memcpy(text,s,len);
      text[len] = '\0';
      unfixed(text);
   }
}
/*...e*/
/*...sprocessnewsgroups:0:*/
static void processnewsgroups(char *s, char *ngname)
{
   static char name[TEXTBUFSIZE];
   char *cp,*cp2;

   cp = s;
   for (;;)
   {
      cp2 = strchr(cp,',');
      if (cp2!=NULL)
         *cp2 = '\0';
      processtext(cp,name);
      if (stricmp(name,ngname)!=0 && xpostcount<MAXXPOST)
      {
         xpost[xpostcount] = malloc(strlen(name)+1);
         if (xpost[xpostcount]!=NULL)
         {
            strcpy(xpost[xpostcount],name);
            trim(xpost[xpostcount]);
            xpostcount++;
         }
      }
      if (cp2==NULL)
         break;
      *cp2 = ',';
      cp = cp2+1;
   }
}
/*...e*/
/*...sprocessdate:0:*/
static void processdate(char *s, TIMESTRUC *msgt, char *tzone)
{
   char *cp,*cp2;
   int i;

   /* remove escaped characters */
   unescape(s);

   msgt->day = 1;
   msgt->month = 0;
   msgt->year = 1970;
   msgt->hour = 0;
   msgt->min = 0;
   msgt->sec = 0;

   /* skip day if present */
   cp = strchr(s,',');
   if (cp==NULL || cp<s+3 || cp>s+12)
      cp = s;
   else
      cp++;

   /* skip left whitespace */
   while (*cp==' ' || *cp=='\t')
      cp++;

   /* scan day */
   cp2 = cp;
   while (*cp2!=' ' && *cp2!='\t' && *cp2!='\0')
      cp2++;
   if (*cp2=='\0')
      return;
   *(cp2++) = '\0';
   while (*cp2==' ' || *cp2=='\t')
      cp2++;
   msgt->day = atoi(cp);

   /* scan month */
   cp = cp2;
   while (*cp2!=' ' && *cp2!='\t' && *cp2!='\0')
      cp2++;
   if (*cp2=='\0')
      return;
   *(cp2++) = '\0';
   while (*cp2==' ' || *cp2=='\t')
      cp2++;
   for (i=0; i<12; i++)
   {
      if (memicmp(cp,month[i],3)==0)
      {
         msgt->month = i;
         break;
      }
   }

   /* scan year */
   cp = cp2;
   while (*cp2!=' ' && *cp2!='\t' && *cp2!='\0')
      cp2++;
   if (*cp2=='\0')
      return;
   *(cp2++) = '\0';
   while (*cp2==' ' || *cp2=='\t')
      cp2++;
   msgt->year = atoi(cp);
   if (msgt->year>=70 && msgt->year<=99)
      msgt->year += 1900;
   else if (msgt->year<70)
      msgt->year += 2000;

   /* scan time */
   cp = cp2;
   while (*cp2!=' ' && *cp2!='\t' && *cp2!='\0')
      cp2++;
   if (*cp2!='\0')
   {
      *(cp2++) = '\0';
      while (*cp2==' ' || *cp2=='\t')
         cp2++;
   }
   sscanf(cp,"%hu:%hu:%hu",&msgt->hour,&msgt->min,&msgt->sec);

   /* scan timezone */
   if (tzone!=NULL)
      strncpy(tzone, cp2, TEXTBUFSIZE);
}
/*...e*/

/*...sismapdomain:0:*/
static int ismapdomain(char *cp)
{
   int i,len,clen;

   len = strlen(cp);

   if (len>=12
       && stricmp(cp+len-12, ".fidonet.org")==0)
      return 1;

   for (i=0; i<fidodomains; i++)
   {
      if (fidodomain[i].domain[0]!='#')
         continue;
      clen = strlen(fidodomain[i].domain);
      if (clen<=len
          && cp[len-clen]=='.'
          && stricmp(cp+(len-clen+1), fidodomain[i].domain+1)==0)
         return 1;
   }

   return 0;
}
/*...e*/

/*...sprocessimport:0:*/
/* isjunk may be set to JUNK_STR if it is initially NO_JUNK
   in non-copy mode */
static void processimport(int copy, int filesonly, int msgcategory,
                   int isownorg, int *isjunk,
                   char *truncate, char *truncend,
                   FILE *fp, FILE *fpkt,
                   long msgpos, long endpos, long *bodypos,
                   char *linebuf, char *filebuf)
{
/*...svariables:0:*/
   static char fname[FILENAME_MAX]; /* must keep filename(s) */
   char decfpath[FILENAME_MAX], *decfname;
   int frenamed;
   char boundary[TEXTBUFSIZE];
   char buffer[BUFSIZE];
   FILE *fdec;
   char *cp,*cp2;
   int buflen,maxlen;
   unsigned attr;
   int i,j,cc,bcc,len,numid;
   int emptylines,notallempty;
   int pasttrunc;
   int inbody;
   int qptrans;
   int nextmimedata,nextmimename;
   int decodefile,dofile;
   int linebytes;
   int b1,b2,b3,b4;
   int filebuflen,writelen;
   long fpos;
   unsigned long decodebytes;
   int ch;
   int insertline;
   int pastqchars, inquote;
   int pastat;
   int dowhite;
   int strip;
   static int alreadyfromto, pastfirstline;
   
/*...e*/

/*...sinitialize:0:*/
   /* seek to start of message */
   fseek(fp,msgpos,SEEK_SET);

   if (copy)
   {
      partno = 1;
      /* write message header */
      if (!filesonly)
         writepktmsgheader(msgcategory, fpkt, fname, *isjunk, isownorg);
   }
   else
   {
      parts = 1;
      msgfromemail[0] = '\0';
      msgfromname[0] = '\0';
      msgtoemail[0] = '\0';
      msgtoname[0] = '\0';
      msgreplyemail[0] = '\0';
      msgreplyname[0] = '\0';
      xfrom[0] = '\0';
      sender[0] = '\0';
      quotename[0] = '\0';
      quoteemail[0] = '\0';
      msgsubj[0] = '\0';
      msgorganization[0] = '\0';
      msgid[0] = '\0';
      replyid[0] = '\0';
      routemailpwd[0] = '\0';
      routemailsubj[0] = '\0';
      rfccharset[0] = '\0';
      mimetype[0] = '\0';
      mimecte[0] = '\0';
      tzinfo[0] = '\0';
      emaillistcount = 0;
      pastqchars = 0;
      inquote = 0;
      *bodypos = msgpos;
      xpostcount = 0;
      alreadyfromto = 0;
   }

   partbytes = 0;
   isfattach = 0;
   qptrans = 0;
   nextmimedata = 0;
   nextmimename = 0;
   decodefile = ENCODE_NONE;
   fdec = NULL;
   decfpath[0] = '\0';
   decfname = decfpath;
   frenamed = 0;
   fname[0] = '\0';
   boundary[0] = '\0';
   filebuflen = 0;
   emptylines = 0;
   inbody = 0;
   insertline = 0;
   pasttrunc = 0;
   pastfirstline = 0;
/*...e*/

   for (;;)
   {
/*...sinsert line:0:*/
      fpos = ftell(fp);
      if (fpos>=endpos)
      {
         if (*bodypos==msgpos)
            *bodypos = endpos;
         break;
      }
      if (copy && !filesonly && (insertline==1))
      {
         insertline++;  /* avoids duplicates */
         dowhite = 0;
         if (xpostcount>0)
         {
            for (i=0; i<xpostcount; i++)
            {
               if (i%3==0)
               {
                  if (i>0)
                     fprintf(fpkt,"\r");
                  fprintf(fpkt,"XPost: ");
               }
               else
                  fprintf(fpkt,", ");
               fprintf(fpkt,"%s",xpost[i]);
               free(xpost[i]);
            }
            fprintf(fpkt,"\r");
            dowhite = 1;
         }
         if (!alreadyfromto)
         {
            switch(infoinsert)
            {
            case INFO_EMAIL:
               if (msgfromemail[0]!='\0' && stricmp(msgfromemail,msgfromname)!=0)
               {
                  fprintf(fpkt,"From: %s\r", msgfromemail);
                  dowhite = 1;
               }
               if (msgtype==EMAIL
                   && msgtoemail[0]!='\0' && stricmp(msgtoemail,msgtoname)!=0)
               {
                  fprintf(fpkt,"To: %s\r", msgtoemail);
                  dowhite = 1;
               }
               break;
            case INFO_NAME:
               if (msgfromname[0]!='\0' && stricmp(msgfromname,msgfromemail)!=0)
               {
                  fprintf(fpkt,"From: %s\r", msgfromname);
                  dowhite = 1;
               }
               if (msgtype==EMAIL
                   && msgtoname[0]!='\0' && stricmp(msgtoname,msgtoemail)!=0)
               {
                  fprintf(fpkt,"To: %s\r", msgtoname);
                  dowhite = 1;
               }
               break;
            }
         }
         cc = 0;
         bcc = 0;
         for (i=0; i<emaillistcount; i++)
         {
            if (emaillist[i].type!=EMAIL_TO)
            {
               if (emaillist[i].type==EMAIL_BCC)
                  bcc = 1;
               else
                  cc = 1;
               continue;
            }
            if (emaillist[i].addr!=NULL)
            {
               fprintf(fpkt,"To: %s",emaillist[i].addr);
               free(emaillist[i].addr);
            }
            if (emaillist[i].name!=NULL)
            {
               fprintf(fpkt," (%s)",emaillist[i].name);
               free(emaillist[i].name);
            }
            fprintf(fpkt,"\r");
         }
         if (cc)
         {
            for (i=0; i<emaillistcount; i++)
            {
               if (emaillist[i].type!=EMAIL_CC)
                  continue;
               if (emaillist[i].addr!=NULL)
               {
                  fprintf(fpkt,"Copy: %s",emaillist[i].addr);
                  free(emaillist[i].addr);
               }
               if (emaillist[i].name!=NULL)
               {
                  fprintf(fpkt," (%s)",emaillist[i].name);
                  free(emaillist[i].name);
               }
               fprintf(fpkt,"\r");
            }
         }
         if (bcc)
         {
            for (i=0; i<emaillistcount; i++)
            {
               if (emaillist[i].type!=EMAIL_BCC)
                  continue;
               if (emaillist[i].addr!=NULL)
               {
                  fprintf(fpkt,"BCopy: %s",emaillist[i].addr);
                  free(emaillist[i].addr);
               }
               if (emaillist[i].name!=NULL)
               {
                  fprintf(fpkt," (%s)",emaillist[i].name);
                  free(emaillist[i].name);
               }
               fprintf(fpkt,"\r");
            }
         }
         if (emaillistcount>0)
            dowhite = 1;
         if (routepwdfailed)
         {
            fprintf(fpkt,"*** Incorrect SoupGate Route-password ");
            fprintf(fpkt,"for node %u:%u/%u",
                         route[routeno]->fido.zone,
                         route[routeno]->fido.net,
                         route[routeno]->fido.node);
            if (route[routeno]->fido.point>0)
               fprintf(fpkt,".%u",route[routeno]->fido.point);
            fprintf(fpkt," ***\r");
            dowhite = 1;
         }
         if (msgtype==EMAIL && isfattach && fncmp(msgsubj,fname)!=0)
         {
            fprintf(fpkt,"Subject: %s\r",msgsubj);
            dowhite = 1;
         }
         if (dowhite)
            fprintf(fpkt,"\r");
      }
/*...e*/
/*...sread line:0:*/
      if (inbody)
      {
         buflen = 0;
         for (;;)
         {
            fgets(linebuf+buflen,LINEBUFSIZE-buflen,fp);
            if (feof(fp))
               break;
            buflen += strlen(linebuf+buflen);
            if (decodefile==ENCODE_UU)
            {
               if (linebuf[buflen-1]=='\n')
                  buflen--;
            }
            else
            {
               while (buflen>0
                      && (linebuf[buflen-1]=='\n' || linebuf[buflen-1]==' '))
                  buflen--;
            }
            if (qptrans && buflen>0 && linebuf[buflen-1]=='=')
               buflen--;
            else
               break;
         }
         linebuf[buflen] = '\0';
         if (feof(fp) && buflen==0)
            break;
      }
      else
      {
         buflen = 0;
         do
         {
            fgets(linebuf+buflen,LINEBUFSIZE-buflen,fp);
            if (feof(fp))
               break;
            buflen += strlen(linebuf+buflen);
            if (linebuf[buflen-1]=='\n')
               buflen--;
            if (buflen==0)
               break;
            ch = fgetc(fp);
            ungetc(ch,fp);
         }
         while (ch!=EOF && ch!='\r' && ch!='\n' && isspace(ch));
         /* strip excess whitespace */
         for (i=0; i<buflen;)
         {
            if (isspace(linebuf[i]))
            {
               if (isspace(linebuf[i-1]) || i==0)
               {
                  memmove(linebuf+i, linebuf+i+1, buflen-i);
                  buflen--;
               }
               else
               {
                  linebuf[i] = ' ';
                  i++;
               }
            }
            else
               i++;
         }
         while (buflen>0 && isspace(linebuf[buflen-1]))
            buflen--;
         linebuf[buflen] = '\0';
      }
/*...e*/
/*...spassword\47\boundary\47\quoted\45\printable:0:*/
      if (pasttrunc)
      {
         if (truncend!=NULL && strstr(linebuf, truncend) != NULL)
            pasttrunc = 0;
         continue;
      }
      if (copy
          && !inbody
          && memicmp(linebuf,"X-MailPassword:",15)==0)
         continue;
      if (inbody
          && boundary[0]!='\0'
          && memcmp(linebuf,"--",2)==0
          && memcmp(linebuf+2,boundary,strlen(boundary))==0)
      {
         if (decodefile!=ENCODE_NONE && copy)
         {
            writefileinfo(decfpath,decfname,frenamed,
                          decodefile,dofile,decodebytes,
                          filesonly,fpkt);
            if (fdec!=NULL)
            {
               if (filebuflen>0)
                  fwrite(filebuf,1,filebuflen,fdec);
               fclose(fdec);
               fdec = NULL;
            }
         }
         decodefile = ENCODE_NONE;
         inbody = 0;
         qptrans = 0;
         nextmimedata = 0;
         nextmimename = 0;
         if (memcmp(linebuf+2+strlen(boundary),"--",2)==0)
            break;
         else
            continue;
      }
      if (qptrans && inbody)
      {
         for (cp=linebuf; *cp!='\0'; cp++)
         {
            if (cp[0]=='=' && isxdigit(cp[1]) && isxdigit(cp[2]))
            {
               cp[0] = hexbyte(cp+1);
               if (cp[0]=='\0')
                  cp[0] = ' ';
               memmove(cp+1,cp+3,strlen(cp+3)+1);
               buflen -= 2;
            }
         }
      }
/*...e*/
/*...sdecode MIME:0:*/
      if (decodefile==ENCODE_MIME && dofile!=ATTACH_IGNORE)
      {
         if (mimevalue(linebuf[0])!=-1)
         {
            if (copy)
            {
               linebytes = 0;
               for (i=0; linebuf[i]!='\0'; i++)
               {
                  if (mimevalue(linebuf[i])!=-1)
                     linebytes++;
               }
               linebytes = (linebytes*3)/4;
               decodebytes += linebytes;
               if (fdec!=NULL)
               {
                  cp = linebuf;
                  while (linebytes>0)
                  {
                     if (filebuflen>=512-3)
                     {
                        fwrite(filebuf,1,filebuflen,fdec);
                        filebuflen = 0;
                     }
                     b1 = mimevalue(*(cp++));
                     b2 = mimevalue(*(cp++));
                     b3 = mimevalue(*(cp++));
                     b4 = mimevalue(*(cp++));
                     if (linebytes-->0 && b1!=-1 && b2!=-1)
                        filebuf[filebuflen++] = (b1<<2)|(b2>>4);
                     if (linebytes-->0 && b2!=-1 && b3!=-1)
                        filebuf[filebuflen++] = (b2<<4)|(b3>>2);
                     if (linebytes-->0 && b3!=-1 && b4!=-1)
                        filebuf[filebuflen++] = (b3<<6)|(b4);
                  }
               }
            }
            if (memcmp(linebuf+buflen-2,"==",2)==0)
            {
               if (copy)
               {
                  writefileinfo(decfpath,decfname,frenamed,
                                decodefile,dofile,decodebytes,
                                filesonly,fpkt);
                  if (fdec!=NULL)
                  {
                     if (filebuflen>0)
                        fwrite(filebuf,1,filebuflen,fdec);
                     fclose(fdec);
                     fdec = NULL;
                     decfpath[0] = '\0';
                  }
               }
               decodefile = ENCODE_NONE;
            }
            continue;
         }
         else
         {
            if (copy)
            {
               writefileinfo(decfpath,decfname,frenamed,
                             decodefile,dofile,decodebytes,
                             filesonly,fpkt);
               if (fdec!=NULL)
               {
                  if (filebuflen>0)
                     fwrite(filebuf,1,filebuflen,fdec);
                  fclose(fdec);
                  fdec = NULL;
                  decfpath[0] = '\0';
               }
            }
            decodefile = ENCODE_NONE;
         }
      }
/*...e*/
/*...sdecode UU:0:*/
      else if (decodefile==ENCODE_UU && dofile!=ATTACH_IGNORE)
      {
         if (linebuf[0]>=33 && linebuf[0]<=96
             && (linebytes = (linebuf[0]-32)&077)
                 <= (strlen(linebuf)-1) * 3 / 4)
         {
            if (copy)
            {
               decodebytes += linebytes;
               if (fdec!=NULL)
               {
                  cp = linebuf + 1;
                  while (linebytes>0)
                  {
                     if (filebuflen>=512-3)
                     {
                        fwrite(filebuf,1,filebuflen,fdec);
                        filebuflen = 0;
                     }
                     b1 = (*(cp++)-32)&077;
                     b2 = (*(cp++)-32)&077;
                     b3 = (*(cp++)-32)&077;
                     b4 = (*(cp++)-32)&077;
                     if (linebytes-->0)
                        filebuf[filebuflen++] = (b1<<2)|(b2>>4);
                     if (linebytes-->0)
                        filebuf[filebuflen++] = (b2<<4)|(b3>>2);
                     if (linebytes-->0)
                        filebuf[filebuflen++] = (b3<<6)|(b4);
                  }
               }
            }
            continue;
         }
         else
         {
            if (copy)
            {
               writefileinfo(decfpath,decfname,frenamed,
                             decodefile,dofile,decodebytes,
                             filesonly,fpkt);
               if (fdec!=NULL)
               {
                  if (filebuflen>0)
                     fwrite(filebuf,1,filebuflen,fdec);
                  fclose(fdec);
                  fdec = NULL;
                  decfpath[0] = '\0';
               }
            }
            decodefile = ENCODE_NONE;
         }
         if (stricmp(linebuf,"end")==0)
            continue;
      }
/*...e*/
/*...sempty line:0:*/
      if (buflen==0)
      {
         if (inbody && notallempty)
            emptylines++;
         else
         {
            if (*bodypos==msgpos)
               *bodypos = ftell(fp);
            inbody = 1;
            insertline++;
            notallempty = 0;
         }
         if (nextmimedata)
         {
            nextmimedata = 0;
            nextmimename = 0;
            frenamed = 0;
            if (decfpath[0]=='\0')
            {
               if (msgtype==EMAIL && !decodedirall)
               {
                  genfname(indir,".ATT",decfpath);
                  decfname = decfpath + strlen(indir);
               }
               else
               {
                  genfname(decodedir,".ATT",decfpath);
                  decfname = decfpath + strlen(decodedir);
               }
            }
            else
            {
#ifndef __unix__
               cp = strrchr(decfpath,'\\');
               if (cp==NULL)
#endif
                  cp = strrchr(decfpath,'/');
               if (cp==NULL)
                  cp = decfpath;
               if (msgtype==EMAIL
                   && (!decodedirall || ismail(cp,strlen(cp))))
               {
                  memmove(decfpath+strlen(indir),cp,strlen(cp)+1);
                  memcpy(decfpath,indir,strlen(indir));
                  decfname = decfpath + strlen(indir);
                  frenamed = starname(decfpath, decfname);
               }
               else
               {
                  memmove(decfpath+strlen(decodedir),cp,strlen(cp)+1);
                  memcpy(decfpath,decodedir,strlen(decodedir));
                  decfname = decfpath + strlen(decodedir);
                  frenamed = starname(decfpath, decfname);
               }
               if (exist(decfpath))
               {
                  frenamed = 1;
                  renfname(decfpath);
               }
            }
            decodefile = ENCODE_MIME;
            dofile = attach[msgtype];
            if (*isjunk != NO_JUNK)
               dofile = ATTACH_IGNORE;
            if (copy)
            {
               if (dofile==ATTACH_DECODE)
               {
                  fdec = _fsopen(decfpath,"wb",SH_DENYWR);
                  if (fdec==NULL)
                     dofile = ATTACH_IGNORE;
               }
               else
                  fdec = NULL;
               filebuflen = 0;
               decodebytes = 0;
            }
            if (dofile==ATTACH_DECODE
                && msgtype==EMAIL)
            {
               if (strlen(fname)+1+strlen(decfname)<72)
               {
                  if (strlen(fname)>0)
                     strcat(fname," ");
                  strcat(fname,decfname);
               }
               isfattach = 1;
            }
            if (dofile!=ATTACH_IGNORE)
               continue;
         }
         else if (nextmimename)
            nextmimename = 0;
      }
/*...e*/
/*...snon\45\empty line:0:*/
      else
      {
         if (inbody && !pastfirstline)
         {
            if (!copy
                && (memicmp(linebuf, "From:", 5)==0
                    || memicmp(linebuf, "To:", 3)==0))
            {
               alreadyfromto = 1;
            }
            pastfirstline = 1;
         }
         notallempty = 1;
         if (inbody && truncate!=NULL && strstr(linebuf, truncate)!=NULL)
         {
            pasttrunc = 1;
            continue;
         }
         if (emptylines>0)
         {
            if (copy && !filesonly)
            {
               for (i=0; i<emptylines; i++)
                  fputc('\r',fpkt);
            }
            partbytes += emptylines;
            emptylines = 0;
         }
/*...e*/
/*...sprocess quote\45\to:0:*/
         if (!copy && inbody && quoteto && pastqchars!=-1)
         {
            inquote = 0;
            for (i=0; i<quotechars; i++)
            {
               cp = strchr(linebuf, quotechar[i].qchar);
               if (cp!=NULL)
               {
                  len = cp - linebuf;
                  if (len >= quotechar[i].mincol
                      && len <= quotechar[i].maxcol)
                  {
                     numid = 0;
                     for (j=0; j<len; j++)
                     {
                        if (!isspace(linebuf[j]))
                        {
                           if (isalpha(linebuf[j]))
                           {
                              numid++;
                           }
                           else
                           {
                              numid = -1;
                              break; /* j loop */
                           }
                        }
                     }
                     if (numid >= quotechar[i].minid
                         && numid <= quotechar[i].maxid)
                     {
                        inquote = 1;
                        break; /* quotechar loop */
                     }
                  }
               }
            }
            if (quotename[0]!='\0' || quoteemail[0]!='\0')
            {
               if (inquote)
                  pastqchars = -1;   /* quoteto done */
               else
               {
                  pastqchars += buflen;
                  if (pastqchars > MAXPASTQCHARS)
                  {
                     quotename[0] = '\0';
                     quoteemail[0] = '\0';
                     pastqchars = 0;
                  }
               }
            }
            if (pastqchars!=-1 && !inquote)
            {
               for (i=0; i<quotemasks; i++)
               {
                  cp = wordfind(quotemask[i], linebuf);
                  if (cp==NULL)
                     continue;
                  while (cp>linebuf && isspace(cp[-1]))
                     cp--;
                  if (cp==linebuf)
                     continue;
                  cp2 = cp;
                  len = 0;
                  pastat = 0;
                  while (cp>linebuf && !isseparator(cp[-1])
                         && (cp<linebuf+2 || !isspace(cp[-1]) || !isspace(cp[-2]))
                         && (!pastat || !isrightsep(cp[-1])))
                  {
                     cp--;
                     len++;
                     if (*cp=='@')
                        pastat = 1;
                  }
                  if (len>BUFSIZE-1)
                     len = BUFSIZE-1;
                  memcpy(buffer,cp,len);
                  buffer[len] = '\0';
                  processemail(buffer,quoteemail,quotename,EMAIL_QUOTE);
                  if (strlen(quotename)<3)
                     quotename[0] = '\0';
                  else
                  {
                     /* name must consist of alphanumeric and
                        punctuation characters only */
                     for (j=0; quotename[j]!='\0'; j++)
                     {
                        if (!isalpha(quotename[j])
                            && quotename[j]!='\''
                            && quotename[j]!='"'
                            && quotename[j]!='.'
                            && quotename[j]!='-'
                            && quotename[j]!=','
                            && quotename[j]!=' ')
                         {
                            quotename[0] = '\0';
                            break;
                         }
                     }
                     if (quotename[0]!='\0')
                     {
                        /* name should not include newsgroup name */
                        if ((cp=wordfind(areaname,quotename))!=NULL)
                        {
                           cp += strlen(areaname);
                           while (*cp==' ')
                              cp++;
                           memmove(quotename,cp,strlen(cp)+1);
                           if (strlen(quotename)<3)
                              quotename[0] = '\0';
                        }
                        else
                        {
                           for (j=0; j<xpostcount; j++)
                           {
                              if ((cp=wordfind(xpost[j],quotename))!=NULL)
                              {
                                 cp += strlen(xpost[j]);
                                 while (*cp==' ')
                                    cp++;
                                 memmove(quotename,cp,strlen(cp)+1);
                                 if (strlen(quotename)<3)
                                    quotename[0] = '\0';
                                 break;
                              }
                           }
                        }
                     }
                  }
                  if (strlen(quoteemail)<6)
                     quoteemail[0] = '\0';
                  if (quoteemail[0]!='\0' || quotename[0]!='\0')
                  {
                     pastqchars = linebuf + buflen - cp2;
                     break;
                  }
               }
            }
         }
/*...e*/
/*...sprocess MIME\47\UU headers:0:*/
         if (inbody
             && strncmp(linebuf,"begin ",6)==0
             && sscanf(linebuf+6,"%o",&attr)==1)
         {
            frenamed = 0;
            cp = linebuf+9;
            while (isspace(*cp))
               cp++;
            strncpy(decfpath, cp, FILENAME_MAX);
            if (decfpath[0]=='\0')
            {
               if (msgtype==EMAIL && !decodedirall)
               {
                  genfname(indir,".ATT",decfpath);
                  decfname = decfpath + strlen(indir);
               }
               else
               {
                  genfname(decodedir,".ATT",decfpath);
                  decfname = decfpath + strlen(decodedir);
               }
            }
            else
            {
#ifndef __unix__
               cp = strrchr(decfpath,'\\');
               if (cp==NULL)
#endif
                  cp = strrchr(decfpath,'/');
               if (cp==NULL)
                  cp = decfpath;
               if (msgtype==EMAIL
                   && (!decodedirall || ismail(cp,strlen(cp))))
               {
                  memmove(decfpath+strlen(indir),cp,strlen(cp)+1);
                  memcpy(decfpath,indir,strlen(indir));
                  decfname = decfpath + strlen(indir);
                  frenamed = starname(decfpath, decfname);
               }
               else
               {
                  memmove(decfpath+strlen(decodedir),cp,strlen(cp)+1);
                  memcpy(decfpath,decodedir,strlen(decodedir));
                  decfname = decfpath + strlen(decodedir);
                  frenamed = starname(decfpath, decfname);
               }
               if (exist(decfpath))
               {
                  frenamed = 1;
                  renfname(decfpath);
               }
            }
            decodefile = ENCODE_UU;
            dofile = attach[msgtype];
            if (*isjunk != NO_JUNK)
               dofile = ATTACH_IGNORE;
            if (copy)
            {
               if (dofile==ATTACH_DECODE)
               {
                  fdec = _fsopen(decfpath,"wb",SH_DENYWR);
                  if (fdec==NULL)
                     dofile = ATTACH_IGNORE;
               }
               else
                  fdec = NULL;
               filebuflen = 0;
               decodebytes = 0;
            }
            if (!copy && dofile==ATTACH_DECODE
                && msgtype==EMAIL)
            {
               if (strlen(fname)+1+strlen(decfname)<72)
               {
                  if (strlen(fname)>0)
                     strcat(fname," ");
                  strcat(fname,decfname);
               }
               isfattach = 1;
            }
            if (dofile!=ATTACH_IGNORE)
               continue;
         }
         if (memicmp(linebuf,"Content-",8)==0)
            nextmimename = 1;
         if (nextmimename)
         {
            for (cp=linebuf; *cp!='\0'; cp++)
            {
               if (memicmp(cp,"name=\"",6)==0)
               {
                  i = 0;
                  cp+=6;
                  while (isspace(*cp) && *cp!='\0')
                     cp++;
                  while (*cp!='\"' && *cp!='\0' && i<FILENAME_MAX)
                     decfpath[i++] = *(cp++);
                  while (i>0 && isspace(decfpath[i-1]))
                     i--;
                  decfpath[i] = '\0';
               }
               else if (boundary[0]=='\0'       /* avoid nesting */
                        && memicmp(cp,"boundary=\"",10)==0)
               {
                  i = 0;
                  cp+=10;
                  while (isspace(*cp) && *cp!='\0')
                     cp++;
                  while (*cp!='\"' && *cp!='\0' && i<TEXTBUFSIZE)
                     boundary[i++] = *(cp++);
                  while (i>0 && isspace(boundary[i-1]))
                     i--;
                  boundary[i] = '\0';
               }
               else if (rfccharset[0]=='\0'       /* avoid nesting */
                        && memicmp(cp,"charset=",8)==0)
               {
                  i = 0;
                  cp+=8;
                  while (isspace(*cp) && *cp!='\0')
                     cp++;
                  while (*cp!=';' && *cp!='\0' && i<TEXTBUFSIZE)
                     rfccharset[i++] = *(cp++);
                  while (i>0 && isspace(rfccharset[i-1]))
                     i--;
                  rfccharset[i] = '\0';
               }
            }
         }
         if (memicmp(linebuf,"Content-Type:",13)==0)
         {
            cp = linebuf + 13;
            while (*cp!='\0' && isspace(*cp))
               cp++;
            if (mimetype[0]=='\0') /* not interested in nested ones */
               strncpy(mimetype, cp, TEXTBUFSIZE);
         }
         if (memicmp(linebuf,"Content-Transfer-Encoding:",26)==0)
         {
            qptrans = 0;
            cp = linebuf + 26;
            while (*cp!='\0' && isspace(*cp))
               cp++;
            if (mimecte[0]=='\0') /* not interested in nested ones */
               strncpy(mimecte, cp, TEXTBUFSIZE);
            for (; *cp!='\0'; cp++)
            {
               if (memicmp(cp,"base64",6)==0)
                  nextmimedata = 1;
               else if (memicmp(cp,"quoted-printable",16)==0)
                  qptrans = 1;
            }
         }
/*...e*/
/*...sprocess headers:0:*/
         if (!copy)
         {
            if (!inbody)
            {
               if (memicmp(linebuf,"From:",5)==0)
                  processemail(linebuf+5,msgfromemail,msgfromname,EMAIL_FROM);
               else if (memicmp(linebuf,"Reply-To:",9)==0)
                  processemail(linebuf+9,msgreplyemail,msgreplyname,EMAIL_REPLY);
               else if (memicmp(linebuf,"To:",3)==0)
                  processemail(linebuf+3,msgtoemail,msgtoname,EMAIL_TO);
               else if (memicmp(linebuf,"Cc:",3)==0)
                  processemail(linebuf+3,NULL,NULL,EMAIL_CC);
               else if (memicmp(linebuf,"Bcc:",4)==0)
                  processemail(linebuf+3,NULL,NULL,EMAIL_BCC);
               else if (memicmp(linebuf,"X-Comment-To:",13)==0
                        && msgtoname[0]=='\0')
                  processtext(linebuf+13,msgtoname);
               else if (memicmp(linebuf,"X-From:",7)==0)
                  processtext(linebuf+7,xfrom);
               else if (memicmp(linebuf,"X-eGroups-From:",15)==0)
                  processtext(linebuf+15,xfrom);
               else if (memicmp(linebuf,"Sender:",7)==0)
                  processtext(linebuf+7,sender);
               else if (memicmp(linebuf,"Subject:",8)==0)
                  processtext(linebuf+8,msgsubj);
               else if (memicmp(linebuf,"Date:",5)==0)
                  processdate(linebuf+5,&msgtime,tzinfo);
               else if (memicmp(linebuf,"Message-ID:",11)==0)
                  processmsgid(linebuf+11,msgid,0);
               else if (memicmp(linebuf,"Organization:",13)==0)
                  processtext(linebuf+13,msgorganization);
               else if (memicmp(linebuf,"In-Reply-To:",12)==0)
                  processmsgid(linebuf+12,replyid,1);
               else if (memicmp(linebuf,"References:",11)==0
                        && replyid[0]=='\0')
                  processmsgid(linebuf+11,replyid,1);
               else if (memicmp(linebuf,"Newsgroups:",11)==0)
               {
                  processnewsgroups(linebuf+11,areaname);
               }
               else if (memicmp(linebuf,"X-MailPassword:",15)==0)
               {
                  processtext(linebuf+15,routemailpwd);
                  continue;
               }
            }
            if (inbody && *isjunk==NO_JUNK && msgtype==EMAIL)
            {
               for (i=0; i<junkstrs; i++)
               {
                  if (junkstr[i].category!=JUNK_EMAIL
                   && junkstr[i].category!=JUNK_ALL)
                     continue;
                  if (substrcmp(junkstr[i].text,linebuf))
                  {
                     *isjunk = JUNK_STR;
                     break;
                  }
               }
            }
         }
/*...e*/
/*...scopy text:0:*/
         if (!filesonly)
         {
            if (inbody)
               strip = 0;
            else if (buflen<=2)
               strip = 1;
            else
            {
               switch(stripkludges[msgcategory])
               {
               case STRIP_ALL:
                  strip = 1;
                  break;
               case STRIP_NONE:
                  strip = 0;
                  break;
               default: /* STRIP_SOME */
                  if (memicmp(linebuf, "Received:", 9)==0
                   || memicmp(linebuf, "Path:", 5)==0
                   || memicmp(linebuf, "References:",11)==0)
                     strip = 1;
                  else
                     strip = 0;
                  break;
               }
            }
            if (!strip)
            {
               if (importlimit>0 && partbytes>0
                   && partbytes+buflen+256>=importlimit)   /* 256 reserve */
               {
                  if (copy)
                     partno++;
                  else
                     parts++;
                  partbytes = 0;
                  if (copy)
                  {
                     fprintf(fpkt,"\r[continued in next message]\r");
                     /* write tearline/origin & terminate message */
                     writemsgbottom(msgcategory, fpkt);
                     /* write message header */
                     writepktmsgheader(msgcategory, fpkt, fname,
                                       *isjunk, isownorg);
                     fprintf(fpkt,"[continued from previous message]\r\r");
                  }
               }
               if (inbody)
                  maxlen = FIDOBODYLINESIZE;
               else
                  maxlen = FIDOKLUDGELINESIZE;
               for (cp=linebuf; cp<linebuf+buflen; cp+=writelen)
               {
                  if (!inbody)
                  {
                     if (copy)
                        fputc('\1',fpkt);
                     partbytes++;
                     if (cp!=linebuf)
                     {
                        if (copy)
                        {
                           fputc(' ',fpkt);
                           fputc(' ',fpkt);
                        }
                        partbytes += 2;
                        maxlen = FIDOKLUDGELINESIZE - 2;
                     }
                  }
                  if (cp<linebuf+buflen-maxlen)
                  {
                     writelen = maxlen;
                     for (i=writelen; i>32; i--)
                     {
                        if (!isalnum(cp[i-1]))
                        {
                           writelen = i;
                           break;
                        }
                     }
                  }
                  else
                     writelen = linebuf+buflen-cp;
                  if (copy)
                     fwrite(cp,1,writelen,fpkt);
                  partbytes += writelen;
                  if (copy)
                     fputc('\r',fpkt);
                  partbytes++;
               }
            }
         }
      }
/*...e*/
   }

/*...sfinish:0:*/
   if (pastqchars!=-1)
   {
      quotename[0] = '\0';
      quoteemail[0] = '\0';
   }
   if (decodefile!=ENCODE_NONE && copy)
   {
      writefileinfo(decfpath,decfname,frenamed,
                    decodefile,dofile,decodebytes,
                    filesonly,fpkt);
      if (fdec!=NULL)
      {
         if (filebuflen>0)
            fwrite(filebuf,1,filebuflen,fdec);
         fclose(fdec);
         fdec = NULL;
      }
      decodefile = ENCODE_NONE;
   }
   if (copy && !filesonly)
   {
      /* write tearline/origin & terminate message */
      writemsgbottom(msgcategory, fpkt);
   }
/*...e*/
}
/*...e*/

/*...simport:0:*/
void import(void)
{
/*...svariables:0:*/
   char fname[FILENAME_MAX];
   char pktname[FILENAME_MAX];
   char buffer[BUFSIZE], buffer2[BUFSIZE];
   char *linebuf, *filebuf;
   FILE *fp,*fp2,*fpkt;
   char *cp,*cp2,*cp3;
   char *truncate, *truncend;
   char *bp;
   long linenr;
   int done;
   int mapno,i,j;
   unsigned long filesize;
   long filepos, msgpos, bodypos, endpos;
   int percent;
   int notalldone;
   int isjunk,isownorg;
   int msgcategory;
   int thisismatch;
   int ismaillist,isignoretext,islistno,islistcommand;
   int badfile;
   long totalmessagecount, totalemailcount, totalnewscount,
        totalmaillistcount, totalignoretextcount, totaljunkcount,
        totalownorgcount, totalpersonalcount, totalfilecount,
        totallistmsgcount, totallistcmdcount, totallistreplycount;
/*...e*/

/*...sinitialization:0:*/
   logprintf("Importing from Soup to PKT\n");

   strcpy(fname,soupdir);
   strcat(fname,AREAS);

   fp = _fsopen(fname,"r",SH_DENYNO);
   if (fp==NULL)
   {
      logprintf("No AREAS found. Nothing to do.\n");
      return;
   }

   if (importlimit>0)
      importlimit -= 512;     /* for our own kludges */

   linenr = 0;

   notalldone = 0;

   /* sets up reply file names */
   for (i=0; i<MAILTYPES; i++)
      genreplyfile(i);

   totalmessagecount = 0;
   totalemailcount = 0;
   totalnewscount = 0;
   totalmaillistcount = 0;
   totalignoretextcount = 0;
   totaljunkcount = 0;
   totalownorgcount = 0;
   totalpersonalcount = 0;
   totalfilecount = 0;
   totallistmsgcount = 0;
   totallistcmdcount = 0;
   totallistreplycount = 0;

   linebuf = memalloc(LINEBUFSIZE);
   filebuf = memalloc(FILEBUFSIZE);
/*...e*/

   for (;;)
   {
/*...sprocess message area:0:*/
      fgets(buffer,BUFSIZE-1,fp);
      if (feof(fp))
         break;
      linenr++;
      cp = strchr(buffer,'\t');
      if (cp!=NULL)
      {
         *(cp++) = '\0';
         cp2 = strchr(cp,'\t');
         if (cp2!=NULL)
            *(cp2++) = '\0';
      }
      if (cp==NULL || cp2==NULL)
      {
         logprintf("Error in line %ld in %s\n",linenr,AREAS);
         continue;
      }
      strncpy(areaname,cp,127);
      areaname[127] = '\0';
      switch(cp2[0])
      {
      case 'u':
         msgformat = FORMAT_USENET;
         msgtype = NEWS;
         break;
      case 'm':
         msgformat = FORMAT_MAILBOX;
         msgtype = EMAIL;
         break;
      case 'M':
         msgformat = FORMAT_MMDF;
         msgtype = EMAIL;
         break;
      case 'B':
         msgformat = FORMAT_BINARY;
         msgtype = NEWS;
         break;
      case 'b':
         msgformat = FORMAT_BINARY;
         msgtype = EMAIL;
         break;
      default:
         msgformat = FORMAT_UNKNOWN;
         break;
      }
      if (msgformat==FORMAT_UNKNOWN)
      {
         logprintf("Unknown message format in line %ld in %s\n",linenr,AREAS);
         notalldone = 1;
         continue;
      }
      switch(cp2[2])
      {
      case 'm':
         msgtype = EMAIL;
         break;
      case 'n':
         msgtype = NEWS;
         break;
      }
      if (stricmp(areaname,EMAILAREANAME)==0)
         msgtype = EMAIL;
      if (linenr==1)
         fprintf(stderr,"\n");
      logprintf("Importing %s\n",areaname);
      strcpy(fname,soupdir);
      strcat(fname,buffer);
      strcat(fname,messageext);
      badfile = 0;
      fp2 = _fsopen(fname,"rb",SH_DENYNO);
      if (fp2==NULL)
      {
         logprintf("Error reading %s\n",fname);
         continue;
      }
      messagecount = 0;
      pktmsgcount = 0;
      emailcount = 0;
      newscount = 0;
      maillistcount = 0;
      ignoretextcount = 0;
      junkcount = 0;
      ownorgcount = 0;
      personalcount = 0;
      filecount = 0;
      listmsgcount = 0;
      listcmdcount = 0;
      listreplycount = 0;
      done = 0;
      fseek(fp2,0L,SEEK_END);
      filesize = ftell(fp2);
      rewind(fp2);
      fpkt = NULL;
/*...e*/
/*...snext message:0:*/
      for (;;)
      {
         filepos = ftell(fp2);
         if (filesize==0)
            percent = 100;
         else
            percent = filepos * 100 / filesize;
         fprintf(stderr,"\r%d%%",percent);
         switch(msgformat)
         {
         case FORMAT_USENET:
            fgets(buffer2,BUFSIZE-1,fp2);
            if (feof(fp2))
            {
               done = 1;
               break;
            }
            if (strncmp(buffer2,RNEWSHEADER,strlen(RNEWSHEADER))!=0)
            {
               badfile = 1;
               done = 1;
               break;
            }
            messagesize = atol(buffer2+9);
            msgpos = ftell(fp2);
            endpos = msgpos + messagesize;
            break;
         case FORMAT_MAILBOX:
            fgets(buffer2,BUFSIZE-1,fp2);
            if (feof(fp2))
            {
               done = 1;
               break;
            }
            if (strncmp(buffer2,"From ",5)!=0)
            {
               badfile = 1;
               done = 1;
               break;
            }
            msgpos = ftell(fp2);
            for (;;)
            {
               endpos = ftell(fp2);
               fgets(buffer2,BUFSIZE-1,fp2);
               if (feof(fp2))
               {
                  endpos = ftell(fp2);
                  break;
               }
               if (strncmp(buffer2,"From ",5)==0)
                  break;
            }
            break;
         case FORMAT_MMDF:
            while (fgetc(fp2)=='\1') ;
            if (feof(fp2))
            {
               done = 1;
               break;
            }
            fseek(fp2,-1L,SEEK_CUR);
            msgpos = ftell(fp2);
            for (;;)
            {
               endpos = ftell(fp2);
               fgets(buffer2,BUFSIZE-1,fp2);
               if (feof(fp2))
               {
                  endpos = ftell(fp2);
                  break;
               }
               bp = strstr(buffer2,"\1\1\1\1");
               if (bp!=NULL)
               {
                  endpos += bp-buffer2;
                  break;
               }
            }
            break;
         case FORMAT_BINARY:
            messagesize = (long)fgetc(fp2) << 24;
            messagesize |= (long)fgetc(fp2) << 16;
            messagesize |= (long)fgetc(fp2) << 8;
            messagesize |= (long)fgetc(fp2);
            if (feof(fp2))
            {
               done = 1;
               break;
            }
            msgpos = ftell(fp2);
            endpos = msgpos + messagesize;
            if (messagesize < 0 || endpos > filesize)
            {
               badfile = 1;
               done = 1;
               break;
            }
            break;
         }
         if (done)
            break;
/*...e*/
/*...swrite packet:0:*/
      if (fpkt==NULL
          || (maxpktsize!=0 && (ftell(fpkt)/1024)+2>=maxpktsize))
      {
         if (fpkt!=NULL)
         {
            closepkt(fpkt);
            if (pktmsgcount==0)
               remove(pktname);
         }
         genfname(indir,packetext,pktname);
         fpkt = _fsopen(pktname,"wb",SH_DENYWR);
         if (fpkt==NULL)
         {
            logprintf("Error writing packet %s\n",pktname);
            continue;
         }
         writepktheader(fpkt);
      }
/*...e*/
/*...sprocess message:0:*/
         routing = 0;
         routepwdfailed = 0;
         ismaillist = 0;
         isignoretext = 0;
         isownorg = 0;
         isjunk = 0; 
         islistno = -1;
         islistcommand = COMMAND_NONE;
         truncate = NULL;
         truncend = NULL;

         /* first step: analyze message */
         processimport(0,0,0,
                       0,&isjunk,
                       NULL,NULL,
                       fp2,fpkt,
                       msgpos, endpos, &bodypos,
                       linebuf,filebuf);

         if (msgtype==EMAIL)
            msgcategory = CAT_EMAIL;
         else
            msgcategory = CAT_NEWS;

/*...scharacter set translation:0:*/
         fidocharset[0] = '\0';
         for (i=0; i<charsetmaps; i++)
         {
            if (charsetmap[i].dir!=GATE_EXPORT
                && stricmp(rfccharset, charsetmap[i].rfccharset) == 0)
            {
               strcpy(fidocharset, charsetmap[i].fidocharset);
               cp3 = strrchr(fidocharset, '#');
               if (cp3!=NULL)
                  *cp3 = ' ';
               break;
            }
         }
/*...e*/
/*...sinitialize header fields:0:*/
         if (msgfromemail[0]=='\0')
         {
            if (msgfromname[0]=='\0')
               strcpy(msgfromemail,"anonymous");
            else
               strcpy(msgfromemail,msgfromname);
         }
         if (msgfromname[0]=='\0' && xfrom[0]!='\0')
         {
            strcpy(linebuf, xfrom);
            processemail(linebuf,NULL,msgfromname,EMAIL_FROM);
         }
         if (msgfromname[0]=='\0')
            strcpy(msgfromname,msgfromemail);
         if (msgtoemail[0]=='\0')
         {
            if (msgtoname[0]=='\0')
               strcpy(msgtoemail,"unknown");
            else
               strcpy(msgtoemail,msgtoname);
         }
         if (msgtoname[0]=='\0')
            strcpy(msgtoname,msgtoemail);
         if (msgreplyemail[0]=='\0')
            strcpy(msgreplyemail,msgfromemail);
         if (msgreplyname[0]=='\0')
            strcpy(msgreplyname,msgfromname);
/*...e*/
/*...sto\45\address translation:0:*/
         if (msgtype==EMAIL)
         {
            mapno = 0;
            for (i=0; i<addrmaps && mapno==0; i++)
            {
               thisismatch = 0;
               if (addrmap[i]->email[0]!='\0')
               {
                  if (stricmp(msgtoemail,addrmap[i]->email)==0)
                  {
#ifdef DEBUGSPECIAL
   logprintf("[Special Debug] AddrMap: email=%s match (entry %d)\n",
                    addrmap[i]->email, i+1);
#endif
                     mapno = i;
                     thisismatch = 1;
                  }
                  else
                  {
                     for (j=0; j<emaillistcount; j++)
                     {
                        if (emaillist[j].addr!=NULL
                            && stricmp(emaillist[j].addr,addrmap[i]->email)==0)
                        {
#ifdef DEBUGSPECIAL
   logprintf("[Special Debug] AddrMap: email=%s match (entry %d)\n",
                    addrmap[i]->email, i+1);
#endif
                           mapno = i;
                           thisismatch = 1;
                           break;
                        }
                     }
                  }
               }
               if (addrmap[i]->name[0]!='\0')
               {
                  if (thisismatch
                      && (msgtoname[0]=='\0'
                          || stricmp(msgtoname, msgtoemail)==0))
                  {
                     strcpy(msgtoname, addrmap[i]->name);
#ifdef DEBUGSPECIAL
   logprintf("[Special Debug] AddrMap: name changed to %s\n", msgtoname);
#endif
                  }
                  else if (stricmp(msgtoname,addrmap[i]->name)==0)
                  {
#ifdef DEBUGSPECIAL
   logprintf("[Special Debug] AddrMap: name=%s match (entry %d)\n",
                    addrmap[i]->name, i+1);
#endif
                     mapno = i;
                  }
                  else
                  {
                     for (j=0; j<emaillistcount; j++)
                     {
                        if (emaillist[j].name!=NULL
                            && stricmp(emaillist[j].name,addrmap[i]->name)==0)
                        {
#ifdef DEBUGSPECIAL
   logprintf("[Special Debug] AddrMap: name=%s match (entry %d)\n",
                    addrmap[i]->name, i+1);
#endif
                           mapno = i;
                           break;
                        }
                     }
                  }
               }
            }
            memcpy(&destaddr,&addrmap[mapno]->fido,sizeof(FIDOADDR));
         }
         else
         {
            memcpy(&destaddr,&addrmap[0]->fido,sizeof(FIDOADDR));
         }
/*...e*/
/*...sfrom\45\address translation:0:*/
         memcpy(&fromaddr,&gateaddr,sizeof(FIDOADDR));
         if (msgtype==EMAIL)
         {
            for (i=0; i<routes; i++)
            {
               if (stricmp(msgfromemail,route[i]->email)==0)
               {
                  routing = 1;
                  routeno = i;
                  if (route[i]->pwd[0]!='\0'
                      && strcmp(routemailpwd,route[i]->pwd)!=0)
                  {
                     routepwdfailed = 1;
                     fprintf(stderr,"\r");
                     logprintf("! Incorrect password for node %u:%u/%u",
                                  route[i]->fido.zone,
                                  route[i]->fido.net,
                                  route[i]->fido.node);
                     if (route[i]->fido.point>0)
                        logprintf(".%u",route[i]->fido.point);
                     logprintf("\n");
                  }
                  else
                     memcpy(&fromaddr,&route[i]->fido,sizeof(FIDOADDR));
                  break;
               }
            }
         }
/*...e*/
/*...slist messages detection:0:*/
         if (msgtype==EMAIL)
         {
            islistno = -1;
            for (i=0; i<lists; i++)
            {
               if (stricmp(msgtoemail, listconfig[i].listaddress)==0)
               {
                  islistno = i;
                  break;
               }
            }
            islistcommand = COMMAND_NONE;
            if (islistno!=-1)
            {
               if (memicmp(msgsubj, "subscribe",9)==0)
                  islistcommand = COMMAND_SUBSCRIBE;
               else if (memicmp(msgsubj, "unsubscribe",11)==0)
                  islistcommand = COMMAND_UNSUBSCRIBE;
               else if (memicmp(msgsubj, "changepass",10)==0)
                  islistcommand = COMMAND_CHANGEPASS;
            }
         }
/*...e*/
/*...sarea \38\ mailing\45\list translation:0:*/
         if (islistno!=-1)
         {
            msgcategory = CAT_LIST;
            if (listconfig[islistno].listarea==NULL)
               fidoareaname[0] = '\0';
            else
               strcpy(fidoareaname, listconfig[islistno].listarea);
         }
         else if (msgtype==EMAIL)
         {
            strcpy(fidoareaname,emailarea);
            for (i=0; i<maillists; i++)
            {
               if (substrcmp(maillist[i]->substr,msgfromname)
                   || substrcmp(maillist[i]->substr,msgfromemail)
                   || substrcmp(maillist[i]->substr,msgtoname)
                   || substrcmp(maillist[i]->substr,msgtoemail)
                   || substrcmp(maillist[i]->substr,msgreplyname)
                   || substrcmp(maillist[i]->substr,msgreplyemail)
                   || substrcmp(maillist[i]->substr,msgsubj)
                   || substrcmp(maillist[i]->substr,sender)
                   || substrcmp(maillist[i]->substr,xfrom))
               {
                  strcpy(fidoareaname,maillist[i]->area);
                  ismaillist = 1;
                  msgcategory = CAT_LIST;
                  if (maillist[i]->truncate[0]!='\0')
                     truncate = maillist[i]->truncate;
                  if (maillist[i]->truncend[0]!='\0')
                     truncend = maillist[i]->truncend;
                  if (maillist[i]->truncsubj[0]!='\0')
                  {
                     cp = strstr(msgsubj, maillist[i]->truncsubj);
                     if (cp!=NULL)
                        memmove(cp, cp+strlen(maillist[i]->truncsubj),
                                strlen(cp+strlen(maillist[i]->truncsubj)) + 1);
                  }
                  break;
               }
               for (j=0; j<emaillistcount; j++)
               {
                  if ((emaillist[j].name!=NULL
                       && substrcmp(maillist[i]->substr,emaillist[j].name))
                   || (emaillist[j].addr!=NULL
                       && substrcmp(maillist[i]->substr,emaillist[j].addr)))
                  {
                     strcpy(fidoareaname,maillist[i]->area);
                     ismaillist = 1;
                     msgcategory = CAT_LIST;
                     if (maillist[i]->truncate[0]!='\0')
                        truncate = maillist[i]->truncate;
                     if (maillist[i]->truncend[0]!='\0')
                        truncend = maillist[i]->truncend;
                     i = maillists; break;
                  }
               }
            }
            for (i=0; i<ignoretexts; i++)
            {
               if (ignoretext[i][0]=='*')
               {
                  if (stricmp(ignoretext[i]+1,msgfromname)==0
                      || stricmp(ignoretext[i]+1,msgfromemail)==0
                      || stricmp(ignoretext[i]+1,msgtoname)==0
                      || stricmp(ignoretext[i]+1,msgtoemail)==0
                      || stricmp(ignoretext[i]+1,msgreplyname)==0
                      || stricmp(ignoretext[i]+1,msgreplyemail)==0
                      || stricmp(ignoretext[i]+1,msgsubj)==0)
                  {
                     isignoretext = 1;
                     break;
                  }
               }
               else if (substrcmp(ignoretext[i],msgfromname)
                   || substrcmp(ignoretext[i],msgfromemail)
                   || substrcmp(ignoretext[i],msgtoname)
                   || substrcmp(ignoretext[i],msgtoemail)
                   || substrcmp(ignoretext[i],msgreplyname)
                   || substrcmp(ignoretext[i],msgreplyemail)
                   || substrcmp(ignoretext[i],msgsubj))
               {
                  isignoretext = 1;
                  break;
               }
               for (j=0; j<emaillistcount; j++)
               {
                  if ((emaillist[j].name!=NULL
                       && substrcmp(ignoretext[i],emaillist[j].name))
                   || (emaillist[j].addr!=NULL
                       && substrcmp(ignoretext[i],emaillist[j].addr)))
                  {
                     isignoretext = 1;
                     i = ignoretexts; break;
                  }
               }
            }
         }
         else
         {
            strcpy(fidoareaname,areaname);
            for (i=0; i<areamaps; i++)
            {
               if (stricmp(areaname,areamap[i]->usenet)==0)
               {
                  strcpy(fidoareaname,areamap[i]->echomail);
                  if (areamap[i]->direction==GATE_EXPORT)
                     isjunk = JUNK_IMPEX;
                  break;
               }
            }
         }
         strupr(fidoareaname);
/*...e*/
/*...squote\45\to translation:0:*/
         if ((msgtype==NEWS || ismaillist || (islistno!=-1))
             && (msgtoname[0]=='\0' || strcmp(msgtoname,msgtoemail)==0))
         {
            if (quotename[0]!='\0')
               strcpy(msgtoname,quotename);
            else if (quoteemail[0]!='\0')
               strcpy(msgtoname,quoteemail);
            else
               strcpy(msgtoname,"All");
         }

/*...e*/
/*...sMSGID\47\REPLY translation:0:*/
         validmsgid = 0;
         if (msgid[0]!='\0')
         {
            cp = strchr(msgid,'@');
            if (cp!=NULL
                && ismapdomain(cp+1)
                && sscanf(msgid,"%lu",&fidomsgid)==1)
            {
               msgidaddr.zone = 0;
               msgidaddr.net = 0;
               msgidaddr.node = 0;
               msgidaddr.point = 0;
               if (cp!=NULL && tolower(cp[1])=='p')
               {
                  sscanf(cp+2,"%hu",&msgidaddr.point);
                  cp = strchr(cp+1,'.');
               }
               if (cp!=NULL && tolower(cp[1])=='f')
               {
                  sscanf(cp+2,"%hu",&msgidaddr.node);
                  cp = strchr(cp+1,'.');
               }
               if (cp!=NULL && tolower(cp[1])=='n')
               {
                  sscanf(cp+2,"%hu",&msgidaddr.net);
                  cp = strchr(cp+1,'.');
               }
               if (cp!=NULL && tolower(cp[1])=='z')
               {
                  sscanf(cp+2,"%hu",&msgidaddr.zone);
                  cp = strchr(cp+1,'.');
               }
               if (cp!=NULL)
                  validmsgid = 1;
            }
         }
         validreplyid = 0;
         if (replyid[0]!='\0')
         {
            cp = strchr(replyid,'@');
            if (cp!=NULL
                && ismapdomain(cp+1)
                && sscanf(replyid,"%ld",&fidoreplyid)==1)
            {
               replyidaddr.zone = 0;
               replyidaddr.net = 0;
               replyidaddr.node = 0;
               replyidaddr.point = 0;
               if (cp!=NULL && tolower(cp[1])=='p')
               {
                  sscanf(cp+2,"%hu",&replyidaddr.point);
                  cp = strchr(cp+1,'.');
               }
               if (cp!=NULL && tolower(cp[1])=='f')
               {
                  sscanf(cp+2,"%hu",&replyidaddr.node);
                  cp = strchr(cp+1,'.');
               }
               if (cp!=NULL && tolower(cp[1])=='n')
               {
                  sscanf(cp+2,"%hu",&replyidaddr.net);
                  cp = strchr(cp+1,'.');
               }
               if (cp!=NULL && tolower(cp[1])=='z')
               {
                  sscanf(cp+2,"%hu",&replyidaddr.zone);
                  cp = strchr(cp+1,'.');
               }
               if (cp!=NULL)
               {
                  validreplyid = 1;
                  if (msgtype!=EMAIL && !ismaillist)
                  {
                     for (i=0; i<addrmaps; i++)
                     {
                        if (memcmp(&replyidaddr,&addrmap[i]->fido,sizeof(FIDOADDR))==0
                            && addrmap[i]->name[0]!='\0')
                        {
#ifdef DEBUGSPECIAL
   logprintf("[Special Debug] AddrMap: personal reply name=%s match (entry %d)\n",
                    addrmap[i]->name, i+1);
#endif
                           strcpy(msgtoname,addrmap[i]->name);
                           personalcount++;
                           break;
                        }
                     }
                  }
               }
            }
         }
/*...e*/
/*...sfidoorg translation:0:*/
         validfidoorgaddr = 0;
         cp = strchr(msgreplyemail,'@');
         if (cp!=NULL
             && ismapdomain(cp+1))
         {
            if (msgreplyname[0]=='\0'
                || stricmp(msgreplyname,msgreplyemail)==0)
            {
               j = 0;
               for (i=0; msgreplyemail[i]!='\0'; i++)
               {
                  if (msgreplyemail[i]=='@')
                     break;
                  else if (msgreplyemail[i]=='.')
                     msgreplyname[j++] = ' ';
                  else
                     msgreplyname[j++] = msgreplyemail[i];
               }
               msgreplyname[j] = '\0';
               if (msgfromname[0]=='\0'
                   || stricmp(msgfromname,msgfromemail)==0)
                  strcpy(msgfromname, msgreplyname);
            }
            fidoorgaddr.zone = 0;
            fidoorgaddr.net = 0;
            fidoorgaddr.node = 0;
            fidoorgaddr.point = 0;
            if (cp!=NULL && tolower(cp[1])=='p')
            {
               sscanf(cp+2,"%hu",&fidoorgaddr.point);
               cp = strchr(cp+1,'.');
            }
            if (cp!=NULL && tolower(cp[1])=='f')
            {
               sscanf(cp+2,"%hu",&fidoorgaddr.node);
               cp = strchr(cp+1,'.');
            }
            if (cp!=NULL && tolower(cp[1])=='n')
            {
               sscanf(cp+2,"%hu",&fidoorgaddr.net);
               cp = strchr(cp+1,'.');
            }
            if (cp!=NULL && tolower(cp[1])=='z')
            {
               sscanf(cp+2,"%hu",&fidoorgaddr.zone);
               cp = strchr(cp+1,'.');
            }
            if (cp!=NULL)
               validfidoorgaddr = 1;
         }
         if (validfidoorgaddr && transfidoorg)
            memcpy(&fromaddr,&fidoorgaddr,sizeof(FIDOADDR));

         validfidoorgaddr = 0;
         cp = strchr(msgtoemail,'@');
         if (cp!=NULL
             && ismapdomain(cp+1))
         {
            if (msgtoname[0]=='\0'
                || stricmp(msgtoname,msgtoemail)==0)
            {
               j = 0;
               for (i=0; msgtoemail[i]!='\0'; i++)
               {
                  if (msgtoemail[i]=='@')
                     break;
                  else if (msgtoemail[i]=='.')
                     msgtoname[j++] = ' ';
                  else
                     msgtoname[j++] = msgtoemail[i];
               }
               msgtoname[j] = '\0';
            }
            fidoorgaddr.zone = 0;
            fidoorgaddr.net = 0;
            fidoorgaddr.node = 0;
            fidoorgaddr.point = 0;
            if (cp!=NULL && tolower(cp[1])=='p')
            {
               sscanf(cp+2,"%hu",&fidoorgaddr.point);
               cp = strchr(cp+1,'.');
            }
            if (cp!=NULL && tolower(cp[1])=='f')
            {
               sscanf(cp+2,"%hu",&fidoorgaddr.node);
               cp = strchr(cp+1,'.');
            }
            if (cp!=NULL && tolower(cp[1])=='n')
            {
               sscanf(cp+2,"%hu",&fidoorgaddr.net);
               cp = strchr(cp+1,'.');
            }
            if (cp!=NULL && tolower(cp[1])=='z')
            {
               sscanf(cp+2,"%hu",&fidoorgaddr.zone);
               cp = strchr(cp+1,'.');
            }
            if (cp!=NULL)
               validfidoorgaddr = 1;
         }
         if (validfidoorgaddr && transfidoorg)
            memcpy(&destaddr,&fidoorgaddr,sizeof(FIDOADDR));

/*...e*/
/*...sorganization translation:0:*/
         if (msgorganization[0]!='\0' && strcmp(msgorganization,organization)==0)
         {
            isownorg = 1;
            isjunk = 0;
            ismaillist = 0;
            if (ownorgaction==ACTION_KILL)
               goto skipmessage;
            else if (ownorgaction==ACTION_MOVE)
               strcpy(fidoareaname,ownorgarea);
            else if (ownorgaction==ACTION_JUNK)
            {
               strcpy(fidoareaname,ownorgarea);
               strcpy(junkmailto, msgtoname);
               strcpy(msgtoname, "JunkMail");
            }
         }
/*...e*/
/*...sjunk mail detection:0:*/
/*...sjunk from:0:*/
         if (!isjunk && !ismaillist && !isownorg)
         {
            for (i=0; i<junkfroms; i++)
            {
               if (junkfrom[i].category!=JUNK_ALL
                   && ((junkfrom[i].category==JUNK_EMAIL && msgtype!=EMAIL)
                       || (junkfrom[i].category==JUNK_NEWS && msgtype!=NEWS)))
                  continue;
               if (emailcmp(junkfrom[i].text,msgfromemail)
                   || emailcmp(junkfrom[i].text,msgreplyemail))
               {
                  isjunk = JUNK_FROM;
                  break;
               }
            }
         }
/*...e*/
/*...sjunk subj:0:*/
         if (!isjunk && !ismaillist && !isownorg)
         {
            for (i=0; i<junksubjs; i++)
            {
               if (junkfrom[i].category!=JUNK_ALL
                   && ((junkfrom[i].category==JUNK_EMAIL && msgtype!=EMAIL)
                       || (junkfrom[i].category==JUNK_NEWS && msgtype!=NEWS)))
                  continue;
               if (substrcmp(junksubj[i].text,msgsubj))
               {
                  isjunk = JUNK_SUBJ;
                  break;
               }
            }
         }
/*...e*/
/*...sjunk group:0:*/
         if (!isjunk && !ismaillist && !isownorg)
         {
            for (i=0; i<junkgroups; i++)
            {
               if (junkfrom[i].category!=JUNK_ALL
                   && ((junkfrom[i].category==JUNK_EMAIL && msgtype!=EMAIL)
                       || (junkfrom[i].category==JUNK_NEWS && msgtype!=NEWS)))
                  continue;
               if (stricmp(junkgroup[i].text,areaname)==0)
               {
                  isjunk = JUNK_GROUP;
                  break;
               }
               for (j=0; j<xpostcount; j++)
               {
                  if (stricmp(junkgroup[i].text,xpost[j])==0)
                  {
                     isjunk = JUNK_GROUP;
                     break;
                  }
               }
               if (isjunk)
                  break;
            }
         }
/*...e*/
/*...sjunk str:0:*/
         if (!isjunk && !ismaillist && !isownorg)
         {
            for (i=0; i<junkstrs; i++)
            {
               if (junkfrom[i].category!=JUNK_ALL
                   && ((junkfrom[i].category==JUNK_EMAIL && msgtype!=EMAIL)
                       || (junkfrom[i].category==JUNK_NEWS && msgtype!=NEWS)))
                  continue;
               if (substrcmp(junkstr[i].text,msgsubj))
               {
                  isjunk = JUNK_STR;
                  break;
               }
            }
         }
/*...e*/
/*...sjunk from\47\to:0:*/
         if (!isjunk && junkfromto && msgtype==EMAIL
             && (stricmp(msgfromemail,msgtoemail)==0
                 || stricmp(msgreplyemail,msgtoemail)==0)
             && !ismaillist && !isownorg)
            isjunk = JUNK_FROMTO;
/*...e*/

#ifdef DEBUGSPECIAL
   logprintf("[Special Debug] isjunk: %d -- ismaillist: %d -- isownorg: %d -- islistno: %d -- islistcommand: %d\n",
             isjunk, ismaillist, isownorg, islistno, islistcommand);
#endif
/*...e*/
/*...sprocess message status:0:*/
         if (isjunk)
         {
            junkcount++;
            if (junkaction==ACTION_KILL)
               goto skipmessage;
            else if (junkaction==ACTION_MOVE)
               strcpy(fidoareaname,junkarea);
            else if (junkaction==ACTION_JUNK)
            {
               strcpy(fidoareaname,junkarea);
               strcpy(junkmailto, msgtoname);
               strcpy(msgtoname, "JunkMail");
            }
         }
         else if (islistno!=-1)
         {
            if (islistcommand!=COMMAND_NONE)
            {
               processlistcommand(islistno, islistcommand);
               listcmdcount++;
               goto skipmessage;
            }
            else
            {
               listmsgcount++;
               if (!sendlistmessage(islistno, fp2, bodypos, endpos)
                   || fidoareaname[0]=='\0')
                  goto skipmessage;
            }
         }

         if (ismaillist)
            maillistcount++;

         if (isownorg)
            ownorgcount++;

         if (isignoretext)
            ignoretextcount++;

/*...e*/
/*...sprint information:0:*/
         if (!isjunk && !ismaillist && !isownorg && !isignoretext
             && islistno==-1)
         {
            if (msgtype==EMAIL)
            {
               emailcount++;
               if (verbose)
               {
                  fprintf(stderr,"\r");
                  logprintf("- from %s",msgfromemail);
                  if (stricmp(msgfromname,msgfromemail)!=0)
                     logprintf(" (%s)",msgfromname);
                  logprintf(" to %s",msgtoemail);
                  if (stricmp(msgtoname,msgtoemail)!=0)
                     logprintf(" (%s)",msgtoname);
                  if (emaillistcount>0)
                     logprintf(" et al.");
                  logprintf("\n");
               }
            }
            else
               newscount++;
         }
/*...e*/

         /* second step: convert message */
         processimport(1,
                       isignoretext,
                       msgcategory,
                       isownorg,
                       &isjunk,
                       truncate,
                       truncend,
                       fp2,
                       fpkt,
                       msgpos, endpos, &bodypos,
                       linebuf, filebuf);

skipmessage:
         messagecount++;
         pktmsgcount++;

         /* point to next message */
         fseek(fp2,endpos,SEEK_SET);
      }
/*...e*/
/*...sclose message area:0:*/
      if (fpkt!=NULL)
      {
         closepkt(fpkt);
         if (pktmsgcount==0)
            remove(pktname);
      }
      fprintf(stderr,"\r");
      fclose(fp2);
      if (badfile)
      {
         strcpy(buffer, fname);
         cp = strrchr(buffer, '.');
         if (cp==NULL)
            cp = buffer + strlen(buffer);
         strcpy(cp, ".BAD");
         if (!testmode)
            rename(fname, buffer);
         logprintf("! Error in message file %s, renamed to %s\n",
                   fname, buffer);
      }
      else
      {
         if (!testmode && (messagecount>0 || filesize==0))
            remove(fname);
         if (!globalstats)
         {
            logprintf("-> %ld message(s) imported\n"
                      "mail: %-5ld     news: %-5ld     lists: %-5ld    junk: %-5ld\n"
                      "personal: %-5ld own org: %-5ld  files: %-5ld    ign/txt: %-5ld\n",
                      messagecount,
                      emailcount,newscount,maillistcount,junkcount,
                      personalcount,ownorgcount,filecount,ignoretextcount);
            if (lists>0)
               logprintf("listmsg: %-5ld  listcmd: %-5ld  reply: %-5ld\n",
                         listmsgcount, listcmdcount, listreplycount);
         }
         totalmessagecount += messagecount;
         totalemailcount += emailcount;
         totalnewscount += newscount;
         totalmaillistcount += maillistcount;
         totalignoretextcount += ignoretextcount;
         totaljunkcount += junkcount;
         totalownorgcount += ownorgcount;
         totalpersonalcount += personalcount;
         totalfilecount += filecount;
         totallistmsgcount += listmsgcount;
         totallistcmdcount += listcmdcount;
         totallistreplycount += listreplycount;
      }
/*...e*/
   }

/*...sclose:0:*/
   fclose(fp);

   strcpy(fname,soupdir);
   strcat(fname,AREAS);
   if (!testmode && !notalldone)
      remove(fname);

   if (!separatemsg)
   {
      /* updates REPLIES etc. */
      for (i=0; i<MAILTYPES; i++)
         genreplyfile(i);
   }

   if (totalmessagecount==0)
   {
      logprintf("No inbound messages found. Nothing to do.\n");
   }
   else if (globalstats)
   {
      logprintf("-> %ld message(s) imported\n"
                "mail: %-5ld     news: %-5ld     lists: %-5ld    junk: %-5ld\n"
                "personal: %-5ld own org: %-5ld  files: %-5ld    ign/txt: %-5ld\n",
                totalmessagecount,
                totalemailcount,totalnewscount,totalmaillistcount,totaljunkcount,
                totalpersonalcount,totalownorgcount,totalfilecount,totalignoretextcount);
      if (lists>0)
         logprintf("listmsg: %-5ld  listcmd: %-5ld  reply: %-5ld\n",
                   totallistmsgcount, totallistcmdcount, totallistreplycount);
   }

   free(linebuf);
   free(filebuf);
/*...e*/
}
/*...e*/
