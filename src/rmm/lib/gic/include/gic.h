/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
 */

#ifndef GIC_H
#define GIC_H

#include <stdbool.h>
#include <utils_def.h>

/* The number of implemented List registers, minus one */
#define ICH_VTR_EL2_LIST_REGS_SHIFT	UL(0)
#define ICH_VTR_EL2_LIST_REGS_WIDTH	UL(5)
#define ICH_VTR_EL2_LIST_REGS_MASK	MASK(ICH_VTR_EL2_LIST_REGS)

/* The number of virtual interrupt identifier bits supported */
#define ICH_VTR_EL2_ID_BITS_SHIFT	UL(23)
#define ICH_VTR_EL2_ID_BITS_WIDTH	UL(3)
#define ICH_VTR_EL2_ID_BITS_MASK	MASK(ICH_VTR_EL2_ID_BITS)

/* The number of virtual preemption bits implemented, minus one */
#define ICH_VTR_EL2_PRE_BITS_SHIFT	UL(26)
#define ICH_VTR_EL2_PRE_BITS_WIDTH	UL(3)
#define ICH_VTR_EL2_PRE_BITS_MASK	MASK(ICH_VTR_EL2_PRE_BITS)

/* The number of virtual priority bits implemented, minus one */
#define ICH_VTR_EL2_PRI_BITS_SHIFT	UL(29)
#define ICH_VTR_EL2_PRI_BITS_WIDTH	UL(3)
#define ICH_VTR_EL2_PRI_BITS_MASK	MASK(ICH_VTR_EL2_PRI_BITS)

/* Global enable bit for the virtual CPU interface */
#define ICH_HCR_EL2_EN_SHIFT		UL(0)
#define ICH_HCR_EL2_EN_BIT		INPLACE(ICH_HCR_EL2_EN, UL(1))

/* Underflow Interrupt Enable */
#define ICH_HCR_EL2_UIE_SHIFT		UL(1)
#define ICH_HCR_EL2_UIE_BIT		INPLACE(ICH_HCR_EL2_UIE, UL(1))

/* List Register Entry Not Present Interrupt Enable */
#define ICH_HCR_EL2_LRENPIE_SHIFT	UL(2)
#define ICH_HCR_EL2_LRENPIE_BIT		INPLACE(ICH_HCR_EL2_LRENPIE, UL(1))

/* No Pending Interrupt Enable */
#define ICH_HCR_EL2_NPIE_SHIFT		UL(3)
#define ICH_HCR_EL2_NPIE_BIT		INPLACE(ICH_HCR_EL2_NPIE, UL(1))

/* VM Group 0 Enabled Interrupt Enable */
#define ICH_HCR_EL2_VGRP0EIE_SHIFT	UL(4)
#define ICH_HCR_EL2_VGRP0EIE_BIT	INPLACE(ICH_HCR_EL2_VGRP0EIE, UL(1))

/* VM Group 0 Disabled Interrupt Enable */
#define ICH_HCR_EL2_VGRP0DIE_SHIFT	UL(5)
#define ICH_HCR_EL2_VGRP0DIE_BIT	INPLACE(ICH_HCR_EL2_VGRP0DIE, UL(1))

/* VM Group 1 Enabled Interrupt Enable */
#define ICH_HCR_EL2_VGRP1EIE_SHIFT	UL(6)
#define ICH_HCR_EL2_VGRP1EIE_BIT	INPLACE(ICH_HCR_EL2_VGRP1EIE, UL(1))

/* VM Group 1 Disabled Interrupt Enable */
#define ICH_HCR_EL2_VGRP1DIE_SHIFT	UL(7)
#define ICH_HCR_EL2_VGRP1DIE_BIT	INPLACE(ICH_HCR_EL2_VGRP1DIE, UL(1))

/*
 * When FEAT_GICv4p1 is implemented:
 * Controls whether deactivation of virtual SGIs
 * can increment ICH_HCR_EL2.EOIcount
 */
#define ICH_HCR_EL2_VSGIEEOICOUNT_SHIFT	UL(8)
#define ICH_HCR_EL2_VSGIEEOICOUNT_BIT	INPLACE(ICH_HCR_EL2_VSGIEEOICOUNT, UL(1))

