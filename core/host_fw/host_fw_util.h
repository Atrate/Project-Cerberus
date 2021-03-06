// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef HOST_FW_UTIL_H_
#define HOST_FW_UTIL_H_

#include <stdint.h>
#include <stdbool.h>
#include "status/rot_status.h"
#include "manifest/pfm/pfm.h"
#include "flash/spi_flash.h"
#include "spi_filter/spi_filter_interface.h"
#include "crypto/hash.h"
#include "crypto/rsa.h"


int host_fw_determine_version (struct spi_flash *flash, const struct pfm_firmware_versions *allowed,
	const struct pfm_firmware_version **version);
int host_fw_determine_offset_version (struct spi_flash *flash, uint32_t offset,
	const struct pfm_firmware_versions *allowed, const struct pfm_firmware_version **version);

bool host_fw_are_images_different (const struct pfm_image_list *img_list1,
	const struct pfm_image_list *img_list2);
int host_fw_verify_images (struct spi_flash *flash, const struct pfm_image_list *img_list,
	struct hash_engine *hash, struct rsa_engine *rsa);
int host_fw_verify_offset_images (struct spi_flash *flash, const struct pfm_image_list *img_list,
	uint32_t offset, struct hash_engine *hash, struct rsa_engine *rsa);
int host_fw_full_flash_verification (struct spi_flash *flash, const struct pfm_image_list *img_list,
	const struct pfm_read_write_regions *writable, uint8_t unused_byte, struct hash_engine *hash,
	struct rsa_engine *rsa);

bool host_fw_are_read_write_regions_different (const struct pfm_read_write_regions *rw1,
	const struct pfm_read_write_regions *rw2);
int host_fw_migrate_read_write_data (struct spi_flash *dest,
	const struct pfm_read_write_regions *dest_writable, struct spi_flash *src,
	const struct pfm_read_write_regions *src_writable);

int host_fw_restore_flash_device (struct spi_flash *restore, struct spi_flash *from,
	const struct pfm_image_list *img_list, const struct pfm_read_write_regions *writable);

int host_fw_config_spi_filter_read_write_regions (struct spi_filter_interface *filter,
	const struct pfm_read_write_regions *writable);


#define	HOST_FW_UTIL_ERROR(code)		ROT_ERROR (ROT_MODULE_HOST_FW_UTIL, code)

/**
 * Error codes that can be generated by the host firmware utilities.
 */
enum {
	HOST_FW_UTIL_INVALID_ARGUMENT = HOST_FW_UTIL_ERROR (0),		/**< Input parameter is null or not valid. */
	HOST_FW_UTIL_NO_MEMORY = HOST_FW_UTIL_ERROR (1),			/**< Memory allocation failed. */
	HOST_FW_UTIL_UNSUPPORTED_VERSION = HOST_FW_UTIL_ERROR (2),	/**< The host firmware does not match a supported version. */
	HOST_FW_UTIL_DIFF_REGION_COUNT = HOST_FW_UTIL_ERROR (3),	/**< Data migration with a different number of regions. */
	HOST_FW_UTIL_DIFF_REGION_ADDR = HOST_FW_UTIL_ERROR (4),		/**< Data migration with different region addresses. */
	HOST_FW_UTIL_DIFF_REGION_SIZE = HOST_FW_UTIL_ERROR (5),		/**< Data migration with different region sizes. */
};


#endif /* HOST_FW_UTIL_H_ */
