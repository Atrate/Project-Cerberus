// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "testing.h"
#include "cmd_interface/cerberus_protocol.h"
#include "cmd_interface/cerberus_protocol_optional_commands.h"
#include "cmd_interface/cerberus_protocol_master_commands.h"
#include "cmd_interface/attestation_cmd_interface.h"
#include "logging/debug_log.h"
#include "recovery/recovery_image_header.h"
#include "attestation/aux_attestation.h"
#include "mock/pfm_mock.h"
#include "mock/recovery_image_mock.h"
#include "cerberus_protocol_optional_commands_testing.h"
#include "recovery_image_header_testing.h"
#include "aux_attestation_testing.h"
#include "ecc_testing.h"


static const char *SUITE = "cerberus_protocol_optional_commands";


void cerberus_protocol_optional_commands_testing_process_fw_update_init (CuTest *test,
	struct cmd_interface *cmd, struct firmware_update_control_mock *update)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_prepare_fw_update *req =
		(struct cerberus_protocol_prepare_fw_update*) request.data;
	uint32_t size = 0x31EEAABB;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_INIT_FW_UPDATE;

	req->total_size = size;
	request.length = sizeof (struct cerberus_protocol_prepare_fw_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&update->mock, update->base.prepare_staging, update, 0, MOCK_ARG (size));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_fw_update_init_invalid_len (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_prepare_fw_update *req =
		(struct cerberus_protocol_prepare_fw_update*) request.data;
	uint32_t size = 0x31EEAABB;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_INIT_FW_UPDATE;

	req->total_size = size;
	request.length = sizeof (struct cerberus_protocol_prepare_fw_update) + 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	request.length = sizeof (struct cerberus_protocol_prepare_fw_update) - 1;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_fw_update_init_fail (CuTest *test,
	struct cmd_interface *cmd, struct firmware_update_control_mock *update)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_prepare_fw_update *req =
		(struct cerberus_protocol_prepare_fw_update*) request.data;
	uint32_t size = 0x31EEAABB;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_INIT_FW_UPDATE;

	req->total_size = size;
	request.length = sizeof (struct cerberus_protocol_prepare_fw_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&update->mock, update->base.prepare_staging, update,
		FIRMWARE_UPDATE_NO_MEMORY, MOCK_ARG (size));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, FIRMWARE_UPDATE_NO_MEMORY, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_fw_update (CuTest *test,
	struct cmd_interface *cmd, struct firmware_update_control_mock *update)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_fw_update *req = (struct cerberus_protocol_fw_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_FW_UPDATE;

	req->payload = 0xAA;
	request.length = sizeof (struct cerberus_protocol_fw_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&update->mock, update->base.write_staging, update, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (&req->payload, 1), MOCK_ARG (1));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_fw_update_no_data (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_fw_update *req = (struct cerberus_protocol_fw_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_FW_UPDATE;

	req->payload = 0xAA;
	request.length = sizeof (struct cerberus_protocol_fw_update) - 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_fw_update_fail (CuTest *test,
	struct cmd_interface *cmd, struct firmware_update_control_mock *update)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_fw_update *req = (struct cerberus_protocol_fw_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_FW_UPDATE;

	req->payload = 0xAA;
	request.length = sizeof (struct cerberus_protocol_fw_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&update->mock, update->base.write_staging, update,
		FIRMWARE_UPDATE_NO_MEMORY, MOCK_ARG_PTR_CONTAINS_TMP (&req->payload, 1), MOCK_ARG (1));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, FIRMWARE_UPDATE_NO_MEMORY, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_complete_fw_update (CuTest *test,
	struct cmd_interface *cmd, struct firmware_update_control_mock *update)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_fw_update *req =
		(struct cerberus_protocol_complete_fw_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_COMPLETE_FW_UPDATE;

	request.length = sizeof (struct cerberus_protocol_complete_fw_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&update->mock, update->base.start_update, update, 0);
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_complete_fw_update_invalid_len (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_fw_update *req =
		(struct cerberus_protocol_complete_fw_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_COMPLETE_FW_UPDATE;

	request.length = sizeof (struct cerberus_protocol_complete_fw_update) + 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_complete_fw_update_fail (CuTest *test,
	struct cmd_interface *cmd, struct firmware_update_control_mock *update)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_fw_update *req =
		(struct cerberus_protocol_complete_fw_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_COMPLETE_FW_UPDATE;

	request.length = sizeof (struct cerberus_protocol_complete_fw_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&update->mock, update->base.start_update, update,
		FIRMWARE_UPDATE_NO_MEMORY);
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, FIRMWARE_UPDATE_NO_MEMORY, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_init_port0 (CuTest *test,
	struct cmd_interface *cmd, struct manifest_cmd_interface_mock *pfm_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_prepare_pfm_update *req =
		(struct cerberus_protocol_prepare_pfm_update*) request.data;
	uint32_t length = 1;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_INIT_PFM_UPDATE;

	req->port_id = 0;
	req->size = length;
	request.length = sizeof (struct cerberus_protocol_prepare_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&pfm_0->mock, pfm_0->base.prepare_manifest, pfm_0, 0, MOCK_ARG (length));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_init_port1 (CuTest *test,
	struct cmd_interface *cmd, struct manifest_cmd_interface_mock *pfm_1)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_prepare_pfm_update *req =
		(struct cerberus_protocol_prepare_pfm_update*) request.data;
	uint32_t length = 1;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_INIT_PFM_UPDATE;

	req->port_id = 1;
	req->size = length;
	request.length = sizeof (struct cerberus_protocol_prepare_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&pfm_1->mock, pfm_1->base.prepare_manifest, pfm_1, 0, MOCK_ARG (length));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_init_port0_null (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_prepare_pfm_update *req =
		(struct cerberus_protocol_prepare_pfm_update*) request.data;
	uint32_t length = 1;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_INIT_PFM_UPDATE;

	req->port_id = 0;
	req->size = length;
	request.length = sizeof (struct cerberus_protocol_prepare_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_init_port1_null (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_prepare_pfm_update *req =
		(struct cerberus_protocol_prepare_pfm_update*) request.data;
	uint32_t length = 1;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_INIT_PFM_UPDATE;

	req->port_id = 1;
	req->size = length;
	request.length = sizeof (struct cerberus_protocol_prepare_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_init_invalid_port (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_prepare_pfm_update *req =
		(struct cerberus_protocol_prepare_pfm_update*) request.data;
	uint32_t length = 1;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_INIT_PFM_UPDATE;

	req->port_id = 2;
	req->size = length;
	request.length = sizeof (struct cerberus_protocol_prepare_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_OUT_OF_RANGE, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_init_invalid_len (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_prepare_pfm_update *req =
		(struct cerberus_protocol_prepare_pfm_update*) request.data;
	uint32_t length = 1;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_INIT_PFM_UPDATE;

	req->port_id = 0;
	req->size = length;
	request.length = sizeof (struct cerberus_protocol_prepare_pfm_update) + 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	request.length = sizeof (struct cerberus_protocol_prepare_pfm_update) - 1;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_init_fail (CuTest *test,
	struct cmd_interface *cmd, struct manifest_cmd_interface_mock *pfm_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_prepare_pfm_update *req =
		(struct cerberus_protocol_prepare_pfm_update*) request.data;
	uint32_t length = 1;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_INIT_PFM_UPDATE;

	req->port_id = 0;
	req->size = length;
	request.length = sizeof (struct cerberus_protocol_prepare_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&pfm_0->mock, pfm_0->base.prepare_manifest, pfm_0, PFM_INVALID_ARGUMENT,
		MOCK_ARG (length));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, PFM_INVALID_ARGUMENT, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_port0 (CuTest *test,
	struct cmd_interface *cmd, struct manifest_cmd_interface_mock *pfm_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_pfm_update *req = (struct cerberus_protocol_pfm_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_PFM_UPDATE;

	req->port_id = 0;
	req->payload = 1;
	request.length = sizeof (struct cerberus_protocol_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&pfm_0->mock, pfm_0->base.store_manifest, pfm_0, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (&req->payload, 1), MOCK_ARG (1));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_port1 (CuTest *test,
	struct cmd_interface *cmd, struct manifest_cmd_interface_mock *pfm_1)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_pfm_update *req = (struct cerberus_protocol_pfm_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_PFM_UPDATE;

	req->port_id = 1;
	req->payload = 1;
	request.length = sizeof (struct cerberus_protocol_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&pfm_1->mock, pfm_1->base.store_manifest, pfm_1, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (&req->payload, 1), MOCK_ARG (1));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_port0_null (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_pfm_update *req = (struct cerberus_protocol_pfm_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_PFM_UPDATE;

	req->port_id = 0;
	req->payload = 1;
	request.length = sizeof (struct cerberus_protocol_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_port1_null (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_pfm_update *req = (struct cerberus_protocol_pfm_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_PFM_UPDATE;

	req->port_id = 1;
	req->payload = 1;
	request.length = sizeof (struct cerberus_protocol_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_no_data (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_pfm_update *req = (struct cerberus_protocol_pfm_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_PFM_UPDATE;

	req->port_id = 0;
	request.length = sizeof (struct cerberus_protocol_pfm_update) - 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_invalid_port (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_pfm_update *req = (struct cerberus_protocol_pfm_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_PFM_UPDATE;

	req->port_id = 2;
	req->payload = 1;
	request.length = sizeof (struct cerberus_protocol_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_OUT_OF_RANGE, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_fail (CuTest *test,
	struct cmd_interface *cmd, struct manifest_cmd_interface_mock *pfm_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_pfm_update *req = (struct cerberus_protocol_pfm_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_PFM_UPDATE;

	req->port_id = 0;
	req->payload = 1;
	request.length = sizeof (struct cerberus_protocol_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&pfm_0->mock, pfm_0->base.store_manifest, pfm_0,
		MANIFEST_MANAGER_NO_MEMORY, MOCK_ARG_PTR_CONTAINS_TMP (&req->payload, 1), MOCK_ARG (1));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, MANIFEST_MANAGER_NO_MEMORY, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_complete_port0 (CuTest *test,
	struct cmd_interface *cmd, struct manifest_cmd_interface_mock *pfm_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_pfm_update *req =
		(struct cerberus_protocol_complete_pfm_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_COMPLETE_PFM_UPDATE;

	req->port_id = 0;
	req->activation = 0;
	request.length =  sizeof (struct cerberus_protocol_complete_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&pfm_0->mock, pfm_0->base.finish_manifest, pfm_0, 0, MOCK_ARG (false));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_complete_port1 (CuTest *test,
	struct cmd_interface *cmd, struct manifest_cmd_interface_mock *pfm_1)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_pfm_update *req =
		(struct cerberus_protocol_complete_pfm_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_COMPLETE_PFM_UPDATE;

	req->port_id = 1;
	req->activation = 0;
	request.length =  sizeof (struct cerberus_protocol_complete_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&pfm_1->mock, pfm_1->base.finish_manifest, pfm_1, 0, MOCK_ARG (false));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_complete_port0_immediate (
	CuTest *test, struct cmd_interface *cmd, struct manifest_cmd_interface_mock *pfm_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_pfm_update *req =
		(struct cerberus_protocol_complete_pfm_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_COMPLETE_PFM_UPDATE;

	req->port_id = 0;
	req->activation = 1;
	request.length =  sizeof (struct cerberus_protocol_complete_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&pfm_0->mock, pfm_0->base.finish_manifest, pfm_0, 0, MOCK_ARG (true));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_complete_port1_immediate (
	CuTest *test, struct cmd_interface *cmd, struct manifest_cmd_interface_mock *pfm_1)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_pfm_update *req =
		(struct cerberus_protocol_complete_pfm_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_COMPLETE_PFM_UPDATE;

	req->port_id = 1;
	req->activation = 1;
	request.length =  sizeof (struct cerberus_protocol_complete_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&pfm_1->mock, pfm_1->base.finish_manifest, pfm_1, 0, MOCK_ARG (true));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_complete_port0_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_pfm_update *req =
		(struct cerberus_protocol_complete_pfm_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_COMPLETE_PFM_UPDATE;

	req->port_id = 0;
	req->activation = 0;
	request.length =  sizeof (struct cerberus_protocol_complete_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_complete_port1_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_pfm_update *req =
		(struct cerberus_protocol_complete_pfm_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_COMPLETE_PFM_UPDATE;

	req->port_id = 1;
	req->activation = 0;
	request.length =  sizeof (struct cerberus_protocol_complete_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_complete_invalid_len (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_pfm_update *req =
		(struct cerberus_protocol_complete_pfm_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_COMPLETE_PFM_UPDATE;

	req->port_id = 0;
	req->activation = 0;
	request.length =  sizeof (struct cerberus_protocol_complete_pfm_update) + 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	request.length = sizeof (struct cerberus_protocol_complete_pfm_update) - 1;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_complete_invalid_port (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_pfm_update *req =
		(struct cerberus_protocol_complete_pfm_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_COMPLETE_PFM_UPDATE;

	req->port_id = 2;
	req->activation = 0;
	request.length =  sizeof (struct cerberus_protocol_complete_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_OUT_OF_RANGE, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_pfm_update_complete_fail (CuTest *test,
	struct cmd_interface *cmd, struct manifest_cmd_interface_mock *pfm_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_pfm_update *req =
		(struct cerberus_protocol_complete_pfm_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_COMPLETE_PFM_UPDATE;

	req->port_id = 0;
	req->activation = 0;
	request.length =  sizeof (struct cerberus_protocol_complete_pfm_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&pfm_0->mock, pfm_0->base.finish_manifest, pfm_0,
		MANIFEST_MANAGER_NO_MEMORY, MOCK_ARG (false));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, MANIFEST_MANAGER_NO_MEMORY, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_id_port0_region0 (CuTest *test,
	struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_0)
{
	struct pfm_mock pfm;
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_id *req = (struct cerberus_protocol_get_pfm_id*) request.data;
	struct cerberus_protocol_get_pfm_id_version_response *resp =
		(struct cerberus_protocol_get_pfm_id_version_response*) request.data;
	uint32_t pfm_id = 0xABCD;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_ID;

	req->port_id = 0;
	req->region = 0;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_pfm_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = pfm_mock_init (&pfm);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.get_active_pfm, pfm_manager_0,
		(intptr_t) &pfm.base);
	status |= mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.free_pfm, pfm_manager_0, 0,
		MOCK_ARG (&pfm.base));

	status |= mock_expect (&pfm.mock, pfm.base.base.get_id, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &pfm_id, sizeof (pfm_id), -1);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_pfm_id_version_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_ID, resp->header.command);
	CuAssertIntEquals (test, 1, resp->valid);
	CuAssertIntEquals (test, pfm_id, resp->version);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = pfm_mock_validate_and_release (&pfm);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_id_port0_region1 (CuTest *test,
	struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_0)
{
	struct pfm_mock pfm;
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_id *req = (struct cerberus_protocol_get_pfm_id*) request.data;
	struct cerberus_protocol_get_pfm_id_version_response *resp =
		(struct cerberus_protocol_get_pfm_id_version_response*) request.data;
	uint32_t pfm_id = 0xABCD;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_ID;

	req->port_id = 0;
	req->region = 1;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_pfm_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = pfm_mock_init (&pfm);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.get_pending_pfm, pfm_manager_0,
		(intptr_t) &pfm.base);
	status |= mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.free_pfm, pfm_manager_0, 0,
		MOCK_ARG (&pfm.base));

	status |= mock_expect (&pfm.mock, pfm.base.base.get_id, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &pfm_id, sizeof (pfm_id), -1);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_pfm_id_version_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_ID, resp->header.command);
	CuAssertIntEquals (test, 1, resp->valid);
	CuAssertIntEquals (test, pfm_id, resp->version);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = pfm_mock_validate_and_release (&pfm);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_id_port1_region0 (CuTest *test,
	struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_1)
{
	struct pfm_mock pfm;
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_id *req = (struct cerberus_protocol_get_pfm_id*) request.data;
	struct cerberus_protocol_get_pfm_id_version_response *resp =
		(struct cerberus_protocol_get_pfm_id_version_response*) request.data;
	uint32_t pfm_id = 0xABCD;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_ID;

	req->port_id = 1;
	req->region = 0;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_pfm_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = pfm_mock_init (&pfm);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm_manager_1->mock, pfm_manager_1->base.get_active_pfm, pfm_manager_1,
		(intptr_t) &pfm.base);
	status |= mock_expect (&pfm_manager_1->mock, pfm_manager_1->base.free_pfm, pfm_manager_1, 0,
		MOCK_ARG (&pfm.base));

	status |= mock_expect (&pfm.mock, pfm.base.base.get_id, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &pfm_id, sizeof (pfm_id), -1);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_pfm_id_version_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_ID, resp->header.command);
	CuAssertIntEquals (test, 1, resp->valid);
	CuAssertIntEquals (test, pfm_id, resp->version);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = pfm_mock_validate_and_release (&pfm);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_id_port1_region1 (CuTest *test,
	struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_1)
{
	struct pfm_mock pfm;
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_id *req = (struct cerberus_protocol_get_pfm_id*) request.data;
	struct cerberus_protocol_get_pfm_id_version_response *resp =
		(struct cerberus_protocol_get_pfm_id_version_response*) request.data;
	uint32_t pfm_id = 0xABCD;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_ID;

	req->port_id = 1;
	req->region = 1;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_pfm_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = pfm_mock_init (&pfm);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm_manager_1->mock, pfm_manager_1->base.get_pending_pfm, pfm_manager_1,
		(intptr_t) &pfm.base);
	status |= mock_expect (&pfm_manager_1->mock, pfm_manager_1->base.free_pfm, pfm_manager_1, 0,
		MOCK_ARG (&pfm.base));

	status |= mock_expect (&pfm.mock, pfm.base.base.get_id, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &pfm_id, sizeof (pfm_id), -1);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_pfm_id_version_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_ID, resp->header.command);
	CuAssertIntEquals (test, 1, resp->valid);
	CuAssertIntEquals (test, pfm_id, resp->version);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = pfm_mock_validate_and_release (&pfm);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_id_no_id_type (CuTest *test,
	struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_0)
{
	struct pfm_mock pfm;
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_id *req = (struct cerberus_protocol_get_pfm_id*) request.data;
	struct cerberus_protocol_get_pfm_id_version_response *resp =
		(struct cerberus_protocol_get_pfm_id_version_response*) request.data;
	uint32_t pfm_id = 0xABCD;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_ID;

	req->port_id = 0;
	req->region = 0;
	request.length = sizeof (struct cerberus_protocol_get_pfm_id) - 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = pfm_mock_init (&pfm);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.get_active_pfm, pfm_manager_0,
		(intptr_t) &pfm.base);
	status |= mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.free_pfm, pfm_manager_0, 0,
		MOCK_ARG (&pfm.base));

	status |= mock_expect (&pfm.mock, pfm.base.base.get_id, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &pfm_id, sizeof (pfm_id), -1);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_pfm_id_version_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_ID, resp->header.command);
	CuAssertIntEquals (test, 1, resp->valid);
	CuAssertIntEquals (test, pfm_id, resp->version);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = pfm_mock_validate_and_release (&pfm);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_id_port0_region0_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_id *req = (struct cerberus_protocol_get_pfm_id*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_ID;

	req->port_id = 0;
	req->region = 0;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_pfm_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_id_port0_region1_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_id *req = (struct cerberus_protocol_get_pfm_id*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_ID;

	req->port_id = 0;
	req->region = 1;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_pfm_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_id_port1_region0_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_id *req = (struct cerberus_protocol_get_pfm_id*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_ID;

	req->port_id = 1;
	req->region = 0;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_pfm_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_id_port1_region1_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_id *req = (struct cerberus_protocol_get_pfm_id*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_ID;

	req->port_id = 1;
	req->region = 1;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_pfm_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_id_no_active_pfm (CuTest *test,
	struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_id *req = (struct cerberus_protocol_get_pfm_id*) request.data;
	struct cerberus_protocol_get_pfm_id_version_response *resp =
		(struct cerberus_protocol_get_pfm_id_version_response*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_ID;

	req->port_id = 0;
	req->region = 0;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_pfm_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.get_active_pfm, pfm_manager_0,
		(intptr_t) NULL);
	status |= mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.free_pfm, pfm_manager_0, 0,
		MOCK_ARG (NULL));

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_pfm_id_version_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_ID, resp->header.command);
	CuAssertIntEquals (test, 0, resp->valid);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_id_no_pending_pfm (CuTest *test,
	struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_id *req = (struct cerberus_protocol_get_pfm_id*) request.data;
	struct cerberus_protocol_get_pfm_id_version_response *resp =
		(struct cerberus_protocol_get_pfm_id_version_response*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_ID;

	req->port_id = 0;
	req->region = 1;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_pfm_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.get_pending_pfm, pfm_manager_0,
		(intptr_t) NULL);
	status |= mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.free_pfm, pfm_manager_0, 0,
		MOCK_ARG (NULL));

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_pfm_id_version_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_ID, resp->header.command);
	CuAssertIntEquals (test, 0, resp->valid);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_id_fail (CuTest *test,
	struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_0)
{
	struct pfm_mock pfm;
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_id *req = (struct cerberus_protocol_get_pfm_id*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_ID;

	req->port_id = 0;
	req->region = 0;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_pfm_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = pfm_mock_init (&pfm);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.get_active_pfm, pfm_manager_0,
		(intptr_t) &pfm.base);
	status |= mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.free_pfm, pfm_manager_0, 0,
		MOCK_ARG (&pfm.base));

	status |= mock_expect (&pfm.mock, pfm.base.base.get_id, &pfm, PFM_NO_MEMORY, MOCK_ARG_NOT_NULL);

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, PFM_NO_MEMORY, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = pfm_mock_validate_and_release (&pfm);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_id_invalid_len (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_id *req = (struct cerberus_protocol_get_pfm_id*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_ID;

	req->port_id = 0;
	req->region = 0;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_pfm_id) + 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	request.length = sizeof (struct cerberus_protocol_get_pfm_id) - sizeof (req->id) - 1;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_id_invalid_port (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_id *req = (struct cerberus_protocol_get_pfm_id*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_ID;

	req->port_id = 2;
	req->region = 0;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_pfm_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_OUT_OF_RANGE, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_id_invalid_region (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_id *req = (struct cerberus_protocol_get_pfm_id*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_ID;

	req->port_id = 0;
	req->region = 2;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_pfm_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_OUT_OF_RANGE, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_port0_region0 (
	CuTest *test, struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	struct cerberus_protocol_get_pfm_supported_fw_response *resp =
		(struct cerberus_protocol_get_pfm_supported_fw_response*) request.data;
	struct pfm_mock pfm;
	struct pfm_firmware_version versions[2];
	int version_len[2];
	struct pfm_firmware_versions versions_list;
	uint32_t pfm_id = 0xAABBCCDD;
	uint32_t offset = 0;
	int status;

	versions[0].fw_version_id =
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1."
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1."
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1";
	version_len[0] = strlen (versions[0].fw_version_id) + 1;

	versions[1].fw_version_id =
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2."
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2."
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2";
	version_len[1] = strlen (versions[1].fw_version_id) + 1;

	versions_list.versions = versions;
	versions_list.count = 2;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 0;
	req->region = 0;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = pfm_mock_init (&pfm);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.get_active_pfm, pfm_manager_0,
		(intptr_t) &pfm.base);
	status |= mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.free_pfm, pfm_manager_0, 0,
		MOCK_ARG (&pfm.base));

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm.mock, pfm.base.base.get_id, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &pfm_id, sizeof (pfm_id), -1);

	status |= mock_expect (&pfm.mock, pfm.base.get_supported_versions, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &versions_list, sizeof (versions_list), -1);
	status |= mock_expect_save_arg (&pfm.mock, 0, 0);

	status |= mock_expect (&pfm.mock, pfm.base.free_fw_versions, &pfm, 0, MOCK_ARG_SAVED_ARG (0));

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test,
		sizeof (struct cerberus_protocol_get_pfm_supported_fw_response) + version_len[0] +
			version_len[1],
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW, resp->header.command);
	CuAssertIntEquals (test, 1, resp->valid);
	CuAssertIntEquals (test, pfm_id, resp->version);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	CuAssertStrEquals (test, versions[0].fw_version_id,
		(char*) cerberus_protocol_pfm_supported_fw (resp));
	CuAssertStrEquals (test, versions[1].fw_version_id,
		(char*) &(cerberus_protocol_pfm_supported_fw (resp)[version_len[0]]));

	status = pfm_mock_validate_and_release (&pfm);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_port0_region1 (
	CuTest *test, struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	struct cerberus_protocol_get_pfm_supported_fw_response *resp =
		(struct cerberus_protocol_get_pfm_supported_fw_response*) request.data;
	struct pfm_mock pfm;
	struct pfm_firmware_version versions[2];
	int version_len[2];
	struct pfm_firmware_versions versions_list;
	uint32_t pfm_id = 0xAABBCCDD;
	uint32_t offset = 0;
	int status;

	versions[0].fw_version_id =
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1."
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1."
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1";
	version_len[0] = strlen (versions[0].fw_version_id) + 1;

	versions[1].fw_version_id =
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2."
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2."
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2";
	version_len[1] = strlen (versions[1].fw_version_id) + 1;

	versions_list.versions = versions;
	versions_list.count = 2;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 0;
	req->region = 1;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = pfm_mock_init (&pfm);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.get_pending_pfm, pfm_manager_0,
		(intptr_t) &pfm.base);
	status |= mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.free_pfm, pfm_manager_0, 0,
		MOCK_ARG (&pfm.base));

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm.mock, pfm.base.base.get_id, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &pfm_id, sizeof (pfm_id), -1);

	status |= mock_expect (&pfm.mock, pfm.base.get_supported_versions, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &versions_list, sizeof (versions_list), -1);
	status |= mock_expect_save_arg (&pfm.mock, 0, 0);

	status |= mock_expect (&pfm.mock, pfm.base.free_fw_versions, &pfm, 0, MOCK_ARG_SAVED_ARG (0));

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test,
		sizeof (struct cerberus_protocol_get_pfm_supported_fw_response) + version_len[0] +
			version_len[1],
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW, resp->header.command);
	CuAssertIntEquals (test, 1, resp->valid);
	CuAssertIntEquals (test, pfm_id, resp->version);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	CuAssertStrEquals (test, versions[0].fw_version_id,
		(char*) cerberus_protocol_pfm_supported_fw (resp));
	CuAssertStrEquals (test, versions[1].fw_version_id,
		(char*) &(cerberus_protocol_pfm_supported_fw (resp)[version_len[0]]));

	status = pfm_mock_validate_and_release (&pfm);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_port1_region0 (
	CuTest *test, struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_1)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	struct cerberus_protocol_get_pfm_supported_fw_response *resp =
		(struct cerberus_protocol_get_pfm_supported_fw_response*) request.data;
	struct pfm_mock pfm;
	struct pfm_firmware_version versions[2];
	int version_len[2];
	struct pfm_firmware_versions versions_list;
	uint32_t pfm_id = 0xAABBCCDD;
	uint32_t offset = 0;
	int status;

	versions[0].fw_version_id =
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1."
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1."
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1";
	version_len[0] = strlen (versions[0].fw_version_id) + 1;

	versions[1].fw_version_id =
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2."
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2."
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2";
	version_len[1] = strlen (versions[1].fw_version_id) + 1;

	versions_list.versions = versions;
	versions_list.count = 2;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 1;
	req->region = 0;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = pfm_mock_init (&pfm);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm_manager_1->mock, pfm_manager_1->base.get_active_pfm, pfm_manager_1,
		(intptr_t) &pfm.base);
	status |= mock_expect (&pfm_manager_1->mock, pfm_manager_1->base.free_pfm, pfm_manager_1, 0,
		MOCK_ARG (&pfm.base));

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm.mock, pfm.base.base.get_id, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &pfm_id, sizeof (pfm_id), -1);

	status |= mock_expect (&pfm.mock, pfm.base.get_supported_versions, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &versions_list, sizeof (versions_list), -1);
	status |= mock_expect_save_arg (&pfm.mock, 0, 0);

	status |= mock_expect (&pfm.mock, pfm.base.free_fw_versions, &pfm, 0, MOCK_ARG_SAVED_ARG (0));

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test,
		sizeof (struct cerberus_protocol_get_pfm_supported_fw_response) + version_len[0] +
			version_len[1],
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW, resp->header.command);
	CuAssertIntEquals (test, 1, resp->valid);
	CuAssertIntEquals (test, pfm_id, resp->version);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	CuAssertStrEquals (test, versions[0].fw_version_id,
		(char*) cerberus_protocol_pfm_supported_fw (resp));
	CuAssertStrEquals (test, versions[1].fw_version_id,
		(char*) &(cerberus_protocol_pfm_supported_fw (resp)[version_len[0]]));

	status = pfm_mock_validate_and_release (&pfm);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_port1_region1 (
	CuTest *test, struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_1)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	struct cerberus_protocol_get_pfm_supported_fw_response *resp =
		(struct cerberus_protocol_get_pfm_supported_fw_response*) request.data;
	struct pfm_mock pfm;
	struct pfm_firmware_version versions[2];
	int version_len[2];
	struct pfm_firmware_versions versions_list;
	uint32_t pfm_id = 0xAABBCCDD;
	uint32_t offset = 0;
	int status;

	versions[0].fw_version_id =
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1."
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1."
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1";
	version_len[0] = strlen (versions[0].fw_version_id) + 1;

	versions[1].fw_version_id =
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2."
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2."
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2";
	version_len[1] = strlen (versions[1].fw_version_id) + 1;

	versions_list.versions = versions;
	versions_list.count = 2;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 1;
	req->region = 1;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = pfm_mock_init (&pfm);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm_manager_1->mock, pfm_manager_1->base.get_pending_pfm, pfm_manager_1,
		(intptr_t) &pfm.base);
	status |= mock_expect (&pfm_manager_1->mock, pfm_manager_1->base.free_pfm, pfm_manager_1, 0,
		MOCK_ARG (&pfm.base));

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm.mock, pfm.base.base.get_id, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &pfm_id, sizeof (pfm_id), -1);

	status |= mock_expect (&pfm.mock, pfm.base.get_supported_versions, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &versions_list, sizeof (versions_list), -1);
	status |= mock_expect_save_arg (&pfm.mock, 0, 0);

	status |= mock_expect (&pfm.mock, pfm.base.free_fw_versions, &pfm, 0, MOCK_ARG_SAVED_ARG (0));

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test,
		sizeof (struct cerberus_protocol_get_pfm_supported_fw_response) + version_len[0] +
			version_len[1],
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW, resp->header.command);
	CuAssertIntEquals (test, 1, resp->valid);
	CuAssertIntEquals (test, pfm_id, resp->version);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	CuAssertStrEquals (test, versions[0].fw_version_id,
		(char*) cerberus_protocol_pfm_supported_fw (resp));
	CuAssertStrEquals (test, versions[1].fw_version_id,
		(char*) &(cerberus_protocol_pfm_supported_fw (resp)[version_len[0]]));

	status = pfm_mock_validate_and_release (&pfm);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_nonzero_offset (
	CuTest *test, struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	struct cerberus_protocol_get_pfm_supported_fw_response *resp =
		(struct cerberus_protocol_get_pfm_supported_fw_response*) request.data;
	struct pfm_mock pfm;
	struct pfm_firmware_version versions[2];
	int version_len[2];
	struct pfm_firmware_versions versions_list;
	uint32_t pfm_id = 0xAABBCCDD;
	uint32_t offset;
	int status;

	versions[0].fw_version_id =
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1."
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1."
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1";
	version_len[0] = strlen (versions[0].fw_version_id) + 1;

	versions[1].fw_version_id =
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2."
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2."
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2";
	version_len[1] = strlen (versions[1].fw_version_id) + 1;

	versions_list.versions = versions;
	versions_list.count = 2;

	offset = version_len[0];

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 0;
	req->region = 0;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = pfm_mock_init (&pfm);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.get_active_pfm, pfm_manager_0,
		(intptr_t) &pfm.base);
	status |= mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.free_pfm, pfm_manager_0, 0,
		MOCK_ARG (&pfm.base));

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm.mock, pfm.base.base.get_id, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &pfm_id, sizeof (pfm_id), -1);

	status |= mock_expect (&pfm.mock, pfm.base.get_supported_versions, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &versions_list, sizeof (versions_list), -1);
	status |= mock_expect_save_arg (&pfm.mock, 0, 0);

	status |= mock_expect (&pfm.mock, pfm.base.free_fw_versions, &pfm, 0, MOCK_ARG_SAVED_ARG (0));

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test,
		sizeof (struct cerberus_protocol_get_pfm_supported_fw_response) + version_len[1],
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW, resp->header.command);
	CuAssertIntEquals (test, 1, resp->valid);
	CuAssertIntEquals (test, pfm_id, resp->version);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	CuAssertStrEquals (test, versions[1].fw_version_id,
		(char*) cerberus_protocol_pfm_supported_fw (resp));

	status = pfm_mock_validate_and_release (&pfm);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_limited_response (
	CuTest *test, struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	struct cerberus_protocol_get_pfm_supported_fw_response *resp =
		(struct cerberus_protocol_get_pfm_supported_fw_response*) request.data;
	struct pfm_mock pfm;
	struct pfm_firmware_version versions[2];
	int version_len[2];
	struct pfm_firmware_versions versions_list;
	uint32_t pfm_id = 0xAABBCCDD;
	uint32_t offset = 0;
	int status;

	versions[0].fw_version_id =
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1."
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1."
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1";
	version_len[0] = strlen (versions[0].fw_version_id) + 1;

	versions[1].fw_version_id =
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2."
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2."
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2";
	version_len[1] = strlen (versions[1].fw_version_id) + 1;

	versions_list.versions = versions;
	versions_list.count = 2;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 0;
	req->region = 0;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response =
		version_len[0] + sizeof (struct cerberus_protocol_get_pfm_supported_fw_response);
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = pfm_mock_init (&pfm);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.get_active_pfm, pfm_manager_0,
		(intptr_t) &pfm.base);
	status |= mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.free_pfm, pfm_manager_0, 0,
		MOCK_ARG (&pfm.base));

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm.mock, pfm.base.base.get_id, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &pfm_id, sizeof (pfm_id), -1);

	status |= mock_expect (&pfm.mock, pfm.base.get_supported_versions, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &versions_list, sizeof (versions_list), -1);
	status |= mock_expect_save_arg (&pfm.mock, 0, 0);

	status |= mock_expect (&pfm.mock, pfm.base.free_fw_versions, &pfm, 0, MOCK_ARG_SAVED_ARG (0));

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test,
		sizeof (struct cerberus_protocol_get_pfm_supported_fw_response) + version_len[0],
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW, resp->header.command);
	CuAssertIntEquals (test, 1, resp->valid);
	CuAssertIntEquals (test, pfm_id, resp->version);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	CuAssertStrEquals (test, versions[0].fw_version_id,
		(char*) cerberus_protocol_pfm_supported_fw (resp));

	status = pfm_mock_validate_and_release (&pfm);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_empty_list (
	CuTest *test, struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	struct cerberus_protocol_get_pfm_supported_fw_response *resp =
		(struct cerberus_protocol_get_pfm_supported_fw_response*) request.data;
	struct pfm_mock pfm;
	struct pfm_firmware_versions versions_list;
	uint32_t pfm_id = 0xAABBCCDD;
	uint32_t offset = 0;
	int status;

	versions_list.versions = NULL;
	versions_list.count = 0;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 0;
	req->region = 0;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = pfm_mock_init (&pfm);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.get_active_pfm, pfm_manager_0,
		(intptr_t) &pfm.base);
	status |= mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.free_pfm, pfm_manager_0, 0,
		MOCK_ARG (&pfm.base));

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm.mock, pfm.base.base.get_id, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &pfm_id, sizeof (pfm_id), -1);

	status |= mock_expect (&pfm.mock, pfm.base.get_supported_versions, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &versions_list, sizeof (versions_list), -1);
	status |= mock_expect_save_arg (&pfm.mock, 0, 0);

	status |= mock_expect (&pfm.mock, pfm.base.free_fw_versions, &pfm, 0, MOCK_ARG_SAVED_ARG (0));

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_pfm_supported_fw_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW, resp->header.command);
	CuAssertIntEquals (test, 1, resp->valid);
	CuAssertIntEquals (test, pfm_id, resp->version);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = pfm_mock_validate_and_release (&pfm);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_empty_list_nonzero_offset (
	CuTest *test, struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	struct cerberus_protocol_get_pfm_supported_fw_response *resp =
		(struct cerberus_protocol_get_pfm_supported_fw_response*) request.data;
	struct pfm_mock pfm;
	struct pfm_firmware_versions versions_list;
	uint32_t pfm_id = 0xAABBCCDD;
	uint32_t offset = 1;
	int status;

	versions_list.versions = NULL;
	versions_list.count = 0;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 0;
	req->region = 0;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = pfm_mock_init (&pfm);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.get_active_pfm, pfm_manager_0,
		(intptr_t) &pfm.base);
	status |= mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.free_pfm, pfm_manager_0, 0,
		MOCK_ARG (&pfm.base));

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm.mock, pfm.base.base.get_id, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &pfm_id, sizeof (pfm_id), -1);

	status |= mock_expect (&pfm.mock, pfm.base.get_supported_versions, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &versions_list, sizeof (versions_list), -1);
	status |= mock_expect_save_arg (&pfm.mock, 0, 0);

	status |= mock_expect (&pfm.mock, pfm.base.free_fw_versions, &pfm, 0, MOCK_ARG_SAVED_ARG (0));

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_pfm_supported_fw_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW, resp->header.command);
	CuAssertIntEquals (test, 1, resp->valid);
	CuAssertIntEquals (test, pfm_id, resp->version);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = pfm_mock_validate_and_release (&pfm);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_port0_region0_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	uint32_t offset = 0;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 0;
	req->region = 0;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_port0_region1_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	uint32_t offset = 0;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 0;
	req->region = 1;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_port1_region0_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	uint32_t offset = 0;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 1;
	req->region = 0;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_port1_region1_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	uint32_t offset = 0;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 1;
	req->region = 1;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_no_active_pfm (
	CuTest *test, struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	struct cerberus_protocol_get_pfm_supported_fw_response *resp =
		(struct cerberus_protocol_get_pfm_supported_fw_response*) request.data;
	uint32_t offset = 0;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 0;
	req->region = 0;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.get_active_pfm, pfm_manager_0,
		(intptr_t) NULL);
	status |= mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.free_pfm, pfm_manager_0, 0,
		MOCK_ARG (NULL));

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_pfm_supported_fw_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW, resp->header.command);
	CuAssertIntEquals (test, 0, resp->valid);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_no_pending_pfm (
	CuTest *test, struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	struct cerberus_protocol_get_pfm_supported_fw_response *resp =
		(struct cerberus_protocol_get_pfm_supported_fw_response*) request.data;
	uint32_t offset = 0;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 0;
	req->region = 1;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.get_pending_pfm, pfm_manager_0,
		(intptr_t) NULL);
	status |= mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.free_pfm, pfm_manager_0, 0,
		MOCK_ARG (NULL));

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_pfm_supported_fw_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW, resp->header.command);
	CuAssertIntEquals (test, 0, resp->valid);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_fail_id (
	CuTest *test, struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	struct pfm_mock pfm;
	uint32_t offset = 0;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 0;
	req->region = 0;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = pfm_mock_init (&pfm);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.get_active_pfm, pfm_manager_0,
		(intptr_t) &pfm.base);
	status |= mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.free_pfm, pfm_manager_0, 0,
		MOCK_ARG (&pfm.base));

	status |= mock_expect (&pfm.mock, pfm.base.base.get_id, &pfm, PFM_NO_MEMORY, MOCK_ARG_NOT_NULL);

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, PFM_NO_MEMORY, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = pfm_mock_validate_and_release (&pfm);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_fail (
	CuTest *test, struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	struct pfm_mock pfm;
	uint32_t pfm_id = 0xAABBCCDD;
	uint32_t offset = 0;
	int status;


	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 0;
	req->region = 0;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = pfm_mock_init (&pfm);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.get_active_pfm, pfm_manager_0,
		(intptr_t) &pfm.base);
	status |= mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.free_pfm, pfm_manager_0, 0,
		MOCK_ARG (&pfm.base));

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm.mock, pfm.base.base.get_id, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &pfm_id, sizeof (pfm_id), -1);

	status |= mock_expect (&pfm.mock, pfm.base.get_supported_versions, &pfm, PFM_NO_MEMORY,
		MOCK_ARG_NOT_NULL);

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, PFM_NO_MEMORY, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = pfm_mock_validate_and_release (&pfm);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_invalid_len (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	uint32_t offset = 0;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 0;
	req->region = 0;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw) + 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw) - 1;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_invalid_region (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	uint32_t offset = 0;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 0;
	req->region = 2;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_OUT_OF_RANGE, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_invalid_offset (
	CuTest *test, struct cmd_interface *cmd, struct pfm_manager_mock *pfm_manager_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	struct cerberus_protocol_get_pfm_supported_fw_response *resp =
		(struct cerberus_protocol_get_pfm_supported_fw_response*) request.data;
	struct pfm_mock pfm;
	struct pfm_firmware_version versions[2];
	int version_len[2];
	struct pfm_firmware_versions versions_list;
	uint32_t pfm_id = 0xAABBCCDD;
	uint32_t offset;
	int status;

	versions[0].fw_version_id =
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1."
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1."
		"1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1";
	version_len[0] = strlen (versions[0].fw_version_id) + 1;

	versions[1].fw_version_id =
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2."
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2."
		"2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2.2";
	version_len[1] = strlen (versions[1].fw_version_id) + 1;

	versions_list.versions = versions;
	versions_list.count = 2;

	offset = version_len[0] + version_len[1];

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 0;
	req->region = 0;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = pfm_mock_init (&pfm);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.get_active_pfm, pfm_manager_0,
		(intptr_t) &pfm.base);
	status |= mock_expect (&pfm_manager_0->mock, pfm_manager_0->base.free_pfm, pfm_manager_0, 0,
		MOCK_ARG (&pfm.base));

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&pfm.mock, pfm.base.base.get_id, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &pfm_id, sizeof (pfm_id), -1);

	status |= mock_expect (&pfm.mock, pfm.base.get_supported_versions, &pfm, 0, MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&pfm.mock, 0, &versions_list, sizeof (versions_list), -1);
	status |= mock_expect_save_arg (&pfm.mock, 0, 0);

	status |= mock_expect (&pfm.mock, pfm.base.free_fw_versions, &pfm, 0, MOCK_ARG_SAVED_ARG (0));

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_pfm_supported_fw_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW, resp->header.command);
	CuAssertIntEquals (test, 1, resp->valid);
	CuAssertIntEquals (test, pfm_id, resp->version);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = pfm_mock_validate_and_release (&pfm);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_pfm_supported_fw_invalid_port (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_pfm_supported_fw *req =
		(struct cerberus_protocol_get_pfm_supported_fw*) request.data;
	uint32_t offset = 0;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW;

	req->port_id = 2;
	req->region = 0;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_pfm_supported_fw);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_OUT_OF_RANGE, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_log_clear_debug (CuTest *test,
	struct cmd_interface *cmd, struct cmd_background_mock *background)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_clear_log *req = (struct cerberus_protocol_clear_log*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_CLEAR_LOG;

	req->log_type = 1;
	request.length = sizeof (struct cerberus_protocol_clear_log);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&background->mock, background->base.debug_log_clear, background, 0);
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_log_clear_tcg (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_clear_log *req = (struct cerberus_protocol_clear_log*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_CLEAR_LOG;

	req->log_type = 2;
	request.length = sizeof (struct cerberus_protocol_clear_log);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_log_clear_invalid_len (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_clear_log *req = (struct cerberus_protocol_clear_log*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_CLEAR_LOG;

	req->log_type = 2;
	request.length = sizeof (struct cerberus_protocol_clear_log) + 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	request.length = sizeof (struct cerberus_protocol_clear_log) - 1;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_log_clear_invalid_type (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_clear_log *req = (struct cerberus_protocol_clear_log*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_CLEAR_LOG;

	req->log_type = 3;
	request.length = sizeof (struct cerberus_protocol_clear_log);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_log_clear_debug_fail (CuTest *test,
	struct cmd_interface *cmd, struct cmd_background_mock *background)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_clear_log *req = (struct cerberus_protocol_clear_log*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_CLEAR_LOG;

	req->log_type = 1;
	request.length = sizeof (struct cerberus_protocol_clear_log);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&background->mock, background->base.debug_log_clear, background,
		CMD_BACKGROUND_NO_MEMORY);
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_BACKGROUND_NO_MEMORY, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_log_info (CuTest *test,
	struct cmd_interface *cmd, struct logging_mock *debug, int tcg_entries)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_log_info *req =
		(struct cerberus_protocol_get_log_info*) request.data;
	struct cerberus_protocol_get_log_info_response *resp =
		(struct cerberus_protocol_get_log_info_response*) request.data;
	uint32_t debug_size = 15;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_LOG_INFO;

	request.length = sizeof (struct cerberus_protocol_get_log_info);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	debug_log = &debug->base;

	status = mock_expect (&debug->mock, debug->base.get_size, debug, debug_size);
	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_log_info_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_LOG_INFO, resp->header.command);
	CuAssertIntEquals (test, debug_size, resp->debug_log_length);
	CuAssertIntEquals (test, tcg_entries * sizeof (struct pcr_store_tcg_log_entry),
		resp->attestation_log_length);
	CuAssertIntEquals (test, 0, resp->tamper_log_length);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	debug_log = NULL;
}

void cerberus_protocol_optional_commands_testing_process_get_log_info_invalid_len (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_log_info *req =
		(struct cerberus_protocol_get_log_info*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_LOG_INFO;

	request.length = sizeof (struct cerberus_protocol_get_log_info) + 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_log_info_fail_debug (CuTest *test,
	struct cmd_interface *cmd, struct logging_mock *debug, int tcg_entries)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_log_info *req =
		(struct cerberus_protocol_get_log_info*) request.data;
	struct cerberus_protocol_get_log_info_response *resp =
		(struct cerberus_protocol_get_log_info_response*) request.data;
	uint32_t debug_size = 0;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_LOG_INFO;

	request.length = sizeof (struct cerberus_protocol_get_log_info);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	debug_log = &debug->base;

	status = mock_expect (&debug->mock, debug->base.get_size, debug, LOGGING_GET_SIZE_FAILED);
	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_log_info_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_LOG_INFO, resp->header.command);
	CuAssertIntEquals (test, debug_size, resp->debug_log_length);
	CuAssertIntEquals (test, tcg_entries * sizeof (struct pcr_store_tcg_log_entry),
		resp->attestation_log_length);
	CuAssertIntEquals (test, 0, resp->tamper_log_length);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	debug_log = NULL;
}

void cerberus_protocol_optional_commands_testing_process_log_read_debug (CuTest *test,
	struct cmd_interface *cmd, struct logging_mock *debug)
{
	uint8_t entry[256 * sizeof (struct debug_log_entry)];
	struct cmd_interface_request request;
	struct cerberus_protocol_get_log *req = (struct cerberus_protocol_get_log*) request.data;
	struct cerberus_protocol_get_log_response *resp =
		(struct cerberus_protocol_get_log_response*) request.data;
	uint32_t offset = 0;
	int status;
	int i_entry;
	int max = CERBERUS_PROTOCOL_MAX_PAYLOAD_PER_MSG;
	int remain = sizeof (entry) - max;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_READ_LOG;

	req->log_type = 1;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_log);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	for (i_entry = 0; i_entry < 256; ++i_entry) {
		struct debug_log_entry *contents =
			(struct debug_log_entry*) &entry[i_entry * sizeof (struct debug_log_entry)];
		contents->header.log_magic = 0xCB;
		contents->header.length = sizeof (struct debug_log_entry);
		contents->header.entry_id = i_entry;
		contents->entry.format = DEBUG_LOG_ENTRY_FORMAT;
		contents->entry.severity = 1;
		contents->entry.component = 2;
		contents->entry.msg_index = 3;
		contents->entry.arg1 = 4;
		contents->entry.arg2 = 5;
	}

	debug_log = &debug->base;

	status = mock_expect (&debug->mock, debug->base.read_contents, debug, max, MOCK_ARG (0),
		MOCK_ARG_NOT_NULL, MOCK_ARG (max));
	status |= mock_expect_output (&debug->mock, 1, entry, sizeof (entry), 2);

	status |= mock_expect (&debug->mock, debug->base.read_contents, debug, remain, MOCK_ARG (max),
		MOCK_ARG_NOT_NULL, MOCK_ARG (max));
	status |= mock_expect_output (&debug->mock, 1, &entry[max], remain, 2);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_log_response) + max,
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_READ_LOG, resp->header.command);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = testing_validate_array (entry, cerberus_protocol_log_data (resp), max);
	CuAssertIntEquals (test, 0, status);

	offset = max;
	req->log_type = CERBERUS_PROTOCOL_DEBUG_LOG;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_log);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_log_response) + remain,
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_READ_LOG, resp->header.command);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = testing_validate_array (&entry[offset], cerberus_protocol_log_data (resp), remain);
	CuAssertIntEquals (test, 0, status);

	debug_log = NULL;
}

void cerberus_protocol_optional_commands_testing_process_log_read_debug_limited_response (
	CuTest *test, struct cmd_interface *cmd, struct logging_mock *debug)
{
	uint8_t entry[256 * sizeof (struct debug_log_entry)];
	struct cmd_interface_request request;
	struct cerberus_protocol_get_log *req = (struct cerberus_protocol_get_log*) request.data;
	struct cerberus_protocol_get_log_response *resp =
		(struct cerberus_protocol_get_log_response*) request.data;
	uint32_t offset = 0;
	int status;
	int i_entry;
	int max = CERBERUS_PROTOCOL_MAX_PAYLOAD_PER_MSG - 128;
	int remain = sizeof (entry) - max;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_READ_LOG;

	req->log_type = 1;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_log);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY - 128;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	for (i_entry = 0; i_entry < 256; ++i_entry) {
		struct debug_log_entry *contents =
			(struct debug_log_entry*) &entry[i_entry * sizeof (struct debug_log_entry)];
		contents->header.log_magic = 0xCB;
		contents->header.length = sizeof (struct debug_log_entry);
		contents->header.entry_id = i_entry;
		contents->entry.format = DEBUG_LOG_ENTRY_FORMAT;
		contents->entry.severity = 1;
		contents->entry.component = 2;
		contents->entry.msg_index = 3;
		contents->entry.arg1 = 4;
		contents->entry.arg2 = 5;
	}

	debug_log = &debug->base;

	status = mock_expect (&debug->mock, debug->base.read_contents, debug, max, MOCK_ARG (0),
		MOCK_ARG_NOT_NULL, MOCK_ARG (max));
	status |= mock_expect_output (&debug->mock, 1, entry, sizeof (entry), 2);

	status |= mock_expect (&debug->mock, debug->base.read_contents, debug, remain, MOCK_ARG (max),
		MOCK_ARG_NOT_NULL, MOCK_ARG (max));
	status |= mock_expect_output (&debug->mock, 1, &entry[max], remain, 2);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_log_response) + max,
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_READ_LOG, resp->header.command);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = testing_validate_array (entry, cerberus_protocol_log_data (resp), max);
	CuAssertIntEquals (test, 0, status);

	offset = max;
	req->log_type = CERBERUS_PROTOCOL_DEBUG_LOG;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_log);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_log_response) + remain,
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_READ_LOG, resp->header.command);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = testing_validate_array (&entry[offset], cerberus_protocol_log_data (resp), remain);
	CuAssertIntEquals (test, 0, status);

	debug_log = NULL;
}

void cerberus_protocol_optional_commands_testing_process_log_read_tcg (CuTest *test,
	struct cmd_interface *cmd, struct hash_engine_mock *hash, struct pcr_store *store)
{
	struct pcr_store_tcg_log_entry exp_buf[6];
	uint8_t buffer0[PCR_DIGEST_LENGTH] = {0};
	uint8_t digests[6][PCR_DIGEST_LENGTH] = {
		{
			0xab,0xe6,0xe6,0x4f,0x38,0x13,0x4f,0x82,0x18,0x33,0xf6,0x5b,0x12,0xc7,0xe7,0x6e,
			0x7f,0xe6,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x7f,0x6e
		},
		{
			0xcd,0xe6,0xe6,0x4f,0x38,0x13,0x4f,0x82,0x18,0x33,0xf6,0x5b,0x12,0xc7,0xe7,0x6e,
			0x7f,0xe6,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x7f,0x6e
		},
		{
			0xef,0xe6,0xe6,0x4f,0x38,0x13,0x4f,0x82,0x18,0x33,0xf6,0x5b,0x12,0xc7,0xe7,0x6e,
			0x7f,0xe6,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x7f,0x6e
		},
		{
			0x12,0xe6,0xe6,0x4f,0x38,0x13,0x4f,0x82,0x18,0x33,0xf6,0x5b,0x12,0xc7,0xe7,0x6e,
			0x7f,0xe6,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x7f,0x6e
		},
		{
			0x23,0xe6,0xe6,0x4f,0x38,0x13,0x4f,0x82,0x18,0x33,0xf6,0x5b,0x12,0xc7,0xe7,0x6e,
			0x7f,0xe6,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x7f,0x6e
		},
		{
			0x45,0xe6,0xe6,0x4f,0x38,0x13,0x4f,0x82,0x18,0x33,0xf6,0x5b,0x12,0xc7,0xe7,0x6e,
			0x7f,0xe6,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x7f,0x6e
		},
	};
	uint32_t offset = 0;
	struct cmd_interface_request request;
	struct cerberus_protocol_get_log *req = (struct cerberus_protocol_get_log*) request.data;
	struct cerberus_protocol_get_log_response *resp =
		(struct cerberus_protocol_get_log_response*) request.data;
	int status;
	int i_measurement;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_READ_LOG;

	req->log_type = 2;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_log);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	memset (exp_buf, 0, sizeof (exp_buf));
	for (i_measurement = 0; i_measurement < 6; ++i_measurement) {
		exp_buf[i_measurement].header.log_magic = 0xCB;
		exp_buf[i_measurement].header.length = sizeof (struct pcr_store_tcg_log_entry);
		exp_buf[i_measurement].header.entry_id = i_measurement;
		exp_buf[i_measurement].entry.digest_algorithm_id = 0x0B;
		exp_buf[i_measurement].entry.digest_count = 1;
		exp_buf[i_measurement].entry.measurement_size = 32;
		exp_buf[i_measurement].entry.event_type = 0xA + i_measurement;
		exp_buf[i_measurement].entry.measurement_type = PCR_MEASUREMENT (0, i_measurement);

		memcpy (exp_buf[i_measurement].entry.digest, digests[i_measurement],
			sizeof (exp_buf[i_measurement].entry.digest));
		memcpy (exp_buf[i_measurement].entry.measurement, digests[5 - i_measurement],
			sizeof (exp_buf[i_measurement].entry.measurement));
	}

	status = mock_expect (&hash->mock, hash->base.start_sha256, hash, 0);
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (buffer0, PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[0], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.finish, hash, 0,
		MOCK_ARG_NOT_NULL, MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect_output (&hash->mock, 0, digests[5], PCR_DIGEST_LENGTH, -1);

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&hash->mock, hash->base.start_sha256, hash, 0);
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[5], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[1], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.finish, hash, 0,
		MOCK_ARG_NOT_NULL, MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect_output (&hash->mock, 0, digests[4], PCR_DIGEST_LENGTH, -1);

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&hash->mock, hash->base.start_sha256, hash, 0);
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[4], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[2], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.finish, hash, 0,
		MOCK_ARG_NOT_NULL, MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect_output (&hash->mock, 0, digests[3], PCR_DIGEST_LENGTH, -1);

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&hash->mock, hash->base.start_sha256, hash, 0);
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[3], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[3], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.finish, hash, 0,
		MOCK_ARG_NOT_NULL, MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect_output (&hash->mock, 0, digests[2], PCR_DIGEST_LENGTH, -1);

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&hash->mock, hash->base.start_sha256, hash, 0);
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[2], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[4], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.finish, hash, 0,
		MOCK_ARG_NOT_NULL, MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect_output (&hash->mock, 0, digests[1], PCR_DIGEST_LENGTH, -1);

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&hash->mock, hash->base.start_sha256, hash, 0);
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[1], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[5], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.finish, hash, 0,
		MOCK_ARG_NOT_NULL, MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect_output (&hash->mock, 0, digests[0], PCR_DIGEST_LENGTH, -1);

	CuAssertIntEquals (test, 0, status);

	for (i_measurement = 0; i_measurement < 6; ++i_measurement) {
		pcr_store_update_digest (store, PCR_MEASUREMENT (0, i_measurement),
			digests[i_measurement], PCR_DIGEST_LENGTH);
		pcr_store_update_event_type (store, PCR_MEASUREMENT (0, i_measurement),
			0x0A + i_measurement);
	}

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_log_response) + sizeof (exp_buf),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_READ_LOG, resp->header.command);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = testing_validate_array ((uint8_t*) exp_buf, cerberus_protocol_log_data (resp),
		sizeof (exp_buf));
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_log_read_tcg_limited_response (
	CuTest *test, struct cmd_interface *cmd, struct hash_engine_mock *hash, struct pcr_store *store)
{
	struct pcr_store_tcg_log_entry exp_buf[6];
	uint8_t buffer0[PCR_DIGEST_LENGTH] = {0};
	uint8_t digests[6][PCR_DIGEST_LENGTH] = {
		{
			0xab,0xe6,0xe6,0x4f,0x38,0x13,0x4f,0x82,0x18,0x33,0xf6,0x5b,0x12,0xc7,0xe7,0x6e,
			0x7f,0xe6,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x7f,0x6e
		},
		{
			0xcd,0xe6,0xe6,0x4f,0x38,0x13,0x4f,0x82,0x18,0x33,0xf6,0x5b,0x12,0xc7,0xe7,0x6e,
			0x7f,0xe6,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x7f,0x6e
		},
		{
			0xef,0xe6,0xe6,0x4f,0x38,0x13,0x4f,0x82,0x18,0x33,0xf6,0x5b,0x12,0xc7,0xe7,0x6e,
			0x7f,0xe6,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x7f,0x6e
		},
		{
			0x12,0xe6,0xe6,0x4f,0x38,0x13,0x4f,0x82,0x18,0x33,0xf6,0x5b,0x12,0xc7,0xe7,0x6e,
			0x7f,0xe6,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x7f,0x6e
		},
		{
			0x23,0xe6,0xe6,0x4f,0x38,0x13,0x4f,0x82,0x18,0x33,0xf6,0x5b,0x12,0xc7,0xe7,0x6e,
			0x7f,0xe6,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x7f,0x6e
		},
		{
			0x45,0xe6,0xe6,0x4f,0x38,0x13,0x4f,0x82,0x18,0x33,0xf6,0x5b,0x12,0xc7,0xe7,0x6e,
			0x7f,0xe6,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x9c,0x4f,0x7f,0x38,0x7f,0x6e
		},
	};
	uint32_t offset = 0;
	struct cmd_interface_request request;
	struct cerberus_protocol_get_log *req = (struct cerberus_protocol_get_log*) request.data;
	struct cerberus_protocol_get_log_response *resp =
		(struct cerberus_protocol_get_log_response*) request.data;
	int status;
	int max = sizeof (exp_buf) - 10 - sizeof (struct cerberus_protocol_get_log_response);
	int i_measurement;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_READ_LOG;

	req->log_type = 2;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_log);
	request.max_response = sizeof (exp_buf) - 10;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	memset (exp_buf, 0, sizeof (exp_buf));
	for (i_measurement = 0; i_measurement < 6; ++i_measurement) {
		exp_buf[i_measurement].header.log_magic = 0xCB;
		exp_buf[i_measurement].header.length = sizeof (struct pcr_store_tcg_log_entry);
		exp_buf[i_measurement].header.entry_id = i_measurement;
		exp_buf[i_measurement].entry.digest_algorithm_id = 0x0B;
		exp_buf[i_measurement].entry.digest_count = 1;
		exp_buf[i_measurement].entry.measurement_size = 32;
		exp_buf[i_measurement].entry.event_type = 0xA + i_measurement;
		exp_buf[i_measurement].entry.measurement_type = PCR_MEASUREMENT (0, i_measurement);

		memcpy (exp_buf[i_measurement].entry.digest, digests[i_measurement],
			sizeof (exp_buf[i_measurement].entry.digest));
		memcpy (exp_buf[i_measurement].entry.measurement, digests[5 - i_measurement],
			sizeof (exp_buf[i_measurement].entry.measurement));
	}

	status = mock_expect (&hash->mock, hash->base.start_sha256, hash, 0);
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (buffer0, PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[0], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.finish, hash, 0,
		MOCK_ARG_NOT_NULL, MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect_output (&hash->mock, 0, digests[5], PCR_DIGEST_LENGTH, -1);

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&hash->mock, hash->base.start_sha256, hash, 0);
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[5], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[1], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.finish, hash, 0,
		MOCK_ARG_NOT_NULL, MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect_output (&hash->mock, 0, digests[4], PCR_DIGEST_LENGTH, -1);

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&hash->mock, hash->base.start_sha256, hash, 0);
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[4], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[2], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.finish, hash, 0,
		MOCK_ARG_NOT_NULL, MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect_output (&hash->mock, 0, digests[3], PCR_DIGEST_LENGTH, -1);

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&hash->mock, hash->base.start_sha256, hash, 0);
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[3], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[3], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.finish, hash, 0,
		MOCK_ARG_NOT_NULL, MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect_output (&hash->mock, 0, digests[2], PCR_DIGEST_LENGTH, -1);

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&hash->mock, hash->base.start_sha256, hash, 0);
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[2], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[4], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.finish, hash, 0,
		MOCK_ARG_NOT_NULL, MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect_output (&hash->mock, 0, digests[1], PCR_DIGEST_LENGTH, -1);

	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&hash->mock, hash->base.start_sha256, hash, 0);
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[1], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.update, hash, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (digests[5], PCR_DIGEST_LENGTH), MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect (&hash->mock, hash->base.finish, hash, 0,
		MOCK_ARG_NOT_NULL, MOCK_ARG (PCR_DIGEST_LENGTH));
	status |= mock_expect_output (&hash->mock, 0, digests[0], PCR_DIGEST_LENGTH, -1);

	CuAssertIntEquals (test, 0, status);

	for (i_measurement = 0; i_measurement < 6; ++i_measurement) {
		pcr_store_update_digest (store, PCR_MEASUREMENT (0, i_measurement),
			digests[i_measurement], PCR_DIGEST_LENGTH);
		pcr_store_update_event_type (store, PCR_MEASUREMENT (0, i_measurement),
			0x0A + i_measurement);
	}

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_log_response) + max,
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_READ_LOG, resp->header.command);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = testing_validate_array ((uint8_t*) exp_buf, cerberus_protocol_log_data (resp), max);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_log_read_debug_fail (CuTest *test,
	struct cmd_interface *cmd, struct logging_mock *debug)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_log *req = (struct cerberus_protocol_get_log*) request.data;
	uint32_t offset = 0;
	int max = CERBERUS_PROTOCOL_MAX_PAYLOAD_PER_MSG;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_READ_LOG;

	req->log_type = 1;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_log);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	debug_log = &debug->base;

	status = mock_expect (&debug->mock, debug->base.read_contents, debug,
		LOGGING_READ_CONTENTS_FAILED, MOCK_ARG (0), MOCK_ARG_NOT_NULL, MOCK_ARG (max));

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, LOGGING_READ_CONTENTS_FAILED, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	debug_log = NULL;
}

