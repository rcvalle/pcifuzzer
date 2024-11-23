/** @file */

#ifndef PCI_H
#define PCI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "io.h"

#include <stddef.h>
#include <stdint.h>

#define _pci_config_define(size, type, mask) \
    static inline type pci_config_read##size(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) \
    { \
        io_write32(0xcf8, 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | offset); \
        return io_read##size(0xcfc + (offset & mask)); \
    } \
\
    static inline void pci_config_write##size( \
            uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, type value) \
    { \
        io_write32(0xcf8, 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | offset); \
        io_write##size(0xcfc + (offset & mask), value); \
    }

_pci_config_define(16, uint16_t, 2)
_pci_config_define(32, uint32_t, 0)
_pci_config_define(8, uint8_t, 3)
#undef _pci_config_define

#ifdef __cplusplus
}
#endif

#endif /* PCI_H */
