/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * xr17v358.h - Register map and constants for the MaxLinear XR17V358
 *              8-channel PCIe UART.
 *
 * The XR17V358 exposes its registers through BAR0 (memory-mapped I/O).
 * Each of the eight channels occupies a 1 KiB (0x400-byte) window:
 *
 *   Channel N base = BAR0 + N * XR17V358_CHAN_STRIDE
 *
 * Device-wide (global) registers live above the per-channel area.
 *
 * Register access is done by casting the mapped virtual address to a
 * pointer to the appropriate register-map struct defined below.  All
 * registers are 8-bit wide and must be accessed as bytes.
 */

#ifndef _XR17V358_H_
#define _XR17V358_H_

#include <linux/types.h>

/* -----------------------------------------------------------------------
 * PCI identity
 * ---------------------------------------------------------------------- */

#define XR17V358_PCI_VENDOR_ID		0x13A8
#define XR17V358_PCI_DEVICE_ID		0x0158

/* -----------------------------------------------------------------------
 * Layout constants
 * ---------------------------------------------------------------------- */

#define XR17V358_NR_CHAN		8	/* channels per device        */
#define XR17V358_CHAN_STRIDE		0x400	/* bytes between channel bases */
#define XR17V358_FIFO_SIZE		128	/* bytes per TX/RX FIFO       */

/* BAR0 offset of the device-wide register block */
#define XR17V358_GLBL_BASE		(XR17V358_NR_CHAN * XR17V358_CHAN_STRIDE)

/* -----------------------------------------------------------------------
 * Per-channel register map
 *
 * One instance of this struct overlays exactly XR17V358_CHAN_STRIDE bytes
 * starting at (BAR0_virt + channel * XR17V358_CHAN_STRIDE).
 * ---------------------------------------------------------------------- */

struct xr17v358_chan_regs {
	/*
	 * 0x00 – RHR / THR
	 *   Read : Receive  Holding Register (DLAB=0)
	 *   Write: Transmit Holding Register (DLAB=0)
	 *   Read/Write: DLL – Divisor Latch LSB (DLAB=1)
	 */
	u8 rhr_thr_dll;

	/*
	 * 0x01 – IER / DLM
	 *   Interrupt Enable Register (DLAB=0)
	 *   Divisor Latch MSB          (DLAB=1)
	 */
	u8 ier_dlm;

	/*
	 * 0x02 – ISR (read) / FCR (write)
	 *   Interrupt Status Register  (read)
	 *   FIFO Control Register      (write)
	 */
	u8 isr_fcr;

	/* 0x03 – LCR: Line Control Register */
	u8 lcr;

	/* 0x04 – MCR: Modem Control Register */
	u8 mcr;

	/* 0x05 – LSR: Line Status Register (read-only) */
	u8 lsr;

	/* 0x06 – MSR: Modem Status Register (read-only) */
	u8 msr;

	/* 0x07 – SPR: Scratch Pad Register */
	u8 spr;

	/* 0x08 – TXTRG: TX FIFO Trigger Level */
	u8 txtrg;

	/* 0x09 – RXTRG: RX FIFO Trigger Level */
	u8 rxtrg;

	/* 0x0A – FCTL: Flow Control Register */
	u8 fctl;

	/* 0x0B – FSTAT: FIFO Size / Status (read-only) */
	u8 fstat;

	/* 0x0C – XONOFF: Xon/Xoff Enable */
	u8 xonoff;

	u8 _pad0[3];			/* 0x0D–0x0F reserved */

	/* 0x10 – XON1: Xon  Character 1 */
	u8 xon1;

	/* 0x11 – XON2: Xon  Character 2 */
	u8 xon2;

	/* 0x12 – XOFF1: Xoff Character 1 */
	u8 xoff1;

	/* 0x13 – XOFF2: Xoff Character 2 */
	u8 xoff2;

	u8 _pad1[4];			/* 0x14–0x17 reserved */

	/* 0x18 – FSYNC: Flow Synchronization */
	u8 fsync;

	u8 _pad2[7];			/* 0x19–0x1F reserved */

	/* 0x20 – TXCNT_LO: TX byte count LSB (read-only) */
	u8 txcnt_lo;

	/* 0x21 – TXCNT_HI: TX byte count MSB (read-only) */
	u8 txcnt_hi;

	/* 0x22 – RXCNT_LO: RX byte count LSB (read-only) */
	u8 rxcnt_lo;

	/* 0x23 – RXCNT_HI: RX byte count MSB (read-only) */
	u8 rxcnt_hi;

