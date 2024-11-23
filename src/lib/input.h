/** @file */

#ifndef INPUT_H
#define INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef void input_error_handler_t(int status, int error, const char *restrict format, va_list ap);

/**
 * Derives a Boolean value from the input.
 *
 * @param [in] stream Input stream.
 * @return Boolean value.
 */
bool input_derive_bool(FILE *restrict stream);

/**
 * Derives a double precision floating point value in the range given by the
 * interval [0,1) from the input.
 *
 * @param [in] stream Input stream.
 * @return Double precision floating point value in the range given by the
 *   interval [0,1) from the input.
 */
double input_derive_double(FILE *restrict stream);

/**
 * Derives a Fermat number given by the binomial number of the form (2^n)+1 in
 * the range given by the interval [3,(2^31)+1] from the input.
 *
 * @param [in] stream Input stream.
 * @return Fermat number given by the binomial number of the form (2^n)+1 in the
 *   range given by the interval [3,(2^31)+1].
 */
unsigned long input_derive_fermat_number(FILE *restrict stream);

/**
 * Derives a single precision floating point value in the range given by the
 * interval [0,1) from the input.
 *
 * @param [in] stream Input stream.
 * @return Single precision floating point value in the range given by the
 *   interval [0,1) from the input.
 */
float input_derive_float(FILE *restrict stream);

/**
 * Derives a Mersenne number given by the binomial number of the form (2^n)-1 in
 * the range given by the interval [1,2^32) from the input.
 *
 * @param [in] stream Input stream.
 * @return Mersenne number given by the binomial number of the form (2^n)-1 in
 *   the range given by the interval [1,2^32).
 */
unsigned long input_derive_mersenne_number(FILE *restrict stream);

/**
 * Derives an unsigned long integer value in the range given by the interval
 * [begin,end] from the input.
 *
 * @param [in] random Pseudo-random number generator.
 * @return Unsigned long integer value in the range given by the interval
 *   [begin,end].
 */
unsigned long input_derive_range(FILE *restrict stream, unsigned long begin, unsigned long end);

/**
 * Reads a 16-bit unsigned integer value from the input.
 *
 * @param [in] stream Input stream.
 * @return 16-bit unsigned integer value.
 */
uint16_t input_read16(FILE *restrict stream);

/**
 * Reads a 32-bit unsigned integer value from the input.
 *
 * @param [in] stream Input stream.
 * @return 32-bit unsigned integer value.
 */
uint32_t input_read32(FILE *restrict stream);

/**
 * Reads a 64-bit unsigned integer value from the input.
 *
 * @param [in] stream Input stream.
 * @return 64-bit unsigned integer value.
 */
uint64_t input_read64(FILE *restrict stream);

/**
 * Reads a 8-bit unsigned integer value from the input.
 *
 * @param [in] stream Input stream.
 * @return 8-bit unsigned integer value.
 */
uint8_t input_read8(FILE *restrict stream);

/**
 * Reads a string 16-bit unsigned integer values from the input.
 *
 * @param [in] stream Input stream.
 * @param [out] string String 16-bit unsigned integer values.
 * @param [in] count Number of 16-bit unsigned integer values.
 */
void input_read_string16(FILE *restrict stream, uint16_t *string, size_t count);

/**
 * Reads a string 32-bit unsigned integer values from the input.
 *
 * @param [in] stream Input stream.
 * @param [out] string String 32-bit unsigned integer values.
 * @param [in] count Number of 32-bit unsigned integer values.
 */
void input_read_string32(FILE *restrict stream, uint32_t *string, size_t count);

/**
 * Reads a string 64-bit unsigned integer values from the input.
 *
 * @param [in] stream Input stream.
 * @param [out] string String 64-bit unsigned integer values.
 * @param [in] count Number of 64-bit unsigned integer values.
 */
void input_read_string64(FILE *restrict stream, uint64_t *string, size_t count);

/**
 * Reads a string 8-bit unsigned integer values from the input.
 *
 * @param [in] stream Input stream.
 * @param [out] string String 8-bit unsigned integer values.
 * @param [in] count Number of 8-bit unsigned integer values.
 */
void input_read_string8(FILE *restrict stream, uint8_t *string, size_t count);

/**
 * Sets the error handler for the ATA device.
 *
 * @param [in] handler Error handler.
 * @return Previous error handler.
 */
input_error_handler_t *input_set_error_handler(input_error_handler_t *handler);

#ifdef __cplusplus
}
#endif

#endif /* INPUT_H */
