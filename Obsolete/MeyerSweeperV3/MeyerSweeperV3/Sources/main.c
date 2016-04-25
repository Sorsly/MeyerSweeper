/*
***********************************************************************
 ECE 362 - Experiment 8 - Fall 2014
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

 The objective of this experiment is to implement a reaction time assessment
 tool that measures, with millisecond accuracy, response to a visual
 stimulus -- here, both a YELLOW LED and the message "Go Team!" displayed on 
 the LCD screen.  The TIM module will be used to generate periodic 
 interrupts every 1.000 ms, to serve as the time base for the reaction measurement.  
 The RTI module will provide a periodic interrupt at a 2.048 ms rate to serve as 
 a time base for sampling the pushbuttons and incrementing the variable "random" 
 (used to provide a random delay for starting a reaction time test). The SPI
 will be used to shift out data to an 8-bit SIPO shift register.  The shift
 register will perform the serial to parallel data conversion for the LCD.

 The following design kit resources will be used:

 - left LED (PT1): indicates test stopped (ready to start reaction time test)
 - right LED (PT0): indicates a reaction time test is in progress
 - left pushbutton (PAD7): starts reaction time test
 - right pushbutton (PAD6): stops reaction time test (turns off right LED
                    and turns left LED back on, and displays test results)
 - LCD: displays status and result messages
 - Shift Register: performs SPI -> parallel conversion for LCD interface

 When the right pushbutton is pressed, the reaction time is displayed
 (refreshed in place) on the first line of the LCD as "RT = NNN ms"
 followed by an appropriate message on the second line 
 e.g., 'Ready to start!' upon reset, 'Way to go HAH!!' if a really 
 fast reaction time is recorded, etc.). The GREEN LED should be turned on
 for a reaction time less than 250 milliseconds and the RED LED should be
 turned on for a reaction time greater than 1 second.

***********************************************************************
*/

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>

/* All funtions after main should be initialized here */
char inchar(void);
void outchar(char x);
void tdisp();
void shiftout(char x);
void lcdwait(int x);
void send_byte(char x);
void send_i(char x);
void chgline(char x);
void print_c(char x);
void pmsglcd(char[]);

void BCDadd(int x);

/* initialization functions */
void commandList(unsigned char *addr);
void commonInit(unsigned char *cmdList);

/* Variable declarations */  	   			 		  			 		       
int leftpb	= 0;  // left pushbutton flag
int rghtpb	= 0;  // right pushbutton flag
int prevpb	= 0;  // previous pushbutton state
int runstp	= 0;  // run/stop flag

int timer = 0;    //time counter var

/* slave select 
  rather than use the built in SS pin on port M (for spi), just make sure to
  enable the SS pin local to each device 
*/
int jhd162a_en;     //2x16 lcd    pull high to dis
int st7735r_en;     //TFT 1.8" lcd
int st7735r_rst;    //ST7735r reset command

/*initialization vars*/
unsigned char colstart;
unsigned char rowstart;

/* ASCII character definitions */
#define CR 0x0D	// ASCII return character   

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

/* LED BIT MASKS */
#define GREEN 0x20    //PTT5
#define RED 0x40      //PTT6
#define YELLOW 0x80   //PTT7



//initialization macros
#define DELAY 0x80

// some flags for initR() :(
#define INITR_GREENTAB 0x0
#define INITR_REDTAB   0x1
#define INITR_BLACKTAB   0x2

#define INITR_18GREENTAB    INITR_GREENTAB
#define INITR_18REDTAB      INITR_REDTAB
#define INITR_18BLACKTAB    INITR_BLACKTAB
#define INITR_144GREENTAB   0x1

#define ST7735_TFTWIDTH  128
// for 1.44" display
#define ST7735_TFTHEIGHT_144 128
// for 1.8" display
#define ST7735_TFTHEIGHT_18  160

#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36
                              
#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// Color definitions
#define	ST7735_BLACK   0x0000
#define	ST7735_BLUE    0x001F
#define	ST7735_RED     0xF800
#define	ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0
#define ST7735_WHITE   0xFFFF
/*
***********************************************************************
Command Tables
***********************************************************************
*/

unsigned char Rcmd1[59] = {                 // Init for 7735R, part 1 (red or green tab)
    15,                       // 15 commands in list:
    ST7735_SWRESET,   DELAY,  //  1: Software reset, 0 args, w/delay
      150,                    //     150 ms delay
    ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, 0 args, w/delay
      255,                    //     500 ms delay
    ST7735_FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
      0x01, 0x2C, 0x2D,       //     Dot inversion mode
      0x01, 0x2C, 0x2D,       //     Line inversion mode
    ST7735_INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
      0x07,                   //     No inversion
    ST7735_PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                   //     -4.6V
      0x84,                   //     AUTO mode
    ST7735_PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
      0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
    ST7735_PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
      0x0A,                   //     Opamp current small
      0x00,                   //     Boost frequency
    ST7735_PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
      0x8A,                   //     BCLK/2, Opamp current small & Medium low
      0x2A,  
    ST7735_PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
      0x0E,
    ST7735_INVOFF , 0      ,  // 13: Don't invert display, no args, no delay
    ST7735_MADCTL , 1      ,  // 14: Memory access control (directions), 1 arg:
      0xC8,                   //     row addr/col addr, bottom to top refresh
    ST7735_COLMOD , 1      ,  // 15: set color mode, 1 arg, no delay:
      0x05 };                 //     16-bit color

