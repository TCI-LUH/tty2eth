#pragma once

#ifdef WOLFSSH_USER_FILESYSTEM


extern int errno;
#include "memfs.h"
#include <fcntl.h>

#define WFILE MemFile
#define WSEEK_END           SEEK_END
#define WBADFILE            NULL
#define WS_DELIM            '/'


#define WOLFSSH_O_RDWR   O_RDWR
#define WOLFSSH_O_RDONLY O_RDONLY
#define WOLFSSH_O_WRONLY O_WRONLY
#define WOLFSSH_O_APPEND O_APPEND
#define WOLFSSH_O_CREAT  O_CREAT
#define WOLFSSH_O_TRUNC  O_TRUNC
#define WOLFSSH_O_EXCL   O_EXCL


#define WFD int

#define WFOPEN(fs, file, filename, mode) memfopen(file, filename, mode)


#define WFCLOSE(fs,file) memfclose(file) 

#define WFREAD(fs, buffer, size, n, file) memfread(buffer, size, n, file)

#define WFWRITE(fs, buffer, size, n, file) memfwrite(buffer, size, n, file)

#define WFSEEK(fs, file, pos, mode) memfseek(file, pos, mode)

#define WFTELL(fs, file) memftell(file)

#define WREWIND(fs, file) memrewind(file)

#define WOPEN(fs, filename, fd, mode) memopen(filename, fd, mode)

#define WCLOSE(fs, fd) memclose(fd)

#define WPREAD(fs, fd, buffer, count, offset) mempread(fd, buffer, count, offset)

#define WPWRITE(fs, fd, buffer, count, offset) mempwrite(fd, buffer, count, offset)

#define WGETCWD memgetcwd

#define WRMDIR memrmdir

#define WMKDIR memmkdir 

#define WREMOVE memremove

#define WRENAME memrename

// #define WSTAT memstat 

// #define WLSTAT memlstat 


#define WSETTIME(fs,f,a,m) (0)
#define WFSETTIME(fs,fd,a,m) (0)

#define WCHMOD(fs, path, mode) (0)
#define WFCHMOD(fs, path, mode) (0)

#define WDIR MemDir*

#define WOPENDIR(fs,h,c,d)  ((*(c) = memopendir((d))) == NULL)
#define WCLOSEDIR(fs, d)  memclosedir(*(d))
#define WREADDIR(fs, d)   memreaddir(*(d))
#define WREWINDDIR(fs, d) memrewinddir(*(d))


typedef struct WOLFSSH WOLFSSH;
typedef struct WS_SFTP_FILEATRB WS_SFTP_FILEATRB;
typedef unsigned char  byte;
int SFTP_GetAttributes(void *fs, const char *fileName, WS_SFTP_FILEATRB *atr, byte noFollow, void *heap);
int SFTP_GetAttributes_Handle(WOLFSSH *ssh, byte *handle, int handleSz, WS_SFTP_FILEATRB *atr);

#endif /*  WOLFSSH_USER_FILESYSTEM */