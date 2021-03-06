#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>

//#include<stdio.h>
//#include<stdlib.h>
//#include<time.h>

#define NROW 8
#define NCOL 10
//#define NBOMB 8

////////////////////////////////////////function declarations
void Initial_board(int(*)[]);
void Initial_mask(int(*)[]);
void Create_board(int,int(*)[]);//set mine and set number of mines around
int GameControl(int,int,int,int(*)[],int(*)[]);//run this function when click one button
int FlagControl(int,int,int,int(*)[]);//flag function
int joy();//joystick function
void difficulty(void);
//-1 means mine; 9 means not clicked; 10 means flag

void splashDisp(void);
void diffDisp(void);

void pos(int, int);
void butts(void);

/* All functions after main should be initialized here */
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

/* Variable declarations */  	   			 		  			 		       
int leftpb	= 0;  // left pushbutton flag
int rghtpb	= 0;  // right pushbutton flag
int prevpb	= 0;  // previous pushbutton state

int buttApb = 0;  // button A flag
int buttBpb = 0;  // button B flag

int gamerun	= 0;  // gamerun flag
int diffrun = 0;  // difficulty menu flag
int setflag = 1;  // dummy flag for setting things, like lcdclr

int timer = 0;    //time counter var

/* slave select 
  rather than use the built in SS pin on port M (for spi), just make sure to
  enable the SS pin local to each device 
*/
int display1_en;     //lower-half 4x20 pull high to dis
int display2_en;     //upper-half 4x20 pull high to dis

/* Game var declarations*/
//  int nrow = 10;        //size
//  int ncol = 10;        //size
int irow = 0;             //place clicked
int icol = 9;             //place clicked
int frow = 0;             //place flag
int fcol = 1;             //place flag
int nbomb = 12;       //number of bomb
int nclick = 0;           //number of empty tiles with no mines nearby
int nflag = 0;
int fail = 0;
int win = 0;
int joymove = 0;
int mine[NROW][NCOL];     //map with mine and other things
int mask[NROW][NCOL];     //cover map
int i;
int j;
int a;
int b;

int dumb;

int buttAatd;
int buttBatd;


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
  lcdwait(2); 

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


//  Add RTI/interrupt initializations here
  
  CRGINT = 0b10000000;    //enables RTI 

  ATDCTL2 = 0b10000000;   //enable atd
  ATDCTL3 = 0b00010000;   //
  ATDCTL4 = 0b10000110;   //set to 10 bit control mode, prescale clock to 2Mhz
  ATDCTL5 = 0b00010000;   //sample across multiple channels
  ATDCTL2_AFFC = 1;       //set to fast flag clear mode
}
	 		  			 		  		
