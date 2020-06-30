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

/* SoupGate export functions */

#include "soupgate.h"

/*...smsgidprint:0:*/
void msgidprint(FILE *fp, char *msgid,
                unsigned long fidomsgid, FIDOADDR *msgidaddr)
{
   int i;
   char *domain;

   if (msgid[0]!='\0')
   {
      if (msgid[0]!='<')
         fprintf(fp,"<");
      fprintf(fp,"%s",msgid);
      if (msgid[strlen(msgid)-1]!='>')
         fprintf(fp,">");
   }
   else
   {
      domain = "#fidonet.org";
      for (i=0; i<fidodomains; i++)
      {
         if (msgidaddr->zone>=fidodomain[i].minzone
             && msgidaddr->zone<=fidodomain[i].maxzone)
         {
            domain = fidodomain[i].domain;
            break;
         }
      }
      fprintf(fp,"<%lu@",fidomsgid);
      if (domain[0]=='#')
      {
         if (msgidaddr->point!=0)
            fprintf(fp,"p%hu.",msgidaddr->point);
         fprintf(fp,"f%hu.n%hu.z%hu.%s>",
                    msgidaddr->node,msgidaddr->net,msgidaddr->zone,
                    domain+1);
      }
      else
      {
         fprintf(fp,"%s>", domain);
      }
   }
   fprintf(fp,"\n");
}
/*...e*/

