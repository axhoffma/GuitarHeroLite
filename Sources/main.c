/*
************************************************************************
 ECE 362 - Mini-Project C Source File - Fall 2017
***********************************************************************
	 	   			 		  			 		  		
 Team ID: 17

 Project Name: ASCII Hero 

 Team Members:

   - Team/Doc Leader: Tim Duex      Signature: ______________________
   
   - Software Leader: Austin Hoffmann      Signature: Austin Hoffmann

   - Interface Leader: Antonio Mojena     Signature: _____________________

   - Peripheral Leader: John Reitz    Signature: ______________________


 Academic Honesty Statement:  In signing above, we hereby certify that we 
 are the individuals who created this HC(S)12 source file and that we have
 not copied the work of any other student (past or present) while completing 
 it. We understand that if we fail to honor this agreement, we will receive 
 a grade of ZERO and be subject to possible disciplinary action.

***********************************************************************

 The objective of this Mini-Project is to create a simple game based on
 the popular game "Guitar Hero". Notes will scroll at a continuous pace,
 and the user tries to time a pushbutton press to coincide with the music
 that is being played. Scores are tracked, with the high score being
 displayed to the user. 


***********************************************************************

 List of project-specific success criteria (functionality that will be
 demonstrated):

 1. PLay a simple song "Mario theme music in this case 

 2. Display the "game board" of notes that the player needs to hit, which
    updates at continuous intervals
 
 3. Display the player's score on the LED screen

 4. Display a welcome message to the user

 5. Have the option to "play again" after the song ends

***********************************************************************

  Date code started: 12/3/2017 

  Update history (add an entry every time a significant change is made):

  Date: 12/5/2017  Name: Austin Hoffmann  Update: First Complete prototype

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
void sound_test(void);
void welcome_screen(void);

/* Display functions */
void update_score(void);
void display_score(void);
void display_buttons(void);
void update_screen(int);

/* Sound functions */
void populate_song(void);


/* Test function initilization here */
//#define SCREEN_TEST
//#define SCORE_TEST 
//#define PUSH_TEST



/* Variable declarations */
//table below is the PWMDTY value that will generate a sine wave when cycled through
unsigned char sineArray [200] = {127,131,135,139,142,146,150,154,158,162,166,170,173,177,181,184,188,191,195,198,201,205,208,211,214,217,219,222,225,227,230,232,234,236,238,240,242,243,245,246,248,249,250,251,251,252,253,253,253,253,253,253,253,253,252,252,251,250,249,248,247,246,244,243,241,239,237,235,233,231,228,226,223,221,218,215,212,209,206,203,200,197,193,190,186,183,179,175,172,168,164,160,156,152,148,144,141,137,133,129,125,120,116,113,109,105,101,97,93,89,85,81,78,74,70,67,63,60,56,53,50,47,44,41,38,35,32,30,27,25,22,20,18,16,14,12,10,9,7,6,5,4,3,2,1,1,0,0,0,0,0,0,0,0,1,2,2,3,4,5,7,8,10,11,13,15,17,19,21,23,26,28,31,34,36,39,42,45,48,52,55,58,62,65,69,72,76,80,83,87,91,95,99,103,107,110,114,118,122,126};
unsigned char sineptr = 0; //ptr used to cycle through the sine values

int playerScore = 0;
int highScore = 0;

enum note{C3 = 459, C3s = 433, D3 = 409, D3s = 386, E3 = 364, F3 = 344, F3s = 324, G3 = 306,
          G3s = 289, A3 = 273, A3s = 258, B3 = 243, C4 = 229, C4s = 216, D4 = 204, D4s = 193,
          E4 = 182, F4 = 172, F4s = 162, G4 = 153, G4s = 144, A4 = 136, A4s = 129, B4 = 121,
          C5 = 115, C5s = 108, D5 = 102, D5s = 96, E5 = 91, F5 = 86, F5s = 81, G5 = 77, G5s = 77,
          A5 = 68};
char runstp = 1;
unsigned char input = 0;
unsigned char startFlg = 0;
unsigned char welcome = 0;

char screen[4] = {' '};
char temp = ' ';
int songbuttons[42] = {2,2,0,2, 0,4,2,0, 1,0,0,8, 0,0,5,0, 0,0,0,2, 0,0,4,0, 2,0,4,8, 0,2,1,2, 1,0,8,8, 0,4,0,2, 6,0};

