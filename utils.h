#ifndef UTILS_H
#define UTILS_H
// function to generate random Nonce to send to client for md5 hash 
void generateRandomNonce(unsigned char nonce[16]);
// This function converts the content of the input buffer to its hexadecimal
void getHexRepresentation(const unsigned char *buffer, size_t size, char *hexString);
#endif