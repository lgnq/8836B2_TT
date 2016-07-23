/*--------------------------------------------------------------------*-

  heartbeat_f85x_003-0_c02b.c (Released 2015-03)

  --------------------------------------------------------------------
   
  Simple 'Heartbeat' module for F85x (8051).

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
#include "heartbeat_f85x_003-0_c02b.h"

// ------ Public variable declarations -----------------------------

// See scheduler module for definition
extern uint8_t Fault_code_G;

/*--------------------------------------------------------------------*-

  HEARTBEAT_Init()

  Prepare for HEARTBEAT_Update() function - see below.

-*--------------------------------------------------------------------*/
void HEARTBEAT_Init(void)
{
//    putchar('X');
}

/*--------------------------------------------------------------------*-

  HEARTBEAT_Update()

  Must schedule this task for release every second.

  Flashes at 0.5 Hz if no fault code.
  Otherwise, displays fault code.

  Incorporates injected task overruns (for demo purposes):
  1. after 20 seconds (brief overrun);
  2. after 60 seconds (permanent fault).

-*--------------------------------------------------------------------*/
void HEARTBEAT_Update(void)
{
	static uint16_t Heartbeat_state = 0;
	static uint16_t Fault_state = 0;
	static uint16_t Task_overrun_counter = 0;

	// Inject brief task overrun after 20 seconds
	// => This should be detected by scheduler (not WDT)
	if (Task_overrun_counter++ == 20)
	{
		// Wait for task to overrun (see scheduler ISR) ...
		while (Fault_code_G == 0);
	}
 
	// Inject permanent task overrun after 60 seconds 
	// => This should trigger WDT
	if (Task_overrun_counter == 60)
	{
		while(1);
	}

	if (Fault_code_G == 0)
	{
		// No faults recorded
		// - just flash at 0.5 Hz

		// Change the LED from OFF to ON (or vice versa)
		if (Heartbeat_state == 1)
		{
			Heartbeat_state = 0;
//			HEARTBEAT_LED_PIN = 1;
            putchar('A');
		}
		else
		{
			Heartbeat_state = 1;
//			HEARTBEAT_LED_PIN = 0;
            putchar('B');
		}
	}
	else
	{
		// If we are here, there is a (non-zero) fault code ...
		Fault_state++;

		if (Fault_state < Fault_code_G*2)
		{
			Heartbeat_state = 0;
//			HEARTBEAT_LED_PIN = 1;
            putchar('1');
		}
		else
		{
			if (Fault_state < Fault_code_G*4)
			{
				// Change the LED from OFF to ON (or vice versa)
				if (Heartbeat_state == 1)
				{
					Heartbeat_state = 0;
//					HEARTBEAT_LED_PIN = 1;
                    putchar('2');
				}
				else
				{
					Heartbeat_state = 1;
//					HEARTBEAT_LED_PIN = 0;
                    putchar('3');
				}
			}
			else
			{
				Fault_state = 0;
			}
		}
	}
}

/*--------------------------------------------------------------------*-

  HEARTBEAT_Update2()

  Simplified "Heartbeat" function.

-*--------------------------------------------------------------------*/
void HEARTBEAT_Update2(void)
{
	static uint16_t Heartbeat_state = 0;

	// Change the LED from OFF to ON (or vice versa)
	if (Heartbeat_state == 1)
	{
		Heartbeat_state = 0;
        putchar('0');
//		HEARTBEAT_LED_PIN = 1;
	}
	else
	{
		Heartbeat_state = 1;
//		HEARTBEAT_LED_PIN = 0;
        putchar('1');
	}
}

/*--------------------------------------------------------------------*-
  ------ END OF FILE -------------------------------------------------
-*--------------------------------------------------------------------*/
