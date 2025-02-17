/*
 * Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <mach/rpm-regulator.h>
#include <mach/msm_bus_board.h>
#include <mach/msm_bus.h>

#include "mach/socinfo.h"
#include "acpuclock.h"
#include "acpuclock-krait.h"

#ifdef CONFIG_LGE_PM
#include <mach/board_lge.h>
#endif

static struct hfpll_data hfpll_data __initdata = {
	.mode_offset = 0x00,
	.l_offset = 0x08,
	.m_offset = 0x0C,
	.n_offset = 0x10,
	.config_offset = 0x04,
	.config_val = 0x7845C665,
	.has_droop_ctl = true,
	.droop_offset = 0x14,
	.droop_val = 0x0108C000,
	.low_vdd_l_max = 22,
	.nom_vdd_l_max = 42,
	.vdd[HFPLL_VDD_NONE] =       0,
	.vdd[HFPLL_VDD_LOW]  =  945000,
	.vdd[HFPLL_VDD_NOM]  = 1050000,
	.vdd[HFPLL_VDD_HIGH] = 1150000,
};

static struct scalable scalable[] __initdata = {
	[CPU0] = {
		.hfpll_phys_base = 0x00903200,
		.aux_clk_sel_phys = 0x02088014,
		.aux_clk_sel = 3,
		.sec_clk_sel = 2,
		.l2cpmr_iaddr = 0x4501,
		.vreg[VREG_CORE] = { "krait0", 1200000 },
		.vreg[VREG_MEM]  = { "krait0_mem", 1150000 },
		.vreg[VREG_DIG]  = { "krait0_dig", 1150000 },
		.vreg[VREG_HFPLL_A] = { "krait0_hfpll", 1800000 },
	},
	[CPU1] = {
		.hfpll_phys_base = 0x00903240,
		.aux_clk_sel_phys = 0x02098014,
		.aux_clk_sel = 3,
		.sec_clk_sel = 2,
		.l2cpmr_iaddr = 0x5501,
		.vreg[VREG_CORE] = { "krait1", 1200000 },
		.vreg[VREG_MEM]  = { "krait1_mem", 1150000 },
		.vreg[VREG_DIG]  = { "krait1_dig", 1150000 },
		.vreg[VREG_HFPLL_A] = { "krait1_hfpll", 1800000 },
	},
	[CPU2] = {
		.hfpll_phys_base = 0x00903280,
		.aux_clk_sel_phys = 0x020A8014,
		.aux_clk_sel = 3,
		.sec_clk_sel = 2,
		.l2cpmr_iaddr = 0x6501,
		.vreg[VREG_CORE] = { "krait2", 1200000 },
		.vreg[VREG_MEM]  = { "krait2_mem", 1150000 },
		.vreg[VREG_DIG]  = { "krait2_dig", 1150000 },
		.vreg[VREG_HFPLL_A] = { "krait2_hfpll", 1800000 },
	},
	[CPU3] = {
		.hfpll_phys_base = 0x009032C0,
		.aux_clk_sel_phys = 0x020B8014,
		.aux_clk_sel = 3,
		.sec_clk_sel = 2,
		.l2cpmr_iaddr = 0x7501,
		.vreg[VREG_CORE] = { "krait3", 1200000 },
		.vreg[VREG_MEM]  = { "krait3_mem", 1150000 },
		.vreg[VREG_DIG]  = { "krait3_dig", 1150000 },
		.vreg[VREG_HFPLL_A] = { "krait3_hfpll", 1800000 },
	},
	[L2] = {
		.hfpll_phys_base = 0x00903300,
		.aux_clk_sel_phys = 0x02011028,
		.aux_clk_sel = 3,
		.sec_clk_sel = 2,
		.l2cpmr_iaddr = 0x0500,
		.vreg[VREG_HFPLL_A] = { "l2_hfpll", 1800000 },
	},
};

/*
 * The correct maximum rate for 8064ab in 600 MHZ.
 * We rely on the RPM rounding requests up here.
*/
static struct msm_bus_paths bw_level_tbl[] __initdata = {
	[0] =  BW_MBPS(640), /* At least  80 MHz on bus. */
	[1] = BW_MBPS(1064), /* At least 133 MHz on bus. */
	[2] = BW_MBPS(1600), /* At least 200 MHz on bus. */
	[3] = BW_MBPS(2128), /* At least 266 MHz on bus. */
	[4] = BW_MBPS(3200), /* At least 400 MHz on bus. */
	[5] = BW_MBPS(4264), /* At least 533 MHz on bus. */
};

