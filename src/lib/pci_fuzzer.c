/** @file */

#include "pci_fuzzer.h"

#include "input.h"
#include "pci_device.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct _pci_fuzzer {
    pci_device_t *pci_device;
    const int *regions;
    size_t num_regions;
    pci_fuzzer_log_handler_t *log_handler;
    FILE *log_stream;
};

static pci_fuzzer_error_handler_t *error_handler = NULL;

void pci_fuzzer_error(pci_fuzzer_t *restrict pci_fuzzer, int status, int error, const char *restrict format, ...);
void pci_fuzzer_log(pci_fuzzer_t *restrict pci_fuzzer, const char *restrict format, ...);

pci_fuzzer_t *
pci_fuzzer_create(pci_device_t *restrict pci_device, const int *regions, size_t num_regions)
{
    pci_fuzzer_t *pci_fuzzer = (pci_fuzzer_t *)calloc(1, sizeof(*pci_fuzzer));
    if (pci_fuzzer == NULL) {
        pci_fuzzer_error(pci_fuzzer, 0, errno, __func__);
        return NULL;
    }

    pci_fuzzer->pci_device = pci_device;
    pci_fuzzer->regions = regions;
    pci_fuzzer->num_regions = num_regions;
    return pci_fuzzer;
}

void
pci_fuzzer_destroy(pci_fuzzer_t *restrict pci_fuzzer)
{
    if (pci_fuzzer == NULL) {
        return;
    }

    free(pci_fuzzer);
}

void
pci_fuzzer_error(pci_fuzzer_t *restrict pci_fuzzer, int status, int error, const char *restrict format, ...)
{
    if (error_handler == NULL) {
        return;
    }

    va_list ap;
    va_start(ap, format);
    (*error_handler)(status, error, format, ap);
    va_end(ap);
}

void
pci_fuzzer_iterate(pci_fuzzer_t *restrict pci_fuzzer, FILE *restrict stream)
{
    size_t region = 0;
    if (pci_fuzzer->regions == NULL || pci_fuzzer->num_regions == 0) {
        size_t num_regions = pci_device_get_num_regions(pci_fuzzer->pci_device);
        region = input_derive_range(stream, 0, num_regions - 1);
    } else {
        size_t region_num = input_derive_range(stream, 0, pci_fuzzer->num_regions - 1);
        region = pci_fuzzer->regions[region_num];
    }

    if (!pci_device_region_is_io(pci_fuzzer->pci_device, region)
            && !pci_device_region_is_mapped(pci_fuzzer->pci_device, region)) {
        return;
    }

    size_t region_size = pci_device_region_get_size(pci_fuzzer->pci_device, region);
    size_t offset = input_derive_range(stream, 0, region_size - 1);
    switch (input_derive_range(stream, 0, 5)) {
    case 0: {
        pci_fuzzer_log(pci_fuzzer, "suu", "function", "pci_device_region_read16", "region", region, "offset", offset);
        pci_device_region_read16(pci_fuzzer->pci_device, region, offset);
        break;
    }

    case 1: {
        pci_fuzzer_log(pci_fuzzer, "suu", "function", "pci_device_region_read32", "region", region, "offset", offset);
        pci_device_region_read32(pci_fuzzer->pci_device, region, offset);
        break;
    }

    case 2: {
        pci_fuzzer_log(pci_fuzzer, "suu", "function", "pci_device_region_read8", "region", region, "offset", offset);
        pci_device_region_read8(pci_fuzzer->pci_device, region, offset);
        break;
    }

    case 3: {
        uint16_t value = input_read16(stream);
        pci_fuzzer_log(pci_fuzzer, "suuu", "function", "pci_device_region_write16", "region", region, "offset", offset,
                "value", value);
        pci_device_region_write16(pci_fuzzer->pci_device, region, offset, value);
        break;
    }

    case 4: {
        uint32_t value = input_read32(stream);
        pci_fuzzer_log(pci_fuzzer, "suuu", "function", "pci_device_region_write32", "region", region, "offset", offset,
                "value", value);
        pci_device_region_write32(pci_fuzzer->pci_device, region, offset, value);
        break;
    }

    case 5: {
        uint8_t value = input_read8(stream);
        pci_fuzzer_log(pci_fuzzer, "suuu", "function", "pci_device_region_write8", "region", region, "offset", offset,
                "value", value);
        pci_device_region_write8(pci_fuzzer->pci_device, region, offset, value);
        break;
    }

    default:
        abort();
    }
}

void
pci_fuzzer_log(pci_fuzzer_t *restrict pci_fuzzer, const char *restrict format, ...)
{
    if (pci_fuzzer->log_handler == NULL) {
        return;
    }

    va_list ap;
    va_start(ap, format);
    (*pci_fuzzer->log_handler)(pci_fuzzer->log_stream, format, ap);
    va_end(ap);
}

pci_fuzzer_error_handler_t *
pci_fuzzer_set_error_handler(pci_fuzzer_error_handler_t *handler)
{
    pci_fuzzer_error_handler_t *previous_handler = error_handler;
    error_handler = handler;
    return previous_handler;
}

pci_fuzzer_log_handler_t *
pci_fuzzer_set_log_handler(pci_fuzzer_t *restrict pci_fuzzer, pci_fuzzer_log_handler_t *handler)
{
    pci_fuzzer_log_handler_t *previous_handler = pci_fuzzer->log_handler;
    pci_fuzzer->log_handler = handler;
    return previous_handler;
}

FILE *
pci_fuzzer_set_log_stream(pci_fuzzer_t *restrict pci_fuzzer, FILE *stream)
{
    FILE *previous_stream = pci_fuzzer->log_stream;
    pci_fuzzer->log_stream = stream;
    return previous_stream;
}
