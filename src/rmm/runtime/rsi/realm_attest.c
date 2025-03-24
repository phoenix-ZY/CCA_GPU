/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
 */

#include <attestation.h>
#include <attestation_token.h>
#include <debug.h>
#include <granule.h>
#include <measurement.h>
#include <realm.h>
#include <realm_attest.h>
#include <smc-rsi.h>
#include <smc.h>
#include <string.h>
#include <utils_def.h>

#define MAX_EXTENDED_SIZE		(64U)

/*
 * Return the Realm Personalization Value.
 *
 * Arguments:
 * rd    - The Realm descriptor.
 * claim - The structure to return the Realm Personalization Value claim
 */
static void get_rpv(struct rd *rd, struct q_useful_buf_c *claim)
{
	claim->ptr = (uint8_t *)&(rd->rpv[0]);
	claim->len = RPV_SIZE;
}

/*
 * Save the input parameters in the context for later iterations to check for
 * consistency.
 */
static void save_input_parameters(struct rec *rec)
{
	rec->token_sign_ctx.token_ipa = rec->regs[1];
	(void)memcpy(rec->token_sign_ctx.challenge, &rec->regs[2],
		     ATTEST_CHALLENGE_SIZE);
}

/*
 * Verify that in all the iterations the input parameters are the same
 * as in the initial call.
 */
static bool verify_input_parameters_consistency(struct rec *rec)
{
	return rec->token_sign_ctx.token_ipa == rec->regs[1];
}

/*
 * Function to continue with the sign operation.
 * It returns void as the result will be updated in the
 * struct attest_result passed as argument.
 */
static void attest_token_continue_sign_state(struct rec *rec,
					     struct attest_result *res)
{
	/*
	 * Sign and finish creating the token.
	 */
	enum attest_token_err_t ret =
		attest_realm_token_sign(&(rec->token_sign_ctx.ctx),
					&(rec->rmm_realm_token));

	if ((ret == ATTEST_TOKEN_ERR_COSE_SIGN_IN_PROGRESS) ||
		(ret == ATTEST_TOKEN_ERR_SUCCESS)) {
		/*
		 * Return to RSI handler function after each iteration
		 * to check is there anything else to do (pending IRQ)
		 * or next signing iteration can be executed.
		 */
		res->incomplete = true;
		res->smc_res.x[0] = RSI_INCOMPLETE;

		/* If this was the last signing cycle */
		if (ret == ATTEST_TOKEN_ERR_SUCCESS) {
			rec->token_sign_ctx.state =
				ATTEST_SIGN_TOKEN_WRITE_IN_PROGRESS;
		}
	} else {
		/* Accessible only in case of failure during token signing */
		ERROR("FATAL_ERROR: Realm token creation failed\n");
		panic();
	}
}

/*
 * Function to continue with the token write operation.
 * It returns void as the result will be updated in the
 * struct attest_result passed as argument.
 */
static void attest_token_continue_write_state(struct rec *rec,
					      struct attest_result *res)
{
	struct rd *rd = NULL;
	struct granule *gr;
	uint8_t *realm_att_token;
	unsigned long realm_att_token_ipa = rec->regs[1];
	enum s2_walk_status walk_status;
	struct s2_walk_result walk_res = { 0UL };
	struct q_useful_buf     attest_token_buf;
	size_t    attest_token_len;

	/*
	 * The refcount on rd and rec will protect from any changes
	 * while REC is running.
	 */
	rd = granule_map(rec->realm_info.g_rd, SLOT_RD);

	/*
	 * Translate realm granule IPA to PA. If returns with
	 * WALK_SUCCESS then the last level page table (llt),
	 * which holds the realm_att_token_buf mapping, is locked.
	 */
	walk_status = realm_ipa_to_pa(rd, realm_att_token_ipa, &walk_res);
	buffer_unmap(rd);

	/* Walk parameter validity was checked by RSI_ATTESTATION_TOKEN_INIT */
	assert(walk_status != WALK_INVALID_PARAMS);

