#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <assert.h>
#include "util/memory.h"
#include "util/string.h"

/* ----------------------------------------------------------------------------
 * Static functions
 * ------------------------------------------------------------------------- */
static uintptr_t
safe_strlen(const char* str)
{
    if(str)
        return strlen(str);
    return 0;
}

/* ------------------------------------------------------------------------- */
static void
safe_strcat(char* target, const char* source)
{
    assert(target);
    if(source)
        strcat(target, source);
}

/* ------------------------------------------------------------------------- */
static uintptr_t
safe_wcslen(const wchar_t* wcs)
{
    if(wcs)
        return wcslen(wcs);
    return 0;
}

/* ------------------------------------------------------------------------- */
static void
safe_wcscat(wchar_t* target, const wchar_t* source)
{
    assert(target);
    if(source)
        wcscat(target, source);
}

/* ----------------------------------------------------------------------------
 * Exported functions
 * ------------------------------------------------------------------------- */
void
free_string(void* ptr)
{
    assert(ptr);
    FREE(ptr);
}

/* ------------------------------------------------------------------------- */
char*
cat_strings(uint32_t num_strs, ...)
{
    uint32_t total_length = 0;
    uint32_t i;
    char* buffer;
    va_list ap;

    /* compute total length of all strings combined and allocate a buffer able
     * to contain all strings plus a null terminator */
    va_start(ap, num_strs);
    for(i = 0; i != num_strs; ++i)
        total_length += safe_strlen(va_arg(ap, char*));
    va_end(ap);

    buffer = (char*)MALLOC((total_length+1) * sizeof(char), "cat_strings()");
    if(!buffer)
    {
        fprintf(stderr, "malloc() failed in cat_strings() -- not enough memory\n");
        return NULL;
    }
    *buffer = '\0'; /* so strcat works */

    /* concatenate all strings into the allocated buffer */
    va_start(ap, num_strs);
    for(i = 0; i != num_strs; ++i)
        safe_strcat(buffer, va_arg(ap, char*));
    va_end(ap);

    return buffer;
}

/* ------------------------------------------------------------------------- */
char*
malloc_string(const char* str)
{
    char* buffer;
    assert(str);

    buffer = (char*)MALLOC((strlen(str)+1) * sizeof(char), "malloc_string()");
    if(!buffer)
    {
        fprintf(stderr, "malloc() failed in malloc_string() -- not enough memory\n");
        return NULL;
    }

    strcpy(buffer, str);
    return buffer;
}

/* ------------------------------------------------------------------------- */
wchar_t*
cat_wstrings(uint32_t num_strs, ...)
{
    uint32_t total_length = 0;
    uint32_t i;
    wchar_t* buffer;
    va_list ap;

    /* compute total length of all strings combined and allocate a buffer able
     * to contain all strings plus a null terminator */
    va_start(ap, num_strs);
    for(i = 0; i != num_strs; ++i)
        total_length += safe_wcslen(va_arg(ap, wchar_t*));
    va_end(ap);

    buffer = (wchar_t*)MALLOC((total_length+1) * sizeof(wchar_t), "cat_wstrings()");
    if(!buffer)
    {
        fprintf(stderr, "malloc() failed in cat_wstrings() -- not enough memory\n");
        return NULL;
    }
    *buffer = L'\0'; /* so wcscat works */

    /* concatenate all strings into the allocated buffer */
    va_start(ap, num_strs);
    for(i = 0; i != num_strs; ++i)
        safe_wcscat(buffer, va_arg(ap, wchar_t*));
    va_end(ap);

    return buffer;
}

/* ------------------------------------------------------------------------- */
wchar_t*
malloc_wstring(const wchar_t* wcs)
{
    wchar_t* buffer;
    assert(wcs);

    buffer = (wchar_t*)MALLOC((wcslen(wcs)+1) * sizeof(wchar_t), "malloc_wstring()");
    if(!buffer)
    {
        fprintf(stderr, "malloc() failed in malloc_wstring() -- not enough memory\n");
        return NULL;
    }

    wcscpy(buffer, wcs);
    return buffer;
}

/* ------------------------------------------------------------------------- */
char*
strtok_r_portable(char* str, char delimiter, char** saveptr)
{
    char* begin_ptr;
    char* end_ptr;

    if(str)
        *saveptr = str - 1;

    /* no more tokens */
    if(!*saveptr)
        return NULL;

    /* get first occurrence of token in string */
    begin_ptr = *saveptr + 1;
    end_ptr = (char*)strchr(begin_ptr, delimiter);
    if(!end_ptr)
        *saveptr = NULL; /* last token has been reached */
    else
    {
        /* update saveptr and replace delimiter with null terminator */
        *saveptr = end_ptr;
        **saveptr = '\0';
    }

    /* empty tokens */
    if(*begin_ptr == '\0')
        return NULL;
    return begin_ptr;
}

/* ------------------------------------------------------------------------- */
wchar_t*
strtowcs(const char* str)
{
    wchar_t* wcs;
    wchar_t* wcs_it;
    uintptr_t len;

    assert(str);

    len = strlen(str);

    wcs = (wchar_t*)MALLOC((len + 1) * sizeof(wchar_t), "strtowcs()");
    if(!wcs)
    {
        fprintf(stderr, "malloc() failed in strtowcs() -- not enough memory\n");
        return NULL;
    }

    for(wcs_it = wcs; *str; ++str)
        *wcs_it++ = (wchar_t)*str;
    *wcs_it = L'\0';
    return wcs;
}

/* ------------------------------------------------------------------------- */
char*
wcstostr(const wchar_t* wcs)
{
    char* str;
    char* str_it;
    uintptr_t len;

    assert(wcs);

    len = wcslen(wcs);

    str = (char*)MALLOC((len + 1) * sizeof(char), "wcstostr()");
    if(!str)
    {
        fprintf(stderr, "malloc() failed in wcstostr() -- not enough memory\n");
        return NULL;
    }

    for(str_it = str; *wcs; ++wcs)
        *str_it++ = (char)*wcs;
    *str_it = '\0';
    return str;
}

/* ------------------------------------------------------------------------- */
void
crlf2lf(char* src)
{
    char* target;

    assert(src);

    target = src;
    while(*src)
    {
        if(*src == '\r') /* skip any CRs */
            ++src;
        *target++ = *src++;
    }

    /* if at least one CR was skipped, a new null-terminator must be set. */
    if(target != src)
        *target = '\0';
}

/* ------------------------------------------------------------------------- */
void
string_reverse(char* str)
{
    char* end;

    assert(str);

    end = str + strlen(str) - 1;
    for(; str < end; ++str, --end)
    {
        char tmp = *str;
        *str = *end;
        *end = tmp;
    }
}

/* ------------------------------------------------------------------------- */
void
string_tolower(char* str)
{
    /*
     * This shit was taken from here.
     * http://stackoverflow.com/questions/2661766/c-convert-a-mixed-case-string-to-all-lower-case
     */
    char* p;
    for(p = str; *p; ++p)
        *p = *p > 0x40 && *p < 0x5b ? *p | 0x60 : *p;
}
