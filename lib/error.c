/** @file */

#include "error.h"

#include <stdarg.h>
#include <stddef.h>

static error_handler_t *error_handler = NULL;

void
error(int status, int error, const char *restrict format, ...)
{
    if (error_handler == NULL) {
        return;
    }

    va_list ap;
    va_start(ap, format);
    (*error_handler)(status, error, format, ap);
    va_end(ap);
}

error_handler_t *
set_error_handler(error_handler_t *handler)
{
    error_handler_t *previous_handler = error_handler;
    error_handler = handler;
    return previous_handler;
}
