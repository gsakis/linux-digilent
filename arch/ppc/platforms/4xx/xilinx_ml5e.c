/*
 * arch/ppc/platforms/4xx/xilinx_ml5e.c
 *
 * Xilinx ML5 PPC440 EMULATION board initialization
 *
 * Author: Grant Likely <grant.likely@secretlab.ca>
 * 	   Wolfgang Reissnegger <w.reissnegger@gmx.net>
 *
 * 2007 (c) Xilinx, Inc.
 * 2005 (c) Secret Lab Technologies Ltd.
 * 2002-2004 (c) MontaVista Software, Inc.
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2.  This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#include <linux/init.h>
#include <linux/irq.h>
#include <linux/tty.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>
#include <linux/serialP.h>
#include <asm/io.h>
#include <asm/machdep.h>
#include <asm/ppc4xx_pic.h>

#include <syslib/ibm44x_common.h>

#include <syslib/gen550.h>
#include <syslib/virtex_devices.h>
#include <platforms/4xx/xparameters/xparameters.h>

const char* virtex_machine_name = "Xilinx ML5E PPC440 EMULATION System";

#if defined(XPAR_POWER_0_POWERDOWN_BASEADDR)
static volatile unsigned *powerdown_base =
    (volatile unsigned *) XPAR_POWER_0_POWERDOWN_BASEADDR;

static void
xilinx_power_off(void)
{
	local_irq_disable();
	out_be32(powerdown_base, XPAR_POWER_0_POWERDOWN_VALUE);
	while (1) ;
}
#endif

#include <asm/pgtable.h>
void __init
ml5e_map_io(void)
{
#if defined(XPAR_POWER_0_POWERDOWN_BASEADDR)
	powerdown_base = ioremap((unsigned long) powerdown_base,
				 XPAR_POWER_0_POWERDOWN_HIGHADDR -
				 XPAR_POWER_0_POWERDOWN_BASEADDR + 1);
#endif
}

static void __init
ml5e_early_serial_map(void)
{
	struct uart_port port;

	/* Setup ioremapped serial port access */
	memset(&port, 0, sizeof(port));
	port.membase	= ioremap64(XPAR_UARTNS550_0_BASEADDR, 8);
	port.irq	= XPAR_INTC_0_UARTNS550_0_VEC_ID;
	port.uartclk	= XPAR_UARTNS550_0_CLOCK_FREQ_HZ;
	port.regshift	= 2;
	port.iotype	= UPIO_MEM;
	port.flags	= UPF_BOOT_AUTOCONF;
	port.line	= 0;

	if (early_serial_setup(&port) != 0) {
		printk("Early serial init of port 0 failed\n");
	}
}

void __init
ml5e_setup_arch(void)
{
	ml5e_early_serial_map();

#ifdef CONFIG_PCI
	ppc4xx_find_bridges();
#endif

	/* Identify the system */
	printk(KERN_INFO "Xilinx ML5E PPC440 EMULATION System\n");
}

void __init
ml5e_init_irq(void)
{
	unsigned int i;

	ppc4xx_pic_init();

	/*
	 * For PowerPC 405 cores the default value for NR_IRQS is 32.
	 * See include/asm-ppc/irq.h for details.
	 * This is just fine for ML300, ML403 and ML5xx
	 */
#if (NR_IRQS != 32)
#error NR_IRQS must be 32 for ML300/ML403/ML5xx
#endif

	for (i = 0; i < NR_IRQS; i++) {
		if (XPAR_INTC_0_KIND_OF_INTR & (0x80000000 >> i))
			irq_desc[i].status &= ~IRQ_LEVEL;
		else
			irq_desc[i].status |= IRQ_LEVEL;
	}
}

/*
 * Return the virtual address representing the top of physical RAM.
 */
static unsigned long __init
ml5e_find_end_of_memory(void)
{
	// wgr HACK
	// Does printk work here already?
	//
	printk("*** HACK: Assuming 64MB memory size. %s, line %d\n",
			__FILE__, __LINE__ +1);
	return 64 * 1024 * 1024;
	// wgr HACK end
}


static void __init
ml5e_calibrate_decr(void)
{
	unsigned int freq;

	// wgr HACK
	// Does printk work here already?
	//
	printk("*** HACK: Assuming 500000000 Hz freq. %s, line %d\n",
			__FILE__, __LINE__ +1);
	freq = 500000000;
// -wgr- 	us_to_tb = freq / 1000000;
	// wgr HACK end

// -wgr- 	tb_ticks_per_jiffy = freq / HZ;
// -wgr- 	tb_to_us = mulhwu_scale_factor(freq, 1000000);
}


void __init
platform_init(unsigned long r3, unsigned long r4, unsigned long r5,
	      unsigned long r6, unsigned long r7)
{
	/* Calling ppc4xx_init will set up the default values for ppc_md.
	 */
	ibm44x_platform_init(r3, r4, r5, r6, r7);


	/* Overwrite the default settings with our platform specific hooks.
	 */
	ppc_md.setup_arch		= ml5e_setup_arch;
	ppc_md.setup_io_mappings	= ml5e_map_io;
	ppc_md.init_IRQ			= ml5e_init_irq;
	ppc_md.find_end_of_memory	= ml5e_find_end_of_memory;
	ppc_md.calibrate_decr		= ml5e_calibrate_decr;

#if defined(XPAR_POWER_0_POWERDOWN_BASEADDR)
	ppc_md.power_off		= xilinx_power_off;
#endif

#ifdef CONFIG_KGDB
	ppc_md.early_serial_map		= virtex_early_serial_map;
#endif
}
































