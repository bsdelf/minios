#ifndef STAND_H
#define STAND_H

#include <types.h>

#define NULL        0

typedef __builtin_va_list   va_list;

#define va_start(ap, last) \
    __builtin_va_start((ap), (last))

#define va_arg(ap, type) \
    __builtin_va_arg((ap), type)

#define va_end(ap) \
    __builtin_va_end(ap)

/* functions */
extern size_t strlen(const char *str);
extern char * strcpy(char * restrict to, const char * restrict from);
extern int sprintf(char *buf, const char *cfmt, ...);
extern int vcprintf(void (*func)(int), const char *cfmt, va_list ap);

extern void bzero(void *dst0, size_t length);
extern void* memset(void *dst0, int c0, size_t length);
extern void* memcpy(void *dst0, const void* src0, size_t length);

/* macros */
#define likely(exp) if (__builtin_expect(exp, 1))
#define unlikely(exp) if (__builtin_expect(exp, 0))

#define ALIGN_8(n) (((n) + 7) & ~7)
#define ALIGN_16(n) (((n) + 15) & ~15)
#define ALIGN_32(n) (((n) + 31) & ~31)
#define ALIGN_4K(n) (((n) + 0xfff) & ~0xfff)

extern char const hex2ascii_data[];
#define hex2ascii(hex)  (hex2ascii_data[hex])

static inline int imax(int a, int b) { return (a > b ? a : b); }

static inline int isupper(int c)
{
        return c >= 'A' && c <= 'Z';
}

static inline int islower(int c)
{
        return c >= 'a' && c <= 'z';
}

static inline int toupper(int c)
{
        return islower(c) ? c - 'a' + 'A' : c;
}

static inline int tolower(int c)
{
        return isupper(c) ? c - 'A' + 'a' : c;
}

#endif
