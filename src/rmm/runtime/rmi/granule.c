/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
 */

#include <asc.h>
#include <granule.h>
#include <smc-handler.h>
#include <smc-rmi.h>
#include <smc.h>
#include <debug.h>
#include <benchmark.h>
unsigned long smc_granule_delegate(unsigned long addr)
{
	smc_granule_delegate_cca_marker();
	struct granule *g;

	g = find_lock_granule(addr, GRANULE_STATE_NS);
	if (g == NULL) {
		return RMI_ERROR_INPUT;
	}

	granule_set_state(g, GRANULE_STATE_DELEGATED);
	asc_mark_secure(addr);
	granule_memzero(g, SLOT_DELEGATED);

	granule_unlock(g);
	return RMI_SUCCESS;
}

unsigned long smc_add_page_to_smmu_tables(unsigned long phys_addr, unsigned long iova, unsigned int sid)
{
	asc_add_translation_table(phys_addr, iova, sid);
	return RMI_SUCCESS;
}

unsigned long smc_granule_delegate_dev(unsigned long addr, unsigned long delegate_flag, unsigned long iova)
{
	asc_mark_secure_dev(addr, delegate_flag, iova);
	//TODO[Supraja] : remove this later if we definitely don't need to maintain state
	// g->nsp = true; We no longer need this
	return RMI_SUCCESS;
}

unsigned long smc_granule_delegate_dev_1(unsigned long addr)
{
	asc_mark_secure_dev_1(addr);
	return RMI_SUCCESS;
}

unsigned long smc_attach_dev(unsigned long addr)
{
	asc_attach_dev(addr);
	return RMI_SUCCESS;
}

unsigned long smc_granule_undelegate(unsigned long addr)
{
	smc_granule_undelegate_cca_marker();
	struct granule *g;
	g = find_lock_granule(addr, GRANULE_STATE_DELEGATED);
	if (g == NULL) {
		return RMI_ERROR_INPUT;
	}

	asc_mark_nonsecure(addr);
	granule_set_state(g, GRANULE_STATE_NS);

	granule_unlock(g);
	return RMI_SUCCESS;
}
