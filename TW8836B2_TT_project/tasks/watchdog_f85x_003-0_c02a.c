/*--------------------------------------------------------------------*-

  watchdog_f85x_003-0_c02a.c (Released 2015-03)

  --------------------------------------------------------------------
   
  Simple 'Watchdog' module for F85x (8051).

-*--------------------------------------------------------------------*/
/*--------------------------------------------------------------------*-

  This code is copyright (c) 2014-2015 SafeTTy Systems Ltd.

  This code is adapted from a Time-Triggered Reference Design (TTRD)
  that is documented in the following book: 

  Pont, M.J. (2014) 
  "The Engineering of Reliable Embedded Systems: LPC1769 Edition", 
  Published by SafeTTy Systems Ltd. ISBN: 978-0-9930355-0-0.

  Both the TTRDs and the above book ("the ERES book") describe 
  patented* technology and are subject to copyright and other 
  restrictions.

  This code may be used without charge: [i] by universities and 
  colleges in courses for which a degree up to and including MSc 
  level (or equivalent) is awarded; [ii] for non-commercial projects 
  carried out by individuals and hobbyists.

  Where this code has been provided to you as part of a training 
  course delivered by SafeTTy Systems Ltd, you may use it for 
  evaluation purposes for a period of up to 12 months from the 
  end date of this course, subject to the condition that no 
  products (of any kind) based either on this code, or on the 
  patented technology described in the ERES book, are released.

  Any and all other use of this code and / or the patented 
  technology described in the ERES book requires purchase of a 
  ReliabiliTTy Technology Licence:
  http://www.safetty.net/products/reliabilitty

  Please contact SafeTTy Systems Ltd if you require clarification
  of these licensing arrangements:
  http://www.safetty.net/contact

  MoniTTor is a registered trademark of SafeTTy Systems Ltd.
  PredicTTor is a registered trademark of SafeTTy Systems Ltd.
  ReliabiliTTy is a registered trademark of SafeTTy Systems Ltd.
  WarranTTor is a registered trademark of SafeTTy Systems Ltd.

  * Patents applied for.

-*--------------------------------------------------------------------*/

// Project header
#include "../main/main.h"

// Task header
#include "watchdog_f85x_003-0_c02a.h"

#pragma SAVE
#pragma OPTIMIZE(8, SIZE)

void watchdog_config(BYTE mode)
{
	SFRB_EWDI = 0;		// Disable WDT Interrupt

	/* SFR_CKCON (0x8E) : base 32kHz clock.
	mode WD[1:0]	CLK(on TW8836B)	CLK(on FPGA)	Delay
	3	00			2^11			2^17			70mSec
	2	01			2^14			2^20			540msec
	1	10			2^17			2^23			4 sec
	0	11			2^20			2^26			32 sec
	*/
	SFR_CKCON &= 0x3F;

	switch (mode)
    {
	case 0: 	SFR_CKCON |= 0xc0;	break;	
	case 1: 	SFR_CKCON |= 0x80;	break;	
	case 2: 	SFR_CKCON |= 0x40;	break;	
	case 3:		SFR_CKCON |= 0x00;	break;	
	default: 	SFR_CKCON |= 0xc0;	break;
	}
	
    SFRB_EA   = 0; 		// Disable Interrupt
	SFR_TA    = 0xaa;
	SFR_TA    = 0x55;
	SFR_WDCON = 0x03;	// - - - - WDIF WTRF EWT RWT.
	SFRB_EA   = 1; 		// Enable Interrupt

	SFR_EIP  |= 0x20;	// BK160208:We have 512 counter, is it enough ?
	SFRB_EWDI = 1;		// Enable WDT Interrupt (disable for test)
}

/*--------------------------------------------------------------------*-

  WATCHDOG_Init()

  Set up watchdog timer on F85x (8051).

  Based on Sci Labs examples code.

-*--------------------------------------------------------------------*/
void WATCHDOG_Init(void)
{
    watchdog_config(0);
}

/*--------------------------------------------------------------------*-

  WATCHDOG_Update()

  Feed the watchdog timer.

  See Watchdog_Init() for further information.

-*--------------------------------------------------------------------*/
void WATCHDOG_Update(void)
{
	SFRB_EWDI = 0;		// Disable WDT Interrupt

	SFRB_EA   = 0; 		// Disable Interrupt
	SFR_TA    = 0xaa;
	SFR_TA    = 0x55;
	SFR_WDCON = 0x03;	// - - - - WDIF WTRF EWT RWT.  Reset Watchdog
	SFRB_EA   = 1; 		// Enable Interrupt
	
    if (SFR_CACHE_EN == 0)
        SFR_CACHE_EN = 1;
    
	SFRB_EWDI = 1;		// Enable WDT Interrupt (disable for test)
}

void watchdog_int(void) interrupt 12 using 3
{
    /*
        disable cache & do not write RWT.
        system will be reset after 512 clock cycle base on 32KHz.
    */
    SFR_CACHE_EN = 0;
}
#pragma RESTORE

/*--------------------------------------------------------------------*-
  ------ END OF FILE -------------------------------------------------
-*--------------------------------------------------------------------*/
