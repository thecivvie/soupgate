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

/* SoupGate configuration functions */

#include "soupgate.h"

/*...sconfigwarn:0:*/
void configwarn(const char *text, long lineno, const char *fname)
{
   fprintf(stderr,"Warning: %s in line %ld of %s\n\n",text,lineno,fname);
}
/*...e*/

/*...sscanfidoaddr:0:*/
static void configscanfidoaddr(char *s, long lineno, char *filename,
                  FIDOADDR *addr)
{
   addr->zone = 0;
   addr->net = 0;
   addr->node = 0;
   addr->point = 0;
   if (sscanf(s,"%hu:%hu/%hu.%hu",
              &addr->zone,&addr->net,&addr->node,&addr->point) < 3)
      configwarn("invalid FTN address",lineno,filename);
}
/*...e*/
/*...sconfigscanemailaddr:0:*/
static void configscanemailaddr(char *s, long lineno, char *filename,
                   EMAILADDR addr)
{
   strncpy(addr,s,EMAILBUFSIZE-1);
   addr[EMAILBUFSIZE-1] = '\0';

   if (strchr(addr,'@') == NULL)
      configwarn("invalid email address",lineno,filename);
}
/*...e*/
/*...sconfigscanyesno:0:*/
static void configscanyesno(char *cp, long lineno, char *filename,
               int *yesno)
{
   if (stricmp(cp,"Yes")==0)
      *yesno = 1;
   else if (stricmp(cp,"No")==0)
      *yesno = 0;
   else
   {
      *yesno = 0;
      configwarn("invalid Yes/No parameter",lineno,filename);
   }
}
/*...e*/
/*...sconfigscantext:0:*/
static void configscantext(char *s, long lineno, char *filename,
              char *text, int textbufsize)
{
   strncpy(text,s,textbufsize-1);
   text[textbufsize-1] = '\0';
}
/*...e*/
/*...sconfigscandir:0:*/
static void configscandir(char *s, long lineno, char *filename,
             char *dir)
{
   int len;

   strncpy(dir,s,FILENAME_MAX-1);
   dir[FILENAME_MAX-1] = '\0';

   len = strlen(dir);
   if (
#ifndef __unix__
       dir[len-1]!='\\' &&
#endif
       dir[len-1]!='/' && len<FILENAME_MAX-1)
      strcat(dir, DEFDIRSEP);
}
/*...e*/
/*...sconfigscanushort:0:*/
static void configscanushort(char *s, long lineno, char *filename,
                unsigned short *number)
{
   *number = 0;
   if (sscanf(s,"%hu",number) < 1)
      configwarn("invalid number",lineno,filename);
}
/*...e*/
/*...sconfigscanulong:0:*/
static void configscanulong(char *s, long lineno, char *filename,
               unsigned long *number)
{
   *number = 0;
   if (sscanf(s,"%lu",number) < 1)
      configwarn("invalid number",lineno,filename);
}
/*...e*/

