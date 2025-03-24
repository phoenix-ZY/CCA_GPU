/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
 */

#ifndef TABLE_H
#define TABLE_H

#include <arch_features.h>
#include <memory.h>

#define MIN_IPA_BITS		32
#define MAX_IPA_BITS		48
#define MAX_IPA_SIZE		(1UL << MAX_IPA_BITS)

#define MIN_STARTING_LEVEL	0
#define RTT_PAGE_LEVEL		3
#define RTT_MIN_BLOCK_LEVEL	2

/* TODO: Fix this when introducing LPA2 support */
COMPILER_ASSERT(MIN_STARTING_LEVEL >= 0);

/* TODO: Allow the NS caller to select the stage 2 starting level */
#define RTT_STARTING_LEVEL	0

/*
 * S2TTE_STRIDE: The number of bits resolved in a single level of translation
 * walk (except for the starting level which may resolve more or fewer bits).
 */
#define S2TTE_STRIDE		(GRANULE_SHIFT - 3)
#define S2TTES_PER_S2TT		(1 << S2TTE_STRIDE)

struct rd;
enum ripas;

unsigned long s2tte_create_ripas(enum ripas ripas);
unsigned long s2tte_create_unassigned(enum ripas ripas);
unsigned long s2tte_create_destroyed(void);
unsigned long s2tte_create_assigned_empty(unsigned long pa, long level);
unsigned long s2tte_create_valid(unsigned long pa, long level);
unsigned long s2tte_create_invalid_ns(void);
unsigned long s2tte_create_valid_ns(unsigned long s2tte, long level);
unsigned long s2tte_create_table(unsigned long pa, long level);

bool host_ns_s2tte_is_valid(unsigned long s2tte, long level);
unsigned long host_ns_s2tte(unsigned long s2tte, long level);

bool s2tte_is_unassigned(unsigned long s2tte);
bool s2tte_is_destroyed(unsigned long s2tte);
bool s2tte_is_assigned(unsigned long s2tte, long level);
bool s2tte_is_valid(unsigned long s2tte, long level);
bool s2tte_is_valid_ns(unsigned long s2tte, long level);
bool s2tte_is_table(unsigned long s2tte, long level);

enum ripas s2tte_get_ripas(unsigned long s2tte);

void s2tt_init_unassigned(unsigned long *s2tt, enum ripas ripas);
void s2tt_init_destroyed(unsigned long *s2tt);
void s2tt_init_assigned_empty(unsigned long *s2tt, unsigned long pa, long level);
void s2tt_init_valid(unsigned long *s2tt, unsigned long pa, long level);
void s2tt_init_valid_ns(unsigned long *s2tt, unsigned long pa, long level);

unsigned long s2tte_pa(unsigned long s2tte, long level);
unsigned long s2tte_pa_table(unsigned long s2tte, long level);
bool addr_is_level_aligned(unsigned long addr, long level);
bool addr_block_intersects_par(struct rd *rd, unsigned long addr, long level);
unsigned long s2tte_map_size(int level);

struct realm_s2_context;
void invalidate_page(const struct realm_s2_context *ctx, unsigned long addr);
void invalidate_block(const struct realm_s2_context *ctx, unsigned long addr);
void invalidate_pages_in_block(const struct realm_s2_context *ctx, unsigned long addr);

bool table_is_unassigned_block(unsigned long *table, enum ripas *ripas);
bool table_is_destroyed_block(unsigned long *table);

bool table_maps_assigned_block(unsigned long *table, long level);
bool table_maps_valid_block(unsigned long *table, long level);
bool table_maps_valid_ns_block(unsigned long *table, long level);

struct rtt_walk {
	struct granule *g_llt;
	unsigned long index;
	long last_level;
};

void rtt_walk_lock_unlock(struct granule *g_root,
			  int start_level,
			  unsigned long ipa_bits,
			  unsigned long map_addr,
			  long level,
			  struct rtt_walk *wi);

/*
 * The MMU is a separate observer, and requires that translation table updates
 * are made with single-copy-atomic stores, necessitating inline assembly. For
 * consistency we use accessors for both reads and writes of translation table
 * entries.
 */
static inline void __tte_write(uint64_t *ttep, uint64_t tte)
{
	SCA_WRITE64(ttep, tte);
	dsb(ish);
}
#define s1tte_write(s1ttep, s1tte)	__tte_write(s1ttep, s1tte)
#define s2tte_write(s2ttep, s2tte)	__tte_write(s2ttep, s2tte)

static inline uint64_t __tte_read(uint64_t *ttep)
{
	return SCA_READ64(ttep);
}
#define s1tte_read(s1ttep)	__tte_read(s1ttep)
#define s2tte_read(s2ttep)	__tte_read(s2ttep)

static inline unsigned int max_ipa_size(void)
{
	return arch_feat_get_pa_width();
}

#endif /* TABLE_H */