#if 0




/* Early serial support functions */
static void __init
ml5e_early_serial_init(int num, struct plat_serial8250_port *pdata)
{
#if defined(CONFIG_SERIAL_TEXT_DEBUG) || defined(CONFIG_KGDB)
	struct uart_port serial_req;

	memset(&serial_req, 0, sizeof(serial_req));
	serial_req.mapbase	= pdata->mapbase;
	serial_req.membase	= pdata->membase;
	serial_req.irq		= pdata->irq;
	serial_req.uartclk	= pdata->uartclk;
	serial_req.regshift	= pdata->regshift;
	serial_req.iotype	= pdata->iotype;
	serial_req.flags	= pdata->flags;
	gen550_init(num, &serial_req);
#endif
}

void __init
ml5e_early_serial_map(void)
{
#ifdef CONFIG_SERIAL_8250
	struct plat_serial8250_port *pdata;
	int i = 0;

	pdata = (struct plat_serial8250_port *) ppc_sys_get_pdata(VIRTEX_UART);
	while(pdata && pdata->flags)
	{
		pdata->membase = ioremap(pdata->mapbase, 0x100);
		ml5e_early_serial_init(i, pdata);
		pdata++;
		i++;
	}
#endif /* CONFIG_SERIAL_8250 */
}

void __init
ml5e_setup_arch(void)
{
	ml5e_early_serial_map();

#ifdef CONFIG_PCI
	ppc4xx_find_bridges();
#endif

	/* Identify the system */
	printk(KERN_INFO "Xilinx ML5E PPC440 EMULATION System\n");
}


void __init
ml5e_init_irq(void)
{
	unsigned int i;

	ppc4xx_pic_init();

	/*
	 * For PowerPC 405 cores the default value for NR_IRQS is 32.
	 * See include/asm-ppc/irq.h for details.
	 * This is just fine for ML300, ML403 and ML5xx
	 */
#if (NR_IRQS != 32)
#error NR_IRQS must be 32 for ML300/ML403/ML5xx
#endif

	for (i = 0; i < NR_IRQS; i++) {
		if (XPAR_INTC_0_KIND_OF_INTR & (0x80000000 >> i))
			irq_desc[i].status &= ~IRQ_LEVEL;
		else
			irq_desc[i].status |= IRQ_LEVEL;
	}
}

void __init
platform_init(unsigned long r3, unsigned long r4, unsigned long r5,
	      unsigned long r6, unsigned long r7)
{
	/* Calling ppc4xx_init will set up the default values for ppc_md.
	 */
	ppc4xx_init(r3, r4, r5, r6, r7);

	identify_ppc_sys_by_id(mfspr(SPRN_PVR));

	/* Overwrite the default settings with our platform specific hooks.
	 */
	ppc_md.setup_arch		= ml5e_setup_arch;
	ppc_md.setup_io_mappings	= ml5e_map_io;
	ppc_md.init_IRQ			= ml5e_init_irq;
	ppc_md.find_end_of_memory	= ml5e_find_end_of_memory;
	ppc_md.calibrate_decr		= ml5e_calibrate_decr;

#if defined(XPAR_POWER_0_POWERDOWN_BASEADDR)
	ppc_md.power_off		= xilinx_power_off;
#endif

#ifdef CONFIG_KGDB
	ppc_md.early_serial_map		= ml5e_early_serial_map;
#endif
}


/* Taken from ibm44x_common.c
 */
phys_addr_t fixup_bigphys_addr(phys_addr_t addr, phys_addr_t size)
{
	phys_addr_t page_4gb = 0;

        /*
	 * Trap the least significant 32-bit portions of an
	 * address in the 440's 36-bit address space.  Fix
	 * them up with the appropriate ERPN
	 */
	if ((addr >= PPC44x_IO_LO) && (addr <= PPC44x_IO_HI))
		page_4gb = PPC44x_IO_PAGE;
	else if ((addr >= PPC44x_PCI0CFG_LO) && (addr <= PPC44x_PCI0CFG_HI))
		page_4gb = PPC44x_PCICFG_PAGE;
#ifdef CONFIG_440SP
	else if ((addr >= PPC44x_PCI1CFG_LO) && (addr <= PPC44x_PCI1CFG_HI))
		page_4gb = PPC44x_PCICFG_PAGE;
	else if ((addr >= PPC44x_PCI2CFG_LO) && (addr <= PPC44x_PCI2CFG_HI))
		page_4gb = PPC44x_PCICFG_PAGE;
#endif
	else if ((addr >= PPC44x_PCIMEM_LO) && (addr <= PPC44x_PCIMEM_HI))
		page_4gb = PPC44x_PCIMEM_PAGE;

	return (page_4gb | addr);
};
EXPORT_SYMBOL(fixup_bigphys_addr);



#endif