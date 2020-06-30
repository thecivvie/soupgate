/* MDEBUG simple, transparent memory allocation tracking utility
   Written by Tom Torfs (tomtorfs@mail.dma.be, 2:292/516@fidonet.org)
   Donated to the public domain, 1998-08-19
   Last update 1998-08-20 */

/* This header may be included multiple times. Depending on whether
   MDEBUG is defined or not, calls to malloc(), calloc(), realloc(),
   and free() will be intercepted. In addition, the following functions
   (implemented as macros) will be available:

   void checkmem(void);

   Will check the memory chain/blocks for corruption, and if this is
   detected an error message will be printed together with a listing
   of the memory chain (to stderr and the logfile), and the program
   will abort. You should insert calls to this function in those
   places in your program where you suspect memory corruption may
   have occurred. Every call to malloc(), calloc(), realloc() or
   free() will automatically call checkmem().

   int displaymemorychain(void);

   Will print out (to stderr and the logfile) the current memory chain,
   indicating any possible corruptions of the chain or memory blocks.
   Returns the number of errors found (0 means chain is OK).

   void noleak(void);

   Will check whether the allocated memory chain is empty (so all
   blocks have been freed). If not, prints out an error message together
   with a listing of the memory chain (to stderr and the logfile).

   void registernoleak(void);

   Registers the noleak() function to be automatically executed at
   program exit. Use this instead of atexit(noleak); because the
   latter will not work if the MDEBUG macro is not defined. */

/* Define the macro MDEBUG_AUTOCHECK to automatically check memory
   everytime one of these functions is called: (unless the macro
   listed next to it is defined)

   file I/O functions        MDEBUG_AUTOCHECK_NOFILEIO
   memory/string functions   MDEBUG_AUTOCHECK_NOMEMSTR

   Variable-argument functions (e.g. printf) are never intercepted. */

#undef malloc
#undef calloc
#undef realloc
#undef free
#undef checkmem
#undef displaymemorychain
#undef noleak
#undef registernoleak

#ifdef MDEBUG

/* for atexit() */
#include <stdio.h>

extern void *mdebug_malloc(size_t size,
                           const char *file, long line);
extern void *mdebug_calloc(size_t n, size_t size,
                           const char *file, long line);
extern void *mdebug_realloc(void *ptr, size_t size,
                            const char *file, long line,
                            const char *ptrname);
extern void mdebug_free(void *ptr,
                        const char *file, long line,
                        const char *ptrname);
extern void mdebug_checkmem(const char *file, long line);
extern int mdebug_displaymemorychain(const char *file, long line);
extern void mdebug_noleak(void);

#define malloc(size) mdebug_malloc(size,__FILE__,__LINE__)
#define calloc(n,size) mdebug_calloc(n,size,__FILE__,__LINE__)
#define realloc(ptr,size) mdebug_realloc(ptr,size,__FILE__,__LINE__,#ptr)
#define free(ptr) mdebug_free(ptr,__FILE__,__LINE__,#ptr)
#define checkmem() mdebug_checkmem(__FILE__,__LINE__)
#define displaymemorychain() mdebug_displaymemorychain(__FILE__,__LINE__)
#define noleak() mdebug_noleak()
#define registernoleak() atexit(mdebug_noleak)

#ifdef MDEBUG_AUTOCHECK

/* file I/O operations */

#ifndef MDEBUG_AUTOCHECK_NOFILEIO
#undef fopen
#define fopen(n,m) (checkmem(),fopen(n,m))
#undef freopen
#define freopen(n,m,f) (checkmem(),freopen(n,m,f))
#undef fclose
#define fclose(f) (checkmem(),fclose(f))
#undef fflush
#define fflush(f) (checkmem(),fflush(f))
#undef fgetc
#define fgetc(f) (checkmem(),fgetc(f))
#undef getc
#define getc(f) (checkmem(),getc(f))
#undef getchar
#define getchar() (checkmem(),getchar())
#undef fputc
#define fputc(c,f) (checkmem(),fputc(c,f))
#undef putc
#define putc(c,f) (checkmem(),putc(c,f))
#undef putchar
#define putchar(c) (checkmem(),putchar(c))
#undef fgets
#define fgets(s,l,f) (checkmem(),fgets(s,l,f))
#undef gets
#define gets(s) (checkmem(),gets(s))
#undef fputs
#define fputs(s,f) (checkmem(),fputs(s,f))
#undef puts
#define puts(s) (checkmem(),puts(s))
#undef fread
#define fread(b,s,n,f) (checkmem(),fread(b,s,n,f))
#undef fwrite
#define fwrite(b,s,n,f) (checkmem(),fwrite(b,s,n,f))
#undef fseek
#define fseek(f,o,w) (checkmem(),fseek(f,o,w))
#undef ftell
#define ftell(f) (checkmem(),ftell(f))
#undef rewind
#define rewind(f) (checkmem(),rewind(f))
#undef remove
#define remove(n) (checkmem(),remove(n))
#undef rename
#define rename(n1,n2) (checkmem(),rename(n1,n2))
#undef setbuf
#define setbuf(f,b) (checkmem(),setbuf(f,b))
#undef setvbuf
#define setvbuf(f,b,m,s) (checkmem(),setvbuf(f,b,m,s))
#undef tmpfile
#define tmpfile() (checkmem(),tmpfile())
#undef tmpnam
#define tmpnam(s) (checkmem(),tmpnam(s))
#endif

/* memory/string operations */

#ifndef MDEBUG_AUTOCHECK_NOMEMSTR
#undef memchr
#define memchr(s,c,n) (checkmem(),memchr(s,c,n))
#undef memcmp
#define memcmp(s1,s2,n) (checkmem(),memcmp(s1,s2,n))
#undef memcpy
#define memcpy(s1,s2,n) (checkmem(),memcpy(s1,s2,n))
#undef memmove
#define memmove(s1,s2,n) (checkmem(),memmove(s1,s2,n))
#undef memset
#define memset(s,c,n) (checkmem(),memset(s,c,n))
#undef strcat
#define strcat(s1,s2) (checkmem(),strcat(s1,s2))
#undef strncat
#define strncat(s1,s2,n) (checkmem(),strncat(s1,s2,n))
#undef strchr
#define strchr(s,c) (checkmem(),strchr(s,c))
#undef strrchr
#define strrchr(s,c) (checkmem(),strrchr(s,c))
#undef strstr
#define strstr(s1,s2) (checkmem(),strstr(s1,s2))
#undef strcmp
#define strcmp(s1,s2) (checkmem(),strcmp(s1,s2))
#undef strncmp
#define strncmp(s1,s2,n) (checkmem(),strncmp(s1,s2,n))
#undef strcpy
#define strcpy(s1,s2) (checkmem(),strcpy(s1,s2))
#undef strncpy
#define strncpy(s1,s2,n) (checkmem(),strncpy(s1,s2,n))
#undef strlen
#define strlen(s) (checkmem(),strlen(s))
#undef strcoll
#define strcoll(s1,s2) (checkmem(),strcoll(s1,s2))
#undef strtok
#define strtok(s1,s2) (checkmem(),strtok(s1,s2))
#undef strspn
#define strspn(s1,s2) (checkmem(),strspn(s1,s2))
#undef strcspn
#define strcspn(s1,s2) (checkmem(),strcspn(s1,s2))
#undef strpbrk
#define strpbrk(s1,s2) (checkmem(),strpbrk(s1,s2))
#endif

#endif

#else

#define checkmem() ((void)0)
#define displaymemorychain() ((void)0)
#define noleak() ((void)0)
#define registernoleak() ((void)0)

#endif
