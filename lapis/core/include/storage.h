
#ifndef LAPIS_CORE_STORAGE_H
#define LAPIS_CORE_STORAGE_H

#include <stdio.h>

#include "dynamicarray.h"
#include "memmap.h"
#include "logger.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#define LP_TRUE 1
#define LP_FALSE 0

typedef char lp_bool;
typedef char lp_char;
typedef char *lp_string;
typedef int lp_int;
typedef float lp_float;
typedef double lp_double;
typedef long lp_long;
typedef short lp_short;
typedef long long lp_longlong;
typedef size_t lp_size_t;
typedef unsigned long lp_ulong;
typedef unsigned int lp_uint;
typedef unsigned char lp_uchar;
typedef unsigned short lp_ushort;
typedef unsigned long long lp_ulonglong;

/* Storage type */
typedef enum lp_storage_type
{
  MMAPPED,
  IN_MEMORY
} lp_storage_type;

/* Data types */
typedef enum lp_dtype
{
  LP_INT,
  LP_FLOAT,
  LP_STRING
} lp_dtype;

/* Shape of the data */
typedef struct shape
{
  lp_size_t rows;
  lp_size_t cols;
} lp_shape;

/* Generalized field */
typedef struct field_t
{
  lp_char *buffer;
  lp_dtype dtype;
  lp_bool quoted;
  lp_size_t start;
  lp_size_t end;
  lp_size_t _mem_used;
} lp_field_t;

/* Memory mapped field */
// typedef struct
// {
//     size_t start;
//     size_t end;
//     lp_bool quoted;
//     lp_dtype dtype;
// } lp_ffield_t;

/* In-memory field */
// typedef struct mfield_t
// {
//     char *buffer;
//     lp_bool quoted;
//     lp_dtype dtype;
// } lp_mfield_t;

/* Memory mapped handle */
typedef struct mmapped_t
{
  char *buffer;
#ifdef _WIN32
  DWORD buffer_size;
#else
  size_t buffer_size;
#endif
} lp_mmapped_t;

/* In-memory handle, for 1D and 2D */
// typedef struct in_memory_t
// {
//     lp_shape _capacity;
//     size_t _mem_used;
//     lp_bool is_2d;
// } lp_in_memory_t;

// Memory mapped fields for 1D, 2D
typedef union mmapped
{
  // lp_ffield_t *fields;
  // lp_ffield_t **cols;
  dynamic_array *fields;
  dynamic_array **cols;
} lp_mmapped_data;

// In-memory fields for 1D, 2D
typedef union in_memory
{
  // lp_mfield_t *fields;
  // lp_mfield_t **cols;
  dynamic_array *fields;
  dynamic_array **cols;
} lp_in_memory_data;

/* Storage handle, every structure frame, series should use this to maintain storage */
typedef struct storage_t
{
  enum lp_storage_type type;
  size_t size;
  size_t capacity;
  lp_shape _shape;
  size_t _mem_used;

  union
  {
    lp_mmapped_t *mmapped; /* Memory mapped handle */
                           // lp_in_memory_t *in_memory; /* In-memory handle */
  } handle;

  /* Data structures for storing actual data or references for 1D, 2D (column storage) */
  union
  {
    // lp_mmapped_data *mmapped;     /* Memory mapped fields */
    // lp_in_memory_data *in_memory; /* In-memory fields */
    dynamic_array *fields;
    dynamic_array **cols;
  } data;

  /* Column names */
  lp_string *col_names;

  void (*lp_alloc)(struct storage_t *s, size_t size);
  void (*lp_free)(struct storage_t *s);
  void (*lp_resize)(struct storage_t *s, size_t size);
  void (*lp_mem_used)(struct storage_t *s);

  /* Reference counting */
  size_t ref_count;
} lp_storage_t;

/* Storage functions */
lp_storage_t *lp_storage_init(lp_storage_type type, lp_shape shape, lp_string *col_names);
void lp_storage_free(lp_storage_t *storage);
lp_bool lp_storage_is_mmapped(lp_storage_t *storage);
lp_bool lp_storage_is_in_memory(lp_storage_t *storage);
lp_bool lp_storage_is_2d(lp_storage_t *storage);
lp_shape lp_storage_get_shape(lp_storage_t *storage);
lp_shape lp_storage_get_capacity(lp_storage_t *storage);
size_t lp_storage_get_mem_used(lp_storage_t *storage);
lp_bool lp_storage_set_2d(lp_storage_t *storage, lp_bool is_2d);
void lp_storage_set_shape(lp_storage_t *storage, lp_shape shape);
lp_bool lp_storage_set_capacity(lp_storage_t *storage, lp_shape capacity);
lp_bool lp_storage_set_mem_used(lp_storage_t *storage, size_t mem_used);
lp_bool lp_storage_inc_mem_used(lp_storage_t *storage, size_t mem_used);
lp_bool lp_storage_dec_mem_used(lp_storage_t *storage, size_t mem_used);
lp_size_t lp_storage_get_col_index(lp_storage_t *storage, const char *col_name);
lp_string lp_storage_get_value_as_string(lp_storage_t *storage, size_t row, size_t col);

/* Allocator functions */
void lp_alloc(lp_storage_t *s, size_t size);
void lp_free(lp_storage_t *s);
void lp_resize(lp_storage_t *s, size_t size);
void lp_mem_used(lp_storage_t *s);

#endif // LAPIS_CORE_STORAGE_H