Example Card Analysis
==============================================================================

/**
 * @file example-8GBcard.md
 * @author Brian E Tovar
 * @modified 24 Sep 2012
 */


Overview
------------------------------------------------------------------------------

This document is a comparison between the debug log for this class and an 
independent hardware source. In this example, the built-in Apple Internal 
Memory Card Reader on a newer-model Macintosh is used. The System Profiler 
app provides access to this information.


Mac OSX System Profiler (Card Reader Pane)
------------------------------------------------------------------------------

Built in SD Card Reader:

  Vendor ID:	          0x14e4
  Device ID:	          0x16bc
  Subsystem Vendor ID:	0x14e4
  Subsystem ID:	        0x0000
  Revision:	            0x0010
  Link Width:	          x1
  Link Speed:	          2.5 GT/s

SDHC Card (Class 4):

  Product Name:	          SDSU08G
  Manufacturer ID:	      0x03
  Revision:	              8.0
  Serial Number:	        33855119
  Manufacturing Date:	    2012-05
  Specification Version:	3.0
  Capacity:	              7.95 GB (7,948,206,080 bytes)
  Removable Media:	      Yes
  BSD Name:	              disk2
  Partition Map Type:	    MBR (Master Boot Record)
  S.M.A.R.T. status:	    Not Supported
  Volumes:
    PIKACHU:
      Available:	  7.94 GB (7,935,361,024 bytes)
      Capacity:	    7.94 GB (7,944,011,776 bytes)
      Writable:	    Yes
      File System:	MS-DOS FAT32
      BSD Name:	    disk2s1
      Mount Point:	/Volumes/PIKACHU
      iocontent:	  DOS_FAT_32


SDIO Debug over SerialUSB
------------------------------------------------------------------------------

*** Starting SDMC test ***
SDIO_DBG: Card detected
SDIO_DBG: Powered on
SDIO_DBG: Sending CMD0
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Command sent
SDIO_DBG: Card should be in IDLE state
SDIO_DBG: Initializing card
SDIO_DBG: Sending CMD8
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Response timeout
SDIO_ERR: Command mismatch, response from CMD0
SDIO_ERR: Unexpected response status
SDIO_DBG: Sending CMD0
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Command sent
SDIO_DBG: Card should be in IDLE state
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
SDIO_DBG: Sending ACMD41
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Ignoring CRC for ACMD41
SDIO_DBG: Response from ACMD41
SDIO_DBG: Volatge window of the card 0xFF80
SDIO_DBG: This is the first ACMD41
SDIO_DBG: Sending CMD55
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Response received
SDIO_DBG: Response from CMD55
SDIO_DBG: Sending ACMD41
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Ignoring CRC for ACMD41
SDIO_DBG: Response from ACMD41
SDIO_DBG: OCR busy
SDIO_DBG: Sending CMD55
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Response received
SDIO_DBG: Response from CMD55
SDIO_DBG: AppCmd not enabled, try again
SDIO_DBG: Sending CMD55
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Response received
SDIO_DBG: Response from CMD55
SDIO_DBG: Sending ACMD41
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Ignoring CRC for ACMD41
SDIO_DBG: Response from ACMD41
SDIO_DBG: OCR busy
SDIO_DBG: Sending CMD55
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Response received
SDIO_DBG: Response from CMD55
SDIO_DBG: Sending ACMD41
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Ignoring CRC for ACMD41
SDIO_DBG: Response from ACMD41
SDIO_DBG: OCR busy
SDIO_DBG: Sending CMD55
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Response received
SDIO_DBG: Response from CMD55
SDIO_DBG: Sending ACMD41
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Ignoring CRC for ACMD41
SDIO_DBG: Response from ACMD41
SDIO_DBG: OCR busy
SDIO_DBG: Sending CMD55
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Response received
SDIO_DBG: Response from CMD55
SDIO_DBG: Sending ACMD41
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Ignoring CRC for ACMD41
SDIO_DBG: Response from ACMD41
SDIO_DBG: Card is ready <-------------
SDIO_DBG: Valid volatge window
SDIO_DBG: Card supports SDHC and SDXC
SDIO_DBG: Initialization complete
SDIO_DBG: Getting Card Identification Number
SDIO_DBG: Sending CMD2
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Response received
SDIO_DBG: Response from CMD2
SDIO_DBG: Manufaturer ID 3
SDIO_DBG: Application ID SD
SDIO_DBG: Product name SU08G
SDIO_DBG: Product revision 8.0
SDIO_DBG: Serial number 33855119
SDIO_DBG: Manufacture date 5/2012
SDIO_DBG: Getting new Relative Card Address
SDIO_DBG: Sending CMD3
SDIO_DBG: Command active
SDIO_DBG: Wait for interrupt... Response received
SDIO_DBG: Response from CMD3
SDIO_DBG: New RCA is 0xAAAA
SDIO_DBG: RESP1 0xAAAA0520
SDIO_DBG: Card should now be in STANDBY state
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
*** SDMC test complete ***


Discussion
------------------------------------------------------------------------------

In previous tests, the card was not ready in the alotted number of tries. This 
test gave the correct CID information verified from an independent source. The 
next step is to verify the CSD register values with some other source since 
Apple hardware does not provide this information with their hardware tools. 
`blkid` has been rumored to fit this bill.