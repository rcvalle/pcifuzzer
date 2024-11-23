/** @file */

#ifndef IO_H
#define IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define _io_define(size, type, suffix, modifier) \
    static inline type io_read##size(uint16_t port) \
    { \
        type value; \
        asm volatile("in" #suffix " %w1, %" #modifier "0" : "=a"(value) : "Nd"(port)); \
        return value; \
    } \
\
    static inline void io_read_string##size(uint16_t port, type *string, size_t count) \
    { \
        asm volatile("rep; ins" #suffix : "+D"(string), "+c"(count) : "d"(port) : "memory"); \
    } \
\
    static inline void io_write##size(uint16_t port, type value) \
    { \
        asm volatile("out" #suffix " %" #modifier "0, %w1" : : "a"(value), "Nd"(port)); \
    } \
\
    static inline void io_write_string##size(uint16_t port, const type *string, size_t count) \
    { \
        asm volatile("rep; outs" #suffix : "+S"(string), "+c"(count) : "d"(port) : "memory"); \
    }

_io_define(16, uint16_t, w, w)
_io_define(32, uint32_t, l, k)
_io_define(8, uint8_t, b, b)
#undef _io_define

#ifdef __cplusplus
}
#endif

#endif /* IO_H */
