#ifndef GEOREPORT_MEMMEM_COMPAT_H
#define GEOREPORT_MEMMEM_COMPAT_H

/*
 * Совместимость с macOS: GNU-функция memmem() отсутствует в стандартной
 * библиотеке macOS. Заголовок подключается CMake только при сборке LibreDWG
 * на Apple-платформах.
 */
#include <stddef.h>
#include <string.h>

#ifndef memmem
static inline void* memmem(
    const void* haystack,
    size_t haystackLength,
    const void* needle,
    size_t needleLength)
{
    if (needleLength == 0)
        return (void*)haystack;

    if (haystack == NULL || needle == NULL || needleLength > haystackLength)
        return NULL;

    const unsigned char* h = (const unsigned char*)haystack;
    const unsigned char* n = (const unsigned char*)needle;
    const size_t last = haystackLength - needleLength;

    for (size_t i = 0; i <= last; ++i)
    {
        if (h[i] == n[0] && memcmp(h + i, n, needleLength) == 0)
            return (void*)(h + i);
    }

    return NULL;
}
#endif

#endif
