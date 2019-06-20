/* arch/arm/mach-drime4/include/mach/nand.h
 *
 * DRIME4 - NAND device controller platform_device info
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

/**
 * struct drime4_nand_set - define a set of one or more nand chips
 * @disable_ecc:	Entirely disable ECC - Dangerous
 * @flash_bbt: 		Openmoko u-boot can create a Bad Block Table
 *			Setting this flag will allow the kernel to
 *			look for it at boot time and also skip the NAND
 *			scan.
 * @options:		Default value to set into 'struct nand_chip' options.
 * @nr_chips:		Number of chips in this set
 * @nr_partitions:	Number of partitions pointed to by @partitions
 * @name:		Name of set (optional)
 * @nr_map:		Map for low-layer logical to physical chip numbers (option)
 * @partitions:		The mtd partition list
 *
 * define a set of one or more nand chips registered with an unique mtd. Also
 * allows to pass flag to the underlying NAND layer. 'disable_ecc' will trigger
 * a warning at boot time.
 */
struct drime4_nand_set {
	unsigned int		disable_ecc:1;
	unsigned int		flash_bbt:1;

	unsigned int		options;
	int			nr_chips;
	int			nr_partitions;
	char			*name;
	int			*nr_map;
	struct mtd_partition	*partitions;
	struct nand_ecclayout	*ecc_layout;
};

struct drime4_platform_nand {
	/* timing information for controller, all times in nanoseconds */

	int	tacls;	/* time for active CLE/ALE to nWE/nOE */
	int	twrph0;	/* active time for nWE/nOE */
	int	twrph1;	/* time for release CLE/ALE from nWE/nOE inactive */

	unsigned int	ignore_unset_ecc:1;

	int		nr_sets;
	struct		drime4_nand_set *sets;

	void		(*select_chip)(struct drime4_nand_set *, int chip);
};

/**
 * drime4_nand_set_platdata() - register NAND platform data.
 * @nand: The NAND platform data to register with dreim4_device_nand.
 *
 * This function copies the given NAND platform data, @nand and registers
 * it with the drime4_device_nand. This allows @nand to be __initdata.
*/
extern void drime4_nand_set_platdata(struct drime4_platform_nand *nand);