/*...sreadconfigfile:0:*/
int readconfigfile(char *configfile, char *argv0)
{
   static unsigned char junkcategory = JUNK_ALL;   /* shared by recursive */
   char configname[FILENAME_MAX]; /* for recursion */
   char buffer[BUFSIZE];
   FILE *fp;
   int i,j,len,strip;
   char *cp, *cp2, *cp3, *cp4, *cp5, *cp6;
   long lineno;
   int min,max;
   int yn;

   if (
#ifndef __unix__
       strchr(configfile,'\\')==NULL &&
#endif
       strchr(configfile,'/')==NULL)
   {
      strncpy(configname,argv0,FILENAME_MAX-1);
#ifndef __unix__
      cp = strrchr(configname,'\\');
      if (cp==NULL)
#endif
         cp = strrchr(configname,'/');
      if (cp==NULL)
         cp = configname;
      else
         cp++;
      strcpy(cp,configfile);
   }
   else
      strcpy(configname,configfile);

   fp = _fsopen(configname,"r",SH_DENYNO);
   if (fp==NULL)
   {
      fprintf(stderr,"Error: can't read %s\n",configname);
      return 1;
   }

   lineno = 0;
   for (;;)
   {
      if (!fgets(buffer,BUFSIZE-1,fp))
         break;
      lineno++;
      len = strlen(buffer);
      /* strip both CR and LF (in case reading DOS file on unix) */
      while (len>0
             && (buffer[len-1]=='\n' || buffer[len-1]=='\r'))
         len--;
      buffer[len] = '\0';
      if (buffer[0]==';' || buffer[0]=='\0')
         continue;

      cp = strchr(buffer,' ');
      if (cp==NULL)
         cp = buffer + strlen(buffer);
      else
      {
         while (*cp==' ')
            *(cp++) = '\0';
      }

      if (stricmp(buffer,"FidoAddr")==0)
      {
         configscanfidoaddr(cp,lineno,configname,&gateaddr);
      }
      else if (stricmp(buffer,"AddrMap")==0)
      {
         if (addrmaps>=MAXADDRMAPS)
         {
            configwarn("too many AddrMap keywords",lineno,configname);
            continue;
         }
         cp2 = strchr(cp,' ');
         if (cp2==NULL)
         {
            configwarn("AddrMap misses parameter",lineno,configname);
            continue;
         }
         while (*cp2==' ')
            *(cp2++) = '\0';
         cp3 = strchr(cp2,' ');
         if (cp3!=NULL)
         {
            while (*cp3==' ')
               *(cp3++) = '\0';
         }
         cp4 = strchr(cp2, ',');
         if (cp4!=NULL)
            *(cp4++) = '\0';
         addrmap[addrmaps] = malloc(sizeof(ADDRMAP));
         if (addrmap[addrmaps]==NULL)
         {
            configwarn("out of memory",lineno,configname);
            continue;
         }
         configscanfidoaddr(cp,lineno,configname,&addrmap[addrmaps]->fido);
         configscanemailaddr(cp2,lineno,configname,addrmap[addrmaps]->email);
         if (cp4==NULL)
            addrmap[addrmaps]->replyto[0] = '\0';
         else
            configscanemailaddr(cp4,lineno,configname,addrmap[addrmaps]->replyto);
         if (cp3==NULL)
            addrmap[addrmaps]->name[0] = '\0';
         else
            configscantext(cp3,lineno,configname,addrmap[addrmaps]->name,TEXTBUFSIZE);
         addrmaps++;
      }
      else if (stricmp(buffer,"MapFidoOrg")==0)
      {
         configscanyesno(cp,lineno,configname,&mapfidoorg);
      }
      else if (stricmp(buffer,"TransFidoOrg")==0)
      {
         configscanyesno(cp,lineno,configname,&transfidoorg);
      }
      else if (stricmp(buffer,"FidoDomain")==0)
      {
         if (fidodomains>=MAXFIDODOMAINS)
         {
            configwarn("too many FidoDomain keywords",lineno,configname);
            continue;
         }
         cp2 = strchr(cp,' ');
         if (cp2==NULL)
         {
            configwarn("FidoDomain misses parameter",lineno,configname);
            continue;
         }
         while (*cp2==' ')
            *(cp2++) = '\0';
         if (strchr(cp,'-')==NULL)
         {
            if (sscanf(cp,"%u",
                       &fidodomain[fidodomains].minzone)
                       != 1)
            {
               configwarn("FidoDomain has invalid parameter",lineno,configname);
               continue;
            }
            fidodomain[fidodomains].maxzone
               = fidodomain[fidodomains].minzone;
         }
         else
         {
            if (sscanf(cp,"%u-%u",
                       &fidodomain[fidodomains].minzone,
                       &fidodomain[fidodomains].maxzone)
                       != 2)
            {
               configwarn("FidoDomain has invalid parameter",lineno,configname);
               continue;
            }
         }
         fidodomain[fidodomains].domain = malloc(strlen(cp2)+1);
         if (fidodomain[fidodomains].domain==NULL)
         {
            configwarn("out of memory",lineno,configname);
            continue;
         }
         strcpy(fidodomain[fidodomains].domain, cp2);
         fidodomains++;
      }
      else if (stricmp(buffer,"HeaderInfo")==0)
      {
         cp2 = strchr(cp,' ');
         if (cp2!=NULL)
         {
            while (*cp2==' ')
               *(cp2++) = '\0';
         }
         if (cp2==NULL)
         {
            configwarn("HeaderInfo misses parameter",lineno,configname);
            continue;
         }
         if (stricmp(cp,"Name")==0)
            infoheader = INFO_NAME;
         else if (stricmp(cp,"Email")==0)
            infoheader = INFO_EMAIL;
         else
         {
            configwarn("HeaderInfo has invalid parameter",lineno,configname);
            continue;
         }
         configscanyesno(cp2,lineno,configname,&yn);
         if (yn)
            infoinsert = (infoheader==INFO_NAME) ? INFO_EMAIL : INFO_NAME;
         else
            infoinsert = INFO_NONE;
      }
      else if (stricmp(buffer,"NoSpamText")==0)
      {
         configscantext(cp,lineno,configname,nospamtext,NOSPAMBUFSIZE);
      }
      else if (stricmp(buffer,"PacketPwd")==0)
      {
         configscantext(cp,lineno,configname,packetpwd,PKTPWDBUFSIZE);
         strupr(packetpwd);
      }
      else if (stricmp(buffer,"DefCharSet")==0)
      {
         configscantext(cp,lineno,configname,defcharset,TEXTBUFSIZE);
      }
      else if (stricmp(buffer,"CharSetMap")==0)
      {
         if (charsetmaps>=MAXCHARSETMAPS)
         {
            configwarn("too many CharSetMap keywords",lineno,configname);
            continue;
         }
         cp2 = strchr(cp,' ');
         if (cp2==NULL)
         {
            configwarn("CharSetMap misses parameter",lineno,configname);
            continue;
         }
         while (*cp2==' ')
            *(cp2++) = '\0';
         cp3 = wordfind("Export", cp2);
         if (cp3==NULL)
         {
            cp3 = wordfind("Import", cp2);
            if (cp3==NULL)
               charsetmap[charsetmaps].dir = GATE_BOTH;
            else
               charsetmap[charsetmaps].dir = GATE_IMPORT;
         }
         else
            charsetmap[charsetmaps].dir = GATE_EXPORT;
         if (cp3!=NULL)
         {
            while (cp3[-1]==' ')
            {
               cp3--;
               *cp3 = '\0';
            }
         }
         charsetmap[charsetmaps].fidocharset = malloc(strlen(cp)+1);
         if (charsetmap[charsetmaps].fidocharset==NULL)
         {
            configwarn("out of memory",lineno,configname);
            continue;
         }
         strcpy(charsetmap[charsetmaps].fidocharset, cp);
         charsetmap[charsetmaps].rfccharset = malloc(strlen(cp2)+1);
         if (charsetmap[charsetmaps].rfccharset==NULL)
         {
            configwarn("out of memory",lineno,configname);
            continue;
         }
         strcpy(charsetmap[charsetmaps].rfccharset, cp2);
         charsetmaps++;
      }
      else if (stricmp(buffer,"QuoteTo")==0)
      {
         configscanyesno(cp,lineno,configname,&quoteto);
      }
      else if (stricmp(buffer,"QuoteChar")==0)
      {
         if (quotechars>=MAXQUOTECHARS)
         {
            configwarn("too many QuoteChar keywords",lineno,configname);
            continue;
         }
         cp2 = strchr(cp,' ');
         if (cp2==NULL)
         {
            configwarn("QuoteChar misses parameter",lineno,configname);
            continue;
         }
         while (*cp2==' ')
            *(cp2++) = '\0';
         cp3 = strchr(cp2, ' ');
         if (cp3!=NULL)
         {
            while (*cp3==' ')
               *(cp3++) = '\0';
         }
         quotechar[quotechars].qchar = *cp;
         if (sscanf(cp2,"%d-%d",&min,&max)!=2
             || min<0 || min>255 || max<0 || max>255 || min>max)
         {
            configwarn("QuoteChar has invalid parameter",lineno,configname);
            continue;
         }
         quotechar[quotechars].mincol = min;
         quotechar[quotechars].maxcol = max;
         min = 0;
         max = 0;
         if (cp3!=NULL)
         {
            if (sscanf(cp3,"%d-%d",&min,&max)!=2
                || min<0 || min>255 || max<0 || max>255 || min>max)
            {
               configwarn("QuoteChar has invalid parameter",lineno,configname);
               continue;
            }
         }
         quotechar[quotechars].minid = min;
         quotechar[quotechars].maxid = max;
         quotechars++;
      }
      else if (stricmp(buffer,"QuoteMask")==0)
      {
         if (quotemasks>=MAXQUOTEMASKS)
         {
            configwarn("too many QuoteMask keywords",lineno,configname);
            continue;
         }
         quotemask[quotemasks] = malloc(strlen(cp)+1);
         if (quotemask[quotemasks]==NULL)
         {
            configwarn("out of memory",lineno,configname);
            continue;
         }
         strcpy(quotemask[quotemasks], cp);
         quotemasks++;
      }
      else if (stricmp(buffer,"ImportLimit")==0)
      {
         configscanulong(cp,lineno,configname,&importlimit);
         if (importlimit!=0 && importlimit<1024)
         {
            configwarn("limit too small",lineno,configname);
            importlimit = 2048;
         }
      }
      else if (stricmp(buffer,"MaxPKTSize")==0)
      {
         configscanulong(cp,lineno,configname,&maxpktsize);
         if (maxpktsize!=0 && maxpktsize<4)
         {
            configwarn("limit too small",lineno,configname);
            maxpktsize = 4;
         }
      }
      else if (stricmp(buffer,"SeparateMSG")==0)
      {
         configscanyesno(cp,lineno,configname,&separatemsg);
      }
      else if (stricmp(buffer,"LineWidth")==0)
      {
         configscanushort(cp,lineno,configname,&souplinewidth);
         if (souplinewidth < MINSOUPLINESIZE)
         {
            configwarn("line width too small",lineno,configname);
            souplinewidth = MINSOUPLINESIZE;
         }
         else if (souplinewidth > MAXSOUPLINESIZE)
         {
            configwarn("line width too large",lineno,configname);
            souplinewidth = MAXSOUPLINESIZE;
         }
      }
      else if (stricmp(buffer,"UseQP")==0)
      {
         configscanyesno(cp,lineno,configname,&useqp);
      }
      else if (stricmp(buffer,"Origin")==0)
      {
         configscantext(cp,lineno,configname,origin,ORIGINBUFSIZE);
      }
      else if (stricmp(buffer,"Organization")==0)
      {
         configscantext(cp,lineno,configname,organization,ORGANBUFSIZE);
      }
      else if (stricmp(buffer,"OwnOrgAction")==0)
      {
         if (stricmp(cp,"ignore")==0)
            ownorgaction = ACTION_IGNORE;
         else if (stricmp(cp,"kill")==0)
            ownorgaction = ACTION_KILL;
         else if (stricmp(cp,"move")==0)
            ownorgaction = ACTION_MOVE;
         else if (stricmp(cp,"junk")==0)
            ownorgaction = ACTION_JUNK;
         else
         {
            ownorgaction = ACTION_IGNORE;
            configwarn("invalid action",lineno,configname);
         }
      }
      else if (stricmp(buffer,"OwnOrgArea")==0)
      {
         configscantext(cp,lineno,configname,ownorgarea,AREABUFSIZE);
         strupr(ownorgarea);
      }
      else if (stricmp(buffer,"JunkAction")==0)
      {
         if (stricmp(cp,"ignore")==0)
            junkaction = ACTION_IGNORE;
         else if (stricmp(cp,"kill")==0)
            junkaction = ACTION_KILL;
         else if (stricmp(cp,"move")==0)
            junkaction = ACTION_MOVE;
         else if (stricmp(cp,"junk")==0)
            junkaction = ACTION_JUNK;
         else
         {
            junkaction = ACTION_IGNORE;
            configwarn("invalid action",lineno,configname);
         }
      }
      else if (stricmp(buffer,"JunkArea")==0)
      {
         configscantext(cp,lineno,configname,junkarea,AREABUFSIZE);
         strupr(junkarea);
      }
      else if (stricmp(buffer,"JunkCategory")==0)
      {
         if (stricmp(cp,"Email")==0)
            junkcategory = JUNK_EMAIL;
         else if (stricmp(cp,"News")==0)
            junkcategory = JUNK_NEWS;
         else if (stricmp(cp,"All")==0)
            junkcategory = JUNK_ALL;
         else if (stricmp(cp,"None")==0)
            junkcategory = JUNK_NONE;
         else
            configwarn("invalid category",lineno,configname);
      }
      else if (stricmp(buffer,"JunkFrom")==0)
      {
         if (junkfroms>=MAXJUNKFROMS)
         {
            configwarn("too many JunkFrom keywords",lineno,configname);
            continue;
         }
         junkfrom[junkfroms].category = junkcategory;
         junkfrom[junkfroms].text = malloc(strlen(cp)+1);
         if (junkfrom[junkfroms].text==NULL)
         {
            configwarn("out of memory",lineno,configname);
            continue;
         }
         strcpy(junkfrom[junkfroms].text,cp);
         junkfroms++;
      }
      else if (stricmp(buffer,"JunkSubj")==0)
      {
         if (junksubjs>=MAXJUNKSUBJS)
         {
            configwarn("too many JunkSubj keywords",lineno,configname);
            continue;
         }
         junksubj[junksubjs].category = junkcategory;
         junksubj[junksubjs].text = malloc(strlen(cp)+1);
         if (junksubj[junksubjs].text==NULL)
         {
            configwarn("out of memory",lineno,configname);
            continue;
         }
         strcpy(junksubj[junksubjs].text,cp);
         junksubjs++;
      }
      else if (stricmp(buffer,"JunkStr")==0)
      {
         if (junkstrs>=MAXJUNKSTRS)
         {
            configwarn("too many JunkStr keywords",lineno,configname);
            continue;
         }
         junkstr[junkstrs].category = junkcategory;
         junkstr[junkstrs].text = malloc(strlen(cp)+1);
         if (junkstr[junkstrs].text==NULL)
         {
            configwarn("out of memory",lineno,configname);
            continue;
         }
         strcpy(junkstr[junkstrs].text,cp);
         junkstrs++;
      }
      else if (stricmp(buffer,"JunkGroup")==0)
      {
         if (junkgroups>=MAXJUNKGROUPS)
         {
            configwarn("too many JunkGroup keywords",lineno,configname);
            continue;
         }
         junkgroup[junkgroups].category = junkcategory;
         junkgroup[junkgroups].text = malloc(strlen(cp)+1);
         if (junkgroup[junkgroups].text==NULL)
         {
            configwarn("out of memory",lineno,configname);
            continue;
         }
         strcpy(junkgroup[junkgroups].text,cp);
         junkgroups++;
      }
      else if (stricmp(buffer,"JunkFromTo")==0)
      {
         configscanyesno(cp,lineno,configname,&junkfromto);
      }
      else if (stricmp(buffer,"ExportMaxTo")==0)
      {
         configscanushort(cp,lineno,configname,&exportmaxto);
      }
      else if (stricmp(buffer,"ExportMaxXPost")==0)
      {
         configscanushort(cp,lineno,configname,&exportmaxxpost);
      }
      else if (stricmp(buffer,"EmailArea")==0)
      {
         configscantext(cp,lineno,configname,emailarea,AREABUFSIZE);
         strupr(emailarea);
         if (emailarea[0]=='\0')
            strcpy(emailarea,NETMAILAREANAME);
      }
      else if (stricmp(buffer,"AreaMap")==0)
      {
         if (areamaps>=MAXAREAMAPS)
         {
            configwarn("too many AreaMap keywords",lineno,configname);
            continue;
         }
         cp2 = strchr(cp,' ');
         if (cp2==NULL)
         {
            configwarn("AreaMap misses parameter",lineno,configname);
            continue;
         }
         while (*cp2==' ')
            *(cp2++) = '\0';
         cp3 = strchr(cp2,' ');
         if (cp3!=NULL)
         {
            while (*cp3==' ')
               *(cp3++) = '\0';
         }
         areamap[areamaps] = malloc(sizeof(AREAMAP));
         if (areamap[areamaps]==NULL)
         {
            configwarn("out of memory",lineno,configname);
            continue;
         }
         configscantext(cp,lineno,configname,areamap[areamaps]->echomail,AREABUFSIZE);
         strupr(areamap[areamaps]->echomail);
         configscantext(cp2,lineno,configname,areamap[areamaps]->usenet,AREABUFSIZE);
         areamap[areamaps]->direction = GATE_BOTH;
         if (cp3!=NULL)
         {
            if (stricmp(cp3,"import")==0)
               areamap[areamaps]->direction = GATE_IMPORT;
            else if (stricmp(cp3,"export")==0)
               areamap[areamaps]->direction = GATE_EXPORT;
            else
               configwarn("unknown direction in line %ld of %s\n\n",
                              lineno,configname);
         }
         areamaps++;
      }
      else if (stricmp(buffer,"StripKludges")==0)
      {
         cp2 = strchr(cp,' ');
         if (cp2!=NULL)
         {
            while (*cp2==' ')
               *(cp2++) = '\0';
         }
         if (stricmp(cp, "Yes") == 0)
            strip = STRIP_ALL;
         else if (stricmp(cp, "No") == 0)
            strip = STRIP_NONE;
         else if (stricmp(cp, "Some") == 0)
            strip = STRIP_SOME;
         else
         {
            configwarn("unknown StripKludges setting in line %ld of %s\n\n",
                           lineno,configname);
            strip = STRIP_NONE;
         }
         if (cp2==NULL || stricmp(cp2,"All")==0)
         {
            for (i=0; i<MSGCATEGORIES; i++)
               stripkludges[i] = strip;
         }
         else if (stricmp(cp2, "Email") == 0)
            stripkludges[CAT_EMAIL] = strip;
         else if (stricmp(cp2, "News") == 0)
            stripkludges[CAT_NEWS] = strip;
         else if (stricmp(cp2, "List") == 0)
            stripkludges[CAT_LIST] = strip;
         else
         {
            configwarn("unknown StripKludges category in line %ld of %s\n\n",
                           lineno,configname);
         }
      }
      else if (stricmp(buffer,"StrictFidoMsgID")==0)
      {
         configscanyesno(cp,lineno,configname,&strictfidomsgid);
      }
      else if (stricmp(buffer,"KillOrigin")==0)
      {
         if (stricmp(cp, "Yes")==0)
            killorigin = KILL_ALL;
         else if (stricmp(cp, "No")==0)
            killorigin = KILL_NONE;
         else if (stricmp(cp, "Email")==0)
            killorigin = KILL_EMAIL;
         else
            configwarn("invalid KillOrigin parameter",lineno,configname);
      }
      else if (stricmp(buffer,"LogFile")==0)
      {
         configscantext(cp,lineno,configname,logfile,FILENAME_MAX);
         if (
#ifndef __unix__
             strchr(logfile,'\\')==NULL &&
#endif
             strchr(logfile,'/')==NULL)
         {
            strncpy(logfile,argv0,FILENAME_MAX-1);
#ifndef __unix__
            cp2 = strrchr(logfile,'\\');
            if (cp2==NULL)
#endif
               cp2 = strrchr(logfile,'/');
            if (cp2==NULL)
               cp2 = logfile;
            else
               cp2++;
            *cp2 = '\0';
            configscantext(cp,lineno,configname,cp2,FILENAME_MAX - strlen(logfile));
         }
      }
      else if (stricmp(buffer,"TimeZone")==0)
      {
         configscantext(cp,lineno,configname,timezonestr,TZBUFSIZE);
      }
      else if (stricmp(buffer,"QuickScan")==0)
      {
         configscanyesno(cp,lineno,configname,&quickscan);
      }
      else if (stricmp(buffer,"MailerType")==0)
      {
         if (stricmp(cp,"ArcMail")==0)
            mailertype = MAILER_ARCMAIL;
         else if (stricmp(cp,"Binkley")==0)
            mailertype = MAILER_BINKLEY;
         else
         {
            mailertype = MAILER_NONE;
            configwarn("invalid mailer type",lineno,configname);
         }
      }
      else if (stricmp(buffer,"ExportAllMail")==0)
      {
         configscanyesno(cp,lineno,configname,&exportallmail);
      }
      else if (stricmp(buffer,"InboundDir")==0)
      {
         configscandir(cp,lineno,configname,indir);
      }
      else if (stricmp(buffer,"OutboundDir")==0)
      {
         configscandir(cp,lineno,configname,outdir);
      }
      else if (stricmp(buffer,"NetmailDir")==0)
      {
         configscandir(cp,lineno,configname,netdir);
      }
      else if (stricmp(buffer,"DecodeDir")==0)
      {
         decodedirall = 0;
         cp2 = strchr(cp,' ');
         if (cp2!=NULL)
         {
            *(cp2++) = '\0';
            if (memicmp(cp2,"ALL",3)==0)
               decodedirall = 1;
            else
               configwarn("invalid DecodeDir parameter",lineno,configname);
         }
         configscandir(cp,lineno,configname,decodedir);
      }
      else if (stricmp(buffer,"SoupDir")==0)
      {
         configscandir(cp,lineno,configname,soupdir);
      }
      else if (stricmp(buffer,"TempDir")==0)
      {
         configscandir(cp,lineno,configname,tempdir);
      }
      else if (stricmp(buffer,"BaseZone")==0)
      {
         configscanushort(cp,lineno,configname,&basezone);
      }
      else if (stricmp(buffer,"AttachMail")==0)
      {
         if (stricmp(cp,"ignore")==0)
            attach[EMAIL] = ATTACH_IGNORE;
         else if (stricmp(cp,"decode")==0)
            attach[EMAIL] = ATTACH_DECODE;
         else if (stricmp(cp,"kill")==0)
            attach[EMAIL] = ATTACH_KILL;
         else
         {
            attach[EMAIL] = ATTACH_DECODE;
            configwarn("invalid action",lineno,configname);
         }
      }
      else if (stricmp(buffer,"AttachNews")==0)
      {
         if (stricmp(cp,"ignore")==0)
            attach[NEWS] = ATTACH_IGNORE;
         else if (stricmp(cp,"decode")==0)
            attach[NEWS] = ATTACH_DECODE;
         else if (stricmp(cp,"kill")==0)
            attach[NEWS] = ATTACH_KILL;
         else
         {
            attach[NEWS] = ATTACH_IGNORE;
            configwarn("invalid action",lineno,configname);
         }
      }
      else if (stricmp(buffer,"EncodeFormat")==0)
      {
         if (stricmp(cp,"MIME")==0)
            encodeformat = ENCODE_MIME;
         else if (stricmp(cp,"UU")==0)
            encodeformat = ENCODE_UU;
         else
         {
            encodeformat = ENCODE_MIME;
            configwarn("invalid format",lineno,configname);
         }
      }
      else if (stricmp(buffer,"Route")==0)
      {
         if (routes>=MAXROUTES)
         {
            configwarn("too many Route keywords",lineno,configname);
            continue;
         }
         cp2 = strchr(cp,' ');
         if (cp2!=NULL)
         {
            while (*cp2==' ')
               *(cp2++) = '\0';
            cp3 = strchr(cp2,' ');
            if (cp3!=NULL)
            {
               while (*cp3==' ')
                  *(cp3++) = '\0';
               cp4 = strchr(cp3,' ');
               if (cp4!=NULL)
               {
                  while (*cp4==' ')
                     *(cp4++) = '\0';
               }
            }
            else
               cp4 = NULL;
         }
         if (cp2==NULL)
         {
            configwarn("Route misses parameter",lineno,configname);
            continue;
         }
         if (cp3!=NULL && memicmp(cp3,"SUBJ=",5)==0)
         {
            if (cp4!=NULL)
            {
               configwarn("Route has too many parameters",lineno,configname);
               continue;
            }
            cp4 = cp3;
            cp3 = NULL;
         }
         else if (cp4!=NULL && memicmp(cp4,"SUBJ=",5)!=0)
         {
            configwarn("Route has invalid parameter",lineno,configname);
            continue;
         }
         route[routes] = malloc(sizeof(ROUTE));
         if (route[routes]==NULL)
         {
            configwarn("out of memory",lineno,configname);
            continue;
         }
         configscanfidoaddr(cp,lineno,configname,&route[routes]->fido);
         configscanemailaddr(cp2,lineno,configname,route[routes]->email);
         if (cp3==NULL)
            route[routes]->pwd[0] = '\0';
         else
            configscantext(cp3,lineno,configname,route[routes]->pwd,ROUTEPWDBUFSIZE);
         if (cp4==NULL)
            route[routes]->mailsubj[0] = '\0';
         else
            configscantext(cp4+5,lineno,configname,route[routes]->mailsubj,TEXTBUFSIZE);
         routes++;
      }
      else if (stricmp(buffer,"Alias")==0)
      {
         if (aliases>=MAXALIASES)
         {
            configwarn("too many Alias keywords",lineno,configname);
            continue;
         }
         cp2 = strchr(cp,' ');
         if (cp2!=NULL)
         {
            while (*cp2==' ')
               *(cp2++) = '\0';
         }
         if (cp2==NULL)
         {
            configwarn("Alias misses parameter",lineno,configname);
            continue;
         }
         alias[aliases] = malloc(sizeof(ALIAS));
         if (alias[aliases]==NULL)
         {
            configwarn("out of memory",lineno,configname);
            continue;
         }
         configscanemailaddr(cp,lineno,configname,alias[aliases]->email);
         configscantext(cp2,lineno,configname,alias[aliases]->alias,TEXTBUFSIZE);
         aliases++;
      }
      else if (stricmp(buffer,"Personal")==0)
      {
         configwarn("obsolete keyword Personal",lineno,configname);
         if (addrmaps>=MAXADDRMAPS)
         {
            configwarn("too many AddrMap/Personal keywords",lineno,configname);
            continue;
         }
         cp2 = strchr(cp,' ');
         if (cp2!=NULL)
         {
            while (*cp2==' ')
               *(cp2++) = '\0';
         }
         if (cp2==NULL)
         {
            configwarn("Personal misses parameter",lineno,configname);
            continue;
         }
         addrmap[addrmaps] = malloc(sizeof(ADDRMAP));
         if (addrmap[addrmaps]==NULL)
         {
            configwarn("out of memory",lineno,configname);
            continue;
         }
         configscanfidoaddr(cp,lineno,configname,&addrmap[addrmaps]->fido);
         configscantext(cp2,lineno,configname,addrmap[addrmaps]->name,TEXTBUFSIZE);
         addrmap[addrmaps]->email[0] = '\0';
         addrmaps++;
      }
      else if (stricmp(buffer,"MailingList")==0)
      {
         if (maillists>=MAXMAILLISTS)
         {
            configwarn("too many MailingList keywords",lineno,configname);
            continue;
         }
         cp2 = strchr(cp,' ');
         if (cp2!=NULL)
         {
            while (*cp2==' ')
               *(cp2++) = '\0';
            cp3 = strchr(cp2,' ');
            if (cp3!=NULL)
            {
               while (*cp3==' ')
                  *(cp3++) = '\0';
               cp5 = strstr(cp3, "TRUNCATE=");
               if (cp5!=NULL)
               {
                  for (i=1; cp5>=cp3+i; i++)
                  {
                     if (*(cp5-i)==' ')
                        *(cp5-i) = '\0';
                     else
                        break;
                  }
                  cp5 += 9;
                  cp6 = strstr(cp5, ";SUBJ:");
                  if (cp6!=NULL)
                  {
                     if (cp6==cp5)
                        cp5 = NULL;
                     else
                        *(cp6-1) = '\0';
                     cp6 += 6;
                  }
               }
               else
                 cp6 = NULL;
            }
            cp4 = strchr(cp2,',');
            if (cp4!=NULL)
               *(cp4++) = '\0';
         }
         if (cp2==NULL || cp3==NULL)
         {
            configwarn("MailingList misses parameter",lineno,configname);
            continue;
         }
         maillist[maillists] = malloc(sizeof(MAILLIST));
         if (maillist[maillists]==NULL)
         {
            configwarn("out of memory",lineno,configname);
            continue;
         }
         configscantext(cp,lineno,configname,maillist[maillists]->area,AREABUFSIZE);
         configscanemailaddr(cp2,lineno,configname,maillist[maillists]->postaddr);
         if (cp4!=NULL)
            configscanemailaddr(cp4,lineno,configname,maillist[maillists]->fromaddr);
         else
            maillist[maillists]->fromaddr[0] = '\0';
         configscantext(cp3,lineno,configname,maillist[maillists]->substr,TEXTBUFSIZE);
         if (cp5!=NULL)
            configscantext(cp5,lineno,configname,maillist[maillists]->truncate,TEXTBUFSIZE);
         else
            maillist[maillists]->truncate[0] = '\0';
         maillist[maillists]->truncend[0] = '\0';
         if (cp6!=NULL)
            configscantext(cp6,lineno,configname,maillist[maillists]->truncsubj,TEXTBUFSIZE);
         else
            maillist[maillists]->truncsubj[0] = '\0';
         maillist[maillists]->authorinfo = INFO_NONE;
         maillists++;
      }
      else if (stricmp(buffer,"MailingListOption")==0)
      {
         cp2 = strchr(cp,' ');
         if (cp2==NULL)
         {
            configwarn("MailingListOption misses parameter",lineno,configname);
            continue;
         }
         while (*cp2==' ')
            *(cp2++) = '\0';
         j = -1;
         for (i=0; i<maillists; i++)
         {
            if (stricmp(maillist[i]->area, cp)==0)
            {
               j = i;
               break;
            }
         }
         if (j==-1)
         {
            configwarn("MailingListOption references undefined mailing list area",lineno,configname);
            continue;
         }
         if (memicmp(cp2, "truncate=",9)==0)
            configscantext(cp2+9,lineno,configname,maillist[j]->truncate,TEXTBUFSIZE);
         else if (memicmp(cp2, "truncend=",9)==0)
            configscantext(cp2+9,lineno,configname,maillist[j]->truncend,TEXTBUFSIZE);
         else if (memicmp(cp2, "truncsubj=",10)==0)
            configscantext(cp2+10,lineno,configname,maillist[j]->truncsubj,TEXTBUFSIZE);
         else if (memicmp(cp2, "author=",7)==0)
         {
            if (stricmp(cp2+7, "both")==0)
               maillist[j]->authorinfo = INFO_BOTH;
            else if (stricmp(cp2+7, "email")==0)
               maillist[j]->authorinfo = INFO_EMAIL;
            else if (stricmp(cp2+7, "name")==0)
               maillist[j]->authorinfo = INFO_NAME;
            else if (stricmp(cp2+7, "none")==0)
               maillist[j]->authorinfo = INFO_NONE;
            else
               configwarn("MailingListOption specifies invalid author= option",lineno,configname);
         }
         else
            configwarn("MailingListOption specifies invalid option",lineno,configname);
      }
      else if (stricmp(buffer,"IgnoreMessageText")==0)
      {
         if (ignoretexts>=MAXIGNORETEXTS)
         {
            configwarn("too many IgnoreMessageText keywords",lineno,configname);
            continue;
         }
         ignoretext[ignoretexts] = malloc(strlen(cp)+1);
         if (ignoretext[ignoretexts]==NULL)
         {
            configwarn("out of memory",lineno,configname);
            continue;
         }
         strcpy(ignoretext[ignoretexts],cp);
         ignoretexts++;
      }
      else if (stricmp(buffer,"ListConfig")==0)
      {
         if (lists>=MAXLISTS)
         {
            configwarn("too many ListConfig keywords",lineno,configname);
            continue;
         }
         readlist(cp,lineno,configname,argv0);
      }
      else if (stricmp(buffer,"Include")==0)
      {
         /* must be last, buffer destroyed on recursive call */
         readconfigfile(cp,argv0);
      }
      else
      {
         configwarn("unknown keyword",lineno,configname);
         continue;
      }
   }

   fclose(fp);
   return 0;
}

/*...e*/
