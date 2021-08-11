#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "network.h"
#include "os.h"
#include "shared_context.h"
#include "utils.h"

// Mappping of chain ids to networks.
const network_info_t NETWORK_MAPPING[] = {
    {.chain_id = 100, .name = "Platon", .ticker = "LAT "}
};

uint32_t get_chain_id(void) {
    uint32_t chain_id = 0;

    switch (txContext.txType) {
        case LEGACY:
            chain_id = u32_from_BE(txContext.content->v, txContext.content->vLength, true);
            break;
        case EIP2930:
            chain_id = u32_from_BE(tmpContent.txContent.chainID.value,
                                   tmpContent.txContent.chainID.length,
                                   true);
            break;
        default:
            PRINTF("Txtype `%d` not supported while generating chainID\n", txContext.txType);
            break;
    }
    PRINTF("ChainID: %d\n", chain_id);
    return chain_id;
}

network_info_t *get_network(void) {
    uint32_t chain_id = get_chain_id();
    for (uint8_t i = 0; i < sizeof(NETWORK_MAPPING) / sizeof(*NETWORK_MAPPING); i++) {
        if (NETWORK_MAPPING[i].chain_id == chain_id) {
            return (network_info_t *) PIC(&NETWORK_MAPPING[i]);
        }
    }
    return NULL;
}

char *get_network_name(void) {
    network_info_t *network = get_network();
    if (network == NULL) {
        return NULL;
    } else {
        return (char *) PIC(network->name);
    }
}

char *get_network_ticker(void) {
    network_info_t *network = get_network();
    if (network == NULL) {
        return chainConfig->coinName;
    } else {
        return (char *) PIC(network->ticker);
    }
}
