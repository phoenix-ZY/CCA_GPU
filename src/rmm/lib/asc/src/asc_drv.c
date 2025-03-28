/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
 */

#include <assert.h>
#include <smc.h>

void asc_mark_secure(unsigned long addr)
{
	__unused int ret;

	ret = monitor_call(SMC_ASC_MARK_SECURE, addr, 0, 0, 0, 0, 0);
	assert(ret == 0);
}

void asc_mark_nonsecure(unsigned long addr)
{
	__unused int ret;

	ret = monitor_call(SMC_ASC_MARK_NONSECURE, addr, 0, 0, 0, 0, 0);
	assert(ret == 0);
}

void asc_mark_secure_dev(unsigned long addr, unsigned long delegate_flag, unsigned long iova)
{
	__unused int ret;

	ret = monitor_call(SMC_ASC_MARK_SECURE_DEV, addr, delegate_flag, iova, 0, 0, 0);
	assert(ret == 0);
}

void asc_mark_secure_dev_1(unsigned long addr)
{
	__unused int ret;
	ret = monitor_call(SMC_ASC_MARK_SECURE_DEV_1, addr, 0, 0, 0);
	assert(ret == 0);
}

void asc_attach_dev(unsigned long addr)
{
	__unused int ret;

	ret = monitor_call(SMC_ASC_ATTACH_DEV, addr, 0, 0, 0, 0, 0);
	assert(ret == 0);
}
