#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * generateRandomNonce - Generates a random nonce.
 * @nonce: The array to store the generated nonce.
 */
void generateRandomNonce(unsigned char nonce[16]) {
    srand((unsigned int)time(NULL));
    for (int i = 0; i < 16; ++i) {
        nonce[i] = (unsigned char)rand();
    }
}
/**
 * getHexRepresentation - Converts a buffer to its hexadecimal representation.
 * @buffer: Pointer to the input buffer.
 * @size: Size of the input buffer.
 * @hexString: Pointer to the output string for the hexadecimal representation.
 *
 * This function converts the content of the input buffer to its hexadecimal
 * representation and stores it in the output string. The resulting string is
 * null-terminated.
 */
void getHexRepresentation(const unsigned char *buffer, size_t size, char *hexString) {
    for (size_t i = 0; i < size; ++i) {
        sprintf(hexString + i * 2, "%02x", buffer[i]);
    }
}