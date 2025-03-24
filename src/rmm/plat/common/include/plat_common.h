/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
 */

#ifndef PLAT_COMMON_H
#define PLAT_COMMON_H

/* Forward declaration */
struct xlat_mmap_region;

int plat_cmn_setup(unsigned long x0, unsigned long x1,
		   unsigned long x2, unsigned long x3,
		   struct xlat_mmap_region *plat_regions);
int plat_cmn_warmboot_setup(void);

#endif /* PLAT_COMMON_H */
