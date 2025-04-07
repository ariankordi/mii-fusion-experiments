#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdbool.h>

// ANSI escape codes for red text.
#define RED_TEXT "\033[1;31m"
#define RESET_TEXT "\033[0m"

// Default number of rounds if not specified.
#define DEFAULT_ROUNDS 100000

// Default test string.
#define DEFAULT_TEST_STRING "The quick brown fox jumps over the lazy dog"

// Base32 alphabet for random generation.
static const char *base32_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

// Precomputed CRC-16/CCITT table (polynomial 0x1021)
static const uint16_t crc16_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

/**
 * @brief "Magic" CRC-16/CCITT function ported from Python.
 *
 * This function emulates the behavior of the original Python function that was
 * rewritten from an assembly snippet. It splits the initial CRC into two bytes,
 * then iterates over each character in the input string, mixing in each byte via
 * XOR and bit-shift operations to update the CRC value.
 *
 * @param initialCrc The initial CRC value.
 * @param data A null-terminated string on which to compute the CRC.
 * @return The computed 16-bit CRC value.
 */
uint16_t crc16_magic(uint16_t initialCrc, const char *data) {
    // Split the initial CRC into two separate bytes.
    uint8_t msb = (uint8_t)(initialCrc >> 8);   // Most significant byte.
    uint8_t lsb = (uint8_t)(initialCrc & 0xFF);   // Least significant byte.

    // Process each character of the string.
    while (*data) {
        // Introduce the character into the CRC by XOR-ing with msb.
        // The 'ord' equivalent in C is simply using the char's integer value.
        int x = ((int)(*data)) ^ msb;
        // Mix the bits in x by XOR-ing it with its own value shifted right by 4.
        x ^= (x >> 4);
        // Compute the new msb:
        // - Start with the previous lsb.
        // - Mix in bits from x shifted right by 3 and left by 4.
        // - Mask with 0xFF to ensure it remains an 8-bit value.
        msb = (uint8_t)((lsb ^ (x >> 3) ^ (x << 4)) & 0xFF);
        // Compute the new lsb:
        // - XOR x with itself shifted left by 5.
        // - Mask with 0xFF for an 8-bit result.
        lsb = (uint8_t)((x ^ (x << 5)) & 0xFF);
        data++;  // Move to the next character.
    }

    // Recombine the two bytes into a 16-bit value.
    return ((uint16_t)msb << 8) | lsb;
}

/**
 * @brief Traditional bitwise (non-table) CRC-16/CCITT calculation.
 *
 * This implementation uses the classic algorithm for computing the CRC by
 * processing one bit at a time for each byte of the input.
 *
 * @param data A pointer to the data buffer.
 * @param length The number of bytes in the data buffer.
 * @return The computed 16-bit CRC value.
 */
