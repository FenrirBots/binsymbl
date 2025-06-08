#include "file.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdint.h>

/*
  I do not know of any "Official" versioning
    for this structure nor a way to detect it,
    instead this is simply used to know the size
    of the structure when iterating it.
*/
enum addressStructureVersion
{
  addressStructureVersion1 = 0x08,
  addressStructureVersion2 = 0x0C,
};

void
print_help()
{
  printf("---- bin2symbl Help Information ----\n");
  printf("bin2symbl --input=<filename>                    --output=<filename>\n");
  printf("bin2symbl --addrs=<filename> --names=<filename> --output=<filename>\n\n");


  printf("--input=<filename>  : A file containg both name and address information\n");
  printf("--output=<filename> : The output file\n");
  printf("---- bin2symbl Help Information ----\n");
}

typedef struct Object
{
  uint32_t rva;
  uint32_t size;
  
  uint32_t length;
  char    *name;
} Object;

/*
  Checks to see if there is a module section.

  NOTE: the modules list can expand infinitely with
          no real indicator of its existence, as a result
          this is not a guaranteed to be 100% accurate.

  @param f - A pointer to the start of a valid bin file
*/
_Bool
bin_sections_module_exists(
  void *f)
{
  if (NULL == f)
    return FALSE;

  if ((*((uint32_t *)(f + 0x08)) > *((uint32_t *)(f)) &&
       *((uint32_t *)(f + 0x08)) - *((uint32_t *)(f)) > 0x100000))
    return TRUE;


/*
  This method will not work as intended as the
    9th byte leads into the length value which
    has a chance to be a valid ascii character.

  In such cases this will return the wrong value.


  if (*((char *)(f + 0x09)) >= 0x41 &&
      *((char *)(f + 0x09)) <= 0x5A)
    return TRUE;

  if (*((char *)(f + 0x09)) >= 0x61 &&
      *((char *)(f + 0x09)) <= 0x7A)
    return TRUE;
*/

  return FALSE;
}

uint32_t
bin_sections_module_count(
  void *f)
{
  if (NULL == f)
    return 0;
    
  return *((uint32_t *)(f));
}

uint32_t
bin_sections_module_length(
  void *f)
{
  uint32_t iter    = 0;
  uint32_t offset  = 0;
  uint32_t entries = 0;

  if (NULL == f)
    return 0;
  
  entries = bin_sections_module_count(f);
  offset += 4;

  for (iter = 0; iter < entries; iter++)
  {
    offset += *((uint32_t *)(f + offset));
    offset += 0x0C;
  }

  return offset;
}

void *
bin_sections_module_end(
  void *f)
{
  if (NULL == f)
    return 0;

  return f + bin_sections_module_length(f);
}

uint32_t
bin_sections_address_count(
  void    *f)
{
  if (NULL == f)
    return 0;
  
  return *((uint32_t *)(f));
}

uint32_t
bin_sections_address_length(
  void    *f,
  uint32_t v)
{
  if (NULL == f ||
      0x00 == v)
    return 0;

  return *((uint32_t *)(f)) * v + 4;
}

void *
bin_sections_address_end(
  void    *f,
  uint32_t v)
{
  if (NULL == f)
    return 0;

  return ((void *)(f + bin_sections_address_length(f, v)));
}

_Bool
bin_sections_names_exists(
  void *f)
{
  /*
    TODO: However the fuck i do this, TODO it
  */
  return TRUE;
}

uint32_t
bin_sections_names_count(
  void *f)
{
  return *((uint32_t *)(f));
}

uint32_t
bin_sections_names_length(
  void *f)
{
  uint32_t iter    = 0;
  uint32_t incr    = 4;
  uint32_t offset  = 4;
  uint32_t entries = 0;

  /*
    TODO: Add support for the names section 
            shipped with fortnite 2018 and 2019
  */
  entries = bin_sections_names_count(f);

  for (iter = 0; iter < entries; iter++)
  {
    offset += *((uint32_t *)(f + offset));
    offset += incr;
  }

  return offset;
}

