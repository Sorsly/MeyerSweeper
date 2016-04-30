/*
***********************************************************************
 ECE 362 - Experiment 9 - Spring 2015
***********************************************************************
	 	   			 		  			 		  		
 Completed by: Alan Han
               7716-H
               1


 Academic Honesty Statement:  In entering my name above, I hereby certify
 that I am the individual who created this HC(S)12 source file and that I 
 have not copied the work of any other student (past or present) while 
 completing it. I understand that if I fail to honor this agreement, I will 
 receive a grade of ZERO and be subject to possible disciplinary action.
***********************************************************************

 The objective of this experiment is to implement an analog signal sampling
 and reconstruction application that allows the user to efficiently cycle
 through different input and output sampling frequencies.

 The following design kit resources will be used:

 - left pushbutton (PAD7): cycles through input sampling frequency choices
                           (5000 Hz, 10,000 Hz, and 20,000 Hz)

 - right pushbutton (PAD6): cycles through output sampling frequency choices
                           (23,529 Hz, 47,059 Hz, and 94,118 Hz)

 - LCD: displays current values of input and output sampling frequencies
 - Shift Register: performs SPI -> parallel conversion for LCD interface

***********************************************************************
*/
 
#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>

/* All funtions after main should be initialized here */
char inchar(void);
void outchar(char);
void fdisp(void);
void shiftout(char);
void lcdwait(void);
void send_byte(char);
void send_i(char);
void chgline(char x);
void print_c(char);
void pmsglcd(char[]);

void isfset();
void osfset();
int joy();

int joymove;

/*  Variable declarations */ 	   			 		  			 		       
int leftpb	= 0;  // left pushbutton flag
int rghtpb	= 0;  // right pushbutton flag
int prevpb	= 0;  // previous pushbutton state

int isfmode = 1;  //input sampling frequency mode
int osfmode = 1;  //output sampling frequency mode

int samp0;        //analog 0 (fx gen input)
int samp1;        //analog 1 (potentiometer input)

/* bonus */
int thresh = 255;
int saw = 0;
int intcnt = 0;
/* shiftout */
int i;

/* LCD COMMUNICATION BIT MASKS */
#define RS 0x04		// RS pin mask (PTT[2])
#define RW 0x08		// R/W pin mask (PTT[3])
#define LCDCLK 0x10	// LCD EN/CLK pin mask (PTT[4])

/* LCD INSTRUCTION CHARACTERS */
#define LCDON 0x0F	// LCD initialization command
#define LCDCLR 0x01	// LCD clear display command
#define TWOLINE 0x38	// LCD 2-line enable command
#define CURMOV 0xFE	// LCD cursor move instruction
#define LINE1 0x80	// LCD line 1 cursor position
#define LINE2 0xC0	// LCD line 2 cursor position

/*	 	   		
***********************************************************************
 Initializations
***********************************************************************
*/

