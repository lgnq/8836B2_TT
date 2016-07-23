
   Time-Triggered Reference Design (TTRD02b)

   Targets TW8836B2 processor.

   Designed to run on TW8836 Demo Board.

   In this simple example, the “Heartbeat” task generates brief 
   overrun after ~20 seconds: this is detected by the (TTC) scheduler.
   
   After a further 40 seconds, the Heartbeat task hangs completely: 
   this causes the system to enter a Fail-Silent Mode.

   [Released 2016-07]