void cerberus_protocol_optional_commands_testing_process_log_read_tcg_fail (CuTest *test,
	struct cmd_interface *cmd, struct hash_engine_mock *hash, struct pcr_store *store)
{
	uint8_t buffer0[PCR_DIGEST_LENGTH] = {0};
	uint32_t offset = 0;
	struct cmd_interface_request request;
	struct cerberus_protocol_get_log *req = (struct cerberus_protocol_get_log*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_READ_LOG;

	req->log_type = 2;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_log);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&hash->mock, hash->base.start_sha256, hash, HASH_ENGINE_NO_MEMORY);
	CuAssertIntEquals (test, 0, status);

	pcr_store_update_digest (store, PCR_MEASUREMENT (0, 0), buffer0, PCR_DIGEST_LENGTH);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, HASH_ENGINE_NO_MEMORY, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_log_read_invalid_offset (CuTest *test,
	struct cmd_interface *cmd, struct logging_mock *debug)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_log *req = (struct cerberus_protocol_get_log*) request.data;
	struct cerberus_protocol_get_log_response *resp =
		(struct cerberus_protocol_get_log_response*) request.data;
	uint32_t offset = 500;
	int status;
	int max = CERBERUS_PROTOCOL_MAX_PAYLOAD_PER_MSG;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_READ_LOG;

	req->log_type = 1;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_log);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	debug_log = &debug->base;

	status = mock_expect (&debug->mock, debug->base.read_contents, debug, 0, MOCK_ARG (500),
		MOCK_ARG_NOT_NULL, MOCK_ARG (max));
	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_log_response), request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_READ_LOG, resp->header.command);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	debug_log = NULL;
}

