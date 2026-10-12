#ifndef GEOREPORT_MEMMEM_COMPAT_H
#define GEOREPORT_MEMMEM_COMPAT_H

/*
 * Старые SDK macOS могут не объявлять GNU/BSD-функцию memmem().
 * Для них перенаправляем вызовы LibreDWG на локальную реализацию.
 * На современных SDK используем системную реализацию, чтобы избежать
 * конфликта с объявлением memmem из string.h.
 */
#include <Availability.h>
#include <stddef.h>
#include <string.h>

#if defined(__APPLE__) && defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && \
    (__MAC_OS_X_VERSION_MAX_ALLOWED < 130000)
static inline void* georeport_memmem(
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
#define memmem georeport_memmem
#endif

#endif