int
main(
  int    argc,
  char **argv)
{
  struct file file   = { 0 };
  struct file write  = { 0 };

  uint32_t iter    = 0x00;
  uint32_t offset  = 0x00;
  uint32_t entries = 0x00;
  _Bool    modules = FALSE;
  void    *addrs   = NULL;
  void    *names   = NULL;
  Object  *object  = NULL;
  uint32_t size    = 0x00;
  char     buffer[10000];

  if (argc < 3)
  {
    print_help();
    goto cleanup;
  }

  for (iter = 0; iter < argc; iter++)
  {
    if (0 == strnicmp(argv[iter], "--help", 6))
    {
      print_help();
      goto cleanup;
    }

    if (0 == strnicmp(argv[iter], "--input=", 8))
    {
      file.filename = argv[iter] + 0x08;
      file_open(&file);
      filemap_create(&file);
      continue;
    }

    if (0 == strnicmp(argv[iter], "--output=", 9))
    {
      write.filename = argv[iter] + 0x09;
      file_open(&write);
      continue;
    }
  }

parse:
  if (NULL == file.file ||
      NULL == write.file)
  {
    printf("Error opening the file.\n");
    goto cleanup;
  }

  modules = bin_sections_module_exists(file.memptr);
  addrs   = file.memptr;

  if (TRUE == modules)
    addrs = bin_sections_module_end(file.memptr);

  names   = bin_sections_address_end(addrs, (TRUE == modules ? addressStructureVersion2 : addressStructureVersion1));
  entries = *((uint32_t *)(addrs));
  object  = calloc(entries, sizeof(*object));
  addrs  += 0x04;
  names  += 0x04;


  for (iter = 0; iter < entries; iter++)
  {
    object[iter].rva    = *((uint32_t *)(addrs));
    object[iter].size   = *((uint32_t *)(addrs + 0x04));
    object[iter].length = *((uint32_t *)(names));
    object[iter].name   = names + 0x04;

    addrs += 0x08;
    if (TRUE == modules)
      addrs += 0x04;

    names += *((uint32_t *)(names));
    names += 0x04;
  }

  for (iter = 0; iter < entries; iter++)
  {
    // printf("Addr: 0x%.8x - %s\n", object[iter].rva, object[iter].name);
    size = sprintf(buffer, "0x%.8x - %s\n", object[iter].rva, object[iter].name);
    WriteFile(write.file, buffer, size, NULL, NULL);

    // printf("Addr   : 0x%.8x\n", *((uint32_t *)(addrs)));
    
    // addrs += 0x08;
    // if (TRUE == modules)
    //   addrs += 0x04;

    // printf("Name   : %s\n", (char *)(names + 0x04));
    // printf("Names: 0x%.8x\n", *((uint32_t *)(names)));
    // names += *((uint32_t *)(names));
    // names += 0x04;

    // printf("Names: 0x%.8x\n", names - file.memptr);
    // printf("Addrs: 0x%.8x\n", addrs - file.memptr);
  }

cleanup:
  if (NULL != file.file)
    file_close(&file);

  if (NULL != write.file)
    file_close(&write);

end:
  return 0;
}

// struct Object
// {
//   uint32_t rva;
//   uint32_t length;
// };

// struct ObjectName
// {
//   uint32_t length;
//   uint8_t *data;
// };


// void
// get_names(
//   struct ObjectName *names,
//   void              *memory,
//   uint32_t           count)
// {
//   uint32_t iter = 0;

//   if (NULL == names ||
//       NULL == memory ||
//       0    == count)
//     return;
  
//   for (iter = 0; iter < count; iter++)
//   {
//     names[iter].length = *((uint32_t *)(memory));
//     names[iter].data   = memory + 4;

//     memory += 4;
//     memory += names[iter].length;
//   }
// }

// int
// main(int    argc,
//      char **argv)
// {
//   struct file in  = { 0 };
//   struct file out = { 0 };

//   struct Object     *address = NULL;
//   struct ObjectName *names   = NULL;
//   uint32_t           entries = 0;
//   uint32_t           iter    = 0;

//   char     buffer[1024];
//   uint32_t size = 0;

//   if (argc < 3)
//   {
//     printf("%s <filename> <output>", argv[0]);
//     return 0;
//   }

//   file_open(&out, (uint8_t *)argv[2]);
//   file_open(&in , (uint8_t *)argv[1]);
//   filemap_create(&in);

//   if (NULL == in.file ||
//       NULL == in.memptr)
//     goto cleanup;

//   if (NULL == out.file)
//     goto cleanup;

//   entries = *((uint32_t *)(in.memptr));
//   address = in.memptr + 4;
//   names   = calloc(entries, sizeof(*names));

//   if (names == 0)
//     goto cleanup;

//   get_names(names, in.memptr + 0x013AC82C + 0x04, entries);

//   for (iter = 0; iter < entries; iter++)
//   {
//     size = sprintf(buffer, "0x%x - %s\n", address[iter].rva, names[iter].data);
//     WriteFile(out.file, buffer, size, NULL, NULL);
//   }
  
// cleanup:
//   if (NULL != names)
//     free(names);

// q  if (NULL != in.file)
//     file_close(&in);

//   if (NULL != out.file)
//     file_close(&out);

//   return 0;
// }