/** @file */

#include "pci_device.h"

#include "io.h"
#include "pci.h"

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define MAX_REGIONS 6

struct _pci_device {
    int bus;
    int device;
    int function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint32_t class_code;
    uint8_t header_type;
    size_t num_regions;
    struct region {
        uint64_t base_address;
        uint64_t size;
        void *map;
        bool is_io;
        bool is_64;
    } regions[MAX_REGIONS];
};

static pci_device_error_handler_t *error_handler = NULL;

void pci_device_error(pci_device_t *restrict pci_device, int status, int error, const char *restrict format, ...);
int pci_device_regions_map(pci_device_t *restrict pci_device);
int pci_device_regions_unmap(pci_device_t *restrict pci_device);

pci_device_t *
pci_device_create(int bus, int device, int function)
{
    pci_device_t *pci_device = (pci_device_t *)calloc(1, sizeof(*pci_device));
    if (pci_device == NULL) {
        pci_device_error(pci_device, 0, errno, __func__);
        return NULL;
    }

    if (bus < 0 || bus > 255) {
        errno = EINVAL;
        pci_device_error(pci_device, 0, errno, __func__);
        goto err;
    }

    if (device < 0 || device > 31) {
        errno = EINVAL;
        pci_device_error(pci_device, 0, errno, __func__);
        goto err;
    }

    if (function < 0 || function > 7) {
        errno = EINVAL;
        pci_device_error(pci_device, 0, errno, __func__);
        goto err;
    }

    pci_device->bus = bus;
    pci_device->device = device;
    pci_device->function = function;
    pci_device->vendor_id = pci_config_read16(pci_device->bus, pci_device->device, pci_device->function, 0);
    if (pci_device->vendor_id == 0xffff) {
        pci_device_error(pci_device, 0, 0, "%s: Invalid device.\n", __func__);
        goto err;
    }

    pci_device->device_id = pci_config_read16(pci_device->bus, pci_device->device, pci_device->function, 2);
    pci_device->class_code = pci_config_read32(pci_device->bus, pci_device->device, pci_device->function, 8) >> 8;
    /* The first part of the predefined header (i.e., the first 16 bytes) are
       defined the same for all types of devices. The the second part of the
       predefined header may have different layouts depending on the base
       function that the device supports. The Header Type field (at offset 14)
       specifies what layout is provided. */
    pci_device->header_type = pci_config_read8(pci_device->bus, pci_device->device, pci_device->function, 14);
    /* Bits 0 to 6 identify the layout of the second part of the predefined
       header. */
    switch (pci_device->header_type & 0x7f) {
    case 0:
        pci_device->num_regions = 6;
        break;

    case 1:
        /* PCI-to-PCI bridge */
        pci_device->num_regions = 2;
        break;

    case 2:
        /* CardBus bridge */
        pci_device->num_regions = 1;
        break;

    default:
        pci_device_error(pci_device, 0, 0, "%s: Unknown header type.\n", __func__);
        goto err;
    }

    if (pci_device_regions_map(pci_device) == -1) {
        pci_device_error(pci_device, 0, errno, __func__);
        goto err;
    }

    return pci_device;

err:
    pci_device_destroy(pci_device);
    return NULL;
}

void
pci_device_destroy(pci_device_t *restrict pci_device)
{
    if (pci_device == NULL) {
        return;
    }

    pci_device_regions_unmap(pci_device);
    free(pci_device);
}

void
pci_device_error(pci_device_t *restrict pci_device, int status, int error, const char *restrict format, ...)
{
    if (error_handler == NULL) {
        return;
    }

    va_list ap;
    va_start(ap, format);
    (*error_handler)(status, error, format, ap);
    va_end(ap);
}

size_t
pci_device_get_num_regions(pci_device_t *restrict pci_device)
{
    return pci_device->num_regions;
}

bool
pci_device_is_ata_controller(pci_device_t *restrict pci_device)
{
    /* Mass storage controller */
    return (((pci_device->class_code & 0xff0000) >> 16) == 0x01)
           /* ATA/IDE controller */
           && (((pci_device->class_code & 0xff00) >> 8) == 0x01);
}

uint64_t
pci_device_region_get_base_address(pci_device_t *restrict pci_device, size_t region_num)
{
    if (region_num >= pci_device->num_regions) {
        errno = EINVAL;
        pci_device_error(pci_device, 0, errno, __func__);
        return (uint64_t)-1;
    }

    return pci_device->regions[region_num].base_address;
}

