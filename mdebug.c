/* MDEBUG simple, transparent memory allocation tracking utility
   Written by Tom Torfs (tomtorfs@mail.dma.be, 2:292/516@fidonet.org)
   Donated to the public domain, 1998-08-19
   Last update 1998-08-20 */

/* The macros MDEBUG_SIG (string), MDEBUG_PRE and MDEBUG_POST (numbers)
   may be overridden from the commandline. MDEBUG_PRE should be a
   suitable number for every possible alignment and should be larger
   than sizeof(struct mdebug_info). */

/* Define macro MDEBUG_NOLOG to disable logging to memlog file;
   define macro MDEBUG_QUIET to disable messages to stderr;
   define macro MDEBUG_NOMSG to disable all messages except in
   case of error or a call to mdebug_displaymemorychain()
   define macro MDEBUG_OKMSG to enable messages in case of a
   successful call to mdebug_checkmem() */

/* See mdebug.h for instructions on usage */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>

#ifndef MDEBUG_SIG
#define MDEBUG_SIG "{MDEBUG}"
#endif

#define MDEBUG_SIGLEN (sizeof MDEBUG_SIG - 1)

#ifndef MDEBUG_PRE
#define MDEBUG_PRE 64
#endif

#ifndef MDEBUG_POST
#define MDEBUG_POST 128
#endif

struct mdebug_info {size_t size;
                    struct mdebug_info *next;
                    struct mdebug_info *prev;};

static struct mdebug_info *mdebug_first = NULL;
static struct mdebug_info *mdebug_last = NULL;

static FILE *logfile = NULL;

static void mdebug_addlink(struct mdebug_info *ptr, size_t size);
static void mdebug_removelink(struct mdebug_info *ptr);
static int mdebug_checkpointer(void *ptr);

static void mdebug_initblock(void *ptr, size_t size);
static void *mdebug_checkblock(void *ptr);

static void mdebug_corruptblock(void *ptr,
                                const char *ptrname,
                                const char *file, long line,
                                void *badp);
static void mdebug_corruptpointer(void *ptr,
                                  const char *ptrname,
                                  const char *file, long line);
static void mdebug_corruptchain(const char *file, long line);

static void mdebug_log(char *s, ...);

void mdebug_checkmem(const char *file, long line);

void *mdebug_malloc(size_t size,
                    const char *file, long line)
{
   void *ptr;

   mdebug_checkmem(file,line);

#ifndef MDEBUG_NOMSG
   mdebug_log("MDEBUG: %s(%ld): malloc(%lu): ",
                  file, line, (unsigned long)size);
#endif

   if (MDEBUG_PRE + size + MDEBUG_POST <= size)   /* overflow */
   {
#ifndef MDEBUG_NOMSG
      mdebug_log("overflow\n");
#endif
      return NULL;
   }

   ptr = malloc(MDEBUG_PRE + size + MDEBUG_POST);

   if (ptr==NULL)
   {
#ifndef MDEBUG_NOMSG
      mdebug_log("NULL\n");
#endif
      return NULL;
   }
   else
   {
#ifndef MDEBUG_NOMSG
      mdebug_log("%p\n",(void *)((char *)ptr + MDEBUG_PRE));
#endif
      mdebug_initblock(ptr, size);
      return (char *)ptr + MDEBUG_PRE;
   }
}

void *mdebug_calloc(size_t n, size_t size,
                    const char *file, long line)
{
   void *ptr;

   mdebug_checkmem(file,line);

#ifndef MDEBUG_NOMSG
   mdebug_log("MDEBUG: %s(%ld): calloc(%lu,%lu): ",
                  file, line, (unsigned long)n,(unsigned long)size);
#endif

   if (n*size<n || n*size<size
       || MDEBUG_PRE + n*size + MDEBUG_POST <= n*size)   /* overflow */
   {
#ifndef MDEBUG_NOMSG
      mdebug_log("overflow\n");
#endif
      return NULL;
   }

   ptr = malloc(MDEBUG_PRE + n*size + MDEBUG_POST);

   if (ptr==NULL)
   {
#ifndef MDEBUG_NOMSG
      mdebug_log("NULL\n");
#endif
      return NULL;
   }
   else
   {
#ifndef MDEBUG_NOMSG
      mdebug_log("%p\n",ptr);
#endif
      memset((char *)ptr + MDEBUG_PRE, 0, n*size);
      mdebug_initblock(ptr, n*size);
      return (char *)ptr + MDEBUG_PRE;
   }
}

