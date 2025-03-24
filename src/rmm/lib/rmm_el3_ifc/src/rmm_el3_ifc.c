/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
 */

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <rmm_el3_ifc.h>
#include <rmm_el3_ifc_priv.h>
#include <smc.h>
#include <stdint.h>
#include <xlat_defs.h>

/* Boot Interface arguments */
static uintptr_t rmm_shared_buffer_start_pa;
static unsigned long rmm_el3_ifc_abi_version;

/* Platform paramters */
uintptr_t rmm_shared_buffer_start_va;

/* Internal status */
static bool initialized;

/*
 * Abort the boot process and return to EL3 FW reporting
 * the ec error code.
 */
__dead2 static void report_fail_to_el3(uint64_t ec)
{
	(void)monitor_call(SMC_RMM_BOOT_COMPLETE,
			   ec, 0UL, 0UL, 0UL, 0UL, 0UL);
	/* EL3 should never return back here */
	panic();
}

/*
 * Validate the boot arguments and Initialize the rmm_el3_ifc library.
 * This function must be called only once during cold boot.
 *
 * This function must be called prior to enable the MMU and data cache.
 */
int rmm_el3_ifc_init(unsigned long x0, unsigned long x1, unsigned long x2,
		     unsigned long x3, uintptr_t shared_buf_va)
{
	assert(is_mmu_enabled() == false);
	assert(initialized == false);
	assert((shared_buf_va & PAGE_SIZE_MASK) == 0UL);
	assert(shared_buf_va != 0UL);

	/*
	 * Validate that the version number is correct. Only the Major is
	 * considered for this check.
	 *
	 * Version Minor will be used to implement compatibility where
	 * feasible.
	 *
	 * x1: RMM-EL3 Interface version.
	 */
	if (RMM_EL3_IFC_GET_VERS_MAJOR(x1) != RMM_EL3_IFC_VERS_MAJOR) {
		report_fail_to_el3(E_RMM_BOOT_VERSION_MISMATCH);
	}

	/*
	 * Validate the number of CPUs received from EL3.
	 *
	 * x2: Number of CPUs in the system as reported by EL3.
	 */
	if (x2 > MAX_CPUS) {
		report_fail_to_el3(E_RMM_BOOT_CPUS_OUT_OF_RANGE);
	}

	/*
	 * Validate that the CPU Id is in the range of the maximum
	 * number of CPUs.
	 *
	 * x0: CPU Id.
	 * x2: Number of CPUs in the system as reported by EL3.
	 */
	if (x0 >= x2) {
		report_fail_to_el3(E_RMM_BOOT_CPU_ID_OUT_OF_RANGE);
	}

	/*
	 * Validate that the shared buffer pointer is not NULL.
	 *
	 * x3: Pointer to the start of the EL3-RMM shared buffer.
	 */
	if ((x3 == 0UL) || ((x3 & PAGE_SIZE_MASK) != 0U)) {
		report_fail_to_el3(E_RMM_BOOT_INVALID_SHARED_POINTER);
	}

	rmm_el3_ifc_abi_version = x1;
	rmm_shared_buffer_start_pa = (uintptr_t)x3;
	rmm_shared_buffer_start_va = shared_buf_va;

	initialized = true;

	flush_dcache_range((uintptr_t)(void *)&rmm_shared_buffer_start_pa,
			 sizeof(rmm_shared_buffer_start_pa));
	flush_dcache_range((uintptr_t)(void *)&rmm_el3_ifc_abi_version,
			 sizeof(rmm_el3_ifc_abi_version));
	flush_dcache_range((uintptr_t)(void *)&rmm_shared_buffer_start_va,
			 sizeof(rmm_shared_buffer_start_va));
	flush_dcache_range((uintptr_t)(void *)&initialized, sizeof(bool));

	/* Process the Boot Manifest */
	rmm_el3_ifc_process_boot_manifest();

	return 0;
}

/*
 * Get a pointer to the PA of the start of the RMM<->EL3 shared area.
 */
uintptr_t rmm_el3_ifc_get_shared_buf_pa(void)
{
	assert((initialized == true));

	return rmm_shared_buffer_start_pa;
}

/* Get the raw value of the boot interface version */
unsigned int rmm_el3_ifc_get_version(void)
{
	assert(initialized == true);

	return rmm_el3_ifc_abi_version;
}