void  initializations(void) {

/* Set the PLL speed (bus clock = 24 MHz) */
  CLKSEL = CLKSEL & 0x80; //; disengage PLL from system
  PLLCTL = PLLCTL | 0x40; //; turn on PLL
  SYNR = 0x02;            //; set PLL multiplier
  REFDV = 0;              //; set PLL divider
  while (!(CRGFLG & 0x08)){  }
  CLKSEL = CLKSEL | 0x80; //; engage PLL


/* Disable watchdog timer (COPCTL register) */
  COPCTL = 0x40   ; //COP off; RTI and COP stopped in BDM-mode

/* Initialize asynchronous serial port (SCI) for 9600 baud, no interrupts */
  SCIBDH =  0x00; //set baud rate to 9600
  SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  
  SCICR1 =  0x00; //$9C = 156
  SCICR2 =  0x0C; //initialize SCI for program-driven operation
  DDRB   =  0x10; //set PB4 for output mode
  PORTB  =  0x10; //assert DTR pin on COM port

         
/* Add additional port pin initializations here */
  DDRT = 0xFF;

/* Initialize the SPI to 6 Mbs */
  SPIBR  = 0b00000001;  //SPRx = 1, SPPRx = 0, (0+1)*(2^(1+1)) = 4, 24MHz/4 = 6 Mbps  
  SPICR1 = 0b01010000;  //pg156 <- pg159
  SPICR2 = 0b00000000;  //pg160 <- pg159
	 	   			 		  			 		  		
/* Initialize digital I/O port pins */
  DDRAD = 0; 		//program port AD for input mode
  ATDDIEN = 0xC0; //program PAD7 and PAD6 pins as digital inputs

/* Initialize the LCD
     - pull LCDCLK high (idle)
     - pull R/W' low (write state)
     - turn on LCD (LCDON instruction)
     - enable two-line mode (TWOLINE instruction)
     - clear LCD (LCDCLR instruction)
     - wait for 2ms so that the LCD can wake up     
*/ 
  PTT_PTT4 = 1;
  PTT_PTT3 = 0;
  send_i(LCDON);
  send_i(TWOLINE);
  send_i(LCDCLR);
  lcdwait(); 
  
/* Initialize RTI for 2.048 ms interrupt rate */	
  CRGINT = CRGINT | 0x80;  //enable RTI (pg32)
  RTICTL = 0x1F;  //pg33
  
/* Initialize TIM Ch 7 (TC7) for periodic interrupts every 1.000 ms
     - enable timer subsystem
     - set channel 7 for output compare
     - set appropriate pre-scale factor and enable counter reset after OC7
     - set up channel 7 to generate 1 ms interrupt rate
     - initially disable TIM Ch 7 interrupts      
*/
  TSCR1 = 0b10000000; //enable subsys
  TIOS =  0b10000000; //chan 7 for output compare
  TSCR2 = 0b00001100; //set pre-scale factor and enable counter reset
  TC7 =   1500;       //generate 1500clicks = 1 ms interrupt rate          from notes
  TIE =   0b10000000; //interrupt disabled
  
/* ATD Initializations */

  ATDCTL2 = 0b10000000;   //enable atd
  ATDCTL3 = 0b00010000;   //sequence length is 2 (for multi channel sampling)
  ATDCTL4 = 0b10000101;   //set to 8 bit control mode, prescale clock to 2Mhz
  ATDCTL5 = 0b00010100;   //sample across multiple channels (need to be across channels 4 & 5)
  //ATDCTL2_AFFC = 1;       //set to fast flag clear mode
  
	
/* PWM Initializations */
  
  MODRR     = 0b00000001; // PT0 is PWM Ch 0 output
  PWME	    =	0b00000001;	// PWM enable
  PWMPOL	  =	0b00000001;	// PWM polarity
  PWMCLK	  =	0b00000000;	// PWM clock source select (ch0 SA)
  PWMPRCLK  =	0b00000001;	// PWM pre-scale clock select 
  PWMCAE	  =	0b00000000;	// PWM center align enable (left)
  PWMCTL	  =	0b00000000;	// PWM control (concatenate enable)
  PWMPER0	  =	0b11111111;	// PWM period registers   pg 293  FF
  PWMDTY0	  =	0b00000000;	// PWM duty registers     pg 293  50%

}

/*	 		  			 		  		
***********************************************************************
 Main
***********************************************************************
*/

void main(void) {
  	DisableInterrupts;
	initializations(); 		  			 		  		
	EnableInterrupts;



  for(;;) {
  

  /* write your code here */
    if (leftpb){
      isfset();
      fdisp();
      
      leftpb = 0;
      PTT_PTT1 = 0;
   	  PTT_PTT0 = 1;
    }
    if (rghtpb){
      osfset();
      fdisp();
      
      rghtpb = 0;
      PTT_PTT1 = 1;
   	  PTT_PTT0 = 0;
    }
    
  } /* loop forever */
   
}  /* do not leave main */



