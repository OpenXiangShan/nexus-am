#include "fs.h"

typedef struct {
	char *name;
	size_t size;
	off_t disk_offset;
} file_info;

typedef struct {
	bool used;
	uint32_t index;
	off_t offset;
} Fstate;

extern uint32_t* const fb;

/* This is the information about all files in disk. */
static const file_info file_table[] __attribute__((used)) = {
  {"/dev/fb", 320 * 200 * sizeof(*fb), 0},
  {"/dev/events", 0, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))
enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_NORMAL};

Fstate files[FD_NORMAL + NR_FILES];

void ramdisk_read(void *, uint32_t, uint32_t);
void ramdisk_write(const void *, uint32_t, uint32_t);
size_t read_events(void *buf);

int fs_open(const char *pathname, int flags, int mode) {
	int i, fd;
	for (i = 0; i < NR_FILES; i ++) {
		if (strcmp(file_table[i].name, pathname) == 0) {
      switch (i) {
        case 0: fd = FD_FB; break;
        case 1: fd = FD_EVENTS; break;
        default: fd = FD_NORMAL + i; break;
      }
			assert(files[fd].used == false);
			files[fd].used = true;
			files[fd].index = i;
			files[fd].offset = 0;
			return fd;
		}
	}

	panic("No such file: %s", pathname);
	return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
	assert(fd > 2);
  if (fd == FD_EVENTS) {
    return read_events(buf);
  }

	int remain_bytes = file_table[ files[fd].index ].size - files[fd].offset;
	int bytes_to_read = (remain_bytes > len ? len : remain_bytes);

	ramdisk_read(buf, file_table[files[fd].index].disk_offset + files[fd].offset, bytes_to_read);
	files[fd].offset += bytes_to_read;
	return bytes_to_read;
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
	int remain_bytes = file_table[ files[fd].index ].size - files[fd].offset;
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
      memcpy((void *)fb + files[fd].offset, buf, bytes_to_write);
      break;

    default:
      ramdisk_write(buf, file_table[files[fd].index].disk_offset + files[fd].offset, bytes_to_write);
      break;
  }

	files[fd].offset += bytes_to_write;

	return bytes_to_write;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
	int new_offset = files[fd].offset;
	int file_size = file_table[files[fd].index].size;
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
	files[fd].offset = new_offset;

	return new_offset;
}

int fs_close(int fd) {
	if(fd >= 0 && fd <= 2) return 0;

	assert(files[fd].used == true);
	files[fd].used = false;
	return 0;
}
