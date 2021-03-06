// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef PFM_MANAGER_H_
#define PFM_MANAGER_H_

#include <stdint.h>
#include <stddef.h>
#include "platform.h"
#include "manifest/manifest_manager.h"
#include "manifest/pfm/pfm.h"
#include "manifest/pfm/pfm_observer.h"
#include "common/observable.h"


/**
 * API for managing the PFM for a single set of protected flash.
 */
struct pfm_manager {
	struct manifest_manager base;					/**< Manifest manager interface */
	struct observable observable;					/**< The manager for PFM observers. */

	/**
	 * Get the active PFM for the protected flash.  The PFM instance must be released with the
	 * manager.
	 *
	 * @param manager The PFM manager to query.
	 *
	 * @return The active PFM or null if there is no active PFM.
	 */
	struct pfm* (*get_active_pfm) (struct pfm_manager *manager);

	/**
	 * Get the PFM that is waiting to be activated.  The PFM instance must be released with the
	 * manager.
	 *
	 * @param manager The PFM manager to query.
	 *
	 * @return The pending PFM or null if there is no pending PFM.
	 */
	struct pfm* (*get_pending_pfm) (struct pfm_manager *manager);

	/**
	 * Release a PFM instance retrieved from the manager.  PFM instances must only be released by
	 * the manager that allocated them.
	 *
	 * @param manager The PFM manager that allocated the PFM instance.
	 * @param pfm The PFM to release.
	 */
	void (*free_pfm) (struct pfm_manager *manager, struct pfm *pfm);
};


int pfm_manager_add_observer (struct pfm_manager *manager, struct pfm_observer *observer);
int pfm_manager_remove_observer (struct pfm_manager *manager, struct pfm_observer *observer);

/* Internal functions for use by derived types. */
int pfm_manager_init (struct pfm_manager *manager, int port);
void pfm_manager_release (struct pfm_manager *manager);

void pfm_manager_on_pfm_verified (struct pfm_manager *manager);
void pfm_manager_on_pfm_activated (struct pfm_manager *manager);


#endif /* PFM_MANAGER_H_ */
