#ifndef __lat_plugin_handler_H__

#include "lat_plugin_interface.h"

void lat_plugin_prepare_init(latPluginInitContract_t *init, uint8_t *selector, uint32_t dataSize);
void lat_plugin_prepare_provide_parameter(latPluginProvideParameter_t *provideParameter,
                                          uint8_t *parameter,
                                          uint32_t parameterOffset);
void lat_plugin_prepare_finalize(latPluginFinalize_t *finalize);
void lat_plugin_prepare_provide_token(latPluginProvideToken_t *provideToken,
                                      tokenDefinition_t *token1,
                                      tokenDefinition_t *token2);
void lat_plugin_prepare_query_contract_ID(ethQueryContractID_t *queryContractID,
                                          char *name,
                                          uint32_t nameLength,
                                          char *version,
                                          uint32_t versionLength);
void lat_plugin_prepare_query_contract_UI(ethQueryContractUI_t *queryContractUI,
                                          uint8_t screenIndex,
                                          char *title,
                                          uint32_t titleLength,
                                          char *msg,
                                          uint32_t msgLength);

lat_plugin_result_t lat_plugin_perform_init(uint8_t *contractAddress,
                                            latPluginInitContract_t *init);
// NULL for cached address, or base contract address
lat_plugin_result_t lat_plugin_call(int method, void *parameter);
int compound_plugin_call(uint8_t *contractAddress, int method, void *parameter);

void plugin_ui_start(void);

#endif
