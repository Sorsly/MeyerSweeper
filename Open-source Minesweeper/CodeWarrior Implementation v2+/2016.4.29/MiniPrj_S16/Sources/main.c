/*
************************************************************************
 ECE 362 - Mini-Project C Source File - Spring 2016
***********************************************************************
	 	   			 		  			 		  		
 Team ID: 12

 Project Name: Meyer Sweeper

 Team Members:

   - Team/Doc Leader: Thao Nguyen      Signature: ___Thao Nguyen_______
   
   - Software Leader: Zhihao Lin      Signature: ___Zhihao Lin________

   - Interface Leader: Alan Han     Signature: ___Alan Han__________

   - Peripheral Leader: Alan Han    Signature: ___Alan Han__________


 Academic Honesty Statement:  In signing above, we hereby certify that we 
 are the individuals who created this HC(S)12 source file and that we have
 not copied the work of any other student (past or present) while completing 
 it. We understand that if we fail to honor this agreement, we will receive 
 a grade of ZERO and be subject to possible disciplinary action.

***********************************************************************

 The objective of this Mini-Project is to construct a "portable" gamepad
 for our minesweeper clone game realized with a 9S12C32 microcontroller,
 (2) 4x20 LCD character displays, a shift register realized on a GAL22V10,
 a joystick, and some pushbuttons. Our demonstration will run off of
 a wallwart for practical purposes.

***********************************************************************

 List of project-specific success criteria (functionality that will be
 demonstrated):

 1. LCDs display information in the proper format (both displays work together)

 2. During gameplay, the timer function works as expected (counts up)

 3. The joystick can be read and utilized to play the game

 4. The leds will operate according to game win/lose conditions

 5. The game is enjoyable :)

***********************************************************************

  Date code started: 04/22/16

  Update history (add an entry every time a significant change is made):

  Date: 04/22/16  Name: Zhihao   Update: started game engine

  Date: 04/23/16  Name: Zhihao   Update: continued work on game engine

  Date: 04/25/16  Name: Zhihao & Alan  Update: figured out ddram addressing
  
  Date: 04/27/16  Name: Zhihao & Alan  Update: programmed different stages of game
  
  Date: 04/28/16  Name: Zhihao & Alan & Thao Update: programmed difficulty, enabling of screens
  
  Date: 04/29/16  Name: Zhihao & Alan  Update: interfaced joystick, menu design


***********************************************************************
*/

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>

#define NROW 8
#define NCOL 10
//#define NBOMB 8

////////////////////////////////////////function declarations
void Initial_board(int(*)[]);
void Initial_mask(int(*)[]);
void Create_board(int,int(*)[]);//set mine and set number of mines around
void GameControl(int,int);//run this function when click one button
void FlagControl(int,int);//flag function
int joy();//joystick function
void difficulty(void);
void gameInfo(void);
void minejoy(void);
void minebutt(void);
void endgame(void);
//-1 means mine; 9 means not clicked; 10 means flag; 11 means question

void splashDisp(void);
void diffDisp(void);
void gameDisp(void);
void gameRefresh(void);

//2-disp utility funcs
void pos(int, int);
void lclr(void);
void mensel(int, int, int);
void timchar(void);
void minchar(void);

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
//pushbutton flag
int prevpb	= 0;  // previous pushbutton state
int buttApb = 0;  // button A flag
int buttBpb = 0;  // button B flag

//flags
int gamerun	= 0;  // gamerun flag
int diffrun = 0;  // difficulty menu flag
int flag1 = 1;    //splash stage flag 
int flag2 = 0;    //diff stage flag
int flag3 = 0;    //game stage flag
int flag4 = 0;    //win/lose stage flag

//indices
int diffMenu = 0;   //diff menu item index
int gameMode = 0;   //cursor mode type (dig: 0, flag: 1, )

//info hold 
int curhold;        //stores char code for tile under cursor
int maskhold[NROW][NCOL];   //stores previous map
int irowhold;       //prev irow
int icolhold;       //prev icol

//counters
int timer = 0;    //time counter var
int timerms = 0;  //ms time counter var 
char timar[4];     //time char array
char minar[3];    //mine char array
int curcnt;       //cursor counter