void cerberus_protocol_optional_commands_testing_process_log_read_invalid_type (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_log *req = (struct cerberus_protocol_get_log*) request.data;
	uint32_t offset = 0;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_READ_LOG;

	req->log_type = 4;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_log);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_log_read_invalid_len (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_log *req = (struct cerberus_protocol_get_log*) request.data;
	uint32_t offset = 0;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_READ_LOG;

	req->log_type = 4;
	req->offset = offset;
	request.length = sizeof (struct cerberus_protocol_get_log) + 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	request.length = sizeof (struct cerberus_protocol_get_log) - 1;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_rsa (CuTest *test,
	struct cmd_interface *cmd, struct cmd_background_mock *background)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal *req =
		(struct cerberus_protocol_message_unseal*) request.data;
	struct cerberus_protocol_unseal_pmrs sealing;
	int status;

	memset (sealing.pmr[0], 0, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[1], 1, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[2], 2, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[3], 3, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[4], 4, sizeof (sealing.pmr[0]));

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE;

	req->hmac_type = CERBERUS_PROTOCOL_UNSEAL_HMAC_SHA256;
	req->seed_type = CERBERUS_PROTOCOL_UNSEAL_SEED_RSA;
	req->seed_length = KEY_SEED_ENCRYPT_OAEP_LEN;
	memcpy (&req->seed, KEY_SEED_ENCRYPT_OAEP, KEY_SEED_ENCRYPT_OAEP_LEN);
	cerberus_protocol_unseal_ciphertext_length (req) = CIPHER_TEXT_LEN;
	memcpy (cerberus_protocol_unseal_ciphertext (req), CIPHER_TEXT, CIPHER_TEXT_LEN);
	cerberus_protocol_unseal_hmac_length (req) = PAYLOAD_HMAC_LEN;
	memcpy (cerberus_protocol_unseal_hmac (req), PAYLOAD_HMAC, PAYLOAD_HMAC_LEN);
	memcpy ((uint8_t*) cerberus_protocol_get_unseal_pmr_sealing (req), &sealing, sizeof (sealing));
	request.length = (sizeof (struct cerberus_protocol_message_unseal) - 1) +
		KEY_SEED_ENCRYPT_OAEP_LEN + 2 + CIPHER_TEXT_LEN + 2 + PAYLOAD_HMAC_LEN + sizeof (sealing);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&background->mock, background->base.unseal_start, background, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (request.data, request.length), MOCK_ARG (request.length));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_ecc (CuTest *test,
	struct cmd_interface *cmd, struct cmd_background_mock *background)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal *req =
		(struct cerberus_protocol_message_unseal*) request.data;
	struct cerberus_protocol_unseal_pmrs sealing;
	int status;

	memset (sealing.pmr[0], 0, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[1], 1, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[2], 2, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[3], 3, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[4], 4, sizeof (sealing.pmr[0]));

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE;

	req->hmac_type = CERBERUS_PROTOCOL_UNSEAL_HMAC_SHA256;
	req->seed_type = CERBERUS_PROTOCOL_UNSEAL_SEED_ECDH;
	req->seed_length = ECC_PUBKEY_DER_LEN;
	memcpy (&req->seed, ECC_PUBKEY_DER, ECC_PUBKEY_DER_LEN);
	cerberus_protocol_unseal_ciphertext_length (req) = CIPHER_TEXT_LEN;
	memcpy (cerberus_protocol_unseal_ciphertext (req), CIPHER_TEXT, CIPHER_TEXT_LEN);
	cerberus_protocol_unseal_hmac_length (req) = PAYLOAD_HMAC_LEN;
	memcpy (cerberus_protocol_unseal_hmac (req), PAYLOAD_HMAC, PAYLOAD_HMAC_LEN);
	memcpy ((uint8_t*) cerberus_protocol_get_unseal_pmr_sealing (req), &sealing, sizeof (sealing));
	request.length = (sizeof (struct cerberus_protocol_message_unseal) - 1) +
		ECC_PUBKEY_DER_LEN + 2 + CIPHER_TEXT_LEN + 2 + PAYLOAD_HMAC_LEN + sizeof (sealing);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&background->mock, background->base.unseal_start, background, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (request.data, request.length), MOCK_ARG (request.length));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_fail (CuTest *test,
	struct cmd_interface *cmd, struct cmd_background_mock *background)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal *req =
		(struct cerberus_protocol_message_unseal*) request.data;
	struct cerberus_protocol_unseal_pmrs sealing;
	int status;

	memset (sealing.pmr[0], 0, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[1], 1, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[2], 2, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[3], 3, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[4], 4, sizeof (sealing.pmr[0]));

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE;

	req->hmac_type = CERBERUS_PROTOCOL_UNSEAL_HMAC_SHA256;
	req->seed_type = CERBERUS_PROTOCOL_UNSEAL_SEED_RSA;
	req->seed_length = KEY_SEED_ENCRYPT_OAEP_LEN;
	memcpy (&req->seed, KEY_SEED_ENCRYPT_OAEP, KEY_SEED_ENCRYPT_OAEP_LEN);
	cerberus_protocol_unseal_ciphertext_length (req) = CIPHER_TEXT_LEN;
	memcpy (cerberus_protocol_unseal_ciphertext (req), CIPHER_TEXT, CIPHER_TEXT_LEN);
	cerberus_protocol_unseal_hmac_length (req) = PAYLOAD_HMAC_LEN;
	memcpy (cerberus_protocol_unseal_hmac (req), PAYLOAD_HMAC, PAYLOAD_HMAC_LEN);
	memcpy ((uint8_t*) cerberus_protocol_get_unseal_pmr_sealing (req), &sealing, sizeof (sealing));
	request.length = (sizeof (struct cerberus_protocol_message_unseal) - 1) +
		KEY_SEED_ENCRYPT_OAEP_LEN + 2 + CIPHER_TEXT_LEN + 2 + PAYLOAD_HMAC_LEN + sizeof (sealing);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&background->mock, background->base.unseal_start, background,
		CMD_BACKGROUND_UNSEAL_FAILED, MOCK_ARG_PTR_CONTAINS_TMP (request.data, request.length),
		MOCK_ARG (request.length));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_BACKGROUND_UNSEAL_FAILED, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_invalid_hmac (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal *req =
		(struct cerberus_protocol_message_unseal*) request.data;
	struct cerberus_protocol_unseal_pmrs sealing;
	int status;

	memset (sealing.pmr[0], 0, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[1], 1, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[2], 2, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[3], 3, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[4], 4, sizeof (sealing.pmr[0]));

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE;

	req->hmac_type = 1;
	req->seed_type = CERBERUS_PROTOCOL_UNSEAL_SEED_RSA;
	req->seed_length = KEY_SEED_ENCRYPT_OAEP_LEN;
	memcpy (&req->seed, KEY_SEED_ENCRYPT_OAEP, KEY_SEED_ENCRYPT_OAEP_LEN);
	cerberus_protocol_unseal_ciphertext_length (req) = CIPHER_TEXT_LEN;
	memcpy (cerberus_protocol_unseal_ciphertext (req), CIPHER_TEXT, CIPHER_TEXT_LEN);
	cerberus_protocol_unseal_hmac_length (req) = PAYLOAD_HMAC_LEN;
	memcpy (cerberus_protocol_unseal_hmac (req), PAYLOAD_HMAC, PAYLOAD_HMAC_LEN);
	memcpy ((uint8_t*) cerberus_protocol_get_unseal_pmr_sealing (req), &sealing, sizeof (sealing));
	request.length = (sizeof (struct cerberus_protocol_message_unseal) - 1) +
		KEY_SEED_ENCRYPT_OAEP_LEN + 2 + CIPHER_TEXT_LEN + 2 + PAYLOAD_HMAC_LEN + sizeof (sealing);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_OUT_OF_RANGE, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_invalid_seed (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal *req =
		(struct cerberus_protocol_message_unseal*) request.data;
	struct cerberus_protocol_unseal_pmrs sealing;
	int status;

	memset (sealing.pmr[0], 0, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[1], 1, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[2], 2, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[3], 3, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[4], 4, sizeof (sealing.pmr[0]));

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE;

	req->hmac_type = CERBERUS_PROTOCOL_UNSEAL_HMAC_SHA256;
	req->seed_type = 2;
	req->seed_length = KEY_SEED_ENCRYPT_OAEP_LEN;
	memcpy (&req->seed, KEY_SEED_ENCRYPT_OAEP, KEY_SEED_ENCRYPT_OAEP_LEN);
	cerberus_protocol_unseal_ciphertext_length (req) = CIPHER_TEXT_LEN;
	memcpy (cerberus_protocol_unseal_ciphertext (req), CIPHER_TEXT, CIPHER_TEXT_LEN);
	cerberus_protocol_unseal_hmac_length (req) = PAYLOAD_HMAC_LEN;
	memcpy (cerberus_protocol_unseal_hmac (req), PAYLOAD_HMAC, PAYLOAD_HMAC_LEN);
	memcpy ((uint8_t*) cerberus_protocol_get_unseal_pmr_sealing (req), &sealing, sizeof (sealing));
	request.length = (sizeof (struct cerberus_protocol_message_unseal) - 1) +
		KEY_SEED_ENCRYPT_OAEP_LEN + 2 + CIPHER_TEXT_LEN + 2 + PAYLOAD_HMAC_LEN + sizeof (sealing);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_OUT_OF_RANGE, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_rsa_invalid_padding (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal *req =
		(struct cerberus_protocol_message_unseal*) request.data;
	struct cerberus_protocol_unseal_pmrs sealing;
	int status;

	memset (sealing.pmr[0], 0, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[1], 1, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[2], 2, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[3], 3, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[4], 4, sizeof (sealing.pmr[0]));

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE;

	req->hmac_type = CERBERUS_PROTOCOL_UNSEAL_HMAC_SHA256;
	req->seed_type = CERBERUS_PROTOCOL_UNSEAL_SEED_RSA;
	req->seed_params.rsa.padding = 3;
	req->seed_length = KEY_SEED_ENCRYPT_OAEP_LEN;
	memcpy (&req->seed, KEY_SEED_ENCRYPT_OAEP, KEY_SEED_ENCRYPT_OAEP_LEN);
	cerberus_protocol_unseal_ciphertext_length (req) = CIPHER_TEXT_LEN;
	memcpy (cerberus_protocol_unseal_ciphertext (req), CIPHER_TEXT, CIPHER_TEXT_LEN);
	cerberus_protocol_unseal_hmac_length (req) = PAYLOAD_HMAC_LEN;
	memcpy (cerberus_protocol_unseal_hmac (req), PAYLOAD_HMAC, PAYLOAD_HMAC_LEN);
	memcpy ((uint8_t*) cerberus_protocol_get_unseal_pmr_sealing (req), &sealing, sizeof (sealing));
	request.length = (sizeof (struct cerberus_protocol_message_unseal) - 1) +
		KEY_SEED_ENCRYPT_OAEP_LEN + 2 + CIPHER_TEXT_LEN + 2 + PAYLOAD_HMAC_LEN + sizeof (sealing);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_OUT_OF_RANGE, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_no_seed (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal *req =
		(struct cerberus_protocol_message_unseal*) request.data;
	struct cerberus_protocol_unseal_pmrs sealing;
	int status;

	memset (sealing.pmr[0], 0, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[1], 1, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[2], 2, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[3], 3, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[4], 4, sizeof (sealing.pmr[0]));

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE;

	req->hmac_type = CERBERUS_PROTOCOL_UNSEAL_HMAC_SHA256;
	req->seed_type = CERBERUS_PROTOCOL_UNSEAL_SEED_RSA;
	req->seed_length = 0;
	cerberus_protocol_unseal_ciphertext_length (req) = CIPHER_TEXT_LEN;
	memcpy (cerberus_protocol_unseal_ciphertext (req), CIPHER_TEXT, CIPHER_TEXT_LEN);
	cerberus_protocol_unseal_hmac_length (req) = PAYLOAD_HMAC_LEN;
	memcpy (cerberus_protocol_unseal_hmac (req), PAYLOAD_HMAC, PAYLOAD_HMAC_LEN);
	memcpy ((uint8_t*) cerberus_protocol_get_unseal_pmr_sealing (req), &sealing, sizeof (sealing));
	request.length = (sizeof (struct cerberus_protocol_message_unseal) - 1) + 2 + CIPHER_TEXT_LEN +
		2 + PAYLOAD_HMAC_LEN + sizeof (sealing);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_incomplete_seed (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal *req =
		(struct cerberus_protocol_message_unseal*) request.data;
	struct cerberus_protocol_unseal_pmrs sealing;
	int status;

	memset (sealing.pmr[0], 0, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[1], 1, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[2], 2, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[3], 3, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[4], 4, sizeof (sealing.pmr[0]));

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE;

	req->hmac_type = CERBERUS_PROTOCOL_UNSEAL_HMAC_SHA256;
	req->seed_type = CERBERUS_PROTOCOL_UNSEAL_SEED_RSA;
	req->seed_length = KEY_SEED_ENCRYPT_OAEP_LEN;
	memcpy (&req->seed, KEY_SEED_ENCRYPT_OAEP, KEY_SEED_ENCRYPT_OAEP_LEN);
	cerberus_protocol_unseal_ciphertext_length (req) = CIPHER_TEXT_LEN;
	memcpy (cerberus_protocol_unseal_ciphertext (req), CIPHER_TEXT, CIPHER_TEXT_LEN);
	cerberus_protocol_unseal_hmac_length (req) = PAYLOAD_HMAC_LEN;
	memcpy (cerberus_protocol_unseal_hmac (req), PAYLOAD_HMAC, PAYLOAD_HMAC_LEN);
	memcpy ((uint8_t*) cerberus_protocol_get_unseal_pmr_sealing (req), &sealing, sizeof (sealing));
	request.length = (sizeof (struct cerberus_protocol_message_unseal) - 1) +
		KEY_SEED_ENCRYPT_OAEP_LEN - 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_no_ciphertext (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal *req =
		(struct cerberus_protocol_message_unseal*) request.data;
	struct cerberus_protocol_unseal_pmrs sealing;
	int status;

	memset (sealing.pmr[0], 0, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[1], 1, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[2], 2, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[3], 3, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[4], 4, sizeof (sealing.pmr[0]));

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE;

	req->hmac_type = CERBERUS_PROTOCOL_UNSEAL_HMAC_SHA256;
	req->seed_type = CERBERUS_PROTOCOL_UNSEAL_SEED_RSA;
	req->seed_length = KEY_SEED_ENCRYPT_OAEP_LEN;
	memcpy (&req->seed, KEY_SEED_ENCRYPT_OAEP, KEY_SEED_ENCRYPT_OAEP_LEN);
	cerberus_protocol_unseal_ciphertext_length (req) = 0;
	cerberus_protocol_unseal_hmac_length (req) = PAYLOAD_HMAC_LEN;
	memcpy (cerberus_protocol_unseal_hmac (req), PAYLOAD_HMAC, PAYLOAD_HMAC_LEN);
	memcpy ((uint8_t*) cerberus_protocol_get_unseal_pmr_sealing (req), &sealing, sizeof (sealing));
	request.length = (sizeof (struct cerberus_protocol_message_unseal) - 1) +
		KEY_SEED_ENCRYPT_OAEP_LEN + 2 + 2 + PAYLOAD_HMAC_LEN + sizeof (sealing);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_incomplete_ciphertext (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal *req =
		(struct cerberus_protocol_message_unseal*) request.data;
	struct cerberus_protocol_unseal_pmrs sealing;
	int status;

	memset (sealing.pmr[0], 0, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[1], 1, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[2], 2, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[3], 3, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[4], 4, sizeof (sealing.pmr[0]));

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE;

	req->hmac_type = CERBERUS_PROTOCOL_UNSEAL_HMAC_SHA256;
	req->seed_type = CERBERUS_PROTOCOL_UNSEAL_SEED_RSA;
	req->seed_length = KEY_SEED_ENCRYPT_OAEP_LEN;
	memcpy (&req->seed, KEY_SEED_ENCRYPT_OAEP, KEY_SEED_ENCRYPT_OAEP_LEN);
	cerberus_protocol_unseal_ciphertext_length (req) = CIPHER_TEXT_LEN;
	memcpy (cerberus_protocol_unseal_ciphertext (req), CIPHER_TEXT, CIPHER_TEXT_LEN);
	cerberus_protocol_unseal_hmac_length (req) = PAYLOAD_HMAC_LEN;
	memcpy (cerberus_protocol_unseal_hmac (req), PAYLOAD_HMAC, PAYLOAD_HMAC_LEN);
	memcpy ((uint8_t*) cerberus_protocol_get_unseal_pmr_sealing (req), &sealing, sizeof (sealing));
	request.length = (sizeof (struct cerberus_protocol_message_unseal) - 1) +
		KEY_SEED_ENCRYPT_OAEP_LEN + 2 + CIPHER_TEXT_LEN - 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_no_hmac (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal *req =
		(struct cerberus_protocol_message_unseal*) request.data;
	struct cerberus_protocol_unseal_pmrs sealing;
	int status;

	memset (sealing.pmr[0], 0, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[1], 1, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[2], 2, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[3], 3, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[4], 4, sizeof (sealing.pmr[0]));

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE;

	req->hmac_type = CERBERUS_PROTOCOL_UNSEAL_HMAC_SHA256;
	req->seed_type = CERBERUS_PROTOCOL_UNSEAL_SEED_RSA;
	req->seed_length = KEY_SEED_ENCRYPT_OAEP_LEN;
	memcpy (&req->seed, KEY_SEED_ENCRYPT_OAEP, KEY_SEED_ENCRYPT_OAEP_LEN);
	cerberus_protocol_unseal_ciphertext_length (req) = CIPHER_TEXT_LEN;
	memcpy (cerberus_protocol_unseal_ciphertext (req), CIPHER_TEXT, CIPHER_TEXT_LEN);
	cerberus_protocol_unseal_hmac_length (req) = 0;
	memcpy ((uint8_t*) cerberus_protocol_get_unseal_pmr_sealing (req), &sealing, sizeof (sealing));
	request.length = (sizeof (struct cerberus_protocol_message_unseal) - 1) +
		KEY_SEED_ENCRYPT_OAEP_LEN + 2 + CIPHER_TEXT_LEN + 2 + sizeof (sealing);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_bad_hmac_length (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal *req =
		(struct cerberus_protocol_message_unseal*) request.data;
	struct cerberus_protocol_unseal_pmrs sealing;
	int status;

	memset (sealing.pmr[0], 0, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[1], 1, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[2], 2, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[3], 3, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[4], 4, sizeof (sealing.pmr[0]));

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE;

	req->hmac_type = CERBERUS_PROTOCOL_UNSEAL_HMAC_SHA256;
	req->seed_type = CERBERUS_PROTOCOL_UNSEAL_SEED_RSA;
	req->seed_length = KEY_SEED_ENCRYPT_OAEP_LEN;
	memcpy (&req->seed, KEY_SEED_ENCRYPT_OAEP, KEY_SEED_ENCRYPT_OAEP_LEN);
	cerberus_protocol_unseal_ciphertext_length (req) = CIPHER_TEXT_LEN;
	memcpy (cerberus_protocol_unseal_ciphertext (req), CIPHER_TEXT, CIPHER_TEXT_LEN);
	cerberus_protocol_unseal_hmac_length (req) = PAYLOAD_HMAC_LEN + 1;
	request.length = (sizeof (struct cerberus_protocol_message_unseal) - 1) +
		KEY_SEED_ENCRYPT_OAEP_LEN + 2 + CIPHER_TEXT_LEN + 2 + PAYLOAD_HMAC_LEN + 1 +
		sizeof (sealing);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);

	cerberus_protocol_unseal_hmac_length (req) = PAYLOAD_HMAC_LEN - 1;
	memcpy ((uint8_t*) cerberus_protocol_get_unseal_pmr_sealing (req), &sealing, sizeof (sealing));
	request.length = (sizeof (struct cerberus_protocol_message_unseal) - 1) +
		KEY_SEED_ENCRYPT_OAEP_LEN + 2 + CIPHER_TEXT_LEN + 2 + (PAYLOAD_HMAC_LEN - 1) +
		sizeof (sealing);
	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_incomplete_hmac (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal *req =
		(struct cerberus_protocol_message_unseal*) request.data;
	struct cerberus_protocol_unseal_pmrs sealing;
	int status;

	memset (sealing.pmr[0], 0, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[1], 1, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[2], 2, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[3], 3, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[4], 4, sizeof (sealing.pmr[0]));

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE;

	req->hmac_type = CERBERUS_PROTOCOL_UNSEAL_HMAC_SHA256;
	req->seed_type = CERBERUS_PROTOCOL_UNSEAL_SEED_RSA;
	req->seed_length = KEY_SEED_ENCRYPT_OAEP_LEN;
	memcpy (&req->seed, KEY_SEED_ENCRYPT_OAEP, KEY_SEED_ENCRYPT_OAEP_LEN);
	cerberus_protocol_unseal_ciphertext_length (req) = CIPHER_TEXT_LEN;
	memcpy (cerberus_protocol_unseal_ciphertext (req), CIPHER_TEXT, CIPHER_TEXT_LEN);
	cerberus_protocol_unseal_hmac_length (req) = PAYLOAD_HMAC_LEN;
	memcpy (cerberus_protocol_unseal_hmac (req), PAYLOAD_HMAC, PAYLOAD_HMAC_LEN);
	memcpy ((uint8_t*) cerberus_protocol_get_unseal_pmr_sealing (req), &sealing, sizeof (sealing));
	request.length = (sizeof (struct cerberus_protocol_message_unseal) - 1) +
		KEY_SEED_ENCRYPT_OAEP_LEN + 2 + CIPHER_TEXT_LEN + 2 + PAYLOAD_HMAC_LEN - 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_invalid_len (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal *req =
		(struct cerberus_protocol_message_unseal*) request.data;
	struct cerberus_protocol_unseal_pmrs sealing;
	int status;

	memset (sealing.pmr[0], 0, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[1], 1, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[2], 2, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[3], 3, sizeof (sealing.pmr[0]));
	memset (sealing.pmr[4], 4, sizeof (sealing.pmr[0]));

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE;

	req->hmac_type = CERBERUS_PROTOCOL_UNSEAL_HMAC_SHA256;
	req->seed_type = CERBERUS_PROTOCOL_UNSEAL_SEED_RSA;
	req->seed_length = KEY_SEED_ENCRYPT_OAEP_LEN;
	memcpy (&req->seed, KEY_SEED_ENCRYPT_OAEP, KEY_SEED_ENCRYPT_OAEP_LEN);
	cerberus_protocol_unseal_ciphertext_length (req) = CIPHER_TEXT_LEN;
	memcpy (cerberus_protocol_unseal_ciphertext (req), CIPHER_TEXT, CIPHER_TEXT_LEN);
	cerberus_protocol_unseal_hmac_length (req) = PAYLOAD_HMAC_LEN;
	memcpy (cerberus_protocol_unseal_hmac (req), PAYLOAD_HMAC, PAYLOAD_HMAC_LEN);
	memcpy ((uint8_t*) cerberus_protocol_get_unseal_pmr_sealing (req), &sealing, sizeof (sealing));
	request.length = sizeof (struct cerberus_protocol_message_unseal) - 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);

	request.length = (sizeof (struct cerberus_protocol_message_unseal) - 1) +
		KEY_SEED_ENCRYPT_OAEP_LEN + 2 + CIPHER_TEXT_LEN + 2 + PAYLOAD_HMAC_LEN + sizeof (sealing) -
		1;
	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);

	request.length = (sizeof (struct cerberus_protocol_message_unseal) - 1) +
		KEY_SEED_ENCRYPT_OAEP_LEN + 2 + CIPHER_TEXT_LEN + 2 + PAYLOAD_HMAC_LEN + sizeof (sealing) +
		1;
	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_result (CuTest *test,
	struct cmd_interface *cmd, struct cmd_background_mock *background)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal_result *req =
		(struct cerberus_protocol_message_unseal_result*) request.data;
	struct cerberus_protocol_message_unseal_result_completed_response *resp =
		(struct cerberus_protocol_message_unseal_result_completed_response*) request.data;
	size_t max_buf_len = MCTP_PROTOCOL_MAX_MESSAGE_BODY -
		sizeof (struct cerberus_protocol_message_unseal_result_completed_response) + 1;
	uint32_t attestation_status = 0;
	uint8_t key[] = {
		0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0xAA,0xBB,0xCC,0xDD,
		0xEE,0xFF,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0xEE,0xDD
	};
	uint16_t key_len = sizeof (key);
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE_RESULT;

	request.length = sizeof (struct cerberus_protocol_message_unseal_result);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&background->mock, background->base.unseal_result, background,
		0, MOCK_ARG_NOT_NULL, MOCK_ARG_PTR_CONTAINS_TMP (&max_buf_len, sizeof (max_buf_len)),
		MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&background->mock, 0, key, sizeof (key), -1);
	status |= mock_expect_output (&background->mock, 1, &key_len, sizeof (key_len), -1);
	status |= mock_expect_output (&background->mock, 2, &attestation_status,
		sizeof (attestation_status), -1);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test,
		sizeof (struct cerberus_protocol_message_unseal_result_completed_response) -
			sizeof (resp->key) + key_len,
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_UNSEAL_MESSAGE_RESULT, resp->header.command);
	CuAssertIntEquals (test, attestation_status, resp->unseal_status);
	CuAssertIntEquals (test, key_len, resp->key_length);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = testing_validate_array (key, &resp->key, sizeof (key));
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_result_limited_response (
	CuTest *test, struct cmd_interface *cmd, struct cmd_background_mock *background)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal_result *req =
		(struct cerberus_protocol_message_unseal_result*) request.data;
	struct cerberus_protocol_message_unseal_result_completed_response *resp =
		(struct cerberus_protocol_message_unseal_result_completed_response*) request.data;
	size_t max_buf_len = MCTP_PROTOCOL_MAX_MESSAGE_BODY - 128 -
		sizeof (struct cerberus_protocol_message_unseal_result_completed_response) + 1;
	uint32_t attestation_status = 0;
	uint8_t key[] = {
		0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0xAA,0xBB,0xCC,0xDD,
		0xEE,0xFF,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0xEE,0xDD
	};
	uint16_t key_len = sizeof (key);
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE_RESULT;

	request.length = sizeof (struct cerberus_protocol_message_unseal_result);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY - 128;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&background->mock, background->base.unseal_result, background,
		0, MOCK_ARG_NOT_NULL, MOCK_ARG_PTR_CONTAINS_TMP (&max_buf_len, sizeof (max_buf_len)),
		MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&background->mock, 0, key, sizeof (key), -1);
	status |= mock_expect_output (&background->mock, 1, &key_len, sizeof (key_len), -1);
	status |= mock_expect_output (&background->mock, 2, &attestation_status,
		sizeof (attestation_status), -1);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test,
		sizeof (struct cerberus_protocol_message_unseal_result_completed_response) -
			sizeof (resp->key) + key_len,
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_UNSEAL_MESSAGE_RESULT, resp->header.command);
	CuAssertIntEquals (test, attestation_status, resp->unseal_status);
	CuAssertIntEquals (test, key_len, resp->key_length);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = testing_validate_array (key, &resp->key, sizeof (key));
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_result_busy (CuTest *test,
	struct cmd_interface *cmd, struct cmd_background_mock *background)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal_result *req =
		(struct cerberus_protocol_message_unseal_result*) request.data;
	struct cerberus_protocol_message_unseal_result_response *resp =
		(struct cerberus_protocol_message_unseal_result_response*) request.data;
	size_t max_buf_len = MCTP_PROTOCOL_MAX_MESSAGE_BODY -
		sizeof (struct cerberus_protocol_message_unseal_result_completed_response) + 1;
	uint32_t attestation_status = ATTESTATION_CMD_STATUS_RUNNING;
	uint16_t key_len = 0;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE_RESULT;

	request.length = sizeof (struct cerberus_protocol_message_unseal_result);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&background->mock, background->base.unseal_result, background,
		0, MOCK_ARG_NOT_NULL, MOCK_ARG_PTR_CONTAINS_TMP (&max_buf_len, sizeof (max_buf_len)),
		MOCK_ARG_NOT_NULL);
	status |= mock_expect_output (&background->mock, 1, &key_len, sizeof (key_len), -1);
	status |= mock_expect_output (&background->mock, 2, &attestation_status,
		sizeof (attestation_status), -1);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_message_unseal_result_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_UNSEAL_MESSAGE_RESULT, resp->header.command);
	CuAssertIntEquals (test, attestation_status, resp->unseal_status);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_result_fail (CuTest *test,
	struct cmd_interface *cmd, struct cmd_background_mock *background)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal_result *req =
		(struct cerberus_protocol_message_unseal_result*) request.data;
	size_t max_buf_len = MCTP_PROTOCOL_MAX_MESSAGE_BODY -
		sizeof (struct cerberus_protocol_message_unseal_result_completed_response) + 1;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE_RESULT;

	request.length = sizeof (struct cerberus_protocol_message_unseal_result);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&background->mock, background->base.unseal_result, background,
		CMD_BACKGROUND_UNSEAL_RESULT_FAILED, MOCK_ARG_NOT_NULL,
		MOCK_ARG_PTR_CONTAINS_TMP (&max_buf_len, sizeof (max_buf_len)), MOCK_ARG_NOT_NULL);

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_BACKGROUND_UNSEAL_RESULT_FAILED, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_request_unseal_result_invalid_len (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_message_unseal_result *req =
		(struct cerberus_protocol_message_unseal_result*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UNSEAL_MESSAGE_RESULT;

	request.length = sizeof (struct cerberus_protocol_message_unseal_result) + 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_host_reset_status_port0_out_of_reset (
	CuTest *test, struct cmd_interface *cmd, struct host_control_mock *host_ctrl_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_host_state *req =
		(struct cerberus_protocol_get_host_state*) request.data;
	struct cerberus_protocol_get_host_state_response *resp =
		(struct cerberus_protocol_get_host_state_response*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_HOST_STATE;

	req->port_id = 0;
	request.length = sizeof (struct cerberus_protocol_get_host_state);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&host_ctrl_0->mock, host_ctrl_0->base.is_processor_in_reset, host_ctrl_0,
		0);
	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_host_state_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_HOST_STATE, resp->header.command);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_HOST_RUNNING, resp->reset_status);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_host_reset_status_port0_held_in_reset (
	CuTest *test, struct cmd_interface *cmd, struct host_control_mock *host_ctrl_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_host_state *req =
		(struct cerberus_protocol_get_host_state*) request.data;
	struct cerberus_protocol_get_host_state_response *resp =
		(struct cerberus_protocol_get_host_state_response*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_HOST_STATE;

	req->port_id = 0;
	request.length = sizeof (struct cerberus_protocol_get_host_state);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&host_ctrl_0->mock, host_ctrl_0->base.is_processor_in_reset, host_ctrl_0,
		1);
	status |= mock_expect (&host_ctrl_0->mock, host_ctrl_0->base.is_processor_held_in_reset,
		host_ctrl_0, 1);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_host_state_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_HOST_STATE, resp->header.command);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_HOST_HELD_IN_RESET, resp->reset_status);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_host_reset_status_port0_not_held_in_reset (
	CuTest *test, struct cmd_interface *cmd, struct host_control_mock *host_ctrl_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_host_state *req =
		(struct cerberus_protocol_get_host_state*) request.data;
	struct cerberus_protocol_get_host_state_response *resp =
		(struct cerberus_protocol_get_host_state_response*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_HOST_STATE;

	req->port_id = 0;
	request.length = sizeof (struct cerberus_protocol_get_host_state);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&host_ctrl_0->mock, host_ctrl_0->base.is_processor_in_reset, host_ctrl_0,
		1);
	status |= mock_expect (&host_ctrl_0->mock, host_ctrl_0->base.is_processor_held_in_reset,
		host_ctrl_0, 0);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_host_state_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_HOST_STATE, resp->header.command);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_HOST_IN_RESET, resp->reset_status);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_host_reset_status_port0_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_host_state *req =
		(struct cerberus_protocol_get_host_state*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_HOST_STATE;

	req->port_id = 0;
	request.length = sizeof (struct cerberus_protocol_get_host_state);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_host_reset_status_port1_out_of_reset (
	CuTest *test, struct cmd_interface *cmd, struct host_control_mock *host_ctrl_1)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_host_state *req =
		(struct cerberus_protocol_get_host_state*) request.data;
	struct cerberus_protocol_get_host_state_response *resp =
		(struct cerberus_protocol_get_host_state_response*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_HOST_STATE;

	req->port_id = 1;
	request.length = sizeof (struct cerberus_protocol_get_host_state);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&host_ctrl_1->mock, host_ctrl_1->base.is_processor_in_reset, host_ctrl_1,
		0);
	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_host_state_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_HOST_STATE, resp->header.command);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_HOST_RUNNING, resp->reset_status);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_host_reset_status_port1_held_in_reset (
	CuTest *test, struct cmd_interface *cmd, struct host_control_mock *host_ctrl_1)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_host_state *req =
		(struct cerberus_protocol_get_host_state*) request.data;
	struct cerberus_protocol_get_host_state_response *resp =
		(struct cerberus_protocol_get_host_state_response*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_HOST_STATE;

	req->port_id = 1;
	request.length = sizeof (struct cerberus_protocol_get_host_state);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&host_ctrl_1->mock, host_ctrl_1->base.is_processor_in_reset, host_ctrl_1,
		1);
	status |= mock_expect (&host_ctrl_1->mock, host_ctrl_1->base.is_processor_held_in_reset,
		host_ctrl_1, 1);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_host_state_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_HOST_STATE, resp->header.command);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_HOST_HELD_IN_RESET, resp->reset_status);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_host_reset_status_port1_not_held_in_reset (
	CuTest *test, struct cmd_interface *cmd, struct host_control_mock *host_ctrl_1)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_host_state *req =
		(struct cerberus_protocol_get_host_state*) request.data;
	struct cerberus_protocol_get_host_state_response *resp =
		(struct cerberus_protocol_get_host_state_response*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_HOST_STATE;

	req->port_id = 1;
	request.length = sizeof (struct cerberus_protocol_get_host_state);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&host_ctrl_1->mock, host_ctrl_1->base.is_processor_in_reset, host_ctrl_1,
		1);
	status |= mock_expect (&host_ctrl_1->mock, host_ctrl_1->base.is_processor_held_in_reset,
		host_ctrl_1, 0);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_get_host_state_response),
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_HOST_STATE, resp->header.command);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_HOST_IN_RESET, resp->reset_status);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_host_reset_status_port1_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_host_state *req =
		(struct cerberus_protocol_get_host_state*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_HOST_STATE;

	req->port_id = 1;
	request.length = sizeof (struct cerberus_protocol_get_host_state);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_host_reset_status_invalid_len (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_host_state *req =
		(struct cerberus_protocol_get_host_state*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_HOST_STATE;

	req->port_id = 0;
	request.length = sizeof (struct cerberus_protocol_get_host_state) + 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	request.length = sizeof (struct cerberus_protocol_get_host_state) - 1;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_host_reset_status_invalid_port (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_host_state *req =
		(struct cerberus_protocol_get_host_state*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_HOST_STATE;

	req->port_id = 2;
	request.length = sizeof (struct cerberus_protocol_get_host_state);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_OUT_OF_RANGE, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_host_reset_status_check_error (
	CuTest *test, struct cmd_interface *cmd, struct host_control_mock *host_ctrl_1)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_host_state *req =
		(struct cerberus_protocol_get_host_state*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_HOST_STATE;

	req->port_id = 1;
	request.length = sizeof (struct cerberus_protocol_get_host_state);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&host_ctrl_1->mock, host_ctrl_1->base.is_processor_in_reset, host_ctrl_1,
		HOST_CONTROL_RESET_CHECK_FAILED);
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, HOST_CONTROL_RESET_CHECK_FAILED, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_host_reset_status_hold_check_error (
	CuTest *test, struct cmd_interface *cmd, struct host_control_mock *host_ctrl_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_host_state *req =
		(struct cerberus_protocol_get_host_state*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_HOST_STATE;

	req->port_id = 0;
	request.length = sizeof (struct cerberus_protocol_get_host_state);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&host_ctrl_0->mock, host_ctrl_0->base.is_processor_in_reset, host_ctrl_0,
		1);
	status |= mock_expect (&host_ctrl_0->mock, host_ctrl_0->base.is_processor_held_in_reset,
		host_ctrl_0, HOST_CONTROL_HOLD_CHECK_FAILED);

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, HOST_CONTROL_HOLD_CHECK_FAILED, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_reset_bypass_no_nonce_authorized (
	CuTest *test, struct cmd_interface *cmd, struct cmd_authorization_mock *auth,
	struct cmd_background_mock *background)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	int status;
	uint8_t *null = NULL;
	size_t zero = 0;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 0;
	request.length = sizeof (struct cerberus_protocol_reset_config);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&auth->mock, auth->base.authorize_revert_bypass, auth, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (&null, sizeof (null)),
		MOCK_ARG_PTR_CONTAINS_TMP (&zero, sizeof (zero)));
	status |= mock_expect (&background->mock, background->base.reset_bypass, background, 0);

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_reset_bypass_no_nonce_challenge (
	CuTest *test, struct cmd_interface *cmd, struct cmd_authorization_mock *auth)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	struct cerberus_protocol_reset_config_response *resp =
		(struct cerberus_protocol_reset_config_response*) request.data;
	int status;
	int i;
	uint8_t *null = NULL;
	size_t zero = 0;
	uint8_t nonce[32];
	uint8_t *challenge = nonce;
	size_t length = sizeof (nonce);

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 0;
	request.length = sizeof (struct cerberus_protocol_reset_config);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	for (i = 0; i < sizeof (nonce); i++) {
		nonce[i] = i;
	}

	status = mock_expect (&auth->mock, auth->base.authorize_revert_bypass, auth,
		AUTHORIZATION_CHALLENGE, MOCK_ARG_PTR_CONTAINS_TMP (&null, sizeof (null)),
		MOCK_ARG_PTR_CONTAINS_TMP (&zero, sizeof (zero)));
	status |= mock_expect_output (&auth->mock, 0, &challenge, sizeof (challenge), -1);
	status |= mock_expect_output (&auth->mock, 1, &length, sizeof (length), -1);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_reset_config_response) + length,
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_RESET_CONFIG, resp->header.command);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, true, request.crypto_timeout);

	status = testing_validate_array (nonce, cerberus_protocol_reset_authorization (resp), length);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_reset_bypass_no_nonce_max_challenge (
	CuTest *test, struct cmd_interface *cmd, struct cmd_authorization_mock *auth)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	struct cerberus_protocol_reset_config_response *resp =
		(struct cerberus_protocol_reset_config_response*) request.data;
	int status;
	int i;
	uint8_t *null = NULL;
	size_t zero = 0;
	uint8_t nonce[CERBERUS_PROTOCOL_MAX_PAYLOAD_PER_MSG];
	uint8_t *challenge = nonce;
	size_t length = sizeof (nonce);

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 0;
	request.length = sizeof (struct cerberus_protocol_reset_config);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	for (i = 0; i < sizeof (nonce); i++) {
		nonce[i] = i;
	}

	status = mock_expect (&auth->mock, auth->base.authorize_revert_bypass, auth,
		AUTHORIZATION_CHALLENGE, MOCK_ARG_PTR_CONTAINS_TMP (&null, sizeof (null)),
		MOCK_ARG_PTR_CONTAINS_TMP (&zero, sizeof (zero)));
	status |= mock_expect_output (&auth->mock, 0, &challenge, sizeof (challenge), -1);
	status |= mock_expect_output (&auth->mock, 1, &length, sizeof (length), -1);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_reset_config_response) + length,
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_RESET_CONFIG, resp->header.command);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, true, request.crypto_timeout);

	status = testing_validate_array (nonce, cerberus_protocol_reset_authorization (resp), length);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_reset_bypass_no_nonce_not_authorized (
	CuTest *test, struct cmd_interface *cmd, struct cmd_authorization_mock *auth)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	int status;
	uint8_t *null = NULL;
	size_t zero = 0;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 0;
	request.length = sizeof (struct cerberus_protocol_reset_config);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&auth->mock, auth->base.authorize_revert_bypass, auth,
		AUTHORIZATION_NOT_AUTHORIZED, MOCK_ARG_PTR_CONTAINS_TMP (&null, sizeof (null)),
		MOCK_ARG_PTR_CONTAINS_TMP (&zero, sizeof (zero)));

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, AUTHORIZATION_NOT_AUTHORIZED, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_reset_bypass_with_nonce_authorized (
	CuTest *test, struct cmd_interface *cmd, struct cmd_authorization_mock *auth,
	struct cmd_background_mock *background)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	int status;
	int i;
	size_t length = 253;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 0;
	for (i = 0; i < length; i++) {
		cerberus_protocol_reset_authorization (req)[i] = i;
	}

	request.length = sizeof (struct cerberus_protocol_reset_config) + length;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&auth->mock, auth->base.authorize_revert_bypass, auth, 0,
		MOCK_ARG_PTR_PTR_CONTAINS_TMP (cerberus_protocol_reset_authorization (req), length),
		MOCK_ARG_PTR_CONTAINS_TMP (&length, sizeof (length)));
	status |= mock_expect (&background->mock, background->base.reset_bypass, background, 0);

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_reset_bypass_with_nonce_not_authorized (
	CuTest *test, struct cmd_interface *cmd, struct cmd_authorization_mock *auth)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	int status;
	int i;
	size_t length = 253;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 0;
	for (i = 0; i < length; i++) {
		cerberus_protocol_reset_authorization (req)[i] = i;
	}

	request.length = sizeof (struct cerberus_protocol_reset_config) + length;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&auth->mock, auth->base.authorize_revert_bypass, auth,
		AUTHORIZATION_NOT_AUTHORIZED,
		MOCK_ARG_PTR_PTR_CONTAINS_TMP (cerberus_protocol_reset_authorization (req), length),
		MOCK_ARG_PTR_CONTAINS_TMP (&length, sizeof (length)));

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, AUTHORIZATION_NOT_AUTHORIZED, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_reset_bypass_no_nonce_invalid_challenge (
	CuTest *test, struct cmd_interface *cmd, struct cmd_authorization_mock *auth)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	int status;
	int i;
	uint8_t *null = NULL;
	size_t zero = 0;
	uint8_t nonce[CERBERUS_PROTOCOL_MAX_PAYLOAD_PER_MSG + 1];
	uint8_t *challenge = nonce;
	size_t length = sizeof (nonce);

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 0;
	request.length = sizeof (struct cerberus_protocol_reset_config);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	for (i = 0; i < sizeof (nonce); i++) {
		nonce[i] = i;
	}

	status = mock_expect (&auth->mock, auth->base.authorize_revert_bypass, auth,
		AUTHORIZATION_CHALLENGE, MOCK_ARG_PTR_CONTAINS_TMP (&null, sizeof (null)),
		MOCK_ARG_PTR_CONTAINS_TMP (&zero, sizeof (zero)));
	status |= mock_expect_output (&auth->mock, 0, &challenge, sizeof (challenge), -1);
	status |= mock_expect_output (&auth->mock, 1, &length, sizeof (length), -1);

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BUF_TOO_SMALL, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_reset_bypass_no_nonce_invalid_challenge_limited_response (
	CuTest *test, struct cmd_interface *cmd, struct cmd_authorization_mock *auth)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	int status;
	int i;
	uint8_t *null = NULL;
	size_t zero = 0;
	uint8_t nonce[CERBERUS_PROTOCOL_MAX_PAYLOAD_PER_MSG + 1 - 128];
	uint8_t *challenge = nonce;
	size_t length = sizeof (nonce);

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 0;
	request.length = sizeof (struct cerberus_protocol_reset_config);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY - 128;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	for (i = 0; i < sizeof (nonce); i++) {
		nonce[i] = i;
	}

	status = mock_expect (&auth->mock, auth->base.authorize_revert_bypass, auth,
		AUTHORIZATION_CHALLENGE, MOCK_ARG_PTR_CONTAINS_TMP (&null, sizeof (null)),
		MOCK_ARG_PTR_CONTAINS_TMP (&zero, sizeof (zero)));
	status |= mock_expect_output (&auth->mock, 0, &challenge, sizeof (challenge), -1);
	status |= mock_expect_output (&auth->mock, 1, &length, sizeof (length), -1);

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BUF_TOO_SMALL, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_reset_bypass_error (
	CuTest *test, struct cmd_interface *cmd, struct cmd_authorization_mock *auth,
	struct cmd_background_mock *background)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	int status;
	uint8_t *null = NULL;
	size_t zero = 0;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 0;
	request.length = sizeof (struct cerberus_protocol_reset_config);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&auth->mock, auth->base.authorize_revert_bypass, auth, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (&null, sizeof (null)),
		MOCK_ARG_PTR_CONTAINS_TMP (&zero, sizeof (zero)));
	status |= mock_expect (&background->mock, background->base.reset_bypass, background,
		CMD_BACKGROUND_BYPASS_FAILED);

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_BACKGROUND_BYPASS_FAILED, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_restore_defaults_no_nonce_authorized (
	CuTest *test, struct cmd_interface *cmd, struct cmd_authorization_mock *auth,
	struct cmd_background_mock *background)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	int status;
	uint8_t *null = NULL;
	size_t zero = 0;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 1;
	request.length = sizeof (struct cerberus_protocol_reset_config);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&auth->mock, auth->base.authorize_reset_defaults, auth, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (&null, sizeof (null)),
		MOCK_ARG_PTR_CONTAINS_TMP (&zero, sizeof (zero)));
	status |= mock_expect (&background->mock, background->base.restore_defaults, background, 0);

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_restore_defaults_no_nonce_challenge (
	CuTest *test, struct cmd_interface *cmd, struct cmd_authorization_mock *auth)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	struct cerberus_protocol_reset_config_response *resp =
		(struct cerberus_protocol_reset_config_response*) request.data;
	int status;
	int i;
	uint8_t *null = NULL;
	size_t zero = 0;
	uint8_t nonce[32];
	uint8_t *challenge = nonce;
	size_t length = sizeof (nonce);

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 1;
	request.length = sizeof (struct cerberus_protocol_reset_config);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	for (i = 0; i < sizeof (nonce); i++) {
		nonce[i] = i;
	}

	status = mock_expect (&auth->mock, auth->base.authorize_reset_defaults, auth,
		AUTHORIZATION_CHALLENGE, MOCK_ARG_PTR_CONTAINS_TMP (&null, sizeof (null)),
		MOCK_ARG_PTR_CONTAINS_TMP (&zero, sizeof (zero)));
	status |= mock_expect_output (&auth->mock, 0, &challenge, sizeof (challenge), -1);
	status |= mock_expect_output (&auth->mock, 1, &length, sizeof (length), -1);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_reset_config_response) + length,
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_RESET_CONFIG, resp->header.command);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, true, request.crypto_timeout);

	status = testing_validate_array (nonce, cerberus_protocol_reset_authorization (resp), length);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_restore_defaults_no_nonce_max_challenge (
	CuTest *test, struct cmd_interface *cmd, struct cmd_authorization_mock *auth)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	struct cerberus_protocol_reset_config_response *resp =
		(struct cerberus_protocol_reset_config_response*) request.data;
	int status;
	int i;
	uint8_t *null = NULL;
	size_t zero = 0;
	uint8_t nonce[CERBERUS_PROTOCOL_MAX_PAYLOAD_PER_MSG];
	uint8_t *challenge = nonce;
	size_t length = sizeof (nonce);

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 1;
	request.length = sizeof (struct cerberus_protocol_reset_config);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	for (i = 0; i < sizeof (nonce); i++) {
		nonce[i] = i;
	}

	status = mock_expect (&auth->mock, auth->base.authorize_reset_defaults, auth,
		AUTHORIZATION_CHALLENGE, MOCK_ARG_PTR_CONTAINS_TMP (&null, sizeof (null)),
		MOCK_ARG_PTR_CONTAINS_TMP (&zero, sizeof (zero)));
	status |= mock_expect_output (&auth->mock, 0, &challenge, sizeof (challenge), -1);
	status |= mock_expect_output (&auth->mock, 1, &length, sizeof (length), -1);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, sizeof (struct cerberus_protocol_reset_config_response) + length,
		request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_RESET_CONFIG, resp->header.command);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, true, request.crypto_timeout);

	status = testing_validate_array (nonce, cerberus_protocol_reset_authorization (resp), length);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_restore_defaults_no_nonce_not_authorized (
	CuTest *test, struct cmd_interface *cmd, struct cmd_authorization_mock *auth)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	int status;
	uint8_t *null = NULL;
	size_t zero = 0;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 1;
	request.length = sizeof (struct cerberus_protocol_reset_config);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&auth->mock, auth->base.authorize_reset_defaults, auth,
		AUTHORIZATION_NOT_AUTHORIZED, MOCK_ARG_PTR_CONTAINS_TMP (&null, sizeof (null)),
		MOCK_ARG_PTR_CONTAINS_TMP (&zero, sizeof (zero)));

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, AUTHORIZATION_NOT_AUTHORIZED, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_restore_defaults_with_nonce_authorized (
	CuTest *test, struct cmd_interface *cmd, struct cmd_authorization_mock *auth,
	struct cmd_background_mock *background)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	int status;
	int i;
	size_t length = 253;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 1;
	for (i = 0; i < length; i++) {
		cerberus_protocol_reset_authorization (req)[i] = i;
	}

	request.length = sizeof (struct cerberus_protocol_reset_config) + length;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&auth->mock, auth->base.authorize_reset_defaults, auth, 0,
		MOCK_ARG_PTR_PTR_CONTAINS_TMP (cerberus_protocol_reset_authorization (req), length),
		MOCK_ARG_PTR_CONTAINS_TMP (&length, sizeof (length)));
	status |= mock_expect (&background->mock, background->base.restore_defaults, background, 0);

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_restore_defaults_with_nonce_not_authorized (
	CuTest *test, struct cmd_interface *cmd, struct cmd_authorization_mock *auth)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	int status;
	int i;
	size_t length = 253;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 1;
	for (i = 0; i < length; i++) {
		cerberus_protocol_reset_authorization (req)[i] = i;
	}

	request.length = sizeof (struct cerberus_protocol_reset_config) + length;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&auth->mock, auth->base.authorize_reset_defaults, auth,
		AUTHORIZATION_NOT_AUTHORIZED,
		MOCK_ARG_PTR_PTR_CONTAINS_TMP (cerberus_protocol_reset_authorization (req), length),
		MOCK_ARG_PTR_CONTAINS_TMP (&length, sizeof (length)));

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, AUTHORIZATION_NOT_AUTHORIZED, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_restore_defaults_no_nonce_invalid_challenge (
	CuTest *test, struct cmd_interface *cmd, struct cmd_authorization_mock *auth)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	int status;
	int i;
	uint8_t *null = NULL;
	size_t zero = 0;
	uint8_t nonce[CERBERUS_PROTOCOL_MAX_PAYLOAD_PER_MSG + 1];
	uint8_t *challenge = nonce;
	size_t length = sizeof (nonce);

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 1;
	request.length = sizeof (struct cerberus_protocol_reset_config);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	for (i = 0; i < sizeof (nonce); i++) {
		nonce[i] = i;
	}

	status = mock_expect (&auth->mock, auth->base.authorize_reset_defaults, auth,
		AUTHORIZATION_CHALLENGE, MOCK_ARG_PTR_CONTAINS_TMP (&null, sizeof (null)),
		MOCK_ARG_PTR_CONTAINS_TMP (&zero, sizeof (zero)));
	status |= mock_expect_output (&auth->mock, 0, &challenge, sizeof (challenge), -1);
	status |= mock_expect_output (&auth->mock, 1, &length, sizeof (length), -1);

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BUF_TOO_SMALL, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_restore_defaults_no_nonce_invalid_challenge_limited_response (
	CuTest *test, struct cmd_interface *cmd, struct cmd_authorization_mock *auth)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	int status;
	int i;
	uint8_t *null = NULL;
	size_t zero = 0;
	uint8_t nonce[CERBERUS_PROTOCOL_MAX_PAYLOAD_PER_MSG + 1 - 128];
	uint8_t *challenge = nonce;
	size_t length = sizeof (nonce);

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 1;
	request.length = sizeof (struct cerberus_protocol_reset_config);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY - 128;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	for (i = 0; i < sizeof (nonce); i++) {
		nonce[i] = i;
	}

	status = mock_expect (&auth->mock, auth->base.authorize_reset_defaults, auth,
		AUTHORIZATION_CHALLENGE, MOCK_ARG_PTR_CONTAINS_TMP (&null, sizeof (null)),
		MOCK_ARG_PTR_CONTAINS_TMP (&zero, sizeof (zero)));
	status |= mock_expect_output (&auth->mock, 0, &challenge, sizeof (challenge), -1);
	status |= mock_expect_output (&auth->mock, 1, &length, sizeof (length), -1);

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BUF_TOO_SMALL, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_restore_defaults_error (
	CuTest *test, struct cmd_interface *cmd, struct cmd_authorization_mock *auth,
	struct cmd_background_mock *background)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	int status;
	uint8_t *null = NULL;
	size_t zero = 0;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 1;
	request.length = sizeof (struct cerberus_protocol_reset_config);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&auth->mock, auth->base.authorize_reset_defaults, auth, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (&null, sizeof (null)),
		MOCK_ARG_PTR_CONTAINS_TMP (&zero, sizeof (zero)));
	status |= mock_expect (&background->mock, background->base.restore_defaults, background,
		CMD_BACKGROUND_DEFAULT_FAILED);

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_BACKGROUND_DEFAULT_FAILED, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_reset_config_invalid_len (CuTest *test,
	struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 0;
	request.length = sizeof (struct cerberus_protocol_reset_config) - 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_reset_config_invalid_request_subtype (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_reset_config *req =
		(struct cerberus_protocol_reset_config*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_RESET_CONFIG;

	req->type = 2;
	request.length = sizeof (struct cerberus_protocol_reset_config);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = false;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, true, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_prepare_recovery_image_port0 (CuTest *test,
	struct cmd_interface *cmd, struct recovery_image_cmd_interface_mock *recovery_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_prepare_recovery_image_update *req =
		(struct cerberus_protocol_prepare_recovery_image_update*) request.data;
	uint32_t length = 1;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_PREPARE_RECOVERY_IMAGE;

	req->port_id = 0;
	req->size = length;
	request.length = sizeof (struct cerberus_protocol_prepare_recovery_image_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&recovery_0->mock, recovery_0->base.prepare_recovery_image, recovery_0, 0,
		MOCK_ARG (length));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_prepare_recovery_image_port1 (CuTest *test,
	struct cmd_interface *cmd, struct recovery_image_cmd_interface_mock *recovery_1)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_prepare_recovery_image_update *req =
		(struct cerberus_protocol_prepare_recovery_image_update*) request.data;
	uint32_t length = 1;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_PREPARE_RECOVERY_IMAGE;

	req->port_id = 1;
	req->size = length;
	request.length = sizeof (struct cerberus_protocol_prepare_recovery_image_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&recovery_1->mock, recovery_1->base.prepare_recovery_image, recovery_1, 0,
		MOCK_ARG (length));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_prepare_recovery_image_port0_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_prepare_recovery_image_update *req =
		(struct cerberus_protocol_prepare_recovery_image_update*) request.data;
	uint32_t length = 1;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_PREPARE_RECOVERY_IMAGE;

	req->port_id = 0;
	req->size = length;
	request.length = sizeof (struct cerberus_protocol_prepare_recovery_image_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_prepare_recovery_image_port1_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_prepare_recovery_image_update *req =
		(struct cerberus_protocol_prepare_recovery_image_update*) request.data;
	uint32_t length = 1;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_PREPARE_RECOVERY_IMAGE;

	req->port_id = 1;
	req->size = length;
	request.length = sizeof (struct cerberus_protocol_prepare_recovery_image_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_prepare_recovery_image_invalid_len (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_prepare_recovery_image_update *req =
		(struct cerberus_protocol_prepare_recovery_image_update*) request.data;
	uint32_t length = 1;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_PREPARE_RECOVERY_IMAGE;

	req->port_id = 0;
	req->size = length;
	request.length = sizeof (struct cerberus_protocol_prepare_recovery_image_update) + 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	request.length = sizeof (struct cerberus_protocol_prepare_recovery_image_update) - 1;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_prepare_recovery_image_fail (CuTest *test,
	struct cmd_interface *cmd, struct recovery_image_cmd_interface_mock *recovery_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_prepare_recovery_image_update *req =
		(struct cerberus_protocol_prepare_recovery_image_update*) request.data;
	uint32_t length = 1;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_PREPARE_RECOVERY_IMAGE;

	req->port_id = 0;
	req->size = length;
	request.length = sizeof (struct cerberus_protocol_prepare_recovery_image_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&recovery_0->mock, recovery_0->base.prepare_recovery_image, recovery_0,
		RECOVERY_IMAGE_INVALID_ARGUMENT, MOCK_ARG (length));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, RECOVERY_IMAGE_INVALID_ARGUMENT, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_prepare_recovery_image_bad_port_index (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_prepare_recovery_image_update *req =
		(struct cerberus_protocol_prepare_recovery_image_update*) request.data;
	uint32_t length = 1;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_PREPARE_RECOVERY_IMAGE;

	req->port_id = 2;
	req->size = length;
	request.length = sizeof (struct cerberus_protocol_prepare_recovery_image_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_OUT_OF_RANGE, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_update_recovery_image_port0 (CuTest *test,
	struct cmd_interface *cmd, struct recovery_image_cmd_interface_mock *recovery_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_recovery_image_update *req =
		(struct cerberus_protocol_recovery_image_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UPDATE_RECOVERY_IMAGE;

	req->port_id = 0;
	req->payload = 0xAA;
	request.length = sizeof (struct cerberus_protocol_recovery_image_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&recovery_0->mock, recovery_0->base.update_recovery_image, recovery_0, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (&req->payload, 1), MOCK_ARG (1));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_update_recovery_image_port1 (CuTest *test,
	struct cmd_interface *cmd, struct recovery_image_cmd_interface_mock *recovery_1)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_recovery_image_update *req =
		(struct cerberus_protocol_recovery_image_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UPDATE_RECOVERY_IMAGE;

	req->port_id = 1;
	req->payload = 0xAA;
	request.length = sizeof (struct cerberus_protocol_recovery_image_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&recovery_1->mock, recovery_1->base.update_recovery_image, recovery_1, 0,
		MOCK_ARG_PTR_CONTAINS_TMP (&req->payload, 1), MOCK_ARG (1));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_update_recovery_image_port0_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_recovery_image_update *req =
		(struct cerberus_protocol_recovery_image_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UPDATE_RECOVERY_IMAGE;

	req->port_id = 0;
	req->payload = 0xAA;
	request.length = sizeof (struct cerberus_protocol_recovery_image_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_update_recovery_image_port1_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_recovery_image_update *req =
		(struct cerberus_protocol_recovery_image_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UPDATE_RECOVERY_IMAGE;

	req->port_id = 1;
	req->payload = 0xAA;
	request.length = sizeof (struct cerberus_protocol_recovery_image_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_update_recovery_image_no_data (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_recovery_image_update *req =
		(struct cerberus_protocol_recovery_image_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UPDATE_RECOVERY_IMAGE;

	req->port_id = 0;
	request.length =
		sizeof (struct cerberus_protocol_recovery_image_update) - sizeof (req->payload);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_update_recovery_image_bad_port_index (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_recovery_image_update *req =
		(struct cerberus_protocol_recovery_image_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UPDATE_RECOVERY_IMAGE;

	req->port_id = 2;
	req->payload = 0xAA;
	request.length = sizeof (struct cerberus_protocol_recovery_image_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_OUT_OF_RANGE, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_update_recovery_image_fail (CuTest *test,
	struct cmd_interface *cmd, struct recovery_image_cmd_interface_mock *recovery_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_recovery_image_update *req =
		(struct cerberus_protocol_recovery_image_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_UPDATE_RECOVERY_IMAGE;

	req->port_id = 0;
	req->payload = 0xAA;
	request.length = sizeof (struct cerberus_protocol_recovery_image_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;
	request.target_eid = MCTP_PROTOCOL_BMC_EID;

	status = mock_expect (&recovery_0->mock, recovery_0->base.update_recovery_image, recovery_0,
		RECOVERY_IMAGE_INVALID_ARGUMENT, MOCK_ARG_PTR_CONTAINS_TMP (&req->payload, 1), MOCK_ARG (1));
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, RECOVERY_IMAGE_INVALID_ARGUMENT, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_activate_recovery_image_port0 (
	CuTest *test, struct cmd_interface *cmd, struct recovery_image_cmd_interface_mock *recovery_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_recovery_image_update *req =
		(struct cerberus_protocol_complete_recovery_image_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_ACTIVATE_RECOVERY_IMAGE;

	req->port_id = 0;
	request.length = sizeof (struct cerberus_protocol_complete_recovery_image_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&recovery_0->mock, recovery_0->base.activate_recovery_image, recovery_0,
		0);
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_activate_recovery_image_port1 (
	CuTest *test, struct cmd_interface *cmd, struct recovery_image_cmd_interface_mock *recovery_1)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_recovery_image_update *req =
		(struct cerberus_protocol_complete_recovery_image_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_ACTIVATE_RECOVERY_IMAGE;

	req->port_id = 1;
	request.length = sizeof (struct cerberus_protocol_complete_recovery_image_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&recovery_1->mock, recovery_1->base.activate_recovery_image, recovery_1,
		0);
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test, 0, request.length);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_activate_recovery_image_port0_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_recovery_image_update *req =
		(struct cerberus_protocol_complete_recovery_image_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_ACTIVATE_RECOVERY_IMAGE;

	req->port_id = 0;
	request.length = sizeof (struct cerberus_protocol_complete_recovery_image_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_activate_recovery_image_port1_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_recovery_image_update *req =
		(struct cerberus_protocol_complete_recovery_image_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_ACTIVATE_RECOVERY_IMAGE;

	req->port_id = 1;
	request.length = sizeof (struct cerberus_protocol_complete_recovery_image_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_activate_recovery_image_invalid_len (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_recovery_image_update *req =
		(struct cerberus_protocol_complete_recovery_image_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_ACTIVATE_RECOVERY_IMAGE;

	req->port_id = 0;
	request.length = sizeof (struct cerberus_protocol_complete_recovery_image_update) + 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	request.length = sizeof (struct cerberus_protocol_complete_recovery_image_update) - 1;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_activate_recovery_image_bad_port_index (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_recovery_image_update *req =
		(struct cerberus_protocol_complete_recovery_image_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_ACTIVATE_RECOVERY_IMAGE;

	req->port_id = 2;
	request.length = sizeof (struct cerberus_protocol_complete_recovery_image_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_OUT_OF_RANGE, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_activate_recovery_image_fail (CuTest *test,
	struct cmd_interface *cmd, struct recovery_image_cmd_interface_mock *recovery_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_complete_recovery_image_update *req =
		(struct cerberus_protocol_complete_recovery_image_update*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_ACTIVATE_RECOVERY_IMAGE;

	req->port_id = 0;
	request.length = sizeof (struct cerberus_protocol_complete_recovery_image_update);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&recovery_0->mock, recovery_0->base.activate_recovery_image, recovery_0,
		RECOVERY_IMAGE_INVALID_ARGUMENT);
	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, RECOVERY_IMAGE_INVALID_ARGUMENT, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_recovery_image_version_port0 (
	CuTest *test, struct cmd_interface *cmd, struct recovery_image_manager_mock *recovery_manager_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_recovery_image_id *req =
		(struct cerberus_protocol_get_recovery_image_id*) request.data;
	struct cerberus_protocol_get_recovery_image_id_version_response *resp =
		(struct cerberus_protocol_get_recovery_image_id_version_response*) request.data;
	struct recovery_image_mock image;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_RECOVERY_IMAGE_VERSION;

	req->port_id = 0;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_recovery_image_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = recovery_image_mock_init (&image);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&recovery_manager_0->mock,
		recovery_manager_0->base.get_active_recovery_image, recovery_manager_0,
		(intptr_t) &image.base);

	status |= mock_expect (&image.mock, image.base.get_version, &image, 0, MOCK_ARG_NOT_NULL,
		MOCK_ARG (CERBERUS_PROTOCOL_FW_VERSION_LEN));
	status |= mock_expect_output (&image.mock, 0, RECOVERY_IMAGE_HEADER_VERSION_ID,
		RECOVERY_IMAGE_HEADER_VERSION_ID_LEN, 1);

	status |= mock_expect (&recovery_manager_0->mock, recovery_manager_0->base.free_recovery_image,
		recovery_manager_0, 0, MOCK_ARG (&image));

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test,
		sizeof (struct cerberus_protocol_get_recovery_image_id_version_response), request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_RECOVERY_IMAGE_VERSION, resp->header.command);
	CuAssertStrEquals (test, RECOVERY_IMAGE_HEADER_VERSION_ID, resp->version);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = recovery_image_mock_validate_and_release (&image);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_recovery_image_version_port1 (
	CuTest *test, struct cmd_interface *cmd, struct recovery_image_manager_mock *recovery_manager_1)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_recovery_image_id *req =
		(struct cerberus_protocol_get_recovery_image_id*) request.data;
	struct cerberus_protocol_get_recovery_image_id_version_response *resp =
		(struct cerberus_protocol_get_recovery_image_id_version_response*) request.data;
	struct recovery_image_mock image;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_RECOVERY_IMAGE_VERSION;

	req->port_id = 1;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_recovery_image_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = recovery_image_mock_init (&image);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&recovery_manager_1->mock,
		recovery_manager_1->base.get_active_recovery_image, recovery_manager_1,
		(intptr_t) &image.base);

	status |= mock_expect (&image.mock, image.base.get_version, &image, 0, MOCK_ARG_NOT_NULL,
		MOCK_ARG (CERBERUS_PROTOCOL_FW_VERSION_LEN));
	status |= mock_expect_output (&image.mock, 0, RECOVERY_IMAGE_HEADER_VERSION_ID,
		RECOVERY_IMAGE_HEADER_VERSION_ID_LEN, 1);

	status |= mock_expect (&recovery_manager_1->mock, recovery_manager_1->base.free_recovery_image,
		recovery_manager_1, 0, MOCK_ARG (&image));

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test,
		sizeof (struct cerberus_protocol_get_recovery_image_id_version_response), request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_RECOVERY_IMAGE_VERSION, resp->header.command);
	CuAssertStrEquals (test, RECOVERY_IMAGE_HEADER_VERSION_ID, resp->version);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = recovery_image_mock_validate_and_release (&image);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_recovery_image_version_no_id_type (
	CuTest *test, struct cmd_interface *cmd, struct recovery_image_manager_mock *recovery_manager_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_recovery_image_id *req =
		(struct cerberus_protocol_get_recovery_image_id*) request.data;
	struct cerberus_protocol_get_recovery_image_id_version_response *resp =
		(struct cerberus_protocol_get_recovery_image_id_version_response*) request.data;
	struct recovery_image_mock image;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_RECOVERY_IMAGE_VERSION;

	req->port_id = 0;
	request.length = sizeof (struct cerberus_protocol_get_recovery_image_id) - 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = recovery_image_mock_init (&image);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&recovery_manager_0->mock,
		recovery_manager_0->base.get_active_recovery_image, recovery_manager_0,
		(intptr_t) &image.base);

	status |= mock_expect (&image.mock, image.base.get_version, &image, 0, MOCK_ARG_NOT_NULL,
		MOCK_ARG (CERBERUS_PROTOCOL_FW_VERSION_LEN));
	status |= mock_expect_output (&image.mock, 0, RECOVERY_IMAGE_HEADER_VERSION_ID,
		RECOVERY_IMAGE_HEADER_VERSION_ID_LEN, 1);

	status |= mock_expect (&recovery_manager_0->mock, recovery_manager_0->base.free_recovery_image,
		recovery_manager_0, 0, MOCK_ARG (&image));

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test,
		sizeof (struct cerberus_protocol_get_recovery_image_id_version_response), request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_RECOVERY_IMAGE_VERSION, resp->header.command);
	CuAssertStrEquals (test, RECOVERY_IMAGE_HEADER_VERSION_ID, resp->version);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = recovery_image_mock_validate_and_release (&image);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_recovery_image_version_port0_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_recovery_image_id *req =
		(struct cerberus_protocol_get_recovery_image_id*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_RECOVERY_IMAGE_VERSION;

	req->port_id = 0;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_recovery_image_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_recovery_image_version_port1_null (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_recovery_image_id *req =
		(struct cerberus_protocol_get_recovery_image_id*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_RECOVERY_IMAGE_VERSION;

	req->port_id = 1;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_recovery_image_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_UNSUPPORTED_INDEX, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_recovery_image_version_no_image (
	CuTest *test, struct cmd_interface *cmd, struct recovery_image_manager_mock *recovery_manager_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_recovery_image_id *req =
		(struct cerberus_protocol_get_recovery_image_id*) request.data;
	struct cerberus_protocol_get_recovery_image_id_version_response *resp =
		(struct cerberus_protocol_get_recovery_image_id_version_response*) request.data;
	char empty_string[CERBERUS_PROTOCOL_FW_VERSION_LEN] = {0};
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_RECOVERY_IMAGE_VERSION;

	req->port_id = 0;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_recovery_image_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = mock_expect (&recovery_manager_0->mock,
		recovery_manager_0->base.get_active_recovery_image, recovery_manager_0,
		(intptr_t) NULL);

	CuAssertIntEquals (test, 0, status);

	request.new_request = true;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, 0, status);
	CuAssertIntEquals (test,
		sizeof (struct cerberus_protocol_get_recovery_image_id_version_response), request.length);
	CuAssertIntEquals (test, MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF, resp->header.msg_type);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_MSFT_PCI_VID, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0, resp->header.seq_num);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_RECOVERY_IMAGE_VERSION, resp->header.command);
	CuAssertStrEquals (test, empty_string, resp->version);
	CuAssertIntEquals (test, false, request.new_request);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_recovery_image_version_fail (
	CuTest *test, struct cmd_interface *cmd, struct recovery_image_manager_mock *recovery_manager_0)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_recovery_image_id *req =
		(struct cerberus_protocol_get_recovery_image_id*) request.data;
	struct recovery_image_mock image;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_RECOVERY_IMAGE_VERSION;

	req->port_id = 0;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_recovery_image_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	status = recovery_image_mock_init (&image);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&recovery_manager_0->mock,
		recovery_manager_0->base.get_active_recovery_image, recovery_manager_0,
		(intptr_t) &image.base);

	status |= mock_expect (&image.mock, image.base.get_version, &image,
		RECOVERY_IMAGE_HEADER_BAD_VERSION_ID, MOCK_ARG_NOT_NULL,
		MOCK_ARG (CERBERUS_PROTOCOL_FW_VERSION_LEN));

	status |= mock_expect (&recovery_manager_0->mock, recovery_manager_0->base.free_recovery_image,
		recovery_manager_0, 0, MOCK_ARG (&image));

	CuAssertIntEquals (test, 0, status);

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, RECOVERY_IMAGE_HEADER_BAD_VERSION_ID, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	status = recovery_image_mock_validate_and_release (&image);
	CuAssertIntEquals (test, 0, status);
}

void cerberus_protocol_optional_commands_testing_process_get_recovery_image_version_invalid_len (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_recovery_image_id *req =
		(struct cerberus_protocol_get_recovery_image_id*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_RECOVERY_IMAGE_VERSION;

	req->port_id = 0;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_recovery_image_id) + 1;
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);

	request.length = sizeof (struct cerberus_protocol_get_recovery_image_id) - 2;
	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_BAD_LENGTH, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}

void cerberus_protocol_optional_commands_testing_process_get_recovery_image_version_bad_port_index (
	CuTest *test, struct cmd_interface *cmd)
{
	struct cmd_interface_request request;
	struct cerberus_protocol_get_recovery_image_id *req =
		(struct cerberus_protocol_get_recovery_image_id*) request.data;
	int status;

	memset (&request, 0, sizeof (request));
	req->header.msg_type = MCTP_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	req->header.pci_vendor_id = CERBERUS_PROTOCOL_MSFT_PCI_VID;
	req->header.command = CERBERUS_PROTOCOL_GET_RECOVERY_IMAGE_VERSION;

	req->port_id = 2;
	req->id = 0;
	request.length = sizeof (struct cerberus_protocol_get_recovery_image_id);
	request.max_response = MCTP_PROTOCOL_MAX_MESSAGE_BODY;
	request.source_eid = MCTP_PROTOCOL_BMC_EID;
	request.target_eid = MCTP_PROTOCOL_PA_ROT_CTRL_EID;

	request.crypto_timeout = true;
	status = cmd->process_request (cmd, &request);
	CuAssertIntEquals (test, CMD_HANDLER_OUT_OF_RANGE, status);
	CuAssertIntEquals (test, false, request.crypto_timeout);
}


/*******************
 * Test cases
 *******************/

static void cerberus_protocol_optional_commands_test_prepare_pfm_update_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x5b,
		0x01,0x02,0x03,0x04,0x05
	};
	struct cerberus_protocol_prepare_pfm_update *req;

	TEST_START;

	CuAssertIntEquals (test, sizeof (raw_buffer_req),
		sizeof (struct cerberus_protocol_prepare_pfm_update));

	req = (struct cerberus_protocol_prepare_pfm_update*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_INIT_PFM_UPDATE, req->header.command);

	CuAssertIntEquals (test, 0x01, req->port_id);
	CuAssertIntEquals (test, 0x05040302, req->size);
}

static void cerberus_protocol_optional_commands_test_pfm_update_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x5c,
		0x01,
		0x02,0x03,0x04,0x05
	};
	struct cerberus_protocol_pfm_update *req;

	TEST_START;

	req = (struct cerberus_protocol_pfm_update*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_PFM_UPDATE, req->header.command);

	CuAssertIntEquals (test, 0x01, req->port_id);
	CuAssertPtrEquals (test, &raw_buffer_req[6], &req->payload);
}

static void cerberus_protocol_optional_commands_test_complete_pfm_update_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x5d,
		0x01,0x02
	};
	struct cerberus_protocol_complete_pfm_update *req;

	TEST_START;

	CuAssertIntEquals (test, sizeof (raw_buffer_req),
		sizeof (struct cerberus_protocol_complete_pfm_update));

	req = (struct cerberus_protocol_complete_pfm_update*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_COMPLETE_PFM_UPDATE, req->header.command);

	CuAssertIntEquals (test, 0x01, req->port_id);
	CuAssertIntEquals (test, 0x02, req->activation);
}

static void cerberus_protocol_optional_commands_test_get_pfm_id_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x59,
		0x01,0x02,0x03
	};
	uint8_t raw_buffer_resp_version[] = {
		0x7e,0x14,0x13,0x03,0x59,
		0x03,0x04,0x05,0x06,0x07
	};
	uint8_t raw_buffer_resp_platform[] = {
		0x7e,0x14,0x13,0x03,0x59,
		0x08,
		0x30,0x31,0x32,0x33,0x34,0x35,0x00
	};
	struct cerberus_protocol_get_pfm_id *req;
	struct cerberus_protocol_get_pfm_id_version_response *resp1;
	struct cerberus_protocol_get_pfm_id_platform_response *resp2;

	TEST_START;

	CuAssertIntEquals (test, sizeof (raw_buffer_req), sizeof (struct cerberus_protocol_get_pfm_id));
	CuAssertIntEquals (test, sizeof (raw_buffer_resp_version),
		sizeof (struct cerberus_protocol_get_pfm_id_version_response));

	req = (struct cerberus_protocol_get_pfm_id*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_ID, req->header.command);

	CuAssertIntEquals (test, 0x01, req->port_id);
	CuAssertIntEquals (test, 0x02, req->region);
	CuAssertIntEquals (test, 0x03, req->id);

	resp1 = (struct cerberus_protocol_get_pfm_id_version_response*) raw_buffer_resp_version;
	CuAssertIntEquals (test, 0, resp1->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, resp1->header.msg_type);
	CuAssertIntEquals (test, 0x1314, resp1->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp1->header.rq);
	CuAssertIntEquals (test, 0, resp1->header.d_bit);
	CuAssertIntEquals (test, 0, resp1->header.crypt);
	CuAssertIntEquals (test, 0x03, resp1->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_ID, resp1->header.command);

	CuAssertIntEquals (test, 0x03, resp1->valid);
	CuAssertIntEquals (test, 0x07060504, resp1->version);

	resp2 = (struct cerberus_protocol_get_pfm_id_platform_response*) raw_buffer_resp_platform;
	CuAssertIntEquals (test, 0, resp2->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, resp2->header.msg_type);
	CuAssertIntEquals (test, 0x1314, resp2->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp2->header.rq);
	CuAssertIntEquals (test, 0, resp2->header.d_bit);
	CuAssertIntEquals (test, 0, resp2->header.crypt);
	CuAssertIntEquals (test, 0x03, resp2->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_ID, resp2->header.command);

	CuAssertIntEquals (test, 0x08, resp2->valid);
	CuAssertStrEquals (test, "012345", (char*) &resp2->platform);
}

static void cerberus_protocol_optional_commands_test_get_pfm_supported_fw_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x5a,
		0x01,0x02,0x03,0x04,0x05,0x06
	};
	uint8_t raw_buffer_resp[] = {
		0x7e,0x14,0x13,0x03,0x5a,
		0x03,0x04,0x05,0x06,0x07,
		0x30,0x31,0x32,0x33,0x34,0x35,0x00
	};
	struct cerberus_protocol_get_pfm_supported_fw *req;
	struct cerberus_protocol_get_pfm_supported_fw_response *resp;

	TEST_START;

	CuAssertIntEquals (test, sizeof (raw_buffer_req),
		sizeof (struct cerberus_protocol_get_pfm_supported_fw));

	req = (struct cerberus_protocol_get_pfm_supported_fw*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW,
		req->header.command);

	CuAssertIntEquals (test, 0x01, req->port_id);
	CuAssertIntEquals (test, 0x02, req->region);
	CuAssertIntEquals (test, 0x06050403, req->offset);

	resp = (struct cerberus_protocol_get_pfm_supported_fw_response*) raw_buffer_resp;
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, resp->header.msg_type);
	CuAssertIntEquals (test, 0x1314, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0x03, resp->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW, resp->header.command);

	CuAssertIntEquals (test, 0x03, resp->valid);
	CuAssertIntEquals (test, 0x07060504, resp->version);
	CuAssertPtrEquals (test, &raw_buffer_resp[10], cerberus_protocol_pfm_supported_fw (resp));
}

static void cerberus_protocol_optional_commands_test_prepare_recovery_image_update_format (
	CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x72,
		0x01,0x02,0x03,0x04,0x05
	};
	struct cerberus_protocol_prepare_recovery_image_update *req;

	TEST_START;

	CuAssertIntEquals (test, sizeof (raw_buffer_req),
		sizeof (struct cerberus_protocol_prepare_recovery_image_update));

	req = (struct cerberus_protocol_prepare_recovery_image_update*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_PREPARE_RECOVERY_IMAGE, req->header.command);

	CuAssertIntEquals (test, 0x01, req->port_id);
	CuAssertIntEquals (test, 0x05040302, req->size);
}

static void cerberus_protocol_optional_commands_test_recovery_image_update_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x73,
		0x01,
		0x02,0x03,0x04,0x05
	};
	struct cerberus_protocol_recovery_image_update *req;

	TEST_START;

	req = (struct cerberus_protocol_recovery_image_update*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_UPDATE_RECOVERY_IMAGE, req->header.command);

	CuAssertIntEquals (test, 0x01, req->port_id);
	CuAssertPtrEquals (test, &raw_buffer_req[6], &req->payload);
}

static void cerberus_protocol_optional_commands_test_complete_recovery_image_update_format (
	CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x74,
		0x01
	};
	struct cerberus_protocol_complete_recovery_image_update *req;

	TEST_START;

	CuAssertIntEquals (test, sizeof (raw_buffer_req),
		sizeof (struct cerberus_protocol_complete_recovery_image_update));

	req = (struct cerberus_protocol_complete_recovery_image_update*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_ACTIVATE_RECOVERY_IMAGE, req->header.command);

	CuAssertIntEquals (test, 0x01, req->port_id);
}

static void cerberus_protocol_optional_commands_test_get_recovery_image_id_format (
	CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x75,
		0x01,02
	};
	uint8_t raw_buffer_resp_version[] = {
		0x7e,0x14,0x13,0x03,0x75,
		0x30,0x31,0x32,0x33,0x34,0x35,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};
	uint8_t raw_buffer_resp_platform[] = {
		0x7e,0x14,0x13,0x03,0x75,
		0x36,0x37,0x38,0x39,0x00
	};
	struct cerberus_protocol_get_recovery_image_id *req;
	struct cerberus_protocol_get_recovery_image_id_version_response *resp1;
	struct cerberus_protocol_get_recovery_image_id_platform_response *resp2;

	TEST_START;

	CuAssertIntEquals (test, sizeof (raw_buffer_req),
		sizeof (struct cerberus_protocol_get_recovery_image_id));
	CuAssertIntEquals (test, sizeof (raw_buffer_resp_version),
		sizeof (struct cerberus_protocol_get_recovery_image_id_version_response));

	req = (struct cerberus_protocol_get_recovery_image_id*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_RECOVERY_IMAGE_VERSION,
		req->header.command);

	CuAssertIntEquals (test, 0x01, req->port_id);

	resp1 =
		(struct cerberus_protocol_get_recovery_image_id_version_response*) raw_buffer_resp_version;
	CuAssertIntEquals (test, 0, resp1->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, resp1->header.msg_type);
	CuAssertIntEquals (test, 0x1314, resp1->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp1->header.rq);
	CuAssertIntEquals (test, 0, resp1->header.d_bit);
	CuAssertIntEquals (test, 0, resp1->header.crypt);
	CuAssertIntEquals (test, 0x03, resp1->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_RECOVERY_IMAGE_VERSION, resp1->header.command);

	CuAssertStrEquals (test, "012345", resp1->version);

	resp2 = (struct cerberus_protocol_get_recovery_image_id_platform_response*)
		raw_buffer_resp_platform;
	CuAssertIntEquals (test, 0, resp2->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, resp2->header.msg_type);
	CuAssertIntEquals (test, 0x1314, resp2->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp2->header.rq);
	CuAssertIntEquals (test, 0, resp2->header.d_bit);
	CuAssertIntEquals (test, 0, resp2->header.crypt);
	CuAssertIntEquals (test, 0x03, resp2->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_RECOVERY_IMAGE_VERSION, resp2->header.command);

	CuAssertStrEquals (test, "6789", (char*) &resp2->platform);
}

static void cerberus_protocol_optional_commands_test_get_host_state_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x40,
		0x01
	};
	uint8_t raw_buffer_resp[] = {
		0x7e,0x14,0x13,0x03,0x40,
		0x02
	};
	struct cerberus_protocol_get_host_state *req;
	struct cerberus_protocol_get_host_state_response *resp;

	TEST_START;

	CuAssertIntEquals (test, sizeof (raw_buffer_req),
		sizeof (struct cerberus_protocol_get_host_state));
	CuAssertIntEquals (test, sizeof (raw_buffer_resp),
		sizeof (struct cerberus_protocol_get_host_state_response));

	req = (struct cerberus_protocol_get_host_state*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_HOST_STATE,
		req->header.command);

	CuAssertIntEquals (test, 0x01, req->port_id);

	resp = (struct cerberus_protocol_get_host_state_response*) raw_buffer_resp;
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, resp->header.msg_type);
	CuAssertIntEquals (test, 0x1314, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0x03, resp->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_HOST_STATE, resp->header.command);

	CuAssertIntEquals (test, 0x02, resp->reset_status);
}

static void cerberus_protocol_optional_commands_test_pmr_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x80,
		0x01,
		0x88,0x69,0xde,0x57,0x9d,0xd0,0xe9,0x05,0xe0,0xa7,0x11,0x24,0x57,0x55,0x94,0xf5,
		0x0a,0x03,0xd3,0xd9,0xcd,0xf1,0x6e,0x9a,0x3f,0x9d,0x6c,0x60,0xc0,0x32,0x4b,0x54,
	};
	uint8_t raw_buffer_resp[] = {
		0x7e,0x14,0x13,0x03,0x80,
		0xf1,0x3b,0x43,0x16,0x2c,0xe4,0x05,0x75,0x73,0xc5,0x54,0x10,0xad,0xd5,0xc5,0xc6,
		0x0e,0x9a,0x37,0xff,0x3e,0xa0,0x02,0x34,0xd6,0x41,0x80,0xfa,0x1a,0x0e,0x0a,0x04,
		0x20,
		0x88,0x69,0xde,0x57,0x9d,0xd0,0xe9,0x05,0xe0,0xa7,0x11,0x24,0x57,0x55,0x94,0xf5,
		0x0a,0x03,0xd3,0xd9,0xcd,0xf1,0x6e,0x9a,0x3f,0x9d,0x6c,0x60,0xc0,0x32,0x4b,0x54,
		0x30,0x46,0x02,0x21,0x00,0x86,0x1d,0x0e,0x39,0x20,0xdc,0xae,0x77,0xcc,0xb0,0x33,
		0x38,0xb7,0xd8,0x47,0xb9,0x7a,0x6b,0x65,0x3b,0xe2,0x72,0x52,0x8f,0x77,0x82,0x00,
		0x82,0x8f,0x6f,0xc5,0x9e,0x02,0x21,0x00,0xf8,0xf9,0x96,0xaf,0xd5,0xc5,0x50,0x16,
		0xa9,0x31,0x2d,0xad,0x1e,0xec,0x61,0x3a,0x80,0xe5,0x7a,0x1f,0xa0,0xc3,0x0c,0x35,
		0x41,0x00,0x96,0xcf,0x71,0x24,0x08,0x43
	};
	struct cerberus_protocol_pmr *req;
	struct cerberus_protocol_pmr_response *resp;

	TEST_START;

	CuAssertIntEquals (test, sizeof (raw_buffer_req), sizeof (struct cerberus_protocol_pmr));

	req = (struct cerberus_protocol_pmr*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PMR, req->header.command);

	CuAssertIntEquals (test, 0x01, req->measurement_number);
	CuAssertPtrEquals (test, &raw_buffer_req[6], req->nonce);

	resp = (struct cerberus_protocol_pmr_response*) raw_buffer_resp;
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, resp->header.msg_type);
	CuAssertIntEquals (test, 0x1314, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0x03, resp->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_PMR, resp->header.command);

	CuAssertPtrEquals (test, &raw_buffer_resp[5], resp->nonce);
	CuAssertIntEquals (test, 0x20, resp->pmr_length);
	CuAssertPtrEquals (test, &raw_buffer_resp[38], &resp->measurement);
	CuAssertPtrEquals (test, &raw_buffer_resp[70], cerberus_protocol_pmr_get_signature (resp));
}

static void cerberus_protocol_optional_commands_test_update_pmr_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x86,
		0x01,
		0x88,0x69,0xde,0x57,0x9d,0xd0,0xe9,0x05,0xe0,0xa7,0x11,0x24,0x57,0x55,0x94,0xf5,
		0x0a,0x03,0xd3,0xd9,0xcd,0xf1,0x6e,0x9a,0x3f,0x9d,0x6c,0x60,0xc0,0x32,0x4b,0x54,
	};
	struct cerberus_protocol_update_pmr *req;

	TEST_START;

	req = (struct cerberus_protocol_update_pmr*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_UPDATE_PMR, req->header.command);

	CuAssertIntEquals (test, 0x01, req->measurement_number);
	CuAssertPtrEquals (test, &raw_buffer_req[6], &req->measurement_ext);
}

static void cerberus_protocol_optional_commands_test_key_exchange_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x84,
		0x01,
		0x88,0x69,0xde,0x57,0x9d,0xd0,0xe9,0x05,0xe0,0xa7,0x11,0x24,0x57,0x55,0x94,0xf5,
		0x0a,0x03,0xd3,0xd9,0xcd,0xf1,0x6e,0x9a,0x3f,0x9d,0x6c,0x60,0xc0,0x32,0x4b,0x54,
	};
	struct cerberus_protocol_key_exchange *req;

	TEST_START;

	req = (struct cerberus_protocol_key_exchange*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_EXCHANGE_KEYS, req->header.command);

	CuAssertIntEquals (test, 0x01, req->key_type);
	CuAssertPtrEquals (test, &raw_buffer_req[6], &req->key);
}

static void cerberus_protocol_optional_commands_test_get_log_info_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x4f,
	};
	uint8_t raw_buffer_resp[] = {
		0x7e,0x14,0x13,0x03,0x4f,
		0x01,0x02,0x03,0x04,
		0x05,0x06,0x07,0x08,
		0x09,0x0a,0x0b,0x0c
	};
	struct cerberus_protocol_get_log_info *req;
	struct cerberus_protocol_get_log_info_response *resp;

	TEST_START;

	CuAssertIntEquals (test, sizeof (raw_buffer_req),
		sizeof (struct cerberus_protocol_get_log_info));
	CuAssertIntEquals (test, sizeof (raw_buffer_resp),
		sizeof (struct cerberus_protocol_get_log_info_response));

	req = (struct cerberus_protocol_get_log_info*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_LOG_INFO, req->header.command);

	resp = (struct cerberus_protocol_get_log_info_response*) raw_buffer_resp;
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, resp->header.msg_type);
	CuAssertIntEquals (test, 0x1314, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0x03, resp->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_LOG_INFO, resp->header.command);

	CuAssertIntEquals (test, 0x04030201, resp->debug_log_length);
	CuAssertIntEquals (test, 0x08070605, resp->attestation_log_length);
	CuAssertIntEquals (test, 0x0c0b0a09, resp->tamper_log_length);
}

static void cerberus_protocol_optional_commands_test_get_log_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x50,
		0x01,0x02,0x03,0x04,0x05
	};
	uint8_t raw_buffer_resp[] = {
		0x7e,0x14,0x13,0x03,0x50,
		0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c
	};
	struct cerberus_protocol_get_log *req;
	struct cerberus_protocol_get_log_response *resp;

	TEST_START;

	CuAssertIntEquals (test, sizeof (raw_buffer_req), sizeof (struct cerberus_protocol_get_log));

	req = (struct cerberus_protocol_get_log*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_READ_LOG, req->header.command);

	CuAssertIntEquals (test, 0x01, req->log_type);
	CuAssertIntEquals (test, 0x05040302, req->offset);

	resp = (struct cerberus_protocol_get_log_response*) raw_buffer_resp;
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, resp->header.msg_type);
	CuAssertIntEquals (test, 0x1314, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0x03, resp->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_READ_LOG, resp->header.command);

	CuAssertPtrEquals (test, &raw_buffer_resp[5], cerberus_protocol_log_data (resp));
}

