#include "file.h"

#include <stdint.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


/*
  TODO: Convert this to use fopen instead of CreateFileA 
    and convert FILE* to HANDLE using _get_osfhandle(...)
      
  This allows for the code to be compiled on linux more easily.
*/
_Bool
file_open(
  struct file *f)
{
  if (NULL == f           ||
      NULL == f->filename ||
      NULL != f->file)
    return FALSE;

  f->file = CreateFileA(
    f->filename,
    GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL,
    OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
    NULL
  );
  
  if (NULL == f->file)
    return FALSE;

  f->length = file_length(f);
  return TRUE;
}

_Bool
file_close(
  struct file *f)
{
  if (NULL == f)
    return FALSE;
  
  if (NULL != f->memmap ||
      NULL != f->memptr)
    filemap_destroy(f);

  if (NULL != f->file)
    CloseHandle(f->file);

  f->file = NULL;
  return TRUE;
}

uint32_t
file_length(
  struct file *f)
{
  if (NULL == f ||
      NULL == f->file)
    return 0;

  f->length = GetFileSize(f->file, NULL);

  if (INVALID_FILE_SIZE == f->length)
    f->length = 0;

  return f->length;
}