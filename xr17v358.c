// SPDX-License-Identifier: GPL-2.0-only
/*
 * xr17v358.c - Linux serial driver skeleton for the MaxLinear XR17V358
 *              8-channel PCIe UART.
 *
 * The XR17V358 provides eight independent 16550-compatible UART channels
 * over a single PCIe function.  Registers are accessed through BAR0
 * (memory-mapped I/O) using the struct-based register map defined in
 * xr17v358.h:
 *
 *   channel N base  = BAR0_virt + N * XR17V358_CHAN_STRIDE
 *   global reg base = BAR0_virt + XR17V358_GLBL_BASE
 *
 * This skeleton implements:
 *   - PCI probe / remove
 *   - MMIO BAR mapping
 *   - uart_port registration for each channel
 *   - Minimal uart_ops (tx_empty, set_mctrl, get_mctrl, stop_tx,
 *     start_tx, stop_rx, startup, shutdown, set_termios, type,
 *     release_port, request_port, config_port)
 *   - Shared IRQ handler stub
 */

#include <linux/bitops.h>
#include <linux/idr.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <linux/serial_core.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>

#include "xr17v358.h"

/* -----------------------------------------------------------------------
 * Driver / module bookkeeping
 * ---------------------------------------------------------------------- */

#define DRIVER_NAME	"xr17v358"
#define DRIVER_DESC	"MaxLinear XR17V358 8-channel PCIe UART driver"

/*
 * Reference clock frequency in Hz.  The XR17V358 evaluation board uses a
 * 125 MHz oscillator; boards with a different crystal can override this
 * via the "uartclk" module parameter.
 */
static unsigned int xr17v358_uartclk = 125000000;
module_param_named(uartclk, xr17v358_uartclk, uint, 0444);
MODULE_PARM_DESC(uartclk, "Reference clock frequency in Hz (default: 125000000)");

/* Per-device instance index allocator for unique uart line numbers */
static DEFINE_IDA(xr17v358_ida);

/* -----------------------------------------------------------------------
 * Per-device private data
 * ---------------------------------------------------------------------- */

struct xr17v358_port {
	struct uart_port	uport;		/* must be first */
	struct xr17v358_chan_regs __iomem *regs; /* channel register map  */
	unsigned int		chan_idx;	/* 0-7                   */
};

struct xr17v358_dev {
	struct pci_dev			*pdev;
	void __iomem			*base;	  /* BAR0 virtual base       */
	struct xr17v358_glbl_regs __iomem *glbl;  /* global register map      */
	struct xr17v358_port		 port[XR17V358_NR_CHAN];
	struct uart_driver		*udrv;
	int				 dev_idx; /* IDA-allocated device index */
};

/* -----------------------------------------------------------------------
 * Register helpers – CHAN_RD/CHAN_WR resolve a struct field to its MMIO
 * address and call ioread8/iowrite8 to ensure correct typed MMIO access.
 * ---------------------------------------------------------------------- */
#define CHAN_RD(regs, field)		ioread8(&(regs)->field)
#define CHAN_WR(regs, field, val)	iowrite8((val), &(regs)->field)
#define GLBL_RD(regs, field)		ioread8(&(regs)->field)

/* -----------------------------------------------------------------------
 * uart_ops helpers
 * ---------------------------------------------------------------------- */

static inline struct xr17v358_port *to_xr_port(struct uart_port *port)
{
	return container_of(port, struct xr17v358_port, uport);
}

/* -----------------------------------------------------------------------
 * uart_ops – tx_empty
 * Returns TIOCSER_TEMT when both the TX FIFO and shift register are empty.
 * ---------------------------------------------------------------------- */

static unsigned int xr17v358_tx_empty(struct uart_port *port)
{
	struct xr17v358_port *xp = to_xr_port(port);
	u8 lsr = CHAN_RD(xp->regs, lsr);

	return (lsr & XR17V358_LSR_TEMT) ? TIOCSER_TEMT : 0;
}

/* -----------------------------------------------------------------------
 * uart_ops – set_mctrl
 * Apply DTR / RTS / LOOP bits from the uart_port modem-control word.
 * ---------------------------------------------------------------------- */

static void xr17v358_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	struct xr17v358_port *xp = to_xr_port(port);
	u8 mcr = 0;

	if (mctrl & TIOCM_DTR)
		mcr |= XR17V358_MCR_DTR;
	if (mctrl & TIOCM_RTS)
		mcr |= XR17V358_MCR_RTS;
	if (mctrl & TIOCM_OUT1)
		mcr |= XR17V358_MCR_OUT1;
	if (mctrl & TIOCM_OUT2)
		mcr |= XR17V358_MCR_OUT2;
	if (mctrl & TIOCM_LOOP)
		mcr |= XR17V358_MCR_LOOP;

	CHAN_WR(xp->regs, mcr, mcr);
}