/*...sexportmessage:0:*/
void exportmessage(FILE *msgf, int flow)
{
/*...svariables:0:*/
   USERCONFIG *ptr;
   USERDATA data;
   char buffer[BUFSIZE];
   char filebuffer[FILEBUFSIZE];
   char filename[FILENAME_MAX];
   int mapno;
   char *cp,*cp2,*cp3,*cp4;
   FILE *fp, *fp2;
   unsigned long messagesize;
   unsigned long startpos,fpos;
   int i,j,buflen;
   int action;
   int status;
   int splitline;
   int filebufpos,filebufbytes;
   int b[3];
   int linebytes,nbytes;
   unsigned long fsize, wbytes;
   int pastorigin;
   int uucp,cc,bcc;
   int ignorenextempty;
   int toomanyto,toomanyxpost;
   char quotestr[8];
   int qptrans;
   unsigned long defmsgid;
   int setsubj;
   char *domain;
   int emptybody;
   int islistno, ismaillist;
   int setmsgid, setreplyid;
   int kfs, tfs;
/*...e*/

/*...sread header info:0:*/
   sscanf(msgdate,"%hu %s %hu  %hu:%hu:%hu",
                  &msgtime.day,buffer,&msgtime.year,
                  &msgtime.hour,&msgtime.min,&msgtime.sec);
   if (msgtime.year>=80)
      msgtime.year += 1900;
   else
      msgtime.year += 2000;
   msgtime.month = 0;
  
   for (i=0; i<12; i++)
   {
      if (memicmp(buffer,month[i],3)==0)
      {
         msgtime.month = i;
         break;
      }
   }

   msgfromemail[0] = '\0';
   msgreplyemail[0] = '\0';
   msgid[0] = '\0';
   replyid[0] = '\0';
   setmsgid = 0;
   setreplyid = 0;
   msgidaddr.zone = fromaddr.zone;
   msgidaddr.net = fromaddr.net;
   msgidaddr.node = fromaddr.node;
   msgidaddr.point = fromaddr.point;
   replyidaddr.zone = 0;
   replyidaddr.net = 0;
   replyidaddr.node = 0;
   replyidaddr.point = 0;
   fidomsgid = 0;
   fidoreplyid = 0;
   defmsgid = (unsigned long)time(NULL);
   strcpy(fidoareaname,NETMAILAREANAME);
   if (routing)
      uucp = 0;
   else
   {
      uucp = (stricmp(msgtoname,UUCP)==0);
      strncpy(msgtoemail,msgtoname,EMAILBUFSIZE-1);
   }
   emaillistcount = 0;
   ignorenextempty = 0;
   toomanyto = 0;
   toomanyxpost = 0;
   kfs = 0;
   tfs = 0;
   do
   {
      fpos = ftell(msgf);
      status = readasciicr(msgf,buffer,BUFSIZE);
      defmsgid ^= crc32(buffer,strlen(buffer));
      if (buffer[0]=='\1')
      {
         if (strncmp(buffer+1,"INTL ",5)==0)
         {
            sscanf(buffer+6,"%hu:%hu/%hu %hu:%hu/%hu",
                   &destaddr.zone,&destaddr.net,&destaddr.node,
                   &fromaddr.zone,&fromaddr.net,&fromaddr.node);
         }
         else if (strncmp(buffer+1,"FMPT ",5)==0)
         {
            sscanf(buffer+6,"%hu",&fromaddr.point);
         }
         else if (strncmp(buffer+1,"TOPT ",5)==0)
         {
            sscanf(buffer+6,"%hu",&destaddr.point);
         }
         else if (strncmp(buffer+1,"FLAGS ",6)==0)
         {
            cp = buffer+7;
            do
            {
               cp2 = strchr(cp, ' ');
               while (cp2!=NULL && *cp2==' ')
                  *(cp2++) = '\0';
               if (stricmp(cp, "KFS") == 0)
                  kfs = 1;
               else if (stricmp(cp, "TFS") == 0)
                  tfs = 1;
               cp = cp2;
            }
            while (cp!=NULL);
         }
         else if (strncmp(buffer+1,"MSGID:",6)==0)
         {
            cp = buffer+7;
            while (*cp==' ')
               cp++;
            cp2 = strchr(cp,' ');
            if (cp2==NULL)
               cp2 = cp;
            else
               *(cp2++) = '\0';
            if (strchr(cp,'@')!=NULL
                && strchr(cp,':')==NULL && strchr(cp,'/')==NULL)
               strncpy(msgid,cp,TEXTBUFSIZE);
            else
               sscanf(cp,"%hu:%hu/%hu.%hu",
                          &msgidaddr.zone,&msgidaddr.net,
                          &msgidaddr.node,&msgidaddr.point);
            if (sscanf(cp2,"%lX",&fidomsgid) < 1)
               fidomsgid = crc32(cp2,strlen(cp2));
            setmsgid = 1;
         }
         else if (strncmp(buffer+1,"REPLY:",6)==0)
         {
            cp = buffer+7;
            while (*cp==' ')
               cp++;
            cp2 = strchr(cp,' ');
            if (cp2==NULL)
               cp2 = cp;
            else
               *(cp2++) = '\0';
            if (strchr(cp,'@')!=NULL
                && strchr(cp,':')==NULL && strchr(cp,'/')==NULL)
               strncpy(replyid,cp,TEXTBUFSIZE);
            else
               sscanf(cp,"%hu:%hu/%hu.%hu",
                          &replyidaddr.zone,&replyidaddr.net,
                          &replyidaddr.node,&replyidaddr.point);
            if (sscanf(cp2,"%lX",&fidoreplyid) < 1)
               fidoreplyid = crc32(cp2,strlen(cp2));
            setreplyid = 1;
         }
         else if (strncmp(buffer+1,"CHRS:",5)==0)
         {
            cp = buffer+6;
            while (*cp==' ')
               cp++;
            strncpy(fidocharset,cp,TEXTBUFSIZE);
            cp = strrchr(fidocharset, ' ');
            if (cp!=NULL && isdigit(cp[1]))
               *cp = '#';
         }
      }
      else if (strncmp(buffer,"AREA:",5)==0)
         strncpy(fidoareaname,buffer+5,AREABUFSIZE-1);
      else if (!routing && memicmp(buffer,"To:",3)==0)
      {
         cp = buffer+3;
         while (*cp==' ')
            cp++;
         if (emaillistcount<exportmaxto+uucp-1)
         {
            emaillist[emaillistcount].type = EMAIL_TO;
            emaillist[emaillistcount].addr = malloc(strlen(cp)+1);
            if (emaillist[emaillistcount].addr!=NULL)
               strcpy(emaillist[emaillistcount].addr,cp);
            emaillist[emaillistcount].name = NULL;
            emaillistcount++;
         }
         else
            toomanyto = 1;
         ignorenextempty = 1;
      }
      else if (!routing && memicmp(buffer,"Copy:",5)==0)
      {
         cp = buffer+5;
         while (*cp==' ')
            cp++;
         if (emaillistcount<exportmaxto+uucp-1)
         {
            emaillist[emaillistcount].type = EMAIL_CC;
            emaillist[emaillistcount].addr = malloc(strlen(cp)+1);
            if (emaillist[emaillistcount].addr!=NULL)
               strcpy(emaillist[emaillistcount].addr,cp);
            emaillist[emaillistcount].name = NULL;
            emaillistcount++;
         }
         else
            toomanyto = 1;
         ignorenextempty = 1;
      }
      else if (!routing && memicmp(buffer,"BCopy:",6)==0)
      {
         cp = buffer+6;
         while (*cp==' ')
            cp++;
         if (emaillistcount<exportmaxto+uucp-1)
         {
            emaillist[emaillistcount].type = EMAIL_BCC;
            emaillist[emaillistcount].addr = malloc(strlen(cp)+1);
            if (emaillist[emaillistcount].addr!=NULL)
               strcpy(emaillist[emaillistcount].addr,cp);
            emaillist[emaillistcount].name = NULL;
            emaillistcount++;
         }
         else
            toomanyto = 1;
         ignorenextempty = 1;
      }
      else if (!routing && memicmp(buffer,"From:",5)==0)
      {
         cp = buffer+5;
         while (*cp==' ')
            cp++;
         strncpy(msgfromemail, cp, EMAILBUFSIZE);
         ignorenextempty = 1;
      }
      else if (!routing && memicmp(buffer,"XPost:",6)==0)
      {
         cp = buffer+6;
         while (*cp==' ')
            cp++;
         if (xpostcount<exportmaxxpost)
         {
            xpost[xpostcount] = malloc(strlen(cp)+1);
            if (xpost[xpostcount]!=NULL)
            {
               strcpy(xpost[xpostcount],cp);
               trim(xpost[xpostcount]);
               xpostcount++;
            }
         }
         else
            toomanyxpost = 1;
         ignorenextempty = 1;
      }
      else if (buffer[0]=='\0' && ignorenextempty)
         ignorenextempty = 0;
      else
         break;
   }
   while (status);
   fseek(msgf,fpos,SEEK_SET);

   if (setmsgid)
   {
      /* assume MSGID has correct address when no kludges etc. */
      if (msgidaddr.zone!=0)  /* some minimal checking */
      {
         fromaddr.zone = msgidaddr.zone;
         fromaddr.net = msgidaddr.net;
         fromaddr.node = msgidaddr.node;
         fromaddr.point = msgidaddr.point;
      }
   }
   else
   {
      /* from might be modified by kludges */
      msgidaddr.zone = fromaddr.zone;
      msgidaddr.net = fromaddr.net;
      msgidaddr.node = fromaddr.node;
      msgidaddr.point = fromaddr.point;
   }

   if (setmsgid
       && msgidaddr.net==fromaddr.net
       && msgidaddr.node==fromaddr.node)
   {
      if (fromaddr.point==0 && msgidaddr.point!=0)
      {
         /* steal point # from MSGID address (in case no FMPT line) */
         fromaddr.point = msgidaddr.point;
      }
      if (fromaddr.zone==0 && msgidaddr.zone!=0)
      {
         /* steal zone # from MSGID address (in case no INTL line) */
         fromaddr.zone = msgidaddr.zone;
      }
   }

   if (fidomsgid==0)
      fidomsgid = defmsgid;

   for (i=0; i<aliases; i++)
   {
      if (!uucp && stricmp(msgtoemail,alias[i]->alias)==0)
         strcpy(msgtoemail,alias[i]->email);
      for (j=0; j<emaillistcount; j++)
      {
         if (stricmp(emaillist[j].addr,alias[i]->alias)==0)
         {
            cp = realloc(emaillist[j].addr, strlen(alias[i]->alias)+1);
            if (cp!=NULL)
            {
               emaillist[j].addr = cp;
               strcpy(emaillist[j].addr,alias[i]->email);
            }
         }
      }
   }
/*...e*/
/*...scharacter set translation:0:*/
   strcpy(rfccharset, defcharset);
   j = strlen(fidocharset);
   if (j>0)
   {
      for (i=0; i<charsetmaps; i++)
      {
         if (charsetmap[i].dir!=GATE_IMPORT
             && memicmp(fidocharset, charsetmap[i].fidocharset, j) == 0)
         {
            strcpy(rfccharset, charsetmap[i].rfccharset);
            break;
         }
      }
   }
/*...e*/
/*...sarea translation:0:*/
   islistno = -1;
   ismaillist = -1;
   if (stricmp(fidoareaname,NETMAILAREANAME)==0
       || stricmp(fidoareaname,emailarea)==0)
   {
      strcpy(areaname,EMAILAREANAME);
      msgtype = EMAIL;
   }
   else
   {
      if (stricmp(fidoareaname,ownorgarea)==0
            || stricmp(fidoareaname,junkarea)==0)
      {
         areaname[0] = '\0';
      }
      else
      {
         strcpy(areaname,fidoareaname);
         strlwr(areaname);
      }
      for (i=0; i<areamaps; i++)
      {
         if (stricmp(fidoareaname,areamap[i]->echomail)==0)
         {
            strcpy(areaname,areamap[i]->usenet);
            if (areamap[i]->direction==GATE_IMPORT)
               return;
            break;
         }
      }
      msgtype = NEWS;
      for (i=0; i<maillists; i++)
      {
         if (stricmp(fidoareaname,maillist[i]->area)==0)
         {
            strcpy(msgtoemail,maillist[i]->postaddr);
            if (maillist[i]->fromaddr[0]!='\0')
               strcpy(msgfromemail,maillist[i]->fromaddr);
            /* kill other To:/Cc: addresses for now
               (because of problem in C_Echo with people
                putting email addresses in To: field and/or
                To: emailadress on first line)
                must be improved somehow (option perhaps?) */
            emaillistcount = 0;
            msgtype = EMAIL;
            ismaillist = i;
            break;
         }
      }
      for (i=0; i<lists; i++)
      {
         if (listconfig[i].listarea!=NULL
             && stricmp(fidoareaname, listconfig[i].listarea)==0)
         {
            islistno = i;
            msgtype = EMAIL;
            break;
         }
      }
      if (msgtype==NEWS && areaname[0]=='\0')
         return;
   }
/*...e*/
/*...saddress translation:0:*/
   /* if msgfromemail not filled in by either From: or mailing
      list forced from address, do our own translation */
   if (msgfromemail[0]=='\0')
   {
      strcpy(msgfromemail,addrmap[0]->email);
      mapno = -1;
      for (i=0; i<addrmaps; i++)
      {
         if (memcmp(&fromaddr,&addrmap[i]->fido,sizeof(FIDOADDR))==0)
         {
            mapno = i;
            break;
         }
      }
      if (mapno==-1)
      {
         for (i=0; i<addrmaps; i++)
         {
            if (addrmap[i]->name[0]!='\0'
                && stricmp(msgfromname,addrmap[i]->name)==0)
            {
               mapno = i;
               break;
            }
         }
      }
      if (mapno!=-1)
      {
         if (addrmap[mapno]->email[0]!='\0')
            strcpy(msgfromemail,addrmap[mapno]->email);
         if (addrmap[mapno]->replyto[0]!='\0')
            strcpy(msgreplyemail,addrmap[mapno]->replyto);
      }
      else if (mapfidoorg)
      {
         domain = "#fidonet.org";
         for (i=0; i<fidodomains; i++)
         {
            if (fromaddr.zone>=fidodomain[i].minzone
                && fromaddr.zone<=fidodomain[i].maxzone)
            {
               domain = fidodomain[i].domain;
               break;
            }
         }
         if (strchr(domain,'@')==NULL)
         {
            strcpy(msgfromemail,msgfromname);
            for (cp=msgfromemail; *cp!='\0'; cp++)
            {
               if (*cp==' ')
                  *cp = '.';
            }
            strcat(msgfromemail,"@");
            if (domain[0]=='#')
            {
               if (fromaddr.point!=0)
                  sprintf(msgfromemail+strlen(msgfromemail),"p%u.",fromaddr.point);
               sprintf(msgfromemail+strlen(msgfromemail),"f%u.n%u.z%u.%s",
                                       fromaddr.node,fromaddr.net,fromaddr.zone,
                                       domain+1);
            }
            else
               strcat(msgfromemail,domain);
         }
         else
            strcpy(msgfromemail,domain);
      }
   }

   strcpy(msgreplyname, msgfromname);
   if (msgreplyemail[0]=='\0')
      strcpy(msgreplyemail, msgfromemail);
/*...e*/
/*...scheck body:0:*/
   qptrans = 0;
   emptybody = 1;
   fpos = ftell(msgf);

   do
   {
      status = readasciicr(msgf,buffer,souplinewidth);
      if (status && buffer[0]!='\0' && buffer[0]!='\1')
      {
         emptybody = 0;
         buflen = strlen(buffer);
         for (i=0; i<buflen; i++)
         {
            if (i==0 && buffer[i]==1)
               continue;
            if (buffer[i]<32 || buffer[i]>126)
            {
               qptrans = 1;
               break;
            }
         }
      }
   }
   while (status && !qptrans);

   fseek(msgf,fpos,SEEK_SET);

   if (emptybody)
   {
      if (isfattach && flow && flowfiles>0)
      {
         for (i=0; i<flowfiles; i++)
         {
            if (routing || flowfilelist[i].type!=FLOW_PKT)
            {
               emptybody = 0;
               break;
            }
         }
      }
   }

   if (emptybody)
      return;
/*...e*/
/*...scheck header info:0:*/
   if (toomanyto)
   {
      logprintf("! To/Copy-limit exceeded in msg from %hu:%hu/%hu.%hu (%s)\n",
                fromaddr.zone,fromaddr.net,fromaddr.node,fromaddr.point,
                msgfromname);
   }

   if (toomanyxpost)
   {
      logprintf("! XPost-limit exceeded in msg from %hu:%hu/%hu.%hu (%s)\n",
                fromaddr.zone,fromaddr.net,fromaddr.node,fromaddr.point,
                msgfromname);
   }

   if (uucp && emaillistcount==0)
   {
      logprintf("! No To-address specified in msg from %hu:%hu/%hu.%hu (%s)\n",
                fromaddr.zone,fromaddr.net,fromaddr.node,fromaddr.point,
                msgfromname);
   }

/*...e*/
/*...sappend message:0:*/
   fp = _fsopen(replyfile[msgtype],"r+b",SH_DENYWR);
   if (fp==NULL)
      fp = _fsopen(replyfile[msgtype],"w+b",SH_DENYWR);
   if (fp==NULL)
      return;

   fseek(fp,0L,SEEK_END);
   startpos = ftell(fp);
   messagesize = 0;
   fwrite(&messagesize,1,4,fp);

/*...e*/
/*...swrite header info:0:*/
   if (islistno!=-1)
   {
      fprintf(fp,"Bcc: ");
      j = 0;
      for (i=0; i<=26; i++)
      {
         ptr = listconfig[islistno].users[i];
         while (ptr!=NULL)
         {
            if (getuserdata(islistno, ptr, &data)
                && data.status != STATUS_BLACKLIST)
            {
               if (j>0)
               {
                  if (j%2==0)
                     fprintf(fp, ",\n   ");
                  else
                     fprintf(fp, ", ");
               }
               fprintf(fp,"<%s>", data.email);
               j++;
            }
            ptr = ptr->next;
         }
      }
      fprintf(fp,"\n");
      getuserdata(islistno, NULL, NULL);   /* close file */
   }
   else if (msgtype==EMAIL)
   {
      fprintf(fp,"To: ");
      if (!uucp)
      {
         fprintf(fp,"%s",msgtoemail);
         if (routing && msgtoname[0]!='\0')
            fprintf(fp," (%s)",msgtoname);
      }
      else if (emaillistcount>0)
      {
         if (emaillist[0].addr!=NULL)
            strcpy(msgtoemail, emaillist[0].addr);
         if (emaillist[0].name!=NULL)
            strcpy(msgtoname, emaillist[0].name);
      }
      j = 0;
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
         if (j>0 || !uucp)
         {
            if (j%3==0)
               fprintf(fp,",\n    ");
            else
               fprintf(fp,", ");
         }
         if (emaillist[i].addr!=NULL)
         {
            fprintf(fp,"%s",emaillist[i].addr);
            free(emaillist[i].addr);
         }
         if (emaillist[i].name!=NULL)
         {
            fprintf(fp," (%s)",emaillist[i].name);
            free(emaillist[i].name);
         }
         j++;
      }
      fprintf(fp,"\n");
      if (cc)
      {
         fprintf(fp,"Cc: ");
         j = 0;
         for (i=0; i<emaillistcount; i++)
         {
            if (emaillist[i].type!=EMAIL_CC)
               continue;
            if (j>0)
            {
               if (j%3==0)
                  fprintf(fp,",\n    ");
               else
                  fprintf(fp,", ");
            }
            if (emaillist[i].addr!=NULL)
            {
               fprintf(fp,"%s",emaillist[i].addr);
               free(emaillist[i].addr);
            }
            if (emaillist[i].name!=NULL)
            {
               fprintf(fp," (%s)",emaillist[i].name);
               free(emaillist[i].name);
            }
            j++;
         }
         fprintf(fp,"\n");
      }
      if (bcc)
      {
         fprintf(fp,"Bcc: ");
         j = 0;
         for (i=0; i<emaillistcount; i++)
         {
            if (emaillist[i].type!=EMAIL_BCC)
               continue;
            if (j>0)
            {
               if (j%3==0)
                  fprintf(fp,",\n    ");
               else
                  fprintf(fp,", ");
            }
            if (emaillist[i].addr!=NULL)
            {
               fprintf(fp,"%s",emaillist[i].addr);
               free(emaillist[i].addr);
            }
            if (emaillist[i].name!=NULL)
            {
               fprintf(fp," (%s)",emaillist[i].name);
               free(emaillist[i].name);
            }
            j++;
         }
         fprintf(fp,"\n");
      }
   }
   else
   {
      fprintf(fp,"Newsgroups: %s",areaname);
      for (i=0; i<xpostcount; i++)
      {
         fprintf(fp,",%s",xpost[i]);
         free(xpost[i]);
      }
      fprintf(fp,"\n");
   }
   fprintf(fp,"From: %s%s (%s)\n",
              msgtype==NEWS?nospamtext:"",
              msgfromemail,msgfromname);
   if (islistno==-1)
   {
      if (stricmp(msgfromemail, msgreplyemail)!=0)
         fprintf(fp,"Reply-to: %s (%s)\n",
                    msgreplyemail,msgreplyname);
   }
   else
   {
      fprintf(fp,"Reply-to: %s\n", listconfig[islistno].listaddress);
      fprintf(fp,"Sender: %s\n", listconfig[islistno].listaddress);
      if (listconfig[islistno].keeperaddress)
      {
         fprintf(fp,"Errors-To: %s\n", listconfig[islistno].keeperaddress);
         fprintf(fp,"Return-Path: %s\n", listconfig[islistno].keeperaddress);
      }
   }
   setsubj = 0;
   if (isfattach && routing && routemailsubj[0]!='\0')
   {
      cp = msgsubj;
      do
      {
         while (*cp==' ')
            cp++;
         if (*cp=='\0')
            break;
         cp2 = strchr(cp,' ');
         if (cp2==NULL)
            cp2 = cp + strlen(cp);
         if (ismail(cp,cp2-cp))
            setsubj = 1;
         cp = cp2;
      }
      while (!setsubj);
   }
   if (setsubj)
      fprintf(fp,"Subject: %s\n",routemailsubj);
   else
      fprintf(fp,"Subject: %s\n",msgsubj);
   fprintf(fp,"Date: %s, %02d %-3s %04d %02d:%02d:%02d",
               weekday[dow(msgtime.year,msgtime.month+1,msgtime.day)],
               msgtime.day,month[msgtime.month],msgtime.year,
               msgtime.hour,msgtime.min,msgtime.sec);
   if (timezonestr[0]!='\0')
      fprintf(fp," %s",timezonestr);
   fprintf(fp,"\n");
   fprintf(fp,"Message-ID: ");
   msgidprint(fp,msgid,fidomsgid,&msgidaddr);
   if (fidoreplyid!=0 || replyid[0]!='\0')
   {
      if (msgtype==EMAIL)
         fprintf(fp,"In-Reply-To: ");
      else
         fprintf(fp,"References: ");
      msgidprint(fp,replyid,fidoreplyid,&replyidaddr);
   }
   if (organization[0]!='\0')
      fprintf(fp,"Organization: %s\n",organization);
   if ((msgtype==NEWS || islistno!=-1 || ismaillist!=-1) && msgtoname[0]!='\0')
      fprintf(fp,"X-Comment-To: %s\n",msgtoname);
   if (msgtype==EMAIL && routing && routemailpwd[0]!='\0')
      fprintf(fp,"X-MailPassword: %s\n",routemailpwd);
   fprintf(fp,"X-Mail%s: SoupGate-%s v%d.%02d\n",
              islistno==-1 ? "Converter" : "Server",
              osstr,MAJORVERSION,MINORVERSION);

   fprintf(fp,"Mime-version: 1.0\n");
   if (isfattach)
   {
      fprintf(fp,"Content-Type: multipart/mixed; boundary=\"%s\"\n",
                 MIMEBOUNDARY);
      fprintf(fp,"\n--%s\n",MIMEBOUNDARY);
   }
   fprintf(fp,"Content-Type: text/plain; charset=%s\n", rfccharset);
   fprintf(fp,"Content-Transfer-Encoding: ");
   if (!qptrans)
      fprintf(fp,"7bit\n");
   else if (useqp)
      fprintf(fp,"quoted-printable\n");
   else
      fprintf(fp,"8bit\n");

   fprintf(fp,"\n");
   
   if (ismaillist!=-1 && maillist[ismaillist]->authorinfo!=INFO_NONE)
   {
      if (maillist[ismaillist]->authorinfo==INFO_NAME
           && stricmp(msgfromname, msgfromemail)!=0)
         fprintf(fp, " * Author: %s\n\n", msgfromname);
      else if (maillist[ismaillist]->authorinfo==INFO_BOTH
           && stricmp(msgfromname, msgfromemail)!=0)
         fprintf(fp, " * Author: %s <%s>\n\n", msgfromname, msgfromemail);
      else
         fprintf(fp, " * Author: %s\n\n", msgfromemail);
   }
