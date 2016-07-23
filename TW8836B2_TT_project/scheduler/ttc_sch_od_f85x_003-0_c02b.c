/*--------------------------------------------------------------------*-

  ttc_sch_od_f85x_003-0_c02b.c (Released 2015-03)

  --------------------------------------------------------------------

  Time-Triggered Co-operative (TTC) task scheduler for F85x (8051).

  See "The Engineering of Reliable Embedded Systems" (Chapter 2)
  for further information about this scheduler - details below.

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

// ------ Public variable definitions ------------------------------

// May be used (for example) to report faults using Heartbeat LED.
// See Heartbeat task (if used) for basic fault-reporting mechanism
uint8_t Fault_code_G;

// ------ Private variable definitions -----------------------------

// The array of tasks
// Check array size in scheduler header file
sTask SCH_tasks_G[SCH_MAX_TASKS];

// The current tick count
static volatile uint8_t Tick_count_G = 0;

// Flag indicating whether any task is running
static volatile uint8_t Task_running_G = 0;

// ------ Private function prototypes ------------------------------

static void SCH_Go_To_Sleep(void);

// Prototype for Timer 2 ISR
INTERRUPT_PROTO (SCH_UPDATE, INTERRUPT_TIMER2);

/*--------------------------------------------------------------------*-

  SCH_Init()

  Scheduler initialisation function.  Prepares scheduler
  data structures and sets up timer interrupts every millisecond

  You must call this function before using the scheduler.  

-*--------------------------------------------------------------------*/
void SCH_Init(void)
{
	uint8_t i;

	// Reset the global fault variable
	Fault_code_G = 0;  

	for (i = 0; i < SCH_MAX_TASKS; i++) 
	{
		SCH_tasks_G[i].pTask = 0;
	}

#if 0    
	// Now set up Timer 2
	// [Based on SciLab example code]
	// Configure Timer2 to 16-bit auto-reload and generate an interrupt at
	// interval specified by <counts> using SYSCLK/48 as its time base.
	TMR2CN = 0x00;                // Stop Timer2; Clear TF2;
	                             // use SYSCLK/12 as timebase
	CKCON &= ~0x60;               // Timer2 clocked based on T2XCLK;

	// Init reload values
	TMR2RL = -(SYSCLK / 12 / 1000);                   
	TMR2 = 0xffff;                // Set to reload immediately
#endif

#if 0    
    WriteTW88(REG4E6, 0x0);			// Timer2 Divider : remo timer
    WriteTW88(REG4E7, 0x1B);		// 27M/27 = 1usec
    
	SFR_T2CON = 0x12;		// 0001 0010 
							// |||| |||+-- T2I0 \ Timer2 Input Selection 
							// ||||	||+--- T2I1 / 00=No,  01=Timer,  10=Counter, 11=Gate
							// ||||	|+---- T2CM:  Compare mode
							// ||||	+----- T2R0 \ Timer2 Reload Mode 
							// |||+------- T2R1	/ 00=No,  01=No,     10=Auto,    11=pin T2EX
							// ||+-------- ---
							// |+--------- I3FR: Timer2 Compare0 Interrupt Edge...
							// +---------- T2PS: Timer2 Prescaler
    		
	SFR_TH2 = 0xff;	        // 0xFF2E = 0x10000-0xD2 = 0x10000-210. 
	SFR_TL2 = 0x2E;	        // it means 210 usec interval.     
#endif

	// Not yet started timer
}

/*--------------------------------------------------------------------*-

  SCH_Start()

  Starts the scheduler, by enabling SysTick interrupt.

  NOTES: 
  * All tasks must be added before starting scheduler.
  * Any other interrupts MUST be synchronised to this tick.  

-*--------------------------------------------------------------------*/
void SCH_Start(void) 
{
    SFRB_ET0 = 1;   // Enable Timer2 interrupts
	SFRB_EA  = 1;   // Enable all interrupts
}

/*--------------------------------------------------------------------*-

  SCH_Update()

  This is the scheduler ISR.  It is called at a rate
  determined by the timer settings in the SCH_Init() function.

-*--------------------------------------------------------------------*/

void timer2_int(void) interrupt 5 using 1	// suggest: use register block 3			
{
#if 0    
	// Increment tick count (only)
	Tick_count_G++;

	// As this is a TTC scheduler, we don't usually expect
	// to have a task running when the timer ISR is called
	if (Task_running_G == 1)
	{
		// Simple fault reporting. via heartbeat / fault LED.
		// (This value is *not* reset.)
		Fault_code_G = FAULT_SCH_SYSTEM_OVERLOAD;
	}
#endif
    
	SFR_T2IF &= 0xfc;			// Clear Interrupt Flag
}

void timer0_int(void) interrupt 1 using 1
{
	// Increment tick count (only)
	Tick_count_G++;

	// As this is a TTC scheduler, we don't usually expect
	// to have a task running when the timer ISR is called
	if (Task_running_G == 1)
	{
		// Simple fault reporting. via heartbeat / fault LED.
		// (This value is *not* reset.)
		Fault_code_G = FAULT_SCH_SYSTEM_OVERLOAD;
	}
}

