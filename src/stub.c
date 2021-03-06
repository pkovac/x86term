#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "base.h"
#include <stdarg.h>
#define __unused __attribute__((unused))
char heapbase[262144];
int abs(int i)
{
    if (i>0) return i;
    else return i*-1;
}
int fprintf(__unused FILE* stream, __unused const char* format, ...)
{
    return 0;
}
static void* heaptop = NULL;
void *malloc(size_t size)
{
    if (!heaptop) heaptop = heapbase;
    void* newmem;
    newmem = heaptop;
    if (heaptop + size > (void*)heapbase + sizeof(heapbase)) {
        kpanic("Heap overflow. Requested size:", (void*)size);
    }
    heaptop += size;
    return newmem;
}
void free(__unused void *ptr)
{
    return; /* Herp derp */
}

void* memset(void* s, int c, size_t n)
{   
    /* TODO: Optimize */
    unsigned int i;
    char* vp = (char*)s;
    for (i = 0; i < n; i++)
        vp[i] = (char)c;
    return s;
}
void *memcpy(void *dest, const void *src, size_t n)
{
    unsigned int i;
    char *cdest, *csrc;
    cdest = (char*)dest;
    csrc = (char*)src;
    for (i = 0; i < n; i++)
        cdest[i] = csrc[i];
    return dest;
}
void *memmove(void *dest, const void *src, size_t n)
{
    int i;
    char *cdest, *csrc;
    cdest = (char*)dest;
    csrc = (char*)src;
    if (dest < src) 
        for (i=0; i < (int)n; i++) cdest[i] = csrc[i];
    else if (dest > src) 
        for (i=n-1; i>=0; i--) cdest[i] = csrc[i];
    return dest;
}
char *strncpy(char *dest, const char *src, size_t n)
{
    unsigned int i;
    for (i=0; i < n; i++)
    {
        dest[i] = src[i];
        if (src[i] == '\0') break;
    }
    return dest;
}
size_t strlen(const char *s)
{
    size_t i = 0;
    while (*(s++)) i++;
    return i;
}
int strncmp(const char *s1, const char *s2, size_t n)
{
    unsigned int i;
    for (i = 0; i < n; i++)
        if ((s1[i] != s2[i]) || (s1[i] == 0)) return s1[i]-s2[i];
    return 0;
}
inline static int char_is_in(char c, const char* s)
{
    char sc;
    while ((sc = *(s++)))
        if (sc == c)  return 1;
    return 0;
}

size_t strspn(const char *s, const char *accept)
{
    char src;
    size_t len = 0;
    while ((src = *(s++))) 
    {
        if (char_is_in(src, accept))
            len++;
        else
            break;
    }   
    return len;
}

size_t strcspn(const char *s, const char *reject)
{
    char src;
    size_t len = 0;
    while ((src = *(s++)))
    {
        if (char_is_in(src, reject))
            break;
        else
            len++;
    }
    return len;
}

/* vnsprintf and support code by Duskwuff */
static char *tgt_buf;
static int tgt_remain, total_written;

static void addch(char ch)
{
    if(tgt_remain > 0) {
        *tgt_buf++ = ch;
        tgt_remain--;
    }

    total_written++;
}

static void add_dec(int n)
{
    if(n < 0) {
        addch('-');
        n = -n; // FIXME: This is broken for INT_MIN.
    }

    int digits =
        (n < 10) ? 1 :
        (n < 100) ? 2 :
        (n < 1000) ? 3 :
        (n < 10000) ? 4 :
        (n < 100000) ? 5 :
        (n < 1000000) ? 6 :
        (n < 10000000) ? 7 :
        (n < 100000000) ? 8 :
        (n < 1000000000) ? 9 : 10;

    int i;
    char tmp[10];

    for(i = digits - 1; i >= 0; i--) {
        tmp[i] = '0' + (n % 10);
        n /= 10;
    }

    for(i = 0; i < digits; i++)
        addch(tmp[i]);
}
int vsnprintf(char *tgt, size_t len, const char *fmt, va_list ap)
{
    tgt_buf = tgt;
    tgt_remain = len - 1;
    total_written = 0;

    char ch;
    while((ch = *fmt++)) {
        if(ch != '%') {
            addch(ch);
            continue;
        }

        ch = *fmt++;
        switch(ch) {
            case 'd':
                add_dec(va_arg(ap, int));
                break;

            case 'c':
                addch(va_arg(ap, int));
                break;

            case 's':
                {
                    char *str = va_arg(ap, char *);
                    while(*str)
                        addch(*str++);
                }
                break;

            case '%':
                addch('%');
                break;

            default:
                addch('%');
                addch(ch);
        }
    }

    *tgt_buf = 0;
    return total_written;
}