/* Structure to represent a musical note */
typedef struct Note {
    int note; 
    int beats;
} Note;


Note lastNote; //duration of the last note
int rtiCnt = 0; //number of interrupts since last update
int displayCnt = 0;
int beatCount = 1;

/*Array of Notes that represents the song */
#define SONG_SIZE 125 
Note song[SONG_SIZE];
//Need to start at -1 so the first increment gets song[0]
int songPtr = 0;

/*Array of ints that represents the playboard */
int board[SONG_SIZE];
int boardPtr = 0;

/* Special ASCII characters */
#define CR 0x0D		// ASCII return�
#define LF 0x0A		// ASCII new line�

/* LCD COMMUNICATION BIT MASKS (note - different than previous labs) */
#define LCDRS  PTT_PTT5		// RS pin mask (PTT[2])
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

#define NOTE0 0x00
#define NOTE1 0x80
#define NOTE2 0x40
#define NOTE3 0x20
#define NOTE4 0x10

#define NOTE12 0xB0
#define NOTE13 0xA0
#define NOTE14 0x90

#define NOTE23 0x60
#define NOTE24 0x50

#define NOTE34 0x30


	 	   		
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
  MODRR = 0x06; //set PT2 & PT1 to output PWM signal
  PWME = 0x06;  //enable PWM Channel 1&2
  PWMPOL = 0x06; //set PWM Channel 1&2 to active high polarity
  PWMCTL = 0; //no concantenation 
  PWMCAE = 0; //no center align 
  PWMPER2 = 0xFF; //set max period for channel 2
  PWMDTY2 = 0; //set no duty cycle to start
  PWMPER1 = 0xFF; //set max period for channel 1
  PWMDTY1 = 0; //set no duty cycle to start
  PWMCLK = 0; //select clock B for channel 2 and clock A for channel 1
  PWMPRCLK = 0; //set clock to 24 MHz
    
/* Initialize TIM CH7 for periodic interrupts every ms */
  TSCR1 = 0x80; //Enable system
  TIOS = 0x80; //Output compare on channel 7
  TIE = 0x00; //No interrupts initially
  TCTL1 = 0x00; //Disconnected from output logic
  TSCR2 = 0x09; //Counter Resets on Channel 7. Clock scaler = 2 
  TC7 = 229; //interrupts set up to fire an interrupt rate of 52,400 Hz for middle C

/* Initialize LED screen */
  DDRT = 0xFF;
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
#ifdef SOUND_TEST
    sound_test();
#endif

    EnableInterrupts;
    TIE = 0x80;
    for(;;) {

    } /* loop forever */
   
}   /* do not leave main */




/*
***********************************************************************   ����  � ������   �� 
 RTI interrupt service routine: RTI_ISR
************************************************************************
*/

interrupt 7 void RTI_ISR(void)
{
    // clear RTI interrupt flagt 
    CRGFLG = CRGFLG | 0x80; 

    //Get the user input
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

    if(!startFlg) {
        if(!welcome) {
           welcome_screen();
            welcome = 1;
        }
        if(input) {
            startFlg = 1;
            update_score();
        }
        return;
    }

    rtiCnt++;
    displayCnt++;
    //Check if we need to update the note
    if(rtiCnt >= (lastNote.beats * (293 / 12))){ 

        rtiCnt = 0;
            
            
        lastNote = song[songPtr];
        if(lastNote.note != 0) {
          TC7 = lastNote.note;
          runstp = 1;
        } else {
            runstp = 0;
        }
        if(input & board[songPtr]) {
          update_score();
        }
          
        //Get the next note
        songPtr = (songPtr + 1) % SONG_SIZE;

        //Check if it is the end of the game
        if(songPtr == 0) {
            playerScore = 0;
            startFlg = 0;
            welcome = 0;
            boardPtr = 0;
            rtiCnt = 0;
            displayCnt = 0;
            lastNote.beats = 39;
            lastNote.note = 0;
            return;
        }

    }

    //Update screen every 1/8th note
    if(displayCnt >= (293 / 12) * beatCount) {
        beatCount++;

        if(boardPtr < SONG_SIZE) {
          if(boardPtr == 0 || displayCnt >= (293 / 12) * song[boardPtr - 1].beats) {
            beatCount = 1;
            displayCnt = 0;
            update_screen(board[boardPtr]);
            boardPtr++;
          } else {
            update_screen(0);
          }
        } else {
          displayCnt = 0;
          beatCount = 1;
          update_screen(0);
        }
    }
    
}

