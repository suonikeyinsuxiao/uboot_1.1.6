/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <regs.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */

#define M_MDIV	0x03c
#define M_PDIV	0x02
#define M_SDIV	0x0

#define U_M_MDIV	0x030
#define U_M_PDIV	0x2
#define U_M_SDIV	0x1

static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n" "bne 1b":"=r" (loops):"0"(loops));
}

static void cs8900_pre_init(void)
{
	BWSCON_REG = ( BWSCON_REG & ~(0xf<<12)) | (0xd << 12);
	BANKCON3_REG = (3<<11)|(0x7<<8)|(0x1<<6)|(0x3<<4)|(0x3<<2);
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init(void)
{
	cs8900_pre_init();
	gd->bd->bi_arch_number = MACH_TYPE;
	gd->bd->bi_boot_params = 0x30000100;

	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
#if (CONFIG_NR_DRAM_BANKS == 2)
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
#endif
	return 0;
}

#ifdef BOARD_LATE_INIT
int board_late_init (void)
{
	uint *magic = (uint*)(PHYS_SDRAM_1);
	char boot_cmd[100];

	if ((0x24564236 == magic[0]) && (0x20764316 == magic[1])) {
		sprintf(boot_cmd, "nand erase 0 40000;nand write %08x 0 40000", PHYS_SDRAM_1+0x8000);
		magic[0] = 0;
		magic[1] = 0;
		printf("\n\nready for self-burning U-Boot image\n\n");
		setenv("bootdelay", "0");
		setenv("bootcmd", boot_cmd);
	}
}
#endif

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
#ifdef CONFIG_SMDK2440
	printf("Board: SMDK2440\n");
#elif defined (CONFIG_SMDK2442)
	printf("Board: SMDK2442\n");
#else
#error no board definition is found.
#endif
	return (0);
}
#endif

#ifdef CONFIG_ENABLE_MMU
ulong virt_to_phy_smdk2440(ulong addr)
{
	if ((0xc0000000 <= addr) && (addr < 0xc8000000))
		return (addr - 0xc0000000 + 0x30000000);
	else if ((PHYS_SDRAM_1 <= addr) && (addr < (PHYS_SDRAM_1+PHYS_SDRAM_1_SIZE))) {
		return addr;
	}
	else
		printf("do not support this address : %08lx\n", addr);

	return addr;
}
#endif

#if (CONFIG_COMMANDS & CFG_CMD_NAND) && defined(CFG_NAND_LEGACY)
#define TECH		0x3f
#define TACLS		7	//0  //1clk(0ns)
#define TWRPH0		7
#define TWRPH1		7
				//0  //1clk(10ns)  //TACLS+TWRPH0+TWRPH1>=50ns

#include <linux/mtd/nand.h>
extern struct nand_chip nand_dev_desc[CFG_MAX_NAND_DEVICE];
void nand_init(void)
{
        NFCONF_REG = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4)|(0<<0);
        NFCONT_REG = (0<<13)|(0<<12)|(0<<10)|(0<<9)|(0<<8)|(1<<6)|(1<<5)|(1<<4)|(1<<1)|(1<<0);

	nand_probe(CFG_NAND_BASE);
	if (nand_dev_desc[0].ChipID != NAND_ChipID_UNKNOWN) {
		print_size(nand_dev_desc[0].totlen, "\n");
	}
}
#endif
