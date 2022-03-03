/************************************************************************************
* main function in pixyTestClient1.c
* - to display single line to indicate what angle it is now based on posCount
*
* Author: Gwang Sik Kim
* Date: Feb 26, 2022
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/

#include <msp430.h> 
#include <stdio.h> // sprintf is included.
// *** also needs to configure 'full' support of printf in properties.
#include <stdlib.h> // malloc and free
#include "usciI2C.h"
#include "pixyCam2.h"
#include "ucsiUart.h"
#include "usciCmdInterpreter.h"

#include "nok5110LCD.h"
#include "usciSpi.h"
#include "nokLcdDraw.h"


// constants
#define RX_BUFF_SIZE            20
#define MAX_DISP_MSG_CLIENT     50
#define MAX_UART_RX_STRING      50

// test macro
#define TEST1                   0
#define TEST2                   1
// timer count
#define     CYCLE_COUNT         17000 // 17ms
#define     DELAY_1SEC  1000000


// prototype
void initTA0(int cycle);

// global variable - inherited from LS7366RTestClient... fedi Client.
long signed int* posCount;
long signed int posCountPrev;
unsigned char bufferCNTR[4] = {0, 0, 0, 0};
unsigned char bufferDTR[4] = {0, 0, 0, 0};
long signed int posHome;
char dispMode = 0;
char testMode = 1;
long signed int posCountTest;
long int posCntFrHm;
unsigned char vecDispCont = 0;


int main(void){

	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	

    // ======================== UART config =================================
    ucsiA1UartInit();
    // ======================== =========== =================================

    // ======================== SPI config =================================
    // SPI initialize...
    //const unsigned char spiMst = MASTER_MODE; // master
    //const unsigned int spiClkDiv = CLOCK_DIV_1;
    //const unsigned int spiClkDiv = CLOCK_DIV_2;
    //const unsigned int spiClkDiv = CLOCK_DIV_4;
    //const unsigned int spiClkDiv = SPI_CLOCK_DIV_16;
    //unsigned char const spiLoopBack = LOOPBACK_DISABLE;
    //unsigned char const spiLoopBack = LOOPBACK_ENABLE;
    //const unsigned char sclkMode = DATA_CAPTURE_RISING | INACTIVE_LOW;  // UCCKPH = 1, capture > change, UCCKPL = 0, inactive low.
    usciB1SpiInit(MASTER_MODE, SPI_CLOCK_DIV_16, DATA_CAPTURE_RISING | INACTIVE_LOW, LOOPBACK_DISABLE);

    // SPI configured as...
    // 19200 Baud Rate, DIV = 16, MASTER, CAPTURE > CHANGE, Inactive Low...
    // Loopback Disabled.
    // P4.3 CLK, P4.1 MOSI
    // ======================== =========== =================================

    // ======================== I2C initialize  ============================
    usciB0I2CInit(I2C_CLOCK_DIV_16);
    //UCB0IE |= UCNACKIE; // NACK interrupt enabled
    // ======================== =========== =================================

    // ======================== Timer config =================================

    //unsigned int cycle = CYCLE_COUNT; // 1us -> 50000 count -> 50ms.
    initTA0(CYCLE_COUNT);
    __enable_interrupt();
    // ======================== ============ =================================

    // ======================== UART command setup  ============================
    CMD cmdList[MAX_CMDS];
    initCmdList(cmdList);


    char* errorFlag;
    int index = -2;
    int errIdx = -2;
    // ======================== =========== =================================

    // ======================== pixy initialize  ============================
    pixyCmdInit();
    pixyResponseInit();
    // ======================== =========== =================================

    // ======================== nokLCD initialize  ============================
    nokLcdInit();
    nokLcdClear();

    // start
    nokLcdLoading(); // BCIT

    __delay_cycles(DELAY_1SEC);
    nokLcdClear();
    // ======================== =========== =================================


    if (TEST1){
        while(1){
            //pixyCmdInit(cmds);
            //pixyResponseInit(payload);
            //pixyGetVersion(cmds, payload);
            //pixyGetResolution(cmds, payload);

        }
    }

    else if (TEST2){
        while(1){
            char* dispMsg = (char*)malloc(sizeof(char) * MAX_DISP_MSG_CLIENT);
            char* uartRxString = (char*)malloc(sizeof(char) * MAX_UART_RX_STRING);
            sprintf(dispMsg, "Waiting for the user...\r\n");
            ucsiA1UartTxString((unsigned char*)dispMsg);
            //rewind(stdin);
            errorFlag = usciA1UartGets(uartRxString); // this receives the command that user types


            if (errorFlag[0] == ASCII_NULL){            // checking if it's NULL
                if (vecDispCont != 0){
                    vecDispCont = 0;
                    sprintf(dispMsg, "pixyVec continuous mode stopped.\r\n");
                    ucsiA1UartTxString((unsigned char*)dispMsg);
                }
                else{
                    sprintf(dispMsg, "NULL cannot be accepted.\r\n");
                    ucsiA1UartTxString((unsigned char*)dispMsg);
                }

            }
            else{                                       // checking if it's valid command
                sprintf(dispMsg, "User Enters a command... \r\n");
                TX_DISPMSG;

                // checking the index from the command list
                index = parseCmd(cmdList, uartRxString);
                sprintf(dispMsg, "The command index is %d.\r\n", index);
                TX_DISPMSG;

                // execute command
                errIdx = executeCmd(cmdList, index);
                if(errIdx == -1){ // failure case
                    sprintf(dispMsg, "The command arguments were not acceptable.\r\n");
                    TX_DISPMSG;
                }
                else{// success case
                    sprintf(dispMsg, "Success to call command.\r\n");
                    TX_DISPMSG;
                }
            }

            free(dispMsg);
            free(uartRxString);
        }
    }


	return 0;
}


/*************************************************************************************
 * ISR Name         : timer0A1ISR
 * Description      : interrupted by the flag set by comparing CCR0
 *                    the timer TA0 Main Timer and channel 1 to 5 use this ISR.
 *
 * Created Date     : Taken care of by created data of the main function.
 * Last Modified    : Usually taken care of by revision control.
 *
 **************************************************************************************/
