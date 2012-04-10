/******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2012 LeafLabs, LLC
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *****************************************************************************/

/** @file Commands.h
 *  @breif List of card commands for all SecureDigital and MultiMedia cards
*/

#ifndef _SD_COMMANDS_H_
#define _SD_COMMANDS_H_

typedef enum SDIOCommand {
// Basic Commands (class 0)
	/** CMD0 - Resets all cards to idle state */
	GO_IDLE_STATE        = 0,
	/** CMD1 - reserved MMC */
	SEND_OP_COND         = 1,
	/** CMD2 - Asks any card to send the CID numbers on the CMD line */
	ALL_SEND_CID         = 2,
	/** CMD3 - Ask the card to publish a new relative address */
	SEND_RELATIVE_ADDR   = 3,
	/** CMD4 - Programs the DSR of all cards */
	SET_DSR              = 4,
	/** CMD5 - reserved for SDIO cards */
	CMD5                 = 5,
	/** CMD6 - reserved */
	CMD6                 = 6,
	/** CMD7 - toggles a card between the stand-by and transfer
	 * states or between the programming and disconnect states */
	SELECT_DESELECT_CARD = 7,
	/** CMD8 - SD Memory Card interface condition */
	SEND_IF_COND         = 8,
	/** CMD9 - Addressed card sends its card-specific data (CSD)
	 * on the CMD line */
	SEND_CSD             = 9,
	/** CMD10 - Addressed card sends its card identification (CID)
	 * on the CMD line */
	SEND_CID             = 10,
	/** CMD11 - Switch to 1.8V bus signaling level */
	//READ_DAT_UNTIL_STOP = 0X0B;
	VOLTAGE_SWITCH       = 11,
	/** CMD12 - Forces the card to stop transmission */
	STOP_TRANSMISSION    = 12,
	/** CMD13 - Addressed card sends its status register */
	SEND_STATUS          = 13,
	/** CMD14 - Reserved */
	CMD14                = 14,
	/** CMD15 - Sends an addressed card into the Inactive State */
	GO_INACTIVE_STATE    = 15,
} SDIOCommand;

#endif
