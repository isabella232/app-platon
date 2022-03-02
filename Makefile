#*******************************************************************************
#   Ledger App
#   (c) 2017 Ledger
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#*******************************************************************************

ifeq ($(BOLOS_SDK),)
$(error Environment variable BOLOS_SDK is not set)
endif

include $(BOLOS_SDK)/Makefile.defines

DEFINES_LIB = USE_LIB_PLATON
APP_LOAD_PARAMS= --curve secp256k1 $(COMMON_LOAD_PARAMS)
# Allow the app to use path 45 for multi-sig (see BIP45).
APP_LOAD_PARAMS += --path "45'"
# Samsung temporary implementation for wallet ID on 0xda7aba5e/0xc1a551c5
APP_LOAD_PARAMS += --path "1517992542'/1101353413'"

APPVERSION_M=1
APPVERSION_N=0
APPVERSION_P=0
APPVERSION=$(APPVERSION_M).$(APPVERSION_N).$(APPVERSION_P)
APP_LOAD_FLAGS= --appFlags 0x240 --dep Lat:$(APPVERSION)

ifeq ($(CHAIN),)
CHAIN=platon
endif

ifeq ($(CHAIN),platon)
# Lock the application on its standard path for 1.5. Please complain if non compliant
APP_LOAD_PARAMS += --path "44'/486'"
DEFINES += CHAINID_UPCASE=\"PLATON\" CHAINID_COINNAME=\"LAT\" CHAIN_KIND=CHAIN_KIND_PLATON CHAIN_ID=100 CHAIN_HRP=\"lat\"
APPNAME = "PlatON"
DEFINES_LIB=
APP_LOAD_FLAGS=--appFlags 0xa40
endif

APP_LOAD_PARAMS += $(APP_LOAD_FLAGS) --path "44'/1'"
DEFINES += $(DEFINES_LIB)

# Ledger: add the "Pending security review" disclaimer
APP_LOAD_PARAMS += --tlvraw 9F:01
DEFINES += HAVE_PENDING_REVIEW_SCREEN

#prepare hsm generation
ifeq ($(TARGET_NAME), TARGET_NANOS)
ICONNAME=icons/nanos_app_$(CHAIN).gif
else
ICONNAME=icons/nanox_app_$(CHAIN).gif
endif

################
# Default rule #
################
all: default

############
# Platform #
############

DEFINES   += OS_IO_SEPROXYHAL
DEFINES   += HAVE_BAGL HAVE_SPRINTF
DEFINES   += HAVE_IO_USB HAVE_L4_USBLIB IO_USB_MAX_ENDPOINTS=4 IO_HID_EP_LENGTH=64 HAVE_USB_APDU
DEFINES   += LEDGER_MAJOR_VERSION=$(APPVERSION_M) LEDGER_MINOR_VERSION=$(APPVERSION_N) LEDGER_PATCH_VERSION=$(APPVERSION_P)

# U2F
DEFINES   += HAVE_U2F HAVE_IO_U2F
DEFINES   += U2F_PROXY_MAGIC=\"w0w\"
DEFINES   += USB_SEGMENT_SIZE=64
DEFINES   += BLE_SEGMENT_SIZE=32 #max MTU, min 20
DEFINES   += UNUSED\(x\)=\(void\)x
DEFINES   += APPVERSION=\"$(APPVERSION)\"
DEFINES   += HAVE_UX_FLOW

#WEBUSB_URL     = www.ledgerwallet.com
#DEFINES       += HAVE_WEBUSB WEBUSB_URL_SIZE_B=$(shell echo -n $(WEBUSB_URL) | wc -c) WEBUSB_URL=$(shell echo -n $(WEBUSB_URL) | sed -e "s/./\\\'\0\\\',/g")

DEFINES   += HAVE_WEBUSB WEBUSB_URL_SIZE_B=0 WEBUSB_URL=""

ifeq ($(TARGET_NAME),TARGET_NANOX)
DEFINES   += HAVE_BLE BLE_COMMAND_TIMEOUT_MS=2000
DEFINES   += HAVE_BLE_APDU # basic ledger apdu transport over BLE
endif