	u8 _pad3[0x1DC];		/* 0x24–0x1FF reserved */

	/* 0x200 – EFR: Enhanced Feature Register (LCR=0xBF to unlock) */
	u8 efr;

	u8 _pad4[3];			/* 0x201–0x203 reserved */

	/* 0x204 – FCTR: Feature Control Register */
	u8 fctr;

	u8 _pad5[0x1FB];		/* 0x205–0x3FF pad to CHAN_STRIDE */
} __packed;

static_assert(sizeof(struct xr17v358_chan_regs) == XR17V358_CHAN_STRIDE,
	      "xr17v358_chan_regs size mismatch");

/* -----------------------------------------------------------------------
 * Device-wide (global) register map
 *
 * Overlays the memory at BAR0 + XR17V358_GLBL_BASE.
 * ---------------------------------------------------------------------- */

struct xr17v358_glbl_regs {
	/* 0x00 – ISR0: Global Interrupt Status byte 0 (channels 0-3) */
	u8 isr0;

	/* 0x01 – ISR1: Global Interrupt Status byte 1 (channels 4-7) */
	u8 isr1;

	u8 _pad0[6];			/* 0x02–0x07 reserved */

	/* 0x08 – CLKPRE: Clock Prescaler */
	u8 clkpre;

	u8 _pad1[7];			/* 0x09–0x0F reserved */

	/* 0x10 – REVID: Revision ID (read-only) */
	u8 revid;
} __packed;

/* -----------------------------------------------------------------------
 * IER – Interrupt Enable Register bit definitions
 * ---------------------------------------------------------------------- */

#define XR17V358_IER_RXRDY	BIT(0)	/* RX data available           */
#define XR17V358_IER_TXRDY	BIT(1)	/* TX holding register empty   */
#define XR17V358_IER_RXLST	BIT(2)	/* RX line status change       */
#define XR17V358_IER_MDMCHG	BIT(3)	/* Modem status change         */
#define XR17V358_IER_SLEEP	BIT(4)	/* Sleep mode (enhanced)       */
#define XR17V358_IER_XOFF	BIT(5)	/* Xoff received               */
#define XR17V358_IER_RTS	BIT(6)	/* RTS de-assertion            */
#define XR17V358_IER_CTS	BIT(7)	/* CTS de-assertion            */

/* -----------------------------------------------------------------------
 * ISR – Interrupt Status Register (read-only)
 * ---------------------------------------------------------------------- */

#define XR17V358_ISR_NOINT	BIT(0)	/* 1 = no interrupt pending    */
#define XR17V358_ISR_SRC_MASK	0x3E	/* interrupt source bits [5:1] */
#define XR17V358_ISR_FIFOEN	0xC0	/* FIFO enabled status [7:6]   */

/* ISR source codes (bits [5:1], shifted right by 1) */
#define XR17V358_ISR_SRC_MDMCHG	0x00	/* modem status change         */
#define XR17V358_ISR_SRC_TXRDY	0x01	/* TX holding register empty   */
#define XR17V358_ISR_SRC_RXRDY	0x02	/* RX data available           */
#define XR17V358_ISR_SRC_RXLST	0x03	/* RX line status              */
#define XR17V358_ISR_SRC_RXTMO	0x06	/* RX timeout                  */
#define XR17V358_ISR_SRC_XOFF	0x08	/* Xoff / special character    */
#define XR17V358_ISR_SRC_CTSRTS	0x10	/* CTS/RTS de-asserted         */

/* -----------------------------------------------------------------------
 * FCR – FIFO Control Register (write-only)
 * ---------------------------------------------------------------------- */

#define XR17V358_FCR_FIFO_EN	BIT(0)	/* enable TX and RX FIFOs      */
#define XR17V358_FCR_RXFIFO_RST	BIT(1)	/* reset / clear RX FIFO       */
#define XR17V358_FCR_TXFIFO_RST	BIT(2)	/* reset / clear TX FIFO       */
#define XR17V358_FCR_DMA_MODE	BIT(3)	/* DMA mode select             */
#define XR17V358_FCR_TXTRG_MASK	0x30	/* TX trigger level [5:4]      */
#define XR17V358_FCR_RXTRG_MASK	0xC0	/* RX trigger level [7:6]      */

/* -----------------------------------------------------------------------
 * LCR – Line Control Register
 * ---------------------------------------------------------------------- */

