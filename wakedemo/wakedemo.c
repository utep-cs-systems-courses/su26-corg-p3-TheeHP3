#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"
#include "buzzer.h"

/*------------------------------------------------*/
/*                 Hardware Defines                */
/*------------------------------------------------*/

#define LED BIT6

#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8

#define SWITCHES (SW1 | SW2 | SW3 | SW4)

/*------------------------------------------------*/
/*               Game Variables                   */
/*------------------------------------------------*/

int switches = 0;

char redrawScreen = 1;

char gameStarted = 0;

char currentQuestion = 0;

char gameOver = 0;
char gameWon = 0;

char lives = 3;

char waitingForRelease = 0;

/* Correct answer for each question
   (1=S1, 2=S2, 3=S3, 4=S4) */

char correctAnswers[] = {
  1,
  1,
  1
};

/*------------------------------------------------*/
/*                 Quiz Data                      */
/*------------------------------------------------*/

char *questions[] = {
  "Question 1",
  "Question 2",
  "Question 3"
};

char *questionInfo[] = {
  "The answer is 1",
  "The answer is 1",
  "The answer is 1"
};

char *answers[][4] = {

  {
    "S1: Answer 1",
    "S2: Answer 2",
    "S3: Answer 3",
    "S4: Answer 4"
  },

  {
    "S1: Answer 1",
    "S2: Answer 2",
    "S3: Answer 3",
    "S4: Answer 4"
  },

  {
    "S1: Answer 1",
    "S2: Answer 2",
    "S3: Answer 3",
    "S4: Answer 4"
  }

};

/*------------------------------------------------*/
/*            Switch Initialization              */
/*------------------------------------------------*/

static char switch_update_interrupt_sense()
{
  char p2val = P2IN;

  P2IES |= (p2val & SWITCHES);
  P2IES &= (p2val | ~SWITCHES);

  return p2val;
}

void switch_init()
{
  P2REN |= SWITCHES;
  P2IE  |= SWITCHES;
  P2OUT |= SWITCHES;
  P2DIR &= ~SWITCHES;

  switch_update_interrupt_sense();
}

/*------------------------------------------------*/
/*             Sound Helper Functions            */
/*------------------------------------------------*/

void playCorrectSound()
{
  buzzer_set_period(1200);

  __delay_cycles(200000);

  buzzer_set_period(0);
}

void playWrongSound()
{
  buzzer_set_period(4000);

  __delay_cycles(250000);

  buzzer_set_period(0);
}

void playGameOverSound()
{
  buzzer_set_period(5000);

  __delay_cycles(1000000);

  buzzer_set_period(0);
}

void playWinSound()
{
  buzzer_set_period(1500);
  __delay_cycles(200000);

  buzzer_set_period(0);
  __delay_cycles(50000);

  buzzer_set_period(1200);
  __delay_cycles(200000);

  buzzer_set_period(0);
  __delay_cycles(50000);

  buzzer_set_period(900);
  __delay_cycles(300000);

  buzzer_set_period(0);
}
/*------------------------------------------------*/
/*               LCD Drawing                     */
/*------------------------------------------------*/

void drawStartScreen()
{
  clearScreen(COLOR_BLUE);

  drawString5x7(24,
                20,
                "QUIZ GAME",
                COLOR_WHITE,
                COLOR_BLUE);

  drawString5x7(8,
                60,
                "Press Any Button",
                COLOR_WHITE,
                COLOR_BLUE);
}

void drawQuestionScreen()
{
  char lifeString[9];

  clearScreen(COLOR_BLACK);

  lifeString[0]='L';
  lifeString[1]='i';
  lifeString[2]='v';
  lifeString[3]='e';
  lifeString[4]='s';
  lifeString[5]=':';
  lifeString[6]=' ';
  lifeString[7]='0'+lives;
  lifeString[8]='\0';

  drawString5x7(2,
                2,
                lifeString,
                COLOR_GREEN,
                COLOR_BLACK);

  drawString5x7(2,
                22,
                questions[currentQuestion],
                COLOR_WHITE,
                COLOR_BLACK);

  drawString5x7(2,
                36,
                questionInfo[currentQuestion],
                COLOR_YELLOW,
                COLOR_BLACK);

  drawString5x7(2,
                64,
                answers[currentQuestion][0],
                COLOR_WHITE,
                COLOR_BLACK);

  drawString5x7(2,
                80,
                answers[currentQuestion][1],
                COLOR_WHITE,
                COLOR_BLACK);

  drawString5x7(2,
                96,
                answers[currentQuestion][2],
                COLOR_WHITE,
                COLOR_BLACK);

  drawString5x7(2,
                112,
                answers[currentQuestion][3],
                COLOR_WHITE,
                COLOR_BLACK);
}

