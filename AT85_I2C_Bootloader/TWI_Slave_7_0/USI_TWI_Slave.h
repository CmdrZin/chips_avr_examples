/*
 * Modified: 1/23/2017 ndp	Fix ATtiny85 definitions for 6.2 iotnx5.h defines.
 */

#ifdef __USI__

 /****************************************************************************/
  void USI_TWI_SLAVE_Init(uint8_t);
  void USI_TWI_SLAVE_Abort(void);
  void USI_TWI_SLAVE_ReadAndProcessPacket(void);
  void USI_TWI_SLAVE_Process_Start_Condition(void);
  void USI_TWI_SLAVE_Process_Overflow_Condition(void);
 
  void prog_buffer_set_byte(uint8_t );
 
   uint8_t bootloader_exit;
   uint8_t bootloader_active;
  
 
 #define TRUE                1
 #define FALSE               0

 // TWI write states.
#define USI_TWI_SLAVE_WRITE_ADDR_HI_BYTE              (0x00)
#define USI_TWI_SLAVE_WRITE_ADDR_LO_BYTE              (0x01)
#define USI_TWI_SLAVE_WRITE_DATA_BYTE                 (0x02)
  
// TWI overflow states.
#define USI_TWI_SLAVE_OVERFLOW_STATE_NONE             (0x00)
#define USI_TWI_SLAVE_OVERFLOW_STATE_ACK_PR_RX        (0x01)
#define USI_TWI_SLAVE_OVERFLOW_STATE_DATA_RX          (0x02)
#define USI_TWI_SLAVE_OVERFLOW_STATE_ACK_PR_TX        (0x03)
#define USI_TWI_SLAVE_OVERFLOW_STATE_PR_ACK_TX        (0x04)
#define USI_TWI_SLAVE_OVERFLOW_STATE_DATA_TX          (0x05)
  
// newly added .. 
#define USI_TWI_SLAVE_OVERFLOW_STATE_CMD_RX                 (0x06) 
#define USI_TWI_SLAVE_OVERFLOW_STATE_DATA_TX_AVERSION       (0x07) 
#define USI_TWI_SLAVE_OVERFLOW_STATE_DATA_TX_BVERSION       (0x08) 
#define USI_TWI_SLAVE_OVERFLOW_STATE_DATA_TX_GETERRCONDN    (0x09) 
  

  
//////////////////////////////////////////////////////////////////////
 //  typedef     unsigned char       uint8_t;

//////////////////////////////////////////////////////////////////
///////////////// Driver Buffer Definitions //////////////////////
//////////////////////////////////////////////////////////////////
// 1,2,4,8,16,32,64,128 or 256 bytes are allowed buffer sizes

#define TWI_RX_BUFFER_SIZE  (16)
#define TWI_RX_BUFFER_MASK ( TWI_RX_BUFFER_SIZE - 1 )

#if ( TWI_RX_BUFFER_SIZE & TWI_RX_BUFFER_MASK )
        #error TWI RX buffer size is not a power of 2
#endif

// 1,2,4,8,16,32,64,128 or 256 bytes are allowed buffer sizes

#define TWI_TX_BUFFER_SIZE  (16)
#define TWI_TX_BUFFER_MASK ( TWI_TX_BUFFER_SIZE - 1 )

#if ( TWI_TX_BUFFER_SIZE & TWI_TX_BUFFER_MASK )
        #error TWI TX buffer size is not a power of 2
#endif



#define USI_SLAVE_CHECK_ADDRESS                (0x00)
#define USI_SLAVE_SEND_DATA                    (0x01)
#define USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA (0x02)
#define USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA   (0x03)
#define USI_SLAVE_REQUEST_DATA                 (0x04)
#define USI_SLAVE_GET_DATA_AND_SEND_ACK        (0x05)
 
 
 //////////////////////////////////////////////////////////////////////////

 
