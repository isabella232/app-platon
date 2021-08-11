
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include "string.h"
#include "stdbool.h"

/** The Bech32 character set for encoding. */
static const char* charset = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

/** The Bech32 character set for decoding. */
static const int8_t charset_rev[128] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, -1, 10, 17, 21, 20, 26, 30, 7,
    5,  -1, -1, -1, -1, -1, -1, -1, 29, -1, 24, 13, 25, 9,  8,  23, -1, 18, 22,
    31, 27, 19, -1, 1,  0,  3,  16, 11, 28, 12, 14, 6,  4,  2,  -1, -1, -1, -1,
    -1, -1, 29, -1, 24, 13, 25, 9,  8,  23, -1, 18, 22, 31, 27, 19, -1, 1,  0,
    3,  16, 11, 28, 12, 14, 6,  4,  2,  -1, -1, -1, -1, -1};

static bool convert_bits(int frombits, int tobits, bool pad, const uint8_t* in, size_t in_len, uint8_t* out, size_t* out_len){
    int acc = 0;
    int bits = 0;
    const int maxv = (1 << tobits) - 1;
    const int max_acc = (1 << (frombits + tobits - 1)) - 1;
    int pos = 0;
    for (size_t i = 0; i < in_len; ++i) {
        int value = in[i];
        acc = ((acc << frombits) | value) & max_acc;
        bits += frombits;
        while (bits >= tobits) {
            bits -= tobits;
            out[pos++] = (acc >> bits) & maxv;
        }
    }
    if (pad) {
        if (bits) out[pos++] = (acc << (tobits - bits)) & maxv;
    } else if (bits >= frombits || ((acc << (tobits - bits)) & maxv)) {
        return false;
    }
    *out_len = pos;
    return true;
}

static size_t expand_hrp(const char* hrp, uint8_t* out){
    size_t hrp_len =  strlen(hrp);
    for(size_t i = 0; i < hrp_len; ++i){
        unsigned char c = hrp[i];
        out[i] = c >> 5;
        out[i + hrp_len + 1] = c & 0x1f;   
    }
    out[hrp_len] = 0;
    return 2 * hrp_len + 1;
}

static size_t cat(const uint8_t *x, size_t x_len, const uint8_t *y, size_t y_len, uint8_t *out){
    size_t out_len = x_len + y_len;
    size_t i = 0;
    for(; i < x_len; ++i){
        out[i] = x[i];
    }

    for(; i < out_len; ++i){
        out[i] = y[i-x_len];
    }

    return out_len;
}

static uint32_t polymod(const uint8_t* values, size_t values_len){
    uint32_t chk = 1;
    for (size_t i = 0; i < values_len; ++i) {
        uint8_t top = chk >> 25;
        chk = (chk & 0x1ffffff) << 5 ^ values[i] ^
            (-((top >> 0) & 1) & 0x3b6a57b2UL) ^
            (-((top >> 1) & 1) & 0x26508e6dUL) ^
            (-((top >> 2) & 1) & 0x1ea119faUL) ^
            (-((top >> 3) & 1) & 0x3d4233ddUL) ^
            (-((top >> 4) & 1) & 0x2a1462b3UL);
    }
    return chk;
}

static size_t create_checksum(const char* hrp, const uint8_t *values, size_t values_len, uint8_t* out){
    uint8_t hrp_bytes[11];
    size_t hrp_bytes_len = expand_hrp(hrp, hrp_bytes);

    uint8_t enc[60] = {};
    size_t enc_len = cat(hrp_bytes, hrp_bytes_len, values, values_len, enc);

    uint32_t mod = polymod(enc, enc_len + 6) ^ 1;

    size_t out_len = 6;
    for (size_t i = 0; i < out_len; ++i) {
        out[i] = (mod >> (5 * (5 - i))) & 31;
    }
    return out_len;
}

static bool verify_checksum(const char* hrp, const uint8_t* values, size_t values_len){
    uint8_t hrp_bytes[11];
    size_t hrp_bytes_len = expand_hrp(hrp, hrp_bytes);

    uint8_t enc[60];
    size_t enc_len = cat(hrp_bytes, hrp_bytes_len, values, values_len, enc);

    return polymod(enc, enc_len) == 1;
}

void encode(const uint8_t address[20], const char* hrp, char *result){
    uint8_t values[40];
    size_t values_len = 0;
    convert_bits(8, 5, true, address, 20, values, &values_len);

    uint8_t check_sum[8];
    size_t check_sum_len = create_checksum(hrp, values, values_len, check_sum);

    uint8_t combined[45] = {0};
    size_t combined_len = cat(values, values_len, check_sum, check_sum_len, combined);

    size_t hrp_len = strlen(hrp);
    strcpy(result, hrp);
    result[hrp_len] = '1';

    for(size_t i = 0; i < combined_len; i++){
        result[hrp_len+1+i] = charset[combined[i]];
    }

    result[hrp_len+1+combined_len] = '\0';
}

bool decode(const char* str_address, const char* str_hrp, uint8_t address[20]){
    bool lower = false, upper = false;
    bool ok = true;

    int len = strlen(str_address);
    for (int i = 0; ok && i < len; ++i){
        unsigned char c = str_address[i];
        if (c < 33 || c > 126) ok = false;
        if (c >= 'a' && c <= 'z') lower = true;
        if (c >= 'A' && c <= 'Z') upper = true;
    }
    if (lower && upper) ok = false;

    int pos = 0;
    for(int i = len -1; i >= 0; --i){
        if('1' == str_address[i]){
            pos = i;
            break;
        }
    }

    if(ok && len <= 90 && pos != 0 && pos >= 1 && pos + 7 <= len){
        uint8_t values[50];
        int values_len = len - 1 - pos;
        for (int i = 0; i < len - 1 - pos; ++i) {
            unsigned char c = str_address[i + pos + 1];
            if (charset_rev[c] == -1) ok = false;
            values[i] = charset_rev[c];
        }
        
        if (ok) {
            char hrp[10] = {};
            for (int i = 0; i < pos; ++i) {
                hrp[i] = (char) tolower((int) str_address[i]);
            }
            if (strcmp(hrp, str_hrp)) {
                return false;
            }

            if (verify_checksum(hrp, values, values_len)) {
                uint8_t conv[25];
                uint8_t temp_values[40];
                size_t temp_values_len = values_len - 6;
                for(int i = 0; i < values_len - 6; ++i){
                    temp_values[i] = values[i];
                }

                size_t conv_len = 0;
                bool ok_convert = convert_bits(5, 8, false, temp_values, temp_values_len, conv, &conv_len);
                if (ok_convert && 20 == conv_len) {
                    memcpy(address, conv, conv_len);
                }
            }
        }
    }

    return true;
}


