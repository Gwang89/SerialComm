/*
 * usciI2C.h
 *
 *  Created on: 2022. 2. 23.
 *      Author: GwangSik
 */

#ifndef USCII2C_H_
#define USCII2C_H_


// macros and constants
#define     I2C_SDA_B1                  BIT0            // P3.0 SDA
#define     I2C_SCL_B1                  BIT1            // P3.1 SCL
#define     I2C_CLOCK_DIV_1             1
#define     I2C_CLOCK_DIV_2             2
#define     I2C_CLOCK_DIV_4             4
#define     I2C_CLOCK_DIV_16            16
#define     I2C_CLOCK_DIV_32            32
#define     I2C_CLOCK_DIV_64            64
#define     I2C_CLOCK_DIV_128           128




#define POLLING_TXIFG           while (!(UCB0IFG&UCTXIFG))
#define POLLING_RXIFG           while (!(UCB0IFG&UCRXIFG))

#define CHECK_NACK_SET          (UCB0IFG & UCNACKIFG)
#define NACK_RESET              UCB0IFG &= ~UCNACKIFG
#define TXIFG_RESET             UCB0IFG &= ~UCTXIFG
#define RXIFG_RESET             UCB0IFG &= ~UCRXIFG
#define ISSUE_STOP_BIT          UCB0CTL1 |= UCTXSTP
#define ISSUE_START_BIT         UCB0CTL1 |= UCTXSTT
#define WAIT_FOR_TXIFG          while (!(UCB0IFG&UCTXIFG)); UCB0IFG &= ~UCTXIFG
#define WAIT_FOR_RXIFG          while (!(UCB0IFG&UCRXIFG)); UCB0IFG &= ~UCRXIFG

#define STOP_IF_NACK            if (CHECK_NACK_SET){NACK_RESET;ISSUE_STOP_BIT;return success = -1;}

// prototype
void usciB0I2CInit(unsigned int sclkDiv);
int usciB0I2CMstTransmit(unsigned char* txBuffer, char length);
int usciB0I2CMstReceive(unsigned char* rxBuffer, char length);
void usciB0I2CClkDiv(unsigned int sclkDiv);

extern const unsigned char slaveAddress;

#endif /* USCII2C_H_ */
