/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
 * SPDX-FileCopyrightText: Copyright Arm Limited and Contributors.
 */

/* This file is derived from xlat_table_v2 library in TF-A project */

#include <arch_features.h>
#include <debug.h>
#include <errno.h>
#include <xlat_contexts.h>
#include <xlat_defs_private.h>
#include <xlat_tables.h>
#include <xlat_tables_private.h>

/*
 * Encode a Physical Address Space size for its use in TCR_ELx.
 */
static unsigned long long tcr_physical_addr_size_bits(uintptr_t max_addr)
{
	if ((max_addr & ADDR_MASK_48_TO_63) != 0U) {
		/* Physical address can't exceed 48 bits */
		panic();
	}

	/* 48 bits address */
	if ((max_addr & ADDR_MASK_44_TO_47) != 0U) {
		return TCR_PS_BITS_256TB;
	}

	/* 44 bits address */
	if ((max_addr & ADDR_MASK_42_TO_43) != 0U) {
		return TCR_PS_BITS_16TB;
	}

	/* 42 bits address */
	if ((max_addr & ADDR_MASK_40_TO_41) != 0U) {
		return TCR_PS_BITS_4TB;
	}

	/* 40 bits address */
	if ((max_addr & ADDR_MASK_36_TO_39) != 0U) {
		return TCR_PS_BITS_1TB;
	}

	/* 36 bits address */
	if ((max_addr & ADDR_MASK_32_TO_35) != 0U) {
		return TCR_PS_BITS_64GB;
	}

	return TCR_PS_BITS_4GB;
}

/*
 * Configure MMU registers. This function assumes that all the contexts use the
 * same limits for VA and PA spaces.
 */
int xlat_arch_setup_mmu_cfg(struct xlat_ctx * const ctx)
{
	uint64_t mair;
	uint64_t tcr;
	uint64_t ttbrx;
	uintptr_t va_space_size;
	struct xlat_ctx_cfg *ctx_cfg;
	struct xlat_ctx_tbls *ctx_tbls;
	unsigned int txsz;
	unsigned int t0sz;
	unsigned int t1sz;

	if (ctx == NULL) {
		return -EINVAL;
	}

	ctx_cfg = ctx->cfg;
	ctx_tbls = ctx->tbls;

	if (ctx_cfg == NULL || ctx_tbls == NULL) {
		return -EINVAL;
	}

	if (xlat_ctx_cfg_initialized(ctx) == false) {
		return -EINVAL;
	}

	/* MMU cannot be enabled at this point */
	if (is_mmu_enabled() == true) {
		return -EPERM;
	}

	/* Set attributes in the right indices of the MAIR. */
	mair = MAIR_ATTR_SET(ATTR_DEVICE, ATTR_DEVICE_INDEX);
	mair |= MAIR_ATTR_SET(ATTR_IWBWA_OWBWA_NTR, ATTR_IWBWA_OWBWA_NTR_INDEX);
	mair |= MAIR_ATTR_SET(ATTR_NON_CACHEABLE, ATTR_NON_CACHEABLE_INDEX);

	va_space_size = (uintptr_t)ctx_cfg->max_va_size;

	/*
	 * __builtin_ctzll(0) is undefined but here we are guaranteed that
	 * va_space_size is in the range [1,UINTPTR_MAX].
	 */
	txsz = 64 - __builtin_ctzll(va_space_size);

	/*
	 * Read TCR_EL2 in order to extract t0sz and t1sz. So we can update the right
	 * field depending on which context we are configuring and leave the other one
	 * untouched.
	 * It will not be a problem if TCR_EL2 was previoulsy configured, as the new
	 * value of it will be the same with the only difference of the txsz field we
	 * want to update.
	 */
	tcr = read_tcr_el2();
	if (ctx_cfg->region == VA_LOW_REGION) {
		t0sz = txsz;
		t1sz = (tcr >> TCR_EL2_T1SZ_SHIFT) & TCR_EL2_T1SZ_MASK;
	} else {
		t0sz = (tcr >> TCR_EL2_T0SZ_SHIFT) & TCR_EL2_T0SZ_MASK;
		t1sz = txsz;
	}

	/* Recompute the value for TCR_EL2 */
	tcr = (uint64_t)t0sz << TCR_EL2_T0SZ_SHIFT;
	tcr |= (uint64_t)t1sz << TCR_EL2_T1SZ_SHIFT;

	/*
	 * Set the cacheability and shareability attributes for memory
	 * associated with translation table walks.
	 */
	/* Inner & outer WBWA & shareable for both halfs. */
	tcr |= TCR_EL2_IRGN0_WBWA | TCR_EL2_ORGN0_WBWA | TCR_EL2_SH0_IS;
	tcr |= TCR_EL2_IRGN1_WBWA | TCR_EL2_ORGN1_WBWA | TCR_EL2_SH1_IS;

	/*
	 * ASID and hierarchical permissions.
	 */
	tcr |= TCR_EL2_AS | TCR_EL2_HPD0 | TCR_EL2_HPD1;

	/*
	 * Granule size. Only 4K supported on both halfs.
	 */
	tcr |= TCR_EL2_TG0_4K | TCR_EL2_TG1_4K;

	/*
	 * Set physical address size to the limit supported by the PE.
	 */
	tcr |= tcr_physical_addr_size_bits(xlat_arch_get_max_supported_pa());

	write_mair_el2(mair);
	write_tcr_el2(tcr);

	/*
	 * Set TTBR bits as well and enable CnP bit so as to share page
	 * tables with all PEs.
	 */
	ttbrx = (uint64_t)(void *)ctx_tbls->base_table;

	/*
	 * The VA region is not common for the HIGH region as it is used
	 * by slot buffer.
	 */
	if (ctx_cfg->region == VA_HIGH_REGION) {
		ttbrx &= ~TTBR_CNP_BIT;
	} else {
		ttbrx |= TTBR_CNP_BIT;
	}

	if (ctx_cfg->region == VA_LOW_REGION) {
		write_ttbr0_el2(ttbrx);
	} else {
		write_ttbr1_el2(ttbrx);
	}

	return 0;
}

