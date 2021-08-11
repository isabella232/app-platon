#include <string.h>
#include "lat_plugin_handler.h"
#include "lat_plugin_internal.h"
#include "shared_context.h"

void lat_plugin_prepare_init(latPluginInitContract_t *init, uint8_t *selector, uint32_t dataSize) {
    memset((uint8_t *) init, 0, sizeof(latPluginInitContract_t));
    init->selector = selector;
    init->dataSize = dataSize;
}

void lat_plugin_prepare_provide_parameter(latPluginProvideParameter_t *provideParameter,
                                          uint8_t *parameter,
                                          uint32_t parameterOffset) {
    memset((uint8_t *) provideParameter, 0, sizeof(latPluginProvideParameter_t));
    provideParameter->parameter = parameter;
    provideParameter->parameterOffset = parameterOffset;
}

void lat_plugin_prepare_finalize(latPluginFinalize_t *finalize) {
    memset((uint8_t *) finalize, 0, sizeof(latPluginFinalize_t));
}

void lat_plugin_prepare_provide_token(latPluginProvideToken_t *provideToken,
                                      tokenDefinition_t *token1,
                                      tokenDefinition_t *token2) {
    memset((uint8_t *) provideToken, 0, sizeof(latPluginProvideToken_t));
    provideToken->token1 = token1;
    provideToken->token2 = token2;
}

void lat_plugin_prepare_query_contract_ID(ethQueryContractID_t *queryContractID,
                                          char *name,
                                          uint32_t nameLength,
                                          char *version,
                                          uint32_t versionLength) {
    memset((uint8_t *) queryContractID, 0, sizeof(ethQueryContractID_t));
    queryContractID->name = name;
    queryContractID->nameLength = nameLength;
    queryContractID->version = version;
    queryContractID->versionLength = versionLength;
}

void lat_plugin_prepare_query_contract_UI(ethQueryContractUI_t *queryContractUI,
                                          uint8_t screenIndex,
                                          char *title,
                                          uint32_t titleLength,
                                          char *msg,
                                          uint32_t msgLength) {
    memset((uint8_t *) queryContractUI, 0, sizeof(ethQueryContractUI_t));
    queryContractUI->screenIndex = screenIndex;
    queryContractUI->title = title;
    queryContractUI->titleLength = titleLength;
    queryContractUI->msg = msg;
    queryContractUI->msgLength = msgLength;
}

lat_plugin_result_t lat_plugin_perform_init(uint8_t *contractAddress,
                                            latPluginInitContract_t *init) {
    uint8_t i;
    const uint8_t **selectors;
    dataContext.tokenContext.pluginStatus = LAT_PLUGIN_RESULT_UNAVAILABLE;

    PRINTF("Selector %.*H\n", 4, init->selector);
    if (externalPluginIsSet) {
        // check if the registered external plugin matches the TX contract address / method selector
        if (memcmp(contractAddress,
                   dataContext.tokenContext.contract_address,
                   sizeof(dataContext.tokenContext.contract_address)) != 0) {
            os_sched_exit(0);
        }
        if (memcmp(init->selector,
                   dataContext.tokenContext.method_selector,
                   sizeof(dataContext.tokenContext.method_selector)) != 0) {
            os_sched_exit(0);
        }
        PRINTF("External plugin will be used\n");
        dataContext.tokenContext.pluginStatus = LAT_PLUGIN_RESULT_OK;
        contractAddress = NULL;
    } else {
        // Search internal plugin list
        for (i = 0;; i++) {
            uint8_t j;
            selectors = (const uint8_t **) PIC(INTERNAL_LAT_PLUGINS[i].selectors);
            if (selectors == NULL) {
                break;
            }
            for (j = 0; ((j < INTERNAL_LAT_PLUGINS[i].num_selectors) && (contractAddress != NULL));
                 j++) {
                if (memcmp(init->selector, (const void *) PIC(selectors[j]), SELECTOR_SIZE) == 0) {
                    if ((INTERNAL_LAT_PLUGINS[i].availableCheck == NULL) ||
                        ((PluginAvailableCheck) PIC(INTERNAL_LAT_PLUGINS[i].availableCheck))()) {
                        strcpy(dataContext.tokenContext.pluginName, INTERNAL_LAT_PLUGINS[i].alias);
                        dataContext.tokenContext.pluginStatus = LAT_PLUGIN_RESULT_OK;
                        contractAddress = NULL;
                        break;
                    }
                }
            }
        }
    }

    // Do not handle a plugin if running in swap mode
    if (called_from_swap && (contractAddress != NULL)) {
        PRINTF("eth_plug_init aborted in swap mode\n");
        return 0;
    }

    lat_plugin_result_t status = LAT_PLUGIN_RESULT_UNAVAILABLE;

    if (contractAddress != NULL) {
        PRINTF("No plugin available for %.*H\n", 20, contractAddress);
        return status;
    }

    PRINTF("lat_plugin_init\n");
    PRINTF("Trying plugin %s\n", dataContext.tokenContext.pluginName);
    status = lat_plugin_call(LAT_PLUGIN_INIT_CONTRACT, (void *) init);

    if (status <= LAT_PLUGIN_RESULT_UNSUCCESSFUL) {
        return status;
    }
    PRINTF("lat_plugin_init ok %s\n", dataContext.tokenContext.pluginName);
    dataContext.tokenContext.pluginStatus = LAT_PLUGIN_RESULT_OK;
    return LAT_PLUGIN_RESULT_OK;
}

