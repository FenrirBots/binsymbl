#ifndef __INC_FILE_H
#define __INC_FILE_H
#include <stdint.h>

struct file
{
  void    *file;
  char    *filename;
  uint32_t length;

  /* Specific to mapping memory, null if unused. */
  void *memmap;
  void *memptr;

};

_Bool
file_open(
  struct file *f);

_Bool
file_close(
  struct file *f);

uint32_t
file_length(
  struct file *f);

_Bool
filemap_create(
  struct file *f);

_Bool
filemap_destroy(
  struct file *f);
#endif