/* -----------------------------------------------------------------------
 * uart_ops – get_mctrl
 * Read current modem-status lines from the MSR.
 * ---------------------------------------------------------------------- */

static unsigned int xr17v358_get_mctrl(struct uart_port *port)
{
	struct xr17v358_port *xp = to_xr_port(port);
	u8 msr = CHAN_RD(xp->regs, msr);
	unsigned int ret = 0;

	if (msr & XR17V358_MSR_CTS)
		ret |= TIOCM_CTS;
	if (msr & XR17V358_MSR_DSR)
		ret |= TIOCM_DSR;
	if (msr & XR17V358_MSR_RI)
		ret |= TIOCM_RI;
	if (msr & XR17V358_MSR_DCD)
		ret |= TIOCM_CAR;

	return ret;
}

/* -----------------------------------------------------------------------
 * uart_ops – stop_tx
 * Disable the TX ready interrupt so the driver stops feeding bytes.
 * ---------------------------------------------------------------------- */

static void xr17v358_stop_tx(struct uart_port *port)
{
	struct xr17v358_port *xp = to_xr_port(port);
	u8 ier = CHAN_RD(xp->regs, ier_dlm);

	ier &= ~XR17V358_IER_TXRDY;
	CHAN_WR(xp->regs, ier_dlm, ier);
}

/* -----------------------------------------------------------------------
 * uart_ops – start_tx
 * Enable the TX ready interrupt; the IRQ handler will drain the circ buf.
 * ---------------------------------------------------------------------- */

static void xr17v358_start_tx(struct uart_port *port)
{
	struct xr17v358_port *xp = to_xr_port(port);
	u8 ier = CHAN_RD(xp->regs, ier_dlm);

	ier |= XR17V358_IER_TXRDY;
	CHAN_WR(xp->regs, ier_dlm, ier);
}

/* -----------------------------------------------------------------------
 * uart_ops – stop_rx
 * Disable the RX-data-available interrupt.
 * ---------------------------------------------------------------------- */

static void xr17v358_stop_rx(struct uart_port *port)
{
	struct xr17v358_port *xp = to_xr_port(port);
	u8 ier = CHAN_RD(xp->regs, ier_dlm);

	ier &= ~XR17V358_IER_RXRDY;
	CHAN_WR(xp->regs, ier_dlm, ier);
}

/* -----------------------------------------------------------------------
 * uart_ops – startup
 * Called when the port is opened.  Enable FIFOs and RX/status interrupts.
 * ---------------------------------------------------------------------- */

static int xr17v358_startup(struct uart_port *port)
{
	struct xr17v358_port *xp = to_xr_port(port);

	/* Reset and enable TX/RX FIFOs */
	CHAN_WR(xp->regs, isr_fcr,
		XR17V358_FCR_FIFO_EN |
		XR17V358_FCR_RXFIFO_RST |
		XR17V358_FCR_TXFIFO_RST);

	/* Enable RX-data-available, RX line-status, and modem-change IRQs */
	CHAN_WR(xp->regs, ier_dlm,
		XR17V358_IER_RXRDY |
		XR17V358_IER_RXLST |
		XR17V358_IER_MDMCHG);

	/* Assert OUT2 to enable the interrupt output pin */
	CHAN_WR(xp->regs, mcr, XR17V358_MCR_OUT2);

	return 0;
}

/* -----------------------------------------------------------------------
 * uart_ops – shutdown
 * Called when the last file descriptor for the port is closed.
 * ---------------------------------------------------------------------- */

static void xr17v358_shutdown(struct uart_port *port)
{
	struct xr17v358_port *xp = to_xr_port(port);

	/* Disable all channel interrupts */
	CHAN_WR(xp->regs, ier_dlm, 0);

	/* Deassert OUT2 */
	CHAN_WR(xp->regs, mcr, 0);

	/* Disable and flush FIFOs */
	CHAN_WR(xp->regs, isr_fcr, 0);
}

/* -----------------------------------------------------------------------
 * uart_ops – set_termios
 * Program baud rate divisor and line parameters.
 * ---------------------------------------------------------------------- */

