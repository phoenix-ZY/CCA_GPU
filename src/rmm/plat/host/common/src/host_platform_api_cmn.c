/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
 */

#include <debug.h>
#include <host_defs.h>
#include <host_utils.h>
#include <plat_common.h>
#include <stdint.h>
#include <xlat_tables.h>

COMPILER_ASSERT(RMM_MAX_GRANULES >= HOST_NR_GRANULES);

/* No regions to add for host */
struct xlat_mmap_region plat_regions[] = {
	{0}
};

/*
 * Local platform setup for RMM.
 *
 * This function will only be invoked during
 * warm boot and is expected to setup architecture and platform
 * components local to a PE executing RMM.
 */
void plat_warmboot_setup(uint64_t x0, uint64_t x1,
			 uint64_t x2, uint64_t x3)
{
	/* Avoid MISRA C:2102-2.7 warnings */
	(void)x0;
	(void)x1;
	(void)x2;
	(void)x3;

	if (plat_cmn_warmboot_setup() != 0) {
		panic();
	}
}

/*
 * Global platform setup for RMM.
 *
 * This function will only be invoked once during cold boot
 * and is expected to setup architecture and platform components
 * common for all PEs executing RMM. The translation tables should
 * be initialized by this function.
 */
void plat_setup(uint64_t x0, uint64_t x1,
		uint64_t x2, uint64_t x3)
{
	/* Initialize xlat table */
	if (plat_cmn_setup(x0, x1, x2, x3, plat_regions) != 0) {
		panic();
	}

	plat_warmboot_setup(x0, x1, x2, x3);
}

unsigned long plat_granule_addr_to_idx(unsigned long addr)
{
	if (!(GRANULE_ALIGNED(addr) &&
		(addr < (host_util_get_granule_base() + HOST_MEM_SIZE))) &&
		(addr >= host_util_get_granule_base())) {
		return UINT64_MAX;
	}

	return (addr - host_util_get_granule_base()) / GRANULE_SIZE;
}

unsigned long plat_granule_idx_to_addr(unsigned long idx)
{
	assert(idx < HOST_NR_GRANULES);
	return host_util_get_granule_base() + (idx * GRANULE_SIZE);
}