/*...e*/
/*...swrite body:0:*/
   splitline = 0;
   pastorigin = 0;
   quotestr[0] = '\0';
   do
   {
      fpos = ftell(msgf);
      status = readasciicr(msgf,buffer,souplinewidth);
      if (pastorigin)
         continue;
      if (!splitline)
      {
         if (buffer[0]=='\1')
         {
            while (buffer[0]=='\1' && status==2)
               status = readasciicr(msgf,buffer,souplinewidth);
            continue;
         }
         if (memcmp(buffer,"---",3)==0
             && (buffer[3]==' ' || buffer[3]=='\0'))
         {
            if (!status)
            {
               pastorigin = 1;
               continue;
            }
            readasciicr(msgf,buffer,souplinewidth);
            if (memcmp(buffer," * Origin:",10)==0
                || memicmp(buffer, "\x01Via ",5)==0)
            {
               pastorigin = 1;
               continue;
            }
            fseek(msgf, fpos, SEEK_SET);
            status = readasciicr(msgf,buffer,souplinewidth);
         }
         if (memcmp(buffer," * Origin:",10)==0)
         {
            pastorigin = 1;
            continue;
         }
      }
      buflen = strlen(buffer);
      if (quotestr[0]!='\0')
      {
         i = strlen(quotestr);
         memmove(buffer+i,buffer,buflen+1);
         memcpy(buffer,quotestr,i);
         buflen += i;
      }
      if (status==2)
      {
         if (memchr(buffer,'>',6)!=NULL
                  && memchr(buffer,'<',7)==NULL)
         {
            memcpy(quotestr,buffer,6);
            cp = memchr(quotestr,'>',6);
            if (cp==NULL)
               quotestr[0] = '\0';
            else
            {
               *(cp+1) = ' ';
               *(cp+2) = '\0';
            }
         }
         splitline = 0;
         /* first try splitting on a space */
         for (i=buflen-1; i>32; i--)
         {
            if (i<souplinewidth && isspace(buffer[i]))
            {
               fseek(msgf,fpos+i+1,SEEK_SET);
               buflen = i+1;
               buffer[buflen] = '\0';
               splitline = 1;
               break;
            }
         }
         if (!splitline)
         {
            /* then try something non-alphanumeric */
            for (i=buflen-1; i>32; i--)
            {
               if (i<souplinewidth && !isalnum(buffer[i]))
               {
                  fseek(msgf,fpos+i+1,SEEK_SET);
                  buflen = i+1;
                  buffer[buflen] = '\0';
                  splitline = 1;
                  break;
               }
            }
         }
         /* if we're not split by now, we'll just keep the split
            the file read caused */
         splitline = 1;
      }
      else
      {
         quotestr[0] = '\0';
         splitline = 0;
      }
      if (qptrans && useqp)
      {
         for (i=0; i<buflen; i++)
         {
            if (buffer[i]=='=' || buffer[i]<32 || buffer[i]>126)
            {
               memmove(buffer+i+3,buffer+i+1,buflen-i);
               buffer[i+1] = tohexdigit(buffer[i]>>4);
               buffer[i+2] = tohexdigit(buffer[i]&0x0F);
               buffer[i] = '=';
               i+=2;
               buflen+=2;
            }
         }
      }
      fprintf(fp,"%s\n",buffer);
   }
   while (status);
