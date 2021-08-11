#include "shared_context.h"
#include "string.h"

uint32_t set_result_get_publicKey() {
    uint32_t tx = 0;
    G_io_apdu_buffer[tx++] = 65;
    memmove(G_io_apdu_buffer + tx, tmpCtx.publicKeyContext.publicKey.W, 65);
    tx += 65;
    uint8_t address_length = strlen((char *)tmpCtx.publicKeyContext.address);
    G_io_apdu_buffer[tx++] = address_length;
    memmove(G_io_apdu_buffer + tx, tmpCtx.publicKeyContext.address, address_length);
    tx += address_length;
    if (tmpCtx.publicKeyContext.getChaincode) {
        memmove(G_io_apdu_buffer + tx, tmpCtx.publicKeyContext.chainCode, 32);
        tx += 32;
    }
    return tx;
}
