#pragma once

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#ifdef _WIN32
char *map_file(const char *filename, DWORD *fileSize);
int unmap_file(char *mappedFile, DWORD fileSize);
#else
char *map_file(const char *filename, size_t *size);
int unmap_file(char *addr, size_t size);
#endif
