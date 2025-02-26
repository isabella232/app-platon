# Platon application : Common Technical Specifications

This application describes the APDU messages interface to communicate with the Platon application.

The application covers the following functionalities :

- Retrieve a public Platon address given a BIP 32 path
- Sign a basic Platon transaction given a BIP 32 path
- Provide callbacks to validate the data associated to an Platon transaction

The application interface can be accessed over HID or BLE

## General purpose APDUs

### GET PLATON PUBLIC ADDRESS

#### Description

This command returns the public key and Platon address for the given BIP 32 path.

The address can be optionally checked on the device before being returned.

#### Coding

##### command

| *CLA* | *INS*   | *P1*                | *P2*        | *Lc*      | *Le*   |
| ---   | ---     |   ---               | ---         | ---       | ---    |
|   E0  |   02    |  00 : return address <br> 01 : display address and confirm before returning |   00 : do not return the chain code <br> 01 : return the chain code | variable | variable

##### Input data

| *Description*                                                                     | *Length* |
| ---   | ---     |
| Number of BIP 32 derivations to perform (max 10)                                  | 1 |
| First derivation index (big endian)                                               | 4 |
| ...                                                                               | 4 |
| Last derivation index (big endian)                                                | 4 |

##### Output data

| *Description*                                                                     | *Length* |
| ---   | ---     |
| Public Key length                                                                 | 1 |
| Uncompressed Public Key                                                           | var |
| Platon address length                                                           | 1 |
| Platon address                                                                  | var |
| Chain code if requested                                                           | 32 |

### SIGN PLATON TRANSACTION

#### Description

This command signs an Platon transaction after having the user validate the following parameters

- Gas price
- Gas limit
- Recipient address
- Value

