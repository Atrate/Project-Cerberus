// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "spi_filter_interface_mock.h"


static int spi_filter_interface_mock_get_port (struct spi_filter_interface *filter)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN_NO_ARGS (&mock->mock, spi_filter_interface_mock_get_port, filter);
}

static int spi_filter_interface_mock_get_mfg_id (struct spi_filter_interface *filter,
	uint8_t *mfg_id)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_get_mfg_id, filter, MOCK_ARG_CALL (mfg_id));
}

static int spi_filter_interface_mock_set_mfg_id (struct spi_filter_interface *filter,
	uint8_t mfg_id)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_set_mfg_id, filter, MOCK_ARG_CALL (mfg_id));
}

static int spi_filter_interface_mock_get_flash_size (struct spi_filter_interface *filter,
	uint32_t *bytes)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_get_flash_size, filter,
		MOCK_ARG_CALL (bytes));
}

static int spi_filter_interface_mock_set_flash_size (struct spi_filter_interface *filter,
	uint32_t bytes)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_set_flash_size, filter,
		MOCK_ARG_CALL (bytes));
}

static int spi_filter_interface_mock_get_filter_enabled (struct spi_filter_interface *filter,
	bool *enabled)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_get_filter_enabled, filter,
		MOCK_ARG_CALL (enabled));
}

static int spi_filter_interface_mock_enable_filter (struct spi_filter_interface *filter,
	bool enable)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_enable_filter, filter,
		MOCK_ARG_CALL (enable));
}

static int spi_filter_interface_mock_get_ro_cs (struct spi_filter_interface *filter,
	spi_filter_cs *act_sel)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_get_ro_cs, filter,
		MOCK_ARG_CALL (act_sel));
}

static int spi_filter_interface_mock_set_ro_cs (struct spi_filter_interface *filter,
	spi_filter_cs act_sel)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_set_ro_cs, filter, MOCK_ARG_CALL (act_sel));
}

#ifdef SPI_FILTER_SUPPORT_RO_READ_SWITCH
static int spi_filter_interface_mock_get_ro_read_region (struct spi_filter_interface *filter,
	spi_filter_device *act_device)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_get_ro_read_region, filter,
		MOCK_ARG_CALL (act_device));
}

static int spi_filter_interface_mock_set_ro_read_region (struct spi_filter_interface *filter,
	spi_filter_device act_device)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_set_ro_read_region, filter,
		MOCK_ARG_CALL (act_device));
}

static int spi_filter_interface_mock_get_ro_read_region_switch_enabled (
	struct spi_filter_interface *filter, bool *enabled)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_get_ro_read_region_switch_enabled, filter,
		MOCK_ARG_CALL (enabled));
}

static int spi_filter_interface_mock_enable_ro_read_region_switch (
	struct spi_filter_interface *filter, bool enable)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_enable_ro_read_region_switch, filter,
		MOCK_ARG_CALL (enable));
}
#endif

static int spi_filter_interface_mock_get_addr_byte_mode (struct spi_filter_interface *filter,
	spi_filter_address_mode *mode)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_get_addr_byte_mode, filter,
		MOCK_ARG_CALL (mode));
}

static int spi_filter_interface_mock_set_addr_byte_mode (struct spi_filter_interface *filter,
	spi_filter_address_mode mode)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_set_addr_byte_mode, filter,
		MOCK_ARG_CALL (mode));
}

static int spi_filter_interface_mock_get_flash_dirty_state (struct spi_filter_interface *filter,
	spi_filter_flash_state *state)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_get_flash_dirty_state, filter,
		MOCK_ARG_CALL (state));
}

static int spi_filter_interface_mock_clear_flash_dirty_state (struct spi_filter_interface *filter)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN_NO_ARGS (&mock->mock, spi_filter_interface_mock_clear_flash_dirty_state, filter);
}

static int spi_filter_interface_mock_get_bypass_mode (struct spi_filter_interface *filter,
	spi_filter_bypass_mode *bypass)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_get_bypass_mode, filter,
		MOCK_ARG_CALL (bypass));
}