/*...e*/
/*...swrite attached files:0:*/
   if (isfattach)
   {
      cp = msgsubj;
      do
      {
         while (*cp==' ')
            cp++;
         if (*cp=='\0')
            break;
         cp2 = strchr(cp,' ');
         if (cp2!=NULL)
            *cp2 = '\0';
         strncpy(buffer,cp,BUFSIZE-1);
         if (flow && flowfiles>0)
         {
            for (i=0; i<flowfiles; i++)
            {
               cp3 = flowfilelist[i].file;
               if (*cp3=='^' || *cp3=='#')
                  cp3++;
               if (fncmp(cp,cp3)==0)
               {
                  if (!routing && flowfilelist[i].type==FLOW_PKT)
                     goto skipfile;
                  strncpy(buffer,flowfilelist[i].file,BUFSIZE-1);
                  flowfilelist[i].type = FLOW_USED;
                  break;
               }
            }
         }
         cp3 = buffer;
         switch(*cp3)
         {
         case '^':
            cp3++;
            action = FILE_KILL;
            break;
         case '#':
            cp3++;
            action = FILE_TRUNCATE;
            break;
         default:
            if (kfs)
               action = FILE_KILL;
            else if (tfs)
               action = FILE_TRUNCATE;
            else
               action = FILE_KEEP;
            break;
         }
         strcpy(filename, cp3);
         fp2 = _fsopen(filename,"rb",SH_DENYNO);
         if (fp2!=NULL)
         {
            fseek(fp2,0L,SEEK_END);
            fsize = ftell(fp2);
            rewind(fp2);
#ifndef __unix__
            cp4 = strrchr(filename,'\\');
            if (cp4==NULL)
#endif
               cp4 = strrchr(filename,'/');
            if (cp4==NULL)
               cp4 = filename;
            else
               cp4++;
            logprintf("* %s-encoding %s\n",encodename[encodeformat],cp4);
            strlwr(cp);
            fprintf(fp,"\n--%s\n",MIMEBOUNDARY);
            fprintf(fp,"Content-Type: application/octet-stream; name=\"%s\"\n",cp4);
            fprintf(fp,"Content-Transfer-Encoding: ");
            if (encodeformat==ENCODE_MIME)
            {
               fprintf(fp,"base64\n");
               fprintf(fp,"\n");
            }
            else
            {
               fprintf(fp,"7bit\n");
               fprintf(fp,"\n");
               fprintf(fp,"begin 644 %s\n",cp4);
            }
            filebufpos = 0;
            filebufbytes = 0;
            wbytes = 0;
            while (fsize>0)
            {
               if (fsize<ENCBYTESPERLINE[encodeformat])
                  linebytes = (int)fsize;
               else
                  linebytes = ENCBYTESPERLINE[encodeformat];
               buflen = 0;
               if (encodeformat==ENCODE_UU)
                  buffer[buflen++] = (linebytes&077) + 32;
               fsize -= linebytes;
               while (linebytes>0)
               {
                  nbytes = 0;
                  for (i=0; i<3; i++)
                  {
                     if (filebufbytes==0)
                     {
                        filebufbytes = fread(filebuffer,1,FILEBUFSIZE,fp2);
                        filebufpos = 0;
                     }
                     if (filebufbytes==0)
                        b[i] = 0;
                     else
                     {
                        b[i] = filebuffer[filebufpos++];
                        filebufbytes--;
                        nbytes++;
                     }
                  }
                  if (encodeformat==ENCODE_MIME)
                  {
                     buffer[buflen++] = MIMEALPHABET[b[0]>>2];
                     buffer[buflen++] = MIMEALPHABET[((b[0]<<4)&060)|((b[1]>>4)&017)];
                     if (nbytes>=2)
                        buffer[buflen++] = MIMEALPHABET[((b[1]<<2)&074)|((b[2]>>6)&003)];
                     if (nbytes>=3)
                        buffer[buflen++] = MIMEALPHABET[b[2]&077];
                  }
                  else
                  {
                     buffer[buflen++] = uuenc(b[0]>>2);
                     buffer[buflen++] = uuenc(((b[0]<<4)&060)|((b[1]>>4)&017));
                     if (nbytes>=2)
                        buffer[buflen++] = uuenc(((b[1]<<2)&074)|((b[2]>>6)&003));
                     if (nbytes>=3)
                        buffer[buflen++] = uuenc(b[2]&077);
                  }
                  wbytes += (nbytes>=3) ? 4 : ((nbytes>=2) ? 3 : 2);
                  linebytes -= 3;
               }
               if (encodeformat==ENCODE_MIME && fsize==0)
               {
                  while (wbytes%4!=0)
                  {
                     buffer[buflen++] = '=';
                     wbytes++;
                  }
               }
               buffer[buflen] = '\0';
               fprintf(fp,"%s\n",buffer);
            }
            if (encodeformat==ENCODE_UU)
            {
               fprintf(fp,"`\n");
               fprintf(fp,"end\n");
            }
            fprintf(fp,"\n");
            fclose(fp2);
            if (!testmode)
            {
               switch(action)
               {
               case FILE_KILL:
                  remove(filename);
                  break;
               case FILE_TRUNCATE:
                  fp2 = _fsopen(filename,"wb",SH_DENYWR);
                  if (fp2!=NULL)
                     fclose(fp2);
                  break;
               }
            }
         }
skipfile:
         if (cp2!=NULL)
            *(cp2++) = ' ';
         cp = cp2;
      }
      while (cp!=NULL);
   }
