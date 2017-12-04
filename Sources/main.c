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
void display_buttons(void);

/* Sound functions */
void populate_song(void);


/* Test function initilization here */
//#define SCREEN_TEST
//#define SCORE_TEST 1
//#define PUSH_TEST



/* Variable declarations */
//table below is the PWMDTY value that will generate a sine wave when cycled through
unsigned char sineArray [200] = {127,131,135,139,142,146,150,154,158,162,166,170,173,177,181,184,188,191,195,198,201,205,208,211,214,217,219,222,225,227,230,232,234,236,238,240,242,243,245,246,248,249,250,251,251,252,253,253,253,253,253,253,253,253,252,252,251,250,249,248,247,246,244,243,241,239,237,235,233,231,228,226,223,221,218,215,212,209,206,203,200,197,193,190,186,183,179,175,172,168,164,160,156,152,148,144,141,137,133,129,125,120,116,113,109,105,101,97,93,89,85,81,78,74,70,67,63,60,56,53,50,47,44,41,38,35,32,30,27,25,22,20,18,16,14,12,10,9,7,6,5,4,3,2,1,1,0,0,0,0,0,0,0,0,1,2,2,3,4,5,7,8,10,11,13,15,17,19,21,23,26,28,31,34,36,39,42,45,48,52,55,58,62,65,69,72,76,80,83,87,91,95,99,103,107,110,114,118,122,126};
unsigned char sineptr = 0; //ptr used to cycle through the sine values

int playerScore = 0;
int highScore = 0;

enum note{C3 = 459, C3s = 433, D3 = 409, D3s = 386, E3 = 364, F3 = 344, F3s = 324, G3 = 306,
          G3s = 289, A3 = 273, A3s = 258, B3 = 243, C4 = 229, C4s = 216, D4 = 204, D4s = 193,
          E4 = 182, F4 = 172, F4s = 162, G4 = 153, G4s = 144, A4 = 136, A4s = 129, B4 = 121};
char runstp = 1;
unsigned char input = 0;

char screen[32] = {' '};
char temp = ' ';



/* Structure to represent a musical note */
typedef struct Note {
    int note; 
    int beats;
} Note;


Note lastNote; //duration of the last note
int rtiCnt = 0; //number of interrupts since last update

/*Array of Notes that represents the song */
#define SONG_SIZE 4 
Note song[SONG_SIZE];
int songPtr = 0;

/*Array of ints that represents the playboard */
int board[SONG_SIZE];
int boardPtr = 0;

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
#define LINE1  0x80	// LCD line 1 cursor position
#define LINE2  0xC0	// LCD line 2 cursor position


/* Input port mappings */
#define INPUT1 PTAD_PTAD0
#define INPUT2 PTAD_PTAD1
#define INPUT3 PTAD_PTAD2
#define INPUT4 PTAD_PTAD3


#define NOTE1 0x80
#define NOTE2 0x40
#define NOTE3 0x20
#define NOTE4 0x10


	 	   		
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
  CRGINT = 0x80;

/* Initialize the PWM unit*/
  MODRR = 0x02; //set PT1 to output PWM signal
  PWME = 0x02;  //enable PWM Channel 1
  PWMPOL = 0x02; //set PWM Channel 1 to active high polarity
  PWMCTL = 0; //no concantenation 
  PWMCAE = 0; //no center align 
  PWMPER1 = 0xFF; //set max period for channel 1
  PWMDTY1 = 0; //set no duty cycle to start
  PWMCLK = 0; //select clock A for channel 1
  PWMPRCLK = 0; //set clock to 24 MHz
    
/* Initialize TIM CH7 for periodic interrupts every ms */
  TSCR1 = 0x80; //Enable system
  TIOS = 0x80; //Output compare on channel 7
  TIE = 0x00; //No interrupts initially
  TCTL1 = 0x00; //Disconnected from output logic
  TSCR2 = 0x09; //Counter Resets on Channel 7. Clock scaler = 2 
  TC7 = 229; //interrupts set up to fire an interrupt rate of 52,400 Hz for middle C

/* Initialize LED screen */
  DDRT = 0x1C;
  PTT_PTT4  = 1; 
  PTT_PTT3  = 0;
  send_i(LCDON);
  send_i(TWOLINE);
  send_i(LCDCLR);
  lcdwait();

/* initialize Pushbuttons */
  DDRAD = 0x00;
  ATDDIEN = 0x0F;
  
	      
}

void populate_song() {
    song[0].note = C4;
    song[0].beats = 2;
    song[1].note = E4;
    song[1].beats = 2;
    song[2].note = G4;
    song[2].beats = 2;
    song[3].note = E4;
    song[3].beats = 2;
    lastNote.note = song[0].note;
    lastNote.beats = song[0].beats;
    TC7 = lastNote.note;

    //Make the board
    board[0] = NOTE1;
    board[1] = NOTE3;
    board[2] = NOTE4;
    board[3] = NOTE3;
}
	 		  			 		  		