static void cerberus_protocol_optional_commands_test_clear_log_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x51,
		0x01
	};
	struct cerberus_protocol_clear_log *req;

	TEST_START;

	CuAssertIntEquals (test, sizeof (raw_buffer_req), sizeof (struct cerberus_protocol_clear_log));

	req = (struct cerberus_protocol_clear_log*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_CLEAR_LOG, req->header.command);

	CuAssertIntEquals (test, 0x01, req->log_type);
}

static void cerberus_protocol_optional_commands_test_get_attestation_data_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x52,
		0x01,0x02,0x03,0x04,0x05,0x06
	};
	uint8_t raw_buffer_resp[] = {
		0x7e,0x14,0x13,0x03,0x52,
		0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c
	};
	struct cerberus_protocol_get_attestation_data *req;
	struct cerberus_protocol_get_attestation_data_response *resp;

	TEST_START;

	CuAssertIntEquals (test, sizeof (raw_buffer_req),
		sizeof (struct cerberus_protocol_get_attestation_data));

	req = (struct cerberus_protocol_get_attestation_data*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_ATTESTATION_DATA, req->header.command);

	CuAssertIntEquals (test, 0x01, req->pmr);
	CuAssertIntEquals (test, 0x02, req->entry);
	CuAssertIntEquals (test, 0x06050403, req->offset);

	resp = (struct cerberus_protocol_get_attestation_data_response*) raw_buffer_resp;
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, resp->header.msg_type);
	CuAssertIntEquals (test, 0x1314, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0x03, resp->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_GET_ATTESTATION_DATA, resp->header.command);

	CuAssertPtrEquals (test, &raw_buffer_resp[5], cerberus_protocol_attestation_data (resp));
}