#if defined(__AVR_ATtiny2313__) | defined(__AVR_ATtiny2313A__) | \
    defined(__AVR_ATtiny4313__) | defined(__AVR_ATtiny4313A__)

    #define DDR_USI             DDRB
    #define PORT_USI            PORTB
    #define PIN_USI             PINB
    #define PORT_USI_SDA        PORTB5
    #define PORT_USI_SCL        PORTB7
    #define PIN_USI_SDA         PINB5
    #define PIN_USI_SCL         PINB7
    #define USI_START_COND_INT  USISIF
    #define USI_START_VECTOR    USI_STRT_vect
    #define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif

// Fix definitions for 6.2 iotnx5.h defines..1-23-2017 ndp.
#if defined(__AVR_ATtiny25__) | defined(__AVR_ATtiny25V__) | \
    defined(__AVR_ATtiny45__) | defined(__AVR_ATtiny45V__) | \
    defined(__AVR_ATtiny85__) | defined(__AVR_ATtiny85V__) 
    #define DDR_USI             DDRB
    #define PORT_USI            PORTB
    #define PIN_USI             PINB
    #define PORT_USI_SDA        PB0				// PORTB0
    #define PORT_USI_SCL        PB2				// PORTB2
    #define PIN_USI_SDA         PINB0
    #define PIN_USI_SCL         PINB2
    #define USI_START_COND_INT  USISIF			// USICIF
    #define USI_START_VECTOR    USI_START_vect
    #define USI_OVERFLOW_VECTOR USI_OVF_vect 
#endif

#if defined(__AVR_ATtiny24__) | defined(__AVR_ATtiny24A__) | \
    defined(__AVR_ATtiny44__) | defined(__AVR_ATtiny44A__) | \
    defined(__AVR_ATtiny84__) | defined(__AVR_ATtiny84V__) 

    #define DDR_USI             DDRA
    #define PORT_USI            PORTA
    #define PIN_USI             PINA
    #define PORT_USI_SDA        PORTA6
    #define PORT_USI_SCL        PORTA4
    #define PIN_USI_SDA         PINA6
    #define PIN_USI_SCL         PINA4
    #define USI_START_COND_INT  USISIF
    #define USI_START_VECTOR    USI_START_vect
    #define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif
 

#if defined(__AVR_ATtiny261__) | defined(__AVR_ATtiny261A__) | \
    defined(__AVR_ATtiny461__) | defined(__AVR_ATtiny461A__) | \
     defined(__AVR_ATtiny861A__) | defined(__AVR_ATtiny861A__) 

     #define DDR_USI             DDRB
     #define PORT_USI            PORTB
     #define PIN_USI             PINB
     #define PORT_USI_SDA        PORTB0
     #define PORT_USI_SCL        PORTB2
     #define PIN_USI_SDA         PINB0
     #define PIN_USI_SCL         PINB2
     #define USI_START_COND_INT  USISIF
     #define USI_START_VECTOR    USI_START_vect
     #define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif


#if defined(__AVR_ATtiny43U__) 
     #define DDR_USI             DDRB
     #define PORT_USI            PORTB
     #define PIN_USI             PINB
     #define PORT_USI_SDA        PORTB4
     #define PORT_USI_SCL        PORTB6
     #define PIN_USI_SDA         PINB4
     #define PIN_USI_SCL         PINB6
     #define USI_START_COND_INT  USISIF
     #define USI_START_VECTOR    USI_START_vect
     #define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif


#if defined(__AVR_ATtiny87__) | defined(__AVR_ATtiny167__) 

    #define DDR_USI             DDRA
    #define PORT_USI            PORTA
    #define PIN_USI             PINA
    #define PORT_USI_SDA        PORTA4
    #define PORT_USI_SCL        PORTA5
    #define PIN_USI_SDA         PINA4
    #define PIN_USI_SCL         PINA5
    #define USI_START_COND_INT  USISIF
    #define USI_START_VECTOR    USI_START_vect
    #define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif





#endif
 