/*	 		  			 		  		
***********************************************************************
Main
***********************************************************************
*/
void main(void) {
    DisableInterrupts
    initializations(); 		  			 		  		
    populate_song();
#ifdef SCREEN_TEST
    screen_test();
#endif
#ifdef PUSH_TEST
    push_test();
#endif 
#ifdef SCORE_TEST
    score_test();
#endif 
    EnableInterrupts;
    TIE = 0x80;
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
    rtiCnt++;
    input = 0;
    if(INPUT1) {
        input = input ^ 0x80;
    }
    if(INPUT2) {
        input = input ^ 0x40;
    }
    if(INPUT3) {
        input = input ^ 0x20;
    }
    if(INPUT4) {
        input = input ^ 0x10;
    }

    //Check if we need to update the note
    if(rtiCnt >= (lastNote.beats * (293 / 2))){ 
        rtiCnt = 0;
        songPtr = (songPtr + 1) % SONG_SIZE;
        if(songPtr == 0) {
            playerScore = 0;
        }
        lastNote = song[songPtr];
        if(board[boardPtr] & input) {
            runstp = 1;
            //Send new note to be outputted
            TC7 = lastNote.note;
            //Update score
            update_score(1);
            //Update screen
        }
        else {
            runstp = 0;
            update_score(0);
        }
        boardPtr = (boardPtr + 1) % SONG_SIZE;
    }

    
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
    //generate the sinewave using the lookup table set up to generate a
    //261.6 Hz sinewave using a 52,320 Hz interrupt rate | closer to a 
    //262 Hz wave 
    if(runstp){
        PWMDTY1 = sineArray[sineptr];
        sineptr = (sineptr + 1) % 200;
    }
    else{ PWMDTY1 = 0;}
}

/*
***********************************************************************                       
  Sound routines
***********************************************************************
*/


void push_test() {
    for(;;) {
        display_buttons();
        lcdwait();
    }
}

void display_buttons() {
    chgline(LINE1);
    if(INPUT1) {
        print_c('1');
    }
    else {
        print_c('0');
    }
    if(INPUT2) {
        print_c('1');
    }
    else {
        print_c('0');
    }
    if(INPUT3) {
        print_c('1');
    }
    else {
        print_c('0');
    }
    if(INPUT4) {
        print_c('1');
    }
    else {
        print_c('0');
    }
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
    if(playerScore > highScore) {
        highScore = playerScore;
    }
    display_score();
}

void display_score(void) {
    char thousands;
    char hundreds;
    char tens;
    char ones;
    thousands = playerScore / 1000;
    hundreds = (playerScore % 1000) / 100;
    tens = ((playerScore % 1000) % 100) / 10;
    ones = (((playerScore % 1000) % 100)) % 10;
    send_i(LCDCLR);
    chgline(LINE1);
    pmsglcd("Score: ");
    print_c(thousands + 48);
    print_c(hundreds + 48);
    print_c(tens + 48);
    print_c(ones + 48);
    chgline(LINE2);
    pmsglcd("High Score: ");
    thousands = highScore / 1000;
    hundreds = (highScore % 1000) / 100;
    tens = ((highScore % 1000) % 100) / 10;
    ones = (((highScore % 1000) % 100)) % 10;
    print_c(thousands + 48);
    print_c(hundreds + 48);
    print_c(tens + 48);
    print_c(ones + 48);
}

/*
***********************************************************************                       
  LCD Printing routines		 		  		
***********************************************************************
*/

void shiftout(char output) {
    //Wait for register to be clear
    while(!SPISR_SPTEF) {}
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
    PTT_PTT4 = 0;
    PTT_PTT4 = 1;
    PTT_PTT4 = 0;
    lcdwait();
}

void send_i(char instruction) {
    PTT_PTT2 = 0;
    send_byte(instruction);
}

void chgline(char line) {
    send_i(CURMOV);
    send_i(line);
}

void print_c(char character) {
    PTT_PTT2 = 1;
    send_byte(character);
}

void pmsglcd(char str[]) {
    int i = 0;
    while(str[i] != '\0') {
        print_c(str[i]);
        i++;
    }
}

/*
***********************************************************************                       
  Terminal routines
***********************************************************************
*/
void printscreen() {
  int i = 0;
  int j = 0;
  for(i = 0; i < 8; i++) {
    for(j = 0; j < 4; j++ ) {
      temp = screen[4*i+j];
      outchar(temp);
      outchar(' ');
      outchar(' ');  
    }
    outchar('\n');
  }
}

void updatescreen(int bits) {
    int i = 0;
    for(i = 0; i < 28; i++) {
    screen[i+4] = screen[i];
    }
    if(bits & 0x80) {
    screen[0] = 'O';
    } else {
    screen[0] = ' ';
    }
    if(bits & 0x40) {
    screen[1] = 'O';
    } else {
    screen[1] = ' ';
    }
    if(bits & 0x20) {
    screen[2] = 'O';
    } else {
    screen[2] = ' ';
    }
    if(bits & 0x10) {
    screen[3] = 'O';
    } else {
    screen[3] = ' ';
    }
    clearScreen();
    printscreen();
}

void clearScreen() {
    int i;
    for(i = 0; i < 30; i++) {
        outchar('\n');
    }
}

void screen_test(void) {
    for(;;) {
        updateScreen(NOTE1);
        updateScreen(0);
        updateScreen(NODE2);
        updateScreen(0);
        updateScreen(NOTE3);
        updateScreen(0);
        updateScreen(NOTE4);
        updateScreen(0);
        updateScreen(NOTE3);
        updateScreen(0);
        updateScreen(NODE2);
        updateScreen(0);
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