static int spi_filter_interface_mock_set_bypass_mode (struct spi_filter_interface *filter,
	spi_filter_bypass_mode bypass)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_set_bypass_mode, filter,
		MOCK_ARG_CALL (bypass));
}

static int spi_filter_interface_mock_get_filter_rw_region (struct spi_filter_interface *filter,
	uint8_t region, uint32_t *start_addr, uint32_t *end_addr)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_get_filter_rw_region, filter,
		MOCK_ARG_CALL (region), MOCK_ARG_CALL (start_addr), MOCK_ARG_CALL (end_addr));
}

static int spi_filter_interface_mock_set_filter_rw_region (struct spi_filter_interface *filter,
	uint8_t region, uint32_t start_addr, uint32_t end_addr)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN (&mock->mock, spi_filter_interface_mock_set_filter_rw_region, filter,
		MOCK_ARG_CALL (region), MOCK_ARG_CALL (start_addr), MOCK_ARG_CALL (end_addr));
}

static int spi_filter_interface_mock_clear_filter_rw_regions (struct spi_filter_interface *filter)
{
	struct spi_filter_interface_mock *mock = (struct spi_filter_interface_mock*) filter;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	MOCK_RETURN_NO_ARGS (&mock->mock, spi_filter_interface_mock_clear_filter_rw_regions, filter);
}

static int spi_filter_interface_mock_func_arg_count (void *func)
{
	if ((func == spi_filter_interface_mock_get_filter_rw_region) ||
		(func == spi_filter_interface_mock_set_filter_rw_region)) {
		return 3;
	}
	else if ((func == spi_filter_interface_mock_get_mfg_id) ||
		(func == spi_filter_interface_mock_set_mfg_id) ||
		(func == spi_filter_interface_mock_get_flash_size) ||
		(func == spi_filter_interface_mock_set_flash_size) ||
		(func == spi_filter_interface_mock_get_filter_enabled) ||
		(func == spi_filter_interface_mock_enable_filter) ||
		(func == spi_filter_interface_mock_get_ro_cs) ||
		(func == spi_filter_interface_mock_set_ro_cs) ||
#ifdef SPI_FILTER_SUPPORT_RO_READ_SWITCH
		(func == spi_filter_interface_mock_get_ro_read_region) ||
		(func == spi_filter_interface_mock_set_ro_read_region) ||
		(func == spi_filter_interface_mock_get_ro_read_region_switch_enabled) ||
		(func == spi_filter_interface_mock_enable_ro_read_region_switch) ||
#endif
		(func == spi_filter_interface_mock_get_addr_byte_mode) ||
		(func == spi_filter_interface_mock_set_addr_byte_mode) ||
		(func == spi_filter_interface_mock_get_flash_dirty_state) ||
		(func == spi_filter_interface_mock_get_bypass_mode) ||
		(func == spi_filter_interface_mock_set_bypass_mode)) {
		return 1;
	}
	else {
		return 0;
	}
}

