#include "memmap.h"

#ifdef _WIN32
char *map_file(const char *filename, DWORD *fileSize)
{
    HANDLE hFile = CreateFile(
        filename,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Could not open file (error %d)\n", GetLastError());
        return NULL;
    }

    *fileSize = GetFileSize(hFile, NULL);
    if (*fileSize == INVALID_FILE_SIZE)
    {
        printf("Could not get file size (error %d)\n", GetLastError());
        CloseHandle(hFile);
        return NULL;
    }

    HANDLE hFileMapping = CreateFileMapping(
        hFile,
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL);

    if (hFileMapping == NULL)
    {
        printf("Could not create file mapping (error %d)\n", GetLastError());
        CloseHandle(hFile);
        return NULL;
    }

    char *mappedFile = (char *)MapViewOfFile(
        hFileMapping,
        FILE_MAP_READ,
        0,
        0,
        0);

    if (mappedFile == NULL)
    {
        printf("Could not map view of file (error %d)\n", GetLastError());
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return NULL;
    }

    // Clean up handles (the mapping object and file handle can be closed without affecting the mapped view)
    CloseHandle(hFileMapping);
    CloseHandle(hFile);

    return mappedFile;
}

int unmap_file(char *mappedFile, DWORD fileSize)
{
    if (mappedFile)
    {
        return UnmapViewOfFile(mappedFile);
    }
    return 0;
}

#else

char *map_file(const char *filename, size_t *size)
{
    struct stat sb;
    char *addr;

    int fd = open(filename, O_RDONLY);

    if (fd == -1)
        return NULL;
    if (fstat(fd, &sb) == -1)
        return NULL;

    *size = sb.st_size;
    addr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED)
        return NULL;

    close(fd);

    return addr;
}

int unmap_file(char *addr, size_t size)
{
    return munmap(addr, size);
}

#endif