/*...e*/
/*...sclose message:0:*/
   if (isfattach)
      fprintf(fp,"\n--%s--\n",MIMEBOUNDARY);

   fflush(fp);

   messagesize = ftell(fp) - startpos - 4;
   fseek(fp,startpos,SEEK_SET);
   fwrite((unsigned char *)&messagesize+3,1,1,fp);
   fwrite((unsigned char *)&messagesize+2,1,1,fp);
   fwrite((unsigned char *)&messagesize+1,1,1,fp);
   fwrite((unsigned char *)&messagesize+0,1,1,fp);

   fclose(fp);

   newmsg[msgtype]++;

   messagecount++;
   if (msgtype==EMAIL)
      emailcount++;
   else
      newscount++;

   if (separatemsg)
      genreplyfile(msgtype);
/*...e*/
/*...sdisplay info:0:*/
   if (islistno!=-1)
      listmsgcount++;

   if (verbose)
   {
      fprintf(stderr,"\r");
      logprintf("- from %hu:%hu/%hu.%hu (%s) to ",
                fromaddr.zone, fromaddr.net, fromaddr.node, fromaddr.point,
                msgfromname);
      if (islistno!=-1)
      {
         logprintf("%s", listconfig[islistno].listname);
      }
      else if (msgtype==EMAIL)
      {
         logprintf("%s", msgtoemail);
         if (emaillistcount>uucp)
            logprintf(" et al.");
      }
      else
      {
         logprintf("%s newsgroup", areaname);
         if (xpostcount>0)
            logprintf(" et al.");
      }
      logprintf("\n");
   }