void drawGameOverScreen()
{
  clearScreen(COLOR_BLACK);

  drawString5x7(25,
                60,
                "GAME OVER",
                COLOR_RED,
                COLOR_BLACK);

  drawString5x7(20,
                80,
                "Try Again!",
                COLOR_WHITE,
                COLOR_BLACK);
}

void drawWinScreen()
{
  clearScreen(COLOR_BLUE);

  drawString5x7(35,
                60,
                "YOU WIN",
                COLOR_YELLOW,
                COLOR_BLUE);

  drawString5x7(20,
                80,
                "Great Job!",
                COLOR_WHITE,
                COLOR_BLUE);
}

void redrawGame()
{
  if(gameOver)
    drawGameOverScreen();

  else if(gameWon)
    drawWinScreen();

  else if(gameStarted)
    drawQuestionScreen();

  else
    drawStartScreen();
}
/*------------------------------------------------*/
/*             Answer Processing                 */
/*------------------------------------------------*/

void processAnswer(char answer)
{
  if(waitingForRelease)
    return;

  if(gameOver || gameWon)
    return;

  waitingForRelease = 1;

  if(answer == correctAnswers[currentQuestion])
  {
    playCorrectSound();

    /*
      Check whether this was the final question.
    */
    if(currentQuestion == 2)
    {
      gameWon = 1;
      playWinSound();
    }
    else
    {
      currentQuestion++;
    }

    redrawScreen = 1;
  }
  else
  {
    playWrongSound();

    if(lives > 0)
      lives--;

    /*
      Player has lost all lives.
    */
    if(lives == 0)
    {
      gameOver = 1;
      playGameOverSound();
    }

    redrawScreen = 1;
  }
}

/*------------------------------------------------*/
/*          Switch Interrupt Handler             */
/*------------------------------------------------*/

void switch_interrupt_handler()
{
  char p2val = switch_update_interrupt_sense();

  switches = ~p2val & SWITCHES;

  /*
    No buttons are currently pressed.
    This releases the button lockout.
  */
  if(!switches)
  {
    waitingForRelease = 0;
    return;
  }

  /*
    Ignore button presses while waiting for
    the previous button to be released.
  */
  if(waitingForRelease)
    return;

  /*
    Do not accept answers after the game ends.
  */
  if(gameOver || gameWon)
    return;

  /*
    Start the game.
  */
  if(!gameStarted)
  {
    gameStarted = 1;
    redrawScreen = 1;
    waitingForRelease = 1;

    return;
  }

  if(switches & SW1)
    processAnswer(1);

  else if(switches & SW2)
    processAnswer(2);

  else if(switches & SW3)
    processAnswer(3);

  else if(switches & SW4)
    processAnswer(4);
} 

/*------------------------------------------------*/
/*      Part 2 begins immediately after here      */
/*------------------------------------------------*/

void wdt_c_handler()
{

}


/*------------------------------------------------*/
/*                     Main                       */
/*------------------------------------------------*/

void main()
{
  /*
    Initialize the green LED.
    P1.0 is reserved for the LCD, so we use P1.6.
  */
  P1DIR |= LED;
  P1OUT |= LED;

  configureClocks();

  lcd_init();

  buzzer_init();
  buzzer_set_period(0);

  switch_init();

  enableWDTInterrupts();

  or_sr(0x8);

  //Main game loop.
  while(1)
  {
    // Only redraw the LCD when something has changed.
    if(redrawScreen)
    {
      redrawScreen = 0;

      redrawGame();
    }

    /*
      Turn LED off before putting CPU to sleep.
    */
    P1OUT &= ~LED;

    /*
      Turn CPU off while keeping interrupts enabled.
      A button interrupt will wake the CPU.
    */
    or_sr(0x10);

    /*
      Turn LED back on after waking.
    */
    P1OUT |= LED;
  }
}


/*------------------------------------------------*/
/*             Port 2 Interrupt                  */
/*------------------------------------------------*/

void __interrupt_vec(PORT2_VECTOR) Port_2()
{
  //Check whether one of our four buttons caused the interrupt.
  if(P2IFG & SWITCHES)
  {
    //Clear the interrupt flags.
    P2IFG &= ~SWITCHES;

    //Process the button press.
    switch_interrupt_handler();
  }
}
