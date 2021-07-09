MKDV_MK := $(abspath $(lastword $(MAKEFILE_LIST)))
TEST_DIR := $(dir $(MKDV_MK))

MKDV_VERBOSE ?= 1

PYTBLINK_RTL_HDL_DIR := $(abspath $(TEST_DIR)/../..)
PACKAGES_DIR := $(PYTBLINK_RTL_HDL_DIR)/packages
DV_MK := $(shell PATH=$(PACKAGES_DIR)/python/bin:$(PATH) python3 -m mkdv mkfile)

MKDV_TOOL ?= icarus
MKDV_VL_SRCS += $(TEST_DIR)/smoke_tb.sv
TOP_MODULE = smoke_tb

MKDV_PYTHONPATH += $(PACKAGES_DIR)/tblink-rpc-core/python
MKDV_PYTHONPATH += $(PYTBLINK_RTL_HDL_DIR)/ext/src

VPI_LIBS += $(PYTBLINK_RTL_HDL_DIR)/build/libpytblink_rpc_hdl.so

include $(DV_MK)

RULES := 1

include $(DV_MK)

