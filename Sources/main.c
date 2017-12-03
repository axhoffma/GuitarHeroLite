/*
************************************************************************
 ECE 362 - Mini-Project C Source File - Fall 2016
***********************************************************************
	 	   			 		  			 		  		
 Team ID: < ? >

 Project Name: < ? >

 Team Members:

   - Team/Doc Leader: < ? >      Signature: ______________________
   
   - Software Leader: < ? >      Signature: ______________________

   - Interface Leader: < ? >     Signature: ______________________

   - Peripheral Leader: < ? >    Signature: ______________________


 Academic Honesty Statement:  In signing above, we hereby certify that we 
 are the individuals who created this HC(S)12 source file and that we have
 not copied the work of any other student (past or present) while completing 
 it. We understand that if we fail to honor this agreement, we will receive 
 a grade of ZERO and be subject to possible disciplinary action.

***********************************************************************

 The objective of this Mini-Project is to .... < ? >


***********************************************************************

 List of project-specific success criteria (functionality that will be
 demonstrated):

 1. Output a sound based off of if the player hit the note at the right
    time

 2. Display the "game board" of notes that the player needs to hit

 3. Display the player's score on the LED screen

 4.

 5.

***********************************************************************

  Date code started: 12/3/2017 

  Update history (add an entry every time a significant change is made):

  Date: < ? >  Name: < ? >   Update: < ? >

  Date: < ? >  Name: < ? >   Update: < ? >

  Date: < ? >  Name: < ? >   Update: < ? >


***********************************************************************
*/

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "platform.h"   /* Common device operations */
#include <mc9s12c32.h>

/* All functions after main should be initialized here */
char inchar(void);
void outchar(char x);

/* LCD Display functions */
void lcd_disp(void);
void shiftout(char);
void lcdwait(void);
void send_byte(char);
void send_i(char);
void chgline(char);
void print_c(char);
void pmsglcd(char[]);

/* Unit test functions */
void screen_test(void);
void score_test(void);
void push_test(void);

/* Score functions */
void update_score(int);
void display_score(void);



/* Test function initilization here */
//#define SCREEN_TEST
#define SCORE_TEST
//#define PUSH_TEST



/* Variable declarations */
int playerScore = 0;
int maxScore = 0;

   	   			 		  			 		       

/* Special ASCII characters */
#define CR 0x0D		// ASCII return 
#define LF 0x0A		// ASCII new line 

/* LCD COMMUNICATION BIT MASKS (note - different than previous labs) */
#define LCDRS  PTT_PTT2		// RS pin mask (PTT[2])
#define LCDRW  PTT_PTT3		// R/W pin mask (PTT[3])
#define LCDCLK PTT_PTT4 	// LCD EN/CLK pin mask (PTT[4])

/* LCD INSTRUCTION CHARACTERS */
#define LCDON 0x0F	// LCD initialization command
#define LCDCLR 0x01	// LCD clear display command
#define TWOLINE 0x38	// LCD 2-line enable command
#define CURMOV 0xFE	// LCD cursor move instruction
#define LINE1 = 0x80	// LCD line 1 cursor position
#define LINE2 = 0xC0	// LCD line 2 cursor position


	 	   		
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

/* Initialize asynchronous serial port (SCI) for 9600 baud, interrupts off initially */
  SCIBDH =  0x00; //set baud rate to 9600
  SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  
  SCICR1 =  0x00; //$9C = 156
  SCICR2 =  0x0C; //initialize SCI for program-driven operation
  DDRB   =  0x10; //set PB4 for output mode
  PORTB  =  0x10; //assert DTR pin on COM port

/* Initialize SPI device for 6 Mbs baud rate */
  SPIBR = 0x01;
  SPICR2 = 0x00;
  SPICR1 = 0x50;

/* Initialize RTI 2.048 ms interrupt rate */
  RTICTL = 0x41;
  CRGINIT = 0x80;

