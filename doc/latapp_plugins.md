# Platon application Plugins : Technical Specifications

## About

This specification describes the plugin interface used to display a specific UI on device for Platon smart contracts.

Feel free to checkout the ParaSwap plugin to see an actual implementation. Link: https://github.com/LedgerHQ/app-Ethereum/blob/named-external-plugins/doc/ethapp_plugins.asc .

## Flow overview

When signing an Platon transaction containing data, the Platon application looks for a plugin using .either a selector list or the contract address.

If a plugin is found, each network serialized data field (32 bytes) is passed to the plugin along with the field offset. The plugin can decide to stop the signature process if a data field isn't expected

After all fields have been received, the plugin can report to the Platon application whether the full data is accepted, and the user interface model that'll be used to display the data

### Amount/Address user interface

In this model, the generic (without data) transaction display is used, with the amount and destination address replaced by data provided by the plugin

### Generic user interface

In this model, the plugin first reports a number of screens (2 lines of text, the second line being scrollable) to be displayed 

The Platon application will request each screen to be displayed to the plugin and let the user browse through them.

The first screen being displayed is always a description of the plugin being used (name and version reported by the plugin), and the last screens include the transaction fees in ETH and a confirmation prompt

### Code flow

The plugin interfacing logic is described in _src/eth_plugin_interface.h_

The plugin common dispatcher is found in _src/eth_plugin_handler.c_

The plugin generic UI dispatcher is found in _src/eth_plugin_ui.c_

Sample internal plugins are provided in _src_plugins/_

## Creating a plugin

### Creating an internal plugin

Internal plugins are triggered on specific selectors. You can modify _src/eth_plugin_internal.c_ to add your mapping.

Other specific mappings can be also added by modifying the common dispatcher

### Creating an external plugin

An external plugin is a library application named after the base64 encoding of the 20 bytes smart contract address 

## Detailed flow messages

### Generic fields

The following generic fields are present in all messages :

* pluginSharedRW : scratch objects and utilities available to the plugin (can be read and written)

* pluginSharedRO : transaction data available to the plugin (can only be read)

* pluginContext : arbitrary data blob holding the plugin context, to be set and used by the plugin

* result : return code set by the plugin following the message processing

### LAT_PLUGIN_INIT_CONTRACT

```c

typedef struct latPluginInitContract_t {

  // in

  latPluginSharedRW_t *pluginSharedRW;
  latPluginSharedRO_t *pluginSharedRO;
  uint8_t *pluginContext;
  uint32_t pluginContextLength;
  uint8_t *selector; // 4 bytes selector
  uint32_t dataSize;

  char *alias; // 29 bytes alias if LAT_PLUGIN_RESULT_OK_ALIAS set

  uint8_t result;

} latPluginInitContract_t;

```

This message is sent when the selector of the data has been parsed. The following specific fields are filled when the plugin is called :

* pluginContextLength : length of the data field available to store the plugin context
* selector : 4 bytes selector of the data field
* dataSize : size in bytes of the data field

The following return codes are expected, any other will abort the signing process :

* LAT_PLUGIN_RESULT_OK : if the plugin can be successfully initialized
* LAT_PLUGIN_RESULT_OK_ALIAS : if a base64 encoded alias of another plugin to call is copied to the _alias_ field. In this case, the dispatcher will follow the alias chain, and the original plugin will only be called to retrieve its name when using a generic user interface
* LAT_PLUGIN_RESULT_FALLBACK : if the signing logic should fallback to the generic one

### LAT_PLUGIN_PROVIDE_PARAMETER

```c

typedef struct latPluginProvideParameter_t {

  latPluginSharedRW_t *pluginSharedRW;
  latPluginSharedRO_t *pluginSharedRO;
  uint8_t *pluginContext;
  uint8_t *parameter; // 32 bytes parameter
  uint32_t parameterOffset;

  uint8_t result;

} latPluginProvideParameter_t;

```

This message is sent when a new 32 bytes component of the data field is available. The following specific fields are filled when the plugin is called :

* parameter : pointer to the 32 bytes parameter being parsed
* parameterOffset : offset to this parameter from the beginning of the data field (starts at 4, following the selector)

The following return codes are expected, any other will abort the signing process : 

* LAT_PLUGIN_RESULT_OK : if the plugin can be successfully initialized
* LAT_PLUGIN_RESULT_FALLBACK : if the signing logic should fallback to the generic one

### LAT_PLUGIN_FINALIZE

```c

typedef struct latPluginFinalize_t {

  latPluginSharedRW_t *pluginSharedRW;
  latPluginSharedRO_t *pluginSharedRO;
  uint8_t *pluginContext;

  uint8_t *tokenLookup1; // set by the plugin if a token should be looked up
  uint8_t *tokenLookup2; 

  uint8_t *amount; // set an uint256 pointer if uiType is UI_AMOUNT_ADDRESS
  uint8_t *address;  // set to the destination address if uiType is UI_AMOUNT_ADDRESS. Set to the user's address if uiType is UI_TYPE_GENERIC

  uint8_t uiType; 
  uint8_t numScreens; // ignored if uiType is UI_AMOUNT_ADDRESS
  uint8_t result;

} latPluginFinalize_t;

```