/*
***********************************************************************   ����  � ������   �� 
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
        PWMDTY2 = sineArray[sineptr];
        PWMDTY1 = sineArray[sineptr];
        sineptr = (sineptr + 1) % 200;
    }
    else{ 
      PWMDTY2 = 0;
      PWMDTY1 = 0;
    }
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
        update_score();
    }
    update_score();
    for(i = 1; i < 510; i++) {
        update_score();
    }
}
void update_score() {
    playerScore++;
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
    PTT_PTT5 = 0;
    send_byte(instruction);
}

void chgline(char line) {
    send_i(CURMOV);
    send_i(line);
}

void print_c(char character) {
    PTT_PTT5 = 1;
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
  int j = 0;
  int i = 0;
   for(j = 0; j < 4; j++ ) {
     temp = screen[j];
     if (temp == 'O') {
      for(i = 0; i < 3; i++) {
        outchar(' ');
      }
      for(i = 0; i < 13; i++) {
       outchar(temp); 
      }
      for(i = 0; i < 3; i++) {
       outchar(' '); 
      }
      outchar('|'); 
    } else {
      for(i = 0; i < 3; i++) {
        outchar(' ');
      }
      for(i = 0; i < 13; i++) {
       outchar(temp); 
      }
      for(i = 0; i < 3; i++) {
       outchar(' '); 
      }
      outchar('|');      
    }
  }
    outchar('\n');
    outchar(13);
}

void welcome_screen()
{
    //80 wide 40 tall
    int j;
    int i;
    char welcomeTo[] = "Welcome to ASCII Hero Lite! "; 
    char pressButton[] = "Press any button to start!";
    int length;
    for(j = 0; j<79; j++) //line 1
        outchar('*');
    outchar('\n');
    outchar(13);
    
    i = 0;
    for(j=0; j<19; j++)//line 2-19
    {
        outchar('*');
        for(i = 0; i <77; i++) {
          
            outchar(' ');
        }
        outchar('*');
        outchar('\n');
        outchar(13);
    }
    length = 28;
    //line 20
    outchar('*');
    for(j=0; j<=26; j++)
        outchar(' ');
        
    for(j=26; j<26+length; j++)
        outchar(welcomeTo[j-26]);

    for(j=26+length; j<76; j++)
        outchar(' ');
    outchar('*');
    outchar('\n');
    outchar(13);
    
    length = 27; 
    outchar('*');
    for(j=0; j<=26; j++)
        outchar(' ');

    for(j=26; j<26+length; j++)
        outchar(pressButton[j-26]);

    for(j=26+length; j<77; j++)
        outchar(' ');

    outchar('*');
    outchar('\n');
    outchar(13);

    for(j=22; j<39; j++)
    {
        outchar('*');
        for(i = 0; i <77; i++)
            outchar(' ');
        outchar('*');
        outchar('\n');
        outchar(13);
    }    
    
    for(j = 0; j<79; j++) //line 1
        outchar('*');
    outchar('\n');
    outchar(13);
}

void update_screen(int bits) {
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
    printscreen();
}

void screen_test(void) {
    for(;;) {
        update_screen(NOTE1);
        update_screen(0);
        update_screen(NOTE2);
        update_screen(0);
        update_screen(NOTE3);
        update_screen(0);
        update_screen(NOTE4);
        update_screen(0);
        update_screen(NOTE3);
        update_screen(0);
        update_screen(NOTE2);
        update_screen(0);
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


void populate_song() {
    //Bar 1
    song[0].note = E5;
    song[0].beats = 2;
    song[1].note = E5;
    song[1].beats = 2;
    song[2].note =  0;
    song[2].beats = 2;
    song[3].note = E5;
    song[3].beats = 2;
    //Bar 2 
    song[4].note = 0;
    song[4].beats = 2;
    song[5].note = C5;
    song[5].beats = 2;
    song[6].note =  E5;
    song[6].beats = 2;
    song[7].note = 0;
    song[7].beats = 2;
    //Bar 3 
    song[8].note = G5;
    song[8].beats = 2;
    song[9].note = 0;
    song[9].beats = 2;
    song[10].note = 0;
    song[10].beats = 4;
    //Bar 4 
    song[11].note = G4;
    song[11].beats = 2;
    song[12].note = 0;
    song[12].beats = 2;
    song[13].note = 0;
    song[13].beats = 4;
    //Bar 5 
    song[14].note = C5;
    song[14].beats = 2;
    song[15].note = 0;
    song[15].beats = 2;
    song[16].note = 0;
    song[16].beats = 2;
    song[17].note = G4;
    song[17].beats = 2;
    //Bar 6
    song[18].note = 0;
    song[18].beats = 4;
    song[19].note = E4;
    song[19].beats = 2;
    song[20].note = 0;
    song[20].beats = 2;
    //Bar 7
    song[21].note = 0;
    song[21].beats = 2;
    song[22].note = A4;
    song[22].beats = 2;
    song[23].note = 0;
    song[23].beats = 2;
    song[24].note = B4;
    song[24].beats = 2;
    //Bar 8 
    song[25].note = 0;
    song[25].beats = 2;
    song[26].note = A4s;
    song[26].beats = 2;
    song[27].note = A4;
    song[27].beats = 2;
    song[28].note = 0;
    song[28].beats = 2;
    //Bar 9  
    song[29].note = G4;
    song[29].beats = 4;
    song[30].note = E5;
    song[30].beats = 4;
    song[31].note = G5;
    song[31].beats = 4;
    //Bar 10  
    song[32].note = A5;
    song[32].beats = 2;
    song[33].note = 0;
    song[33].beats = 2;
    song[34].note = F5;
    song[34].beats = 2;
    song[35].note = G5;
    song[35].beats = 2;
    //Bar 11  
    song[36].note = 0;
    song[36].beats = 2;
    song[37].note = E5;
    song[37].beats = 2;
    song[38].note = 0;
    song[38].beats = 2;
    song[39].note = C5;
    song[39].beats = 2;
    //Bar 12  
    song[40].note = D5;
    song[40].beats = 2;
    song[41].note = B4;
    song[41].beats = 2;
    song[42].note = 0;
    song[42].beats = 4;
    //Bar 13  
    song[43].note = C5;
    song[43].beats = 2;
    song[44].note = 0;
    song[44].beats = 2;
    song[45].note = 0;
    song[45].beats = 2;
    song[46].note = G4;
    song[46].beats = 2;
    //Bar 14  
    song[47].note = 0;
    song[47].beats = 4;
    song[48].note = E4;
    song[48].beats = 2;
    song[49].note = 0;
    song[49].beats = 2;
    //Bar 15
    song[50].note = 0;
    song[50].beats = 2;
    song[51].note = A4;
    song[51].note = 2;
    song[52].note = 0;
    song[52].beats = 2;
    song[53].beats = B4;
    song[53].beats = 2;
    //Bar 16
    song[54].note = 0;
    song[54].beats = 2;
    song[55].note = A4s;
    song[55].note = 2;
    song[56].note = A4;
    song[56].beats = 2;
    song[57].beats = 0;
    song[57].beats = 2;
    //Bar 17
    song[58].note = song[29].note;
    song[58].beats = song[29].beats;
    song[59].note = song[30].note;
    song[59].beats = song[30].beats;
    song[60].note = song[31].note;
    song[60].beats = song[31].beats;
    //Bar 18
    song[61].note = song[32].note;
    song[61].beats = song[32].beats;
    song[62].note = song[33].note;
    song[62].beats = song[33].beats;
    song[63].note = song[34].note;
    song[63].beats = song[34].beats;
    song[64].note = song[35].note;
    song[64].beats = song[35].beats;
    //Bar 19
    song[65].note = song[36].note;
    song[65].beats = song[36].beats;
    song[66].note = song[37].note;
    song[66].beats = song[37].beats;
    song[67].note = song[38].note;
    song[67].beats = song[38].beats;
    song[68].note = song[39].note;
    song[68].beats = song[39].beats;
    //Bar 20 
    song[69].note = song[40].note;
    song[69].beats = song[40].beats;
    song[70].note = song[41].note;
    song[70].beats = song[41].beats;
    song[71].note = song[42].note;
    song[71].beats = song[42].beats;
    //Bar 21 
    song[72].note = 0;
    song[72].beats = 4;
    song[73].note = G5;
    song[73].beats = 2;
    song[74].note = F5s;
    song[74].beats = 2;
    //Bar 22 
    song[75].note = F5;
    song[75].beats = 2;
    song[76].note = D5s;
    song[76].beats = 2;
    song[77].note = 0;
    song[77].beats = 2;
    song[78].note = E5;
    song[78].beats = 2;
    //Bar 23 
    song[79].note = 0;
    song[79].beats = 2;
    song[80].note = G4s;
    song[80].beats = 2;
    song[81].note = A4;
    song[81].beats = 2;
    song[82].note = C5;
    song[82].beats = 2;
    //Bar 24 
    song[83].note = 0;
    song[83].beats = 2;
    song[84].note = A4;
    song[84].beats = 2;
    song[85].note = C5;
    song[85].beats = 2;
    song[86].note = D5;
    song[86].beats = 2;
    //Bar 25 
    song[87].note = song[72].note;
    song[87].beats = song[72].beats;
    song[88].note = song[73].note;
    song[88].beats = song[73].beats;
    song[89].note = song[74].note;
    song[89].beats = song[74].beats;
    //Bar 26 
    song[90].note = song[75].note;
    song[90].beats = song[75].beats;
    song[91].note = song[76].note;
    song[91].beats = song[76].beats;
    song[92].note = song[77].note;
    song[92].beats = song[77].beats;
    song[93].note = song[78].note;
    song[93].beats = song[78].beats;
    //Bar 27 
    song[94].note = 0;
    song[94].beats = 2;
    song[95].note = C6;
    song[95].beats = 2;
    song[96].note = 0;
    song[96].beats = 2;
    song[97].note = C6;
    song[97].beats = 2;
    //Bar 28 
    song[98].note = C6;
    song[98].beats = 2;
    song[99].note = 0;
    song[99].beats = 2;
    song[100].note = 0;
    song[100].beats = 4;
    //Bar 29 
    song[101].note = song[72].note;
    song[101].beats = song[72].beats;
    song[102].note = songs[73].note;
    song[102].beats = songs[73].beats;
    song[103].note = songs[74].note;
    song[103].beats = songs[74].beats;
    //Bar 30 
    song[104].note = song[75].note;
    song[104].beats = song[75].beats;
    song[105].note = songs[76].note;
    song[105].beats = songs[76].beats;
    song[106].note = songs[77].note;
    song[106].beats = songs[77].beats;
    song[107].note = songs[78].note;
    song[107].beats = songs[78].beats;
    //Bar 31 
    song[108].note = song[79].note;
    song[108].beats = song[79].beats;
    song[109].note = songs[80].note;
    song[109].beats = songs[80].beats;
    song[110].note = songs[81].note;
    song[110].beats = songs[81].beats;
    song[111].note = songs[82].note;
    song[111].beats = songs[82].beats;
    //Bar 31 
    song[112].note = song[83].note;
    song[112].beats = song[83].beats;
    song[113].note = songs[84].note;
    song[113].beats = songs[84].beats;
    song[114].note = songs[85].note;
    song[114].beats = songs[85].beats;
    song[115].note = songs[86].note;
    song[115].beats = songs[86].beats;
    //Bar 32 
    song[116].note = 0;
    song[116].beats = 4;
    song[117].note = D5s;
    song[117].beats = 2;
    song[118].note = 0;
    song[118].beats = 2;
    //Bar 33 
    song[119].note = 0;
    song[119].beats = 2;
    song[120].note = D5;
    song[120].beats = 2;
    song[121].note = 0;
    song[121].beats = 4;
    //Bar 34 
    song[122].note = C5;
    song[122].beats = 2;
    song[123].note = 0;
    song[123].beats = 2;
    song[124].note = 0;
    song[124].beats = 4;

    lastNote.note = 0;
    lastNote.beats = 39;
    runstp = 0;

    //Make the board
    board[0] = NOTE3;
    board[1] = NOTE3;
    board[2] = NOTE0;
    board[3] = NOTE3;

    board[4] = NOTE0;
    board[5] = NOTE2;
    board[6] = NOTE3;
    board[7] = NOTE0;

    board[8] = NOTE4;
    board[9] = NOTE0;
    board[10] = NOTE0;
    board[11] = NOTE1;

    board[12] = NOTE0;
    board[13] = NOTE0;
    board[14] = NOTE24;
    board[15] = NOTE0;

    board[16] = NOTE0;
    board[17] = NOTE0;
    board[18] = NOTE0;
    board[19] = NOTE3;

    board[20] = NOTE0;
    board[21] = NOTE0;
    board[22] = NOTE2;
    board[23] = NOTE0;

    board[24] = NOTE3;//2
    board[25] = NOTE0;//0
    board[26] = NOTE2;//4
    board[27] = NOTE1;//8

    board[28] = NOTE0;
    board[29] = NOTE3;
    board[30] = NOTE4;
    board[31] = NOTE3;

    board[32] = NOTE4;
    board[33] = NOTE0;
    board[34] = NOTE1;
    board[35] = NOTE1;

    board[36] = NOTE0;
    board[37] = NOTE2;
    board[38] = NOTE0;
    board[39] = NOTE3;

    board[40] = NOTE23;
    board[41] = NOTE0;
    board[42] = NOTE3;
    board[43] = NOTE3;
    board[44] = NOTE0;
    board[45] = NOTE3;

    board[46] = NOTE0;
    board[47] = NOTE2;
    board[48] = NOTE3;
    board[49] = NOTE0;

    board[50] = NOTE4;
    board[51] = NOTE0;
    board[52] = NOTE0;
    board[53] = NOTE1;

    board[53] = NOTE0;
    board[54] = NOTE0;
    board[55] = NOTE24;
    board[56] = NOTE0;

    board[57] = NOTE0;
    board[58] = NOTE0;
    board[59] = NOTE0;
    board[60] = NOTE3;

    board[61] = NOTE0;
    board[62] = NOTE0;
    board[63] = NOTE2;
    board[64] = NOTE0;

    board[65] = NOTE3;//2
    board[66] = NOTE0;//0
    board[67] = NOTE2;//4
    board[68] = NOTE1;//8

    board[69] = NOTE0;
    board[70] = NOTE3;
    board[71] = NOTE4;
    board[72] = NOTE3;

    board[73] = NOTE4;
    board[74] = NOTE0;
    board[75] = NOTE1;
    board[76] = NOTE1;

    board[77] = NOTE0;
    board[78] = NOTE2;
    board[79] = NOTE0;
    board[80] = NOTE3;

    board[81] = NOTE23;
    board[82] = NOTE0;

    board[83] = NOTE23;
    board[84] = NOTE0;
    board[85] = NOTE3;
    board[86] = NOTE3;
    board[87] = NOTE0;
    board[88] = NOTE3;

    board[89] = NOTE0;
    board[90] = NOTE2;
    board[91] = NOTE3;
    board[92] = NOTE0;

    board[93] = NOTE4;
    board[94] = NOTE0;
    board[95] = NOTE0;
    board[96] = NOTE1;

    board[97] = NOTE0;
    board[98] = NOTE0;
    board[99] = NOTE24;
    board[100] = NOTE0;

    board[101] = NOTE0;
    board[102] = NOTE0;
    board[103] = NOTE0;
    board[104] = NOTE3;

    board[105] = NOTE0;
    board[106] = NOTE0;
    board[107] = NOTE2;
    board[108] = NOTE0;

    board[109] = NOTE3;//2
    board[110] = NOTE0;//0
    board[111] = NOTE2;//4
    board[112] = NOTE1;//8

    board[113] = NOTE0;
    board[114] = NOTE3;
    board[115] = NOTE4;
    board[116] = NOTE3;

    board[117] = NOTE4;
    board[118] = NOTE0;
    board[119] = NOTE1;
    board[120] = NOTE1;

    board[121] = NOTE0;
    board[122] = NOTE2;
    board[123] = NOTE0;
    board[124] = NOTE3;
}