/*--------------------------------------------------------------------*-

  SCH_Dispatch_Tasks()

  This is the 'dispatcher' function.  When a task (function)
  is due to run, SCH_Dispatch_Tasks() will run it.
  This function must be called (repeatedly) from the main loop.

-*--------------------------------------------------------------------*/
void SCH_Dispatch_Tasks(void) 
{
	uint16_t Index;
	uint16_t Update_required = 0;

	SFRB_ET0 = 0;  // Protect shared resource (Tick_count_G)
	if (Tick_count_G > 0)
	{
		Tick_count_G--;
		Update_required = 1;
	}
	SFRB_ET0 = 1;

	while (Update_required)
	{
		// Go through the task array
		for (Index = 0; Index < SCH_MAX_TASKS; Index++)
		{
			// Check if there is a task at this location
			if (SCH_tasks_G[Index].pTask)
			{
				if (--SCH_tasks_G[Index].Delay == 0)
				{
					SFRB_ET0 = 0;
					Task_running_G = 1;  // Set flag
					SFRB_ET0 = 1;
               
					(*SCH_tasks_G[Index].pTask)();  // Run the task
               
					SFRB_ET0 = 0;
					Task_running_G = 0;  // Clear flag
					SFRB_ET0 = 1;

					// All tasks are periodic in this design
					// - schedule task to run again
					SCH_tasks_G[Index].Delay = SCH_tasks_G[Index].Period;
				}
			}         
		} 

		SFRB_ET0 = 0;
		if (Tick_count_G > 0)
		{
			Tick_count_G--;
			Update_required = 1;
		}
		else
		{
			Update_required = 0;
		}
		SFRB_ET0 = 1;
	}

	// The scheduler may enter idle mode at this point (if used) 
	SCH_Go_To_Sleep();        
}

/*--------------------------------------------------------------------*-

  SCH_Add_Task()

  Causes a task (function) to be executed at regular intervals.

  pTask  - The name of the task (function) to be scheduled.
           NOTE: All scheduled functions must be 'void, void' -
           that is, they must take no parameters, and have 
           a void return type (in this design).
                   
  DELAY  - The interval (ticks) before the task is first executed.

  PERIOD - Task period (in ticks).  Must be > 0.

  WCET   - Worst-Case Execution Time (microseconds)
           [Used only for documentation in this design.]

  BCET   - Best-Case Execution Time (microseconds)
           [Used only for documentation in this design.]

  RETURN VALUE:  
  Returns the position in the task array at which the task has been 
  added.  If the return value is SCH_MAX_TASKS then the task could 
  not be added to the array (there was insufficient space, or the
  requested task period was 0).
  If the return value is < SCH_MAX_TASKS, then the task was added
  successfully.  

  Note: this return value may be used (in later designs) to
  support the use of backup tasks.
 
-*--------------------------------------------------------------------*/
uint16_t SCH_Add_Task(void (* pTask)(), 
                      const uint16_t DELAY,
                      const uint16_t PERIOD,
                      const uint16_t WCET,
                      const uint16_t BCET
                      )
{
	uint16_t Return_value = 0;
	uint16_t Index = 0;
   
	// First find a gap in the array (if there is one)
	while ((SCH_tasks_G[Index].pTask != 0) && (Index < SCH_MAX_TASKS))
	{
		Index++;
	} 
   
	// Have we reached the end of the list?   
	if (Index == SCH_MAX_TASKS)
	{
		// Task list is full
		//
		// Set the global fault variable
		Fault_code_G = FAULT_SCH_TOO_MANY_TASKS;

		// Also return a fault code
		Return_value = SCH_MAX_TASKS;
	}
      
	// Check for "one shot" tasks
	// - not permitted in this design
	if (PERIOD == 0)
	{
		// Set the global fault variable
		Fault_code_G = FAULT_SCH_ONE_SHOT_TASK;

		// Also return a fault code
		Return_value = SCH_MAX_TASKS;
	}

	if (Return_value != SCH_MAX_TASKS)
	{
		// If we're here, there is a space in the task array
		// and the task to be added is periodic
		SCH_tasks_G[Index].pTask  = pTask;

		SCH_tasks_G[Index].Delay  = DELAY + 1;
		SCH_tasks_G[Index].Period = PERIOD;
		SCH_tasks_G[Index].WCET   = WCET;
		SCH_tasks_G[Index].BCET   = BCET;

		Return_value = Index;
	}

	return Return_value;
}

/*--------------------------------------------------------------------*-

  SCH_Go_To_Sleep()

  This scheduler enters 'sleep mode' between clock ticks
  to [i] reduce tick jitter; and [ii] save power.

  The next clock tick will return the processor
  to the normal operating state.

-*--------------------------------------------------------------------*/
void SCH_Go_To_Sleep()
{
///	PCON |= 0x01; // Enter idle mode (generic 8051 version)
///	PCON = PCON;  // Reqd to avoid interrupt blocking
}

/*--------------------------------------------------------------------*-
  ------ END OF FILE -------------------------------------------------
-*--------------------------------------------------------------------*/
