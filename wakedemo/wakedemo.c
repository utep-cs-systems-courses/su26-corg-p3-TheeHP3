#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"

/* --------------------------------------------------------- */
/*                  DO NOT TOUCH P1.0 (LCD)                  */
/* --------------------------------------------------------- */

#define LED BIT6

#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8

#define SWITCHES (SW1 | SW2 | SW3 | SW4)

/* --------------------------------------------------------- */
/*                  Game Variables                           */
/* --------------------------------------------------------- */

int switches = 0;

char redrawScreen = 1;

int gameStarted = 0;

int currentQuestion = 0;

int lives = 3;

/* --------------------------------------------------------- */
/*                  Question Data                            */
/* --------------------------------------------------------- */

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
    {"S1: Answer 1",
     "S2: Answer 2",
     "S3: Answer 3",
     "S4: Answer 4"},

    {"S1: Answer 1",
     "S2: Answer 2",
     "S3: Answer 3",
     "S4: Answer 4"},

    {"S1: Answer 1",
     "S2: Answer 2",
     "S3: Answer 3",
     "S4: Answer 4"}
};

/* --------------------------------------------------------- */
/*                  Switch Code                              */
/* --------------------------------------------------------- */

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
    P2IE |= SWITCHES;
    P2OUT |= SWITCHES;
    P2DIR &= ~SWITCHES;

    switch_update_interrupt_sense();
}

void switch_interrupt_handler()
{
    char p2val = switch_update_interrupt_sense();

    switches = ~p2val & SWITCHES;

    /* Start game on first button press */

    if (!gameStarted && switches)
    {
        gameStarted = 1;
        redrawScreen = 1;
    }
}

/* --------------------------------------------------------- */
/*                  LCD Drawing                              */
/* --------------------------------------------------------- */

void drawStartScreen()
{
    clearScreen(COLOR_BLUE);

    drawString5x7(20,20,
                  "QUIZ GAME",
                  COLOR_WHITE,
                  COLOR_BLUE);

    drawString5x7(8,50,
                  "Press Any Button",
                  COLOR_WHITE,
                  COLOR_BLUE);
}

void drawQuestion()
{
    char livesText[10];

    clearScreen(COLOR_BLACK);

    livesText[0] = 'L';
    livesText[1] = 'i';
    livesText[2] = 'v';
    livesText[3] = 'e';
    livesText[4] = 's';
    livesText[5] = ':';
    livesText[6] = ' ';
    livesText[7] = '0' + lives;
    livesText[8] = '\0';

    drawString5x7(5,
                  5,
                  livesText,
                  COLOR_GREEN,
                  COLOR_BLACK);

    drawString5x7(5,
                  25,
                  questions[currentQuestion],
                  COLOR_WHITE,
                  COLOR_BLACK);

    drawString5x7(5,
                  40,
                  questionInfo[currentQuestion],
                  COLOR_YELLOW,
                  COLOR_BLACK);

    drawString5x7(5,
                  70,
                  answers[currentQuestion][0],
                  COLOR_WHITE,
                  COLOR_BLACK);

    drawString5x7(5,
                  85,
                  answers[currentQuestion][1],
                  COLOR_WHITE,
                  COLOR_BLACK);

    drawString5x7(5,
                  100,
                  answers[currentQuestion][2],
                  COLOR_WHITE,
                  COLOR_BLACK);

    drawString5x7(5,
                  115,
                  answers[currentQuestion][3],
                  COLOR_WHITE,
                  COLOR_BLACK);
}

void redrawGame()
{
    if (!gameStarted)
        drawStartScreen();
    else
        drawQuestion();
}

/* --------------------------------------------------------- */
/*          Watchdog Timer Interrupt Handler                 */
/* --------------------------------------------------------- */

void wdt_c_handler()
{
    /* Nothing timed yet.
       We simply keep the framework your professor provided. */
}

/* --------------------------------------------------------- */
/*                      Main                                 */
/* --------------------------------------------------------- */

void main()
{
    P1DIR |= LED;
    P1OUT |= LED;

    configureClocks();

    lcd_init();

    switch_init();

    enableWDTInterrupts();

    or_sr(0x8);

    while (1)
    {
        if (redrawScreen)
        {
            redrawScreen = 0;
            redrawGame();
        }

        P1OUT &= ~LED;

        or_sr(0x10);

        P1OUT |= LED;
    }
}

/* --------------------------------------------------------- */
/*             Port 2 Interrupt                              */
/* --------------------------------------------------------- */

void __interrupt_vec(PORT2_VECTOR) Port_2()
{
    if (P2IFG & SWITCHES)
    {
        P2IFG &= ~SWITCHES;

        switch_interrupt_handler();
    }
}
