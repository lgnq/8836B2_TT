#include "reg.h"
#include "typedefs.h"
#include "tw8836.h"
#include "mcu.h"

//===== memory register ========
volatile BYTE	XDATA *DATA regTW88 = REG_START_ADDRESS;

//*****************************************************************************
//      UART 0 Interrupt                                                   
//*****************************************************************************
DATA 	BYTE	RS_buf[RS_BUF_MAX];
DATA 	BYTE	RS_in    = 0;
DATA	BYTE	RS_out   = 0;
		bit		RS_Xbusy = 0;

/**
* UART0 interrupt service routine
* 
* enable SFR_ES0
*/
void uart0_int(void) interrupt 4 using 2
{
	BYTE count;

	if (SFRB_RI)
    {
        //--- Receive interrupt ----
		SFRB_RI = 0;
		
        count = SFR_UART0FIFO & 0x1F;
		if (count & 0x10)
        {
            SFR_UART0FIFO = 0x90;		// overflowed, buffer clear
        }
		else
        {
            while (count)
            {
                RS_buf[RS_in++] = SFR_SBUF;
                if (RS_in >= RS_BUF_MAX)
                    RS_in = 0;

                count--;
            };
        }
    }

	if (SFRB_TI)
    {
        //--- Transmit interrupt ----
		SFRB_TI = 0;
		RS_Xbusy = 0;
	}
}

void putchar(BYTE c)
{
	while (RS_Xbusy)
        ;

	SFRB_ES = 0;
    
	SFR_SBUF = c;
	RS_Xbusy = 1;
	
    SFRB_ES = 1;
}

