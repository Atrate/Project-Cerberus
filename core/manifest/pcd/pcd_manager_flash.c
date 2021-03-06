// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "state_manager/system_state_manager.h"
#include "pcd_manager_flash.h"


static struct pcd* pcd_manager_flash_get_pcd (struct pcd_manager_flash *manager, bool active)
{
	struct pcd_flash *flash;
	struct manifest_manager_flash_region *region;

	if (manager == NULL) {
		return NULL;
	}

	region = manifest_manager_flash_get_manifest_region (&manager->manifest_manager, active);
	if (region == NULL) {
		return NULL;
	}

	flash = (struct pcd_flash*) region->manifest;

	return &flash->base;
}

static struct pcd* pcd_manager_flash_get_active_pcd (struct pcd_manager *manager)
{
	struct pcd_manager_flash *pcd_mgr = (struct pcd_manager_flash*) manager;

	return pcd_manager_flash_get_pcd (pcd_mgr, true);
}

static void pcd_manager_flash_free_pcd (struct pcd_manager *manager, struct pcd *pcd)
{
	struct pcd_manager_flash *pcd_mgr = (struct pcd_manager_flash*) manager;

	if (pcd_mgr == NULL) {
		return;
	}

	manifest_manager_flash_free_manifest (&pcd_mgr->manifest_manager, (struct manifest*) pcd);
}

static int pcd_manager_flash_activate_pending_pcd (struct manifest_manager *manager)
{
	struct pcd_manager_flash *pcd_mgr = (struct pcd_manager_flash*) manager;
	int status;

	if (pcd_mgr == NULL) {
		return MANIFEST_MANAGER_INVALID_ARGUMENT;
	}

	status = manifest_manager_flash_activate_pending_manifest (&pcd_mgr->manifest_manager);
	if (status == 0) {
		pcd_manager_on_pcd_activated (&pcd_mgr->base);
	}

	return status;
}

static int pcd_manager_flash_clear_pending_region (struct manifest_manager *manager, size_t size)
{
	struct pcd_manager_flash *pcd_mgr = (struct pcd_manager_flash*) manager;

	if (pcd_mgr == NULL) {
		return MANIFEST_MANAGER_INVALID_ARGUMENT;
	}

	return manifest_manager_flash_clear_pending_region (&pcd_mgr->manifest_manager, size);
}

static int pcd_manager_flash_write_pending_data (struct manifest_manager *manager,
	const uint8_t *data, size_t length)
{
	struct pcd_manager_flash *pcd_mgr = (struct pcd_manager_flash*) manager;

	if (pcd_mgr == NULL) {
		return MANIFEST_MANAGER_INVALID_ARGUMENT;
	}

	return manifest_manager_flash_write_pending_data (&pcd_mgr->manifest_manager, data, length);
}

static int pcd_manager_flash_verify_pending_pcd (struct manifest_manager *manager)
{
	struct pcd_manager_flash *pcd_mgr = (struct pcd_manager_flash*) manager;
	int status;

	if (pcd_mgr == NULL) {
		return MANIFEST_MANAGER_INVALID_ARGUMENT;
	}

	status = manifest_manager_flash_verify_pending_manifest (&pcd_mgr->manifest_manager);
	if (status == 0) {
		pcd_manager_on_pcd_verified (&pcd_mgr->base);
		pcd_manager_flash_activate_pending_pcd (manager);
	}

	return status;
}

int pcd_manager_flash_clear_all_manifests (struct manifest_manager *manager)
{
	struct pcd_manager_flash *pcd_mgr = (struct pcd_manager_flash*) manager;

	if (pcd_mgr == NULL) {
		return MANIFEST_MANAGER_INVALID_ARGUMENT;
	}

	return manifest_manager_flash_clear_all_manifests (&pcd_mgr->manifest_manager);
}

/**
 * If there is both an active and pending PCD, check that the platform identifier of the pending
 * PCD matches the active.  If not, mark the pending PCD as invalid.
 *
 * @param manager The PCD manager to update.
 *
 * @return 0 if the check completed successfully or an error code.
 */