#pragma vector = TIMER0_A1_VECTOR
__interrupt void timer0A1ISR(void){
    static int count = 0;

    switch(count){
    case 0:
        break;

    case 1:
        break;

    case 2:
        break;

    case 3:
        break;

    case 4:
        break;

    case 14:// 15 * COUNT(17ms) = 255ms
        if(vecDispCont == 1){ // every 17ms it updates.
            //pixyGetVector(vecDispCont);
            pixyVecContLcdDisp();
        }
        if(vecDispCont == 2){
            pixyVecContLcdSingleLine();
        }
        break;
    }
    count++;
    if (count == 15){
        count = 0;
    }
    TA0CTL &= ~TAIFG;
}



/*************************************************************************************
 * Function Name    : initTA0
 * Description      : To initialize timer A0 as Upmode
 *
 * Arguments        : (int, var) cycle is for M cycle to compare with CCR0
 * Return           : (void) N/A
 *
 * Created Date     : Taken care of by created data of the main function.
 * Last Modified    : Usually taken care of by revision control.
 *
 **************************************************************************************/
void initTA0(int cycle){
    TA0CTL = TASSEL_2 | ID_0 | MC_1 | TAIE; // Timer A0 Control Register
    // TA Select_2 means SMCLK = 1.08 MHz
    // ID_0 means divisor 1 ( = SMCLK)
    // MC_1 Up Mode
    // TAIE is interrupt Enable...

    TA0CCR0 = cycle - 1;
    // capture/control register 0 = M - 1, M is M cycles where the timer rolls over up to the number.

    TA0CTL &= ~TAIFG; // clear flag
}
