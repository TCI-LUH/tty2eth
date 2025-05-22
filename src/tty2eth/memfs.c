
#include "memfs.h"
#include "utils/assert.h"
#include "storage.h"

#include <wolfssh/wolfsftp.h>
// #include "wolfssh/internal.h"

#include <string.h>
#include <errno.h>
#include <cmsis_os2.h>

FileDef fileDefinitions[MEMFS_MAX_FILES];

MemFile openFiles[MEMFS_MAX_OPEN_FILES];
MemDir openDirs[MEMFS_MAX_OPEN_DIRS];

static FileDef *memFindAllocatedFile(const char *filename)
{
    for (int i = 0; i < MEMFS_MAX_FILES; i++)
    {
        if ((filename == NULL && fileDefinitions[i].filename == NULL) || strcmp(fileDefinitions[i].filename, filename) == 0)
        {
            return &fileDefinitions[i];
        }
    }

    return NULL;
}

FileDef * memAllocateFile(const char *filename, void *start, size_t fileSize, size_t bufSize, bool saveStorage, fileUpdateEvent update)
{
    if (assert(filename[0] == '/', "cannot allocate File: file name must be start with '/': '%s'", filename))
        return false;

    FileDef *fileDef = memFindAllocatedFile(NULL);

    if (assert(fileDef, "cannot allocate File: out of file definitions: '%s'", filename))
        return NULL;

    *fileDef = (FileDef){
        .filename = filename,
        .start = start,
        .fileSize = fileSize,
        .bufSize = bufSize,
        .saveStorage = saveStorage,
        .update = update,
        .mutex = osMutexNew(NULL),
        };

    return fileDef;
}

int memfopen(MemFile **file, const char *filename, const char *mode)
{
    FileDef *fileDef = memFindAllocatedFile(filename);
    if (fileDef == NULL)
        return false;

    for (int i = 0; i < MEMFS_MAX_OPEN_FILES; i++)
    {
        if (openFiles[i].fileDef == NULL)
        {
            *file = &openFiles[i];
            **file = (MemFile){
                .fileDef = fileDef,
                .fd = i};
            return true;
        }
    }

    return false;
}

int memfclose(MemFile *file)
{
    if(file->hasWritten)
    {
        osMutexAcquire(file->fileDef->mutex, 0);

        if(file->fileDef->filename != NULL && file->fileDef->saveStorage)
            saveStorage();
        if(file->fileDef->update)
            file->fileDef->update(file->fileDef);
        osMutexRelease(file->fileDef->mutex);
    }

    *file = (MemFile){0};
    return 0;
}

size_t memfpread(void *buffer, size_t size, size_t n, MemFile *file, off_t *offset)
{
    osMutexAcquire(file->fileDef->mutex, 0);

    size_t available = file->fileDef->fileSize - *offset;
    size_t s = size * n;
    if (s > available)
        s = available;
    memcpy(buffer, file->fileDef->start + *offset, s);
    *offset += s;

    osMutexRelease(file->fileDef->mutex);
    return s;
}

size_t memfread(void *buffer, size_t size, size_t n, MemFile *file)
{
    return memfpread(buffer, size, n, file, &file->pos) / size;
}

size_t memfpwrite(const void *buffer, size_t size, size_t n, MemFile *file, off_t *offset)
{
    osMutexAcquire(file->fileDef->mutex, 0);

    size_t available = file->fileDef->bufSize - *offset;
    size_t s = size * n;
    if (s > available)
    {
        file->hasWritten = false;
        file->error = ENOSPC;

        osMutexRelease(file->fileDef->mutex);
        return 0;
    }
    memcpy(file->fileDef->start + *offset, buffer, s);
    buffer += s;
    *offset += s;
    if(s > 0)
        file->hasWritten = true;
    if (*offset > file->fileDef->fileSize)
        file->fileDef->fileSize = *offset;

    osMutexRelease(file->fileDef->mutex);
    return s;
}
size_t memfwrite(const void *buffer, size_t size, size_t n, MemFile *file)
{
    return memfpwrite(buffer, size, n, file, &file->pos) / size;
}

int memfseek(MemFile *file, long int pos, int mode)
{
    if (mode == SEEK_CUR)
        pos += file->pos;

    if (pos > file->fileDef->fileSize)
        return -1;

    if (mode == SEEK_END)
        pos = file->fileDef->fileSize - pos;

    file->pos = pos;
    return 0;
}

long int memftell(MemFile *file)
{
    return file->pos;
}

void memrewind(MemFile *file)
{
    file->pos = 0;
}

int memopen(const char *filename, int fd, int mode)
{
    MemFile *file = NULL;
    int ret = memfopen(&file, filename, "");
    if (ret == 0 || file == NULL)
    {
        errno = EACCES;
        return -1;
    }

    return file->fd;
}