void *mdebug_realloc(void *ptr, size_t size,
                     const char *file, long line,
                     const char *ptrname)
{
   void *newptr;
   void *badp;

   mdebug_checkmem(file,line);

#ifndef MDEBUG_NOMSG
   mdebug_log("MDEBUG: %s(%ld): realloc(%s,%lu): ",
                  file, line, ptrname, (unsigned long)size);
   if (ptr==NULL)
      mdebug_log("NULL");
   else
      mdebug_log("%p",ptr);
#endif

   if (MDEBUG_PRE + size + MDEBUG_POST <= size)   /* overflow */
   {
#ifndef MDEBUG_NOMSG
      mdebug_log(" -> overflow\n");
#endif
      return NULL;
   }

   if (mdebug_checkpointer(ptr) != 0)
      mdebug_corruptpointer(ptr,ptrname,file,line);

   if ((badp=mdebug_checkblock((char *)ptr-MDEBUG_PRE)) != NULL)
      mdebug_corruptblock((char *)ptr-MDEBUG_PRE,ptrname,file,line,badp);

   mdebug_removelink((struct mdebug_info *)((char *)ptr - MDEBUG_PRE));

   if (size==0)
   {
      free((char *)ptr - MDEBUG_PRE);
      newptr = NULL;
   }
   else
   {
      newptr = realloc((char *)ptr - MDEBUG_PRE,
                       MDEBUG_PRE + size + MDEBUG_POST);
   }

   if (newptr==NULL)
   {
#ifndef MDEBUG_NOMSG
      mdebug_log(" -> NULL\n");
#endif
      if (size!=0)
         mdebug_addlink((struct mdebug_info *)((char *)ptr - MDEBUG_PRE),
                        ((struct mdebug_info *)((char *)ptr - MDEBUG_PRE))
                        ->size);
      return NULL;
   }
   else
   {
#ifndef MDEBUG_NOMSG
      mdebug_log(" -> %p\n",(void *)((char *)newptr + MDEBUG_PRE));
#endif
      mdebug_initblock(newptr, size);
      return (char *)newptr + MDEBUG_PRE;
   }
}

void mdebug_free(void *ptr,
                 const char *file, long line,
                 const char *ptrname)
{
   void *badp;

   mdebug_checkmem(file,line);

#ifndef MDEBUG_NOMSG
   mdebug_log("MDEBUG: %s(%ld): free(%s): ",
                  file, line, ptrname);
   if (ptr==NULL)
      mdebug_log("NULL\n");
   else
      mdebug_log("%p\n",ptr);
#endif

   if (ptr==NULL)
      return;

   if (mdebug_checkpointer(ptr) != 0)
      mdebug_corruptpointer(ptr,ptrname,file,line);

   if ((badp=mdebug_checkblock((char *)ptr-MDEBUG_PRE)) != NULL)
      mdebug_corruptblock((char *)ptr-MDEBUG_PRE,ptrname,file,line,badp);

   mdebug_removelink((struct mdebug_info *)((char *)ptr - MDEBUG_PRE));

   free((char *)ptr - MDEBUG_PRE);
}

void mdebug_checkmem(const char *file, long line)
{
   struct mdebug_info *mptr;
   void *badp;

   for (mptr=mdebug_first; mptr!=NULL; mptr=mptr->next)
   {
      if (mptr==mdebug_first && mptr->prev!=NULL)
         mdebug_corruptchain(file,line);
      if (mptr==mdebug_last && mptr->next!=NULL)
         mdebug_corruptchain(file,line);
      if (mptr->prev==NULL && mptr!=mdebug_first)
         mdebug_corruptchain(file,line);
      if (mptr->next==NULL && mptr!=mdebug_last)
         mdebug_corruptchain(file,line);
      if (mptr->prev!=NULL && mptr->prev->next!=mptr)
         mdebug_corruptchain(file,line);
      if (mptr->next!=NULL && mptr->next->prev!=mptr)
         mdebug_corruptchain(file,line);
      if ((badp=mdebug_checkblock((void *)mptr)) != NULL)
         mdebug_corruptblock((void *)mptr,"(unknown)",file,line,badp);
   }

#ifdef MDEBUG_OKMSG
   mdebug_log("MDEBUG: %s(%ld): checkmem(): ok\n",
                  file, line);
#endif
}

