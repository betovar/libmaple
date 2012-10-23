Example Card Analysis
==============================================================================

/**
 * @file sdcard-example.md
 * @author Brian E Tovar
 * @modified 22 Oct 2012
 */

An example of the SD Card library is performed on an 8GB card with serial 
debugging over USB. Checkout out the commit tagged `sdio-demo` to try it out.

* Author: Brian E Tovar
* Email:  betovar@leaflabs.com
* Date:   22 Oct 2012


Overview
==============================================================================

This document is a comparison between the debug log for the SD Card library 
and an independent hardware source. In this example, the CID and CSD registers 
are obtained from an Android mobile phone connect to a personal computer with 
USB debugging. Details on this process are described in 
`sdcard-verification.md`.


Results
==============================================================================


SDIO Debug over SerialUSB
-------------------------

*** Starting SDMC test ***
SDIO_DBG: Card detected
SDIO_DBG: Powered on
SDIO_DBG: Sending CMD0
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Command sent
SDIO_DBG: Response from CMD0
SDIO_DBG: Sending CMD0
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Command sent
SDIO_DBG: Response from CMD0
SDIO_DBG: Sending CMD0
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Command sent
SDIO_DBG: Response from CMD0
SDIO_ERR: Card not in IDLE state
SDIO_DBG: Initializing card
SDIO_DBG: Sending CMD8
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Response received
SDIO_DBG: Response from CMD8
SDIO_DBG: Valid check pattern
SDIO_DBG: Valid supplied voltage
SDIO_DBG: Interface condition check passed
SDIO_DBG: This is the inquiry ACMD41
SDIO_DBG: Sending CMD55
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Response received
SDIO_DBG: Response from CMD55
SDIO_DBG: AppCmd enabled
SDIO_DBG: Sending ACMD41
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Ignoring CRC for ACMD41
SDIO_DBG: Response from ACMD41
SDIO_DBG: This is the first ACMD41
SDIO_DBG: Sending CMD55
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Response received
SDIO_DBG: Response from CMD55
SDIO_DBG: AppCmd enabled
SDIO_DBG: Sending ACMD41
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Ignoring CRC for ACMD41
SDIO_DBG: Response from ACMD41
SDIO_DBG: OCR busy
SDIO_DBG: Sending CMD55
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Response received
SDIO_DBG: Response from CMD55
SDIO_DBG: AppCmd enabled
SDIO_DBG: Sending ACMD41
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Ignoring CRC for ACMD41
SDIO_DBG: Response from ACMD41
SDIO_DBG: Card is ready <-----------------------------------------
SDIO_DBG: Valid voltage window
SDIO_DBG: Card supports SDHC and SDXC
SDIO_DBG: Initialization complete
SDIO_DBG: Getting Card Identification Number
SDIO_DBG: Sending CMD2
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Response received
SDIO_DBG: Response from CMD2
SDIO_DBG: Getting new Relative Card Address
SDIO_DBG: Sending CMD3
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Response received
SDIO_DBG: Response from CMD3
SDIO_ERR: Card now in an UNKNOWN state
SDIO_DBG: New RCA is 0xAAAA
SDIO_DBG: Getting Card Specific Data
SDIO_DBG: Sending CMD9
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Response received
SDIO_DBG: Response from CMD9
SDIO_DBG: Card version 2.0
SDIO_DBG: TAAC 14
SDIO_DBG: NSAC 0
SDIO_DBG: TRAN_SPEED 50
SDIO_DBG: CCC 1461
SDIO_DBG: READ_BL_LEN 9
SDIO_DBG: READ_BL_PARTIAL 0
SDIO_DBG: WRITE_BLK_MISALIGN 0
SDIO_DBG: READ_BLK_MISALIGN 0
SDIO_DBG: DSR_IMP 0
SDIO_DBG: C_SIZE 15159
SDIO_DBG: Getting Card Identification Number
SDIO_DBG: Sending CMD10
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Response received
SDIO_DBG: Response from CMD10
SDIO_DBG: MID 3
SDIO_DBG: OID SD
SDIO_DBG: PNM SU08G
SDIO_DBG: PRV 8.0
SDIO_DBG: PSN 33855119
SDIO_DBG: MDT 5/2012
*** SDMC test complete ***


Comparison of Data
------------------

The first four lines in each set are printed out over SerialUSB on the 
maple_native. 
The longer fifth line is from the Andriod SDK `adb` shell.

CID1: 0x03534453
CID2: 0x55303847
CID3: 0x80020496
CID4: 0x8F00C560
CID:  0x03534453 55303847 80020496 8f00c560

CSD1: 0x400E0032
CSD2: 0x5B590000
CSD3: 0x3B377F80
CSD4: 0x0A4040AE
CSD:  0x400e003 25b590000 3b377f80 0a4040ae


Discussion
==============================================================================

These results from the maple-native match those taken from the independent 
source. It is possible that these results are complete and reliable due to the 
fact that this 8GB card is new, and menufactured by SanDisk. With the low cost 
of cards like this one weighed against the number of knock-off cards out there 
in the market, it's not worth saving the extra cents on an unreliable card.