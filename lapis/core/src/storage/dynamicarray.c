#include "dynamicarray.h"

dynamic_array *dynamic_array_init(size_t capacity, size_t element_size)
{
  dynamic_array *array = (dynamic_array *)malloc(sizeof(dynamic_array));
  if (array == NULL)
  {
    return NULL;
  }

  array->size = 0;
  array->capacity = capacity;
  array->element_size = element_size;
  array->data = (void *)malloc(capacity * element_size);
  if (array->data == NULL)
  {
    free(array);
    return NULL;
  }

  return array;
}

void dynamic_array_free(dynamic_array *array)
{
  free(array->data);
  free(array);
}

short dynamic_array_push(dynamic_array *array, void *element)
{
  if (array->size == array->capacity)
  {
    array->capacity *= 2;
    array->data = (void *)realloc(array->data, array->capacity * array->element_size);
  }

  void *destination = (char *)array->data + (array->size * array->element_size);
  memcpy(destination, element, array->element_size);
  array->size++;
  return 0;
}

void *dynamic_array_get(dynamic_array *array, size_t index)
{
  if (index >= array->size)
  {
    return NULL;
  }

  return (char *)array->data + (index * array->element_size);
}

void *dynamic_array_get_copy(dynamic_array *array, size_t index)
{
  if (index >= array->size)
  {
    return NULL;
  }

  void *destination = malloc(array->element_size);
  if (destination == NULL)
  {
    return NULL;
  }

  memcpy(destination, (char *)array->data + (index * array->element_size), array->element_size);
  return destination;
}

void *dynamic_array_set(dynamic_array *array, size_t index, void *element)
{
  if (index >= array->size)
  {
    return NULL;
  }

  void *destination = (char *)array->data + (index * array->element_size);
  memcpy(destination, element, array->element_size);
  return destination;
}

size_t dynamic_array_size(dynamic_array *array)
{
  return array->size;
}

size_t dynamic_array_capacity(dynamic_array *array)
{
  return array->capacity;
}

void dynamic_array_resize(dynamic_array *array, size_t new_capacity)
{
  array->data = (void **)realloc(array->data, new_capacity * sizeof(void *));
  array->capacity = new_capacity;
}

dynamic_array *dynamic_array_copy(dynamic_array *src)
{
  dynamic_array *dest = dynamic_array_init(src->capacity, src->element_size);
  if (dest == NULL)
  {
    return NULL;
  }

  // Copy raw memory from source to destination
  memcpy(dest->data, src->data, src->size * src->element_size);
  dest->size = src->size;

  return dest;
}
