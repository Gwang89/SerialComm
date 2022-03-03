/*
 * usciI2C.c
 *
 *  Created on: 2022. 2. 23.
 *      Author: GwangSik
 */

#include <msp430.h>
#include "usciI2C.h"


//global variable
unsigned char nackMode = 0;
/*
 *
 * ***** psuedo code section *****
 *
 * this initialization is very similar to SPI system, so it needs to initialize
 * what mode, what frequency and what pins are used, mst/slv selection, etc.
 * UCWRST needs to be set to initialize it.
 * (4.1.3) usciB1I2CInit ( sclkDiv){
 *     1. P4SEL |= P4.1, P4.2 (select peripheral, not I/o)
 *
 *     2. UCWRST = 1 (modify)
 *       (1) CTL0
 *          - UCA10 reset (own address 7 bit)
 *          - UCSLA10 reset (slave 7 bit addr)
 *          - UCMM reset (single master)
 *          - UCMST set (master mode)
 *          - UCMODE = 11b (i2C mode)
 *          - UCSYNC = set (synchronous)
 *       (2) CTL1
 *          - UCSSEL - 11b or 10b (SMCLK)
 *          - UCTR - 1 transmitter as default, but everytime we send/receive it, we need to assure it.
 *
 *       (3) sclkDiv -> CLK divisor calculation
 *          --> BR0, BR1 assigned.
 *     3. UCWRST = 0 (modify ended)
 *
 * }
 *
 *
 *  we need to set UCTR is transmitter to make it sure...
 *  input argument should be the pointer to the txBuffer, and n length of the bytes, and the slave address to send.
 *  firstly, configure the slave address and transmitter, and then polling the txIFG and send the data.
 *  if NACK is issued from slave, or n-bytes of data are completely transmitted,
 *  STOP should be issued. When NACK is issued, NACK mode can be set by ISR and it stops the transmission.
 *
 * (4.1.4) char usciB1I2CMstTransmit(char* txBuffer, int length, char slaveAddr){
 *
 *    0. slaveAddr -> i2CSA1 assigned .
 *
 *    1. CTL1, UCTR = 1 (transmitter)
        UCTXSTT = 1 // start bit
 *
 *    2. for loop or while loop. transmitting a byte n times
 *    // polling TXIFG
 *    while (!(UCB1IFG & UCTXIFG) && !NACK_FLAG_SET); // polling... detecting '1', flag set.
 *
 *    if ( n != length)
 *          UCB1TXBUF = txBuffer[n];  // transmitting txBuffer
 *    else // last byte?
 *          UCB1TXBUF = txBuffer[n];
 *          UCTXSTP issued...
 *
 *     n++ until n == length
 *
 *
 *    3. if NACK_FLAG_SET
 *      success = -1.
 *
 *
 *    4. return success; // return value. 0 if successful. -1 if failed
 *  }
 *
 *  in ISR?
 *  switch(NACKIFG_IS_SET)
 *      NACK_FLAG_SET
 *
 *
 *
 *
 *
 * receive function is similar to transmit function, except it's RXIFG polling,
 * and rxbuffer = RXBUF...
 * (4.1.5) char usciB1I2CMstReceive(char* rxBuffer, int length, char slaveAddr){
 *
 *  *    0. slaveAddr -> i2CSA1 assigned .
 *
 *    1. CTL1, UCTR = 0 (receiver)
        UCTXSTT = 1 // start bit
 *
 *    2. for loop or while loop. receiving  a byte n times
 *    // polling RXIFG
 *    while (!(UCB1IFG & UCRXIFG) && !NACK_FLAG_SET); // polling... detecting '1', flag set.
 *
 *    if ( n != length)
 *          rxBuffer[n] = UCB1RXBUF;  // receiving rxBuffer
 *    else // last byte?
 *          rxBuffer[n] = UCB1RXBUF;
 *          UCTXSTP issued...
 *
 *     n++ until n == length
 *
 *
 *    3. if NACK_FLAG_SET
 *      success = -1.
 *
 *
 *    4. return success; // return value. 0 if successful. -1 if failed
 *  }
 *
 *  in ISR?
 *  switch(NACKIFG_IS_SET)
 *      NACK_FLAG_SET
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */




