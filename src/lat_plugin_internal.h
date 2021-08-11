#ifndef __lat_plugin_internal_H__

#include "lat_plugin_interface.h"

#define SELECTOR_SIZE 4

typedef bool (*PluginAvailableCheck)(void);

typedef struct internalLatPlugin_t {
    PluginAvailableCheck availableCheck;
    const uint8_t** selectors;
    uint8_t num_selectors;
    char alias[10];
    PluginCall impl;
} internalLatPlugin_t;

#define NUM_PRC20_SELECTORS 2
extern const uint8_t* const PRC20_SELECTORS[NUM_PRC20_SELECTORS];

#define NUM_PRC721_SELECTORS 1
extern const uint8_t* const PRC721_SELECTORS[NUM_PRC721_SELECTORS];

extern internalLatPlugin_t const INTERNAL_LAT_PLUGINS[];

#endif