/* Initialize TIM CH7 for periodic interrupts every ms */
  TSCR1 = 0x80; //Enable system
  TIOS = 0x80; //Output compare on channel 7
  TIE = 0x00; //No interrupts initially
  TCTL1 = 0x00; //Disconnected from output logic
  TSRCR2 = 0x0E; //Counter Resets on Channel 7. Clock scaler = 64
  TC7 = 375;

/* Initialize LED screen */
  LCDCLK = 0; 
  LCDRW  = 1;
  send_i(LCDON);
  send_i(TWOLINE);
  send_i(LCDCLR);
  lcdwait();
	      
}

	 		  			 		  		
/*	 		  			 		  		
***********************************************************************
Main
***********************************************************************
*/
void main(void) {
    DisableInterrupts
    initializations(); 		  			 		  		
    EnableInterrupts;
#ifdef SCREEN_TEST
    screen_test();
#endif
#ifdef PUSH_TEST
    push_test();
#endif 
#ifdef SCORE_TEST
    score_test();
#endif 
    for(;;) {

    } /* loop forever */
   
}   /* do not leave main */




/*
***********************************************************************                       
 RTI interrupt service routine: RTI_ISR
************************************************************************
*/

interrupt 7 void RTI_ISR(void)
{
    // clear RTI interrupt flagt 
    CRGFLG = CRGFLG | 0x80; 
}

/*
***********************************************************************                       
  TIM interrupt service routine	  		
***********************************************************************
*/

interrupt 15 void TIM_ISR(void)
{
    // clear TIM CH 7 interrupt flag 
    TFLG1 = TFLG1 | 0x80; 
}

/*
***********************************************************************                       
  Score routines
***********************************************************************
*/
void score_test() {
    int i = 0;
    for(i = 1; i < 501; i++) {
        update_score(i);
    }
    update_score(0);
    for(i = 1; i < 510; i++) {
        update_score(i);
    }
}
void update_score(int hit) {
    if(hit) {
        playerScore++;
    }
    if(playerScore > maxScore) [
        maxScore = playerScore;
    }
    display_score();
}

void display_score(void) {
    lcd_disp();
}

/*
***********************************************************************                       
  LCD Printing routines		 		  		
***********************************************************************
*/
void lcd_disp() {
    char output;
    send_i(LCDCLEAR);
    chgline(LINE1);
    pmsglcd("Score: ");
    pmsglcd("todo"); //TODO print their score
    chgline(LINE2);
    pmsglcd("todo");
    pmsglcd(); //TODO print the high score
}

void shiftout(char output) {
    //Wait for register to be clear
    while(!SPISR_SPRTEF) {}
    SPIDR = output; 
    //Wait for 30 cycles
    asm {
        pshx
        psha
        pshc
        ldx #6
        loopi:
        dbne x,loopi
        pulc
        pula
        pulx
    }
}

void lcdwait(void) {
    //Wait for 2ms
    asm {
        pshx
        psha
        pshc
        ldx #7992
        loopi:
        dbne x,loopi
        pulc
        pula
        pulx
    }
}

void send_byte(char byte) {
    shiftout(byte);
    LCD_CLOCK = 0;
    LCD_CLOCK = 1;
    LCD_CLOCK = 0;
    lcdwait();
}

void send_i(char instruction) {
    LCDRS = 0;
    send_byte(instruction);
}

void chgline(char line) {
    send_i(CURMOV);
    send_i(line);
}

void print_c(char character) {
    LCDRS = 1;
    send_byte(character);
}

void pmsglcd(char[] str) {
    int i = 0;
    while(str[i] != '\0') {
        print_c(str[i]);
        i++;
    }
}

/*
***********************************************************************
 Character I/O Library Routines for 9S12C32 
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
 Name:         outchar    (use only for DEBUGGING purposes)
 Description:  outputs ASCII character x to SCI serial port
 Example:      outchar('x');
***********************************************************************
*/

void outchar(char x) {
  /* sends a character to the terminal channel */
    while (!(SCISR1 & 0x80));  /* wait for output buffer empty */
    SCIDRL = x;
}