size_t
pci_device_region_get_size(pci_device_t *restrict pci_device, size_t region_num)
{
    if (region_num >= pci_device->num_regions) {
        errno = EINVAL;
        pci_device_error(pci_device, 0, errno, __func__);
        return (uint64_t)-1;
    }

    return pci_device->regions[region_num].size;
}

bool
pci_device_region_is_io(pci_device_t *restrict pci_device, size_t region_num)
{
    if (region_num >= pci_device->num_regions) {
        errno = EINVAL;
        pci_device_error(pci_device, 0, errno, __func__);
        return (uint64_t)-1;
    }

    return pci_device->regions[region_num].is_io;
}

bool
pci_device_region_is_mapped(pci_device_t *restrict pci_device, size_t region_num)
{
    if (region_num >= pci_device->num_regions) {
        errno = EINVAL;
        pci_device_error(pci_device, 0, errno, __func__);
        return (uint64_t)-1;
    }

    return (!pci_device->regions[region_num].is_io && (pci_device->regions[region_num].map != MAP_FAILED));
}

#define _pci_device_region_define(_size, type) \
    type pci_device_region_read##_size(pci_device_t *restrict pci_device, size_t region_num, size_t offset) \
    { \
        if (region_num >= pci_device->num_regions || offset >= pci_device->regions[region_num].size) { \
            errno = EINVAL; \
            pci_device_error(pci_device, 0, errno, __func__); \
            return (type)-1; \
        } \
\
        if (!pci_device->regions[region_num].is_io && (pci_device->regions[region_num].map == MAP_FAILED)) { \
            errno = EINVAL; \
            pci_device_error(pci_device, 0, errno, __func__); \
            return (type)-1; \
        } \
\
        type value; \
        if (pci_device->regions[region_num].is_io) { \
            value = io_read##_size(pci_device->regions[region_num].base_address + offset); \
            return value; \
        } \
\
        value = ((type *)pci_device->regions[region_num].map)[offset]; \
        return value; \
    } \
\
    void pci_device_region_write##_size( \
            pci_device_t *restrict pci_device, size_t region_num, size_t offset, type value) \
    { \
        if (region_num >= pci_device->num_regions || offset >= pci_device->regions[region_num].size) { \
            errno = EINVAL; \
            pci_device_error(pci_device, 0, errno, __func__); \
            return; \
        } \
\
        if (!pci_device->regions[region_num].is_io && (pci_device->regions[region_num].map == MAP_FAILED)) { \
            errno = EINVAL; \
            pci_device_error(pci_device, 0, errno, __func__); \
            return; \
        } \
\
        if (pci_device->regions[region_num].is_io) { \
            io_write##_size(pci_device->regions[region_num].base_address + offset, value); \
            return; \
        } \
\
        ((type *)pci_device->regions[region_num].map)[offset] = value; \
    }

_pci_device_region_define(16, uint16_t)
_pci_device_region_define(32, uint32_t)
_pci_device_region_define(8, uint8_t)
#undef _pci_device_region_define

