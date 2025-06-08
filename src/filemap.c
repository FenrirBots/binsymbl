#include "file.h"

#include <stdint.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


_Bool
filemap_create(
  struct file *f)
{
  if (NULL == f ||
      NULL == f->file)
    return FALSE;
  
  if (NULL != f->memmap ||
      NULL != f->memptr)
    return FALSE;

  f->memmap = CreateFileMappingA(
    f->file,
    NULL,
    PAGE_READWRITE,
    0, 0, NULL
  );
  
  if (NULL == f->memmap)
    return FALSE;

  f->memptr = MapViewOfFile(
    f->memmap,
    FILE_MAP_READ | FILE_MAP_WRITE,
    0, 0, 0
  );
  
  if (NULL == f->memptr)
    CloseHandle(f->memmap);
  
  return TRUE;
}

_Bool
filemap_destroy(
  struct file *f)
{
  if (NULL == f)
    return FALSE;

  if (NULL != f->memptr)
    UnmapViewOfFile(f->memptr);
    
  if (NULL != f->memmap)
    CloseHandle(f->memmap);

  f->memptr = NULL;
  f->memmap = NULL;
  return TRUE;
}