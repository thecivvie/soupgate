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

/* SoupGate lists functions */

#include "soupgate.h"

/*...sexpandfname:0:*/
static void expandfname(char *name, char *argv0, char *fname)
{
   char *cp;

   if (
#ifndef __unix__
       strchr(name,'\\')==NULL &&
#endif
       strchr(name,'/')==NULL)
   {
      strncpy(fname,argv0,FILENAME_MAX-1);
#ifndef __unix__
      cp = strrchr(fname,'\\');
      if (cp==NULL)
#endif
         cp = strrchr(fname,'/');
      if (cp==NULL)
         cp = fname;
      else
         cp++;
      strcpy(cp,name);
   }
   else
      strcpy(fname,name);
}
/*...e*/
/*...sreadlist:0:*/
int readlist(char *listconfigfile, long lineno, char *origname, char *argv0)
{
   char fname[FILENAME_MAX];
   char tempname[FILENAME_MAX];
   char buffer[BUFSIZE];
   USERCONFIG *lastuser[27], *next;
   FILE *fp;
   int i,j,len;
   int deleted;
   char *cp;
   unsigned long ofs;

   lineno = 0;

   expandfname(listconfigfile, argv0, fname);

   fp = _fsopen(fname, "r", SH_DENYNO);
   if (fp==NULL)
   {
      configwarn("unable to open list configuration file",lineno,fname);
      return 0;
   }

   listconfig[lists].configname = malloc(strlen(fname)+1);
   if (listconfig[lists].configname!=NULL)
      strcpy(listconfig[lists].configname, fname);

   listconfig[lists].listname = NULL;
   listconfig[lists].listaddress = NULL;
   listconfig[lists].listarea = NULL;
   listconfig[lists].password = NULL;
   listconfig[lists].newstatus = 1;
   listconfig[lists].keepername = NULL;
   listconfig[lists].keeperaddress = NULL;
   listconfig[lists].subscribeinfo = NULL;
   listconfig[lists].welcome = NULL;
   listconfig[lists].goodbye = NULL;
   listconfig[lists].passwordinfo = NULL;
   listconfig[lists].readonly = NULL;
   listconfig[lists].blacklist = NULL;
   listconfig[lists].numusers = 0;
   for (i=0; i<=26; i++)
      listconfig[lists].users[i] = NULL;

   for (;;)
   {
      if (!fgets(buffer,BUFSIZE-1,fp))
         break;
      lineno++;
      len = strlen(buffer);
      /* strip both CR and LF, in case reading DOS file on unix */
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
      if (memicmp(buffer,"ListUsers",9)==0)
         break;
      else if (stricmp(buffer,"ListName")==0)
      {
         listconfig[lists].listname = malloc(strlen(cp)+1);
         if (listconfig[lists].listname==NULL)
         {
            configwarn("out of memory",lineno,fname);
            continue;
         }
         strcpy(listconfig[lists].listname, cp);
      }
      else if (stricmp(buffer,"ListAddress")==0)
      {
         listconfig[lists].listaddress = malloc(strlen(cp)+1);
         if (listconfig[lists].listaddress==NULL)
         {
            configwarn("out of memory",lineno,fname);
            continue;
         }
         strcpy(listconfig[lists].listaddress, cp);
      }
      else if (stricmp(buffer,"ListArea")==0)
      {
         listconfig[lists].listarea = malloc(strlen(cp)+1);
         if (listconfig[lists].listarea==NULL)
         {
            configwarn("out of memory",lineno,fname);
            continue;
         }
         strcpy(listconfig[lists].listarea, cp);
      }
      else if (stricmp(buffer,"ListPassword")==0)
      {
         listconfig[lists].password = malloc(strlen(cp)+1);
         if (listconfig[lists].password==NULL)
         {
            configwarn("out of memory",lineno,fname);
            continue;
         }
         strcpy(listconfig[lists].password, cp);
      }
      else if (stricmp(buffer,"NewStatus")==0)
      {
         listconfig[lists].newstatus = (unsigned char)atoi(cp);
         if (listconfig[lists].newstatus > 3)
            listconfig[lists].newstatus = 1;
      }
      else if (stricmp(buffer,"KeeperName")==0)
      {
         listconfig[lists].keepername = malloc(strlen(cp)+1);
         if (listconfig[lists].keepername==NULL)
         {
            configwarn("out of memory",lineno,fname);
            continue;
         }
         strcpy(listconfig[lists].keepername, cp);
      }
      else if (stricmp(buffer,"KeeperAddress")==0)
      {
         listconfig[lists].keeperaddress = malloc(strlen(cp)+1);
         if (listconfig[lists].keeperaddress==NULL)
         {
            configwarn("out of memory",lineno,fname);
            continue;
         }
         strcpy(listconfig[lists].keeperaddress, cp);
      }
      else if (stricmp(buffer,"SubscribeInfo")==0)
      {
         expandfname(cp, argv0, tempname);
         listconfig[lists].subscribeinfo = malloc(strlen(tempname)+1);
         if (listconfig[lists].subscribeinfo==NULL)
         {
            configwarn("out of memory",lineno,fname);
            continue;
         }
         strcpy(listconfig[lists].subscribeinfo, tempname);
      }
      else if (stricmp(buffer,"Welcome")==0)
      {
         expandfname(cp, argv0, tempname);
         listconfig[lists].welcome = malloc(strlen(tempname)+1);
         if (listconfig[lists].welcome==NULL)
         {
            configwarn("out of memory",lineno,fname);
            continue;
         }
         strcpy(listconfig[lists].welcome, tempname);
      }
      else if (stricmp(buffer,"GoodBye")==0)
      {
         expandfname(cp, argv0, tempname);
         listconfig[lists].goodbye = malloc(strlen(tempname)+1);
         if (listconfig[lists].goodbye==NULL)
         {
            configwarn("out of memory",lineno,fname);
            continue;
         }
         strcpy(listconfig[lists].goodbye, tempname);
      }
      else if (stricmp(buffer,"PasswordInfo")==0)
      {
         expandfname(cp, argv0, tempname);
         listconfig[lists].passwordinfo = malloc(strlen(tempname)+1);
         if (listconfig[lists].passwordinfo==NULL)
         {
            configwarn("out of memory",lineno,fname);
            continue;
         }
         strcpy(listconfig[lists].passwordinfo, tempname);
      }
      else if (stricmp(buffer,"NewPassword")==0)
      {
         expandfname(cp, argv0, tempname);
         listconfig[lists].newpassword = malloc(strlen(tempname)+1);
         if (listconfig[lists].newpassword==NULL)
         {
            configwarn("out of memory",lineno,fname);
            continue;
         }
         strcpy(listconfig[lists].newpassword, tempname);
      }
      else if (stricmp(buffer,"ReadOnly")==0)
      {
         expandfname(cp, argv0, tempname);
         listconfig[lists].readonly = malloc(strlen(tempname)+1);
         if (listconfig[lists].readonly==NULL)
         {
            configwarn("out of memory",lineno,fname);
            continue;
         }
         strcpy(listconfig[lists].readonly, tempname);
      }
      else if (stricmp(buffer,"BlackList")==0)
      {
         expandfname(cp, argv0, tempname);
         listconfig[lists].blacklist = malloc(strlen(tempname)+1);
         if (listconfig[lists].blacklist==NULL)
         {
            configwarn("out of memory",lineno,fname);
            continue;
         }
         strcpy(listconfig[lists].blacklist, tempname);
      }
      else
      {
         configwarn("invalid list configuration keyword",lineno,fname);
         continue;
      }
   }

   if (listconfig[lists].listname==NULL)
   {
      configwarn("required list keyword ListName or ListAddress missing",lineno,fname);
      /* free(NULL) is allowed */
      free(listconfig[lists].configname);
      free(listconfig[lists].listname);
      free(listconfig[lists].listaddress);
      free(listconfig[lists].listarea);
      free(listconfig[lists].keepername);
      free(listconfig[lists].keeperaddress);
      free(listconfig[lists].subscribeinfo);
      free(listconfig[lists].welcome);
      free(listconfig[lists].goodbye);
      free(listconfig[lists].passwordinfo);
      free(listconfig[lists].readonly);
      free(listconfig[lists].blacklist);
      return 0;
   }

   if (!feof(fp))
   {
      for (i=0; i<=26; i++)
         lastuser[i] = NULL;
      for (;;)
      {
         ofs = ftell(fp);
         fgets(buffer,BUFSIZE-1,fp);
         if (feof(fp))
            break;
         lineno++;
         if (buffer[0]=='*')
            deleted = 1;
         else if (buffer[0]=='+')
            deleted = 0;
         else
         {
            configwarn("user list contains erroneous line",lineno,fname);
            continue;
         }
         for (i=0; i<4; i++)
         {
            fgets(buffer,BUFSIZE-1,fp);
            if (feof(fp))
            {
               configwarn("user list ends in incomplete record",lineno,fname);
               break;
            }
            lineno++;
            if (i==0 && !deleted)
            {
               /* strip both CR and LF in case of DOS format on unix */
               cp = buffer + strlen(buffer) - 1;
               while (cp>=buffer
                      && (*cp=='\n' || *cp=='\r'))
                  *(cp--) = '\0';
               next = malloc(sizeof(USERCONFIG));
               if (next==NULL)
               {
                  configwarn("out of memory",lineno,fname);
                  break;
               }
               next->crc32 = crc32(buffer, strlen(buffer));
               next->ofs = ofs;
               next->next = NULL;
               j = classifyuser(buffer);
               if (lastuser[j]==NULL)
                  listconfig[lists].users[j] = next;
               else
                  lastuser[j]->next = next;
               lastuser[j] = next;
               listconfig[lists].numusers++;
            }
         }
      }
   }

   fclose(fp);

   if (testmode)
   {
      strcpy(tempname, listconfig[lists].configname);
      cp = strrchr(tempname, '.');
      if (cp==NULL)
         cp = tempname + strlen(tempname);
      strcpy(cp, ".~LC");

      fcopy(listconfig[lists].configname, tempname,
            buffer, BUFSIZE);

      strcpy(listconfig[lists].configname, tempname);

      if (lists==0)
         fprintf(stderr, "[Test mode: using list config file duplicates *.~LC]\n\n");
   }

   lists++;

   return 1;
}
/*...e*/
/*...spacklists:0:*/
void packlists(void)
{
   char temppath[FILENAME_MAX], tempname[FILENAME_MAX];
   char buffer[BUFSIZE];
   FILE *fp1, *fp2;
   char *cp;
   int i, list;
   long numdel, numinc, lineno;

   if (lists==0)
   {
      logprintf("No lists are configured; cannot PACK\n");
      return;
   }

   for (list=0; list<lists; list++)
   {
      logprintf("Packing list #%d: %s\n", list+1, listconfig[list].listname);
      /* temppath must be same dir as list config for rename() to work */
      strcpy(temppath, listconfig[list].configname);
#ifndef __unix__
      cp = strrchr(temppath, '\\');
      if (cp==NULL)
#endif
         cp = strrchr(temppath, '/');
      if (cp==NULL)
         temppath[0] = '\0';
      else
         cp[1] = '\0';
      genfnamebase(temppath,"list",".tmp",tempname);
      fp1 = _fsopen(listconfig[list].configname, "r", SH_DENYNO);
      if (fp1==NULL)
      {
         logprintf("! error opening %s for reading\n", listconfig[list].configname);
         continue;
      }
      fp2 = _fsopen(tempname, "w", SH_DENYWR);
      if (fp2==NULL)
      {
         fclose(fp1);
         logprintf("! error opening %s for writing\n", tempname);
         continue;
      }
      lineno = 0;
      do
      {
         if (!fgets(buffer,BUFSIZE-1,fp1))
            break;
         lineno++;
         fputs(buffer, fp2);
      }
      while (stricmp(buffer,"ListUsers\n")!=0);
      numdel = 0;
      numinc = 0;
      if (!feof(fp1))
      {
         for (;;)
         {
            if (!fgets(buffer,BUFSIZE-1,fp1))
               break;
            lineno++;
            if (buffer[0]=='*')
            {
               for (i=0; i<4; i++)
               {
                  if (!fgets(buffer,BUFSIZE-1,fp1))
                     goto done;
                  lineno++;
                  if (i==0)
                  {
                     if (verbose)
                        logprintf("* removed deleted user record %s", buffer);
                     numdel++;
                  }
               }
            }
            else if (buffer[0]=='+')
            {
               for (i=0; i<4; i++)
               {
                  if (!fgets(buffer,BUFSIZE-1,fp1))
                  {
                     switch(i)
                     {
                     case 0:
                        goto done;
                     case 2:
                        sprintf(buffer, "%d\n", listconfig[list].newstatus);
                        break;
                     default:
                        strcpy(buffer, "\n");
                        break;
                     }
                     if (i==3)
                        logprintf("* extended partial user record at end of file\n");
                  }
                  else
                     lineno++;
                  if (i==0)
                     fputs("+\n", fp2);
                  fputs(buffer, fp2);
               }
            }
            else
            {
               if (verbose)
                  logprintf("* removed erroneous line %ld: %s",
                            lineno, buffer);
               numinc++;
            }
         }
      }
done:
      fclose(fp2);
      fclose(fp1);
      if (numdel==0 && numinc==0)
         logprintf("--> nothing to do\n");
      else
         logprintf("--> removed %lu deleted user records\n"
                   "    + %lu erroneous lines\n",
                      numdel, numinc);
      remove(listconfig[list].configname);
      rename(tempname, listconfig[list].configname);
   }
}
/*...e*/