int memclose(int fd)
{
    if (fd < 0 || fd >= MEMFS_MAX_OPEN_FILES || openFiles[fd].fileDef == NULL)
    {
        errno = EBADF;
        return -1;
    }

    return memfclose(&openFiles[fd]);
}

size_t mempread(int fd, void *buffer, size_t count, off_t *offset)
{
    if (fd < 0 || fd >= MEMFS_MAX_OPEN_FILES || openFiles[fd].fileDef == NULL)
    {
        errno = EBADF;
        return -1;
    }
    off_t off = *offset;
    size_t ret = memfpread(buffer, count, 1, &openFiles[fd], &off);
    if (ret != count)
        errno = openFiles[fd].error;
    return ret;
}

size_t mempwrite(int fd, void *buffer, size_t count, off_t *offset)
{
    if (fd < 0 || fd >= MEMFS_MAX_OPEN_FILES || openFiles[fd].fileDef == NULL)
    {
        errno = EBADF;
        return -1;
    }
    off_t off = *offset;
    size_t ret = memfpwrite(buffer, count, 1, &openFiles[fd], &off);
    if (ret != count)
        errno = openFiles[fd].error;
    return ret;
}

char *memgetcwd(void *fs, char *buf, size_t len)
{
    if (len < 2)
    {
        errno = ERANGE;
        return NULL;
    }
    if (buf == NULL)
    {
        errno = EINVAL;
        return NULL;
    }

    buf[0] = '/';
    buf[1] = 0;
    return buf;
}

int memrmdir(void *fs, const char *p)
{
    errno = EACCES;
    return -1;
}

int memmkdir(void *fs, const char *p, mode_t m)
{
    errno = EACCES;
    return -1;
}

int memremove(void *fs, const char *p)
{
    errno = EACCES;
    return -1;
}

int memrename(void *fs, const char *p, const char *np)
{
    errno = EACCES;
    return -1;
}

// int memstat(const char *p, stat_t *b)
// {
//     return 0;
// }

// int memlstat(const char *p, stat_t *b)
// {
//     return 0;
// }

// int memchmod(void *fs, const char *path, mode_t mode)
// {
//     errno = EACCES;
//     return -1;
// }

int SFTP_GetAttributes(void *fs, const char *fileName,
                       WS_SFTP_FILEATRB *atr, byte noFollow, void *heap)
{
    *atr = (WS_SFTP_FILEATRB){0};

    FileDef *fileDef = memFindAllocatedFile(fileName);
    if (fileDef == NULL)
        return WS_BAD_FILE_E;

    atr->flags |= WOLFSSH_FILEATRB_SIZE | WOLFSSH_FILEATRB_PERM;

    atr->per = FILEATRB_PER_MASK_PERM | FILEATRB_PER_FILE;

    atr->sz[0] = (word32)(fileDef->fileSize & 0xFFFFFFFF);
#if SIZEOF_OFF_T == 8
    atr->sz[1] = (word32)((fileDef->fileSize >> 32) & 0xFFFFFFFF);
#endif

    return 0;
}
int SFTP_GetAttributes_Handle(WOLFSSH *ssh, byte *handle, int handleSz,
                              WS_SFTP_FILEATRB *atr)
{
    *atr = (WS_SFTP_FILEATRB){0};
    return 0;
}

// int wolfSSH_SFTP_SetDefaultPath(WOLFSSH* ssh, const char* path)
// {
//     if(strcmp(path, "/") != 0)
//         return WS_ERROR;
//     ssh->sftpDefaultPath = "/";
//     return WS_SUCCESS;
// }


MemDir *memopendir(const char *name)
{
    if(strcmp(name, "/") != 0)
    {
        errno = EACCES;
        return NULL;
    }

    for (int i = 0; i < MEMFS_MAX_OPEN_DIRS; i++)
    {
        if (openDirs[i].name == NULL)
        {
            MemDir* ret = &openDirs[i];
            *ret = (MemDir){
                .name = "/",
                .fileIdx = 0,
                .fd = i
                };
            return ret;
        }
    }
    return NULL;
}

int memclosedir(MemDir *dirp)
{
    *dirp = (MemDir){0};
    return 0;
}

struct dirent *memreaddir(MemDir *dirp)
{
    if(dirp->fileIdx > MEMFS_MAX_FILES)
        return NULL;
    FileDef* fileDef = &fileDefinitions[dirp->fileIdx];
    if(fileDef->filename == NULL)
        return NULL;
    
    dirp->fileIdx += 1;

    dirp->entry = (struct dirent){
        .d_ino = dirp->fileIdx,
        .d_off = 0,
        .d_reclen = sizeof(struct dirent),
        .d_type = 8 /*DT_REG*/,
    };
    strcpy(dirp->entry.d_name, fileDef->filename + 1); // skip first '/' in filename

    return &dirp->entry;
}

void memrewinddir(MemDir *dirp)
{
    dirp->fileIdx = 0;
}