/* Game var declarations*/
int irow = 0;             //place clicked
int icol = 0;             //place clicked
int frow = 0;             //place flag
int fcol = 1;             //place flag
int nbomb = 12;       //number of bomb
int nclick = 0;           //number of empty tiles with no mines nearby
int nflag = 0;
int fail = 0;
int win = 0;
int joymove;
int mine[NROW][NCOL];     //map with mine and other things
int mask[NROW][NCOL];     //cover map
int i;
int j;
int a;
int b;

int dumb;
int truth =0;

int buttAatd = 0;
int buttU = 0;
int buttD = 0;
int buttL = 0;
int buttR = 0;



/* ASCII character definitions */
#define CR 0x0D	// ASCII return character   

/* LCD COMMUNICATION BIT MASKS */
#define RS 0x04		// RS pin mask (PTT[2])
#define RW 0x08		// R/W pin mask (PTT[3])
#define LCDCLK 0x10	// LCD EN/CLK pin mask (PTT[4])

/* LCD INSTRUCTION CHARACTERS */
#define LCDON 0x0D	// LCD initialization command     no cursor, blink on
#define LCDCLR 0x01	// LCD clear display command
#define TWOLINE 0x38	// LCD 2-line enable command
#define CURMOV 0xFE	// LCD cursor move instruction
#define LINE1 0x80	// LCD line 1 cursor position
#define LINE2 0xC0	// LCD line 2 cursor position     no cursor, blink off

#define CUROFF 0x0C   //turns off cursor

#define DISP1EN PTT_PTT3    //lower-half 4x20 pull high to dis
#define DISP2EN PTT_PTT5    //upper-half 4x20 pull high to dis

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
  lcdwait(4); 

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
  TIE =   0b10000000; //interrupt enabled


//  Add RTI/interrupt initializations here
  
  CRGINT = 0b10000000;    //enables RTI 

  ATDCTL2 = 0b10000000;   //enable atd
  ATDCTL3 = 0b00010000;   //sample length (2)
  ATDCTL4 = 0b10000101;   //set to 8 bit control mode, prescale clock to 2Mhz
  ATDCTL5 = 0b00110100;   //scan sample across multiple channels
  //ATDCTL2_AFFC = 1;       //set to fast flag clear mode
  
  /* PWM Initializations */
  
  MODRR     = 0b00000011; // PT0 is PWM Ch 1&0 output
  PWME	    =	0b00000011;	// PWM enable
  PWMPOL	  =	0b00000001;	// PWM polarity
  PWMCLK	  =	0b00000000;	// PWM clock source select (ch0 SA)
  PWMPRCLK  =	0b00000001;	// PWM pre-scale clock select 
  PWMCAE	  =	0b00000000;	// PWM center align enable (left)
  PWMCTL	  =	0b00000000;	// PWM control (concatenate enable)
  PWMPER0	  =	0b11111111;	// PWM period registers   pg 293  FF
  PWMDTY0	  =	0b00000000;	// PWM duty registers     pg 293  50%
  PWMPER1	  =	0b11111111;	// PWM period registers   pg 293  FF
  PWMDTY1	  =	0b00000000;	// PWM duty registers     pg 293  50%

}
	 		  			 		  		
