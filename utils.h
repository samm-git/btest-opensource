#ifndef UTILS_H
#define UTILS_H
#define USERNAME_SIZE 16
#define HASH_SIZE 32
#define AUTHSTR_SIZE 32
// function to generate random Nonce to send to client for md5 hash
void generateRandomNonce(unsigned char nonce[16]);
// This function converts the content of the input buffer to its hexadecimal
void getHexRepresentation(const unsigned char *buffer, size_t size, char *hexString);
// Checks if a C string is not empty.
int isStringNotEmpty(const char *str);
// This function checks authentication based on the provided username and hash.
int isauth(const char *opt_authuser, const unsigned char *username, const char *receivedhash, const char *serverdigest);
#endif