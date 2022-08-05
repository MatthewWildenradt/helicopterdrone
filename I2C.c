// References:
// LPC111x User Manual
// LPC11xx.h
// DS3502 Datasheet

// Important User Manual content for basic Master Transmitter setup:
// 15.1-15.7
// 15.8.1
// 15.10.1
// 15.11.[1-2,4-6]
// For clearing and setting I2C Configuration Bits: 15.7.1, 15.7.6

#include "I2C.h"

// Synchronization Construct
static volatile I2CTask_t i2c_task;	

void I2C_init(void) {
	// TODO: Enable IOCON Clock (15.2.2)
	LPC_SYSCON->SYSAHBCLKCTRL |= AHBCLKCTRL_IOCON;
	// TODO: Enable I2C Clock (15.2.2)
	LPC_SYSCON->SYSAHBCLKCTRL |= AHBCLKCTRL_I2C;
	// TODO: Configure I2C Pins (15.2.1) 
	// Use Standard/Fast Mode I2C (See 15.6 for which pins)
  //LPC_IOCON->PIO0_4 &= ~I2CMODE_STANDARD_FAST;
	//LPC_IOCON->PIO0_5 &= ~I2CMODE_STANDARD_FAST;
  LPC_IOCON->PIO0_4 |= 0x1;
	LPC_IOCON->PIO0_5 |= 0x1;
	// TODO: Reset I2C Block (15.2.3)
	LPC_SYSCON->PRESETCTRL |= I2C_RST_N;
	// TODO: Set I2C Clock Rate to fastest speed supported by DS3502
	// See DS3502 I2C AC Electrical Characteristics for desired speed
	// and LPC1114 User Manual 15.7.5.1 for how to set it (Assume I2C_PCLK is at 40Mhz)
  LPC_I2C->SCLL = 70;
  LPC_I2C->SCLH = 30;
	// TODO: Configure I2C in Master Transmitter Mode (15.10.1)
	LPC_I2C->CONSET = I2EN; 
  LPC_I2C->CONCLR = STA | STO | SI | AA;  
	// TODO: Enable I2C Interrupt (28.6.2.1, 6.4 or LPC11xx.h))
  NVIC_EnableIRQ(I2C_IRQn);
	// I2C ready
	i2c_task.is_busy = 0;
}

void I2C_write(uint32_t address, uint8_t *buffer, uint32_t count) {
	
	// Wait until I2C is free (NOT busy)
	while(i2c_task.is_busy);

	// Initialize task object for writing
	i2c_task.address = address;
	i2c_task.action = I2C_WRITE;
	i2c_task.buffer = buffer;
	i2c_task.byte_count = count;
	i2c_task.current_byte = 0;
	i2c_task.is_busy = 1;		// Set busy flag
	
	// TODO: Transmit START condition (15.10.1)
	LPC_I2C->CONSET = STA; 
	// Wait on busy flag (until all bytes in the buffer are sent)
	while(i2c_task.is_busy);
	
}

void I2C_read(uint32_t address, uint8_t *buffer, uint32_t count) {
	
	// Wait until I2C is free (NOT busy)
	while(i2c_task.is_busy);

	// Initialize task object for writing
	i2c_task.address = address;
	i2c_task.action = I2C_READ;
	i2c_task.buffer = buffer;
	i2c_task.byte_count = count;
	i2c_task.current_byte = 0;
	i2c_task.is_busy = 1;		// Set busy flag
	
	// TODO: Transmit START condition (15.10.1)
	LPC_I2C->CONSET = STA; 
	// Wait on busy flag (until all bytes in the buffer are sent)
	while(i2c_task.is_busy);
	
}


void I2C_IRQHandler(void) {
	
	// See Table 236 for complete state machine guide
	switch(LPC_I2C->STAT) {
		case 0x08:				// Master Transmitter/Receiver Mode: START condition successful
			
		  // TODO: Load I2C slave address and data direction (i2c.action) into i2c data register (15.7.3, 15.8.1)
			LPC_I2C->DAT = i2c_task.address<<1;
      LPC_I2C->DAT |= I2C_WRITE;
		  // TODO: Clear SI and STA bits
      LPC_I2C->CONCLR = STA;
      LPC_I2C->CONCLR = SI;
			break;
    
    
    case 0x10:

      LPC_I2C->DAT = i2c_task.address<<1;
      LPC_I2C->DAT |= I2C_READ;
		  // TODO: Clear SI and STA bits
      LPC_I2C->CONCLR = STA;
      LPC_I2C->CONCLR = STO|SI;
      break;
    
		case 0x18:				// Master Transmitter Mode: Slave ADDR + WRITE sent, and ACK received. 
			// Send first byte from buffer    
    LPC_I2C->DAT = (uint32_t) i2c_task.buffer[i2c_task.current_byte];
    if (i2c_task.action == I2C_WRITE) i2c_task.current_byte++;
		  // TODO: Clear the SI bit
			LPC_I2C->CONCLR = SI;
			break;
      
		case 0x28:				// Master Transmitter Mode: DATA transmitted
		
    if ((i2c_task.current_byte < i2c_task.byte_count) && i2c_task.action != I2C_READ) { // If we have more data to transmit
				// Load next available byte from buffer
				LPC_I2C->DAT = (uint32_t) i2c_task.buffer[i2c_task.current_byte++];
			} 
			else { // Otherwise we are done with the I2C transaction
				// Reset busy flag (continue)
          
        if (i2c_task.action==I2C_WRITE){
          i2c_task.is_busy = 0;	
          
          // TODO: Transmit STOP condition
          LPC_I2C->CONSET = STO; 
        }
        else{
          LPC_I2C->CONSET = STA; 
        }
			} 
			
			// TODO: Clear the SI bit
			LPC_I2C->CONCLR = SI;
			break;
      
    case 0x40:
      if (i2c_task.byte_count==++i2c_task.current_byte){LPC_I2C->CONCLR =AA;}
         else{LPC_I2C->CONSET =AA;}
          LPC_I2C->CONCLR = STA | STO | SI;
       
    break;
        
    case 0x50:
      if (i2c_task.current_byte< i2c_task.byte_count){
          i2c_task.buffer[i2c_task.current_byte++]=LPC_I2C->DAT;
          LPC_I2C->CONCLR = STA | STO | AA | SI;
        }
        else{
          i2c_task.buffer[i2c_task.current_byte++]=LPC_I2C->DAT;
          LPC_I2C->CONSET = AA; 
          LPC_I2C->CONCLR = STA | STO | SI;
        }
        break;
    case 0x58:
          i2c_task.buffer[i2c_task.current_byte++]=LPC_I2C->DAT;   
				// TODO: Transmit STOP condition
      LPC_I2C->CONSET = STO; 
    	LPC_I2C->CONCLR = SI;
    i2c_task.is_busy = 0;	  
    break;
        
	}
  
    
      
  
  
  
  
  
  
  
  
  
  
  
}
