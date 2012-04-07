#ifdef SD_SDIO_1BIT_BUS_PROTOCOL || SD_SDIO_4BIT_BUS_PROTOCOL
// Basic Commands (class 0)
/** CMD0 - Resets all cards to idle state */
static uint8 GO_IDLE_STATE = 0x00;
/** CMD1 - reserved MMC */
//static uint8 SEND_OP_COND = 0X01;
/** CMD2 - Asks any card to send the CID numbers on the CMD line */
static uint8 ALL_SEND_CID = 0x02;
/** CMD3 - Ask the card to publish a new relative address */
static uint8 SEND_RELATIVE_ADDR = 0x03;
/** CMD4 - Programs the DSR of all cards */
static uint8 SET_DSR = 0x04; 
/** CMD5 - reserved for SDIO cards */
//static uint8 CMD5 = 0x05;
/** CMD6 - reserved */
//static uint8 CMD6 = 0x06;
/** CMD7 - toggles a card between the stand-by and transfer
 * states or between the programming and disconnect states */
static uint8 SELECT_DESELECT_CARD = 0x07;
/** CMD8 - SD Memory Card interface condition */
//static uint8 SEND_IF_COND = 0x08;
/** CMD9 - Addressed card sends its card-specific data (CSD)
 * on the CMD line */
static uint8 SEND_CSD = 0x09;
/** CMD10 - Addressed card sends its card identification (CID)
 * on the CMD line */
static uint8 SEND_CID = 0x0A;
/** CMD11 - Switch to 1.8V bus signaling level */
//static uint8  READ_DAT_UNTIL_STOP = 0X0B;
//static uint8 VOLTAGE_SWITCH = 0x0B;
/** CMD12 - Forces the card to stop transmission */
static uint8 STOP_TRANSMISSION = 0x0C;
/** CMD13 - Addressed card sends its status register */
static uint8 SEND_STATUS = 0x0D;
/** CMD14 - Reserved */
//static uint8 CMD14 = 0x0E;
/** CMD15 - Sends an addressed card into the Inactive State */
static uint8 GO_INACTIVE_STATE = 0x0F;
#endif