uintptr_t xlat_arch_get_max_supported_pa(void)
{
	return (1UL << arch_feat_get_pa_width()) - 1UL;
}

void xlat_arch_tlbi_va(uintptr_t va)
{
	/*
	 * Ensure the translation table write has drained into memory before
	 * invalidating the TLB entry.
	 */
	dsb(ishst);

	tlbivae2is(TLBI_ADDR(va));
}

void xlat_arch_tlbi_va_sync(void)
{
	/*
	 * A TLB maintenance instruction can complete at any time after
	 * it is issued, but is only guaranteed to be complete after the
	 * execution of DSB by the PE that executed the TLB maintenance
	 * instruction. After the TLB invalidate instruction is
	 * complete, no new memory accesses using the invalidated TLB
	 * entries will be observed by any observer of the system
	 * domain. See section D4.8.2 of the ARMv8 (issue k), paragraph
	 * "Ordering and completion of TLB maintenance instructions".
	 */
	dsb(ish);

	/*
	 * The effects of a completed TLB maintenance instruction are
	 * only guaranteed to be visible on the PE that executed the
	 * instruction after the execution of an ISB instruction by the
	 * PE that executed the TLB maintenance instruction.
	 */
	isb();
}

/*
 * Determine the physical address space encoded in the 'attr' parameter.
 */
uint64_t xlat_arch_get_pas(uint64_t attr)
{
	uint64_t pas = MT_PAS(attr);

	switch (pas) {
	case MT_REALM:
		return 0U;
	case MT_NS:
		return LOWER_ATTRS(NS);
	default:
		panic();
	}

	/* Avoid -Werror=return-type. Should never reach here. */
	return 0U;
}