static void xr17v358_set_termios(struct uart_port *port,
				 struct ktermios *new,
				 const struct ktermios *old)
{
	struct xr17v358_port *xp = to_xr_port(port);
	unsigned int baud;
	unsigned int quot;
	u8 lcr = 0;
	unsigned long flags;

	/* Resolve the closest supported baud rate */
	baud = uart_get_baud_rate(port, new, old, 0, port->uartclk / 16);
	quot = uart_get_divisor(port, baud);

	/* Word length */
	switch (new->c_cflag & CSIZE) {
	case CS5: lcr |= XR17V358_LCR_WLS_5; break;
	case CS6: lcr |= XR17V358_LCR_WLS_6; break;
	case CS7: lcr |= XR17V358_LCR_WLS_7; break;
	default:  lcr |= XR17V358_LCR_WLS_8; break;
	}

	/* Stop bits */
	if (new->c_cflag & CSTOPB)
		lcr |= XR17V358_LCR_STB;

	/* Parity */
	if (new->c_cflag & PARENB) {
		lcr |= XR17V358_LCR_PEN;
		if (!(new->c_cflag & PARODD))
			lcr |= XR17V358_LCR_EPS;
		if (new->c_cflag & CMSPAR)
			lcr |= XR17V358_LCR_STP;
	}

	uart_port_lock_irqsave(port, &flags);

	uart_update_timeout(port, new->c_cflag, baud);

	/* Enable divisor latch, program divisor, then restore LCR */
	CHAN_WR(xp->regs, lcr, lcr | XR17V358_LCR_DLAB);
	CHAN_WR(xp->regs, rhr_thr_dll, quot & 0xFF);
	CHAN_WR(xp->regs, ier_dlm,    (quot >> 8) & 0xFF);
	CHAN_WR(xp->regs, lcr, lcr);

	uart_port_unlock_irqrestore(port, flags);
}

/* -----------------------------------------------------------------------
 * uart_ops – type
 * Return a human-readable port-type string shown in /proc/tty/driver/
 * ---------------------------------------------------------------------- */

static const char *xr17v358_type(struct uart_port *port)
{
	return DRIVER_NAME;
}

/* -----------------------------------------------------------------------
 * uart_ops – release_port / request_port / config_port
 * MMIO resources are managed at the PCI device level; these stubs satisfy
 * the uart_ops interface without double-mapping the BAR.
 * ---------------------------------------------------------------------- */

static void xr17v358_release_port(struct uart_port *port)
{
}

static int xr17v358_request_port(struct uart_port *port)
{
	return 0;
}

static void xr17v358_config_port(struct uart_port *port, int flags)
{
	if (flags & UART_CONFIG_TYPE)
		port->type = PORT_16550A;
}

/* -----------------------------------------------------------------------
 * uart_ops – verify_port
 * ---------------------------------------------------------------------- */

static int xr17v358_verify_port(struct uart_port *port,
				struct serial_struct *ser)
{
	if (ser->type != PORT_UNKNOWN && ser->type != PORT_16550A)
		return -EINVAL;
	return 0;
}

/* -----------------------------------------------------------------------
 * uart_ops table
 * ---------------------------------------------------------------------- */

static const struct uart_ops xr17v358_ops = {
	.tx_empty	= xr17v358_tx_empty,
	.set_mctrl	= xr17v358_set_mctrl,
	.get_mctrl	= xr17v358_get_mctrl,
	.stop_tx	= xr17v358_stop_tx,
	.start_tx	= xr17v358_start_tx,
	.stop_rx	= xr17v358_stop_rx,
	.startup	= xr17v358_startup,
	.shutdown	= xr17v358_shutdown,
	.set_termios	= xr17v358_set_termios,
	.type		= xr17v358_type,
	.release_port	= xr17v358_release_port,
	.request_port	= xr17v358_request_port,
	.config_port	= xr17v358_config_port,
	.verify_port	= xr17v358_verify_port,
};

/* -----------------------------------------------------------------------
 * Interrupt handler
 *
 * The XR17V358 multiplexes all eight channels through a single PCIe IRQ.
 * The global ISR registers (isr0 / isr1) indicate which channel(s) fired.
 * ---------------------------------------------------------------------- */

static void xr17v358_handle_rx(struct xr17v358_port *xp)
{
	struct uart_port *port = &xp->uport;
	struct tty_port *tport = &port->state->port;
	u8 lsr;

	while ((lsr = CHAN_RD(xp->regs, lsr)) & XR17V358_LSR_DR) {
		u8 ch = CHAN_RD(xp->regs, rhr_thr_dll);
		u8 flag = TTY_NORMAL;

		port->icount.rx++;

		if (lsr & XR17V358_LSR_BI) {
			port->icount.brk++;
			flag = TTY_BREAK;
		} else if (lsr & XR17V358_LSR_PE) {
			port->icount.parity++;
			flag = TTY_PARITY;
		} else if (lsr & XR17V358_LSR_FE) {
			port->icount.frame++;
			flag = TTY_FRAME;
		} else if (lsr & XR17V358_LSR_OE) {
			port->icount.overrun++;
		}

		if (uart_handle_sysrq_char(port, ch))
			continue;

		uart_insert_char(port, lsr, XR17V358_LSR_OE, ch, flag);
	}

	tty_flip_buffer_push(tport);
}

