#include "fs.h"

typedef struct {
	char *name;
	size_t size;
	off_t disk_offset;
	off_t open_offset;
} Finfo;

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  {"/dev/fb", 0, 0},
  {"/dev/events", 0, 0},
  {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))
enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

void init_fs() {
  file_table[FD_FB].size = _screen.width * _screen.height * sizeof(uint32_t);
}

size_t fs_filesz(int fd) {
  return file_table[fd].size;
}

void ramdisk_read(void *, uint32_t, uint32_t);
void ramdisk_write(const void *, uint32_t, uint32_t);
size_t events_read(void *buf, size_t len);
void dispinfo_read(void *buf, off_t offset, size_t len);
void fb_write(const void *buf, off_t offset, size_t len);

int fs_open(const char *pathname, int flags, int mode) {
	int i;
	for (i = 0; i < NR_FILES; i ++) {
		if (strcmp(file_table[i].name, pathname) == 0) {
			file_table[i].open_offset = 0;
			return i;
		}
	}

	panic("No such file: %s", pathname);
	return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
	assert(fd > 2);
  if (fd == FD_EVENTS) {
    return events_read(buf, len);
  }

  Finfo *f = file_table + fd;
	int remain_bytes = f->size - f->open_offset;
	int bytes_to_read = (remain_bytes > len ? len : remain_bytes);

  if (fd == FD_DISPINFO) {
    dispinfo_read(buf, f->disk_offset + f->open_offset, bytes_to_read);
  }
  else {
    ramdisk_read(buf, f->disk_offset + f->open_offset, bytes_to_read);
  }
	f->open_offset += bytes_to_read;
	return bytes_to_read;
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
  Finfo *f = file_table + fd;
	int remain_bytes = f->size - f->open_offset;
	int bytes_to_write = (remain_bytes > len ? len : remain_bytes);
  switch (fd) {
    case FD_STDOUT:
    case FD_STDERR:
      for(int i = 0; i < len; i ++) {
        _putc( ((char *)buf)[i] );
      }
    case FD_EVENTS:
      return len;

    case FD_FB:
      fb_write(buf, f->open_offset, bytes_to_write);
      break;

    default:
      ramdisk_write(buf, f->disk_offset + f->open_offset, bytes_to_write);
      break;
  }

	f->open_offset += bytes_to_write;

	return bytes_to_write;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  Finfo *f = file_table + fd;
	int new_offset = f->open_offset;
	int file_size = file_table[fd].size;
	switch(whence) {
		case SEEK_CUR: new_offset += offset; break;
		case SEEK_SET: new_offset = offset; break;
		case SEEK_END: new_offset = file_size + offset; break;
		default: assert(0);
	}
	if(new_offset < 0) {
		new_offset = 0;
	}
	else if(new_offset > file_size) {
		new_offset = file_size;
	}
	f->open_offset = new_offset;

	return new_offset;
}

int fs_close(int fd) {
	return 0;
}
