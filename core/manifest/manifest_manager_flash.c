// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "manifest_manager_flash.h"
#include "manifest_manager.h"
#include "manifest_logging.h"
#include "flash/flash_util.h"
#include "flash/flash_common.h"
#include "crypto/ecc.h"


/**
 * If there is both an active and pending manifest available, check the ID of the pending manifest
 * to see that it is valid relative to the active manifest. If not, mark the pending manifest as
 * invalid.
 *
 * @param manager The manifest manager to use for validation.
 *
 * @return 0 if the check completed successfully or an error code.
 */
static int manifest_manager_flash_check_pending_manifest_id (struct manifest_manager_flash *manager)
{
	struct manifest_manager_flash_region *active;
	struct manifest_manager_flash_region *pending;
	uint32_t active_id;
	uint32_t pending_id;
	int status = 0;

	active = manifest_manager_flash_get_region (manager, true);
	pending = manifest_manager_flash_get_region (manager, false);

	if (active->is_valid && pending->is_valid) {
		status = active->manifest->get_id (active->manifest, &active_id);
		if (status == 0) {
			status = pending->manifest->get_id (pending->manifest, &pending_id);
		}

		if (status == 0) {
			if (pending_id <= active_id) {
				pending->is_valid = false;
				status = MANIFEST_MANAGER_INVALID_ID;
			}
		}
		else {
			pending->is_valid = false;
		}
	}

	return status;
}

/**
 * Check if a single manifest flash region contains a valid manifest.
 *
 * @param manager The manifest manager to use for verification.
 * @param manifest The manifest interface to use for verification.
 * @param region The region to verify.
 *
 * @return 0 if the manifest was determined to be either valid or invalid. An error code if the
 * validity of the manifest could not be determined.
 */
static int manifest_manager_flash_verify_manifest (struct manifest_manager_flash *manager,
	struct manifest *manifest, struct manifest_manager_flash_region *region)
{
	int status = manifest->verify (manifest, manager->hash, manager->verification, NULL, 0);

	if (status == 0) {
		region->is_valid = true;
	}
	else if ((status == RSA_ENGINE_BAD_SIGNATURE) || (status == ECC_ENGINE_BAD_SIGNATURE) ||
		(status == MANIFEST_BAD_MAGIC_NUMBER) || (status == MANIFEST_BAD_LENGTH) ||
		(status == MANIFEST_MALFORMED)) {
		region->is_valid = false;
		status = 0;
	}

	return status;
}

/**
 * Initialize the manager for handling manifests.
 *
 * @param manager The manifest manager to initialize.
 * @param region1 The manifest instance for the first flash region that can hold a manifest.
 * @param region2 The manifest instance for the second flash region that can hold a manifest.
 * @param state The state information for manifest management.
 * @param hash The hash engine to be used for manifest validation.
 * @param verification The module to use for manifest verification.
 * @param region1_flash The flash device containing region 1 manifest
 * @param region1_addr The starting flash address of region 1 manifest
 * @param region2_flash The flash device containing region 2 manifest
 * @param region2_addr The starting flash address of region 2 manifest
 * @param manifest_index State manager manifest index to utilize.
 *
 * @return 0 if the manifest manager was successfully initialized or an error code.
 */
int manifest_manager_flash_init (struct manifest_manager_flash *manager, struct manifest *region1,
	struct manifest *region2, struct state_manager *state, struct hash_engine *hash,
	struct signature_verification *verification, struct spi_flash *region1_flash,
	uint32_t region1_addr, struct spi_flash *region2_flash, uint32_t region2_addr,
	uint8_t manifest_index)
{
	int status;

	if ((state == NULL) || (hash == NULL) || (verification == NULL)) {
		return MANIFEST_MANAGER_INVALID_ARGUMENT;
	}

	manager->region1.manifest = region1;
	manager->region2.manifest = region2;
	manager->state = state;
	manager->hash = hash;
	manager->verification = verification;
	manager->manifest_index = manifest_index;

	status = state->is_manifest_valid (state, manifest_index);
	if (status != 0) {
		return status;
	}

	status = manifest_manager_flash_verify_manifest (manager, region1, &manager->region1);
	if (status != 0) {
		return status;
	}

	status = manifest_manager_flash_verify_manifest (manager, region2, &manager->region2);
	if (status != 0) {
		return status;
	}

	status = manifest_manager_flash_check_pending_manifest_id (manager);
	if ((status != 0) && (status != MANIFEST_MANAGER_INVALID_ID)) {
		return status;
	}

	status = flash_updater_init (&manager->region1.updater, &region1_flash->base, region1_addr,
		FLASH_BLOCK_SIZE);
	if (status != 0) {
		return status;
	}

	status = flash_updater_init (&manager->region2.updater, &region2_flash->base, region2_addr,
		FLASH_BLOCK_SIZE);
	if (status != 0) {
		goto error_updater;
	}

	status = platform_mutex_init (&manager->lock);
	if (status != 0) {
		goto error_mutex;
	}

	return 0;

error_mutex:
	flash_updater_release (&manager->region2.updater);
error_updater:
	flash_updater_release (&manager->region1.updater);
	return status;
}