static void xr17v358_handle_tx(struct xr17v358_port *xp)
{
	struct uart_port *port = &xp->uport;
	u8 ch;

	/*
	 * uart_port_tx_limited() handles x_char, uart_tx_stopped(), FIFO
	 * draining, uart_write_wakeup(), and stop_tx() automatically.
	 */
	uart_port_tx_limited(port, ch, XR17V358_FIFO_SIZE,
		/* tx_ready  */ true,
		/* put_char  */ CHAN_WR(xp->regs, rhr_thr_dll, ch),
		/* tx_done   */ ({}));
}

static void xr17v358_handle_chan(struct xr17v358_port *xp)
{
	struct uart_port *port = &xp->uport;
	u8 isr;

	uart_port_lock(port);

	do {
		isr = CHAN_RD(xp->regs, isr_fcr);
		if (isr & XR17V358_ISR_NOINT)
			break;

		switch ((isr & XR17V358_ISR_SRC_MASK) >> 1) {
		case XR17V358_ISR_SRC_RXRDY:
		case XR17V358_ISR_SRC_RXTMO:
			xr17v358_handle_rx(xp);
			break;
		case XR17V358_ISR_SRC_TXRDY:
			xr17v358_handle_tx(xp);
			break;
		case XR17V358_ISR_SRC_RXLST:
			/* Read LSR to clear the interrupt */
			(void)CHAN_RD(xp->regs, lsr);
			break;
		case XR17V358_ISR_SRC_MDMCHG:
			/* Read MSR to clear the interrupt */
			(void)CHAN_RD(xp->regs, msr);
			break;
		default:
			break;
		}
	} while (!(isr & XR17V358_ISR_NOINT));

	uart_port_unlock(port);
}

static irqreturn_t xr17v358_irq(int irq, void *data)
{
	struct xr17v358_dev *xdev = data;
	irqreturn_t ret = IRQ_NONE;
	u8 isr0, isr1;
	int ch;

	isr0 = GLBL_RD(xdev->glbl, isr0);
	isr1 = GLBL_RD(xdev->glbl, isr1);

	/* Channels 0-3 are in isr0, channels 4-7 are in isr1 */
	for (ch = 0; ch < 4; ch++) {
		if (isr0 & XR17V358_GLBL_ISR_CH_INT(ch)) {
			xr17v358_handle_chan(&xdev->port[ch]);
			ret = IRQ_HANDLED;
		}
	}
	for (ch = 4; ch < XR17V358_NR_CHAN; ch++) {
		if (isr1 & XR17V358_GLBL_ISR_CH_INT(ch)) {
			xr17v358_handle_chan(&xdev->port[ch]);
			ret = IRQ_HANDLED;
		}
	}

	return ret;
}

/* -----------------------------------------------------------------------
 * uart_driver
 * ---------------------------------------------------------------------- */

static struct uart_driver xr17v358_uart_driver = {
	.owner		= THIS_MODULE,
	.driver_name	= DRIVER_NAME,
	.dev_name	= "ttyXR",
	.major		= 0,	/* dynamic major */
	.minor		= 0,
	.nr		= XR17V358_NR_CHAN * 8, /* up to 8 boards */
};

/* -----------------------------------------------------------------------
 * PCI probe
 * ---------------------------------------------------------------------- */