/*
 * Trap all EL1 accesses to System registers
 * that are common to Group 0 and Group 1 to EL2
 */
#define ICH_HCR_EL2_TC_SHIFT		UL(10)
#define ICH_HCR_EL2_TC_BIT		INPLACE(ICH_HCR_EL2_TC, UL(1))

/*
 * Trap all EL1 accesses to ICC_* and ICV_* System registers
 * for Group 0 interrupts to EL2
 */
#define ICH_HCR_EL2_TALL0_SHIFT		UL(11)
#define ICH_HCR_EL2_TALL0_BIT		INPLACE(ICH_HCR_EL2_TALL0, UL(1))

/*
 * Trap all EL1 accesses to ICC_* and ICV_* System registers
 * for Group 1 interrupts to EL2
 */
#define ICH_HCR_EL2_TALL1_SHIFT		UL(12)
#define ICH_HCR_EL2_TALL1_BIT		INPLACE(ICH_HCR_EL2_TALL1, UL(1))

/* Trap all locally generated SEIs */
#define ICH_HCR_EL2_TSEI_SHIFT		UL(13)
#define ICH_HCR_EL2_TSEI_BIT		INPLACE(ICH_HCR_EL2_TSEI, UL(1))

/*
 * When FEAT_GICv3_TDIR is implemented:
 * Trap EL1 writes to ICC_DIR_EL1 and ICV_DIR_EL1
 */
#define ICH_HCR_EL2_TDIR_SHIFT		UL(14)
#define ICH_HCR_EL2_TDIR_BIT		INPLACE(ICH_HCR_EL2_TDIR, UL(1))

/*
 * When ICH_VTR_EL2.DVIM == 1:
 * Directly-injected Virtual Interrupt Mask
 */
#define ICH_HCR_EL2_DVIM_SHIFT		UL(15)
#define ICH_HCR_EL2_DVIM_BIT		INPLACE(ICH_HCR_EL2_DVIM, UL(1))

#define ICH_HCR_EL2_EOI_COUNT_SHIFT	UL(27)
#define ICH_HCR_EL2_EOI_COUNT_WIDTH	UL(5)
#define ICH_HCR_EL2_EOI_COUNT_MASK	MASK(ICH_HCR_EL2_EOI_COUNT)

/* Virtual INTID of the interrupt */
#define ICH_LR_VINTID_SHIFT		UL(0)
#define ICH_LR_VINTID_WIDTH		UL(32)
#define ICH_LR_VINTID_MASK		MASK(ICH_LR_VINTID)

/*
 * Physical INTID, for hardware interrupts
 * When ICH_LR<n>_EL2.HW is 0 (there is no corresponding physical interrupt),
 * this field has the following meaning:
 *	Bits[44:42] : RES0.
 *	Bit[41] : EOI. If this bit is 1, then when the interrupt identified by
 *		vINTID is deactivated, a maintenance interrupt is asserted.
 *	Bits[40:32] : RES0
 */
#define ICH_LR_PINTID_SHIFT		UL(32)
#define ICH_LR_PINTID_WIDTH		UL(13)
#define ICH_LR_PINTID_MASK		MASK(ICH_LR_PINTID)

#define ICH_LR_EOI_SHIFT		UL(41)
#define ICH_LR_EOI_BIT			INPLACE(ICH_LR_EOI, UL(1))

/* The priority of this interrupt */
#define ICH_LR_PRIORITY_SHIFT		UL(48)
#define ICH_LR_PRIORITY_WIDTH		UL(8)
#define ICH_LR_PRIORITY_MASK		MASK(ICH_LR_PRIORITY)

/* The group for this virtual interrupt */
#define ICH_LR_GROUP_SHIFT		UL(60)
#define ICH_LR_GROUP_BIT		INPLACE(ICH_LR_GROUP, UL(1))

/*
 * Indicates whether this virtual interrupt
 * maps directly to a hardware interrupt
 */
#define ICH_LR_HW_SHIFT			UL(61)
#define ICH_LR_HW_BIT			INPLACE(ICH_LR_HW, UL(1))