	if (walk_status == WALK_FAIL) {
		if (s2_walk_result_match_ripas(&walk_res, RMI_EMPTY)) {
			res->smc_res.x[0] = RSI_ERROR_INPUT;
		} else {
			/*
			 * Translation failed, IPA is not mapped. Return to NS host to
			 * fix the issue.
			 */
			res->walk_result.abort = true;
			res->walk_result.rtt_level = walk_res.rtt_level;
			res->smc_res.x[0] = RSI_INCOMPLETE;
		}
		return;
	}

	/* Map realm data granule to RMM address space */
	gr = find_granule(walk_res.pa);
	realm_att_token = granule_map(gr, SLOT_RSI_CALL);

	attest_token_buf.ptr = realm_att_token;
	attest_token_buf.len = ATTEST_TOKEN_BUFFER_SIZE;

	attest_token_len = attest_cca_token_create(&attest_token_buf,
						   &rec->rmm_realm_token);

	/* Unmap realm granule */
	buffer_unmap(realm_att_token);

	/* Unlock last level page table (walk_res.g_llt) */
	granule_unlock(walk_res.llt);

	/* Write output parameters */
	if (attest_token_len == 0) {
		res->smc_res.x[0] = RSI_ERROR_INPUT;
	} else {
		res->smc_res.x[0] = RSI_SUCCESS;
		res->smc_res.x[1] = attest_token_len;
	}

	/* The signing has either succeeded or failed. Reset the state. */
	rec->token_sign_ctx.state = ATTEST_SIGN_NOT_STARTED;
}

unsigned long handle_rsi_attest_token_init(struct rec *rec)
{
	struct rd *rd = NULL;
	unsigned long ret;
	unsigned long realm_buf_ipa = rec->regs[1];
	struct q_useful_buf rmm_realm_token_buf = {
		rec->rmm_realm_token_buf, sizeof(rec->rmm_realm_token_buf)};
	struct q_useful_buf_c rpv;
	int att_ret;

	assert(rec != NULL);

	/*
	 * Calling RSI_ATTESTATION_TOKEN_INIT any time aborts any ongoing
	 * operation.
	 * TODO: This can be moved to attestation lib
	 */
	if (rec->token_sign_ctx.state != ATTEST_SIGN_NOT_STARTED) {
		int restart;

		rec->token_sign_ctx.state = ATTEST_SIGN_NOT_STARTED;
		restart = attestation_heap_reinit_pe(rec->aux_data.attest_heap_buf,
						      REC_HEAP_PAGES * SZ_4K);
		if (restart != 0) {
			/* There is no provision for this failure so panic */
			panic();
		}
	}

	if (!GRANULE_ALIGNED(realm_buf_ipa)) {
		return RSI_ERROR_INPUT;
	}

	/*
	 * rd lock is acquired so that measurement cannot be updated
	 * simultaneously by another rec
	 */
	granule_lock(rec->realm_info.g_rd, GRANULE_STATE_RD);
	rd = granule_map(rec->realm_info.g_rd, SLOT_RD);
	if (!addr_in_par(rd, realm_buf_ipa)) {
		ret = RSI_ERROR_INPUT;
		goto out_unmap_rd;
	}

	/*
	 * Save the input parameters in the context for later iterations
	 * to check.
	 */
	save_input_parameters(rec);

	get_rpv(rd, &rpv);
	att_ret = attest_realm_token_create(rd->algorithm, rd->measurement,
					    MEASUREMENT_SLOT_NR,
					    &rpv,
					    &rec->token_sign_ctx,
					    &rmm_realm_token_buf);
	if (att_ret != 0) {
		ERROR("FATAL_ERROR: Realm token creation failed,\n");
		panic();
	}

	rec->token_sign_ctx.state = ATTEST_SIGN_IN_PROGRESS;
	ret = RSI_SUCCESS;

out_unmap_rd:
	buffer_unmap(rd);
	granule_unlock(rec->realm_info.g_rd);
	return ret;
}

void attest_realm_token_sign_continue_start(void)
{
	fpu_save_my_state();
}