/*...sgetuserdata:0:*/
/* NULL, NULL means close file */
int getuserdata(int listno, USERCONFIG *config, USERDATA *data)
{
   static FILE *fp;
   char buffer[BUFSIZE];
   char *cp;

   if (config==NULL && data==NULL)
   {
      if (fp!=NULL)
      {
         fclose(fp);
         fp = NULL;
      }
      return 1;
   }

   if (fp==NULL)
   {
      fp = _fsopen(listconfig[listno].configname, "r", SH_DENYNO);
      if (fp==NULL)
      {
         logprintf("! error obtaining user data from file %s\n",
                   listconfig[listno].configname);
         return 0;
      }
   }

   fseek(fp, config->ofs, SEEK_SET);
   if (!fgets(buffer,BUFSIZE,fp)
       || stricmp(buffer,"+\n")!=0)
   {
conserror:
      logprintf("! internal consistency error in file %s\n",
                listconfig[listno].configname);
      return 0;
   }

   if (!fgets(buffer,BUFSIZE,fp))
      goto conserror;
   /* strip both CR and LF in case of DOS format on unix */
   cp = buffer + strlen(buffer) - 1;
   while (cp>=buffer
          && (*cp=='\n' || *cp=='\r'))
      *(cp--) = '\0';
   strncpy(data->email, buffer, EMAILBUFSIZE);

   if (!fgets(buffer,BUFSIZE,fp))
      goto conserror;
   /* strip both CR and LF in case of DOS format on unix */
   cp = buffer + strlen(buffer) - 1;
   while (cp>=buffer
          && (*cp=='\n' || *cp=='\r'))
      *(cp--) = '\0';
   strncpy(data->name, buffer, TEXTBUFSIZE);

   if (!fgets(buffer,BUFSIZE,fp))
      goto conserror;
   data->status = atoi(buffer);

   if (!fgets(buffer,BUFSIZE,fp))
      goto conserror;
   /* strip both CR and LF in case of DOS format on unix */
   cp = buffer + strlen(buffer) - 1;
   while (cp>=buffer
          && (*cp=='\n' || *cp=='\r'))
      *(cp--) = '\0';
   strncpy(data->password, buffer, TEXTBUFSIZE);

   return 1;
}
/*...e*/
/*...sfinduserdata:0:*/
/* triple pointers... fun --- feel free to pass NULL for the last 2 params */
int finduserdata(int listno, char *email, USERDATA *data,
                 USERCONFIG ***configptr, USERCONFIG **config)
{
   unsigned long crc;
   USERCONFIG *userptr;
   int found, j;

   crc = crc32(email, strlen(email));

   j = classifyuser(email);
   if (configptr!=NULL)
      *configptr = &listconfig[listno].users[j];
   userptr = listconfig[listno].users[j];
   found = 0;
   while (userptr!=NULL)
   {
      if (userptr->crc32 == crc)
      {
         if (getuserdata(listno, userptr, data)
             && stricmp(data->email, email) == 0)
         {
            if (config!=NULL)
               *config = userptr;
            found = 1;
            break;
         }
      }
      if (configptr!=NULL)
         *configptr = &userptr->next;
      userptr = userptr->next;
   }
   
   getuserdata(listno,NULL,NULL); /* close file */

   return found;
}
/*...e*/
/*...scalcpassword:0:*/
static void calcpassword(int listno, char *email, char *pwd)
{
   int i;
   int emaillen, pwdlen;

   if (listconfig[listno].password==NULL)
      return;

   pwdlen = strlen(listconfig[listno].password);
   if (pwdlen==0)
      return;

   emaillen = strlen(email);

   for (i=0; i<emaillen; i++)
      email[i] ^= ~listconfig[listno].password[i%pwdlen];

   /* note: email may no longer be a nul terminated string! */

   sprintf(pwd, "%lx", crc32(email, emaillen));

   for (i=0; i<emaillen; i++)
      email[i] ^= ~listconfig[listno].password[i%pwdlen];
}
/*...e*/
/*...screateuserdata:0:*/
void createuserdata(int listno, USERDATA *data)
{
   strcpy(data->email, msgfromemail);
   if (stricmp(msgfromemail, msgfromname) == 0)
      data->name[0] = '\0';
   else
      strcpy(data->name, msgfromname);
   data->status = listconfig[listno].newstatus;
   calcpassword(listno, data->email, data->password);
}
/*...e*/