/*
 * The state of the interrupt:
 * 0b00	Invalid (Inactive)
 * 0b01	Pending
 * 0b10	Active
 * 0b11	Pending and active
 */
#define ICH_LR_STATE_SHIFT		UL(62)
#define ICH_LR_STATE_WIDTH		UL(2)
#define ICH_LR_STATE_MASK		MASK(ICH_LR_STATE)

#define ICH_LR_STATE_INVALID		INPLACE(ICH_LR_STATE, UL(0))
#define ICH_LR_STATE_PENDING		INPLACE(ICH_LR_STATE, UL(1))
#define ICH_LR_STATE_ACTIVE		INPLACE(ICH_LR_STATE, UL(2))
#define ICH_LR_STATE_PENDING_ACTIVE	INPLACE(ICH_LR_STATE, UL(3))

/*
 * A `_ns` mask defines bits that can be set/cleared by the NS hypervisor.
 */

#define ICH_HCR_EL2_NS_MASK			  \
		(ICH_HCR_EL2_UIE_BIT		| \
		 ICH_HCR_EL2_LRENPIE_BIT	| \
		 ICH_HCR_EL2_NPIE_BIT		| \
		 ICH_HCR_EL2_VGRP0EIE_BIT	| \
		 ICH_HCR_EL2_VGRP0DIE_BIT	| \
		 ICH_HCR_EL2_VGRP1EIE_BIT	| \
		 ICH_HCR_EL2_VGRP1DIE_BIT	| \
		 ICH_HCR_EL2_TDIR_BIT)
/*
 * Maximum number of Interrupt Controller
 * Hyp Active Priorities Group 0/1 Registers [0..3]
 */
#define ICH_MAX_APRS		4

/* Maximum number of Interrupt Controller List Registers */
#define ICH_MAX_LRS		16

/*******************************************************************************
 * GICv3 and 3.1 definitions
 ******************************************************************************/
#define MIN_SGI_ID		U(0)
#define MIN_SEC_SGI_ID		U(8)

/* PPIs INTIDs 16-31 */
#define MIN_PPI_ID		U(16)
#define MAX_PPI_ID		U(31)

/* SPIs INTIDs 32-1019 */
#define MIN_SPI_ID		U(32)
#define MAX_SPI_ID		U(1019)

/* GICv3.1 extended PPIs INTIDs 1056-1119 */
#define MIN_EPPI_ID		U(1056)
#define MAX_EPPI_ID		U(1119)

/* GICv3.1 extended SPIs INTIDs 4096 - 5119 */
#define MIN_ESPI_ID		U(4096)
#define MAX_ESPI_ID		U(5119)

/* Constant to categorize LPI interrupt */
#define MIN_LPI_ID		U(8192)

struct gic_cpu_state {
	/* Interrupt Controller Hyp Active Priorities Group 0 Registers */
	unsigned long ich_ap0r_el2[ICH_MAX_APRS];
	/* Interrupt Controller Hyp Active Priorities Group 1 Registers */
	unsigned long ich_ap1r_el2[ICH_MAX_APRS];
	/* GICv3 Virtual Machine Control Register */
	unsigned long ich_vmcr_el2;		/* RecRun out */
	/* Interrupt Controller Hyp Control Register */
	unsigned long ich_hcr_el2;		/* RecRun in/out */

	/* GICv3 List Registers */
	unsigned long ich_lr_el2[ICH_MAX_LRS];	/* RecRun in/out */
	/* GICv3 Maintenance Interrupt State Register */
	unsigned long ich_misr_el2;		/* RecRun out */
};

struct rmi_rec_entry;
struct rmi_rec_exit;

void gic_get_virt_features(void);
void gic_cpu_state_init(struct gic_cpu_state *gicstate);
void gic_copy_state_from_ns(struct gic_cpu_state *gicstate,
			    struct rmi_rec_entry *rec_entry);
void gic_copy_state_to_ns(struct gic_cpu_state *gicstate,
			  struct rmi_rec_exit *rec_exit);
bool gic_validate_state(struct gic_cpu_state *gicstate);
void gic_restore_state(struct gic_cpu_state *gicstate);
void gic_save_state(struct gic_cpu_state *gicstate);

#endif /* GIC_H */