/*
***********************************************************************
 Main - Setup
***********************************************************************
*/
void main(void) {
  /* put your own code here */
  DisableInterrupts;
	initializations(); 		  			 		  		
	EnableInterrupts;
  
  //printf("%ld\n",time(0));  //prints time elapsed since 1970 
  
  Initial_board(mine);
  Initial_mask(mask);

/*
***********************************************************************
 Main - Loop
***********************************************************************
*/

  for(;;) {
    PTT_PTT3 = display1_en; //display1_en = 1 means PTT_PTT3 = 0 (en)
    PTT_PTT5 = display2_en; //display2_en = 1 means PTT_PTT5 = 0 (en)

    //butts();
    
    
    if(diffrun == 0 && gamerun == 0){         //splash screen
      if(setflag == 1){
        send_i(LCDCLR);
        splashDisp();
        setflag = 0;
      }
      buttBatd++;
      if(buttBpb){                            //buttBpb (FLAG), as opposed to buttBatd
        
        diffrun = 1;
        setflag = 1;
      }    
    }
    
    if(diffrun == 1 && gamerun == 0){         //difficulty screen
      if(setflag == 1){
        send_i(LCDCLR);
        diffDisp();
        setflag = 0;
      }
      difficulty();
      if(buttBpb){
        gamerun = 1;
        diffrun = 0;
        setflag = 1;
      }
    }

    if(diffrun == 0 && gamerun == 1){
      if(setflag == 1){
        send_i(LCDCLR);
        Create_board(nbomb,mine);
        joymove = 99;
        setflag = 0;
      }
      //printf("Mask: \n");
      if(joymove != 0){ 
        for(i = 0; i < NROW; i++){
          for(j = 0; j < NCOL; j++){
            //printf("%2d ",mask[i][j]);
            pos(i,j);
            if(mask[i][j] == 9){
              pmsglcd("?");
            }else if(mask[i][j] == 0){
              pmsglcd(" ");
            }else if(mask[i][j] >= 1 && mask[i][j] <= 8){
              dumb = 30 + mask[i][j];
              print_c(dumb);
            }else if(mask[i][j] == 10){
              pmsglcd("F");
            }
          }
        //printf("\n");
        }
      }
      joymove = joy();
      if(joymove == 2){
        if(irow != 0){
          irow--;
        }
      }
      if(joymove == 4){
        if(icol != 0){
          icol--;
        }
      }
      if(joymove == 6){
        if(icol != (NCOL - 1)){
          icol++;
        }
      }
      if(joymove == 8){
        if(icol != (NROW - 1)){
          irow++;
        }
      }
      
      nclick = GameControl(irow,icol,nclick,mine,mask);    
    }
    
    
    
    
    
    
    if(nclick == -1){
      fail = 1;
      
      for(a = 0; a < NROW; a++){
        for(b = 0; b < NCOL; b++){
          mask[a][b] = mine[a][b];
        }
      }  
    }
    
    
    
    //printf("Number Click: %d\n",nclick);
    
    nflag = FlagControl(frow,fcol,nflag,mask);
    
    if((nflag + nclick) == nbomb){
      win = 1;
    }
    
    //printf("Number Flag: %d\n",nflag);
    //printf("Flag: \n");
    
    for(i = 0; i < NROW; i++){
      for(j = 0; j < NCOL; j++){
        //printf("%2d ",mask[i][j]);
      }
    //printf("\n");
    }

//printf("Mine: \n");
    
    for(i = 0; i < NROW; i++){
      for(j = 0; j < NCOL; j++){
        //printf("%2d ",mine[i][j]);
      }
      //printf("\n");
    }
  
  
/*
//If the left pushbutton
    if(leftpb && !gamerun){
      leftpb = 0;
      gamerun = 1;
      
      send_i(LCDCLR);     //display message
      pmsglcd("Ready, Set...");  
    }      

//If the right pushbutton
    if(rghtpb && gamerun){
      rghtpb = 0;
      gamerun = 0;   
    }
*/    
  
  } /* loop forever */
  /* please make sure that you never leave main */
}

/////////////////////////////////////////////////////functions
/*
                              _       _ _      __                      
                             (_)     (_) |    / _|                     
  __ _  __ _ _ __ ___   ___   _ _ __  _| |_  | |_ _   _ _ __   ___ ___ 
 / _` |/ _` | '_ ` _ \ / _ \ | | '_ \| | __| |  _| | | | '_ \ / __/ __|
| (_| | (_| | | | | | |  __/ | | | | | | |_  | | | |_| | | | | (__\__ \
 \__, |\__,_|_| |_| |_|\___| |_|_| |_|_|\__| |_|  \__,_|_| |_|\___|___/
  __/ |                                                                
 |___/                                                                 
 */
 
/*
********************************************************
Initial_board - initializes board
********************************************************
*/
void Initial_board(int board[][NCOL])
{
  int i;
  int j;

  for(i = 0; i < NROW; i++){
    for(j = 0; j < NCOL; j++){
      board[i][j] = 0;
    }
  }
}
/*
********************************************************
Initial_mask - initializes mask
********************************************************
*/
void Initial_mask(int board[][NCOL])
{
  int i;
  int j;

  for(i = 0; i < NROW; i++){
    for(j = 0; j < NCOL; j++){
      board[i][j] = 9;
    }
  }
}

/*
********************************************************
difficulty - choose diffculty
********************************************************
*/

void difficulty(void) 
{

  if(joymove == 2){
    if(nbomb != 8){
    nbomb -= 4;
    pos(8,2);
    }
  }
  if(joymove == 8){
    if(nbomb != 16){
    nbomb += 4;
    pos(8,4);
    }
  }
  if(joymove == 8){
    if(nbomb == 16){
    nbomb = 79;
    pos(9,5);
    }
  } 
}

