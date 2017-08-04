#ifndef __FS_H__
#define __FS_H__

#include "common.h"

enum {SEEK_SET, SEEK_CUR, SEEK_END};

int fs_open(const char *, int, int);
ssize_t fs_read(int, void *, size_t);
ssize_t fs_write(int, const void *, size_t);
off_t fs_lseek(int, off_t, int);
int fs_close(int);

size_t fs_filesz(int);

#endif