static void cerberus_protocol_optional_commands_test_prepare_fw_update_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x66,
		0x01,0x02,0x03,0x04
	};
	struct cerberus_protocol_prepare_fw_update *req;

	TEST_START;

	CuAssertIntEquals (test, sizeof (raw_buffer_req),
		sizeof (struct cerberus_protocol_prepare_fw_update));

	req = (struct cerberus_protocol_prepare_fw_update*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_INIT_FW_UPDATE, req->header.command);

	CuAssertIntEquals (test, 0x04030201, req->total_size);
}

static void cerberus_protocol_optional_commands_test_fw_update_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x67,
		0x02,0x03,0x04,0x05
	};
	struct cerberus_protocol_fw_update *req;

	TEST_START;

	req = (struct cerberus_protocol_fw_update*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_FW_UPDATE, req->header.command);

	CuAssertPtrEquals (test, &raw_buffer_req[5], &req->payload);
}

static void cerberus_protocol_optional_commands_test_complete_fw_update_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x69,
	};
	struct cerberus_protocol_complete_fw_update *req;

	TEST_START;

	CuAssertIntEquals (test, sizeof (raw_buffer_req),
		sizeof (struct cerberus_protocol_complete_fw_update));

	req = (struct cerberus_protocol_complete_fw_update*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_COMPLETE_FW_UPDATE, req->header.command);
}