/*...ssendlisttemplate:0:*/
void sendlisttemplate(int listno, USERDATA *userdata, char *template)
{
/*...svariables:0:*/
   char buffer[BUFSIZE];
   static unsigned long msgidcounter = 0;
   FILE *fp,*tfp;
   unsigned long startpos, messagesize;
   time_t t;
   struct tm *tm;
/*...e*/

/*...sinitialize:0:*/
   if (template==NULL)
      return;

   tfp = _fsopen(template, "r", SH_DENYNO);
   if (tfp==NULL)
      return;

   fp = _fsopen(replyfile[EMAIL],"r+b",SH_DENYWR);
   if (fp==NULL)
      fp = _fsopen(replyfile[EMAIL],"w+b",SH_DENYWR);
   if (fp==NULL)
   {
      fclose(tfp);
      return;
   }

   fseek(fp,0L,SEEK_END);
   startpos = ftell(fp);
   messagesize = 0;
   fwrite(&messagesize,1,4,fp);
/*...e*/

/*...swrite header:0:*/
   fprintf(fp,"To: %s", userdata->email);
   if (userdata->name[0]!='\0')
      fprintf(fp," (%s)", userdata->name);
   fprintf(fp,"\n");
   fprintf(fp,"From: %s", listconfig[listno].listaddress);
   if (listconfig[listno].listname[0]!='\0')
      fprintf(fp," (%s)", listconfig[listno].listname);
   fprintf(fp,"\n");
   fprintf(fp,"Reply-to: %s\n", listconfig[listno].listaddress);
   fprintf(fp,"Subject: Re: %s\n",msgsubj);
   time(&t);
   tm = localtime(&t);
   fprintf(fp,"Date: %s, %02d %-3s %04d %02d:%02d:%02d",
               weekday[tm->tm_wday],
               tm->tm_mday,month[tm->tm_mon],tm->tm_year+1900,
               tm->tm_hour,tm->tm_min,tm->tm_sec);
   if (timezonestr[0]!='\0')
      fprintf(fp," %s",timezonestr);
   fprintf(fp,"\n");
   msgidcounter += 0x398A6CDF; /* random prime; should not repeat soon */
   fprintf(fp,"Message-ID: <admin-%lu-%s>\n",
              (unsigned long)t
                 ^ crc32(userdata->email, strlen(userdata->email))
                 ^ crc32(msgsubj, strlen(msgsubj))
                 ^ msgidcounter,
              listconfig[listno].listaddress);
   fprintf(fp,"In-Reply-To: ");
   msgidprint(fp,msgid,fidomsgid,&msgidaddr);
   if (msgorganization[0]!='\0')
   {
      fprintf(fp,"Organization: %s\n",msgorganization);
      if (organization[0]!='\0')
         fprintf(fp,"X-MailListOrg: %s\n",organization);
   }
   else if (organization[0]!='\0')
      fprintf(fp,"Organization: %s\n",organization);
   fprintf(fp,"X-MailServer: SoupGate-%s v%d.%02d\n",
              osstr,MAJORVERSION,MINORVERSION);

   fprintf(fp,"\n");
/*...e*/

/*...swrite body:0:*/
   while (fgets(buffer,TEMPLATELINESIZE,tfp)!=NULL)
   {
      /* buffer's size is greater than TEMPLATELINESIZE */
      replacemacro(buffer, BUFSIZE, "@LISTNAME@", listconfig[listno].listname);
      replacemacro(buffer, BUFSIZE, "@LISTADDRESS@", listconfig[listno].listaddress);
      replacemacro(buffer, BUFSIZE, "@LISTAREA@", listconfig[listno].listarea);
      replacemacro(buffer, BUFSIZE, "@YOURNAME@", userdata->name);
      replacemacro(buffer, BUFSIZE, "@YOURADDRESS@", userdata->email);
      replacemacro(buffer, BUFSIZE, "@PASSWORD@", userdata->password);
      replacemacro(buffer, BUFSIZE, "@KEEPERNAME@", listconfig[listno].keepername);
      replacemacro(buffer, BUFSIZE, "@KEEPERADDRESS@", listconfig[listno].keeperaddress);

      /* \n included */
      fputs(buffer, fp);
   }
/*...e*/

/*...sprint information:0:*/
   if (verbose)
   {
      fprintf(stderr,"\r");
      logprintf("+ reply from %s",listconfig[listno].listaddress);
      if (listconfig[listno].listname[0]!='\0')
         logprintf(" (%s)", listconfig[listno].listname);
      logprintf(" to %s", userdata->email);
      if (userdata->name[0]!='\0')
         logprintf(" (%s)", userdata->name);
      logprintf(": %s\n", template);
   }
/*...e*/

/*...sfinish:0:*/
   fclose(tfp);

   fflush(fp);

   messagesize = ftell(fp) - startpos - 4;
   fseek(fp,startpos,SEEK_SET);
   fwrite((unsigned char *)&messagesize+3,1,1,fp);
   fwrite((unsigned char *)&messagesize+2,1,1,fp);
   fwrite((unsigned char *)&messagesize+1,1,1,fp);
   fwrite((unsigned char *)&messagesize+0,1,1,fp);

   fclose(fp);

   listreplycount++;
   newmsg[EMAIL]++;

   if (separatemsg)
      genreplyfile(EMAIL);
/*...e*/
}
/*...e*/
/*...ssendlistmessage:0:*/
/* only from MSG -> MSG; PKT -> MSG handled in exportmessage()
   returns 1 if message may be processed further; 0 if it shouldn't */
