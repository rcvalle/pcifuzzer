/** @file */

#ifndef PCI_FUZZER_H
#define PCI_FUZZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pci_device.h"

#include <stdarg.h>
#include <stdio.h>

#define PCI_FUZZER_MAX_INPUT 28

typedef struct _pci_fuzzer pci_fuzzer_t; /**< PCI fuzzer. */

typedef void pci_fuzzer_error_handler_t(int status, int error, const char *restrict format, va_list ap);
typedef void pci_fuzzer_log_handler_t(FILE *restrict stream, const char *restrict format, va_list ap);

/**
 * Creates an PCI fuzzer.
 *
 * @param [in] pci_device PCI device.
 * @param [in] regions List of PCI device regions.
 * @param [in] num_regions Number of PCI device regions.
 * @return An PCI fuzzer.
 */
pci_fuzzer_t *pci_fuzzer_create(pci_device_t *restrict pci_device, const int *regions, size_t num_regions);

/**
 * Destroys the PCI fuzzer.
 *
 * @param [in] pci_fuzzer PCI fuzzer.
 */
void pci_fuzzer_destroy(pci_fuzzer_t *restrict pci_fuzzer);

/**
 * Performs an iteration.
 *
 * @param [in] pci_fuzzer PCI fuzzer.
 * @param [in] stream Input stream.
 */
void pci_fuzzer_iterate(pci_fuzzer_t *restrict pci_fuzzer, FILE *restrict stream);

/**
 * Sets the error handler for the PCI fuzzer.
 *
 * @param [in] handler Error handler.
 * @return Previous error handler.
 */
pci_fuzzer_error_handler_t *pci_fuzzer_set_error_handler(pci_fuzzer_error_handler_t *handler);

/**
 * Sets the log handler for the PCI fuzzer.
 *
 * @param [in] handler Log handler.
 * @return Previous log handler.
 */
pci_fuzzer_log_handler_t *pci_fuzzer_set_log_handler(
        pci_fuzzer_t *restrict pci_fuzzer, pci_fuzzer_log_handler_t *handler);

/**
 * Sets the log stream for the PCI fuzzer.
 *
 * @param [in] stream Log stream.
 * @return Previous log stream.
 */
FILE *pci_fuzzer_set_log_stream(pci_fuzzer_t *restrict pci_fuzzer, FILE *stream);

#ifdef __cplusplus
}
#endif

#endif /* PCI_FUZZER_H */