#define XR17V358_LCR_WLS_MASK	0x03	/* word length [1:0]           */
#define XR17V358_LCR_WLS_5	0x00	/*   5 bits                    */
#define XR17V358_LCR_WLS_6	0x01	/*   6 bits                    */
#define XR17V358_LCR_WLS_7	0x02	/*   7 bits                    */
#define XR17V358_LCR_WLS_8	0x03	/*   8 bits                    */
#define XR17V358_LCR_STB	BIT(2)	/* stop bits (0=1, 1=1.5/2)    */
#define XR17V358_LCR_PEN	BIT(3)	/* parity enable               */
#define XR17V358_LCR_EPS	BIT(4)	/* even parity select          */
#define XR17V358_LCR_STP	BIT(5)	/* stick parity                */
#define XR17V358_LCR_BRK	BIT(6)	/* set break condition         */
#define XR17V358_LCR_DLAB	BIT(7)	/* divisor latch access        */
#define XR17V358_LCR_EFR_MAGIC	0xBF	/* magic value to access EFR   */

/* -----------------------------------------------------------------------
 * MCR – Modem Control Register
 * ---------------------------------------------------------------------- */

#define XR17V358_MCR_DTR	BIT(0)	/* DTR output                  */
#define XR17V358_MCR_RTS	BIT(1)	/* RTS output                  */
#define XR17V358_MCR_OUT1	BIT(2)	/* OP1 output                  */
#define XR17V358_MCR_OUT2	BIT(3)	/* OP2 output / global IRQ en  */
#define XR17V358_MCR_LOOP	BIT(4)	/* local loopback mode         */
#define XR17V358_MCR_XON_ANY	BIT(5)	/* Xon-any (enhanced)          */
#define XR17V358_MCR_IRDA	BIT(6)	/* IrDA mode                   */
#define XR17V358_MCR_CLKDIV	BIT(7)	/* clock divisor               */

/* -----------------------------------------------------------------------
 * LSR – Line Status Register (read-only)
 * ---------------------------------------------------------------------- */

#define XR17V358_LSR_DR		BIT(0)	/* data ready                  */
#define XR17V358_LSR_OE		BIT(1)	/* overrun error               */
#define XR17V358_LSR_PE		BIT(2)	/* parity error                */
#define XR17V358_LSR_FE		BIT(3)	/* framing error               */
#define XR17V358_LSR_BI		BIT(4)	/* break interrupt             */
#define XR17V358_LSR_THRE	BIT(5)	/* TX holding register empty   */
#define XR17V358_LSR_TEMT	BIT(6)	/* TX empty (FIFO + shift reg) */
#define XR17V358_LSR_FIFOERR	BIT(7)	/* at least one RX FIFO error  */

/* -----------------------------------------------------------------------
 * MSR – Modem Status Register (read-only)
 * ---------------------------------------------------------------------- */

#define XR17V358_MSR_DCTS	BIT(0)	/* delta CTS                   */
#define XR17V358_MSR_DDSR	BIT(1)	/* delta DSR                   */
#define XR17V358_MSR_TERI	BIT(2)	/* trailing edge RI            */
#define XR17V358_MSR_DDCD	BIT(3)	/* delta DCD                   */
#define XR17V358_MSR_CTS	BIT(4)	/* CTS state                   */
#define XR17V358_MSR_DSR	BIT(5)	/* DSR state                   */
#define XR17V358_MSR_RI		BIT(6)	/* RI state                    */
#define XR17V358_MSR_DCD	BIT(7)	/* DCD state                   */

/* -----------------------------------------------------------------------
 * EFR – Enhanced Feature Register (LCR must equal XR17V358_LCR_EFR_MAGIC)
 * ---------------------------------------------------------------------- */

#define XR17V358_EFR_SWFLOW_TX	0x03	/* SW flow ctrl TX [1:0]       */
#define XR17V358_EFR_SWFLOW_RX	0x0C	/* SW flow ctrl RX [3:2]       */
#define XR17V358_EFR_ENH_EN	BIT(4)	/* enhanced feature enable     */
#define XR17V358_EFR_SPC_CHAR	BIT(5)	/* special character detect    */
#define XR17V358_EFR_AUTO_RTS	BIT(6)	/* automatic RTS flow control  */
#define XR17V358_EFR_AUTO_CTS	BIT(7)	/* automatic CTS flow control  */

/* -----------------------------------------------------------------------
 * Global ISR – per-channel bits inside isr0 / isr1
 *
 * isr0 covers channels 0-3, isr1 covers channels 4-7.
 * Within each byte, bits 1:0 carry the interrupt state for each channel.
 * ---------------------------------------------------------------------- */

#define XR17V358_GLBL_ISR_CH_INT(ch)	BIT(((ch) % 4) * 2)

#endif /* _XR17V358_H_ */