unsigned char Rcmd2green[13] = {            // Init for 7735R, part 2 (green tab only)
    2,                        //  2 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x02,             //     XSTART = 0
      0x00, 0x7F+0x02,        //     XEND = 127
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x01,             //     XSTART = 0
      0x00, 0x9F+0x01 };      //     XEND = 159

unsigned char Rcmd3[43] = {                 // Init for 7735R, part 3 (red or green tab)
    4,                        //  4 commands in list:
    ST7735_GMCTRP1, 16      , //  1: Magical unicorn dust, 16 args, no delay:
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16      , //  2: Sparkles and rainbows, 16 args, no delay:
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST7735_NORON  ,    DELAY, //  3: Normal display on, no args, w/delay
      10,                     //     10 ms delay
    ST7735_DISPON ,    DELAY, //  4: Main screen turn on, no args w/delay
      100 };                  //     100 ms delay

	 	   		
/*
***********************************************************************
 Initializations
***********************************************************************
*/

void  initializations(void) {

/* Set the PLL speed (bus clock = 24 MHz) */
  CLKSEL = CLKSEL & 0x80; // disengage PLL from system
  PLLCTL = PLLCTL | 0x40; // turn on PLL
  SYNR = 0x02;            // set PLL multiplier
  REFDV = 0;              // set PLL divider
  while (!(CRGFLG & 0x08)){  }
  CLKSEL = CLKSEL | 0x80; // engage PLL

/* Disable watchdog timer (COPCTL register) */
  COPCTL = 0x40;   //COP off, RTI and COP stopped in BDM-mode

/* Initialize asynchronous serial port (SCI) for 9600 baud, no interrupts */
  SCIBDH =  0x00; //set baud rate to 9600
  SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  
  SCICR1 =  0x00; //$9C = 156
  SCICR2 =  0x0C; //initialize SCI for program-driven operation
  DDRB   =  0x10; //set PB4 for output mode
  PORTB  =  0x10; //assert DTR pin on COM port
         
         
/* Add additional port pin initializations here */
  DDRT = 0xFF;

/* Initialize SPI for baud rate of 6 Mbs */
  SPIBR  = 0b00000001;  //SPRx = 1, SPPRx = 0, (0+1)*(2^(1+1)) = 4, 24MHz/4 = 6 Mbps  
  SPICR1 = 0b01010000;  //pg156 <- pg159
  SPICR2 = 0b00000000;  //pg160 <- pg159
  
/* Initialize digital I/O port pins */
  DDRAD = 0; 		//program port AD for input mode
  ATDDIEN = 0xC0; //program PAD7 and PAD6 pins as digital inputs

/* Initialize the JHD162A LCD
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
  lcdwait(2); 

/* Initialize the ST7735R LCD
    -
*/

  commonInit(Rcmd1);
  commandList(Rcmd2green);    //INITR_GREENTAB
  colstart = 2;
  rowstart = 1;
  commandList(Rcmd3);
  

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
  TIE =   0b00000000; //interrupt disabled
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
    PTT_PTT3 = !jhd162a_en; //jhd162a_en = 1 means PTT_PTT3 = 0 (en)
    PTT_PTT5 = !st7735r_en; //st7735r_en = 1 means PTT_PTT5 = 0 (en)

//If the left pushbutton
    if(leftpb && !runstp){
      leftpb = 0;
      runstp = 1;
      
      send_i(LCDCLR);     //display message
      pmsglcd("Ready, Set...");  
    }      

//If the right pushbutton
    if(rghtpb && runstp){
      rghtpb = 0;
      runstp = 0;   
    }
    
  } /* loop forever */
  
}  /* do not leave main */


/*
***********************************************************************
commandList()
***********************************************************************
*/

void commandList(unsigned char *addr) {

  unsigned char numCommands;
  unsigned char numArgs;     //supposed to be 8bit
  unsigned char ms;          //supposed to be 16bit
  int j = 0;

  numCommands = addr[j++];   // Number of commands to follow
  while(numCommands--) {                 // For each command...
    writecommand(addr[j++]); //   Read, issue command
    numArgs  = addr[j++];    //   Number of args to follow
    ms       = numArgs & DELAY;          //   If hibit set, delay follows args
    numArgs &= ~DELAY;                   //   Mask out delay bit
    while(numArgs--) {                   //   For each argument...
      writedata(addr[j++]);  //     Read, issue argument
    }

    if(ms) {
      ms = addr[j++]; // Read post-command delay time (ms)
      if(ms == 255) ms = 500;     // If 255, delay for 500 ms
      delay(ms);
    }
  }
}