This message is sent when the data field has been fully parsed. The following specific fields can be filled by the plugin : 

* tokenLookup1 : the pointer shall be set to a 20 bytes address to look up an ERC 20 token descriptor if needed by the plugin
* tokenLookup2 : the pointer shall be set to a 20 bytes address to look up an ERC 20 token descriptor if needed by the plugin  
* uiType : set to either LAT_UI_TYPE_AMOUNT_ADDRESS for an amount/address UI or LAT_UI_TYPE_GENERIC for a generic UI

The following specific fields are filled by the plugin when returning an amount/address UI :

* amount : set to a pointer to a 256 bits number
* address : set to a pointer to a 20 bytes address

The following specific fields are filled by the plugin when returning a generic UI :  

* numScreens : number of screens handled by the plugin

The following return codes are expected, any other will abort the signing process : 

* LAT_PLUGIN_RESULT_OK : if the plugin can be successfully initialized
* LAT_PLUGIN_RESULT_FALLBACK : if the signing logic should fallback to the generic one

### LAT_PLUGIN_PROVIDE_TOKEN

```c

typedef struct latPluginProvideToken_t {

  latPluginSharedRW_t *pluginSharedRW;
  latPluginSharedRO_t *pluginSharedRO;
  uint8_t *pluginContext;

  tokenDefinition_t *token1; // set by the ETH application, to be saved by the plugin
  tokenDefinition_t *token2;

  uint8_t additionalScreens; // Used by the plugin if it needs to display additional screens based on the information received from the token definitions.

  uint8_t result;

} latPluginProvideToken_t;

```

This message is sent if a token lookup was required by the plugin when parsing a finalize message. The following specific fields are filled when the plugin is called :

* token1 : pointer to a token definition matching tokenLookup1, or NULL if not found 
* token2 : pointer to a token definition matching tokenLookup2, or NULL if not found or not requested

The following return codes are expected, any other will abort the signing process : 

* LAT_PLUGIN_RESULT_OK : if the plugin can be successfully initialized
* LAT_PLUGIN_RESULT_FALLBACK : if the signing logic should fallback to the generic one

### LAT_PLUGIN_QUERY_CONTRACT_ID

```c

typedef struct ethQueryContractID_t {

  latPluginSharedRW_t *pluginSharedRW;
  latPluginSharedRO_t *pluginSharedRO;
  uint8_t *pluginContext;

  char *name;
  uint32_t nameLength;
  char *version;
  uint32_t versionLength;

  uint8_t result; 

} ethQueryContractID_t;

```

This message is sent after the parsing finalization and token lookups if requested if a generic UI is used. The following specific fields are provided when the plugin is called :

* name : pointer to the name of the plugin, to be filled by the plugin
* nameLength : maximum name length
* version : pointer to the version of the plugin, to be filled by the plugin
* versionLength : maximum version length

The following return codes are expected, any other will abort the signing process : 

* LAT_PLUGIN_RESULT_OK : if the plugin can be successfully initialized

### LAT_PLUGIN_QUERY_CONTRACT_UI

```c

typedef struct ethQueryContractUI_t {

  latPluginSharedRW_t *pluginSharedRW;
  latPluginSharedRO_t *pluginSharedRO;
  uint8_t *pluginContext;
  uint8_t screenIndex;
  char *title;
  uint32_t titleLength;
  char *msg;
  uint32_t msgLength;

  uint8_t result;

} ethQueryContractUI_t;

```

This message is sent when a plugin screen shall be displayed if a generic UI is used. The following specific fields are provided when the plugin is called : 

* screenIndex : index of the screen to display, starting from 0
* title : pointer to the first line of the screen, to be filled by the plugin
* titleLength : maximum title length
* msg : pointer to the second line of the screen, to be filled by the plugin
* msgLength : maximum msg length

The following return codes are expected, any other will abort the signing process : 

* LAT_PLUGIN_RESULT_OK : if the plugin can be successfully initialized

## Caveats

When setting a pointer from the plugin space, make sure to use an address that will be accessible from the Platon application (typically in the plugin RAM context, *not* on the plugin stack)

Do not use data types that need to be aligned (such as uint32_t) in the plugin context.

## TODOs

Provide a sample callback mechanism for common plugin actions (amount to string, 256 bits number multiplication ...) to avoid duplicating code in the plugin space

Provide external plugins samples

Fully support Starkware as an independant application (APDU logic added)

Support extra flags for the generic UI (fast confirmation on first screen, ...)

Support extra plugin provisioning (signed list of associated smart contract addresses, ...)