/*
***********************************************************************
                 _                           _               
                (_)                         | |              
 _ __ ___   __ _ _ _ __    ______   ___  ___| |_ _   _ _ __  
| '_ ` _ \ / _` | | '_ \  |______| / __|/ _ \ __| | | | '_ \ 
| | | | | | (_| | | | | |          \__ \  __/ |_| |_| | |_) |
|_| |_| |_|\__,_|_|_| |_|          |___/\___|\__|\__,_| .__/ 
                                                      | |    
                                                      |_|    
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
  send_i(LCDCLR);                   //for redundancy purposes
/*
***********************************************************************
                 _                  _                   
                (_)                | |                  
 _ __ ___   __ _ _ _ __    ______  | | ___   ___  _ __  
| '_ ` _ \ / _` | | '_ \  |______| | |/ _ \ / _ \| '_ \ 
| | | | | | (_| | | | | |          | | (_) | (_) | |_) |
|_| |_| |_|\__,_|_|_| |_|          |_|\___/ \___/| .__/ 
                                                 | |    
                                                 |_|    
***********************************************************************
*/

  for(;;) {                           
    if(diffrun == 0 && gamerun == 0){                 //splash screen
      if(flag1 == 1){
        splashDisp();
        flag1 = 0;
        
      }
      if(buttBpb && prevpb == 0){                            //buttBpb (FLAG), as opposed to buttBatd
        diffrun = 1;
        flag2 = 1;
        buttBpb = 0;                          //resets button B
        
      }    
    }
    
    
    else if(diffrun == 1 && gamerun == 0){           //difficulty screen
      if(flag2 == 1){
        diffDisp();
        flag2 = 0;
      }      
      difficulty();
      if(buttBpb && prevpb == 0){
        gamerun = 1;
        diffrun = 0;
        flag3 = 1;
        buttBpb = 0;                          //resets button B
      }
    }
    
    
    else if(diffrun == 0 && gamerun == 1){
      if(flag3 == 1){
        send_i(LCDCLR);
        gameDisp();                                 //game screen
        flag3 = 0;
      }
      gameInfo();                             //sidebar info
      minejoy();                              //joystick func in game stage
      minebutt();                             //click func in game stage
      gameRefresh();

      endgame();
    }
  } /* loop forever */
}


/*
     _      _                 
    | |    | |                
  __| | ___| |__  _   _  __ _ 
 / _` |/ _ \ '_ \| | | |/ _` |
| (_| |  __/ |_) | |_| | (_| |
 \__,_|\___|_.__/ \__,_|\__, |
                         __/ |
                        |___/ 
                              
      
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
*/