/*
***********************************************************************
commonInit()
***********************************************************************
*/
// Initialization code common to both 'B' and 'R' type displays
void commonInit(unsigned char *cmdList) 
{
  colstart  = rowstart = 0; // May be overridden in init func
//Pin defines
  //PT5: TFT_CS (chip enable)
  //PT2: D/C (data or command selector) (shares with JHD162A register select)
  //PM5: SCK (SPI clock (no shift register to deal with))
  //PT6: RESET (in case ST7735R needs to be reset)
  
//SPI settings
//unnecessary
//reset settings

  // toggle RST low to reset; CS low so it'll listen to us

  if (st7735r_rst) {
    PTT_PTT6 = 1;
    lcdwait(500);
    PTT_PTT6 = 0;
    lcdwait(500);
    PTT_PTT6 = 1;
    lcdwait(500);
  }

  commandList(cmdList);       //7735R (green tab)
}


/*
***********************************************************************
 RTI interrupt service routine: RTI_ISR

  Initialized for 2.048 ms interrupt rate

  Samples state of pushbuttons (PAD7 = left, PAD6 = right)

  If change in state from "high" to "low" detected, set pushbutton flag
     leftpb (for PAD7 H -> L), rghtpb (for PAD6 H -> L)
     Recall that pushbuttons are momentary contact closures to ground

  Also, increments 2-byte variable "random" each time interrupt occurs
  NOTE: Will need to truncate "random" to 12-bits to get a reasonable delay 
***********************************************************************
*/

interrupt 7 void RTI_ISR(void)
{
  	// clear RTI interrupt flag
  	CRGFLG = CRGFLG | 0x80; 
    if (PORTAD0_PTAD7 < prevpb){    //0 is button pushed
	    prevpb = 0;
	    leftpb = 1;
	  } else if (PORTAD0_PTAD7 == 1){
	    prevpb = 1;
	  }
	  
	  if (PORTAD0_PTAD6 < prevpb){
	    prevpb = 0;
	    rghtpb = 1;
	  } else if (PORTAD0_PTAD6 == 1){
	    prevpb = 1;
	  }
	  
}

/*
*********************************************************************** 
  TIM Channel 7 interrupt service routine
  Initialized for 1.00 ms interrupt rate
  Increment (3-digit) BCD variable "react" by one
***********************************************************************
*/

interrupt 15 void TIM_ISR(void)
{
	// clear TIM CH 7 interrupt flag
 	TFLG1 = TFLG1 | 0x80; 

  timer++;
}


/*
*********************************************************************** 
  tdisp: displays score and time left
***********************************************************************
*/
 
void tdisp()
{ 
  //actual display function    
}

/*
***********************************************************************
  shiftout: Transmits the character x to external shift 
            register using the SPI.  It should shift MSB first.  
             
            MISO = PM[4]
            SCK  = PM[5]
***********************************************************************
*/
 
void shiftout(char x)   //pg161

{ 
  int i;
  
  if(jhd162a_en && !st7735r_en){  
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
  }else if(!jhd162a_en && st7735r_en){
  
  }
}

/*
***********************************************************************
  lcdwait: Delay for approx 1 ms
***********************************************************************
*/

void lcdwait(int time)
{
  int j = 0;
  while(j<(2750*time)){
    asm{
      nop;
      nop;
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
  if(jhd162a_en && !st7735r_en){    
  // shift out character
    shiftout(x);
  // pulse LCD clock line low->high->low
    PTT_PTT4 = 0;
    PTT_PTT4 = 1;
    PTT_PTT4 = 0;
  // wait 2 ms for LCD to process data
    lcdwait(2);
  }else if(!jhd162a_en && st7735r_en){
  
  }  
}

/*
***********************************************************************
  send_i: Sends instruction byte x to LCD  
***********************************************************************
*/

void send_i(char x)
{
  if(jhd162a_en && !st7735r_en){  
  // set the register select line low (instruction data)
    PTT_PTT2 = 0;
  // send byte
    send_byte(x);
  }else if(!jhd162a_en && st7735r_en){  
  
  }
}

/*
***********************************************************************
  chgline: Move LCD cursor to position x
  NOTE: Cursor positions are encoded in the LINE1/LINE2 variables
***********************************************************************
*/

void chgline(char x)
{
  if(jhd162a_en && !st7735r_en){
    PTT_PTT2 = 0;   //instruction register
    send_i(CURMOV);
    send_i(x);
  }else if(!jhd162a_en && st7735r_en){  
  
  }
}

/*
***********************************************************************
  print_c: Print (single) character x on LCD            
***********************************************************************
*/
 
void print_c(char x)
{
  if(jhd162a_en && !st7735r_en){  
    PTT_PTT2 = 1;   //data register
    send_byte(x);
  }else if(!jhd162a_en && st7735r_en){
  
  }  
}                                                                                                   

/*
***********************************************************************
  pmsglcd: print character string str[] on LCD
***********************************************************************
*/

void pmsglcd(char str[])
{
  if(jhd162a_en && !st7735r_en){  
    int i = 0;
    while(str[i] != '\0'){
      print_c(str[i]);
      if(i == strlen(str)){  
        return;
      }
      i++;
    }
  }else if(!jhd162a_en && st7735r_en){
  
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
