/** @file */

#ifndef ERROR_H
#define ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

typedef void error_handler_t(int status, int error, const char *restrict format, va_list ap);

void error(int status, int error, const char *restrict format, ...);
error_handler_t *set_error_handler(error_handler_t *handler);

#ifdef __cplusplus
}
#endif

#endif /* ERROR_H */