static struct msm_bus_scale_pdata bus_scale_data __initdata = {
	.usecase = bw_level_tbl,
	.num_usecases = ARRAY_SIZE(bw_level_tbl),
	.active_only = 1,
	.name = "acpuclk-8064",
};

static struct l2_level l2_freq_tbl[] __initdata = {
	[0]  = { {  384000, PLL_8, 0, 0x00 },  1050000, 1050000, 1 },
	[1]  = { {  432000, HFPLL, 2, 0x20 }, 1050000, 1050000, 2 },
	[2]  = { {  486000, HFPLL, 2, 0x24 }, 1050000, 1050000, 2 },
	[3]  = { {  540000, HFPLL, 2, 0x28 }, 1050000, 1050000, 2 },
	[4]  = { {  594000, HFPLL, 1, 0x16 }, 1050000, 1050000, 2 },
	[5]  = { {  648000, HFPLL, 1, 0x18 }, 1050000, 1050000, 4 },
	[6]  = { {  702000, HFPLL, 1, 0x1A }, 1150000, 1150000, 4 },
	[7]  = { {  756000, HFPLL, 1, 0x1C }, 1150000, 1150000, 4 },
	[8]  = { {  810000, HFPLL, 1, 0x1E }, 1150000, 1150000, 4 },
	[9]  = { {  864000, HFPLL, 1, 0x20 }, 1150000, 1150000, 4 },
	[10] = { {  918000, HFPLL, 1, 0x22 }, 1150000, 1150000, 5 },
	[11] = { {  972000, HFPLL, 1, 0x24 }, 1150000, 1150000, 5 },
	[12] = { { 1026000, HFPLL, 1, 0x26 }, 1150000, 1150000, 5 },
	[13] = { { 1080000, HFPLL, 1, 0x28 }, 1150000, 1150000, 5 },
	[14] = { { 1134000, HFPLL, 1, 0x2A }, 1150000, 1150000, 5 },
	[15] = { { 1188000, HFPLL, 1, 0x2C }, 1150000, 1150000, 5 },
	{ }
};

#ifdef CONFIG_LGE_PM
#if defined(CONFIG_MACH_APQ8064_J1KD) || defined(CONFIG_MACH_APQ8064_J1D)
static struct acpu_level tbl_slow_factory_56k[] __initdata = {
	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0),   975000 },
	{ 0, {   432000, HFPLL, 2, 0x20 }, L2(6),  1000000 },
	{ 1, {   486000, HFPLL, 2, 0x24 }, L2(6),  1000000 },
	{ 0, {   540000, HFPLL, 2, 0x28 }, L2(6),  1000000 },
	{ 1, {   594000, HFPLL, 1, 0x16 }, L2(6),  1000000 },
	{ 0, {   648000, HFPLL, 1, 0x18 }, L2(6),  1025000 },
	{ 1, {   702000, HFPLL, 1, 0x1A }, L2(6),  1025000 },
	{ 0, { 0 } }
};

static struct acpu_level tbl_nom_factory_56k[] __initdata = {
	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0),   925000 },
	{ 0, {   432000, HFPLL, 2, 0x20 }, L2(6),   950000 },
	{ 1, {   486000, HFPLL, 2, 0x24 }, L2(6),   950000 },
	{ 0, {   540000, HFPLL, 2, 0x28 }, L2(6),   975000 },
	{ 1, {   594000, HFPLL, 1, 0x16 }, L2(6),   975000 },
	{ 0, {   648000, HFPLL, 1, 0x18 }, L2(6),   975000 },
	{ 1, {   702000, HFPLL, 1, 0x1A }, L2(6),   975000 },
	{ 0, { 0 } }
};

static struct acpu_level tbl_fast_factory_56k[] __initdata = {
	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0),   875000 },
	{ 0, {   432000, HFPLL, 2, 0x20 }, L2(6),   900000 },
	{ 1, {   486000, HFPLL, 2, 0x24 }, L2(6),   900000 },
	{ 0, {   540000, HFPLL, 2, 0x28 }, L2(6),   925000 },
	{ 1, {   594000, HFPLL, 1, 0x16 }, L2(6),   925000 },
	{ 0, {   648000, HFPLL, 1, 0x18 }, L2(6),   950000 },
	{ 1, {   702000, HFPLL, 1, 0x1A }, L2(6),   950000 },
	{ 0, { 0 } }
};
#endif

