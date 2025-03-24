/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
 */

#ifndef RSI_WALK_H
#define RSI_WALK_H

struct smc_result;

struct rsi_walk_result {
	/*
	 * If true, RTT walk failed due to missing PTE at level @rtt_level.
	 *
	 * If false, @smc_result contains GPR values to be returned to the
	 * Realm.
	 */
	bool abort;
	unsigned long rtt_level;
};

struct rsi_walk_smc_result {
	/* Result of RTT walk performed by RSI command */
	struct rsi_walk_result walk_result;

	/*
	 * If @walk_result.abort is false, @smc_res contains GPR values to be
	 * returned to the Realm.
	 */
	struct smc_result smc_res;
};

#endif /* RSI_WALK_H */
