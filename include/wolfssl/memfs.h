#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#define MEMFS_MAX_FILES 5
#define MEMFS_MAX_OPEN_FILES 10
#define MEMFS_MAX_OPEN_DIRS 10

typedef void *osMutexId_t;

// typedef int mode_t; 

typedef struct FileDef FileDef;

typedef void (*fileUpdateEvent)(FileDef* file);

typedef struct FileDef
{
    const char *filename;
    void* start;
    size_t fileSize;
    size_t bufSize;
    bool saveStorage;
    fileUpdateEvent update;
    osMutexId_t mutex;
} FileDef;

typedef struct
{
    FileDef *fileDef;
    int fd;
    off_t pos;
    bool hasWritten;
    int error;
} MemFile;

struct dirent
{
    ino_t d_ino;
    off_t d_off;
    unsigned short d_reclen;
    unsigned char d_type; 
    char d_name[256]; 
};

typedef struct
{
    const char *name;
    int fd;
    int fileIdx;
    struct dirent entry;
} MemDir;



FileDef* memAllocateFile(const char *filename, void* start, size_t fileSize, size_t bufSize, bool saveStorage, fileUpdateEvent update);

int memfopen(MemFile **file, const char *filename, const char *mode);

int memfclose(MemFile *file);

size_t memfread(void *buffer, size_t size, size_t n, MemFile *file);

size_t memfwrite(const void *buffer, size_t size, size_t n, MemFile *file);

int memfseek(MemFile *file, long int pos, int mode);

long int memftell(MemFile *file);
void memrewind(MemFile *file);

int memopen(const char *filename, int fd, int mode);

int memclose(int fd);

size_t mempread(int fd, void *b, size_t c, off_t *o);

size_t mempwrite(int fd, void *b, size_t c, off_t *o);

char *memgetcwd(void *fs, char *f, size_t l);

int memrmdir(void *fs, const char *p);

int memmkdir(void *fs, const char *p, mode_t m);

int memremove(void *fs, const char *p);

int memrename(void *fs, const char *p, const char *np);

// int memstat(const char *p, stat_t *b);

// int memlstat(const char *p, stat_t *b);

// int memwchmod(void *fs, const char *p, mode_t m);

MemDir *memopendir(const char *name);
int memclosedir(MemDir *dirp);
struct dirent *memreaddir(MemDir *dirp);
void memrewinddir(MemDir *dirp);