static struct acpu_level tbl_slow_factory_1026[] __initdata = {
	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0),   975000 },
	{ 0, {   432000, HFPLL, 2, 0x20 }, L2(6),  1000000 },
	{ 1, {   486000, HFPLL, 2, 0x24 }, L2(6),  1000000 },
	{ 0, {   540000, HFPLL, 2, 0x28 }, L2(6),  1000000 },
	{ 1, {   594000, HFPLL, 1, 0x16 }, L2(6),  1000000 },
	{ 0, {   648000, HFPLL, 1, 0x18 }, L2(6),  1025000 },
	{ 1, {   702000, HFPLL, 1, 0x1A }, L2(6),  1025000 },
	{ 0, {   756000, HFPLL, 1, 0x1C }, L2(6),  1075000 },
	{ 1, {   810000, HFPLL, 1, 0x1E }, L2(6),  1075000 },
	{ 0, {   864000, HFPLL, 1, 0x20 }, L2(6),  1100000 },
	{ 1, {   918000, HFPLL, 1, 0x22 }, L2(6),  1100000 },
	{ 0, {   972000, HFPLL, 1, 0x24 }, L2(6),  1125000 },
	{ 1, {  1026000, HFPLL, 1, 0x26 }, L2(6),  1125000 },
	{ 0, { 0 } }
};

static struct acpu_level tbl_nom_factory_1026[] __initdata = {
	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0),   925000 },
	{ 0, {   432000, HFPLL, 2, 0x20 }, L2(6),   950000 },
	{ 1, {   486000, HFPLL, 2, 0x24 }, L2(6),   950000 },
	{ 0, {   540000, HFPLL, 2, 0x28 }, L2(6),   975000 },
	{ 1, {   594000, HFPLL, 1, 0x16 }, L2(6),   975000 },
	{ 0, {   648000, HFPLL, 1, 0x18 }, L2(6),   975000 },
	{ 1, {   702000, HFPLL, 1, 0x1A }, L2(6),   975000 },
	{ 0, {   756000, HFPLL, 1, 0x1C }, L2(6),  1025000 },
	{ 1, {   810000, HFPLL, 1, 0x1E }, L2(6),  1025000 },
	{ 0, {   864000, HFPLL, 1, 0x20 }, L2(6),  1050000 },
	{ 1, {   918000, HFPLL, 1, 0x22 }, L2(6),  1050000 },
	{ 0, {   972000, HFPLL, 1, 0x24 }, L2(6),  1075000 },
	{ 1, {  1026000, HFPLL, 1, 0x26 }, L2(6),  1075000 },
	{ 0, { 0 } }
};

static struct acpu_level tbl_fast_factory_1026[] __initdata = {
	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0),   875000 },
	{ 0, {   432000, HFPLL, 2, 0x20 }, L2(6),   900000 },
	{ 1, {   486000, HFPLL, 2, 0x24 }, L2(6),   900000 },
	{ 0, {   540000, HFPLL, 2, 0x28 }, L2(6),   925000 },
	{ 1, {   594000, HFPLL, 1, 0x16 }, L2(6),   925000 },
	{ 0, {   648000, HFPLL, 1, 0x18 }, L2(6),   950000 },
	{ 1, {   702000, HFPLL, 1, 0x1A }, L2(6),   950000 },
	{ 0, {   756000, HFPLL, 1, 0x1C }, L2(6),   975000 },
	{ 1, {   810000, HFPLL, 1, 0x1E }, L2(6),   975000 },
	{ 0, {   864000, HFPLL, 1, 0x20 }, L2(6),  1000000 },
	{ 1, {   918000, HFPLL, 1, 0x22 }, L2(6),  1000000 },
	{ 0, {   972000, HFPLL, 1, 0x24 }, L2(6),  1025000 },
	{ 1, {  1026000, HFPLL, 1, 0x26 }, L2(6),  1025000 },
	{ 0, { 0 } }
};