/**
 * Release the resources used by a manager of manifests in flash.
 *
 * @param manager The manifest manager to release.
 */
void manifest_manager_flash_release (struct manifest_manager_flash *manager)
{
	platform_mutex_free (&manager->lock);
	flash_updater_release (&manager->region1.updater);
	flash_updater_release (&manager->region2.updater);
}

/**
 * Get the active or pending manifest region based on the current system state.
 *
 * @param manager The manifest manager instance to query.
 * @param active Flag to indicate which region to retrieve, active or pending.
 *
 * @return The manifest region.
 */
struct manifest_manager_flash_region* manifest_manager_flash_get_region (
	struct manifest_manager_flash *manager, bool active)
{
	enum manifest_region current = manager->state->get_active_manifest (manager->state,
		manager->manifest_index);

	if (current == MANIFEST_REGION_1) {
		return (active) ? &manager->region1 : &manager->region2;
	}
	else {
		return (active) ? &manager->region2 : &manager->region1;
	}
}

/**
 * Get the active manifest region for the protected flash. The manifest instance must be released
 * with the manager.
 *
 * @param manager The manifest manager to query.
 * @param active Flag to indicate which region to retrieve, active or pending.
 *
 * @return The active manifest region or null if there is no active manifest.
 */
struct manifest_manager_flash_region* manifest_manager_flash_get_manifest_region (
	struct manifest_manager_flash *manager, bool active)
{
	struct manifest_manager_flash_region *region;

	platform_mutex_lock (&manager->lock);

	region = manifest_manager_flash_get_region (manager, active);
	if (region->is_valid) {
		region->ref_count++;
	}

	platform_mutex_unlock (&manager->lock);

	return (region->is_valid) ? region : NULL;
}

/**
 * Release a manifest instance retrieved from the manager. Manifest instances must only be
 * released by the manager that allocated them.
 *
 * @param manager The manifest manager that allocated the manifest instance.
 * @param manifest The manifest to release.
 */
void manifest_manager_flash_free_manifest (struct manifest_manager_flash *manager,
	struct manifest *manifest)
{
	struct manifest_manager_flash_region *region;

	platform_mutex_lock (&manager->lock);

	if (manifest == manager->region1.manifest) {
		region = &manager->region1;
	}
	else if (manifest == manager->region2.manifest) {
		region = &manager->region2;
	}
	else {
		region = NULL;
	}

	if (region && (region->ref_count > 0)) {
		region->ref_count--;
	}

	platform_mutex_unlock (&manager->lock);
}

/**
 * Activate the pending manifest and discard the active manifest.
 *
 * @param manager The manifest manager to update.
 *
 * @return 0 if the pending manifest was successfully activated or an error if there no pending
 * manifest to activate.
 */
int manifest_manager_flash_activate_pending_manifest (struct manifest_manager_flash *manager)
{
	enum manifest_region active;
	int status = 0;

	platform_mutex_lock (&manager->lock);

	active = manager->state->get_active_manifest (manager->state, manager->manifest_index);

	if (active == MANIFEST_REGION_1) {
		if (!manager->region2.is_valid) {
			status = MANIFEST_MANAGER_NONE_PENDING;
			goto exit;
		}

		manager->state->save_active_manifest (manager->state, manager->manifest_index,
			MANIFEST_REGION_2);
		manager->region1.is_valid = false;
	}
	else {
		if (!manager->region1.is_valid) {
			status = MANIFEST_MANAGER_NONE_PENDING;
			goto exit;
		}

		manager->state->save_active_manifest (manager->state, manager->manifest_index,
			MANIFEST_REGION_1);
		manager->region2.is_valid = false;
	}

exit:
	platform_mutex_unlock (&manager->lock);
	return status;
}