/************************************************************************************
* Function: ucsiB1I2CInit
* - configures B1 I2C controlling the programmers model using the arguments passed by the caller.

* Arguments: (unsigned int)  sclkDiv - clock signal divisor.
* return: none
* Author: Gwang Sik Kim
* Date: Feb 23, 2022
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/
void usciB0I2CInit(unsigned int sclkDiv){


    P3SEL |= I2C_SDA_B1 | I2C_SCL_B1; // port select as peripheral

    // ***** able to modify only when UCSWRST = 1 *****
    UCB0CTL1 |= UCSWRST;                      // **Put state machine in USCI reset while you intitialize it**

    // CTL0
    UCB0CTL0 &= ~UCA10; // own address is 7 bit
    UCB0CTL0 &= ~UCSLA10; // slave address is 7 bit
    UCB0CTL0 &= ~UCMM; // single master
    UCB0CTL0 |= UCMST; // master mode
    UCB0CTL0 |= UCMODE_3; // I2C mode
    UCB0CTL0 |= UCSYNC; // synchronous mode

    // CTL1
    UCB0CTL1 |= UCSSEL_3;  // SMCLK
    UCB0CTL1 |= UCTR; // default transmitter
    UCB0CTL1 &= ~UCSWRST;                       // **Initialize USCI state machine**

    // divisor config
    usciB0I2CClkDiv(sclkDiv);// call usciB1SpiClkDiv



    //UCB1CTL1 |= UCSWRST;
    //UCB1CTL1 &= ~UCSWRST;
    // **Initialize USCI state machine**  take it out of reset
}




/************************************************************************************
* Function: usciB1I2CClkDiv
* - to configure the bit clock pre-scaler, UCB1BR1 and UCB1BR0 as 2 bytes.
*   BR1 is high byte and BR0 is low byte.
*   f_bitClk = f_BRCLK / UCBR

* Arguments: (unsigned int)  sclkDiv - divisor of the clock frequency
* return: none
* Author: Gwang Sik Kim
* Date: Feb 23, 2022
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/
// this function is complete. Understand what it is doing.  Call it when SCLKDIV needs to be changed in Lab.
void usciB0I2CClkDiv(unsigned int sclkDiv){

    UCB0CTL1 |= UCSWRST;                        // you always need to put state machine into reset when configuring USC module

    UCB0BR0 = (sclkDiv&0xFF);                   // 2
    UCB0BR1 = (sclkDiv>>8);                     //

    UCB0CTL1 &= ~UCSWRST;                       // **Initialize USCI state machine**
}







/************************************************************************************
* Function: usciB1I2CMstTransmit
* - to transmit the number of byte on i2c bus
* - configure the slave's address, issue start/stop bit and check NACK.
* Arguments: (unsigned char*) txBuffer - n-bytes of byte array to be transmitted
* return: 0 if nBytes are transmitted, -1 if not.
* Author: Gwang Sik Kim
* Date: Feb 23, 2022
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/
int usciB0I2CMstTransmit(unsigned char* txBuffer, char length) {
    int success = -1;          // 0 if successful. -1 if failed.
    UCB0I2CSA = slaveAddress;      // slaveAddr is assigned to the corresponding register
    UCB0CTL1 |= UCTR; // transmitter

    ISSUE_START_BIT; // start bit

    char n=0;                   // for loop or while loop. transmitting a byte n times

    // nack? slave is not ready
    STOP_IF_NACK

    while(n < length){
        POLLING_TXIFG{
            STOP_IF_NACK
        }
        UCB0TXBUF = txBuffer[n];
        n++;
    }
    POLLING_TXIFG;
    ISSUE_STOP_BIT; // stop bit issue makes TXIFG = 0... ?
    if(!(CHECK_NACK_SET)){
        success = 0;
    }

    return success;
}




/************************************************************************************
* Function: usciB1I2CMstReceive
* - to receive the number of byte on i2c bus
* - configure the slave's address, issue start/stop bit and check NACK.
* Arguments: (unsigned char*) rxBuffer - n-bytes of byte array to be received
* return: 0 if nBytes are received, -1 if not.
* Author: Gwang Sik Kim
* Date: Feb 23, 2022
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/
int usciB0I2CMstReceive(unsigned char* rxBuffer, char length) {
    int success = -1;          // 0 if successful. -1 if failed.
    UCB0I2CSA = slaveAddress;      // slaveAddr is assigned to the corresponding register
    UCB0CTL1 &= ~UCTR; // receiver

    ISSUE_START_BIT; // start bit

    char n=0;                   // for loop or while loop. transmitting a byte n times

    // nack? slave is not ready
    STOP_IF_NACK

    while(n < length){
        //POLLING_RXIFG{
        //    STOP_IF_NACK
        //}
        POLLING_RXIFG;
        if ((UCB0RXBUF != 175) && (n == 0)){
            // sometimes RXBUF == 0x80, but the scope says it's not.
            // why is it 0x80? this if statement can filter it, but would it be okay?
        }
        else{
            rxBuffer[n] = UCB0RXBUF;
            n++;
        }

    }
    POLLING_RXIFG;
    ISSUE_STOP_BIT; // stop bit issue makes TXIFG = 0... ?

    // resetRXIFG
    RXIFG_RESET;

    if(!(CHECK_NACK_SET)){
        success = 0;
    }

    return success;
}







//isr
#pragma vector=USCI_B0_VECTOR
__interrupt void usciB0I2CIsr(void) {
    //static unsigned char k = 0;

// UCB1IV interrupt handler. __even_in_range will optimize the C code so efficient jumps are implemented.
  switch(__even_in_range(UCB0IV,4)){
      case 0: break;                            // Vector 0 - no interrupt
      case 2:                                   // Vector 2 - Arbitration
          break;

      case 4:
          nackMode = 1;                          // Vector 4 - NACK
          break;

      default: break;
  }
}
