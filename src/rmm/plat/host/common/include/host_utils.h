/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
 */

#ifndef HOST_UTILS_H
#define HOST_UTILS_H

#include <types.h>

/***********************************************************************
 * Utility functions to be used across different host platform variants.
 **********************************************************************/

/* Maximum number of sysregs for which we can install callbacks */
#define SYSREG_MAX_CBS		(10U)

/* Maximum size allowed for a sysreg name */
#define MAX_SYSREG_NAME_LEN	(25U)

/*
 * Callback prototype invoked when a sysreg is read.
 *
 * Arguments:
 *	reg - Pointer to the emulated register
 *
 * Returns:
 *	Value read from the emulated sysreg
 */
typedef u_register_t (*rd_cb_t)(u_register_t *reg);

/*
 * Callback prototype invoked when a sysreg is written.
 *
 * Arguments:
 *	val - Value to be written to the sysreg
 *	reg - Pointer to the emulated sysreg
 *
 * Returns:
 *	Void
 */
typedef void (*wr_cb_t)(u_register_t val, u_register_t *reg);

/*
 * Structure to hold the callback pointers and value of the emulated sysreg.
 */
struct sysreg_cb {
	char sysreg[MAX_SYSREG_NAME_LEN + 1U];
	rd_cb_t rd_cb;
	wr_cb_t wr_cb;
	u_register_t value;
};

/*
 * Return the callbacks for a given sysreg or NULL
 * if no callbacks are found.
 */
struct sysreg_cb *host_util_get_sysreg_cb(char *name);

/*
 * Setup callbacks for sysreg read and write operations.
 *
 * This API allows to setup callbacks for each sysreg to be called upon
 * read or write operations. This allows to control what to return on
 * a read or how to process a write.
 *
 * Argsuments:
 *	name - String containing the name of the sysreg. The name of
 *	       the sysreg cannot exceed MAX_SYSREG_NAME_LEN (excluding
 *	       the terminating null character) or it will be truncated.
 *	rd_cb - Callback to be invoked on a read operation.
 *	wr_cb - Callback to be invoked on a write operation.
 *	init - Value used as reset value for the sysreg.
 *
 * Returns:
 *	0 on success or a negative error code otherwise.
 */
int host_util_set_sysreg_cb(char *name, rd_cb_t rd_cb, wr_cb_t wr_cb,
			    u_register_t init);

/*
 * Setup generic callbacks for sysreg read and write operations.
 *
 * This API allows to setup generic callbacks for each sysreg to be called upon
 * read or write operations.
 *
 * Arguments:
 *	name - String containing the name of the sysreg. The name of
 *	       the sysreg cannot exceed MAX_SYSREG_NAME_LEN (excluding
 *	       the terminating null character) or it will be truncated.
 *	init - Value used as reset value for the sysreg.
 *
 * Returns:
 *	0 on success or a negative error code otherwise.
 */
int host_util_set_default_sysreg_cb(char *name, u_register_t init);

/*
 * Clear the list of sysreg callbacks.
 */
void host_util_reset_all_sysreg_cb(void);

/*
 * Return the configured address for the granule base.
 */
unsigned long host_util_get_granule_base(void);

#endif /* HOST_UTILS_H */
