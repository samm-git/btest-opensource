#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "utils.h"

/**
 * generateRandomNonce - Generates a random nonce.
 * @nonce: The array to store the generated nonce.
 */
void generateRandomNonce(unsigned char nonce[16])
{
    srand((unsigned int)time(NULL));
    for (int i = 0; i < 16; ++i)
    {
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
void getHexRepresentation(const unsigned char *buffer, size_t size, char *hexString)
{
    for (size_t i = 0; i < size; ++i)
    {
        sprintf(hexString + i * 2, "%02x", buffer[i]);
    }
}
/**
 * isStringNotEmpty - Checks if a C string is not empty.
 * @str: Pointer to the C string to be checked.
 *
 * This function determines whether the given C string is not empty.
 * It returns 1 if the string is not empty and not NULL, and 0 otherwise.
 *
 * Returns: 1 if the string is not empty, 0 otherwise.
 */
int isStringNotEmpty(const char *str)
{
    return (str != NULL && strlen(str) > 0);
}
/**
 * isauth - Check authentication based on user and hash.
 * @opt_authuser: User-provided authentication username.
 * @username: Server's expected username for comparison.
 * @receivedhash: User-provided received hash for comparison.
 * @serverdigest: Server's expected hash for comparison.
 *
 * It compares opt_authuser with username and receivedhash with serverdigest.
 * Returns: 1 if both username and hash match, 0 otherwise.
 */
int isauth(const char *opt_authuser, const unsigned char *username,
           const char *receivedhash, const char *serverdigest)
{
    // Check if opt_authuser is not empty and equal to username
    int userMatch = (opt_authuser != NULL && *opt_authuser != '\0' &&
                     strncmp(opt_authuser, (const char *)username, USERNAME_SIZE) == 0);

    // Check if receivedhash is not empty and equal to serverdigest
    int hashMatch = (receivedhash != NULL && *receivedhash != '\0' &&
                     strcmp(receivedhash, serverdigest) == 0);

    // Return 1 if both conditions are true, 0 otherwise
    return (userMatch && hashMatch);
}