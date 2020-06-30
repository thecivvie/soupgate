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

/* SoupGate pkt I/O functions */

#include "soupgate.h"

/*...swritepktheader:0:*/
void writepktheader(FILE *fpkt)
{
   struct PACKETHEADER pktheader;
   time_t t;
   struct tm *tm;

   time(&t);
   tm = localtime(&t);

   pktheader.orignode = gateaddr.node;
   pktheader.destnode = addrmap[0]->fido.node;
   pktheader.year = 1900+tm->tm_year;
   pktheader.month = tm->tm_mon;
   pktheader.day = tm->tm_mday;
   pktheader.hour = tm->tm_hour;
   pktheader.minute = tm->tm_min;
   pktheader.second = tm->tm_sec;
   pktheader.baudrate = 0;
   pktheader.version = 2;
   pktheader.orignet = gateaddr.net;
   pktheader.destnet = addrmap[0]->fido.net;
   pktheader.pcodelow = 0;
   pktheader.prevmajor = 0;
   strncpy(pktheader.password,packetpwd,8);
   pktheader.qmorigzone = gateaddr.zone;
   pktheader.qmdestzone = addrmap[0]->fido.zone;
   pktheader.auxnet = 0;
   pktheader.cwvalidate = 0x0100;
   pktheader.pcodehigh = 1;
   pktheader.prevminor = 0;
   pktheader.capword = 0x0001;
   pktheader.origzone = gateaddr.zone;
   pktheader.destzone = addrmap[0]->fido.zone;
   pktheader.origpoint = gateaddr.point;
   pktheader.destpoint = addrmap[0]->fido.point;
   pktheader.extrainfo = 0;
   fwrite(&pktheader,sizeof pktheader,1,fpkt);

   pktmsgcount = 0;
}
/*...e*/
/*...swritepktmsgheader:0:*/
void writepktmsgheader(int msgcategory, FILE *fpkt, char *fname,
                       int isjunk, int isownorg)
{
   struct PACKETMSGHEADER pktmsgheader;
   char buf[100];
   time_t t;
   struct tm *tm;

   time(&t);
   tm = localtime(&t);

   pktmsgheader.version = 2;
   pktmsgheader.orignode = fromaddr.node;
   pktmsgheader.destnode = destaddr.node;
   pktmsgheader.orignet = fromaddr.net;
   pktmsgheader.destnet = destaddr.net;
   if (msgcategory==CAT_EMAIL)
      pktmsgheader.attrib = 0x0001;    /* private */
   else
      pktmsgheader.attrib = 0x0000;    /* public */
   if (msgcategory!=CAT_NEWS && isfattach)
      pktmsgheader.attrib |= 0x0010;   /* file attach */
   pktmsgheader.cost = 0;
   fwrite(&pktmsgheader,sizeof pktmsgheader,1,fpkt);
   fprintf(fpkt,"%02d %-3s %02d  %02d:%02d:%02d%c",
               msgtime.day%100,month[msgtime.month],msgtime.year%100,
               msgtime.hour%100,msgtime.min%100,msgtime.sec%100,0);
   /* the year%100 above is required; the others are included to
      avoid corruption if the date header was parsed incorrectly
      for whatever reason */
   if (infoheader==INFO_NAME)
   {
      fprintf(fpkt,"%.*s%c",35,msgtoname,0);
      fprintf(fpkt,"%.*s%c",35,msgfromname,0);
   }
   else
   {
      fprintf(fpkt,"%.*s%c",35,msgtoemail,0);
      fprintf(fpkt,"%.*s%c",35,msgfromemail,0);
   }
   if (msgcategory!=CAT_NEWS && isfattach)
   {
      strncpy(buf,fname,72);
      buf[71] = '\0';
   }
   else
   {
      strncpy(buf,msgsubj,72);
      buf[71] = '\0';
      if (parts>1)
         sprintf(buf+strlen(buf)," (%ld/%ld)",partno,parts);
   }
   fprintf(fpkt,"%.71s%c",buf,0);
   if (strcmp(fidoareaname,NETMAILAREANAME)!=0)
      fprintf(fpkt,"AREA:%s\r",fidoareaname);
   fprintf(fpkt,"\1INTL %hu:%hu/%hu %hu:%hu/%hu\r",
              destaddr.zone,destaddr.net,destaddr.node,
              fromaddr.zone,fromaddr.net,fromaddr.node);
   if (fromaddr.point!=0)
      fprintf(fpkt,"\1FMPT %hu\r",fromaddr.point);
   if (destaddr.point!=0)
      fprintf(fpkt,"\1TOPT %hu\r",destaddr.point);
   if (!routing)
   {
      fprintf(fpkt,"\1REPLYADDR %s\r",msgreplyemail);
      fprintf(fpkt,"\1REPLYTO %hu:%hu/%hu.%hu %s\r",
                 gateaddr.zone,gateaddr.net,gateaddr.node,gateaddr.point,
                 UUCP);
   }
   if (msgid[0]!='\0' && partno==1)
   {
      if (validmsgid)
      {
         fprintf(fpkt,"\1MSGID: %hu:%hu/%hu.%hu %08lx\r",
                    msgidaddr.zone,msgidaddr.net,msgidaddr.node,msgidaddr.point,
                    fidomsgid);
      }
      else
      {
         fidomsgid = crc32(msgid,strlen(msgid));
         // strlwr(msgid);
         if (strictfidomsgid)
            fprintf(fpkt,"\1MSGID: %hu:%hu/%hu.%hu %08lx\r",
                       gateaddr.zone,gateaddr.net,gateaddr.node,gateaddr.point,
                       fidomsgid);
         else
            fprintf(fpkt,"\1MSGID: <%s> %08lx\r",msgid,fidomsgid);
      }
   }
   if (replyid[0]!='\0' && partno==1)
   {
      if (validreplyid)
      {
         fprintf(fpkt,"\1REPLY: %hu:%hu/%hu.%hu %08lx\r",
                    replyidaddr.zone,replyidaddr.net,replyidaddr.node,replyidaddr.point,
                    fidoreplyid);
      }
      else
      {
         fidoreplyid = crc32(replyid,strlen(replyid));
         // strlwr(replyid);
         if (strictfidomsgid)
            fprintf(fpkt,"\1REPLY: %hu:%hu/%hu.%hu %08lx\r",
                       gateaddr.zone,gateaddr.net,gateaddr.node,gateaddr.point,
                       fidoreplyid);
         else
            fprintf(fpkt,"\1REPLY: <%s> %08lx\r",replyid,fidoreplyid);
      }
   }
   fprintf(fpkt,"\1PID: SoupGate-%s v%d.%02d\r",
                osstr,MAJORVERSION,MINORVERSION);
   if (parts>1)
   {
      sprintf(buf,"%02d %-3s %02d %02d:%02d:%02d @%u/%u",
                  tm->tm_mday,month[tm->tm_mon],tm->tm_year%100,
                  tm->tm_hour,tm->tm_min,tm->tm_sec,
                  fromaddr.net,fromaddr.node);
      fprintf(fpkt,"\1SPLIT: %-32s00000 %02ld/%02ld +++++++++++\r",
                  buf,partno,parts);
   }
   if (fidocharset[0]!='\0')
   {
      fprintf(fpkt,"\1CHRS: %s\r",
                   fidocharset);
   }
   if (isjunk)
   {
      fprintf(fpkt,"\1JUNKMAIL: %s\r",junkname[isjunk]);
      if (junkaction==ACTION_JUNK)
         fprintf(fpkt,"\1JUNKMAIL-TO: %s\r",junkmailto);
   }
   if (isownorg)
   {
      fprintf(fpkt,"\1JUNKMAIL: OwnOrg\r");
      if (ownorgaction==ACTION_JUNK)
         fprintf(fpkt,"\1JUNKMAIL-TO: %s\r",junkmailto);
   }
}
/*...e*/
/*...swritemsgbottom:0:*/
void writemsgbottom(int msgcategory, FILE *fpkt)
{
   char addrbuf[24];
   time_t t;
   struct tm *tm;

   /* add tearline & origin */
   if (killorigin==KILL_NONE
       || (killorigin==KILL_EMAIL && msgcategory!=CAT_EMAIL))
   {
      fprintf(fpkt,"\r--- SoupGate-%s v%d.%02d\r",
                   osstr,MAJORVERSION,MINORVERSION);
      sprintf(addrbuf,"%hu:%hu/%hu",
              gateaddr.zone,gateaddr.net,gateaddr.node);
      if (gateaddr.point>0)
         sprintf(addrbuf+strlen(addrbuf),".%hu",gateaddr.point);
      fprintf(fpkt," * Origin: %.*s (%s)\r",
                  65 - strlen(addrbuf),
                  origin[0]=='\0'?
                     (msgorganization[0]=='\0'?msgfromemail:msgorganization)
                     :origin,
                  addrbuf);
   }
   if (msgcategory==CAT_EMAIL)
   {
      time(&t);
      tm = localtime(&t);

      fprintf(fpkt,"\1Via %hu:%hu/%hu @%04d%02d%02d.%02d%02d%02d SoupGate-%s v%d.%02d\r",
              gateaddr.zone,gateaddr.net,gateaddr.node,
              tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
              tm->tm_hour, tm->tm_min, tm->tm_sec,
              osstr,MAJORVERSION,MINORVERSION);
   }
   else
   {
      fprintf(fpkt,"SEEN-BY: %hu/%hu\r",
              gateaddr.net,gateaddr.node);
      fprintf(fpkt,"\1PATH: %hu/%hu\r",
              gateaddr.net,gateaddr.node);
   }

   fputc(0,fpkt);
}
/*...e*/
/*...swritefileinfo:0:*/
void writefileinfo(char *decfpath, char *decfname, int frenamed,
                   int decodefile, int dofile, long decodebytes,
                   int filesonly, FILE *fpkt)
{
   if (attach[msgtype]!=ATTACH_IGNORE)
   {
      if (dofile==ATTACH_DECODE && decodebytes==0)
      {
         /* silently kill empty file */
         remove(decfpath);
      }
      else
      {
         fprintf(stderr,"\r");
         logprintf("* %s %s-encoded file %s (%ld bytes)%s\n",
                   attachname[dofile],encodename[decodefile],
                   decfname,decodebytes,
                   frenamed?" (auto-renamed)":"");
         if (dofile!=ATTACH_IGNORE && !filesonly)
            fprintf(fpkt,"[SoupGate %s %s-encoded file %s (%ld bytes)%s]\r\n",
                         attachname[dofile],encodename[decodefile],
                         decfname,decodebytes,
                         frenamed?" (auto-renamed)":"");
         filecount++;
      }
   }
}
/*...e*/
/*...sclosepkt:0:*/
void closepkt(FILE *fpkt)
{
   /* terminate packet */
   fputc(0,fpkt);
   fputc(0,fpkt);
   fclose(fpkt);
}
/*...e*/