/*
********************************************************
splashDisp - displays splash screen waiting for button press
********************************************************
*/
void splashDisp(void) 
{
  pos(2,2);
  pmsglcd("MEYER");
  pos(11,5);
  pmsglcd("SWEEPER");
  
  pos(3,6);
  pmsglcd("Press joystick to start");
}

/*
********************************************************
diffDisp - displays splash screen for difficulty selection
********************************************************
*/
void diffDisp(void)
{
  pos(8,2);
  pmsglcd("Easy");

  pos(8,4);
  pmsglcd("Hard");
  pos(9,5);
  pmsglcd("GG");
  
  pos(7,3);           //final cursor position to work with difficulty()
  pmsglcd("Normal");
}


/*
********************************************************
Create_board - populates game board with mines
********************************************************
*/
void Create_board(int nbomb, int board[][NCOL])
{
  int i;
  int j;
  //int nbomb = NBOMB;
  //int n = time(0);
  int n = timer;
  while(nbomb != 0){
    srand(n);
    i = rand() % NROW;
    j = rand() % NCOL;
    //printf("i = %d, j = %d\n",i,j);
    if(!(board[i][j] == -1)){
      board[i][j] = -1;
      if((i - 1) != -1 && (j - 1) != -1 && (!(board[i-1][j-1] == -1))){
	      board[i - 1][j - 1]++;
	    }
      if((i - 1) != -1 && (!(board[i-1][j] == -1))){
	      board[i - 1][j]++;
	    }
      if((i - 1) != -1 && (j + 1) != (NCOL) && (!(board[i-1][j+1] == -1))){
	      board[i - 1][j + 1]++;
    	}
      if((j - 1) != -1 && (!(board[i][j-1] == -1))){
	     board[i][j - 1]++;
	    }
      if((j + 1) != (NCOL) && (!(board[i][j+1] == -1))){
	      board[i][j + 1]++;
	    }
      if((i + 1) != (NROW) && (j - 1) != -1 && (!(board[i+1][j-1] == -1))){
	      board[i + 1][j - 1]++;
	    }
      if((i + 1) != (NROW) && (j) != -1 && (!(board[i+1][j] == -1))){
	      board[i + 1][j]++;
	    }
      if((i + 1) != (NROW) && (j + 1) != (NCOL) && (!(board[i+1][j+1] == -1))){
	      board[i + 1][j + 1]++;
	    }
	    nbomb--;
    }
    ++n;
    ++n;
    ++n;
  }
}

/*
                 _             _    __                      
                | |           | |  / _|                     
  ___ ___  _ __ | |_ _ __ ___ | | | |_ _   _ _ __   ___ ___ 
 / __/ _ \| '_ \| __| '__/ _ \| | |  _| | | | '_ \ / __/ __|
| (_| (_) | | | | |_| | | (_) | | | | | |_| | | | | (__\__ \
 \___\___/|_| |_|\__|_|  \___/|_| |_|  \__,_|_| |_|\___|___/
                                                            
                                                            
*/                                                            

/*
********************************************************
GameControl - controls game
********************************************************
*/
int GameControl( int irow, int icol, int nclick, int mine[][NCOL], int mask[][NCOL])
{
  int i;
  int j;
  int trow;
  int tcol;
  if((mask[irow][icol] != mine[irow][icol]) && (mask[irow][icol] != 10)){
  mask[irow][icol] = mine[irow][icol];
  nclick++;
    if (mine[irow][icol] == -1){
      //printf("Fail!!!\n");
      return -1;
    }
    if (mask[irow][icol] == 0){
      for(i = 0;i < 3;i++){
        for(j = 0;j < 3;j++){
	        trow = irow + i - 1;
	        tcol = icol + j - 1;
	        if ((trow >=0) && (trow < NROW) && (tcol >= 0) && (tcol < NCOL) && (mask[trow][tcol] == 9)){
	          nclick = GameControl(trow,tcol,nclick,mine,mask);
	        }
        } 
      }
    }
  }
  return nclick;
} 
/*
********************************************************
FlagControl - controls flags
********************************************************
*/
int FlagControl(int frow, int fcol, int nflag, int mask[][NCOL])
{
  if(mask[frow][fcol] == 9){
    mask[frow][fcol] = 10;
    nflag++;
  }else if (mask[frow][fcol] == 10){
    mask[frow][fcol] = 9;
    nflag--;
  }
  return nflag;
}

