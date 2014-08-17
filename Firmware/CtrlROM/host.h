#ifndef HOST_H
#define HOST_H

#define HOSTBASE 0xFFFFFF40
#define HW_HOST(x) *(volatile unsigned int *)(HOSTBASE+x)

/* SPI registers */

/* DIP switches, bits 9 downto 0 */
#define HW_HOST_SW 0x0

/* Control the host:
 *  Bit 0: 1=> Reset, 0=> Run
 *  Bit 1: 1=> Inhibit, 0=> Run
 *  Bit 2: 1=> Ctrl owns SD card, 0=> Host owns SD card
*/
#define HW_HOST_CTRL 0x04
#define HW_HOST_CTRLF_RESET 1
#define HW_HOST_CTRLF_INHIBIT 2
#define HW_HOST_CTRLF_SDCARD 4

/* Boot data.
   Blocks until the previous byte has been read,
   so it's safe to just deluge this register with data. */
#define HW_HOST_BOOTDATA 0x08

#endif