/**
 * Clear the pending manifest region in order to accept new manifest data.
 *
 * @param manager The manifest manager for the pending region to clear.
 * @param size Size of incoming manifest
 *
 * @return 0 if the pending manifest region was successfully cleared or an error code.
 */
int manifest_manager_flash_clear_pending_region (struct manifest_manager_flash *manager,
	size_t size)
{
	struct manifest_manager_flash_region *region;
	int status;

	platform_mutex_lock (&manager->lock);

	region = manifest_manager_flash_get_region (manager, false);
	if (region->ref_count == 0) {
		status = flash_updater_check_update_size (&region->updater, size);
		if (status != 0) {
			platform_mutex_unlock (&manager->lock);
			return status;
		}

		manager->updating = &region->updater;
		region->is_valid = false;
	}
	else {
		platform_mutex_unlock (&manager->lock);
		return MANIFEST_MANAGER_PENDING_IN_USE;
	}

	platform_mutex_unlock (&manager->lock);

	return flash_updater_prepare_for_update_erase_all (manager->updating, size);
}

/**
 * Write data to the pending manifest region. This data must be written sequentially.
 *
 * @param manager The manifest interface to use.
 * @param data The data that should be written.
 * @param length The length of the data to write.
 *
 * @return 0 if the data was successfully written or an error code.
 */
int manifest_manager_flash_write_pending_data (struct manifest_manager_flash *manager,
	const uint8_t *data, size_t length)
{
	if (data == NULL) {
		return MANIFEST_MANAGER_INVALID_ARGUMENT;
	}

	if (manager->updating == NULL) {
		return MANIFEST_MANAGER_NOT_CLEARED;
	}

	return flash_updater_write_update_data (manager->updating, data, length);
}

/**
 * After all manifest has been written to the pending area, verify that the region contains a
 * valid manifest.
 *
 * @param manager The manifest manager to use for validation.
 *
 * @return 0 if the pending manifest was successfully validated or an error code.
 */
int manifest_manager_flash_verify_pending_manifest (struct manifest_manager_flash *manager)
{
	struct manifest_manager_flash_region *region;
	int status = 0;

	platform_mutex_lock (&manager->lock);

	if (flash_updater_get_remaining_bytes (manager->updating) > 0) {
		status = MANIFEST_MANAGER_INCOMPLETE_UPDATE;
		goto exit;
	}

	region = manifest_manager_flash_get_region (manager, false);
	if (!region->is_valid) {
		if (manager->updating != NULL) {
			status = region->manifest->verify (region->manifest, manager->hash,
				manager->verification, NULL, 0);
			if (status == 0) {
				region->is_valid = true;
			}
		}
		else {
			status = MANIFEST_MANAGER_NONE_PENDING;
		}
	}
	else {
		status = MANIFEST_MANAGER_HAS_PENDING;
	}

	if (status == 0) {
		status = manifest_manager_flash_check_pending_manifest_id (manager);
		if (status != 0) {
			goto exit;
		}

		if (manager->post_verify) {
			status = manager->post_verify (manager);
			if (status != 0) {
				goto exit;
			}
		}
	}

exit:
	manager->updating = NULL;

	platform_mutex_unlock (&manager->lock);
	return status;
}

/**
 * Erase a single manifest and mark it as invalid.
 *
 * @param region The manifest region to erase.
 * @param in_use_error The error to return if the region is in use.
 *
 * @return 0 if the region was erased or an error code.
 */
static int manifest_manager_flash_clear_manifest (struct manifest_manager_flash_region *region,
	int in_use_error)
{
	if (region->ref_count != 0) {
		return in_use_error;
	}

	region->is_valid = false;
	return flash_erase_region (region->updater.flash, region->updater.base_addr, FLASH_BLOCK_SIZE);
}

/**
 * Erase both the active and pending regions and mark both manifests as invalid.
 *
 * @param manager The manifest manager to clear.
 *
 * @return 0 if the manifests were erased or an error code.
 */
int manifest_manager_flash_clear_all_manifests (struct manifest_manager_flash *manager)
{
	int status;

	platform_mutex_lock (&manager->lock);

	status = manifest_manager_flash_clear_manifest (
		manifest_manager_flash_get_region (manager, false), MANIFEST_MANAGER_PENDING_IN_USE);
	if (status != 0) {
		goto exit;
	}

	manager->updating = NULL;
	status = manifest_manager_flash_clear_manifest (
		manifest_manager_flash_get_region (manager, true), MANIFEST_MANAGER_ACTIVE_IN_USE);

exit:
	platform_mutex_unlock (&manager->lock);
	return status;
}