//#ifdef SD_SPI_BUS_PROTCOL
//// Block-Oriented Read Commands (class 2)
///** SET_BLOCKLEN - In the case of a Standard Capacity SD Memory Card, this command sets the block length (in bytes) for all following block commands (read, write, lock). Default block length is fixed to 512 Bytes. Set length is valid for memory access commands only if partial block read operation are allowed in CSD.
//In the case of SDHC and SDXC Cards, block length set by CMD16 command doen't affect memory read and write commands. Always 512 Bytes fixed block length is used. This command is effective for LOCK_UNLOCK command.
//In both cases, if block length is set larger than 512Bytes, the card sets the BLOCK_LEN_ERROR bit. In DDR50 mode, data is sampled on both edges of the clock. Therefore, block length shall always be even */
//static uint8 CMD16 = 0X00;
///** CMD17 - read a single data block from the card */
//static uint8 READ_SINGLE_BLOCK = 0X11;
///** CMD18 - Continuously transfers data blocks from card to
// * host until interrupted by a STOP_TRANSMISSION command. Block length is
// * specified the same as READ_SINGLE_BLOCK command. */
//static uint8 READ_MULTIPLE_BLOCK = 0X00;
///** CMD19 - 64 bytes tuning pattern is sent for SDR50
// * and SDR104 */
//static uint8 SEND_TUNING_BLOCK = 0X00;
///** CMD20 - Speed Class control command */
//static uint8 SPEED_CLASS_CONTROL = 0X00;
///** Reserved */
//static uint8 CMD21 = 0X00;
///** Reserved */
//static uint8 CMD22 = 0X00;
///** CMD23 - Specify block count for CMD18 and CMD25 */
//static uint8 SET_BLOCK_COUNT = 0X00;
//#endif
//
//#ifdef SD_SPI_BUS_PROTCOL
////Block-Oriented Write Commands (class 4)
///** WRITE_BLOCK - write a single data block to the card */
//static uint8 CMD24 = 0X18;
///** WRITE_MULTIPLE_BLOCK - write blocks of data until a STOP_TRANSMISSION */
//static uint8 CMD25 = 0X19;
///** Reserved For Manufacturer */
//static uint8 CMD26 = 0X00;
///** PROGRAM_CSD - Programming of the programmable bits of the CSD */
//static uint8 CMD27 = 0X00;
//#endif
//
//#ifdef SD_SPI_BUS_PROTCOL
//// Block Oriented Write Protection Commands (class 6)
///** SET_WRITE_PROT */
//static uint8 CMD28 = 0X00;
///** CLR_WRITE_PROT */
//static uint8 CMD29 = 0X00;
///** SEND_WRITE_PROT */
//static uint8 CMD30 = 0X00;
///** Reserved */
//static uint8 CMD31 = 0X00;
//#endif
//
//#ifdef SD_SPI_BUS_PROTCOL
//// Erase Commands (class 5)
///** ERASE_WR_BLK_START - sets the address of the first block to be erased */
//static uint8 CMD32 = 0X20;
///** ERASE_WR_BLK_END - sets the address of the last block of the continuous
//    range to be erased*/
//static uint8 CMD33 = 0X21;
///** ERASE - erase all previously selected blocks */
//static uint8 CMD38 = 0X26;
//#endif
//
//#ifdef SD_SPI_BUS_PROTCOL
//// Lock Card (class 7)
///** Reserved for Security Specification */
//static uint8 CMD40 = 0X00;
///** LOCK_UNLOCK */
//static uint8 CMD42 = 0X00;
//#endif
//
//#ifdef SD_SPI_BUS_PROTCOL
//// Application-Specific Commands (class 8)
///** CMD55 - Indicates to the card that the next command is an application
// * specific command rather than a standard command */
//static uint8 APP_CMD = 0X37;
///** CMD56 - Used either to transfer a data block to the card or to get a
// * data block from the card for general purpose/application specific
// * commands. In case of a SDSC Card, block length is set by the
// * SET_BLOCK_LEN command. In case of SDHC and SDXC Cards, block length is
// * fixed to 512 bytes. The host sets RD/WR=1 for reading data from the card
// * and sets to 0 for writing data to the card. */
//static uint8 GEN_CMD = 0X00;
///** READ_OCR - read the OCR register of a card */
//static uint8 CMD58 = 0X3A;
///** CMD59 */
//static uint8 CRC_ON_OFF = 0X00;
//#endif
//
//// Application Specific Commands used/reserved by SD Memory Card
///** ACMD13 */
//static uint8 SD_STATUS = 0X00;
//static uint8 ACMD22 = 0X00;
///** SET_WR_BLK_ERASE_COUNT - Set the number of write blocks to be
//     pre-erased before writing */
//static uint8 ACMD23 = 0X17;
///** SD_SEND_OP_COMD - Sends host capacity support information and
//    activates the card's initialization process */
//static uint8 ACMD41 = 0X29;
//static uint8 ACMD42 = 0X00;
//static uint8 ACMD51 = 0X00;
///** status for card in the ready state */
//static uint8 R1_READY_STATE = 0X00;
///** status for card in the idle state */
//static uint8 R1_IDLE_STATE = 0X01;
///** status bit for illegal command */
//static uint8 R1_ILLEGAL_COMMAND = 0X04;
///** start data token for read or write single block*/
//static uint8 DATA_START_BLOCK = 0XFE;
///** stop token for write multiple blocks*/
//static uint8 STOP_TRAN_TOKEN = 0XFD;
///** start data token for write multiple blocks*/
//static uint8 WRITE_MULTIPLE_TOKEN = 0XFC;
///** mask for data response tokens after a write block operation */
//static uint8 DATA_RES_MASK = 0X1F;
///** write data accepted token */
//static uint8 DATA_RES_ACCEPTED = 0X05;
//
///** Set SCK to max rate of F_CPU/2. See Sd2Card::setSckRate(). */
//static uint8 SPI_FULL_SPEED = 0;
///** Set SCK rate to F_CPU/4. See Sd2Card::setSckRate(). */
//static uint8 SPI_HALF_SPEED = 1;
///** Set SCK rate to F_CPU/8. Sd2Card::setSckRate(). */
//static uint8 SPI_QUARTER_SPEED = 2;
///** Protect block zero from write if nonzero */
//#define SD_PROTECT_BLOCK_ZERO 1
///** init timeout ms */
//static uint16 SD_INIT_TIMEOUT = 2000;
///** erase timeout ms */
//static uint16 SD_ERASE_TIMEOUT = 10000;
///** read timeout ms */
//static uint16 SD_READ_TIMEOUT = 300;
///** write time out ms */
//static uint16 SD_WRITE_TIMEOUT = 600;
//// SD card errors
///** timeout error for command CMD0 */
//static uint8 SD_CARD_ERROR_CMD0 = 0X1;
///** CMD8 was not accepted - not a valid SD card*/
//static uint8 SD_CARD_ERROR_CMD8 = 0X2;
///** card returned an error response for CMD17 (read block) */
//static uint8 SD_CARD_ERROR_CMD17 = 0X3;
///** card returned an error response for CMD24 (write block) */
//static uint8 SD_CARD_ERROR_CMD24 = 0X4;
///**  WRITE_MULTIPLE_BLOCKS command failed */
//static uint8 SD_CARD_ERROR_CMD25 = 0X05;
///** card returned an error response for CMD58 (read OCR) */
//static uint8 SD_CARD_ERROR_CMD58 = 0X06;
///** SET_WR_BLK_ERASE_COUNT failed */
//static uint8 SD_CARD_ERROR_ACMD23 = 0X07;
///** card's ACMD41 initialization process timeout */
//static uint8 SD_CARD_ERROR_ACMD41 = 0X08;
///** card returned a bad CSR version field */
//static uint8 SD_CARD_ERROR_BAD_CSD = 0X09;
///** erase block group command failed */
//static uint8 SD_CARD_ERROR_ERASE = 0X0A;
///** card not capable of single block erase */
//static uint8 SD_CARD_ERROR_ERASE_SINGLE_BLOCK = 0X0B;
///** Erase sequence timed out */
//static uint8 SD_CARD_ERROR_ERASE_TIMEOUT = 0X0C;
///** card returned an error token instead of read data */
//static uint8 SD_CARD_ERROR_READ = 0X0D;
///** read CID or CSD failed */
//static uint8 SD_CARD_ERROR_READ_REG = 0X0E;
///** timeout while waiting for start of read data */
//static uint8 SD_CARD_ERROR_READ_TIMEOUT = 0X0F;
///** card did not accept STOP_TRAN_TOKEN */
//static uint8 SD_CARD_ERROR_STOP_TRAN = 0X10;
///** card returned an error token as a response to a write operation */
//static uint8 SD_CARD_ERROR_WRITE = 0X11;
///** attempt to write protected block zero */
//static uint8 SD_CARD_ERROR_WRITE_BLOCK_ZERO = 0X12;
///** card did not go ready for a multiple block write */
//static uint8 SD_CARD_ERROR_WRITE_MULTIPLE = 0X13;
///** card returned an error to a CMD13 status check after a write */
//static uint8 SD_CARD_ERROR_WRITE_PROGRAMMING = 0X14;
///** timeout occurred during write programming */
//static uint8 SD_CARD_ERROR_WRITE_TIMEOUT = 0X15;
///** incorrect rate selected */
//static uint8 SD_CARD_ERROR_SCK_RATE = 0X16;

//// card types
///** Standard capacity V1 SD card */
//static uint8 SD_CARD_TYPE_SD1 = 1;
///** Standard capacity V2 SD card */
//static uint8 SD_CARD_TYPE_SD2 = 2;
///** High Capacity SD card */
//static uint8 SD_CARD_TYPE_SDHC = 3;
