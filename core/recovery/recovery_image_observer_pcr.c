// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "recovery_image_observer_pcr.h"
#include "recovery_image_logging.h"
#include "attestation/pcr_store.h"


static void recovery_image_observer_pcr_on_recovery_image_activated (
	struct recovery_image_observer *observer, struct recovery_image *active)
{
	struct recovery_image_observer_pcr *pcr = (struct recovery_image_observer_pcr*) observer;
	uint8_t measurement[SHA256_HASH_LENGTH];
	int status;

	status = active->get_hash (active, pcr->hash, measurement, sizeof (measurement));
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_RECOVERY_IMAGE,
			RECOVERY_IMAGE_LOGGING_GET_MEASUREMENT_FAIL, pcr->measurement_id, status);
		return;
	}

	status = pcr_store_update_digest (pcr->store, pcr->measurement_id, measurement,
		SHA256_HASH_LENGTH);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_RECOVERY_IMAGE,
			RECOVERY_IMAGE_LOGGING_RECORD_MEASUREMENT_FAIL, pcr->measurement_id, status);
	}
}

static void recovery_image_observer_pcr_on_recovery_image_deactivated (
	struct recovery_image_observer *observer)
{
	struct recovery_image_observer_pcr *pcr = (struct recovery_image_observer_pcr*) observer;
	int status;

	status = pcr_store_invalidate_measurement (pcr->store, pcr->measurement_id);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_RECOVERY_IMAGE,
			RECOVERY_IMAGE_LOGGING_INVALIDATE_MEASUREMENT_FAIL, pcr->measurement_id, status);
	}
}

/**
 * Initialize the recovery image observer for updating PCR entries.
 *
 * @param observer The observer to initialize.
 * @param hash The hash engine to use for generating PCR measurements.
 * @param store The PCR store to update as the recovery image changes.
 * @param measurement_type The identifier for the measurement in the PCR.
 *
 * @return 0 if the observer was successfully initialized or an error code.
 */
int recovery_image_observer_pcr_init (struct recovery_image_observer_pcr *observer, struct hash_engine *hash,
	struct pcr_store *store, uint16_t measurement_type)
{
	int status;

	if ((observer == NULL) || (hash == NULL) || (store == NULL)) {
		return RECOVERY_IMAGE_OBSERVER_INVALID_ARGUMENT;
	}

	memset (observer, 0, sizeof (struct recovery_image_observer_pcr));

	status = pcr_store_check_measurement_type (store, measurement_type);
	if (status != 0) {
		return status;
	}

	observer->hash = hash;
	observer->store = store;
	observer->measurement_id = measurement_type;

	observer->base.on_recovery_image_activated = recovery_image_observer_pcr_on_recovery_image_activated;
	observer->base.on_recovery_image_deactivated = recovery_image_observer_pcr_on_recovery_image_deactivated;

	return 0;
}

/**
 * Release the resources used by the recovery image observer.
 *
 * @param observer The observer to release.
 */
void recovery_image_observer_pcr_release (struct recovery_image_observer_pcr *observer)
{

}

/**
 * Force the PCR to be updated with the current active recovery image measurement.
 *
 * @param observer The observer for the PCR to update.
 * @param manager The manager for the PFM measurement to update.
 */
void recovery_image_observer_pcr_record_measurement (struct recovery_image_observer_pcr *observer,
	struct recovery_image_manager *manager)
{
	struct recovery_image *active;

	if ((observer == NULL) || (manager == NULL)) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_RECOVERY_IMAGE,
			RECOVERY_IMAGE_LOGGING_RECORD_INVALID, RECOVERY_IMAGE_OBSERVER_INVALID_ARGUMENT, 0);
		return;
	}

	active = manager->get_active_recovery_image (manager);
	if (active) {
		recovery_image_observer_pcr_on_recovery_image_activated (&observer->base, active);
		manager->free_recovery_image (manager, active);
	}
}
