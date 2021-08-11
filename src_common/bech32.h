#ifndef _BECH32_H_
#define _BECH32_H_

void encode(const uint8_t address[20], const char* hrp, char *result);
bool decode(const char* str_address, const char* str_hrp, uint8_t address[20]);

#endif