/*...e*/
}
/*...e*/

/*...sexportpkt:0:*/
int exportpkt(FILE *fpkt, int flow)
{
   struct PACKETHEADER pktheader;
   struct PACKETMSGHEADER pktmsgheader;
   int type2plus;
   int i;

   if (fread(&pktheader,1,sizeof pktheader,fpkt) < sizeof pktheader
       || pktheader.version<2)
      return 0;

   if (pktheader.capword!=0x0001 || pktheader.cwvalidate!=0x0100)
   {
      type2plus = 0;
      /* not Type 2+ => origzone/origpoint fields undefined */
      pktheader.origzone = addrmap[0]->fido.zone;
      pktheader.origpoint = 0;
      if (routing!=0)
      {
         pktheader.destzone = addrmap[0]->fido.zone;
         pktheader.destpoint = 0;
      }
      else
      {
         pktheader.destzone = gateaddr.zone;
         pktheader.destpoint = gateaddr.point;
      }
   }
   else
      type2plus = 1;

   if (routing==-1)
   {
      if (pktheader.destzone==gateaddr.zone
       && pktheader.destnet==gateaddr.net
       && pktheader.destnode==gateaddr.node
       && pktheader.destpoint==gateaddr.point)
      {
         routing = 0;
      }
      else
      {
         for (i=0; i<routes; i++)
         {
            if (pktheader.destzone==route[i]->fido.zone
             && pktheader.destnet==route[i]->fido.net
             && pktheader.destnode==route[i]->fido.node
             && pktheader.destpoint==route[i]->fido.point)
            {
               routing = 1;
               break;
            }
         }
      }
      if (routing==-1)
         return 0;
   }

   logprintf("Exporting Type 2%s packet from %u:%u/%u",
             type2plus?"+":"",
             pktheader.origzone,pktheader.orignet,pktheader.orignode);
   if (pktheader.origpoint!=0)
      logprintf(".%u",pktheader.origpoint);
   logprintf(" to %u:%u/%u",
             pktheader.destzone,pktheader.destnet,pktheader.destnode);
   if (pktheader.destpoint!=0)
      logprintf(".%u",pktheader.destpoint);
   logprintf("\n");

   while (fread(&pktmsgheader,1,sizeof pktmsgheader,fpkt)==sizeof pktmsgheader)
   {
      isfattach = (pktmsgheader.attrib&0x0010)!=0;    /* file attach */
      fromaddr.node = pktmsgheader.orignode;
      fromaddr.net = pktmsgheader.orignet;
      fromaddr.zone = pktheader.origzone;
      fromaddr.point = pktheader.origpoint;
      readasciiz(fpkt,msgdate,20);
      readasciiz(fpkt,msgtoname,TEXTBUFSIZE-1);
      readasciiz(fpkt,msgfromname,TEXTBUFSIZE-1);
      readasciiz(fpkt,msgsubj,TEXTBUFSIZE-1);
      exportmessage(fpkt,flow);
   }

   return 1;
}
/*...e*/
/*...screatefattach:0:*/
FILE *createfattach(FIDOADDR *destfido, EMAILADDR destemail,
                    char *pktname, char *filename)
{
   struct PACKETHEADER pktheader;
   struct PACKETMSGHEADER pktmsgheader;
   time_t t;
   struct tm *tm;
   FILE *fpkt;
   char *cp;

   fpkt = _fsopen(pktname, "w+b", SH_DENYWR);
   if (fpkt==NULL)
      return NULL;

   time(&t);
   tm = localtime(&t);

   pktheader.orignode = addrmap[0]->fido.node;
   pktheader.destnode = destfido->node;
   pktheader.year = 1900+tm->tm_year;
   pktheader.month = tm->tm_mon;
   pktheader.day = tm->tm_mday;
   pktheader.hour = tm->tm_hour;
   pktheader.minute = tm->tm_min;
   pktheader.second = tm->tm_sec;
   pktheader.baudrate = 0;
   pktheader.version = 2;
   pktheader.orignet = addrmap[0]->fido.net;
   pktheader.destnet = destfido->net;
   pktheader.pcodelow = 0;
   pktheader.prevmajor = 1;
   strncpy(pktheader.password,packetpwd,8);
   pktheader.qmorigzone = addrmap[0]->fido.zone;
   pktheader.qmdestzone = destfido->zone;
   pktheader.auxnet = 0;
   pktheader.cwvalidate = 0x0100;
   pktheader.pcodehigh = 1;
   pktheader.prevminor = 0;
   pktheader.capword = 0x0001;
   pktheader.origzone = addrmap[0]->fido.zone;
   pktheader.destzone = destfido->zone;
   pktheader.origpoint = addrmap[0]->fido.point;
   pktheader.destpoint = destfido->point;
   pktheader.extrainfo = 0;
   fwrite(&pktheader,sizeof pktheader,1,fpkt);

   pktmsgheader.version = 2;
   pktmsgheader.orignode = addrmap[0]->fido.node;
   pktmsgheader.destnode = destfido->node;
   pktmsgheader.orignet = addrmap[0]->fido.net;
   pktmsgheader.destnet = destfido->net;
   pktmsgheader.attrib = 0x0001;    /* private */
   pktmsgheader.attrib |= 0x0010;   /* file attach */
   pktmsgheader.cost = 0;
   fwrite(&pktmsgheader,sizeof pktmsgheader,1,fpkt);

   fprintf(fpkt,"%02d %-3s %02d  %02d:%02d:%02d%c",
               tm->tm_mday,month[tm->tm_mon],tm->tm_year%100,
               tm->tm_hour,tm->tm_min,tm->tm_sec,0);

   fprintf(fpkt,"%.*s%c",35,destemail,0);
   fprintf(fpkt,"%.*s%c",35,addrmap[0]->email,0);
   fprintf(fpkt,"%.71s%c",filename,0);
   fprintf(fpkt,"\1INTL %hu:%hu/%hu %hu:%hu/%hu\r",
              destfido->zone,destfido->net,destfido->node,
              addrmap[0]->fido.zone,addrmap[0]->fido.net,addrmap[0]->fido.node);
   if (addrmap[0]->fido.point!=0)
      fprintf(fpkt,"\1FMPT %hu\r",addrmap[0]->fido.point);
   if (destfido->point!=0)
      fprintf(fpkt,"\1TOPT %hu\r",destfido->point);

#ifndef __unix__
   cp = strrchr(filename,'\\');
   if (cp==NULL)
#endif
      cp = strrchr(filename,'/');
   if (cp==NULL)
   {
      cp = filename;
      if (*cp=='^' || *cp=='#')
         cp++;
   }
   else
      cp++;

   fprintf(fpkt,"[SoupGate-%s v%d.%02d attached file %s]\r%c%c",
           osstr,MAJORVERSION,MINORVERSION,
           cp,
           0,0);

   return fpkt;
}
/*...e*/