#if defined(CONFIG_MACH_APQ8064_GK_KR)||defined(CONFIG_MACH_APQ8064_GKATT)||defined(CONFIG_MACH_APQ8064_GVDCM)||defined(CONFIG_MACH_APQ8064_GV_KR)||defined(CONFIG_MACH_APQ8064_GKGLOBAL)
static struct acpu_level tbl_slow_factory_1134[] __initdata = {
	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0),   975000 },
	{ 0, {   432000, HFPLL, 2, 0x20 }, L2(6),  1000000 },
	{ 1, {   486000, HFPLL, 2, 0x24 }, L2(6),  1000000 },
	{ 0, {   540000, HFPLL, 2, 0x28 }, L2(6),  1000000 },
	{ 1, {   594000, HFPLL, 1, 0x16 }, L2(6),  1000000 },
	{ 0, {   648000, HFPLL, 1, 0x18 }, L2(6),  1025000 },
	{ 1, {   702000, HFPLL, 1, 0x1A }, L2(6),  1025000 },
	{ 0, {   756000, HFPLL, 1, 0x1C }, L2(6),  1075000 },
	{ 1, {   810000, HFPLL, 1, 0x1E }, L2(6),  1075000 },
	{ 0, {   864000, HFPLL, 1, 0x20 }, L2(6),  1100000 },
	{ 1, {   918000, HFPLL, 1, 0x22 }, L2(6),  1100000 },
	{ 0, {   972000, HFPLL, 1, 0x24 }, L2(6),  1125000 },
	{ 1, {  1026000, HFPLL, 1, 0x26 }, L2(6),  1125000 },
	{ 0, {  1080000, HFPLL, 1, 0x28 }, L2(15), 1175000 },
	{ 1, {  1134000, HFPLL, 1, 0x2A }, L2(15), 1175000 },
	{ 0, { 0 } }
};

static struct acpu_level tbl_nom_factory_1134[] __initdata = {
	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0),   925000 },
	{ 0, {   432000, HFPLL, 2, 0x20 }, L2(6),   950000 },
	{ 1, {   486000, HFPLL, 2, 0x24 }, L2(6),   950000 },
	{ 0, {   540000, HFPLL, 2, 0x28 }, L2(6),   975000 },
	{ 1, {   594000, HFPLL, 1, 0x16 }, L2(6),   975000 },
	{ 0, {   648000, HFPLL, 1, 0x18 }, L2(6),   975000 },
	{ 1, {   702000, HFPLL, 1, 0x1A }, L2(6),   975000 },
	{ 0, {   756000, HFPLL, 1, 0x1C }, L2(6),  1025000 },
	{ 1, {   810000, HFPLL, 1, 0x1E }, L2(6),  1025000 },
	{ 0, {   864000, HFPLL, 1, 0x20 }, L2(6),  1050000 },
	{ 1, {   918000, HFPLL, 1, 0x22 }, L2(6),  1050000 },
	{ 0, {   972000, HFPLL, 1, 0x24 }, L2(6),  1075000 },
	{ 1, {  1026000, HFPLL, 1, 0x26 }, L2(6),  1075000 },
	{ 0, {  1080000, HFPLL, 1, 0x28 }, L2(15), 1125000 },
	{ 1, {  1134000, HFPLL, 1, 0x2A }, L2(15), 1125000 },
	{ 0, { 0 } }
};

static struct acpu_level tbl_fast_factory_1134[] __initdata = {
	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0),   875000 },
	{ 0, {   432000, HFPLL, 2, 0x20 }, L2(6),   900000 },
	{ 1, {   486000, HFPLL, 2, 0x24 }, L2(6),   900000 },
	{ 0, {   540000, HFPLL, 2, 0x28 }, L2(6),   925000 },
	{ 1, {   594000, HFPLL, 1, 0x16 }, L2(6),   925000 },
	{ 0, {   648000, HFPLL, 1, 0x18 }, L2(6),   950000 },
	{ 1, {   702000, HFPLL, 1, 0x1A }, L2(6),   950000 },
	{ 0, {   756000, HFPLL, 1, 0x1C }, L2(6),   975000 },
	{ 1, {   810000, HFPLL, 1, 0x1E }, L2(6),   975000 },
	{ 0, {   864000, HFPLL, 1, 0x20 }, L2(6),  1000000 },
	{ 1, {   918000, HFPLL, 1, 0x22 }, L2(6),  1000000 },
	{ 0, {   972000, HFPLL, 1, 0x24 }, L2(6),  1025000 },
	{ 1, {  1026000, HFPLL, 1, 0x26 }, L2(6),  1025000 },
	{ 0, {  1080000, HFPLL, 1, 0x28 }, L2(15), 1075000 },
	{ 1, {  1134000, HFPLL, 1, 0x2A }, L2(15), 1075000 },
	{ 0, { 0 } }
};