static const char* spi_filter_interface_mock_func_name_map (void *func)
{
	if (func == spi_filter_interface_mock_get_port) {
		return "get_port";
	}
	else if (func == spi_filter_interface_mock_get_mfg_id) {
		return "get_mfg_id";
	}
	else if (func == spi_filter_interface_mock_set_mfg_id) {
		return "set_mfg_id";
	}
	else if (func == spi_filter_interface_mock_get_flash_size) {
		return "get_flash_size";
	}
	else if (func == spi_filter_interface_mock_set_flash_size) {
		return "set_flash_size";
	}
	else if (func == spi_filter_interface_mock_get_filter_enabled) {
		return "get_filter_enabled";
	}
	else if (func == spi_filter_interface_mock_enable_filter) {
		return "enable_filter";
	}
	else if (func == spi_filter_interface_mock_get_ro_cs) {
		return "get_ro_cs";
	}
	else if (func == spi_filter_interface_mock_set_ro_cs) {
		return "set_ro_cs";
	}
#ifdef SPI_FILTER_SUPPORT_RO_READ_SWITCH
	else if (func == spi_filter_interface_mock_get_ro_read_region) {
		return "get_ro_read_region";
	}
	else if (func == spi_filter_interface_mock_set_ro_read_region) {
		return "set_ro_read_region";
	}
	else if (func == spi_filter_interface_mock_get_ro_read_region_switch_enabled) {
		return "get_ro_read_region_switch_enabled";
	}
	else if (func == spi_filter_interface_mock_enable_ro_read_region_switch) {
		return "enable_ro_read_region_switch";
	}
#endif
	else if (func == spi_filter_interface_mock_get_addr_byte_mode) {
		return "get_addr_byte_mode";
	}
	else if (func == spi_filter_interface_mock_set_addr_byte_mode) {
		return "set_addr_byte_mode";
	}
	else if (func == spi_filter_interface_mock_get_flash_dirty_state) {
		return "get_flash_dirty_state";
	}
	else if (func == spi_filter_interface_mock_clear_flash_dirty_state) {
		return "clear_flash_dirty_state";
	}
	else if (func == spi_filter_interface_mock_get_bypass_mode) {
		return "get_bypass_mode";
	}
	else if (func == spi_filter_interface_mock_set_bypass_mode) {
		return "set_bypass_mode";
	}
	else if (func == spi_filter_interface_mock_get_filter_rw_region) {
		return "get_filter_rw_region";
	}
	else if (func == spi_filter_interface_mock_set_filter_rw_region) {
		return "set_filter_rw_region";
	}
	else if (func == spi_filter_interface_mock_clear_filter_rw_regions) {
		return "clear_filter_rw_regions";
	}
	else {
		return "unknown";
	}
}

static const char* spi_filter_interface_mock_arg_name_map (void *func, int arg)
{
	if (func == spi_filter_interface_mock_get_mfg_id) {
		switch (arg) {
			case 0:
				return "mfg_id";
		}
	}
	else if (func == spi_filter_interface_mock_set_mfg_id) {
		switch (arg) {
			case 0:
				return "mfg_id";
		}
	}
	else if (func == spi_filter_interface_mock_get_flash_size) {
		switch (arg) {
			case 0:
				return "bytes";
		}
	}
	else if (func == spi_filter_interface_mock_set_flash_size) {
		switch (arg) {
			case 0:
				return "bytes";
		}
	}
	else if (func == spi_filter_interface_mock_get_filter_enabled) {
		switch (arg) {
			case 0:
				return "enabled";
		}
	}
	else if (func == spi_filter_interface_mock_enable_filter) {
		switch (arg) {
			case 0:
				return "enable";
		}
	}
	else if (func == spi_filter_interface_mock_get_ro_cs) {
		switch (arg) {
			case 0:
				return "act_sel";
		}
	}
	else if (func == spi_filter_interface_mock_set_ro_cs) {
		switch (arg) {
			case 0:
				return "act_sel";
		}
	}
#ifdef SPI_FILTER_SUPPORT_RO_READ_SWITCH
	else if (func == spi_filter_interface_mock_get_ro_read_region) {
		switch (arg) {
			case 0:
				return "act_device";
		}
	}
	else if (func == spi_filter_interface_mock_set_ro_read_region) {
		switch (arg) {
			case 0:
				return "act_device";
		}
	}
	else if (func == spi_filter_interface_mock_get_ro_read_region_switch_enabled) {
		switch (arg) {
			case 0:
				return "enabled";
		}
	}
	else if (func == spi_filter_interface_mock_enable_ro_read_region_switch) {
		switch (arg) {
			case 0:
				return "enable";
		}
	}
#endif
	else if (func == spi_filter_interface_mock_get_addr_byte_mode) {
		switch (arg) {
			case 0:
				return "mode";
		}
	}
	else if (func == spi_filter_interface_mock_set_addr_byte_mode) {
		switch (arg) {
			case 0:
				return "mode";
		}
	}
	else if (func == spi_filter_interface_mock_get_flash_dirty_state) {
		switch (arg) {
			case 0:
				return "state";
		}
	}
	else if (func == spi_filter_interface_mock_get_bypass_mode) {
		switch (arg) {
			case 0:
				return "bypass";
		}
	}
	else if (func == spi_filter_interface_mock_set_bypass_mode) {
		switch (arg) {
			case 0:
				return "bypass";
		}
	}
	else if (func == spi_filter_interface_mock_get_filter_rw_region) {
		switch (arg) {
			case 0:
				return "region";

			case 1:
				return "start_addr";

			case 2:
				return "end_addr";
		}
	}
	else if (func == spi_filter_interface_mock_set_filter_rw_region) {
		switch (arg) {
			case 0:
				return "region";

			case 1:
				return "start_addr";

			case 2:
				return "end_addr";
		}
	}

	return "unknown";
}

