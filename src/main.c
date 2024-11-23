/** @file */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../lib/error.h"
#include "../lib/string.h"
#include "lib/pci_device.h"
#include "lib/pci_fuzzer.h"

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/io.h>
#include <unistd.h>

#define MAX_REGIONS 6

#define usage() \
    fprintf(stderr, \
            "Usage: %s [OPTION]... [INPUT]\n" \
            "Options:\n" \
            "  -B, --bus=NUM         Specify the PCI bus number of the device. (The default\n" \
            "                        is 0.)\n" \
            "  -D, --device=NUM      Specify the PCI device number of the device. (The\n" \
            "                        default is 0.)\n" \
            "  -F, --function=NUM    Specify the PCI function number of the ATA/IDE\n" \
            "                        controller. (The default is 0.)\n" \
            "  -d, --debug           Enable debug mode.\n" \
            "  -g, --generate        Use the pseudorandom number generator (i.e., random())\n" \
            "                        for input generation.\n" \
            "  -h, --help            Display help information and exit.\n" \
            "  -o, --output=FILE     Specify the output file name.\n" \
            "  -q, --quiet           Enable quiet mode.\n" \
            "  -r, --regions=LIST    Specify the list of PCI device regions. (The default is\n" \
            "                        all regions.)\n" \
            "  -s, --seed=NUM        Specify the seed for the pseudorandom number generator.\n" \
            "                        (The default is 1.)\n" \
            "  -t, --timeout=NUM     Specify the timeout, in seconds, for each iteration.\n" \
            "                        (The default is 5.)\n" \
            "  -v, --verbose         Enable verbose mode.\n" \
            "      --version         Display version information and exit.\n", \
            PACKAGE_NAME)

#define version() fprintf(stderr, "%s\n", PACKAGE_STRING)

void
default_error_handler(int status, int error, const char *restrict format, va_list ap)
{
    fflush(stdout);
    vfprintf(stderr, format, ap);
    if (error != 0) {
        fprintf(stderr, ": %s\n", strerror(error));
    }

    fflush(stderr);
    abort();
}

void
default_log_handler(FILE *restrict stream, const char *restrict format, va_list ap)
{
    flockfile(stream);
    fprintf(stream, "{ ");
    fprintf(stream, "\"time\": %d,", (unsigned int)time(NULL));
    for (size_t i = 0; format[i] != '\0'; ++i) {
        if (i > 0) {
            fprintf(stream, ", ");
        }

        fprintf(stream, "\"%s\": ", va_arg(ap, char *));
        switch (format[i]) {
        case 'c':
            fprintf(stream, "\"%c\"", va_arg(ap, int));
            break;

        case 'd':
            fprintf(stream, "%d", va_arg(ap, int));
            break;

        case 'f':
            fprintf(stream, "%f", va_arg(ap, double));
            break;

        case 'o':
            fprintf(stream, "%o", va_arg(ap, unsigned int));
            break;

        case 'p':
            fprintf(stream, "%p", va_arg(ap, void *));
            break;

        case 'q':
            fprintf(stream, "%llu", va_arg(ap, unsigned long long int));
            break;

        case 's':
            fprintf(stream, "\"%s\"", va_arg(ap, char *));
            break;

        case 'u':
            fprintf(stream, "%u", va_arg(ap, unsigned int));
            break;

        case 'x':
            fprintf(stream, "%x", va_arg(ap, unsigned int));
            break;

        case 'z':
            fprintf(stream, "%zu", va_arg(ap, size_t));
            break;

        default:
            abort();
        }
    }

    fprintf(stream, " }\n");
    fflush(stream);
    fsync(fileno(stream));
    funlockfile(stream);
}

void
random_buf(void *buf, size_t size)
{
    uint32_t number = 0;
    for (size_t i = 0; i < size; ++i) {
        if ((i % sizeof(uint16_t)) == 0) {
            number = random();
        }

        ((uint8_t *)buf)[i] = (number >> (8 * (i % sizeof(uint16_t)))) & 0xff;
    }
}