#endif
#endif //CONFIG_LGE_PM

static struct acpu_level tbl_slow[] __initdata = {
	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0),   875000 },
	{ 0, {   432000, HFPLL, 2, 0x20 }, L2(6),  900000 },
	{ 1, {   486000, HFPLL, 2, 0x24 }, L2(6),  900000 },
	{ 0, {   540000, HFPLL, 2, 0x28 }, L2(6),  900000 },
	{ 1, {   594000, HFPLL, 1, 0x16 }, L2(6),  900000 },
	{ 0, {   648000, HFPLL, 1, 0x18 }, L2(6),  925000 },
	{ 1, {   702000, HFPLL, 1, 0x1A }, L2(6),  925000 },
	{ 0, {   756000, HFPLL, 1, 0x1C }, L2(6),  975000 },
	{ 1, {   810000, HFPLL, 1, 0x1E }, L2(6),  975000 },
	{ 0, {   864000, HFPLL, 1, 0x20 }, L2(6),  1000000 },
	{ 1, {   918000, HFPLL, 1, 0x22 }, L2(6),  1000000 },
	{ 0, {   972000, HFPLL, 1, 0x24 }, L2(6),  1025000 },
	{ 1, {  1026000, HFPLL, 1, 0x26 }, L2(6),  1025000 },
	{ 0, {  1080000, HFPLL, 1, 0x28 }, L2(15), 1075000 },
	{ 1, {  1134000, HFPLL, 1, 0x2A }, L2(15), 1075000 },
	{ 0, {  1188000, HFPLL, 1, 0x2C }, L2(15), 1100000 },
	{ 1, {  1242000, HFPLL, 1, 0x2E }, L2(15), 1100000 },
	{ 0, {  1296000, HFPLL, 1, 0x30 }, L2(15), 1125000 },
	{ 1, {  1350000, HFPLL, 1, 0x32 }, L2(15), 1125000 },
	{ 0, {  1404000, HFPLL, 1, 0x34 }, L2(15), 1137500 },
	{ 1, {  1458000, HFPLL, 1, 0x36 }, L2(15), 1137500 },
	{ 1, {  1512000, HFPLL, 1, 0x38 }, L2(15), 1150000 },
	{ 0, { 0 } }
};

static struct acpu_level tbl_nom[] __initdata = {
	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0),   825000 },
	{ 0, {   432000, HFPLL, 2, 0x20 }, L2(6),   850000 },
	{ 1, {   486000, HFPLL, 2, 0x24 }, L2(6),   850000 },
	{ 0, {   540000, HFPLL, 2, 0x28 }, L2(6),   875000 },
	{ 1, {   594000, HFPLL, 1, 0x16 }, L2(6),   875000 },
	{ 0, {   648000, HFPLL, 1, 0x18 }, L2(6),   875000 },
	{ 1, {   702000, HFPLL, 1, 0x1A }, L2(6),   875000 },
	{ 0, {   756000, HFPLL, 1, 0x1C }, L2(6),  925000 },
	{ 1, {   810000, HFPLL, 1, 0x1E }, L2(6),  925000 },
	{ 0, {   864000, HFPLL, 1, 0x20 }, L2(6),  950000 },
	{ 1, {   918000, HFPLL, 1, 0x22 }, L2(6),  950000 },
	{ 0, {   972000, HFPLL, 1, 0x24 }, L2(6),  975000 },
	{ 1, {  1026000, HFPLL, 1, 0x26 }, L2(6),  975000 },
	{ 0, {  1080000, HFPLL, 1, 0x28 }, L2(15), 1025000 },
	{ 1, {  1134000, HFPLL, 1, 0x2A }, L2(15), 1025000 },
	{ 0, {  1188000, HFPLL, 1, 0x2C }, L2(15), 1050000 },
	{ 1, {  1242000, HFPLL, 1, 0x2E }, L2(15), 1050000 },
	{ 0, {  1296000, HFPLL, 1, 0x30 }, L2(15), 1075000 },
	{ 1, {  1350000, HFPLL, 1, 0x32 }, L2(15), 1075000 },
	{ 0, {  1404000, HFPLL, 1, 0x34 }, L2(15), 1087500 },
	{ 1, {  1458000, HFPLL, 1, 0x36 }, L2(15), 1087500 },
	{ 1, {  1512000, HFPLL, 1, 0x38 }, L2(15), 1100000 },
	{ 0, { 0 } }
};

