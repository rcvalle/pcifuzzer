/** @file */

#include "input.h"

#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static input_error_handler_t *error_handler = NULL;

bool
input_derive_bool(FILE *restrict stream)
{
    uint8_t input = input_read8(stream);
    return input & 1;
}

double
input_derive_double(FILE *restrict stream)
{
    uint64_t input = input_read64(stream);
    return input / (double)UINT64_MAX;
}

unsigned long
input_derive_fermat_number(FILE *restrict stream)
{
    unsigned long result = input_derive_range(stream, 1, 31);
    return pow(2, result) + 1;
}

float
input_derive_float(FILE *restrict stream)
{
    uint32_t input = input_read32(stream);
    return input / (float)UINT32_MAX;
}

unsigned long
input_derive_mersenne_number(FILE *restrict stream)
{
    unsigned long result = input_derive_range(stream, 1, 32);
    return pow(2, result) - 1;
}

void
input_error(FILE *restrict stream, int status, int error, const char *restrict format, ...)
{
    if (error_handler == NULL) {
        return;
    }

    va_list ap;
    va_start(ap, format);
    (*error_handler)(status, error, format, ap);
    va_end(ap);
}

unsigned long
input_derive_range(FILE *restrict stream, unsigned long begin, unsigned long end)
{
    double result = input_derive_double(stream);
    return result * (end + 1) + begin;
}

#define _input_define(size, type) \
    type input_read##size(FILE *restrict stream) \
    { \
        type value; \
        if (fread(&value, sizeof(type), 1, stream) < 1) { \
            if (feof(stream) || ferror(stream)) { \
                input_error(stream, 0, errno, __func__); \
                abort(); \
            } \
        } \
\
        return value; \
    } \
\
    void input_read_string##size(FILE *restrict stream, type *string, size_t count) \
    { \
        if (fread(string, sizeof(type), count, stream) < count) { \
            if (feof(stream) || ferror(stream)) { \
                input_error(stream, 0, errno, __func__); \
                abort(); \
            } \
        } \
    }

_input_define(16, uint16_t)
_input_define(32, uint32_t)
_input_define(64, uint64_t)
_input_define(8, uint8_t)
#undef _input_define

input_error_handler_t *
input_set_error_handler(input_error_handler_t *handler)
{
    input_error_handler_t *previous_handler = error_handler;
    error_handler = handler;
    return previous_handler;
}