int sendlistmessage(int listno, FILE *sfp,
                    long bodypos, long endpos)
{
/*...svariables:0:*/
   USERCONFIG *ptr;
   USERDATA data;
   char buffer[BUFSIZE];
   FILE *fp;
   unsigned long startpos, messagesize;
   int i, count;
/*...e*/

/*...scheck subscription:0:*/
   if (!finduserdata(listno, msgfromemail, &data, NULL, NULL))
   {
      createuserdata(listno, &data);
      sendlisttemplate(listno, &data, listconfig[listno].subscribeinfo);
      return 0;
   }

   if (data.status==STATUS_BLACKLIST)
   {
      sendlisttemplate(listno, &data, listconfig[listno].blacklist);
      return 0;
   }

   if (data.status==STATUS_READONLY)
   {
      sendlisttemplate(listno, &data, listconfig[listno].readonly);
      return 0;
   }
/*...e*/

/*...sinitialize:0:*/
   if (listconfig[listno].numusers==0)
      return 1;

   fp = _fsopen(replyfile[EMAIL],"r+b",SH_DENYWR);
   if (fp==NULL)
      fp = _fsopen(replyfile[EMAIL],"w+b",SH_DENYWR);
   if (fp==NULL)
      return 1;

   fseek(fp,0L,SEEK_END);
   startpos = ftell(fp);
   messagesize = 0;
   fwrite(&messagesize,1,4,fp);
/*...e*/

/*...swrite header:0:*/
   fprintf(fp,"Bcc: ");
   count = 0;
   for (i=0; i<=26; i++)
   {
      ptr = listconfig[listno].users[i];
      while (ptr!=NULL)
      {
         if (getuserdata(listno, ptr, &data)
             && data.status != STATUS_BLACKLIST)
         {
            if (count>0)
            {
               if (count%2==0)
                  fprintf(fp, ",\n   ");
               else
                  fprintf(fp, ", ");
            }
            fprintf(fp,"<%s>", data.email);
            count++;
         }
         ptr = ptr->next;
      }
   }
   fprintf(fp,"\n");
   getuserdata(listno, NULL, NULL);   /* close file */
   fprintf(fp,"From: %s", msgfromemail);
   if (msgfromname[0]!='\0')
      fprintf(fp," (%s)", msgfromname);
   fprintf(fp,"\n");
   fprintf(fp,"Reply-to: %s\n", listconfig[listno].listaddress);
   fprintf(fp,"Sender: %s\n", listconfig[listno].listaddress);
   if (listconfig[listno].keeperaddress)
      fprintf(fp,"Errors-To: %s\n", listconfig[listno].keeperaddress);
   fprintf(fp,"Subject: %s\n",msgsubj);
   fprintf(fp,"Date: %s, %02d %-3s %04d %02d:%02d:%02d",
               weekday[dow(msgtime.year,msgtime.month+1,msgtime.day)],
               msgtime.day,month[msgtime.month],msgtime.year,
               msgtime.hour,msgtime.min,msgtime.sec);
   if (tzinfo[0]!='\0')
      fprintf(fp," %s",tzinfo);
   fprintf(fp,"\n");
   for (i=0; msgid[i]!='\0'; i++)
      if (msgid[i]=='@')
         msgid[i] = '#';
   fprintf(fp,"Message-ID: <%s-%s>\n",
              msgid,
              listconfig[listno].listaddress);
   if (fidoreplyid!=0 || replyid[0]!='\0')
   {
      fprintf(fp,"In-Reply-To: ");
      msgidprint(fp,replyid,fidoreplyid,&replyidaddr);
   }
   if (organization[0]!='\0')
      fprintf(fp,"Organization: %s\n", organization); /* for bouncebacks */
   fprintf(fp,"X-MailServer: SoupGate-%s v%d.%02d\n",
              osstr,MAJORVERSION,MINORVERSION);

   fprintf(fp,"Mime-version: 1.0\n");
   fprintf(fp,"Content-Type: ");
   if (mimetype[0]=='\0')
   {
      fprintf(fp, "text/plain; charset=%s\n",
               rfccharset[0]=='\0' ? "us-ascii" : rfccharset);
   }
   else
   {
      fprintf(fp, "%s\n", mimetype);
   }
   fprintf(fp,"Content-Transfer-Encoding: %s\n ",
               mimecte[0]=='\0' ? "7bit" : mimecte);

   fprintf(fp,"\n");
/*...e*/

/*...swrite body:0:*/
   fseek(sfp,bodypos,SEEK_SET);

   while (ftell(sfp) < endpos
          && fgets(buffer,BUFSIZE,sfp)!=NULL)
   {
      fputs(buffer, fp);
   }
/*...e*/

/*...sfinish:0:*/
   fflush(fp);

   messagesize = ftell(fp) - startpos - 4;
   fseek(fp,startpos,SEEK_SET);
   fwrite((unsigned char *)&messagesize+3,1,1,fp);
   fwrite((unsigned char *)&messagesize+2,1,1,fp);
   fwrite((unsigned char *)&messagesize+1,1,1,fp);
   fwrite((unsigned char *)&messagesize+0,1,1,fp);

   fclose(fp);

   newmsg[EMAIL]++;

   if (separatemsg)
      genreplyfile(EMAIL);

   return 1;
/*...e*/
}
/*...e*/
/*...sprocesslistcommand:0:*/
void processlistcommand(int listno, int listcommand)
{
   USERDATA userdata;
   USERCONFIG *next, *prev;
   USERCONFIG *config, **configptr;
   char buffer[BUFSIZE];
   unsigned long ofs;
   FILE *fp;
   char *cp;
   char *pwd1, *pwd2;
   int pwd1len, pwd2len;
   int newuser;
   int j;

   if (finduserdata(listno, msgfromemail, &userdata, &configptr, &config))
      newuser = 0;
   else
   {
      newuser = 1;
      createuserdata(listno, &userdata);
   }

   if (newuser && listcommand != COMMAND_SUBSCRIBE)
   {
      sendlisttemplate(listno, &userdata, listconfig[listno].subscribeinfo);
      return;
   }

   if (userdata.status == STATUS_BLACKLIST)
   {
      sendlisttemplate(listno, &userdata, listconfig[listno].blacklist);
      return;
   }

   if (listconfig[listno].password!=NULL)
   {
      cp = msgsubj;
      while (*cp!='\0' && !isspace(*cp))
         cp++;
      while (*cp!='\0' && isspace(*cp))
         cp++;
      pwd1 = cp;
      pwd1len = 0;
      while (*cp!='\0' && !isspace(*cp))
      {
         cp++;
         if (pwd1len < TEXTBUFSIZE - 1)
            pwd1len++;
      }
      while (*cp!='\0' && isspace(*cp))
         cp++;
      pwd2 = cp;
      pwd2len = 0;
      while (*cp!='\0' && !isspace(*cp))
      {
         cp++;
         if (pwd2len < TEXTBUFSIZE - 1)
            pwd2len++;
      }
      if (pwd1len==0
          || (listcommand==COMMAND_CHANGEPASS && pwd2len==0)
          || pwd1len!=strlen(userdata.password)
          || memcmp(pwd1, userdata.password, pwd1len)!=0)   /* case sensitive */
      {
         sendlisttemplate(listno, &userdata, listconfig[listno].passwordinfo);
         return;
      }
   }

   /* if we got here we have a correct password (if required) */

   if (listcommand == COMMAND_SUBSCRIBE)
   {
      if (newuser)   /* no need to do anything otherwise */
      {
         fp = _fsopen(listconfig[listno].configname, "r+", SH_DENYWR);
         if (fp==NULL)
         {
            logprintf("! unable to add new user to %s\n",
                      listconfig[listno].configname);
            return;
         }

         fseek(fp, 0, SEEK_END);
         ofs = ftell(fp);

         fprintf(fp, "+\n");
         fprintf(fp, "%s\n", userdata.email);
         fprintf(fp, "%s\n", userdata.name);
         fprintf(fp, "%d\n", userdata.status);
         fprintf(fp, "%s\n", userdata.password);

         fclose(fp);
         
         next = malloc(sizeof(USERCONFIG));
         if (next==NULL)
         {
            logprintf("! not enough memory for new user; changes will become");
            logprintf("  effective only upon next program invocation\n");
         }
         else
         {
            next->crc32 = crc32(userdata.email, strlen(userdata.email));
            next->ofs = ofs;
            next->next = NULL;
            j = classifyuser(userdata.email);
            prev = listconfig[listno].users[j];
            if (prev==NULL)
            {
               listconfig[listno].users[j] = next;
            }
            else
            {
               while (prev->next!=NULL)
                  prev = prev->next;
               prev->next = next;
            }
            listconfig[listno].numusers++;
         }
      }

      sendlisttemplate(listno, &userdata, listconfig[listno].welcome);
      return;
   }

   if (listcommand==COMMAND_UNSUBSCRIBE)
   {
      /* can't get here for new user */
   
      fp = _fsopen(listconfig[listno].configname, "r+", SH_DENYWR);
      if (fp==NULL)
      {
         logprintf("! unable to delete user from %s\n",
                   listconfig[listno].configname);
         return;
      }

      fseek(fp, config->ofs, SEEK_SET);

      if (!fgets(buffer,BUFSIZE,fp)
          || stricmp(buffer,"+\n")!=0)
      {
   conserror:
         logprintf("! internal consistency error in file %s\n",
                   listconfig[listno].configname);
         return;
      }

      if (!fgets(buffer,BUFSIZE,fp))
         goto conserror;
      /* strip both CR and LF in case of DOS format on unix */
      cp = buffer + strlen(buffer) - 1;
      while (cp>=buffer
             && (*cp=='\n' || *cp=='\r'))
         *(cp--) = '\0';
      if (stricmp(buffer,userdata.email)!=0)
         goto conserror;

      fseek(fp, config->ofs, SEEK_SET);
      fputc('*', fp);      /* delete */

      fclose(fp);

      *configptr = config->next;
      free(config);

      listconfig[listno].numusers--;

      sendlisttemplate(listno, &userdata, listconfig[listno].goodbye);
      return;
   }

   if (listcommand == COMMAND_CHANGEPASS)
   {
      /* can't get here for new user
         pwd2 is checked already */

      memcpy(userdata.password, pwd2, pwd2len);
      userdata.password[pwd2len] = '\0';

      fp = _fsopen(listconfig[listno].configname, "r+", SH_DENYWR);
      if (fp==NULL)
      {
         logprintf("! unable to change user data in %s\n",
                   listconfig[listno].configname);
         return;
      }

      fseek(fp, config->ofs, SEEK_SET);

      if (!fgets(buffer,BUFSIZE,fp)
          || stricmp(buffer,"+\n")!=0)
         goto conserror;

      if (!fgets(buffer,BUFSIZE,fp))
         goto conserror;
      /* strip both CR and LF in case of DOS format on unix */
      cp = buffer + strlen(buffer) - 1;
      while (cp>=buffer
             && (*cp=='\n' || *cp=='\r'))
         *(cp--) = '\0';
      if (stricmp(buffer,userdata.email)!=0)
         goto conserror;

      fseek(fp, config->ofs, SEEK_SET);
      fputc('*', fp);      /* delete */

      fseek(fp, 0, SEEK_END);
      config->ofs = ftell(fp);

      fprintf(fp, "+\n");
      fprintf(fp, "%s\n", userdata.email);
      fprintf(fp, "%s\n", userdata.name);
      fprintf(fp, "%d\n", userdata.status);
      fprintf(fp, "%s\n", userdata.password);

      fclose(fp);

      sendlisttemplate(listno, &userdata, listconfig[listno].newpassword);
      return;
   }
}
/*...e*/