void mcu_init(void)
{
	TWBASE = 0x00;					// Base address of TW88xx
	SFR_CAMC = 1;					// Chip Access Mode Control. E2[0]=1b:16bit mode

    //---------- Initialize Timer Divider ---------
    WriteTW88(REG4E2, 0x69);		// Timer0 Divider : system tic 0. 
    WriteTW88(REG4E3, 0x78);		// 27M/27000 = 1msec

    WriteTW88(REG4E4, 0x01);		// Timer1 Divider : for Touch
    WriteTW88(REG4E5, 0x0e);		// 27M/270 = 10usec	

    WriteTW88(REG4E6, 0);			// Timer2 Divider : remo timer
    WriteTW88(REG4E7, 0x1b);		// 27M/27 = 1usec

    WriteTW88(REG4E8, 0);			// Timer3 Divider : baudrate for UART0
    WriteTW88(REG4E9, 0x0c);		// (22.1184M *2 /32) / 12 = 115200bps on SM0=1 & SMOD0=1	

    WriteTW88(REG4EA, 0);			// Timer4 Divider : baudrate for UART1
    WriteTW88(REG4EB, 0x0c);		// (22.1184M *2 /32) / 12 = 115200bps on SM1=1 & SMOD1=1	

    /*If you want the UART1 swap, add below code. */
    /* WriteTW88(REG04B, ReadTW88(REG04B) | 0x40);	*/
    
	//---------- Initialize interrupt -------------
	SFR_CKCON = 0x00;		// Clock control register			
							// 0000 0000
							// |||| |||+-- MD0 \.
							// |||| ||+--- MD1 	> MD[2:0] Stretch RD/WR timing
							// |||| |+---- MD2 /
							// |||| +----- T0M:  Timer0 Pre-Divider 0=div by 12,  1=div by 4
							// |||+------- T1M:  Timer1 Pre-Divider 0=div by 12,  1=div by 4
							// ||+-------- ---
							// |+--------- WD0 \ Watchdong Timeout Period
							// +---------- WD1 / 00=2^17,  01=2^20,  10=2^23,  11=2^26

    SFR_TMOD = 0x66;		// 0110 0110
							// |||| ||||   << Timer 0 >>
							// |||| |||+-- M0 \  00= 8bit timer,counter /32  01= 16bit timer,counter
							// |||| ||+--- M1 /  10= 8bit auto reload        11= 8bit timer,counter
							// |||| |+---- CT:   0=Timer Mode,    1=Counter Mode
							// |||| +----- GATE: 0=GATE not used, 1=GATE used
							// ||||        << Timer 1 >>
							// |||+------- M0 \  00= 8bit timer,counter /32  01= 16bit timer,counter
							// ||+-------- M1 /  10= 8bit auto reload        11= 8bit timer,counter
							// |+--------- CT:   0=Timer Mode,    1=Counter Mode
							// +---------- GATE: 0=GATE not used, 1=GATE used

    SFR_TCON = 0x55;		// 0101 0101
							// |||| |||+-- IT0:  INT0 Trigger 0=level, 1=edge
							// |||| ||+--- IE0:  INT0 Interrupt Flag
							// |||| |+---- IT1:  INT1 Trigger 0=level, 1=edge
							// |||| +----- IE1:  INT1 Interrupt Flag
							// |||+------- TR0:  Timer0 Run
							// ||+-------- TF0:  Timer0 Flag
							// |+--------- TR1:  Timer1 Run
							// +---------- TF0:  Timer1 Flag

 	SFR_TH0 = 0xff;			// 0xFFFF = 0x10000-1 = 1 msec
	SFR_TL0 = 0xff;			//

							// for TOUCH SAR sensing timer
	SFR_TH1 = 206;			// 
							// TH1 = 156. 1ms
							// TH1 = 206. 0.5ms = 50*10usec

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
	SFR_TL2 = 0x2e;	        // it means 210 usec interval. 

	SFR_PCON = 0xc0;		// 1100 0000
							// |||| |||+-- PMM:  Power Management Mode 0=Disable,  1=Enable
							// |||| ||+--- STOP: Stop Mode             0=Disable,  1=Enable
							// |||| |+---- SWB:  Switch Back from STOP 0=Disable,  1=Enable
							// |||| +----- ---
							// |||+------- PWE:	 (Program write Enable)
							// ||+-------- ---
							// |+--------- SMOD1:UART1 Double baudrate bit
							// +---------- SMOD0:UART0 Double baudrate bit

	SFR_SCON = 0x50;		// 0101 0000
							// |||| |||+-- RI:   Receive Interrupt Flag
							// |||| ||+--- TI:   Transmit Interrupt Flag
							// |||| |+---- RB08: 9th RX data
							// |||| +----- TB08: 9th TX data
							// |||+------- REN:	 Enable Serial Reception
							// ||+-------- SMO2: Enable Multiprocessor communication
							// |+--------- SM01 \   Baudrate Mode
							// +---------- SM00 / 00=f/12,  01=8bit var,  10=9bit,f/32,f/64,  11=9bit var

	SFR_SCON1 = 0x50;		// 0101 0000
							// |||| |||+-- RI:   Receive Interrupt Flag
							// |||| ||+--- TI:   Transmit Interrupt Flag
							// |||| |+---- RB08: 9th RX data
							// |||| +----- TB08: 9th TX data
							// |||+------- REN:	 Enable Serial Reception
							// ||+-------- SMO2: Enable Multiprocessor communication
							// |+--------- SM11 \   Baudrate Mode
							// +---------- SM10 / 00=f/12,  01=8bit var,  10=9bit,f/32,f/64,  11=9bit var

	//---------- Enable Individual Interrupt ----------
	SFR_IP	 = 0x10;		// 0001 0000 interrupt priority
                            // |||| |||+-- PX0 EX0
                            // |||| ||+--- RT0 ET0
                            // |||| |+---- PX1 EX1
                            // |||| +----- PT1 ET1
                            // |||+------- PS  ES (use register bank 2)
                            // ||+-------- PT2 ET2
                            // |+--------- PS1 ES1
                            // +---------- unused 					

	SFR_IE	 = 0x10;		// 1001 0010 interrupt enable
                            // |||| |||+ EX0    : Chip Interrupt. I will enable it later.
                            // |||| ||+- ET0    : Timer0    System Tic
                            // |||| |+-- EX1    : DE End
                            // |||| +--- ET1    : timer1 - touch
                            // |||+----- ES     : UART0
                            // ||+------ ET2    : timer2 - remocon
                            // |+------- ES1    : UART1
                            // +-------- EA     : Global Interrupt. I will enable it below.

#ifdef SUPPORT_UART1
	SFRB_ES1  = 1;			// UART1  	: External MCU
#endif

	//---------- Serial FIFO ----------
	SFR_UART0FIFO = 0x80;	//          : UART0 FIFO

#ifdef SUPPORT_UART1
	SFR_UART1FIFO = 0x80;	//          : UART1 FIFO
#endif

	//---------- Enable global Interrupt ----------
	SFRB_EA   = 1;			// Global Interrupt


	//---------- Extended Interrupt -------------------
	SFR_EIE	 = 0x04;		// 0000 0100 extended interrupt enable
                            // |||| |||+ EINT2  : SPI-DMA done
                            // |||| ||+- EINT3  : Touch Ready
                            // |||| |+-- EINT4  : SW 7FF 
                            // |||| +--- EINT5  : reserved
                            // |||+----- EINT6  : Pen
                            // ||+------ EWDI   : Watchdog
                            // |+------- reserved
                            // +-------- reserved
    SFR_EIP  = 0x00;
                            /* If Watchdog, EIE.5 & EIP.5 will be 1. */
	//---------- Extended2 Interrupt 7~14 Config. ---------------
                            // xxxx xxxx
                            // |||| |||+- EINT7
                            // |||| ||+-- EINT8
                            // |||| |+--- EINT9  :Remocon. SW will enable 9 later.
                            // |||| +---- EINT10 :MODEL_TW8835_EXTI2C. VideoISR.
                            // |||+------ EINT11
                            // ||+------- EINT12
                            // |+-------- EINT13
                            // +--------- EINT14
							// 
    //SFR_E2IF  = 0x00;		// (0xFA)	0000 0000 : Interrupt Flag         
	SFR_E2IE  = 0x00;		// (0xFB)	0000 0000 : Interrupt Enable.
	SFR_E2IP  = 0x00;		// (0xFC)	0000 0000 : Interrupt Priority
	SFR_E2IM  = 0xff;		// (0xFD)	0000 0000 : Interrupt Mode(0=Level, 1=Edge)
	SFR_E2IT  = 0x00;		// (0xFE)	0000 0000 : Level/Edge Polarity  (0=Low/Falling, 1=High/Rising)
							/*                      0: Low Level,  Falling Edge	*/
							/*                      1: High Level, Rising Edge */

    putchar('O');
    putchar('K');
}