/**
 * Initialize a mock instance for a SPI filter.
 *
 * @param mock The mock to initialize.
 *
 * @return 0 if the mock was successfully initialized or an error code.
 */
int spi_filter_interface_mock_init (struct spi_filter_interface_mock *mock)
{
	int status;

	if (mock == NULL) {
		return MOCK_INVALID_ARGUMENT;
	}

	memset (mock, 0, sizeof (struct spi_filter_interface_mock));

	status = mock_init (&mock->mock);
	if (status != 0) {
		return status;
	}

	mock_set_name (&mock->mock, "spi_filter_interface");

	mock->base.get_port = spi_filter_interface_mock_get_port;
	mock->base.get_mfg_id = spi_filter_interface_mock_get_mfg_id;
	mock->base.set_mfg_id = spi_filter_interface_mock_set_mfg_id;
	mock->base.get_flash_size = spi_filter_interface_mock_get_flash_size;
	mock->base.set_flash_size = spi_filter_interface_mock_set_flash_size;
	mock->base.get_filter_enabled = spi_filter_interface_mock_get_filter_enabled;
	mock->base.enable_filter = spi_filter_interface_mock_enable_filter;
	mock->base.get_ro_cs = spi_filter_interface_mock_get_ro_cs;
	mock->base.set_ro_cs = spi_filter_interface_mock_set_ro_cs;
#ifdef SPI_FILTER_SUPPORT_RO_READ_SWITCH
	mock->base.get_ro_read_region = spi_filter_interface_mock_get_ro_read_region;
	mock->base.set_ro_read_region = spi_filter_interface_mock_set_ro_read_region;
	mock->base.get_ro_read_region_switch_enabled =
		spi_filter_interface_mock_get_ro_read_region_switch_enabled;
	mock->base.enable_ro_read_region_switch =
		spi_filter_interface_mock_enable_ro_read_region_switch;
#endif
	mock->base.get_addr_byte_mode = spi_filter_interface_mock_get_addr_byte_mode;
	mock->base.set_addr_byte_mode = spi_filter_interface_mock_set_addr_byte_mode;
	mock->base.get_flash_dirty_state = spi_filter_interface_mock_get_flash_dirty_state;
	mock->base.clear_flash_dirty_state = spi_filter_interface_mock_clear_flash_dirty_state;
	mock->base.get_bypass_mode = spi_filter_interface_mock_get_bypass_mode;
	mock->base.set_bypass_mode = spi_filter_interface_mock_set_bypass_mode;
	mock->base.get_filter_rw_region = spi_filter_interface_mock_get_filter_rw_region;
	mock->base.set_filter_rw_region = spi_filter_interface_mock_set_filter_rw_region;
	mock->base.clear_filter_rw_regions = spi_filter_interface_mock_clear_filter_rw_regions;

	mock->mock.func_arg_count = spi_filter_interface_mock_func_arg_count;
	mock->mock.func_name_map = spi_filter_interface_mock_func_name_map;
	mock->mock.arg_name_map = spi_filter_interface_mock_arg_name_map;

	return 0;
}

/**
 * Release the resource used by a SPI filter mock instance.
 *
 * @param mock The mock to release.
 */
void spi_filter_interface_mock_release (struct spi_filter_interface_mock *mock)
{
	if (mock != NULL) {
		mock_release (&mock->mock);
	}
}

/**
 * Verify that the mock expectations were called and release the mock instance.
 *
 * @param mock The mock to validate.
 *
 * @return 0 if the expectations were met or 1 if not.
 */
int spi_filter_interface_mock_validate_and_release (struct spi_filter_interface_mock *mock)
{
	int status = 1;

	if (mock != NULL) {
		status = mock_validate (&mock->mock);
		spi_filter_interface_mock_release (mock);
	}

	return status;
}