static struct acpu_level tbl_fast[] __initdata = {
	{ 1, {   384000, PLL_8, 0, 0x00 }, L2(0),   775000 },
	{ 0, {   432000, HFPLL, 2, 0x20 }, L2(6),   800000 },
	{ 1, {   486000, HFPLL, 2, 0x24 }, L2(6),   800000 },
	{ 0, {   540000, HFPLL, 2, 0x28 }, L2(6),   825000 },
	{ 1, {   594000, HFPLL, 1, 0x16 }, L2(6),   825000 },
	{ 0, {   648000, HFPLL, 1, 0x18 }, L2(6),   850000 },
	{ 1, {   702000, HFPLL, 1, 0x1A }, L2(6),   850000 },
	{ 0, {   756000, HFPLL, 1, 0x1C }, L2(6),   875000 },
	{ 1, {   810000, HFPLL, 1, 0x1E }, L2(6),   875000 },
	{ 0, {   864000, HFPLL, 1, 0x20 }, L2(6),  900000 },
	{ 1, {   918000, HFPLL, 1, 0x22 }, L2(6),  900000 },
	{ 0, {   972000, HFPLL, 1, 0x24 }, L2(6),  925000 },
	{ 1, {  1026000, HFPLL, 1, 0x26 }, L2(6),  925000 },
	{ 0, {  1080000, HFPLL, 1, 0x28 }, L2(15), 975000 },
	{ 1, {  1134000, HFPLL, 1, 0x2A }, L2(15), 975000 },
	{ 0, {  1188000, HFPLL, 1, 0x2C }, L2(15), 1000000 },
	{ 1, {  1242000, HFPLL, 1, 0x2E }, L2(15), 1000000 },
	{ 0, {  1296000, HFPLL, 1, 0x30 }, L2(15), 1025000 },
	{ 1, {  1350000, HFPLL, 1, 0x32 }, L2(15), 1025000 },
	{ 0, {  1404000, HFPLL, 1, 0x34 }, L2(15), 1037500 },
	{ 1, {  1458000, HFPLL, 1, 0x36 }, L2(15), 1037500 },
	{ 1, {  1512000, HFPLL, 1, 0x38 }, L2(15), 1050000 },
	{ 0, { 0 } }
};

static struct pvs_table pvs_tables[NUM_SPEED_BINS][NUM_PVS] __initdata = {
	[0][PVS_SLOW]    = {tbl_slow, sizeof(tbl_slow),     0 },
	[0][PVS_NOMINAL] = {tbl_nom,  sizeof(tbl_nom),  25000 },
	[0][PVS_FAST]    = {tbl_fast, sizeof(tbl_fast), 25000 },
	[0][PVS_FASTER]  = {tbl_fast, sizeof(tbl_fast), 25000 },
};

#ifdef CONFIG_LGE_PM
#if defined(CONFIG_MACH_APQ8064_J1KD) || defined(CONFIG_MACH_APQ8064_J1D)
static struct pvs_table pvs_tables_factory_56k[NUM_SPEED_BINS][NUM_PVS] __initdata = {
	[0][PVS_SLOW]    = { tbl_slow_factory_56k, sizeof(tbl_slow_factory_56k),     0 },
	[0][PVS_NOMINAL] = { tbl_nom_factory_56k,  sizeof(tbl_nom_factory_56k),  25000 },
	[0][PVS_FAST]    = { tbl_fast_factory_56k, sizeof(tbl_fast_factory_56k), 25000 },
	[0][PVS_FASTER]  = { tbl_fast_factory_56k, sizeof(tbl_fast_factory_56k), 25000 },
};
#endif
static struct pvs_table pvs_tables_factory_1026[NUM_SPEED_BINS][NUM_PVS] __initdata = {
	[0][PVS_SLOW]    = {tbl_slow_factory_1026, sizeof(tbl_slow_factory_1026),     0 },
	[0][PVS_NOMINAL] = {tbl_nom_factory_1026,  sizeof(tbl_nom_factory_1026),  25000 },
	[0][PVS_FAST]    = {tbl_fast_factory_1026, sizeof(tbl_fast_factory_1026), 25000 },
	[0][PVS_FASTER]  = {tbl_fast_factory_1026, sizeof(tbl_fast_factory_1026), 25000 },

};
#if defined(CONFIG_MACH_APQ8064_GK_KR)||defined(CONFIG_MACH_APQ8064_GKATT)||defined(CONFIG_MACH_APQ8064_GVDCM)||defined(CONFIG_MACH_APQ8064_GV_KR)|| defined(CONFIG_MACH_APQ8064_GKGLOBAL)
static struct pvs_table pvs_tables_factory_1134[NUM_SPEED_BINS][NUM_PVS] __initdata = {
	[0][PVS_SLOW]    = {tbl_slow_factory_1134, sizeof(tbl_slow_factory_1134),     0 },
	[0][PVS_NOMINAL] = {tbl_nom_factory_1134,  sizeof(tbl_nom_factory_1134),  25000 },
	[0][PVS_FAST]    = {tbl_fast_factory_1134, sizeof(tbl_fast_factory_1134), 25000 },
	[0][PVS_FASTER]  = {tbl_fast_factory_1134, sizeof(tbl_fast_factory_1134), 25000 },

};
#endif
#endif