int mdebug_displaymemorychain(const char *file, long line)
{
   struct mdebug_info *mptr;
   void *badp;
   int errors;

   mdebug_log("LISTING OF MEMORY CHAIN; called from %s(%ld)\n",
              file, line);

   errors = 0;

   for (mptr=mdebug_first; mptr!=NULL; mptr=mptr->next)
   {
      mdebug_log("   memory block %p; size=%lu;",
               (void *)((char *)mptr + MDEBUG_PRE),
               (unsigned long)mptr->size);
      if (mptr->prev==NULL)
         mdebug_log(" prev=NULL;");
      else
         mdebug_log(" prev=%p;", (void *)((char *)mptr->prev
                                    + MDEBUG_PRE));
      if (mptr->next==NULL)
         mdebug_log(" next=NULL\n");
      else
         mdebug_log(" next=%p\n", (void *)((char *)mptr->next
                                     + MDEBUG_PRE));
      if (mptr==mdebug_first && mptr->prev!=NULL)
      {
         mdebug_log("ERROR: first node has link to previous\n");
         errors++;
      }
      if (mptr==mdebug_last && mptr->next!=NULL)
      {
         mdebug_log("ERROR: last node has link to next\n");
         errors++;
      }
      if (mptr->prev==NULL && mptr!=mdebug_first)
      {
         mdebug_log("ERROR: intermediate node has no link to previous\n");
         errors++;
      }
      if (mptr->next==NULL && mptr!=mdebug_last)
      {
         mdebug_log("ERROR: intermediate node has no link to next\n");
         errors++;
      }
      if (mptr->prev!=NULL && mptr->prev->next!=mptr)
      {
         mdebug_log("ERROR: intermediate node not correctly linked to previous\n");
         errors++;
      }
      if (mptr->next!=NULL && mptr->next->prev!=mptr)
      {
         mdebug_log("ERROR: intermediate node not correctly linked to next\n");
         errors++;
      }
      if ((badp=mdebug_checkblock((void *)mptr)) != NULL)
      {
         mdebug_log("ERROR: corrupted memory block at address %p\n", badp);
         errors++;
      }
   }

   if (mdebug_first==NULL)
      mdebug_log("   no memory blocks currently allocated\n");
   else
      mdebug_log("   %d error(s) found in memory chain\n", errors);

   return errors;
}

void mdebug_noleak(void)
{
   if (mdebug_first==NULL)
      return;

   mdebug_log("WARNING: memory chain not empty; possible memory leak\n");

   mdebug_displaymemorychain(__FILE__,__LINE__);
}

static void mdebug_initblock(void *ptr, size_t size)
{
   int i;

   if (ptr==NULL)
      return;

   for (i=sizeof(struct mdebug_info); i < MDEBUG_PRE; i++)
      ((char *)ptr)[i] = MDEBUG_SIG[i % MDEBUG_SIGLEN];

   for (i=0; i < MDEBUG_POST; i++)
      ((char *)ptr)[MDEBUG_PRE + size + i] = MDEBUG_SIG[i % MDEBUG_SIGLEN];

   mdebug_addlink((struct mdebug_info *)ptr, size);
}

static void *mdebug_checkblock(void *ptr)
{
   int i;
   void *badp;
   size_t size;

   if (ptr==NULL)
      return NULL;

   size = ((struct mdebug_info *)ptr)->size;

   badp = NULL;

   for (i=sizeof(struct mdebug_info); i < MDEBUG_PRE && !badp; i++)
      if (((char *)ptr)[i] != MDEBUG_SIG[i % MDEBUG_SIGLEN])
         badp = (char *)ptr + i;

   for (i=0; i < MDEBUG_POST && !badp; i++)
      if (((char *)ptr)[MDEBUG_PRE + size + i]
                                     != MDEBUG_SIG[i % MDEBUG_SIGLEN])
         badp = (char *)ptr + MDEBUG_PRE + size + i;

   return badp;
}

