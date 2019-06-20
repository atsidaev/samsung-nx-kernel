/* arch/arm/mach-drime4/include/mach/regs-nand.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * DRIMe4 NAND register definitions
*/

#ifndef __ASM_ARM_REGS_NAND
#define __ASM_ARM_REGS_NAND

#define DRIME4_NFCONF			0x00
#define DRIME4_NFCONT			0x04
#define DRIME4_NFCMD			0x08
#define DRIME4_NFADDR			0x0C
#define DRIME4_NFDATA			0x10
#define DRIME4_NFECCD0			0x14
#define DRIME4_NFECCD1			0x18
#define DRIME4_NFECCD			0x1C
#define DRIME4_NFSBLK			0x20
#define DRIME4_NFEBLK			0x24
#define DRIME4_NFSTAT			0x28
#define DRIME4_NFMECC_ERR0		0x2C
#define DRIME4_NFMECC_ERR1		0x30
#define DRIME4_NFMECC0			0x34
#define DRIME4_NFMECC1			0x38
#define DRIME4_NFSECC			0x3C
#define DRIME4_NFMLCBITPT		0x40
#define DRIME4_NF8ECCERR0		0x44
#define DRIME4_NF8ECCERR1		0x48
#define DRIME4_NF8ECCERR2		0x4C
#define DRIME4_NFM8ECC0			0x50
#define DRIME4_NFM8ECC1			0x54
#define DRIME4_NFM8ECC2			0x58
#define DRIME4_NFM8ECC3			0x5C
#define DRIME4_NFM8BITPT0		0x60
#define DRIME4_NFM8BITPT1		0x64


#define DRIME4_NFECC_SLC1BIT		0x0
#define DRIME4_NFECC_MLC4BIT		0x2
#define DRIME4_NFECC_MLC8BIT		0x1

#define DRIME4_NFCONF_NANDBOOT		(1<<31)
#define DRIME4_NFCONF_MLC_CLKCON	(1<<30)
#define DRIME4_NFCONF_LSG_LENGTH	(1<<25)
#define DRIME4_NFCONF_ECC_TYPE(x)	((x)<<23)
#define DRIME4_NFCONF_ECC_TYPE_MASK	(3<<23)
#define DRIME4_NFCONF_TACLS(x)		((x)<<12)
#define DRIME4_NFCONF_TACLS_MASK	(7<<12)	/* 1 extra bit of Tacls */
#define DRIME4_NFCONF_TWRPH0(x)	((x)<<8)
#define DRIME4_NFCONF_TWRPH0_MASK	(7<<8)
#define DRIME4_NFCONF_TWRPH1(x)	((x)<<4)
#define DRIME4_NFCONF_TWRPH1_MASK	(7<<4)
#define DRIME4_NFCONF_MLCFLASH		(1<<3)
#define DRIME4_NFCONF_PAGE_SIZE	(1<<2)
#define DRIME4_NFCONF_ADDR_CYCLE	(1<<1)
#define DRIME4_NFCONF_BUSWIDTH		(1<<0)
#define DRIME4_NFCONF_BUSWIDTH_16	(1<<0)
#define DRIME4_NFCONF_BUSWIDTH_8	(0<<0)


#define DRIME4_NFCONT_ECCDIR_ENC	(1<<20)
#define DRIME4_NFCONT_LOCKTIGHT	(1<<19)
#define DRIME4_NFCONT_SOFTLOCK		(1<<18)
#define DRIME4_NFCONT_ECCEN_INT	(1<<15)
#define DRIME4_NFCONT_ECCDE_INT	(1<<14)
#define DRIME4_NFCONT_ECC_STOP		(1<<13)
#define DRIME4_NFCONT_ILLEGALACC_EN	(1<<12)
#define DRIME4_NFCONT_RNBINT_EN	(1<<11)
#define DRIME4_NFCONT_RN_FALLING	(1<<10)
#define DRIME4_NFCONT_MAIN_ECCLOCK	(1<<9)
#define DRIME4_NFCONT_SPARE_ECCLOCK	(1<<8)
#define DRIME4_NFCONT_INITMECC		(1<<7)
#define DRIME4_NFCONT_INITSECC		(1<<6)
#define DRIME4_NFCONT_nCE		(1<<5)
#define DRIME4_NFCONT_nFCE3		(1<<4)
#define DRIME4_NFCONT_nFCE2		(1<<3)
#define DRIME4_NFCONT_nFCE1		(1<<2)
#define DRIME4_NFCONT_nFCE0		(1<<1)
#define DRIME4_NFCONT_ENABLE		(1<<0)

#define DRIME4_NFSTAT_ECC_ENCDONE	(1<<9)
#define DRIME4_NFSTAT_ECC_DECDONE	(1<<8)
#define DRIME4_NFSTAT_ILLEGAL_ACCESS	(1<<7)
#define DRIME4_NFSTAT_RnB_CHANGE	(1<<6)
#define DRIME4_NFSTAT_nFCE1		(1<<3)
#define DRIME4_NFSTAT_nFCE0		(1<<2)
#define DRIME4_NFSTAT_Res1		(1<<1)
#define DRIME4_NFSTAT_READY		(1<<0)

#define DRIME4_NDMA_CONT		0x00
#define DRIME4_NDMA_CONF		0x04
#define DRIME4_NDMA_SRC_ADDR		0x08
#define DRIME4_NDMA_DST_ADDR		0x0C
#define DRIME4_NDMA_PG_SIZE		0x10
#define DRIME4_NDMA_INT_STS		0x14
#define DRIME4_NDMA_BASE_ADDR		0x18
#define DRIME4_NDMA_STS		0x1C
#define DRIME4_NDMA_ROW_ADDR		0x20
#define DRIME4_NDMA_SCT_COUNT		0x24

#define DRIME4_NDMA_INT_ECCERR		(1<<1)
#define DRIME4_NDMA_INT_COMPLETE	(1<<2)

#define DRIME4_NDMA_STS_COMPLETE	(1<<0)
#define DRIME4_NDMA_STS_ECCERR		(1<<1)

#define DRIME4_NDMA_CTL_START		(1<<0)
#define DRIME4_NDMA_CTL_STOP		(1<<1)
#define DRIME4_NDMA_CTL_RESTART	(1<<2)
#define DRIME4_NDMA_CTL_INTCLR	(1<<3)

#define DRIME4_NDMA_OK			0x00
#define DRIME4_NDMA_ECCERR		0x01
#define DRIME4_NDMA_ECC_UNCORRECT	0x02
#define DRIME4_NDMA_FAIL		0x03


#endif /* __ASM_ARM_REGS_NAND */