static struct acpuclk_krait_params acpuclk_8064_params __initdata = {
	.scalable = scalable,
	.scalable_size = sizeof(scalable),
	.hfpll_data = &hfpll_data,
	.pvs_tables = pvs_tables,
	.l2_freq_tbl = l2_freq_tbl,
	.l2_freq_tbl_size = sizeof(l2_freq_tbl),
	.bus_scale = &bus_scale_data,
	.pte_efuse_phys = 0x007000C0,
	.stby_khz = 384000,
};

static int __init acpuclk_8064_probe(struct platform_device *pdev)
{
#ifdef CONFIG_LGE_PM
	/* Krait freq table set for factory boot mode */
	if (lge_get_factory_boot()) 
	{
#if defined(CONFIG_MACH_APQ8064_J1KD) || defined(CONFIG_MACH_APQ8064_J1D)
		if(lge_get_boot_cable_type() == LGE_BOOT_LT_CABLE_56K)	
		{
			pr_info("select pvs_tables to factory 56K\n");
			acpuclk_8064_params.pvs_tables = pvs_tables_factory_56k;
		}
		else
		{
			pr_info("select pvs_tables to factory\n");
			acpuclk_8064_params.pvs_tables = pvs_tables_factory_1026;
		}		
#elif defined(CONFIG_MACH_APQ8064_GK_KR)||defined(CONFIG_MACH_APQ8064_GKATT)||defined(CONFIG_MACH_APQ8064_GVDCM)||defined(CONFIG_MACH_APQ8064_GV_KR)||defined(CONFIG_MACH_APQ8064_GKGLOBAL)
		if(lge_get_boot_cable_type() == LGE_BOOT_LT_CABLE_56K ||
			lge_get_boot_cable_type() == LGE_BOOT_LT_CABLE_130K ||
			lge_get_boot_cable_type() == LGE_BOOT_LT_CABLE_910K)
		{
			pr_info("select pvs_tables to factory_1026\n");
			acpuclk_8064_params.pvs_tables = pvs_tables_factory_1026;
		}
		else
		{
			pr_info("select pvs_tables to factory_1134\n");
			acpuclk_8064_params.pvs_tables = pvs_tables_factory_1134;
		}

#else
		pr_info("select pvs_tables to factory_1026\n");
		acpuclk_8064_params.pvs_tables = pvs_tables_factory_1026;
#endif
	}
#endif //CONFIG_LGE_PM

	if (cpu_is_apq8064ab() ||
		SOCINFO_VERSION_MAJOR(socinfo_get_version()) == 2) {
		acpuclk_8064_params.hfpll_data->low_vdd_l_max = 37;
		acpuclk_8064_params.hfpll_data->nom_vdd_l_max = 74;
	}

	return acpuclk_krait_init(&pdev->dev, &acpuclk_8064_params);
}

static struct platform_driver acpuclk_8064_driver = {
	.driver = {
		.name = "acpuclk-8064",
		.owner = THIS_MODULE,
	},
};

static int __init acpuclk_8064_init(void)
{
	return platform_driver_probe(&acpuclk_8064_driver,
				     acpuclk_8064_probe);
}
device_initcall(acpuclk_8064_init);
