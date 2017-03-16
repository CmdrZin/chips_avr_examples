#include <avr/io.h>


/*****************************************************************************/
#define TWI_CMD_PAGEUPDATE        	0x01	// TWI Command to program a flash page
#define TWI_CMD_EXECUTEAPP        	0x02	// TWI Command to jump to the application program
#define TWI_CMD_AVERSION          	0x03	// TWI Command to read the application revision identifier
#define TWI_CMD_BVERSION          	0x04	// TWI Command to get the bootloader revision identifier
#define TWI_CMD_ERASEFLASH        	0x05	// TWI Command to erase the entire application section of flash memory
#define TWI_CMD_GETERRCONDN             0x07    // TWI Command to get Error condition

/*****************************************************************************/


#define BOOT_SETTLE_DELAY         	100	// Debounce delay for the boot pin in MCU cycles
#define SLAVE_ADDRESS             	0xb0	// The address identifier of this slave device on the TWI (I2C) bus
#define INTVECT_PAGE_ADDRESS      	0x000	// The location of the start of the interrupt vector table address



// Select the communcation interfacing type: TWI or USI
#if defined(__AVR_ATtiny25__)   | defined(__AVR_ATtiny25V__) | \
    defined(__AVR_ATtiny45__)   | defined(__AVR_ATtiny45V__) | \
    defined(__AVR_ATtiny85__)   | defined(__AVR_ATtiny85V__) | \
    defined(__AVR_ATtiny24__)   | defined(__AVR_ATtiny24A__) | \
    defined(__AVR_ATtiny44__)   | defined(__AVR_ATtiny44A__) | \
    defined(__AVR_ATtiny84__)   | defined(__AVR_ATtiny84A__) | \
    defined(__AVR_ATtiny2313__) | defined(__AVR_ATtiny2313A__) | \
    defined(__AVR_ATtiny4313__) | defined(__AVR_ATtiny4313A__) | \
    defined(__AVR_ATtiny261__)  | defined(__AVR_ATtiny261A__) | \
    defined(__AVR_ATtiny461__)  | defined(__AVR_ATtiny461A__) | \
    defined(__AVR_ATtiny861__)  | defined(__AVR_ATtiny861A__) | \
    defined(__AVR_ATtiny43U__)  | \
    defined(__AVR_ATtiny87__)   | \
    defined(__AVR_ATtiny167__)
    #define __USI__  // set the communication type as USI
#endif 


#if defined(__AVR_ATtiny48__) | defined(__AVR_ATtiny88__) | \
    defined(__AVR_ATmega48A__) | defined(__AVR_ATmega48PA__) | \
    defined(__AVR_ATmega48__)
    #define __TWI__
#endif      

/***************************************************************/


// Page size selection for the controller with 2K flash
#if defined(__AVR_ATtiny25__)   | defined(__AVR_ATtiny25V__) | \
    defined(__AVR_ATtiny24__)   | defined(__AVR_ATtiny24A__) | \
    defined(__AVR_ATtiny2313__) | defined(__AVR_ATtiny2313A__) | \
    defined(__AVR_ATtiny261__)  | defined(__AVR_ATtiny261A__)

    // The flash memory page size for these devices
    #define PAGE_SIZE 32 

    // Page 32, the start of bootloader section
    #define BOOT_PAGE_ADDRESS 0X0400  

    // 2KB of flash divided by pages of size 32 bytes
    #define TOTAL_NO_OF_PAGES  64    
    
    // The number of pages being used for bootloader code
    #define BOOTLOADER_PAGES          	(TOTAL_NO_OF_PAGES - BOOT_PAGE_ADDRESS/PAGE_SIZE)	
 
    // For bounds check during page write/erase operation to protect the bootloader code from being corrupted
    #define LAST_PAGE_NO_TO_BE_ERASED 	(TOTAL_NO_OF_PAGES - BOOTLOADER_PAGES)	
#endif

// Page size selection for the controller with 4K flash

#if defined(__AVR_ATtiny45__)   | defined(__AVR_ATtiny45V__) | \
    defined(__AVR_ATtiny44__)   | defined(__AVR_ATtiny44A__) | \
    defined(__AVR_ATtiny4313__) | defined(__AVR_ATtiny4313A__) | \
    defined(__AVR_ATtiny461__)  | defined(__AVR_ATtiny461A__) | \
    defined(__AVR_ATtiny43U__)  | \
    defined(__AVR_ATtiny48__)   | \
    defined(__AVR_ATmega48__) | \
    defined(__AVR_ATmega48A__) | defined(__AVR_ATmega48PA__)
          
    // The flash memory page size for these devices
    #define PAGE_SIZE 64      
    
    // Page 48, the start of bootloader section
    #define BOOT_PAGE_ADDRESS 0X0C00   
    
    // 4KB of flash divided by pages of size 64 bytes
    #define TOTAL_NO_OF_PAGES  64    

    // The number of pages being used for bootloader code
    #define BOOTLOADER_PAGES          	(TOTAL_NO_OF_PAGES - BOOT_PAGE_ADDRESS/PAGE_SIZE)	

  // For bounds check during page write/erase operation to protect the bootloader code from being corrupted
   #define LAST_PAGE_NO_TO_BE_ERASED 	(TOTAL_NO_OF_PAGES - BOOTLOADER_PAGES)	