static void cerberus_protocol_optional_commands_test_reset_config_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x6a,
		0x01,
		0xf1,0x3b,0x43,0x16,0x2c,0xe4,0x05,0x75,0x73,0xc5,0x54,0x10,0xad,0xd5,0xc5,0xc6,
		0x0e,0x9a,0x37,0xff,0x3e,0xa0,0x02,0x34,0xd6,0x41,0x80,0xfa,0x1a,0x0e,0x0a,0x04,
		0x30,0x46,0x02,0x21,0x00,0x86,0x1d,0x0e,0x39,0x20,0xdc,0xae,0x77,0xcc,0xb0,0x33,
		0x38,0xb7,0xd8,0x47,0xb9,0x7a,0x6b,0x65,0x3b,0xe2,0x72,0x52,0x8f,0x77,0x82,0x00,
		0x82,0x8f,0x6f,0xc5,0x9e,0x02,0x21,0x00,0xf8,0xf9,0x96,0xaf,0xd5,0xc5,0x50,0x16,
		0xa9,0x31,0x2d,0xad,0x1e,0xec,0x61,0x3a,0x80,0xe5,0x7a,0x1f,0xa0,0xc3,0x0c,0x35,
		0x41,0x00,0x96,0xcf,0x71,0x24,0x08,0x43
	};
	uint8_t raw_buffer_resp[] = {
		0x7e,0x14,0x13,0x03,0x6a,
		0xf1,0x3b,0x43,0x16,0x2c,0xe4,0x05,0x75,0x73,0xc5,0x54,0x10,0xad,0xd5,0xc5,0xc6,
		0x0e,0x9a,0x37,0xff,0x3e,0xa0,0x02,0x34,0xd6,0x41,0x80,0xfa,0x1a,0x0e,0x0a,0x04,
	};
	struct cerberus_protocol_reset_config *req;
	struct cerberus_protocol_reset_config_response *resp;

	TEST_START;

	req = (struct cerberus_protocol_reset_config*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_RESET_CONFIG, req->header.command);

	CuAssertIntEquals (test, 0x01, req->type);
	CuAssertPtrEquals (test, &raw_buffer_req[6], cerberus_protocol_reset_authorization (req));

	resp = (struct cerberus_protocol_reset_config_response*) raw_buffer_resp;
	CuAssertIntEquals (test, 0, resp->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, resp->header.msg_type);
	CuAssertIntEquals (test, 0x1314, resp->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp->header.rq);
	CuAssertIntEquals (test, 0, resp->header.d_bit);
	CuAssertIntEquals (test, 0, resp->header.crypt);
	CuAssertIntEquals (test, 0x03, resp->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_RESET_CONFIG, resp->header.command);

	CuAssertPtrEquals (test, &raw_buffer_resp[5], cerberus_protocol_reset_authorization (resp));
}

