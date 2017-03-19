#ifndef __KLIB_H__
#define __KLIB_H__

#include <common.h>

__H_BEGIN__

// printk
void printk(const char *fmt, ...);

// memory allocation
void* kalloc(size_t);
void kfree(void*);

#define assert(cond) \
  do { \
    if (!(cond)) { \
      printk("Assertion fail: %s:%d\n", __FILE__, __LINE__); \
      _panic(1); \
    } \
  } while (0)

__H_END__

// TODO: move adts out of this header
#ifdef __cplusplus
namespace adt {

template <typename T>
class vector {
  int _size;
  T _data[16];

public:
  int size() {
    return this->_size;
  }
  vector() {
    _size = 0;
  }

  void push_back(T x) {
    _data[_size++] = x;
  }

  void sort(int (*cmp)(const T&, const T&)) {
    for (int i = 0; i < _size; i ++)
      for (int j = i + 1; j < _size; j ++)
        if (cmp(_data[i], _data[j]) > 0) {
          T tmp = _data[i];
          _data[i] = _data[j];
          _data[j] = tmp;
        }
  }

  T& operator [](int idx) {
    return _data[idx];
  }

  T* begin() { return _data; }
  T* end() { return _data + _size; }
};

}

#endif



#endif