/*...sbinkexport:0:*/
void binkexport(FIDOADDR *destfido, EMAILADDR destemail, int isroute)
{
   char fname[FILENAME_MAX], fname2[FILENAME_MAX];
   int mailtype,i;
   char *ext,*cp;
   FILE *fp,*fp2;

   strcpy(fname,outdir);
   if (basezone!=0 && destfido->zone!=basezone)
      sprintf(fname+strlen(fname)-1,".%03x" DEFDIRSEP,destfido->zone);
   sprintf(fname+strlen(fname),"%04x%04x",destfido->net,destfido->node);
   if (destfido->point!=0)
      sprintf(fname+strlen(fname),".pnt" DEFDIRSEP "%08x",destfido->point);
   ext = fname + strlen(fname);

   for (mailtype=0;
        mailtype<(exportallmail?BINK_ALLTYPES:BINK_NORMTYPES);
        mailtype++)
   {
      strcpy(ext,binkmailext[mailtype]);
      fp = _fsopen(fname,"rb",SH_DENYNO);
      strcpy(ext,binkflowext[mailtype]);
      fp2 = _fsopen(fname,"r",SH_DENYNO);
      flowfiles = 0;
      if (fp2!=NULL)
      {
         while (flowfiles < MAXFLOWFILES)
         {
            fgets(fname2,FILENAME_MAX-1,fp2);
            if (feof(fp2))
               break;
            cp = fname2 + strlen(fname2) - 1;
            /* remove both CR and LF in case DOS format on unix */
            while (cp>=fname2 && (*cp=='\n' || *cp=='\r'))
               *(cp--) = '\0';
            cp = strrchr(fname2, '.');
            /* packets inside flow files must be treated as mail packets,
               because some tossers write them out that way */
            if (cp!=NULL && stricmp(cp,packetext)==0)
               flowfilelist[flowfiles].type = FLOW_PKT;
            else
               flowfilelist[flowfiles].type = FLOW_FILE;
            flowfilelist[flowfiles].file = malloc(strlen(fname2)+1);
            if (flowfilelist[flowfiles].file==NULL)
               break;
            strcpy(flowfilelist[flowfiles].file,fname2);
            flowfiles++;
         }
         fclose(fp2);
         if (!testmode)
            remove(fname);
      }
      if (fp!=NULL)
      {
         exportpkt(fp,1);
         fclose(fp);
         if (!testmode)
         {
            strcpy(ext,binkmailext[mailtype]);
            remove(fname);
         }
      }
      if (flowfiles>0)
      {
         for (i=0; i<flowfiles; i++)
         {
            if (flowfilelist[i].type != FLOW_PKT)
               continue;
            cp = flowfilelist[i].file;
            if (*cp=='^' || *cp=='#')
               cp++;
            fp = _fsopen(cp,"rb",SH_DENYNO);
            if (fp!=NULL)
            {
               exportpkt(fp,1);
               fclose(fp);
               if (!testmode)
                  remove(cp);
            }
            flowfilelist[i].type = FLOW_USED;
         }
         for (i=0; i<flowfiles; i++)
         {
            if (flowfilelist[i].type != FLOW_FILE)
               continue;
            cp = flowfilelist[i].file;
            if (*cp=='^' || *cp=='#')
               cp++;
            fp = _fsopen(cp,"rb",SH_DENYNO);
            if (fp!=NULL)
            {
               fclose(fp);
               genfname(tempdir,packetext,fname2);
               fp2 = createfattach(destfido,destemail,fname2,
                                   flowfilelist[i].file);
               if (fp2!=NULL)
               {
                  rewind(fp2);
                  exportpkt(fp2,1);
                  fclose(fp2);
                  remove(fname2);
               }
            }
            flowfilelist[i].type = FLOW_USED;
         }
         for (i=flowfiles-1; i>=0; i--)
            free(flowfilelist[i].file);
      }
   }
}
/*...e*/
/*...smsgexport:0:*/
void msgexport(void)
{
   char fname[FILENAME_MAX];
   char buffer[BUFSIZE];
   DIR *dp;
   struct dirent *dirdata;
   struct stat statbuf;
   FILE *fp, *fp2;
   unsigned short destzone,destnet,destnode,destpoint;
   FIDOADDR *addr;
   time_t t;
   struct MESSAGEHEADER msgheader;
   int i,exported;
   unsigned long fpos;
   char *cp, *cp2, *cp3;
   time_t lasttime = 0;
   
   if (quickscan)
   {
      strcpy(fname,netdir);
      strcat(fname,SCANDATEFILE);
      fp = _fsopen(fname,"rb",SH_DENYNO);
      if (fp!=NULL)
      {
         fread(&lasttime, sizeof lasttime, 1, fp);
         /* first time_t should be 0, indicates new format
            for backwards compatibility with SoupGate < 1.05 */
         if (lasttime==0)
            fread(&lasttime, sizeof lasttime, 1, fp);    /* new */
         else
            lasttime = 0;                                /* old */
         fclose(fp);
      }

      fp = _fsopen(fname,"wb",SH_DENYWR);
      if (fp!=NULL)
      {
         t = 0;   /* indicates new format */
         fwrite(&t,sizeof t,1,fp);
         time(&t);
         fwrite(&t,sizeof t,1,fp);
         fclose(fp);
      }
   }

   strcpy(fname,netdir);
   strcat(fname,"*");
   strcat(fname,messageext);
   dp = opendir(fname);
   if (dp!=NULL)
   {
      while ((dirdata=readdir(dp))!=NULL)
      {
         strcpy(fname,netdir);
         strcat(fname,dirdata->d_name);
         if (stat(fname, &statbuf)!=0
             || statbuf.st_mtime<lasttime)
            continue;
         fp = _fsopen(fname,"r+b",SH_DENYWR);
         if (fp==NULL)
            continue;
         exported = 0;
         if (fread(&msgheader,1,sizeof msgheader,fp) == sizeof msgheader
             && (msgheader.attrib&0x0008)==0                     /* not sent */
             && (exportallmail || (msgheader.attrib&0x0200)==0)) /* not hold */
         {
            fpos = ftell(fp);
            destzone = 0xFFFF;
            destnet = msgheader.destnet;
            destnode = msgheader.destnode;
            destpoint = 0xFFFF;
            fromaddr.zone = addrmap[0]->fido.zone;
            fromaddr.net = msgheader.orignet;
            fromaddr.node = msgheader.orignode;
            fromaddr.point = 0;
            for (i=-1; i<routes; i++)
            {
               if (i==-1)
               {
                  addr = &gateaddr;
                  routing = 0;
               }
               else
               {
                  addr = &route[i]->fido;
                  strcpy(routemailpwd,route[i]->pwd);
                  strcpy(routemailsubj,route[i]->mailsubj);
                  strcpy(msgtoemail,route[i]->email);
                  routing = 1;
               }
               if (addr->node==destnode
                   && addr->net==destnet)
               {

                  if (destzone==0xFFFF || destpoint==0xFFFF)
                  {
                     destzone = addrmap[0]->fido.zone;
                     destpoint = 0;
                     for (;;)
                     {
                        readasciicr(fp,buffer,BUFSIZE);
                        if (feof(fp) || buffer[0]!='\1')
                           break;
                        if (memicmp(buffer+1,"TOPT ",5)==0)
                           sscanf(buffer+6,"%hu",&destpoint);
                        else if (memicmp(buffer+1,"FMPT ",5)==0)
                           sscanf(buffer+6,"%hu",&fromaddr.point);
                        else if (memicmp(buffer+1,"INTL ",5)==0)
                           sscanf(buffer+6,"%hu:%hu/%hu %hu:%hu/%hu",
                                  &destzone,&destnet,&destnode,
                                  &fromaddr.zone,&fromaddr.net,&fromaddr.node);
                        /* MSGID not checked for From address here,
                           because from address only used for displaying
                           here, not for routing/translating; the
                           address information will be reread later
                           if this message would be exported */
                     }
                  }
                  if (addr->zone==destzone
                      && addr->point==destpoint)
                  {
                     isfattach = (msgheader.attrib&0x0010)!=0;  /* file attach */
                     strncpy(msgdate,msgheader.date,20);
                     strncpy(msgtoname,msgheader.to,TEXTBUFSIZE-1);
                     strncpy(msgfromname,msgheader.from,TEXTBUFSIZE-1);
                     strncpy(msgsubj,msgheader.subj,TEXTBUFSIZE-1);
                     if (isfattach)
                     {
                        cp = msgsubj;
                        flowfiles = 0;
                        do
                        {
                           while (*cp==' ')
                              cp++;
                           if (*cp=='\0')
                              break;
                           cp2 = strchr(cp,' ');
                           if (cp2!=NULL)
                              *cp2 = '\0';
                           cp3 = strrchr(cp, '.');
                           /* packets attached must be treated as mail packets,
                              because some tossers write them out that way */
                           if (cp3!=NULL && stricmp(cp3,packetext)==0)
                              flowfilelist[flowfiles].type = FLOW_PKT;
                           else
                              flowfilelist[flowfiles].type = FLOW_FILE;
                           flowfilelist[flowfiles].file = malloc(strlen(cp)+1);
                           if (flowfilelist[flowfiles].file==NULL)
                              break;
                           strcpy(flowfilelist[flowfiles].file,cp);
                           flowfiles++;
                           if (cp2!=NULL)
                              *(cp2++) = ' ';
                           cp = cp2;
                        }
                        while (cp!=NULL && flowfiles < MAXFLOWFILES);
                     }
                     logprintf("Exporting netmail message from %u:%u/%u",
                               fromaddr.zone,fromaddr.net,fromaddr.node);
                     if (fromaddr.point!=0)
                        logprintf(".%u",fromaddr.point);
                     logprintf(" to %u:%u/%u",
                               destzone,destnet,destnode);
                     if (destpoint!=0)
                        logprintf(".%u",destpoint);
                     logprintf("\n");
                     fseek(fp,fpos,SEEK_SET);
                     exportmessage(fp,1);
                     if (flowfiles>0)
                     {
                        for (i=0; i<flowfiles; i++)
                        {
                           if (flowfilelist[i].type != FLOW_PKT)
                              continue;
                           cp = flowfilelist[i].file;
                           if (*cp=='^' || *cp=='#')
                              cp++;
                           fp2 = _fsopen(cp,"rb",SH_DENYNO);
                           if (fp2!=NULL)
                           {
                              exportpkt(fp2,0);
                              fclose(fp2);
                              if (!testmode)
                                 remove(cp);
                           }
                           flowfilelist[i].type = FLOW_USED;
                        }
                        for (i=flowfiles-1; i>=0; i--)
                           free(flowfilelist[i].file);
                     }
                     exported = 1;
                     break;
                  }
               }
            }
         }
         if (exported && !testmode)
         {
            rewind(fp);
            msgheader.attrib |= 0x0008;   /* sent */
            fwrite(&msgheader,1,sizeof msgheader,fp);
         }
         fclose(fp);
         if (exported && (msgheader.attrib&0x0080)      /* kill/sent */
             && !testmode)
            remove(fname);
      }
      closedir(dp);
   }
}
/*...e*/
/*...spktexport:0:*/
void pktexport(void)
{
   char fname[FILENAME_MAX];
   DIR *dp;
   struct dirent *dirdata;
   FILE *fp;
   int status;

   routing = -1;   /* indicates routing has to be found out by address */

   strcpy(fname,packetdir);
   strcat(fname,"*");
   strcat(fname,packetext);
   dp = opendir(fname);
   if (dp!=NULL)
   {
      while ((dirdata=readdir(dp))!=NULL)
      {
         strcpy(fname, packetdir);
         strcat(fname, dirdata->d_name);
         fp = _fsopen(fname,"rb",SH_DENYNO);
         if (fp!=NULL)
         {
            status = exportpkt(fp,0);
            fclose(fp);
            if (status && !testmode)
               remove(fname);
         }
      }
      closedir(dp);
   }
}
/*...e*/

