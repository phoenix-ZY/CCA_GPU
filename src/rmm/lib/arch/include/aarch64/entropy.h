/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
 */

#ifndef ENTROPY_H
#define ENTROPY_H

#include <arch.h>
#include <utils_def.h>

/*
 * Write 8 bytes of random data in random. Returns true on success, false on
 * failure.
 */
static inline bool arch_collect_entropy(uint64_t *random)
{
	unsigned long rc;
	uint64_t val;

	asm volatile(
	"	mrs  %[val], " __XSTRING(RNDR) "\n"
	"	str  %[val], %[random_ptr]\n"
	"	cset %[rc], ne\n" /* RNDR sets NZCV to 0b0100 on failure */
	: [random_ptr] "=m" (*random),
	  [rc] "=r" (rc),
	  [val] "=r" (val)
	:
	: "cc"
	);
	return (rc == 1);
}

#endif /* ENTROPY_H */
