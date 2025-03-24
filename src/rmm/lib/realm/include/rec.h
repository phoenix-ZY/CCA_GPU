/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
 */

#ifndef REC_H
#define REC_H

#ifndef __ASSEMBLER__

#include <arch.h>
#include <attestation_token.h>
#include <fpu_helpers.h>
#include <gic.h>
#include <memory_alloc.h>
#include <ripas.h>
#include <sizes.h>
#include <smc-rmi.h>
#include <utils_def.h>

struct granule;

/*
 * System registers whose contents are specific to a REC.
 */
struct sysreg_state {
	unsigned long sp_el0;
	unsigned long sp_el1;
	unsigned long elr_el1;
	unsigned long spsr_el1;
	unsigned long pmcr_el0;
	unsigned long pmuserenr_el0;
	unsigned long tpidrro_el0;
	unsigned long tpidr_el0;
	unsigned long csselr_el1;
	unsigned long sctlr_el1;
	unsigned long actlr_el1;
	unsigned long cpacr_el1;
	unsigned long zcr_el1;
	unsigned long ttbr0_el1;
	unsigned long ttbr1_el1;
	unsigned long tcr_el1;
	unsigned long esr_el1;
	unsigned long afsr0_el1;
	unsigned long afsr1_el1;
	unsigned long far_el1;
	unsigned long mair_el1;
	unsigned long vbar_el1;
	unsigned long contextidr_el1;
	unsigned long tpidr_el1;
	unsigned long amair_el1;
	unsigned long cntkctl_el1;
	unsigned long par_el1;
	unsigned long mdscr_el1;
	unsigned long mdccint_el1;
	unsigned long disr_el1;
	unsigned long mpam0_el1;

	/* Timer Registers */
	unsigned long cnthctl_el2;
	unsigned long cntvoff_el2;
	unsigned long cntpoff_el2;
	unsigned long cntp_ctl_el0;
	unsigned long cntp_cval_el0;
	unsigned long cntv_ctl_el0;
	unsigned long cntv_cval_el0;

	/* GIC Registers */
	struct gic_cpu_state gicstate;

	/* TODO MPAM */
	/* TODO Performance Monitor Registers */
	/* TODO Pointer Authentication Registers */

	unsigned long vmpidr_el2;	/* restored only */
	unsigned long hcr_el2;		/* restored only */
};

/*
 * System registers whose contents are
 * common across all RECs in a Realm.
 */
struct common_sysreg_state {
	unsigned long vttbr_el2;
	unsigned long vtcr_el2;
	unsigned long hcr_el2;
};

/*
 * This structure is aligned on cache line size to avoid cache line trashing
 * when allocated as an array for N CPUs.
 */
struct ns_state {
	struct sysreg_state sysregs;
	unsigned long sp_el0;
	unsigned long icc_sre_el2;
	struct fpu_state *fpu; /* FPU/SVE saved lazily. */
	struct sve_state *sve;
} __attribute__((aligned(CACHE_WRITEBACK_GRANULE)));

/*
 * This structure contains pointers to data that is allocated
 * in auxilary granules.
 */
struct rec_aux_data {
	uint8_t *attest_heap_buf; /* Pointer to the heap buffer of this REC. */
};

/* This structure is used for storing FPU/SIMD context for realm. */
struct rec_fpu_context {
	struct fpu_state fpu;
	bool used;
};

struct rec {
	struct granule *g_rec; /* the granule in which this rec lives */
	unsigned long rec_idx; /* Which rec is this */
	bool runnable;

	unsigned long regs[31];
	unsigned long pc;
	unsigned long pstate;

	struct sysreg_state sysregs;
	struct common_sysreg_state common_sysregs;

	struct {
		unsigned long start;
		unsigned long end;
		unsigned long addr;
		enum ripas ripas;
	} set_ripas;

	/*
	 * Common values across all RECs in a Realm.
	 */
	struct {
		unsigned long ipa_bits;
		int s2_starting_level;
		struct granule *g_rtt;
		struct granule *g_rd;
	} realm_info;

	struct {
		/*
		 * The contents of the *_EL2 system registers at the last time
		 * the REC exited to the host due to a synchronous exception.
		 * These are the unsanitized register values which may differ
		 * from the value returned to the host in rec_exit structure.
		 */
		unsigned long esr;
		unsigned long hpfar;
		unsigned long far;
	} last_run_info;

	/* Structure for storing FPU/SIMD context for realm. */
	struct rec_fpu_context fpu_ctx;

	/* Pointer to per-cpu non-secure state */
	struct ns_state *ns;

	struct {
		/*
		 * Set to 'true' when there is a pending PSCI
		 * command that must be resolved by the host.
		 * The command is encoded in rec->regs[0].
		 *
		 * A REC with pending PSCI is not schedulable.
		 */
		bool pending;
	} psci_info;

	/* Number of auxiliary granules */
	unsigned int num_rec_aux;

	/* Addresses of auxiliary granules */
	struct granule *g_aux[MAX_REC_AUX_GRANULES];
	struct rec_aux_data aux_data;

	unsigned char rmm_realm_token_buf[SZ_1K];
	struct q_useful_buf_c rmm_realm_token;

	struct token_sign_ctx token_sign_ctx;

	/* Buffer allocation info used for heap init and management */
	struct {
		struct buffer_alloc_ctx ctx;
		bool ctx_initialised;
	} alloc_info;

	struct {
		unsigned long vsesr_el2;
		bool inject;
	} serror_info;

	/* True if host call is pending */
	bool host_call;
};
COMPILER_ASSERT(sizeof(struct rec) <= GRANULE_SIZE);

/*
 * Check that mpidr has a valid value with all fields except
 * Aff3[39:32]:Aff2[23:16]:Aff1[15:8]:Aff0[3:0] set to 0.
 */
static inline bool mpidr_is_valid(unsigned long mpidr)
{
	return (mpidr & ~(MASK(MPIDR_EL2_AFF0) |
			  MASK(MPIDR_EL2_AFF1) |
			  MASK(MPIDR_EL2_AFF2) |
			  MASK(MPIDR_EL2_AFF3))) == 0ULL;
}

/*
 * Calculate REC index from mpidr value.
 * index = Aff3[39:32]:Aff2[23:16]:Aff1[15:8]:Aff0[3:0]
 */
static inline unsigned long mpidr_to_rec_idx(unsigned long mpidr)
{
	return (MPIDR_EL2_AFF(0, mpidr) +
		MPIDR_EL2_AFF(1, mpidr) +
		MPIDR_EL2_AFF(2, mpidr) +
		MPIDR_EL2_AFF(3, mpidr));
}

void rec_run_loop(struct rec *rec, struct rmi_rec_exit *rec_exit);

unsigned long smc_rec_create(unsigned long rec_addr,
			     unsigned long rd_addr,
			     unsigned long rec_params_addr);

unsigned long smc_rec_destroy(unsigned long rec_addr);

unsigned long smc_rec_enter(unsigned long rec_addr,
			    unsigned long rec_run_addr);

void inject_serror(struct rec *rec, unsigned long vsesr);

void emulate_stage2_data_abort(struct rec *rec, struct rmi_rec_exit *exit,
			       unsigned long rtt_level);

#endif /* __ASSEMBLER__ */

#endif /* REC_H */