/*...sexport:0:*/
void export(void)
{
   int i;
   const char *origfmt;

   if (packetdir[0]!='\0')
      origfmt = "PKT";
   else if (mailertype==MAILER_ARCMAIL)
      origfmt = "MSG";
   else
      origfmt = "?UT/?LO";

   logprintf("Exporting from %s to Soup\n", origfmt);

   /* sets up reply file names */
   for (i=0; i<MAILTYPES; i++)
      genreplyfile(i);

   messagecount = 0;
   emailcount = 0;
   newscount = 0;
   listmsgcount = 0;

   if (packetdir[0]!='\0')
      pktexport();
   else if (mailertype==MAILER_ARCMAIL)
      msgexport();
   else
   {
      routing = 0;
      binkexport(&gateaddr,NULL,0);
      routing = 1;
      for (i=0; i<routes; i++)
      {
         strcpy(routemailpwd,route[i]->pwd);
         strcpy(routemailsubj,route[i]->mailsubj);
         strcpy(msgtoemail,route[i]->email);
         binkexport(&route[i]->fido,route[i]->email,1);
      }
   }

   if (!separatemsg)
   {
      /* updates REPLIES etc. */
      for (i=0; i<MAILTYPES; i++)
         genreplyfile(i);
   }

   if (messagecount>0)
   {
      logprintf("-> %ld messages exported\n"
                "mail: %-5ld     news: %-5ld",
                messagecount,
                emailcount, newscount);
      if (lists>0)
         logprintf("     lists: %-5ld",
                   listmsgcount);
      logprintf("\n");
   }
   else
      logprintf("No outbound messages found. Nothing to do.\n");
}
/*...e*/