static void cerberus_protocol_optional_commands_test_recover_firmware_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x71,
		0x01,0x02
	};
	struct cerberus_protocol_recover_firmware *req;

	TEST_START;

	CuAssertIntEquals (test, sizeof (raw_buffer_req),
		sizeof (struct cerberus_protocol_recover_firmware));

	req = (struct cerberus_protocol_recover_firmware*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_TRIGGER_FW_RECOVERY, req->header.command);

	CuAssertIntEquals (test, 0x01, req->port_id);
	CuAssertIntEquals (test, 0x02, req->recovery_img);
}

static void cerberus_protocol_optional_commands_test_message_unseal_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x89,
		0x01,0x02,
		0x48,0x00,
		0x30,0x46,0x02,0x21,0x00,0x86,0x1d,0x0e,0x39,0x20,0xdc,0xae,0x77,0xcc,0xb0,0x33,
		0x38,0xb7,0xd8,0x47,0xb9,0x7a,0x6b,0x65,0x3b,0xe2,0x72,0x52,0x8f,0x77,0x82,0x00,
		0x82,0x8f,0x6f,0xc5,0x9e,0x02,0x21,0x00,0xf8,0xf9,0x96,0xaf,0xd5,0xc5,0x50,0x16,
		0xa9,0x31,0x2d,0xad,0x1e,0xec,0x61,0x3a,0x80,0xe5,0x7a,0x1f,0xa0,0xc3,0x0c,0x35,
		0x41,0x00,0x96,0xcf,0x71,0x24,0x08,0x43,
		0x10,0x00,
		0x88,0x69,0xde,0x57,0x9d,0xd0,0xe9,0x05,0xe0,0xa7,0x11,0x24,0x57,0x55,0x94,0xf5,
		0x20,0x00,
		0xf1,0x3b,0x43,0x16,0x2c,0xe4,0x05,0x75,0x73,0xc5,0x54,0x10,0xad,0xd5,0xc5,0xc6,
		0x0e,0x9a,0x37,0xff,0x3e,0xa0,0x02,0x34,0xd6,0x41,0x80,0xfa,0x1a,0x0e,0x0a,0x04,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x53,0x2e,0xaa,0xbd,0x95,0x74,0x88,0x0d,0xbf,0x76,0xb9,0xb8,0xcc,0x00,0x83,0x2c,
		0x20,0xa6,0xec,0x11,0x3d,0x68,0x22,0x99,0x55,0x0d,0x7a,0x6e,0x0f,0x34,0x5e,0x25,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x53,0x2e,0xaa,0xbd,0x95,0x74,0x88,0x0d,0xbf,0x76,0xb9,0xb8,0xcc,0x00,0x83,0x2c,
		0x20,0xa6,0xec,0x11,0x3d,0x68,0x22,0x99,0x55,0x0d,0x7a,0x6e,0x0f,0x34,0x5e,0x25,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x53,0x2e,0xaa,0xbd,0x95,0x74,0x88,0x0d,0xbf,0x76,0xb9,0xb8,0xcc,0x00,0x83,0x2c,
		0x20,0xa6,0xec,0x11,0x3d,0x68,0x22,0x99,0x55,0x0d,0x7a,0x6e,0x0f,0x34,0x5e,0x25,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x53,0x2e,0xaa,0xbd,0x95,0x74,0x88,0x0d,0xbf,0x76,0xb9,0xb8,0xcc,0x00,0x83,0x2c,
		0x20,0xa6,0xec,0x11,0x3d,0x68,0x22,0x99,0x55,0x0d,0x7a,0x6e,0x0f,0x34,0x5e,0x25,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	};
	struct cerberus_protocol_message_unseal *req;
	const struct cerberus_protocol_unseal_pmrs *pmrs;

	TEST_START;

	req = (struct cerberus_protocol_message_unseal*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_UNSEAL_MESSAGE, req->header.command);

	CuAssertIntEquals (test, 0x00, req->reserved);
	CuAssertIntEquals (test, 0x00, req->hmac_type);
	CuAssertIntEquals (test, 0x01, req->seed_type);
	CuAssertIntEquals (test, 0x00, req->seed_params.rsa.reserved);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_UNSEAL_RSA_OAEP_SHA256,
		req->seed_params.rsa.padding);
	CuAssertIntEquals (test, 0x02, req->seed_params.ecdh.reserved);
	CuAssertIntEquals (test, 0x0048, req->seed_length);
	CuAssertPtrEquals (test, &raw_buffer_req[9], &req->seed);
	CuAssertIntEquals (test, 0x0010, cerberus_protocol_unseal_ciphertext_length (req));
	CuAssertPtrEquals (test, &raw_buffer_req[83], cerberus_protocol_unseal_ciphertext (req));
	CuAssertIntEquals (test, 0x0020, cerberus_protocol_unseal_hmac_length (req));
	CuAssertPtrEquals (test, &raw_buffer_req[101], cerberus_protocol_unseal_hmac (req));

	pmrs = cerberus_protocol_get_unseal_pmr_sealing (req);
	CuAssertPtrEquals (test, &raw_buffer_req[133], (uint8_t*) pmrs);
	CuAssertPtrEquals (test, &raw_buffer_req[133], (uint8_t*) pmrs->pmr[0]);
	CuAssertPtrEquals (test, &raw_buffer_req[197], (uint8_t*) pmrs->pmr[1]);
	CuAssertPtrEquals (test, &raw_buffer_req[261], (uint8_t*) pmrs->pmr[2]);
	CuAssertPtrEquals (test, &raw_buffer_req[325], (uint8_t*) pmrs->pmr[3]);
	CuAssertPtrEquals (test, &raw_buffer_req[389], (uint8_t*) pmrs->pmr[4]);

	raw_buffer_req[5] = 0x21;
	CuAssertIntEquals (test, 0x01, req->reserved);
	CuAssertIntEquals (test, 0x00, req->hmac_type);
	CuAssertIntEquals (test, 0x01, req->seed_type);

	raw_buffer_req[5] = 0x29;
	CuAssertIntEquals (test, 0x01, req->reserved);
	CuAssertIntEquals (test, 0x02, req->hmac_type);
	CuAssertIntEquals (test, 0x01, req->seed_type);

	raw_buffer_req[6] = 0x01;
	CuAssertIntEquals (test, 0x00, req->seed_params.rsa.reserved);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_UNSEAL_RSA_OAEP_SHA1,
		req->seed_params.rsa.padding);
	CuAssertIntEquals (test, 0x01, req->seed_params.ecdh.reserved);

	raw_buffer_req[6] = 0x11;
	CuAssertIntEquals (test, 0x02, req->seed_params.rsa.reserved);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_UNSEAL_RSA_OAEP_SHA1,
		req->seed_params.rsa.padding);
	CuAssertIntEquals (test, 0x11, req->seed_params.ecdh.reserved);

	raw_buffer_req[6] = 0x10;
	CuAssertIntEquals (test, 0x02, req->seed_params.rsa.reserved);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_UNSEAL_RSA_PKCS15,
		req->seed_params.rsa.padding);
	CuAssertIntEquals (test, 0x10, req->seed_params.ecdh.reserved);
}