/*
********************************************************
joy - interfaces with joystick
********************************************************
*/
int joy()
{
	int xjoy = 177;
	int yjoy = 177;
	
	//ATDCTL5 = 0x10;
	
	xjoy=ATDDR0H;
	yjoy=ATDDR1H;
  
	while(!ATDSTAT0_SCF){   //wait until conversion is done
    asm{
        nop;
      }
	}


    
  if(ATDDR4H > 153 && (ATDDR5H > 102 && ATDDR5H < 153))
	{
		//move Left
	  joymove = 4;
	}
	else if(ATDDR4H < 102 && (ATDDR5H > 102 && ATDDR5H < 153))
	{
		//move right
		joymove = 6;
	}
	else if(ATDDR5H > 153 && (ATDDR4H > 102 && ATDDR4H < 153))
	{
		//move up
		joymove = 8;
	}
	else if(ATDDR5H < 102 && (ATDDR4H > 102 && ATDDR4H < 153))
	{
		//move down
		joymove = 2;
	} else if((ATDDR5H > 102 && ATDDR5H < 153) && (ATDDR4H > 102 && ATDDR4H < 153)){
	  //move center
	  joymove = 10;
	}
	
	//3/5 * 256 = 153, 2/5 * 256 = 102
	/*
	if(xjoy > 153 && (yjoy > 102 && yjoy < 153))
	{
		//move Left
	  joymove = 4;
	}
	else if(xjoy < 102 && (yjoy > 102 && yjoy < 153))
	{
		//move right
		joymove = 6;
	}
	else if(yjoy > 153 && (xjoy > 102 && xjoy < 153))
	{
		//move up
		joymove = 2;
	}
	else if(yjoy < 102 && (xjoy > 102 && xjoy < 153))
	{
		//move down
		joymove = 8;
	} else if((yjoy > 102 && yjoy < 153) && (xjoy > 102 && xjoy < 153)){
	  //move center
	  joymove = 0;
	}
	*/
	
	return joymove;
}



/*
***********************************************************************
  isfset()
  sets isf to correct frequency
***********************************************************************
*/

void isfset() 
{ 
  if(isfmode == 1){
    TC7 = 300;
  }else if(isfmode == 2){
    TC7 = 150;
  }else if(isfmode == 3){
    TC7 = 75;
  }
}

/*
***********************************************************************
  osfset()
  sets osf to correct frequency
***********************************************************************
*/

void osfset() 
{  
  if(osfmode == 1){       //23529
    PWMPRCLK = 0b00000010;       //=256
  }else if(osfmode == 2){ //47059
    PWMPRCLK = 0b00000001;       //=128
  }else if(osfmode == 3){ //94118
    PWMPRCLK = 0b00000000;       //=64
  }  
}

/*
***********************************************************************
 RTI interrupt service routine: RTI_ISR

  Initialized for 2.048 ms interrupt rate

  Samples state of pushbuttons (PAD7 = left, PAD6 = right)

  If change in state from "high" to "low" detected, set pushbutton flag
     leftpb (for PAD7 H -> L), rghtpb (for PAD6 H -> L)
     Recall that pushbuttons are momentary contact closures to ground
***********************************************************************
*/

interrupt 7 void RTI_ISR(void)
{
  	// clear RTI interrupt flag
  	CRGFLG = CRGFLG | 0x80; 
    //left
    if (PORTAD0_PTAD7 < prevpb){      //0 is button pushed
	    prevpb = 0;
	    leftpb = 1;
	    
	    isfmode++;                       //isf inc and cycle
	    if(isfmode >= 4){
	      isfmode = 1;
	    }
    }
    
	  //right
	  if (PORTAD0_PTAD6 < prevpb){      //right pushed
	    prevpb = 0;
	    rghtpb = 1;
	    
	    osfmode++;                       //osf inc and cycle
	    if(osfmode >= 4){
	      osfmode = 1;
	    }
	  }
	   
	  if (PORTAD0_PTAD7 == 1 && PORTAD0_PTAD6 == 1 ){   //not pushed
	    prevpb = 1;
	  }
	  
}

/*
***********************************************************************
  TIM interrupt service routine
  used to initiate ATD samples (on Ch 0 and Ch 1)	 		  			 		  		
***********************************************************************
*/

interrupt 15 void TIM_ISR(void)
{

        // clear TIM CH 7 interrupt flag 
 	TFLG1 = TFLG1 | 0x80; 
  ATDCTL5 = 0b00010100;   //start conversion on ATD ch 0, mult set
  
  while(!ATDSTAT0_SCF){
    asm{
      nop;
    }
  }
  //intcnt++;
  //if(intcnt >= thresh){
    //intcnt = 0;    
    saw = (saw + 1) % 255;
    PWMDTY0 = saw*ATDDR1H/256;
  //}
  //PWMDTY0 = ATDDR0H*ATDDR1H/256;  //256 instead of 255 because of rounding issues
}