/*
********************************************************
joy - interfaces with joystick
********************************************************
*/
int joy()
{
	int xjoy;
	int yjoy;
	
	int joymove = 0;    //also resets to 0 when nothing happens
	
	//ATDCTL5 = 0x10;
	
	while(!ATDSTAT0_SCF){   //wait until conversion is done
  }
  
	xjoy=ATDDR4H;
	yjoy=ATDDR5H;
	
	if(xjoy > 3 && (yjoy > 2 && yjoy < 3))
	{
		//move Left
	  joymove = 4;
	}
	else if(xjoy < 2 && (yjoy > 2 && yjoy < 3))
	{
		//move right
		joymove = 6;
	}
	else if(yjoy > 3 && (xjoy > 2 && xjoy < 3))
	{
		//move up
		joymove = 2;
	}
	else if(yjoy < 2 && (xjoy > 2 && xjoy < 3))
	{
		//move down
		joymove = 8;
	}
	
	return joymove;
}

/*
********************************************************
butts - converts buttons from analog inputs to digital 
********************************************************
*/
void butts(void)
{  
  //button B - commit
  while(!ATDSTAT0_SCF){   //wait until conversion is done
  }

  if(ATDDR6H >= 10){       //low voltage
    buttBatd = 1;
  }else{
    buttBatd = 0;
  }
  
  //button A - change mode
  
  if(ATDDR7H >= 10){       //low voltage
    buttAatd = 1;
  }else{
    buttAatd = 0;
  }
   
}
/*
       _   _ _ _ _         
      | | (_) (_) |        
 _   _| |_ _| |_| |_ _   _ 
| | | | __| | | | __| | | |
| |_| | |_| | | | |_| |_| |
 \__,_|\__|_|_|_|\__|\__, |
                      __/ |
                     |___/ 
*/

/*
********************************************************
lcdpaint - interprets array of characters and sends array
to BOTH lcd displays
********************************************************
*/                     
//lcdpaint(int *array)
//{ 
//}

/*
********************************************************
pos - moves cursor to coordinates
********************************************************
*/                     
void pos(int x, int y){
  int addr;
  
  ////y interpretation
  //quadrant detection
  if(y >=4 && y <= 7){          //if quad 1 selected
    display1_en = 1;
    display2_en = 0;
  }else if(y >= 0 && y <=3){    //if quad 2 selected
    display1_en = 0;
    display2_en = 1;
  }
  //even/odd detection
  if((y % 2) == 0){           //if row is odd
    addr = 128;
  }else{                      //if row is even
    addr = 192;
  }
  //specific row detection
  if(y == 5 || y == 4 || y == 1 || y == 0){   //higher rows
    addr += x;
  }else{
    addr += 20 + x;
  }    
  
  send_i(CURMOV);
  send_i(addr);
}


/*
 _         _                 _      
| |       | |               | |     
| | ___ __| |   ___ ___   __| | ___ 
| |/ __/ _` |  / __/ _ \ / _` |/ _ \
| | (_| (_| | | (_| (_) | (_| |  __/
|_|\___\__,_|  \___\___/ \__,_|\___|

*/                                    
                                    
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
  	
  	/* 
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
	  */
	  
	  if (PORTAD0_PTAD7 < prevpb){        //button A
	    prevpb = 0;
	    buttApb = 1;
	  } else if (PORTAD0_PTAD7 == 1){
	    prevpb = 1;
	  }
	  
	  if (PORTAD0_PTAD6 < prevpb){        //button B     
	    prevpb = 0;
	    buttBpb = 1;
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
  // shift out character
    shiftout(x);
  // pulse LCD clock line low->high->low
    PTT_PTT4 = 0;
    PTT_PTT4 = 1;
    PTT_PTT4 = 0;
  // wait 2 ms for LCD to process data
    lcdwait(2);
    
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
    int i = 0;
    while(str[i] != '\0'){
      print_c(str[i]);
      if(i == strlen(str)){  
        return;
      }
      i++;
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