void attest_realm_token_sign_continue_finish(void)
{
	fpu_restore_my_state();
}

void handle_rsi_attest_token_continue(struct rec *rec,
				      struct attest_result *res)
{
	assert(rec != NULL);
	assert(res != NULL);

	/* Initialize attest_result */
	res->incomplete = false;
	res->walk_result.abort = false;

	if (!verify_input_parameters_consistency(rec)) {
		res->smc_res.x[0] = RSI_ERROR_INPUT;
		return;
	}

	switch (rec->token_sign_ctx.state) {
	case ATTEST_SIGN_NOT_STARTED:
		/*
		 * Before this call the initial attestation token call
		 * (SMC_RSI_ATTEST_TOKEN_INIT) must have been executed
		 * successfully.
		 */
		res->smc_res.x[0] = RSI_ERROR_STATE;
		break;
	case ATTEST_SIGN_IN_PROGRESS:
		attest_token_continue_sign_state(rec, res);
		break;
	case ATTEST_SIGN_TOKEN_WRITE_IN_PROGRESS:
		attest_token_continue_write_state(rec, res);
		break;
	default:
		/* Any other state is considered an error. */
		assert(false);
	}
}

unsigned long handle_rsi_extend_measurement(struct rec *rec)
{
	struct granule *g_rd;
	struct rd *rd;
	unsigned long index;
	unsigned long rd_addr;
	size_t size;
	unsigned long ret;
	void *extend_measurement;
	unsigned char *current_measurement;
	int __unused meas_ret;

	/*
	 * rd lock is acquired so that measurement cannot be updated
	 * simultaneously by another rec
	 */
	rd_addr = granule_addr(rec->realm_info.g_rd);
	g_rd = find_lock_granule(rd_addr, GRANULE_STATE_RD);

	assert(g_rd != NULL);

	rd = granule_map(rec->realm_info.g_rd, SLOT_RD);

	/*
	 * X1:     index
	 * X2:     size
	 * X3-X10: measurement value
	 */
	index = rec->regs[1];

	if ((index == RIM_MEASUREMENT_SLOT) ||
	    (index >= MEASUREMENT_SLOT_NR)) {
		ret = RSI_ERROR_INPUT;
		goto out_unmap_rd;
	}

	size  = rec->regs[2];

	if (size > MAX_EXTENDED_SIZE) {
		ret = RSI_ERROR_INPUT;
		goto out_unmap_rd;
	}

	extend_measurement = &rec->regs[3];
	current_measurement = rd->measurement[index];

	measurement_extend(rd->algorithm,
			   current_measurement,
			   extend_measurement,
			   size,
			   current_measurement);

	ret = RSI_SUCCESS;

out_unmap_rd:
	buffer_unmap(rd);
	granule_unlock(g_rd);
	return ret;
}

unsigned long handle_rsi_read_measurement(struct rec *rec)
{
	struct rd *rd;
	unsigned long idx;
	size_t measurement_size;

	assert(rec != NULL);

	/* X1: Index */
	idx = rec->regs[1];

	if (idx >= MEASUREMENT_SLOT_NR) {
		return RSI_ERROR_INPUT;
	}

	/*
	 * rd lock is acquired so that measurement cannot be updated
	 * simultaneously by another rec
	 */
	granule_lock(rec->realm_info.g_rd, GRANULE_STATE_RD);
	rd = granule_map(rec->realm_info.g_rd, SLOT_RD);

	measurement_size = measurement_get_size(rd->algorithm);

	(void)memcpy(&rec->regs[1], rd->measurement[idx], measurement_size);

	/* Zero-initialize the unused area */
	if (measurement_size < MAX_MEASUREMENT_SIZE) {
		(void)memset((char *)(&rec->regs[1]) + measurement_size,
			     0, MAX_MEASUREMENT_SIZE - measurement_size);
	}

	buffer_unmap(rd);
	granule_unlock(rec->realm_info.g_rd);

	return RSI_SUCCESS;
}