static int xr17v358_probe(struct pci_dev *pdev,
			  const struct pci_device_id *ent)
{
	struct xr17v358_dev *xdev;
	int ret, ch;

	ret = pcim_enable_device(pdev);
	if (ret)
		return ret;

	ret = pcim_iomap_regions(pdev, BIT(0), DRIVER_NAME);
	if (ret)
		return ret;

	xdev = devm_kzalloc(&pdev->dev, sizeof(*xdev), GFP_KERNEL);
	if (!xdev)
		return -ENOMEM;

	xdev->pdev = pdev;
	xdev->base = pcim_iomap_table(pdev)[0];
	xdev->glbl = xdev->base + XR17V358_GLBL_BASE;
	xdev->udrv = &xr17v358_uart_driver;

	/* Allocate a unique device index for non-overlapping line numbers */
	xdev->dev_idx = ida_alloc(&xr17v358_ida, GFP_KERNEL);
	if (xdev->dev_idx < 0)
		return xdev->dev_idx;

	pci_set_master(pdev);
	pci_set_drvdata(pdev, xdev);

	/* Initialise and register one uart_port per channel */
	for (ch = 0; ch < XR17V358_NR_CHAN; ch++) {
		struct xr17v358_port *xp = &xdev->port[ch];
		struct uart_port *up = &xp->uport;

		xp->chan_idx = ch;
		xp->regs     = xdev->base + ch * XR17V358_CHAN_STRIDE;

		up->dev        = &pdev->dev;
		up->irq        = pdev->irq;
		up->uartclk    = xr17v358_uartclk;
		up->fifosize   = XR17V358_FIFO_SIZE;
		up->iotype     = UPIO_MEM;
		up->flags      = UPF_SHARE_IRQ | UPF_BOOT_AUTOCONF;
		up->ops        = &xr17v358_ops;
		/* Assign globally unique line numbers across multiple boards */
		up->line       = xdev->dev_idx * XR17V358_NR_CHAN + ch;
		up->mapbase    = pci_resource_start(pdev, 0) +
				 ch * XR17V358_CHAN_STRIDE;
		up->membase    = (u8 __iomem *)xp->regs;

		spin_lock_init(&up->lock);

		ret = uart_add_one_port(&xr17v358_uart_driver, up);
		if (ret) {
			dev_err(&pdev->dev,
				"failed to add uart port %d: %d\n", ch, ret);
			goto err_remove_ports;
		}
	}

	/* Register a shared IRQ handler */
	ret = request_irq(pdev->irq, xr17v358_irq, IRQF_SHARED,
			  DRIVER_NAME, xdev);
	if (ret) {
		dev_err(&pdev->dev, "failed to request IRQ %d: %d\n",
			pdev->irq, ret);
		goto err_remove_ports;
	}

	dev_info(&pdev->dev, "XR17V358: %d channels (lines %d-%d), IRQ %d\n",
		 XR17V358_NR_CHAN,
		 xdev->dev_idx * XR17V358_NR_CHAN,
		 xdev->dev_idx * XR17V358_NR_CHAN + XR17V358_NR_CHAN - 1,
		 pdev->irq);
	return 0;

err_remove_ports:
	while (--ch >= 0)
		uart_remove_one_port(&xr17v358_uart_driver, &xdev->port[ch].uport);
	ida_free(&xr17v358_ida, xdev->dev_idx);
	return ret;
}

/* -----------------------------------------------------------------------
 * PCI remove
 * ---------------------------------------------------------------------- */

static void xr17v358_remove(struct pci_dev *pdev)
{
	struct xr17v358_dev *xdev = pci_get_drvdata(pdev);
	int ch;

	free_irq(pdev->irq, xdev);

	for (ch = 0; ch < XR17V358_NR_CHAN; ch++)
		uart_remove_one_port(&xr17v358_uart_driver,
				     &xdev->port[ch].uport);

	ida_free(&xr17v358_ida, xdev->dev_idx);
}

/* -----------------------------------------------------------------------
 * PCI device table
 * ---------------------------------------------------------------------- */

static const struct pci_device_id xr17v358_pci_ids[] = {
	{ PCI_DEVICE(XR17V358_PCI_VENDOR_ID, XR17V358_PCI_DEVICE_ID) },
	{ }
};
MODULE_DEVICE_TABLE(pci, xr17v358_pci_ids);

/* -----------------------------------------------------------------------
 * PCI driver
 * ---------------------------------------------------------------------- */

static struct pci_driver xr17v358_pci_driver = {
	.name     = DRIVER_NAME,
	.id_table = xr17v358_pci_ids,
	.probe    = xr17v358_probe,
	.remove   = xr17v358_remove,
};

/* -----------------------------------------------------------------------
 * Module init / exit
 * ---------------------------------------------------------------------- */

static int __init xr17v358_init(void)
{
	int ret;

	ret = uart_register_driver(&xr17v358_uart_driver);
	if (ret)
		return ret;

	ret = pci_register_driver(&xr17v358_pci_driver);
	if (ret)
		uart_unregister_driver(&xr17v358_uart_driver);

	return ret;
}

static void __exit xr17v358_exit(void)
{
	pci_unregister_driver(&xr17v358_pci_driver);
	uart_unregister_driver(&xr17v358_uart_driver);
	ida_destroy(&xr17v358_ida);
}

module_init(xr17v358_init);
module_exit(xr17v358_exit);

MODULE_AUTHOR("cybercritter");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL v2");