int
main(int argc, char *argv[])
{
    int c = 0;
    enum
    {
        OPT_VERSION = CHAR_MAX + 1,
    };
    /* clang-format off */
    static struct option longopts[] = {
        {"bus",         required_argument, NULL, 'B'             },
        {"device",      required_argument, NULL, 'D'             },
        {"function",    required_argument, NULL, 'F'             },
        {"debug",       no_argument,       NULL, 'd'             },
        {"generate",    no_argument,       NULL, 'g'             },
        {"help",        no_argument,       NULL, 'h'             },
        {"output",      required_argument, NULL, 'o'             },
        {"quiet",       no_argument,       NULL, 'q'             },
        {"regions",     required_argument, NULL, 'r'             },
        {"seed",        required_argument, NULL, 's'             },
        {"timeout",     required_argument, NULL, 't'             },
        {"verbose",     no_argument,       NULL, 'v'             },
        {"version",     no_argument,       NULL, OPT_VERSION     },
        {NULL,          0,                 NULL, 0               }
    };
    /* clang-format on */
    static int longindex = 0;
    unsigned long bus = 0;
    unsigned long device = 0;
    unsigned long function = 0;
    int debug = 0;
    int generate = 0;
    char *input = NULL;
    char *output = NULL;
    int quiet = 0;
    int *regions = NULL;
    size_t num_regions = 0;
    unsigned long seed = 1;
    int timeout = 5;
    int verbose = 0;
    while ((c = getopt_long(argc, argv, "B:D:F:dgho:qr:s:t:v", longopts, &longindex)) != -1) {
        switch (c) {
        case 'B':
            errno = 0;
            bus = strtoul(optarg, NULL, 0);
            if (errno != 0) {
                perror("strtoul");
                exit(EXIT_FAILURE);
            }

            if (bus > 255) {
                fprintf(stderr, "%s: Invalid PCI bus number.\n", __func__);
                exit(EXIT_FAILURE);
            }

            break;

        case 'D':
            errno = 0;
            device = strtoul(optarg, NULL, 0);
            if (errno != 0) {
                perror("strtoul");
                exit(EXIT_FAILURE);
            }

            if (device > 31) {
                fprintf(stderr, "%s: Invalid PCI device number.\n", __func__);
                exit(EXIT_FAILURE);
            }

            break;

        case 'F':
            errno = 0;
            function = strtoul(optarg, NULL, 0);
            if (errno != 0) {
                perror("strtoul");
                exit(EXIT_FAILURE);
            }

            if (function > 7) {
                fprintf(stderr, "%s: Invalid PCI function number.\n", __func__);
                exit(EXIT_FAILURE);
            }

            break;

        case 'd':
            debug = 1;
            break;

        case 'g':
            generate = 1;
            break;

        case 'h':
            usage();
            exit(EXIT_FAILURE);

        case 'o':
            output = optarg;
            break;

        case 'q':
            quiet = 1;
            break;

        case 'r':
            if (string_split_range(optarg, ",", MAX_REGIONS, &regions, &num_regions) == -1) {
                perror("getlist");
                exit(EXIT_FAILURE);
            }

            break;

        case 's':
            errno = 0;
            seed = strtoul(optarg, NULL, 0);
            if (errno != 0) {
                perror("strtoul");
                exit(EXIT_FAILURE);
            }

            break;

        case 't':
            errno = 0;
            timeout = strtoul(optarg, NULL, 0);
            if (errno != 0) {
                perror("strtoul");
                exit(EXIT_FAILURE);
            }

            break;

        case 'v':
            verbose = 1;
            break;

        case OPT_VERSION:
            version();
            exit(EXIT_FAILURE);

        default:
            usage();
            exit(EXIT_FAILURE);
        }
    }

    FILE *stream = stdout;
    if (output != NULL) {
        stream = fopen(output, "a+");
        if (stream == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
    }

    if (iopl(3) == -1) {
        perror("iopl");
        exit(EXIT_FAILURE);
    }

    pci_device_set_error_handler(default_error_handler);
    pci_device_t *pci_device = pci_device_create(bus, device, function);
    if (pci_device == NULL) {
        perror("pci_device_create");
        fclose(stream);
        exit(EXIT_FAILURE);
    }

    pci_fuzzer_set_error_handler(default_error_handler);
    pci_fuzzer_t *pci_fuzzer = pci_fuzzer_create(pci_device, regions, num_regions);
    if (pci_fuzzer == NULL) {
        perror("pci_fuzzer_create");
        goto err;
    }

    pci_fuzzer_set_log_handler(pci_fuzzer, default_log_handler);
    pci_fuzzer_set_log_stream(pci_fuzzer, stream);
    if (generate) {
        srandom(seed);
        for (;;) {
            uint8_t buf[PCI_FUZZER_MAX_INPUT];
            random_buf(buf, sizeof(buf));
            FILE *stream = fmemopen(buf, sizeof(buf), "r");
            if (stream == NULL) {
                perror("fmemopen");
                goto err;
            }

            pci_fuzzer_iterate(pci_fuzzer, stream);
            fclose(stream);
        }
    } else {
        if (argv[optind] != NULL) {
            input = argv[optind];
        }

        FILE *stream = stdin;
        if (input != NULL) {
            FILE *stream = fopen(input, "r");
            if (stream == NULL) {
                perror("fopen");
                exit(EXIT_FAILURE);
            }
        }

        pci_fuzzer_iterate(pci_fuzzer, stream);
        fclose(stream);
    }

    pci_fuzzer_destroy(pci_fuzzer);
    pci_device_destroy(pci_device);
    fclose(stream);
    free(regions);
    exit(EXIT_SUCCESS);

err:
    pci_fuzzer_destroy(pci_fuzzer);
    pci_device_destroy(pci_device);
    fclose(stream);
    free(regions);
    exit(EXIT_FAILURE);
}
