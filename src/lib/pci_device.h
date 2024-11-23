/** @file */

#ifndef PCI_DEVICE_H
#define PCI_DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct _pci_device pci_device_t; /**< PCI device. */

typedef void pci_device_error_handler_t(int status, int error, const char *restrict format, va_list ap);

/**
 * Creates a PCI device.
 *
 * @param [in] bus PCI bus number.
 * @param [in] device PCI device number.
 * @param [in] function PCI function number.
 * @return A PCI device.
 */
pci_device_t *pci_device_create(int bus, int device, int function);

/**
 * Destroys the PCI device.
 *
 * @param [in] pci_device PCI device.
 */
void pci_device_destroy(pci_device_t *restrict pci_device);

/**
 * Returns the number of regions of the PCI device.
 *
 * @param [in] pci_device PCI device.
 * @return Number of regions.
 */
size_t pci_device_get_num_regions(pci_device_t *restrict pci_device);

/**
 * Returns whether the PCI device is an ATA/IDE controller.
 *
 * @param [in] pci_device PCI device.
 * @return Returns true if the PCI device is an ATA/IDE controller; otherwise,
 *   returns false if the PCI device is not an ATA/IDE controller.
 */
bool pci_device_is_ata_controller(pci_device_t *restrict pci_device);

/**
 * Returns the base address register (BAR) of the PCI device region.
 *
 * @param [in] pci_device PCI device.
 * @param [in] region_num Region number.
 * @return Base address register (BAR).
 */
uint64_t pci_device_region_get_base_address(pci_device_t *restrict pci_device, size_t region_num);

/**
 * Returns the size of the PCI device region.
 *
 * @param [in] pci_device PCI device.
 * @param [in] region_num Region number.
 * @return Size.
 */
size_t pci_device_region_get_size(pci_device_t *restrict pci_device, size_t region_num);

/**
 * Returns whether the PCI device region is I/O.
 *
 * @param [in] pci_device PCI device.
 * @param [in] region_num Region number.
 * @return Returns true if the PCI device region is I/O; otherwise, returns
 *   false if the PCI device region is not I/O.
 */
bool pci_device_region_is_io(pci_device_t *restrict pci_device, size_t region_num);

/**
 * Returns whether the PCI device region is mapped.
 *
 * @param [in] pci_device PCI device.
 * @param [in] region_num Region number.
 * @return Returns true if the PCI device region is mapped; otherwise, returns
 *   false if the PCI device region is not mapped.
 */
bool pci_device_region_is_mapped(pci_device_t *restrict pci_device, size_t region_num);

/**
 * Reads a 16-bit value from the PCI device region.
 *
 * @param [in] pci_device PCI device.
 * @param [in] region_num Region number.
 * @param [in] offset Region offset.
 * @return Value.
 */
uint16_t pci_device_region_read16(pci_device_t *restrict pci_device, size_t region_num, size_t offset);

/**
 * Reads a 32-bit value from the PCI device region.
 *
 * @param [in] pci_device PCI device.
 * @param [in] region_num Region number.
 * @param [in] offset Region offset.
 * @return Value.
 */
uint32_t pci_device_region_read32(pci_device_t *restrict pci_device, size_t region_num, size_t offset);

/**
 * Reads an 8-bit value from the PCI device region.
 *
 * @param [in] pci_device PCI device.
 * @param [in] region_num Region number.
 * @param [in] offset Region offset.
 * @return Value.
 */
uint8_t pci_device_region_read8(pci_device_t *restrict pci_device, size_t region_num, size_t offset);

/**
 * Writes a 16-bit value to the PCI device region.
 *
 * @param [in] pci_device PCI device.
 * @param [in] region_num Region number.
 * @param [in] offset Region offset.
 * @param [in] value Value.
 */
void pci_device_region_write16(pci_device_t *restrict pci_device, size_t region_num, size_t offset, uint16_t value);

/**
 * Writes an 32-bit value to the PCI device region.
 *
 * @param [in] pci_device PCI device.
 * @param [in] region_num Region number.
 * @param [in] offset Region offset.
 * @param [in] value Value.
 */
void pci_device_region_write32(pci_device_t *restrict pci_device, size_t region_num, size_t offset, uint32_t value);

/**
 * Writes an 8-bit value to the PCI device region.
 *
 * @param [in] pci_device PCI device.
 * @param [in] region_num Region number.
 * @param [in] offset Region offset.
 * @param [in] value Value.
 */
void pci_device_region_write8(pci_device_t *restrict pci_device, size_t region_num, size_t offset, uint8_t value);

/**
 * Sets the error handler for the PCI device.
 *
 * @param [in] handler Error handler.
 * @return Previous error handler.
 */
pci_device_error_handler_t *pci_device_set_error_handler(pci_device_error_handler_t *handler);

#ifdef __cplusplus
}
#endif

#endif /* PCI_DEVICE_H */
