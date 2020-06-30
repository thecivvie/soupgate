/* some public domain testing code :-) */

#ifdef FDEBUG

FILE *fdebug_fopen(const char *file, long line,
                   const char *fname, const char *mode);
FILE *fdebug_fsopen(const char *file, long line,
                    const char *fname, const char *mode, int flags);

#define fopen(fname,mode) \
      fdebug_fopen(__FILE__,__LINE__,fname,mode)
#define _fsopen(fname,mode,flags) \
      fdebug_fsopen(__FILE__,__LINE__,fname,mode,flags)

#endif