static int pcd_manager_flash_check_pending_platform_id (struct manifest_manager_flash *manager)
{
	struct manifest_manager_flash_region *active;
	struct manifest_manager_flash_region *pending;
	struct pcd_flash *active_pcd;
	struct pcd_flash *pending_pcd;
	char *active_id = NULL;
	char *pending_id = NULL;
	int status = 0;

	active = manifest_manager_flash_get_region (manager, true);
	pending = manifest_manager_flash_get_region (manager, false);

	if (active->is_valid && pending->is_valid) {
		active_pcd = (struct pcd_flash*) active->manifest;
		pending_pcd = (struct pcd_flash*) pending->manifest;

		status = active_pcd->base.base.get_platform_id (&active_pcd->base.base, &active_id);
		if (status == 0) {
			status = pending_pcd->base.base.get_platform_id (&pending_pcd->base.base, &pending_id);
		}

		if (status == 0) {
			if (strcmp (active_id, pending_id) != 0) {
				pending->is_valid = false;
				status = MANIFEST_MANAGER_INCOMPATIBLE;
			}
		}
		else {
			pending->is_valid = false;
		}

		platform_free (active_id);
		platform_free (pending_id);
	}

	return status;
}

/**
 * Initialize the manager for handling PCDs.
 *
 * @param manager The PCD manager to initialize.
 * @param pcd_region1 The PCD instance for the first flash region that can hold a PCD. This region
 * does not need to have a valid PCD.The region is expected to a single flash erase  block as
 * defined by FLASH_BLOCK_SIZE, aligned to the beginning of the block.
 * @param pcd_region2 The PCD instance for the second flash region that can hold a PCD. This region
 * does not need to have a valid PCD. The region is expected to a single flash erase block as
 * defined by FLASH_BLOCK_SIZE, aligned to the beginning of the block.
 * @param state The state information for PCD management.
 * @param hash The hash engine to be used for PCD validation.
 * @param verification The module to be used for PCD verification.
 *
 * @return 0 if the PCD manager was successfully initialized or an error code.
 */
int pcd_manager_flash_init (struct pcd_manager_flash *manager, struct pcd_flash *pcd_region1,
	struct pcd_flash *pcd_region2, struct state_manager *state, struct hash_engine *hash,
	struct signature_verification *verification)
{
	int status;

	if ((manager == NULL) || (pcd_region1 == NULL) || (pcd_region2 == NULL) || (state == NULL)) {
		return MANIFEST_MANAGER_INVALID_ARGUMENT;
	}

	memset (manager, 0, sizeof (struct pcd_manager_flash));

	status = pcd_manager_init (&manager->base);
	if (status != 0) {
		return status;
	}

	status = manifest_manager_flash_init (&manager->manifest_manager, &pcd_region1->base.base,
		&pcd_region2->base.base, state, hash, verification, pcd_flash_get_flash (pcd_region1),
		pcd_flash_get_addr (pcd_region1), pcd_flash_get_flash (pcd_region2),
		pcd_flash_get_addr (pcd_region2), SYSTEM_STATE_MANIFEST_PCD);
	if (status != 0) {
		pcd_manager_release (&manager->base);
		return status;
	}

	manager->base.get_active_pcd = pcd_manager_flash_get_active_pcd;
	manager->base.free_pcd = pcd_manager_flash_free_pcd;
	manager->base.base.activate_pending_manifest = pcd_manager_flash_activate_pending_pcd;
	manager->base.base.clear_pending_region = pcd_manager_flash_clear_pending_region;
	manager->base.base.write_pending_data = pcd_manager_flash_write_pending_data;
	manager->base.base.verify_pending_manifest = pcd_manager_flash_verify_pending_pcd;
	manager->base.base.clear_all_manifests = pcd_manager_flash_clear_all_manifests;

	manager->manifest_manager.post_verify = pcd_manager_flash_check_pending_platform_id;

	status = pcd_manager_flash_activate_pending_pcd (&manager->base.base);
	if (status == MANIFEST_MANAGER_NONE_PENDING) {
		status = 0;
	}

	return 0;
}

/**
 * Release the resources used by a manager of PCDs in flash.
 *
 * @param manager The PCD manager to release.
 */
void pcd_manager_flash_release (struct pcd_manager_flash *manager)
{
	if (manager != NULL) {
		pcd_manager_release (&manager->base);
		manifest_manager_flash_release (&manager->manifest_manager);
	}
}