uint16_t crc16_bitwise(const uint8_t *data, size_t length) {
    // Initialize CRC with 0xFFFF (common for CRC-16/CCITT)
    uint16_t crc = 0xFFFF;

    // Process each byte in the input buffer.
    for (size_t i = 0; i < length; i++) {
        // XOR the high byte of the CRC with the current data byte.
        crc ^= ((uint16_t)data[i]) << 8;
        // Process each bit in the current byte.
        for (int j = 0; j < 8; j++) {
            // If the highest bit is set, shift left and XOR with the polynomial.
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}

/**
 * @brief Table-driven CRC-16/CCITT calculation.
 *
 * This function uses a precomputed lookup table to calculate the CRC, which is
 * typically faster than bitwise computation. The algorithm processes each byte
 * by using the lookup table to incorporate the data into the CRC value.
 *
 * @param data A pointer to the data buffer.
 * @param length The number of bytes in the data buffer.
 * @return The computed 16-bit CRC value.
 */
uint16_t crc16_table_driven(const uint8_t *data, size_t length) {
    // Initialize CRC with 0xFFFF (common for CRC-16/CCITT).
    uint16_t crc = 0xFFFF;

    // Process each byte.
    for (size_t i = 0; i < length; i++) {
        // Calculate the index for the lookup table by XOR-ing the high byte of CRC with the data byte.
        uint8_t index = (uint8_t)((crc >> 8) ^ data[i]);
        // Update the CRC by shifting left 8 bits and XOR-ing with the table entry.
        crc = (crc << 8) ^ crc16_table[index];
    }
    return crc;
}

/**
 * @brief Generate a random Base32 string of a specified length.
 *
 * @param length The desired length of the random string.
 * @return A pointer to the generated null-terminated random string. Caller must free.
 */
char *generate_random_base32(size_t length) {
    char *randStr = malloc(length + 1);
    if (!randStr) {
        fprintf(stderr, "Memory allocation error.\n");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < length; i++) {
        randStr[i] = base32_alphabet[rand() % 32];
    }
    randStr[length] = '\0';
    return randStr;
}

/**
 * @brief Read the entire contents of a file.
 *
 * @param filePath The path to the input file.
 * @param outLength Pointer to store the length of the read data.
 * @return A pointer to the read data. Caller must free.
 */
char *read_file(const char *filePath, size_t *outLength) {
    FILE *fp = fopen(filePath, "rb");
    if (!fp) {
        fprintf(stderr, "Error opening file: %s\n", filePath);
        exit(EXIT_FAILURE);
    }
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);
    char *buffer = malloc(fileSize + 1);
    if (!buffer) {
        fprintf(stderr, "Memory allocation error.\n");
        exit(EXIT_FAILURE);
    }
    size_t bytesRead = fread(buffer, 1, fileSize, fp);
    buffer[bytesRead] = '\0';
    fclose(fp);
    if (outLength) {
        *outLength = bytesRead;
    }
    return buffer;
}

/**
 * @brief Read the entire contents from stdin.
 *
 * @param outLength Pointer to store the length of the read data.
 * @return A pointer to the read data. Caller must free.
 */
char *read_stdin(size_t *outLength) {
    size_t capacity = 1024;
    size_t length = 0;
    char *buffer = malloc(capacity);
    if (!buffer) {
        fprintf(stderr, "Memory allocation error.\n");
        exit(EXIT_FAILURE);
    }
    int c;
    while ((c = getchar()) != EOF) {
        if (length + 1 >= capacity) {
            capacity *= 2;
            buffer = realloc(buffer, capacity);
            if (!buffer) {
                fprintf(stderr, "Memory allocation error.\n");
                exit(EXIT_FAILURE);
            }
        }
        buffer[length++] = (char)c;
    }
    buffer[length] = '\0';
    if (outLength) {
        *outLength = length;
    }
    return buffer;
}

/**
 * @brief Main function that benchmarks three CRC-16/CCITT implementations.
 *
 * Command line arguments:
 *    argv[1]: (optional) Number of rounds (default DEFAULT_ROUNDS).
 *    argv[2]: (optional) Input file path. If "-", read from stdin.
 *             If empty, use default test string.
 *             If "rand", generate a random Base32 string.
 *    argv[3]: (optional) If argv[2] is "rand", this is the length of the random string (default 32).
 *
 * If at any point the magic implementation does not match the other implementations,
 * a red flag message is printed to the console.
 */
int main(int argc, char *argv[]) {
    // Seed the random number generator.
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand((unsigned int)(tv.tv_sec * 1000 + tv.tv_usec / 1000));

    // Determine number of rounds.
    int rounds = DEFAULT_ROUNDS;
    if (argc >= 2 && strlen(argv[1]) > 0) {
        rounds = atoi(argv[1]);
        if (rounds <= 0) {
            fprintf(stderr, "Invalid number of rounds specified.\n");
            return EXIT_FAILURE;
        }
    }

    // Determine input data.
    char *data = NULL;
    size_t dataLength = 0;
    if (argc >= 3 && strlen(argv[2]) > 0) {
        if (strcmp(argv[2], "-") == 0) {
            // Read from stdin.
            data = read_stdin(&dataLength);
        } else if (strcmp(argv[2], "rand") == 0) {
            // Generate random Base32 data.
            size_t randLength = 32;  // default length
            if (argc >= 4) {
                randLength = (size_t)atoi(argv[3]);
                if (randLength == 0) {
                    fprintf(stderr, "Invalid random string length specified.\n");
                    return EXIT_FAILURE;
                }
            }
            data = generate_random_base32(randLength);
            dataLength = strlen(data);
        } else {
            // Read from the provided file.
            data = read_file(argv[2], &dataLength);
        }
    } else {
        // No input file provided; use the default test string.
        data = strdup(DEFAULT_TEST_STRING);
        dataLength = strlen(data);
    }

    // Variables to store accumulated timing for each implementation.
    clock_t magicTimeAccum = 0, bitwiseTimeAccum = 0, tableTimeAccum = 0;
    uint16_t crc_magic = 0, crc_bitwise = 0, crc_table = 0;
    bool mismatchFlag = false;

    // Benchmark each round while comparing results.
    for (int i = 0; i < rounds; i++) {
        clock_t t0, t1, t2, t3, t4;

        // Magic implementation timing.
        t0 = clock();
        crc_magic = crc16_magic(0xFFFF, data);
        t1 = clock();
        magicTimeAccum += (t1 - t0);

        // Bitwise implementation timing.
        t2 = clock();
        crc_bitwise = crc16_bitwise((const uint8_t *)data, dataLength);
        t3 = clock();
        bitwiseTimeAccum += (t3 - t2);

        // Table-driven implementation timing.
        t4 = clock();
        crc_table = crc16_table_driven((const uint8_t *)data, dataLength);
        clock_t t5 = clock();
        tableTimeAccum += (t5 - t4);

        // Check if the magic implementation (untrusted) matches the others.
        if (crc_magic != crc_bitwise || crc_magic != crc_table) {
            fprintf(stderr, RED_TEXT "CRC MISMATCH on iteration %d: magic=0x%04X, bitwise=0x%04X, table=0x%04X\n" RESET_TEXT,
                    i, crc_magic, crc_bitwise, crc_table);
            mismatchFlag = true;
            // Optionally break on first mismatch.
            break;
        }
    }

    // Convert clock ticks to seconds.
    double seconds_magic = ((double)magicTimeAccum) / CLOCKS_PER_SEC;
    double seconds_bitwise = ((double)bitwiseTimeAccum) / CLOCKS_PER_SEC;
    double seconds_table = ((double)tableTimeAccum) / CLOCKS_PER_SEC;

    // Print summary of results.
    printf("Input Data (%zu bytes): \"%s\"\n", dataLength, data);
    printf("Rounds: %d\n\n", rounds);
    printf("Final CRC Values:\n");
    printf("  Magic Implementation:   0x%04X\n", crc_magic);
    printf("  Bitwise Implementation: 0x%04X\n", crc_bitwise);
    printf("  Table-driven Implementation: 0x%04X\n\n", crc_table);
    printf("Timing (total over rounds):\n");
    printf("  Magic Implementation:   %f seconds\n", seconds_magic);
    printf("  Bitwise Implementation: %f seconds\n", seconds_bitwise);
    printf("  Table-driven Implementation: %f seconds\n", seconds_table);

    if (!mismatchFlag) {
        printf("\nAll CRC implementations matched.\n");
    }

    free(data);
    return 0;
}
