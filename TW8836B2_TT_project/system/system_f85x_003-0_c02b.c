/*--------------------------------------------------------------------*-

  system_f85x_003-0_c02b.c (Released 2015-03)

  Controls system configuration after processor reset.

  [Two modes supported - "Normal" and "Fail Silent".]

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

// Task headers
#include "../tasks/heartbeat_f85x_003-0_c02b.h"
#include "../tasks/watchdog_f85x_003-0_c02a.h"

// ------ Public variable definitions ------------------------------

// In many designs, System_mode_G will be used in other modules.
// - we therefore make this variable public.
eSystem_mode System_mode_G;

// ------ Private function declarations ----------------------------

void SYSTEM_Identify_Required_Mode(void);
void SYSTEM_Configure_Required_Mode(void);

/*--------------------------------------------------------------------*-

  SYSTEM_Init()

  Wrapper for system startup functions.

-*--------------------------------------------------------------------*/
void SYSTEM_Init(void)
{
	SYSTEM_Identify_Required_Mode();
	SYSTEM_Configure_Required_Mode();
}

/*--------------------------------------------------------------------*-

  SYSTEM_Identify_Required_Mode()

  Try to work out the cause of the system reset.
  Set the system mode accordingly.

-*--------------------------------------------------------------------*/
void SYSTEM_Identify_Required_Mode(void)
{
	uint8_t WDT_flag = 0;  // Set to 1 if WDT caused latest reset

    //disable watchdog
	SFRB_EWDI = 0;		// Disable WDT Interrupt

	SFRB_EA   = 0; 		//Disable Interrupt
	SFR_TA    = 0xaa;
	SFR_TA    = 0x55;
	SFR_WDCON = 0x00;	// - - - - WDIF WTRF EWT RWT
	SFRB_EA   = 1; 		//Enable Interrupt

	// Check cause of reset
	// First check the PORSF bit:
	// if PORSF is set, all other RSTSRC flags are indeterminate.
#if 0
	if ((RSTSRC & 0x02) == 0x00)        
	{
		if (RSTSRC == 0x08)
		{
			// Last reset was caused by WDT
			WDT_flag = 1;
		}
	}
#endif
    
	if (SFRB_WTRF)
	{
		// Set system mode (Fail Silent)
		System_mode_G = FAIL_SILENT;
	}
	else
	{
		// Here we treat all other forms of reset in the same way
		// Set system mode (Normal)
		System_mode_G = NORMAL;
	}
}

/*--------------------------------------------------------------------*-

  SYSTEM_Configure_Required_Mode()

  Configure the system in the required mode.  

-*--------------------------------------------------------------------*/
void SYSTEM_Configure_Required_Mode(void)
{
	switch (System_mode_G)
	{
		default:          // Default to "FAIL_SILENT"

		case FAIL_SILENT:
			// Reset caused by WDT
			// Trigger "fail silent" behaviour
			SYSTEM_Perform_Safe_Shutdown();

			break;

		case NORMAL:
            mcu_init();
        
			// Set up WDT 
			// Set to overflow after ~12ms
			WATCHDOG_Init();

			// Set up scheduler for 1 ms ticks
			SCH_Init();

			// Prepare for heartbeat task
			HEARTBEAT_Init();

			// Add tasks to schedule.
			// Parameters are:
			// 1. Task name
			// 2. Initial delay / offset (in Ticks)
			// 3. Task period (in Ticks): Must be > 0
			// 4. Task WCET (in microseconds)
			// 5. Task BCET (in microseconds)

			// Add watchdog task first
			SCH_Add_Task(WATCHDOG_Update, 0, 1500, 10, 0);

			// Add heartbeat task
			SCH_Add_Task(HEARTBEAT_Update, 0, 1000, 20, 0);

			// Feed the watchdog
			WATCHDOG_Update();

			break;
	}
}

/*--------------------------------------------------------------------*-

  SYSTEM_Perform_Safe_Shutdown()

  Attempt to move the system into a safe state.
  Feeds Watchdog Timer.

-*--------------------------------------------------------------------*/
void SYSTEM_Perform_Safe_Shutdown(void)
{
	// Used for simple fault reporting.
	uint16_t Delay, j;

	// Feed watchdog
	WATCHDOG_Update();

	// Here we simply "fail silent" with rudimentary fault reporting.
	// Other behaviour may make more sense in your design

	// *************************************
	// NOTE: This function should NOT return
	// *************************************

	HEARTBEAT_Init();

	while (1)
	{
		// Flicker Heartbeat LED to indicate fault
		for (Delay = 0; Delay < 3000; Delay++)
		{
			j *= 3;
			WATCHDOG_Update();
		}

		HEARTBEAT_Update2();
	}
}

/*--------------------------------------------------------------------*-
  ------ END OF FILE -------------------------------------------------
-*--------------------------------------------------------------------*/
