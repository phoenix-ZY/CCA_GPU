/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
 */

#ifndef SPINLOCK_H
#define SPINLOCK_H

/*
 * A trivial spinlock implementation, per ARM DDI 0487D.a, section K11.3.4.
 */

typedef struct {
	unsigned int val;
} spinlock_t;

static inline void spinlock_acquire(spinlock_t *l)
{
	unsigned int tmp;

	asm volatile(
	"	sevl\n"
	"	prfm	pstl1keep, %[lock]\n"
	"1:\n"
	"	wfe\n"
	"	ldaxr	%w[tmp], %[lock]\n"
	"	cbnz	%w[tmp], 1b\n"
	"	stxr	%w[tmp], %w[one], %[lock]\n"
	"	cbnz	%w[tmp], 1b\n"
	: [lock] "+Q" (l->val),
	  [tmp] "=&r" (tmp)
	: [one] "r" (1)
	: "memory"
	);
}

static inline void spinlock_release(spinlock_t *l)
{
	asm volatile(
	"	stlr	wzr, %[lock]\n"
	: [lock] "+Q" (l->val)
	:
	: "memory"
	);
}

#endif /* SPINLOCK_H */