/* assumes linked list is in order; prior call to
   mdebug_checkmem() required */
static void mdebug_addlink(struct mdebug_info *ptr, size_t size)
{
   assert(ptr!=NULL);

   if (mdebug_last==NULL)
      mdebug_first = ptr;
   else
      mdebug_last->next = ptr;

   ptr->size = size;
   ptr->next = NULL;
   ptr->prev = mdebug_last;

   mdebug_last = ptr;
}

/* assumes linked list is in order and links is present;
   prior call to mdebug_checkmem() and mdebug_checkpointer() required */
static void mdebug_removelink(struct mdebug_info *ptr)
{
   assert(ptr!=NULL);

   if (ptr->prev==NULL)
      mdebug_first = ptr->next;
   else
      ptr->prev->next = ptr->next;

   if (ptr->next==NULL)
      mdebug_last = ptr->prev;
   else
      ptr->next->prev = ptr->prev;
}

/* assumes linked list is in order; prior call to
   mdebug_checkmem() required */
static int mdebug_checkpointer(void *ptr)
{
   struct mdebug_info *mptr;
   int bad;

   bad = 1;
   for (mptr=mdebug_first; mptr!=NULL && bad; mptr=mptr->next)
      if ((char *)mptr == (char *)ptr - MDEBUG_PRE)
         bad = 0;

   return bad;
}

static void mdebug_corruptblock(void *ptr,
                                const char *ptrname,
                                const char *file, long line,
                                void *badp)
{
   mdebug_log("MEMORY BLOCK CORRUPTED; called from %s(%ld)\n"
              "   address: %p; size: %lu; name: %s; corruption: %p\n",
                  file,line,
                  (void *)((char *)ptr + MDEBUG_PRE),
                  (unsigned long)
                  ((struct mdebug_info *)ptr)->size,
                  ptrname,
                  badp);
   mdebug_displaymemorychain(__FILE__,__LINE__);
   if (logfile!=NULL)
      fclose(logfile);
   abort();
}

static void mdebug_corruptpointer(void *ptr,
                                  const char *ptrname,
                                  const char *file, long line)
{
   mdebug_log("CORRUPT POINTER PASSED; called from %s(%ld)\n"
              "   address: %p; name: %s; pointer not in memory chain\n",
                  file,line,
                  ptr,
                  ptrname);
   mdebug_displaymemorychain(__FILE__,__LINE__);
   if (logfile!=NULL)
      fclose(logfile);
   abort();
}

static void mdebug_corruptchain(const char *file, long line)
{
   mdebug_log("MEMORY CHAIN CORRUPTED; called from %s(%ld)\n",
                  file, line);
   mdebug_displaymemorychain(__FILE__,__LINE__);
   if (logfile!=NULL)
      fclose(logfile);
   abort();
}

static void mdebug_log(char *s, ...)
{
   time_t t;
   struct tm *tm;
   va_list ap;

#ifndef MDEBUG_QUIET
   va_start(ap,s);
   vfprintf(stderr,s,ap);
   fflush(stderr);
   va_end(ap);
#endif

#ifndef MDEBUG_NOLOG
   if (logfile==NULL)
   {
      logfile = fopen("memlog","a");
      if (logfile!=NULL)
      {
         /* logfile will be closed automatically upon normal exit */
         if (ftell(logfile)>0)
            fprintf(logfile,"\n");
         fprintf(logfile,"--- MDEBUG log entry");
         if (time(&t)!=(time_t)-1)
         {
            tm = localtime(&t);
            fprintf(logfile," %04d-%02d-%02d %02d:%02d:%02d",
                          1900+tm->tm_year,tm->tm_mon+1,tm->tm_mday,
                          tm->tm_hour,tm->tm_min,tm->tm_sec);
         }
         fprintf(logfile,"\n");
      }
   }
#endif

   if (logfile!=NULL)
   {
      va_start(ap,s);
      vfprintf(logfile,s,ap);
      fflush(logfile);
      va_end(ap);
  }
}
