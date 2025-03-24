/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
 */

#include <buffer.h>
#include <host_harness.h>

void *host_buffer_arch_map(enum buffer_slot slot,
			    unsigned long addr, bool ns)
{
	(void)slot;
	(void)ns;

	return (void *)addr;
}

void host_buffer_arch_unmap(void *buf)
{
	(void)buf;
}
