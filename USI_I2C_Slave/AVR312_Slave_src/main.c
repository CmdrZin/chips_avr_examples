// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* File              : USI_TWI_Slave.c
* Compiler          : IAR EWAAVR 3.20c
* Revision          : $Revision: 1.14 $
* Date              : $Date: Friday, December 09, 2005 17:25:38 UTC $
* Updated by        : $Author: jtyssoe $
*
* Support mail      : avr@atmel.com
*
* Supported devices : All device with USI module can be used.
*
* AppNote           : AVR312 - Using the USI module as a I2C slave
*
* Description       : Example showing how to use the USI_TWI drivers;
*                     Loops back received data.
*
*
****************************************************************************/

#include <ioavr.h>
#include <inavr.h>
#include "USI_TWI_Slave.h"

// Sample TWI transmission commands
#define TWI_CMD_MASTER_WRITE 0x10
#define TWI_CMD_MASTER_READ  0x20
#define TWI_CMD_MASTER_WRITE_ALOT 0x30

/* \Brief The main function.
 * The program entry point. Initates TWI and enters eternal loop, waiting for data.
 */
__C_task void main( void )
{
  unsigned char TWI_slaveAddress, temp;

  // LED feedback port - connect port B to the STK500 LEDS
  DDRB  = 0xFF; // Set to ouput
  PORTB = 0x55; // Startup pattern

  DDRA = 0xFF;  // L2 Used to time interrupts.

  // Own TWI slave address
  TWI_slaveAddress = 0x10;
  USI_TWI_Slave_Initialise( TWI_slaveAddress );

  __enable_interrupt();
  // This loop runs forever. If the TWI Transceiver is busy the execution will just continue doing other operations.
  for(;;)
  {

    if( USI_TWI_Data_In_Receive_Buffer() )
    {
        temp = USI_TWI_Receive_Byte();
        PORTB = temp;
        USI_TWI_Transmit_Byte(temp);
    }

    // Do something else while waiting for the TWI transceiver to complete.
    __no_operation(); // Put own code here.
  }
}

/*! \mainpage
 * \section Intro Introduction
 * This documents data structures, functions, variables, defines, enums, and
 * typedefs in the software for application note AVR312.
 *
 * \section CI Compilation Info
 * This software was written for the IAR Embedded Workbench 4.11A.
 *
 * To make project:
 * <ol>
 * <li> Add the file main.c and USI_TWI_Slave to project.
 * <li> Under processor configuration, select any AVR device with USI module.
 * <li> Enable bit definitions in I/O include files
 * <li> High optimization on size is recommended for lower code size.
 * </ol>
 *
 * \section DI Device Info
 * The included source code is written for all devices with with a USI module.
 *
 * \section TS Target setup
 * This example is made to work together with the AVR310 USI TWI Master application note. In adition to connecting the TWI
 * pins, also connect PORTB to the LEDS. The code reads a message as a TWI slave and acts according to if it is a
 * general call, or an address call. If it is an address call, then the first byte is considered a command byte and
 * it then responds differently according to the commands.
 *
 */

