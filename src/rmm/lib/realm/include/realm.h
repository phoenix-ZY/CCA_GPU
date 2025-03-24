/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
 */

#ifndef REALM_H
#define REALM_H

#include <assert.h>
#include <measurement.h>
#include <memory.h>
#include <rec.h>
#include <table.h>

#define REALM_STATE_NEW		0
#define REALM_STATE_ACTIVE	1
#define REALM_STATE_SYSTEM_OFF	2

/*
 * Stage 2 configuration of the Realm
 */
struct realm_s2_context {
	/* Number of IPA bits */
	unsigned int ipa_bits;

	/* Starting level of the stage 2 translation */
	int s2_starting_level;

	/* Number of concatenated starting level rtts */
	unsigned int num_root_rtts;

	/* First level RTT, pointed to by Realm TTBR */
	struct granule *g_rtt;

	/* Virtual Machine Identifier */
	unsigned int vmid;

	/*
	 * TODO: we will need other translation regime state, e.g. TCR, MAIR(?).
	 */
};

/* struct rd is protected by the rd granule lock */
struct rd {
	/*
	 * 'state' & 'rec_count' are only accessed through dedicated
	 * primitives where the following rules apply:
	 *
	 * (1) To write the value, the RMI handler must hold the rd granule
	 *     lock and use a single copy atomic store with release semantics.
	 *
	 * (2) To read the value, the RMI handler must either:
	 *     - Hold the rd granule lock and use a 64-bit single copy
	 *       atomic load, or
	 *     - Hold the rd reference count and use a 64-bit single copy
	 *       atomic load with acquire semantics.
	 *
	 * Other members of the structure are accessed with rd granule lock held.
	 */
	/* 64-bit variable accessed with READ64/WRITE64/ACQUIRE semantic */
	unsigned long state;

	/* Reference count */
	unsigned long rec_count;

	/* Stage 2 configuration of the Realm */
	struct realm_s2_context s2_ctx;

	/* Number of auxiliary REC granules for the Realm */
	unsigned int num_rec_aux;

	/* Algorithm to use for measurements */
	enum hash_algo algorithm;

	/* Realm measurement */
	unsigned char measurement[MEASUREMENT_SLOT_NR][MAX_MEASUREMENT_SIZE];

	/* Realm Personalization Value */
	unsigned char rpv[RPV_SIZE];
};
COMPILER_ASSERT(sizeof(struct rd) <= GRANULE_SIZE);

/*
 * Sets the rd's state while holding the rd granule lock.
 */
static inline void set_rd_state(struct rd *rd, unsigned long state)
{
	SCA_WRITE64_RELEASE(&rd->state, state);
}

/*
 * Gets the rd's state while holding the rd granule lock.
 */
static inline unsigned long get_rd_state_locked(struct rd *rd)
{
	return SCA_READ64(&rd->state);
}

/*
 * Gets the rd's state while holding the rd's reference count, without
 * holding the rd granule lock.
 */
static inline unsigned long get_rd_state_unlocked(struct rd *rd)
{
	return SCA_READ64_ACQUIRE(&rd->state);
}

/*
 * Sets the rd's rec_count while holding the rd granule lock.
 */
static inline void set_rd_rec_count(struct rd *rd, unsigned long val)
{
	SCA_WRITE64_RELEASE(&rd->rec_count, val);
}

/*
 * Gets the rd's rec_count while holding the rd granule lock.
 */
static inline unsigned long get_rd_rec_count_locked(struct rd *rd)
{
	return SCA_READ64(&rd->rec_count);
}

/*
 * Gets the rd's rec_count while holding the rd's reference count, without
 * holding the rd granule lock.
 */
static inline unsigned long get_rd_rec_count_unlocked(struct rd *rd)
{
	return SCA_READ64_ACQUIRE(&rd->rec_count);
}

static inline unsigned long realm_ipa_bits(struct rd *rd)
{
	return rd->s2_ctx.ipa_bits;
}

/*
 * Gets the rd's IPA size.
 */
static inline unsigned long realm_ipa_size(struct rd *rd)
{
	return (1UL << realm_ipa_bits(rd));
}

static inline unsigned long realm_par_size(struct rd *rd)
{
	return (realm_ipa_size(rd) / 2U);
}

static inline int realm_rtt_starting_level(struct rd *rd)
{
	return rd->s2_ctx.s2_starting_level;
}

/*
 * Checks that 'address' is within container's parameters.
 *
 * 'container_base' is the start address of the container.
 * 'container_end' is the first address after the container.
 * The container must not overflow.
 */
static inline bool addr_is_contained(unsigned long container_base,
				     unsigned long container_end,
				     unsigned long address)
{
	assert(container_base <= (container_end - 1));
	return address >= container_base && address <= (container_end - 1);
}

/*
 * Checks that region is within container's parameters.
 *
 * 'container_base' is the start address of the container.
 * 'container_end' is the first address after the container.
 * The container must not overflow.
 * 'region_base' is the start address of the region.
 * 'region_end' is the first address after the region.
 * The region must not overflow.
 */
static inline bool region_is_contained(unsigned long container_base,
				       unsigned long container_end,
				       unsigned long region_base,
				       unsigned long region_end)
{
	assert(region_base <= (region_end - 1UL));
	return addr_is_contained(container_base, container_end, region_base) &&
	       addr_is_contained(container_base, container_end, region_end - 1UL);
}

static inline unsigned long rec_ipa_size(struct rec *rec)
{
	return (1UL << rec->realm_info.ipa_bits);
}

static inline unsigned long rec_par_size(struct rec *rec)
{
	return (rec_ipa_size(rec) / 2U);
}

static inline bool addr_in_rec_par(struct rec *rec, unsigned long addr)
{
	return (addr < rec_par_size(rec));
}

static inline bool region_in_rec_par(struct rec *rec,
				     unsigned long base, unsigned long end)
{
	return region_is_contained(0UL, rec_par_size(rec), base, end);
}

static inline bool addr_in_par(struct rd *rd, unsigned long addr)
{
	return (addr < realm_par_size(rd));
}

enum s2_walk_status {
	/* Successful translation */
	WALK_SUCCESS,
	/* Parameter 'ipa' is unaligned or is not Protected IPA */
	WALK_INVALID_PARAMS,
	/* Mapping is not in the page table */
	WALK_FAIL
};

struct s2_walk_result {
	unsigned long pa;
	unsigned long rtt_level;
	enum ripas ripas;
	bool destroyed;
	struct granule *llt;
};

static inline bool s2_walk_result_match_ripas(struct s2_walk_result *res,
					      enum ripas ripas)
{
	return (!res->destroyed && (res->ripas == ripas));
}

enum s2_walk_status realm_ipa_to_pa(struct rd *rd,
				    unsigned long ipa,
				    struct s2_walk_result *res);

enum s2_walk_status realm_ipa_get_ripas(struct rec *rec, unsigned long ipa,
					enum ripas *ripas_ptr,
					unsigned long *rtt_level);
#endif /* REALM_H */