/*
________________________________________________   
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \  
 \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 
  > > > > > > > > > > > > > > > > > > > > > > > > >
 / / / / / / / / / / / / / / / / / / / / / / / / / 
/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/  
                                                   
*/                                                   
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
     _ _              __                      
    | (_)            / _|                     
  __| |_ ___ _ __   | |_ _   _ _ __   ___ ___ 
 / _` | / __| '_ \  |  _| | | | '_ \ / __/ __|
| (_| | \__ \ |_) | | | | |_| | | | | (__\__ \
 \__,_|_|___/ .__/  |_|  \__,_|_| |_|\___|___/
            | |                               
            |_|                               
*/

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
  int n = timerms;
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
********************************************************
splashDisp - displays splash screen waiting for button press
********************************************************
*/
void splashDisp(void) 
{
  lclr();
  pos(3,2);
  pmsglcd("MEYER  SWEEPER");
  //pmsglcd("SWEEPER");
  
  pos(3,5);
  pmsglcd("Press   joystick");
  pos(5,6);
  pmsglcd("to start");
}

/*
********************************************************
diffDisp - displays splash screen for difficulty selection
********************************************************
*/
void diffDisp(void)
{
  lclr();
  
  pos(8,2);
  pmsglcd("Easy");
  
  pos(8,4);
  pmsglcd("Hard");
  
  pos(9,5);
  pmsglcd("GG");
  
  pos(7,3);
  pmsglcd("Normal");
}

/*
********************************************************
gameDisp - displays game screen for PLAY :D
********************************************************
*/
void gameDisp(void)
{
  lclr();
  Create_board(nbomb,mine);
  for(i = 0; i < NROW; i++){          //y var
    for(j = 0; j < NCOL; j++){        //x var
      pos(j+5,i);
      if(mask[i][j] == 9){
        pmsglcd("#");
      }
    }
  }
}

/*
********************************************************
gameRefresh - refreshes updates on the map
********************************************************
*/
void gameRefresh(void)
{
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
difficulty - choose difficulty
********************************************************
*/

void difficulty(void) 
{
  if(joymove == 8){         //up menu
    diffMenu--;
    if(diffMenu <= -1){
      diffMenu = 3;
    }
  }else if(joymove == 2){   //down menu
    diffMenu++;
    if(diffMenu >= 4){
      diffMenu = 0;
    }
  }
  
  if(diffMenu == 0){        //easy
    nbomb = 8;
    
    mensel(7,2,6);
  }else if(diffMenu == 1){  //normal
    nbomb = 12;
    mensel(7,3,6);
  }else if(diffMenu == 2){  //hard
    nbomb = 16;
    mensel(7,4,6);
  }else if(diffMenu == 3){  //gg
    nbomb = 79;
    mensel(7,5,6);
  }
}

/*
********************************************************
GameControl - controls game
********************************************************
*/
void GameControl( int irow, int icol)
{
  int i;
  int j;
  int trow;
  int tcol;
  //if((mask[irow][icol] != mine[irow][icol]) && (mask[irow][icol] == 9)){
  mask[irow][icol] = mine[irow][icol];
  nclick++;
    if (mine[irow][icol] == -1){
      //printf("Fail!!!\n");
      nclick = -1;
    }
    if (mask[irow][icol] == 0){
      for(i = 0;i < 10;i++){
        for(j = 0;j < 8;j++){
	        trow = irow + i - 5;
	        tcol = icol + j - 4;
	        if ((trow >=0) && (trow < NROW) && (tcol >= 0) && (tcol < NCOL) && (mask[trow][tcol] == 9)){
	           mask[trow][tcol] = mine[trow][tcol];
	           nclick++;
	        }
        } 
      }
    }    
    /*
    if (mask[irow][icol] == 0){
      for(i = 0;i < 3;i++){
        for(j = 0;j < 3;j++){
	        trow = irow + i - 1;
	        tcol = icol + j - 1;
	        if ((trow >=0) && (trow < NROW) && (tcol >= 0) && (tcol < NCOL) && (mask[trow][tcol] == 9)){
	          GameControl(trow,tcol);
	        }
        } 
      }
    }
    */
  //}
}
/*
********************************************************
FlagControl - controls flags
********************************************************
*/
void FlagControl(int frow, int fcol)
{
  if(mask[frow][fcol] == 9){
    mask[frow][fcol] = 10;
    nflag++;
  }else if (mask[frow][fcol] == 10){
    mask[frow][fcol] = 11;
    nflag--;
  }else if (mask[frow][fcol] == 11){
    mask[frow][fcol] = 9;
  }

}

/*
********************************************************
joy - interfaces with joystick
********************************************************
*/
int joy()
{
int joyflag = 1;
int uplimH = 192;
int uplimL = 160;
int lowlimH = 96;
int lowlimL = 64;

	//ATDCTL5 = 0b00010100;
  
	while(!ATDSTAT0_SCF){   //wait until conversion is done
    asm{
        nop;
      }
	}
    
  if(joyflag && ATDDR0H > uplimH && (ATDDR1H > lowlimL && ATDDR1H < uplimH))
	{
		//move Left
	  joymove = 4;
	  joyflag = 0;
	}
	else if(joyflag && ATDDR0H < lowlimL && (ATDDR1H > lowlimL && ATDDR1H < uplimH))
	{
		//move right
		joymove = 6;
	  joyflag = 0;
	}
	else if(joyflag && ATDDR1H > uplimH && (ATDDR0H > lowlimL && ATDDR0H < uplimH))
	{
		//move up
		joymove = 8;
	  joyflag = 0;
	}
	else if(joyflag && ATDDR1H < lowlimL && (ATDDR0H > lowlimL && ATDDR0H < uplimH))
	{
		//move down
		joymove = 2;
	  joyflag = 0;
	} else if((ATDDR1H > lowlimH && ATDDR1H < uplimL) && (ATDDR0H > lowlimH && ATDDR0H < uplimL)){
	  //move center
	  joymove = 0;
	  joyflag = 1;
	}
	
	return joymove;
}

/*
********************************************************
gameInfo - displays game info on sides of game
********************************************************
*/
void gameInfo(void)
{
int curper = 4; //even number cursor period value  
  
  //timer info
  pos(0,0);             
  pmsglcd("Time");
  pos(0,1);
  timchar();
  pmsglcd(timar);
  pos(3,1);  print_c(0b11101111);
  
  //mine info
  pos(16,0);              
  pmsglcd("Mine");
  pos(16,1);
  minchar();
  pmsglcd(minar);
  pos(19,1);
  print_c(0b11111100);
  
  //cursor info
  pos(16,4);
  pmsglcd("Cur");
  pos(16,5);
  if(gameMode == 0){
    pmsglcd("Left");    
    //print_c(0b10101011);
  }else if(gameMode == 1){
    pmsglcd("Rght");
    //print_c(0b11100110);
  }//else if(gameMode == 2){
    //print_c('?');
  //}    
  
  //map info
  irowhold = irow;
  icolhold = icol;
  
  if(mask[irow][icol] == 9){
    curhold = '#';
  }else if(mask[irow][icol] == 10){
    curhold = 0b11100110;
  }else if(mask[irow][icol] == 11){
    curhold = '?';
  }else if(mask[irow][icol] == 0){
    curhold = 0b00010000;
  }else if(mask[irow][icol] >= 1 && mask[irow][icol] <= 8){
    curhold = 30 + mask[irow][icol];
  }
  
  curcnt++;
  if(curcnt >= curper){
    curcnt = 0;
  }else if(curcnt <= curper/2){   
    pos(icol+5,irow);
    print_c(0b11011011);
  }else if(curcnt > curper/2){    
    pos(icolhold+5,irowhold);
    print_c(curhold);     
  }

  if((irow != irowhold) && (icol != icolhold)){
    pos(icolhold+5,irowhold);    print_c(curhold);  }
  
}

/*
********************************************************
minejoy - controls joy interactions during game stage
********************************************************
*/
void minejoy(void){
  
  if(joymove != 0){ 
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
      if(irow != (NROW - 1)){
        irow++;
      }
    }
  }
}
/*
********************************************************
minebutt - controls button interactions during game stage
********************************************************
*/
void minebutt(void)
{
  if(buttApb){
    gameMode++;
    if(gameMode >= 2){
      gameMode = 0;
    }
    buttApb = 0; 
  }
  if(buttBpb){
    pos(icol,irow);                             //x,y
    //mask[y][x]
    if(mask[irow][icol] == 9){
      if(gameMode == 0){//fog o war  
        GameControl(irow,icol);
      }else if(gameMode == 1){
        FlagControl(irow,icol);
      }
    }else if (mask[irow][icol] == 10){
      if(gameMode == 1){
        FlagControl(irow,icol);
      }
    }else if (mask[irow][icol] == 11){
      if(gameMode == 1){
        FlagControl(irow,icol);
      }
    }
    for(i = 0; i < NROW; i++){          //y var
      for(j = 0; j < NCOL; j++){        //x var
        pos(j+5,i);
        if(mask[i][j] == 9){
          pmsglcd("#");
        }else if(mask[i][j] == 10){
          print_c(0b11100110);
        }else if(mask[i][j] == 11){
          pmsglcd("?");
        }else if(mask[i][j] == 0){
          print_c(0b00010000);
        }else if(mask[i][j] == 1){
          pmsglcd("1");
        }else if(mask[i][j] == 2){
          pmsglcd("2");
        }else if(mask[i][j] == 3){
          pmsglcd("3");
        }else if(mask[i][j] == 4){
          pmsglcd("4");
        }else if(mask[i][j] == 5){
          pmsglcd("5");
        }else if(mask[i][j] == 6){
          pmsglcd("6");
        }else if(mask[i][j] == 7){
          pmsglcd("7");
        }else if(mask[i][j] == 8){
          pmsglcd("8");
        }
          //dumb = 30 + mask[i][j];
          //dumb = 0b11100110;
          //truth = 99;
          //print_c(dumb);
        
      }
    }
}
      
        
        
      //print_c("#");
/*    }else if(gameMode == 0 && mask[irow][icol] == 0){                           //blank
      nclick = GameControl(irow,icol,nclick,mine,mask);
      print_c(0b11011011);
    }else if(gameMode == 0 && mask[irow][icol] >= 1 && mask[irow][icol] <= 8){  //num
      nclick = GameControl(irow,icol,nclick,mine,mask);
      dumb = 30 + mask[irow][icol];
      print_c(dumb);
    }else if(gameMode == 1 && mask[irow][icol] == 10){                          //F
      nflag = FlagControl(frow,fcol,nflag,mask);
      pmsglcd("F");
    }else if(gameMode == 2 && mask[irow][icol] == 11){                          //?
      pmsglcd("?");
    }
    */
    buttBpb = 0;
   
}

/*
********************************************************
endgame - determines conditions for endgame
********************************************************
*/
void endgame(void) 
{
  if((nflag + nclick) == NROW*NCOL){
      win = 1;
      
      pos(0,5);
      pmsglcd("YOU");
      pos(0,6);
      pmsglcd("WIN:)");
    }
  if(nclick == -1){
    fail = 1;
    
    pos(0,5);
    pmsglcd("YOU");
    pos(0,6);    pmsglcd("LOSE");
    
    for(a = 0; a < NROW; a++){
      for(b = 0; b < NCOL; b++){
        mask[a][b] = mine[a][b];
      }
    }  
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
  if(y >=4 && y <= 7){          //if quad 2 selected (DISP2EN low)    
    DISP2EN = 1;                //turns off cursor of quad 1
    DISP1EN = 0;
    send_i(CUROFF);
    
    DISP2EN = 0;                //turns on cursor of quad 2
    DISP1EN = 1;
    send_i(LCDON);
  
  }else if(y >= 0 && y <=3){    //if quad 1 selected
    DISP2EN = 0;                //turns off cursor of quad 2
    DISP1EN = 1;
    send_i(CUROFF);
    
    DISP2EN = 1;
    DISP1EN = 0;
    send_i(LCDON);
  
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
********************************************************
lclr - clear entire screen (both quadrants)
********************************************************
*/                     
void lclr(void){
int save[2];
  
  save[0] = DISP2EN;
  save[1] = DISP1EN;
  
  DISP2EN = 0;
  DISP1EN = 0;
  send_i(LCDCLR);
  
  DISP2EN = save[0];
  DISP1EN = save[1];
}

/*
********************************************************
mensel - create selectors for menu, then return cursor
to first char of menu character
********************************************************
*/                     
void mensel(int x, int y, int leng){
  
  if(joymove != 0){
    for(i=2;i<=5;i++){
      pos(x-2,i);
      print_c(' ');
      pos(x+leng+1,i);
      print_c(' ');
    }         
  }
  pos(x-2,y);
  print_c('*');
  pos(x+leng+1,y);
  print_c('*');
  pos(x,y);  
}

/*
********************************************************
timchar - converts tim int to char
********************************************************
*/
void timchar(void) 
{ 
int re;
int ones;
int tens;
int huns;

  re = timer;
  ones = re%10;
  re = re/10;
  tens = re%10;
  re = re/10;
  huns = re;
  
  timar[0] = huns + '0';
  timar[1] = tens + '0';
  timar[2] = ones + '0';
  timar[3] = '\0';    
}

/*
********************************************************
minchar - converts tim int to char
********************************************************
*/
void minchar(void) 
{ 
int re;
int ones;
int tens;

  re = nbomb - nflag;
  ones = re%10;
  re = re/10;
  tens = re;
  
  minar[0] = tens + '0';
  minar[1] = ones + '0';
  minar[2] = '\0';    
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
  	
  	//PORTADO_PTAD = 1 means NOT PRESSED
  	//PORTADO_PTAD = 0 means PRESSED
  	//buttApb = 0 means NOT PRESSED
  	//buttApb = 1 means PRESSED
  	
	  if(PORTAD0_PTAD7 == 1 && prevpb == 0){
	    prevpb = 1;
	    buttApb = 0;
	  }else if(PORTAD0_PTAD7 == 0 && prevpb == 1){        //button A
	    prevpb = 0;
	    buttApb = 1;
	  }else if(PORTAD0_PTAD7 == 1){
	    prevpb = 1;
	  } 
    
    if(PORTAD0_PTAD6 == 1 && prevpb == 1){
      prevpb = 1;
      buttBpb = 0;  
    }else if(PORTAD0_PTAD6 == 0 && prevpb == 1){        //button B     
	    prevpb = 0;
	    buttBpb = 1;
	  }else if(PORTAD0_PTAD6 == 1){
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

  timerms++;
  if(timerms == 1000){
    timer++;
    timerms = 0;
  }
  if(diffrun == 1 && gamerun == 0 || diffrun == 0 && gamerun == 1){
    joy();
  }
  
  if(win == 1 && fail == 0){ 
    PWMDTY0++;
    if(PWMDTY0 >= 255){
      PWMDTY0 = 0;
    }
    PWMDTY1--;
    if(PWMDTY1 <= 0){
      PWMDTY1 = 255;
    }
    
  }else if(win == 0 && fail == 1){
    PWMDTY0 = 255;
    PWMDTY1 = 255;
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


