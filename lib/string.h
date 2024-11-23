/** @file */

#ifndef STRING_H
#define STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define _string_split_range_define(size, type, max) \
    int string_split_range##size( \
            char *string, char *delimiter, size_t max_elements, type **elements, size_t *num_elements) \
    { \
        if (string == NULL || max_elements > (max) || elements == NULL || num_elements == NULL) { \
            errno = EINVAL; \
            return -1; \
        } \
\
        int *bit_array = (int *)calloc((max_elements + (sizeof(int) - 1)) / sizeof(int), sizeof(int)); \
        if (bit_array == NULL) { \
            return -1; \
        } \
\
        char *str = strdup(string); \
        char *lasts = NULL; \
        for (char *token = strtok_r(str, delimiter, &lasts); token != NULL; \
                token = strtok_r(NULL, delimiter, &lasts)) { \
            char *substr = strdup(token); \
            char *lasts = NULL; \
            char *subtoken = strtok_r(substr, "-", &lasts); \
            if (subtoken != NULL) { \
                errno = 0; \
                unsigned long begin = strtoul(subtoken, NULL, 0); \
                if (errno == EINVAL) { \
                    free(substr); \
                    goto err; \
                } \
\
                if (begin > max_elements) { \
                    errno = EINVAL; \
                    free(substr); \
                    goto err; \
                } \
\
                unsigned long end = begin; \
                subtoken = strtok_r(NULL, "-", &lasts); \
                if (subtoken != NULL) { \
                    errno = 0; \
                    end = strtoul(subtoken, NULL, 0); \
                    if (errno == EINVAL) { \
                        free(substr); \
                        goto err; \
                    } \
\
                    if ((begin > end) || (end > max_elements)) { \
                        errno = EINVAL; \
                        free(substr); \
                        goto err; \
                    } \
                } \
\
                for (unsigned long value = begin; (value <= end) && (value <= max_elements); ++value) { \
                    bit_array[value / sizeof(int)] |= 1 << (value % sizeof(int)); \
                } \
            } \
\
            free(substr); \
        } \
\
        *num_elements = 0; \
        for (size_t i = 0; i <= max_elements; ++i) { \
            if ((bit_array[i / sizeof(int)] & (1 << (i % sizeof(int)))) != 0) { \
                ++(*num_elements); \
            } \
        } \
\
        *elements = (type *)calloc(*num_elements, sizeof(**elements)); \
        if (elements == NULL) { \
            return -1; \
        } \
\
        for (size_t i = 0, j = 0; i <= max_elements; ++i) { \
            if ((bit_array[i / sizeof(int)] & (1 << (i % sizeof(int)))) != 0) { \
                (*elements)[j++] = i; \
            } \
        } \
\
        free(str); \
        free(bit_array); \
        return 0; \
\
    err: \
        free(str); \
        free(bit_array); \
        return -1; \
    }

_string_split_range_define(i, int, INT_MAX)
_string_split_range_define(16, uint16_t, UINT16_MAX)
_string_split_range_define(32, uint32_t, UINT32_MAX)
_string_split_range_define(8, uint8_t, UINT8_MAX)
#undef _string_split_range_define

#define string_split_range(string, delimiter, max_elements, elements, num_elements) \
    _Generic((elements), \
        uint16_t **: string_split_range16, \
        uint32_t **: string_split_range32, \
        uint8_t **: string_split_range8, \
        default: string_split_rangei \
        )(string, delimiter, max_elements, elements, num_elements)

#ifdef __cplusplus
}
#endif

#endif /* STRING_H */