/*
***********************************************************************
  fdisp: Display "ISF = NNNNN Hz" on the first line of the LCD and display 
         and "OSF = MMMMM Hz" on the second line of the LCD (where NNNNN and
         MMMMM are the input and output sampling frequencies, respectively         
***********************************************************************
*/

void fdisp()
{
  send_i(LCDCLR);
  
  pmsglcd("ISF = ");
  if(isfmode == 1){
    pmsglcd("5000 Hz");
  }else if(isfmode == 2){
    pmsglcd("10000 Hz");
  }else if(isfmode == 3){
    pmsglcd("20000 Hz");
  }
  
  chgline(LINE2);
  
  pmsglcd("OSF = ");
  if(osfmode == 1){
    pmsglcd("23529 Hz");
  }else if(osfmode == 2){
    pmsglcd("47059 Hz");
  }else if(osfmode == 3){
    pmsglcd("94118 Hz");
  }  
}

/*
***********************************************************************
  shiftout: Transmits the character x to external shift 
            register using the SPI.  It should shift MSB first.  
             
            MISO = PM[4]
            SCK  = PM[5]
***********************************************************************
*/
 
void shiftout(char x)

{
// read the SPTEF bit, continue if bit is 1
  if(SPISR&0x20){  
  // write data to SPI data register
    SPIDR = x;
  // wait for 30 cycles for SPI data to shift out 
    for(i=0; i<10; i++){
      asm{
        nop;
      }
    }
  }
}

/*
***********************************************************************
  lcdwait: Delay for approx 2 ms
***********************************************************************
*/

void lcdwait()
{
  int j = 0;
  while(j<0x1fff){
    asm{
      nop;

    }
    j++;
  } 
}

/*
*********************************************************************** 
  send_byte: writes character x to the LCD
***********************************************************************
*/

void send_byte(char x)
{
// shift out character
  shiftout(x);
// pulse LCD clock line low->high->low
  PTT_PTT4 = 0;
  PTT_PTT4 = 1;
  PTT_PTT4 = 0;
// wait 2 ms for LCD to process data
  lcdwait();  
}

/*
***********************************************************************
  send_i: Sends instruction byte x to LCD  
***********************************************************************
*/

void send_i(char x)
{
// set the register select line low (instruction data)
  PTT_PTT2 = 0;
// send byte
  send_byte(x);  
}

/*
***********************************************************************
  chgline: Move LCD cursor to position x
  NOTE: Cursor positions are encoded in the LINE1/LINE2 variables
***********************************************************************
*/

void chgline(char x)
{
  PTT_PTT2 = 0;   //instruction register
  send_i(CURMOV);
  send_i(x);
}

/*
***********************************************************************
  print_c: Print (single) character x on LCD            
***********************************************************************
*/
 
void print_c(char x)
{
  PTT_PTT2 = 1;   //data register
  send_byte(x);
}

/*
***********************************************************************
  pmsglcd: print character string str[] on LCD
***********************************************************************
*/

void pmsglcd(char str[])
{
  int k = 0;
  while(str[k] != '\0'){
    print_c(str[k]);
    if(i == strlen(str)){  
      return;
    }
    k++;
  } 
}


/*
***********************************************************************
 Character I/O Library Routines for 9S12C32 (for debugging only)
***********************************************************************
 Name:         inchar
 Description:  inputs ASCII character from SCI serial port and returns it
 Example:      char ch1 = inchar();
***********************************************************************
*/

char inchar(void) {
  /* receives character from the terminal channel */
        while (!(SCISR1 & 0x20)); /* wait for input */
    return SCIDRL;
}

/*
***********************************************************************
 Name:         outchar
 Description:  outputs ASCII character x to SCI serial port
 Example:      outchar('x');
***********************************************************************
*/

void outchar(char x) {
  /* sends a character to the terminal channel */
    while (!(SCISR1 & 0x80));  /* wait for output buffer empty */
    SCIDRL = x;
}