ifeq ($(TARGET_NAME),TARGET_NANOS)
DEFINES   += IO_SEPROXYHAL_BUFFER_SIZE_B=72
DEFINES   += HAVE_WALLET_ID_SDK
else
DEFINES   += IO_SEPROXYHAL_BUFFER_SIZE_B=300
DEFINES   += HAVE_GLO096
DEFINES   += HAVE_BAGL BAGL_WIDTH=128 BAGL_HEIGHT=64
DEFINES   += HAVE_BAGL_ELLIPSIS # long label truncation feature
DEFINES   += HAVE_BAGL_FONT_OPEN_SANS_REGULAR_11PX
DEFINES   += HAVE_BAGL_FONT_OPEN_SANS_EXTRABOLD_11PX
DEFINES   += HAVE_BAGL_FONT_OPEN_SANS_LIGHT_16PX
endif

# Enables direct data signing without having to specify it in the settings. Useful when testing with speculos.
ALLOW_DATA:=0
ifneq ($(ALLOW_DATA),0)
DEFINES += HAVE_ALLOW_DATA
endif


# Enabling debug PRINTF
DEBUG:=0
ifneq ($(DEBUG),0)
DEFINES += HAVE_STACK_OVERFLOW_CHECK
ifeq ($(TARGET_NAME),TARGET_NANOS)
DEFINES   += HAVE_PRINTF PRINTF=screen_printf
else
DEFINES   += HAVE_PRINTF PRINTF=mcu_usb_printf
endif
else
DEFINES   += PRINTF\(...\)=
endif

ifneq ($(NOCONSENT),)
DEFINES   += NO_CONSENT
endif

#DEFINES   += HAVE_TOKENS_LIST # Do not activate external PRC-20 support yet

##############
#  Compiler  #
##############
ifneq ($(BOLOS_ENV),)
$(info BOLOS_ENV=$(BOLOS_ENV))
CLANGPATH := $(BOLOS_ENV)/clang-arm-fropi/bin/
GCCPATH := $(BOLOS_ENV)/gcc-arm-none-eabi-5_3-2016q1/bin/
else
$(info BOLOS_ENV is not set: falling back to CLANGPATH and GCCPATH)
endif
ifeq ($(CLANGPATH),)
$(info CLANGPATH is not set: clang will be used from PATH)
endif
ifeq ($(GCCPATH),)
$(info GCCPATH is not set: arm-none-eabi-* will be used from PATH)
endif

CC       := $(CLANGPATH)clang

#CFLAGS   += -O0
CFLAGS   += -O3 -Os -Wno-format-invalid-specifier -Wno-format-extra-args -Wno-main

AS     := $(GCCPATH)arm-none-eabi-gcc

LD       := $(GCCPATH)arm-none-eabi-gcc
LDFLAGS  += -O3 -Os
LDLIBS   += -lm -lgcc -lc

# import rules to compile glyphs(/pone)
include $(BOLOS_SDK)/Makefile.glyphs

### variables processed by the common makefile.rules of the SDK to grab source files and include dirs
APP_SOURCE_PATH  += src_common src src_features src_plugins
SDK_SOURCE_PATH  += lib_stusb lib_stusb_impl lib_u2f
SDK_SOURCE_PATH  += lib_ux
ifeq ($(TARGET_NAME),TARGET_NANOX)
SDK_SOURCE_PATH  += lib_blewbxx lib_blewbxx_impl
endif

### initialize plugin SDK submodule if needed, rebuild it, and warn if a difference is noticed
# ifeq ($(CHAIN),platon)
# ifneq ($(shell git submodule status | grep '^[-+]'),)
# $(info INFO: Need to reinitialize git submodules)
# $(shell git submodule update --init)
# endif

# rebuild
# $(shell python3 lat-plugin-sdk/build_sdk.py)

# check if a difference is noticed (fail if it happens in CI build)
# ifneq ($(shell git status | grep 'lat-plugin-sdk'),)
# ifneq ($(JENKINS_URL),)
# $(error ERROR: please update lat-plugin-sdk submodule first)
# else
# $(warning WARNING: please update lat-plugin-sdk submodule first)
# endif
# endif
# endif

load: all
	python3 -m ledgerblue.loadApp $(APP_LOAD_PARAMS)

delete:
	python3 -m ledgerblue.deleteApp $(COMMON_DELETE_PARAMS)

# import generic rules from the sdk
include $(BOLOS_SDK)/Makefile.rules

#add dependency on custom makefile filename
dep/%.d: %.c Makefile

listvariants:
	@echo VARIANTS CHAIN platon
