/* some public domain testing code :-) */

#include <stdio.h>
#include <io.h>

FILE *fdebug_fopen(const char *file, long line,
                   const char *fname, const char *mode)
{
   printf("***%s(%ld):fopen(\"%s\",\"%s\")\n",
          file, line, fname, mode);
   return fopen(fname, mode);
}

FILE *fdebug_fsopen(const char *file, long line,
                    const char *fname, const char *mode, int flags)
{
   printf("***%s(%ld):_fsopen(\"%s\",\"%s\",%d)\n",
          file, line, fname, mode, flags);
   return _fsopen(fname, mode, flags);
}
