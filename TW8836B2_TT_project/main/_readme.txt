/*--------------------------------------------------------------------*-

   Time-Triggered Reference Design (TTRD02b)

   Targets C8051F85x processor.

   Designed to run on UPMU-F850 Starter Kit.

   In this simple example, the “Heartbeat” task generates brief 
   overrun after ~20 seconds: this is detected by the (TTC) scheduler.
   
   After a further 40 seconds, the Heartbeat task hangs completely: 
   this causes the system to enter a Fail-Silent Mode.

   [Released 2015-03]


   ------------------

   This code is copyright (c) 2014-2015 SafeTTy Systems Ltd.
 
   This code forms part of a Time-Triggered Reference Design (TTRD)
   that is based on examples in the following book: 

   Pont, M.J. (2014) 
   "The Engineering of Reliable Embedded Systems: LPC1769 edition", 
   Published by SafeTTy Systems Ltd. ISBN: 978-0-9930355-0-0.

   Both the TTRDs and the above book ("the ERES book") describe 
   patented* technology and are subject to copyright and other 
   restrictions.

   This code may be used without charge: [i] by universities and 
   colleges in courses for which a degree up to and including �MSc� 
   level (or equivalent) is awarded; [ii] for non-commercial projects 
   carried out by individuals and hobbyists.

   Where this code has been provided as part of a training course 
   delivered by SafeTTy Systems Ltd, it may be used for evaluation 
   purposes for a period of up to 12 months from the end date of 
   this course, subject to the condition that no products (of any 
   kind) based either on this code, or on the patented technology 
   described in the ERES book, are released during this period.

   All other use of this code - including the release of any products 
   based either on this code, or on the patented technology described 
   in the ERES book - is subject to the conditions that: [i] every 
   person involved in the development of a product using this code is 
   in possession of a legitimate copy of the ERES book; [ii] the 
   organisation developing the product is in possession of an 
   active ReliabiliTTy Technology Licence; [iii] any and all products 
   are released under the terms of the ReliabiliTTy Technology Licence:
   http://www.safetty.net/products/reliabilitty

   Please contact SafeTTy Systems Ltd if you require clarification
   of these licensing arrangements:
   http://www.safetty.net/contact

   MoniTTor is a registered trademark of SafeTTy Systems Ltd.
   PredicTTor is a registered trademark of SafeTTy Systems Ltd.
   ReliabiliTTy is a registered trademark of SafeTTy Systems Ltd.
   WarranTTor is a registered trademark of SafeTTy Systems Ltd.

   This file may be freely distributed, provided that it is not 
   altered in any way.  

   * Patents applied for.

-*--------------------------------------------------------------------*/