static void cerberus_protocol_optional_commands_test_message_unseal_result_format (CuTest *test)
{
	uint8_t raw_buffer_req[] = {
		0x7e,0x14,0x13,0x03,0x8a
	};
	uint8_t raw_buffer_resp[] = {
		0x7e,0x14,0x13,0x03,0x8a,
		0x03,0x04,0x05,0x06,
		0x07,0x00,
		0x30,0x31,0x32,0x33,0x34,0x35,0x00
	};
	struct cerberus_protocol_message_unseal_result *req;
	struct cerberus_protocol_message_unseal_result_response *resp1;
	struct cerberus_protocol_message_unseal_result_completed_response *resp2;

	TEST_START;

	CuAssertIntEquals (test, sizeof (raw_buffer_req),
		sizeof (struct cerberus_protocol_message_unseal_result));
	CuAssertIntEquals (test, 9, sizeof (struct cerberus_protocol_message_unseal_result_response));

	req = (struct cerberus_protocol_message_unseal_result*) raw_buffer_req;
	CuAssertIntEquals (test, 0, req->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, req->header.msg_type);
	CuAssertIntEquals (test, 0x1314, req->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, req->header.rq);
	CuAssertIntEquals (test, 0, req->header.d_bit);
	CuAssertIntEquals (test, 0, req->header.crypt);
	CuAssertIntEquals (test, 0x03, req->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_UNSEAL_MESSAGE_RESULT,
		req->header.command);

	resp1 = (struct cerberus_protocol_message_unseal_result_response*) raw_buffer_resp;
	CuAssertIntEquals (test, 0, resp1->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, resp1->header.msg_type);
	CuAssertIntEquals (test, 0x1314, resp1->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp1->header.rq);
	CuAssertIntEquals (test, 0, resp1->header.d_bit);
	CuAssertIntEquals (test, 0, resp1->header.crypt);
	CuAssertIntEquals (test, 0x03, resp1->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_UNSEAL_MESSAGE_RESULT, resp1->header.command);

	CuAssertIntEquals (test, 0x06050403, resp1->unseal_status);

	resp2 = (struct cerberus_protocol_message_unseal_result_completed_response*) raw_buffer_resp;
	CuAssertIntEquals (test, 0, resp2->header.integrity_check);
	CuAssertIntEquals (test, 0x7e, resp2->header.msg_type);
	CuAssertIntEquals (test, 0x1314, resp2->header.pci_vendor_id);
	CuAssertIntEquals (test, 0, resp2->header.rq);
	CuAssertIntEquals (test, 0, resp2->header.d_bit);
	CuAssertIntEquals (test, 0, resp2->header.crypt);
	CuAssertIntEquals (test, 0x03, resp2->header.seq_num);
	CuAssertIntEquals (test, CERBERUS_PROTOCOL_UNSEAL_MESSAGE_RESULT, resp2->header.command);

	CuAssertIntEquals (test, 0x06050403, resp2->unseal_status);
	CuAssertIntEquals (test, 0x0007, resp2->key_length);
	CuAssertPtrEquals (test, &raw_buffer_resp[11], &resp2->key);
}


CuSuite* get_cerberus_protocol_optional_commands_suite ()
{
	CuSuite *suite = CuSuiteNew ();

	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_prepare_pfm_update_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_pfm_update_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_complete_pfm_update_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_get_pfm_id_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_get_pfm_supported_fw_format);
	SUITE_ADD_TEST (suite,
		cerberus_protocol_optional_commands_test_prepare_recovery_image_update_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_recovery_image_update_format);
	SUITE_ADD_TEST (suite,
		cerberus_protocol_optional_commands_test_complete_recovery_image_update_format);
	SUITE_ADD_TEST (suite,
		cerberus_protocol_optional_commands_test_get_recovery_image_id_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_get_host_state_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_pmr_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_update_pmr_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_key_exchange_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_get_log_info_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_get_log_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_clear_log_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_get_attestation_data_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_prepare_fw_update_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_fw_update_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_complete_fw_update_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_reset_config_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_recover_firmware_format);
	SUITE_ADD_TEST (suite, cerberus_protocol_optional_commands_test_message_unseal_format);
	SUITE_ADD_TEST (suite ,cerberus_protocol_optional_commands_test_message_unseal_result_format);

	return suite;
}