#endif


// Page size selection for the controller with 8K flash
#if defined(__AVR_ATtiny85__)   | defined(__AVR_ATtiny85V__) | \
    defined(__AVR_ATtiny84__)   | defined(__AVR_ATtiny84A__) | \
    defined(__AVR_ATtiny861__)  | defined(__AVR_ATtiny861A__) | \
    defined(__AVR_ATtiny87__)   | \
    defined(__AVR_ATtiny88__)
      
    // The flash memory page size for these devices
    #define PAGE_SIZE 64      

    // Page 112, the start of bootloader section
//    #define BOOT_PAGE_ADDRESS 0X1C00   
    #define BOOT_PAGE_ADDRESS 0X1800		// need 2k 01-24-2017 ndp
  
    // 8KB of flash divided by pages of size 64 bytes
    #define TOTAL_NO_OF_PAGES  128    
    
    // The number of pages being used for bootloader code
    #define BOOTLOADER_PAGES          	(TOTAL_NO_OF_PAGES - BOOT_PAGE_ADDRESS/PAGE_SIZE)	

    // For bounds check during page write/erase operation to protect the bootloader code from being corrupted
    #define LAST_PAGE_NO_TO_BE_ERASED 	(TOTAL_NO_OF_PAGES - BOOTLOADER_PAGES)	
#endif

// Page size selection for the controller with 16K flash   
#if defined(__AVR_ATtiny167__) 

    // The flash memory page size for Atiny167
    #define PAGE_SIZE 128     

    // Page 120, the start of bootloader section
    #define BOOT_PAGE_ADDRESS 0X3C00  

    // 16KB of flash divided by pages of size 128 bytes
    #define TOTAL_NO_OF_PAGES  128   

    // The number of pages being used for bootloader code
    #define BOOTLOADER_PAGES          	(TOTAL_NO_OF_PAGES - BOOT_PAGE_ADDRESS/PAGE_SIZE)	

    // For bounds check during page write/erase operation to protect the bootloader code from being corrupted
    #define LAST_PAGE_NO_TO_BE_ERASED 	(TOTAL_NO_OF_PAGES - BOOTLOADER_PAGES)	

#endif   


 /*****************************************************************************/
 /*****************************************************************************/
// Select the correct Bit name of to SELFPROGRAMming
#if defined(__AVR_ATtiny48__) | defined(__AVR_ATtiny48A__) | \
    defined(__AVR_ATtiny88__) | defined(__AVR_ATtiny88A__) | \
    defined(__AVR_ATmega48__) | \
    defined(__AVR_ATmega48A__) | defined(__AVR_ATmega48PA__)
      
    #define SELFPROGEN     SELFPRGEN

#endif
 /*****************************************************************************/
 /*****************************************************************************/
#if defined(__AVR_ATtiny25__)   | defined(__AVR_ATtiny25V__) | \
    defined(__AVR_ATtiny45__)   | defined(__AVR_ATtiny45V__) | \
    defined(__AVR_ATtiny85__)   | defined(__AVR_ATtiny85V__) | \
    defined(__AVR_ATtiny87__)   | defined(__AVR_ATtiny167__)
    
     #define SELFPROGEN SPMEN

#endif
//__no_init uint8_t pageBuffer[PAGE_SIZE];

 /*****************************************************************************/
 /*****************************************************************************/


#define WDT_TIMEOUT_8s      		( _BV( WDP3 ) | _BV( WDP0 ) )	        // Watchdog timeout for inactivity in the boot section
#define WDT_TIMEOUT_16ms    		0	// Watchdog timeout for system reset (cleanup) before jump to application
#define WDT_TIMEOUT_min     		WDT_TIMEOUT_16ms	                // The minimum watchdog reset time interval for jump to application


/*****************************************************************************/
/*****************************************************************************/

#define STATUSMASK_SPMBUSY 		0x01	// The mask bit for SPM busy status code
#define STATUSMASK_BLSCERR              0x02    // The mask bit for attempt to override bootloader section
#define STATUSMASK_TWIABORT             0x04    // The mask bit for indicating TWI abort fn called
#define STATUSMASK_SLTR_BUSY            0x08    // The mask bit for slave transmit
#define STATUSMASK_SLRBAA_BUSY          0x10    // The mask bit for slave receive and ack
#define STATUSMASK_SLRBAN_BUSY          0x20    // The mask bit for slave receive and Nack
#define STATUSMASK_EEPROM_BUSY          0x40    // The mask bit for EEPROM busy
#define STATUSMASK_BOOTLOADER           0x80    // The mask bit for bootloader operations
/*****************************************************************************/
/*****************************************************************************/

#define EEMEM_ADDR_AVERSION 		0x05	// The address in EEPROM where application revision identifier will be stored
#define BVERSION 			0x96	// This bootloader revision identifier


void UpdatePage (uint16_t);  

//#ifndef _BV
//#define _BV( __BIT_POSITION__ ) ( 1 << __BIT_POSITION__ )
//#endif

//#ifdef _BV
//#warning _BV now stands defined
//#endif