The input data is the RLP encoded transaction (as per https://github.com/ethereum/pyethereum/blob/develop/ethereum/transactions.py#L22), without v/r/s present, streamed to the device in 255 bytes maximum data chunks.

#### Coding

##### command

| *CLA* | *INS*  | *P1*               | *P2*       | *Lc*     | *Le*   |
| ---   | ---     |   ---                 | ---         | ---       | ---    |
|   E0  |   04   |  00 : first transaction data block <br> 80 : subsequent transaction data block |   00 | variable | variable |

##### Input data (first transaction data block)

| *Description*                                                                     | *Length* |
| ---   | ---     |
| Number of BIP 32 derivations to perform (max 10)                                  | 1 |
| First derivation index (big endian)                                               | 4 |
| ...                                                                               | 4 |
| Last derivation index (big endian)                                                | 4 |
| RLP transaction chunk                                                             | variable |

##### Input data (other transaction data block)

| *Description*                                                                     | *Length* |
| ---   | ---     |
| RLP transaction chunk                                                             | variable |

##### Output data

| *Description*                                                                     | *Length* |
| ---   | ---     |
| v                                                                                 | 1 |
| r                                                                                 | 32 |
| s                                                                                 | 32 |

### SIGN PLATON PERSONAL MESSAGE

#### Description

This command signs an Platon message following the personal_sign specification (https://github.com/ethereum/go-ethereum/pull/2940) after having the user validate the SHA-256 hash of the message being signed.

This command has been supported since firmware version 1.0.8

The input data is the message to sign, streamed to the device in 255 bytes maximum data chunks

#### Coding

##### command

| *CLA* | *INS*  | *P1*               | *P2*       | *Lc*     | *Le*   |
| ---   | ---     |   ---                 | ---         | ---       | ---    |
|   E0  |   08   |  00 : first message data block <br> 80 : subsequent message data block |   00       | variable | variable |

##### Input data (first message data block)

| *Description*                                                                     | *Length* |
| ---   | ---     |
| Number of BIP 32 derivations to perform (max 10)                                  | 1 |
| First derivation index (big endian)                                               | 4 |
| ...                                                                               | 4 |
| Last derivation index (big endian)                                                | 4 |
| Message length                                                                    | 4 |
| Message chunk                                                                     | variable |

##### Input data (other transaction data block)

| *Description*                                                                     | *Length* |
| ---   | ---     |
| Message chunk                                                                     | variable |

##### Output data

| *Description*                                                                     | *Length* |
| ---   | ---     |
| v                                                                                 | 1 |
| r                                                                                 | 32 |
| s                                                                                 | 32 |

### PROVIDE PRC 20 TOKEN INFORMATION

#### Description

This commands provides a trusted description of an PRC 20 token to associate a contract address with a ticker and number of decimals. 

It shall be run immediately before performing a transaction involving a contract calling this contract address to display the proper token information to the user if necessary, as marked in GET APP CONFIGURATION flags.

The signature is computed on

ticker || address || number of decimals (uint4be) || chainId (uint4be)

signed by the following secp256k1 public key 0482bbf2f34f367b2e5bc21847b6566f21f0976b22d3388a9a5e446ac62d25cf725b62a2555b2dd464a4da0ab2f4d506820543af1d242470b1b1a969a27578f353

#### Coding

##### command

| *CLA* | *INS*  | *P1*               | *P2*       | *Lc*     | *Le*   |
| ---   | ---     |   ---                 | ---         | ---       | ---    |
|   E0  |   0A   |  00   |   00       | variable | 00 |

##### Input data

| *Description*                                                                     | *Length* |
| ---   | ---     |
| Length of PRC 20 ticker                                                           | 1 |
| PRC 20 ticker                                                                     | variable |
| PRC 20 contract address                                                           | 20 |
| Number of decimals (big endian encoded)                                           | 4 |
| Chain ID (big endian encoded)                                                     | 4 |
| Token information signature                                                       | variable |

##### Output data

None

### SET EXTERNAL PLUGIN

#### Description

This commands provides the name of a trusted binding of a plugin with a contract address and a supported method selector. This plugin will be called to interpret contract data in the following transaction signing command.

It shall be run immediately before performing a transaction involving a contract supported by this plugin to display the proper information to the user if necessary.

The function returns an error sw (0x6984) if the plugin requested is not installed on the device, 0x9000 otherwise.

The signature is computed on

len(pluginName) || pluginName || contractAddress || methodSelector

signed by the following secp256k1 public key 0482bbf2f34f367b2e5bc21847b6566f21f0976b22d3388a9a5e446ac62d25cf725b62a2555b2dd464a4da0ab2f4d506820543af1d242470b1b1a969a27578f353

#### Coding

##### command

| *CLA* | *INS*  | *P1*               | *P2*       | *Lc*     | *Le* |
| ---   | ---     |   ---                 | ---         | ---       | ---    |
|   E0  |   12   |  00   |   00       | variable   | 00 |

##### Input data

| *Description*                                                                     | *Length* |
| ---   | ---     |
| Length of plugin name                                                             | 1 |
| plugin name                                                                       | variable |
| contract address                                                                  | 20 |
| method selector                                                                   | 4 |
| signature                                                                         | variable |

##### Output data

None

### GET APP CONFIGURATION

#### Description

This command returns specific application configuration

#### Coding

##### command
| *CLA* | *INS*  | *P1*               | *P2*       | *Lc*     | *Le*   |
| ---   | ---     |   ---                 | ---         | ---       | ---    |
|   E0  |   06   |  00                |   00       | 00       | 04 |

##### Input data

None

##### Output data

| *Description*                                                                     | *Length* |
| ---   | ---     |
| Flags  |   0x01 : arbitrary data signature enabled by user <br>  0x02 : PRC 20 Token information needs to be provided externally |                                                                                  | 01 |
| Application major version                                                         | 01 |
| Application minor version                                                         | 01 |
| Application patch version                                                         | 01 |

## Transport protocol

### General transport description

Ledger APDUs requests and responses are encapsulated using a flexible protocol allowing to fragment large payloads over different underlying transport mechanisms.

The common transport header is defined as follows :

| *Description*                                                                     | *Length* |
| ---   | ---     |
| Communication channel ID (big endian)                                             | 2 |
| Command tag                                                                       | 1 |
| Packet sequence index (big endian)                                                | 2 |
| Payload                                                                           | var |

The Communication channel ID allows commands multiplexing over the same physical link. It is not used for the time being, and should be set to 0101 to avoid compatibility issues with implementations ignoring a leading 00 byte.

The Command tag describes the message content. Use TAG_APDU (0x05) for standard APDU payloads, or TAG_PING (0x02) for a simple link test.

The Packet sequence index describes the current sequence for fragmented payloads. The first fragment index is 0x00.

### APDU Command payload encoding

APDU Command payloads are encoded as follows :

| *Description*                                                                     | *Length* |
| ---   | ---     |
| APDU length (big endian)                                                          | 2 |
| APDU CLA                                                                          | 1 |
| APDU INS                                                                          | 1 |
| APDU P1                                                                           | 1 |
| APDU P2                                                                           | 1 |
| APDU length                                                                       | 1 |
| Optional APDU data                                                                | var |

APDU payload is encoded according to the APDU case

| Case Number  | *Lc* | *Le* | Case description |
| ---   | ---     |  ---     | ---     |
|   1          |  0   |  0   | No data in either direction - L is set to 00 |
|   2          |  0   |  !0  | Input Data present, no Output Data - L is set to Lc |
|   3          |  !0  |  0   | Output Data present, no Input Data - L is set to Le |
|   4          |  !0  |  !0  | Both Input and Output Data are present - L is set to Lc |

### APDU Response payload encoding

APDU Response payloads are encoded as follows :

| *Description*                                                                     | *Length* |
| ---   | ---     |
| APDU response length (big endian)                                                 | 2 |
| APDU response data and Status Word                                                | var |

### USB mapping

Messages are exchanged with the dongle over HID endpoints over interrupt transfers, with each chunk being 64 bytes long. The HID Report ID is ignored.

### BLE mapping

A similar encoding is used over BLE, without the Communication channel ID.

The application acts as a GATT server defining service UUID D973F2E0-B19E-11E2-9E96-0800200C9A66

When using this service, the client sends requests to the characteristic D973F2E2-B19E-11E2-9E96-0800200C9A66, and gets notified on the characteristic D973F2E1-B19E-11E2-9E96-0800200C9A66 after registering for it.

Requests are encoded using the standard BLE 20 bytes MTU size

## Status Words

The following standard Status Words are returned for all APDUs - some specific Status Words can be used for specific commands and are mentioned in the command description.

| *SW*     | *Description* |
| ---   | ---     |
|   6501   | TransactionType not supported |
|   6502   | Output buffer too small for snprintf input |
|   6503   | Plugin error |
|   6504   | Failed to convert from int256 |
|   6700   | Incorrect length |
|   6982   | Security status not satisfied (Canceled by user) |
|   6A80   | Invalid data |
|   6B00   | Incorrect parameter P1 or P2 |
|   6Fxx   | Technical problem (Internal error, please report) |
|   9000   | Normal ending of the command |