lat_plugin_result_t lat_plugin_call(int method, void *parameter) {
    latPluginSharedRW_t pluginRW;
    latPluginSharedRO_t pluginRO;
    char *alias;
    uint8_t i;
    uint8_t internalPlugin = 0;

    pluginRW.sha3 = &global_sha3;
    pluginRO.txContent = &tmpContent.txContent;

    if (dataContext.tokenContext.pluginStatus <= LAT_PLUGIN_RESULT_UNSUCCESSFUL) {
        PRINTF("Cached plugin call but no plugin available\n");
        return dataContext.tokenContext.pluginStatus;
    }
    alias = dataContext.tokenContext.pluginName;

    // Prepare the call

    switch (method) {
        case LAT_PLUGIN_INIT_CONTRACT:
            ((latPluginInitContract_t *) parameter)->interfaceVersion =
                LAT_PLUGIN_interface_VERSION_1;
            ((latPluginInitContract_t *) parameter)->result = LAT_PLUGIN_RESULT_UNAVAILABLE;
            ((latPluginInitContract_t *) parameter)->pluginSharedRW = &pluginRW;
            ((latPluginInitContract_t *) parameter)->pluginSharedRO = &pluginRO;
            ((latPluginInitContract_t *) parameter)->pluginContext =
                (uint8_t *) &dataContext.tokenContext.pluginContext;
            ((latPluginInitContract_t *) parameter)->pluginContextLength =
                sizeof(dataContext.tokenContext.pluginContext);
            ((latPluginInitContract_t *) parameter)->alias = dataContext.tokenContext.pluginName;
            break;
        case LAT_PLUGIN_PROVIDE_PARAMETER:
            ((latPluginProvideParameter_t *) parameter)->result = LAT_PLUGIN_RESULT_UNAVAILABLE;
            ((latPluginProvideParameter_t *) parameter)->pluginSharedRW = &pluginRW;
            ((latPluginProvideParameter_t *) parameter)->pluginSharedRO = &pluginRO;
            ((latPluginProvideParameter_t *) parameter)->pluginContext =
                (uint8_t *) &dataContext.tokenContext.pluginContext;
            break;
        case LAT_PLUGIN_FINALIZE:
            ((latPluginFinalize_t *) parameter)->result = LAT_PLUGIN_RESULT_UNAVAILABLE;
            ((latPluginFinalize_t *) parameter)->pluginSharedRW = &pluginRW;
            ((latPluginFinalize_t *) parameter)->pluginSharedRO = &pluginRO;
            ((latPluginFinalize_t *) parameter)->pluginContext =
                (uint8_t *) &dataContext.tokenContext.pluginContext;
            break;
        case LAT_PLUGIN_PROVIDE_TOKEN:
            ((latPluginProvideToken_t *) parameter)->result = LAT_PLUGIN_RESULT_UNAVAILABLE;
            ((latPluginProvideToken_t *) parameter)->pluginSharedRW = &pluginRW;
            ((latPluginProvideToken_t *) parameter)->pluginSharedRO = &pluginRO;
            ((latPluginProvideToken_t *) parameter)->pluginContext =
                (uint8_t *) &dataContext.tokenContext.pluginContext;
            break;
        case LAT_PLUGIN_QUERY_CONTRACT_ID:
            ((ethQueryContractID_t *) parameter)->result = LAT_PLUGIN_RESULT_UNAVAILABLE;
            ((ethQueryContractID_t *) parameter)->pluginSharedRW = &pluginRW;
            ((ethQueryContractID_t *) parameter)->pluginSharedRO = &pluginRO;
            ((ethQueryContractID_t *) parameter)->pluginContext =
                (uint8_t *) &dataContext.tokenContext.pluginContext;
            break;
        case LAT_PLUGIN_QUERY_CONTRACT_UI:
            ((ethQueryContractUI_t *) parameter)->pluginSharedRW = &pluginRW;
            ((ethQueryContractUI_t *) parameter)->pluginSharedRO = &pluginRO;
            ((ethQueryContractUI_t *) parameter)->pluginContext =
                (uint8_t *) &dataContext.tokenContext.pluginContext;
            break;
        default:
            PRINTF("Unknown plugin method %d\n", method);
            return LAT_PLUGIN_RESULT_UNAVAILABLE;
    }

    // Perform the call

    for (i = 0;; i++) {
        if (INTERNAL_LAT_PLUGINS[i].alias[0] == 0) {
            break;
        }
        if (strcmp(alias, INTERNAL_LAT_PLUGINS[i].alias) == 0) {
            internalPlugin = 1;
            ((PluginCall) PIC(INTERNAL_LAT_PLUGINS[i].impl))(method, parameter);
            break;
        }
    }

    if (!internalPlugin) {
        uint32_t params[3];
        params[0] = (uint32_t) alias;
        params[1] = method;
        params[2] = (uint32_t) parameter;
        BEGIN_TRY {
            TRY {
                os_lib_call(params);
            }
            CATCH_OTHER(e) {
                PRINTF("Plugin call exception for %s\n", alias);
            }
            FINALLY {
            }
        }
        END_TRY;
    }

    // Check the call result
    PRINTF("method: %d\n", method);
    switch (method) {
        case LAT_PLUGIN_INIT_CONTRACT:
            PRINTF("parameter result: %d\n", ((latPluginInitContract_t *) parameter)->result);
            switch (((latPluginInitContract_t *) parameter)->result) {
                case LAT_PLUGIN_RESULT_OK:
                    break;
                case LAT_PLUGIN_RESULT_ERROR:
                    return LAT_PLUGIN_RESULT_ERROR;
                default:
                    return LAT_PLUGIN_RESULT_UNAVAILABLE;
            }
            break;
        case LAT_PLUGIN_PROVIDE_PARAMETER:
            switch (((latPluginProvideParameter_t *) parameter)->result) {
                case LAT_PLUGIN_RESULT_OK:
                case LAT_PLUGIN_RESULT_FALLBACK:
                    break;
                case LAT_PLUGIN_RESULT_ERROR:
                    return LAT_PLUGIN_RESULT_ERROR;
                default:
                    return LAT_PLUGIN_RESULT_UNAVAILABLE;
            }
            break;
        case LAT_PLUGIN_FINALIZE:
            switch (((latPluginFinalize_t *) parameter)->result) {
                case LAT_PLUGIN_RESULT_OK:
                case LAT_PLUGIN_RESULT_FALLBACK:
                    break;
                case LAT_PLUGIN_RESULT_ERROR:
                    return LAT_PLUGIN_RESULT_ERROR;
                default:
                    return LAT_PLUGIN_RESULT_UNAVAILABLE;
            }
            break;
        case LAT_PLUGIN_PROVIDE_TOKEN:
            switch (((latPluginProvideToken_t *) parameter)->result) {
                case LAT_PLUGIN_RESULT_OK:
                case LAT_PLUGIN_RESULT_FALLBACK:
                    break;
                case LAT_PLUGIN_RESULT_ERROR:
                    return LAT_PLUGIN_RESULT_ERROR;
                default:
                    return LAT_PLUGIN_RESULT_UNAVAILABLE;
            }
            break;
        case LAT_PLUGIN_QUERY_CONTRACT_ID:
            if (((ethQueryContractID_t *) parameter)->result <= LAT_PLUGIN_RESULT_UNSUCCESSFUL) {
                return LAT_PLUGIN_RESULT_UNAVAILABLE;
            }
            break;
        case LAT_PLUGIN_QUERY_CONTRACT_UI:
            if (((ethQueryContractUI_t *) parameter)->result <= LAT_PLUGIN_RESULT_OK) {
                return LAT_PLUGIN_RESULT_UNAVAILABLE;
            }
            break;
        default:
            return LAT_PLUGIN_RESULT_UNAVAILABLE;
    }

    return LAT_PLUGIN_RESULT_OK;
}