int
pci_device_regions_map(pci_device_t *restrict pci_device)
{
    for (size_t i = 0, j = 16; i < pci_device->num_regions; ++i, j += 4) {
        /* Size the 32-bit base address register (BAR) */
        /* Disable (I/O and memory) decoding in the command register before
           sizing the BAR. */
        uint16_t command = pci_config_read16(pci_device->bus, pci_device->device, pci_device->function, 2);
        pci_config_write16(pci_device->bus, pci_device->device, pci_device->function, 2, command & ~0x03);
        /* Save the original value of the BAR */
        uint64_t base_address = pci_config_read32(pci_device->bus, pci_device->device, pci_device->function, j);
        /* Write 0xffffffff to the register, then read it back */
        pci_config_write32(pci_device->bus, pci_device->device, pci_device->function, j, 0xffffffff);
        uint64_t size = pci_config_read32(pci_device->bus, pci_device->device, pci_device->function, j);
        size |= ((uint64_t)0xffffffff << 32);
        /* Restore the original value of the BAR before re-enabling decoding in
           the command register. */
        pci_config_write32(pci_device->bus, pci_device->device, pci_device->function, j, base_address);
        /* Re-enable decoding in the command register */
        pci_config_write16(pci_device->bus, pci_device->device, pci_device->function, 2, command);
        /* @todo Investigate why some ATA/IDE controllers in compatibility mode
           don't specify the ATA I/O addresses in BAR0 to BAR3. */
        if (pci_device_is_ata_controller(pci_device)) {
            /* Is in compatibility mode? */
            if ((pci_device->class_code & 0x05) == 0) {
                if (base_address == 0) {
                    switch (j) {
                    case 16:
                        base_address = 0x1f0 | 0x01;
                        size = ~0x07;
                        break;
                    case 20:
                        base_address = 0x3f0 | 0x01;
                        size = ~0x03;
                        break;
                    case 24:
                        base_address = 0x170 | 0x01;
                        size = ~0x07;
                        break;
                    case 28:
                        base_address = 0x370 | 0x01;
                        size = ~0x03;
                        break;
                    }
                }
            }
        }

        /* Is an I/O address space? */
        if (base_address & 0x01) {
            pci_device->regions[i].is_io = true;
            /* Clear encoding information bits (i.e., bit 0 for I/O) */
            base_address &= ~0x01;
            size &= ~0x01;
            /* Invert all bits (i.e., logical NOT), then increment by 1 */
            size = (~size + 1);
        } else {
            /* Is within a 64-bit address space? */
            if (base_address & 0x04) {
                /* Size the 64-bit BAR */
                pci_device->regions[i].is_64 = true;
                /* Disable (I/O and memory) decoding in the command register
                   before sizing the BAR. */
                pci_config_write16(pci_device->bus, pci_device->device, pci_device->function, 2, command & ~0x03);
                /* Save the original value of the BAR, and extend the current
                   base address with the value of the next BAR. */
                base_address |= ((uint64_t)pci_config_read32(
                                         pci_device->bus, pci_device->device, pci_device->function, j += 4)
                                 << 32);
                /* Write 0xffffffff to the register, then read it back, and
                   extend the current size. */
                pci_config_write32(pci_device->bus, pci_device->device, pci_device->function, j, 0xffffffff);
                size |= ((uint64_t)pci_config_read32(pci_device->bus, pci_device->device, pci_device->function, j)
                         << 32);
                /* Restore the original value of the BAR before re-enabling
                   decoding in the command register. */
                pci_config_write32(pci_device->bus, pci_device->device, pci_device->function, j, base_address >> 32);
                /* Re-enable decoding in the command register */
                pci_config_write16(pci_device->bus, pci_device->device, pci_device->function, 2, command);
            }

            /* Clear encoding information bits (i.e., bits 0 to 3 for memory) */
            base_address &= ~0x0f;
            size &= ~0x0f;
            /* Invert all bits (i.e., logical NOT), then increment by 1 */
            size = (~size + 1);
        }

        pci_device->regions[i].base_address = base_address;
        pci_device->regions[i].size = size;
        if (pci_device->regions[i].is_io) {
            continue;
        }

        /* Map the (memory) region */
        int fd = open("/dev/mem", O_RDWR | O_CLOEXEC);
        if (fd == -1) {
            pci_device_error(pci_device, 0, errno, __func__);
            goto err;
        }

        pci_device->regions[i].map = mmap(NULL, pci_device->regions[i].size, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                pci_device->regions[i].base_address);
        if ((pci_device->regions[i].map == MAP_FAILED) && (errno != EPERM)) {
            close(fd);
            pci_device_error(pci_device, 0, errno, __func__);
            goto err;
        }

        close(fd);
    }

    return 0;

err:
    return -1;
}

int
pci_device_regions_unmap(pci_device_t *restrict pci_device)
{
    for (size_t i = 0; i < pci_device->num_regions; ++i) {
        if (pci_device->regions[i].is_io) {
            continue;
        }

        if (!pci_device->regions[i].is_io && (pci_device->regions[i].map == MAP_FAILED)) {
            continue;
        }

        /* Unmap the (memory) region */
        if (munmap((void *)pci_device->regions[i].base_address, pci_device->regions[i].size) == -1) {
            pci_device_error(pci_device, 0, errno, __func__);
            return -1;
        }
    }

    return 0;
}

pci_device_error_handler_t *
pci_device_set_error_handler(pci_device_error_handler_t *handler)
{
    pci_device_error_handler_t *previous_handler = error_handler;
    error_handler = handler;
    return previous_handler;
}
