#include "I2C.h"
#include <stdio.h>
#include <rt_misc.h>
#include "LPC11xx.h"
/* Import external functions from Serial.c file      */


extern void SER_init (void);

#define BNO055_I2C_ADDRESS    0x29
#define BNO_055_chip_address  0x00
#define OPR_address 0x3D
#define IMU(accelerater,gyroscopemode)  0x1000b
#define pitchdata 0x1F
#define pitchdata2 0x1E
#define rolldata 0x1c
#define rolldata2 0x1d
#define Headingdata 0x1B
#define Headingdata2 0x1A
#define Linear_Acceleration_Data1 0x2D
#define Linear_Acceleration_Data2 0x2C

#define Accel_dataz1 0xD
#define Accel_dataz2 0xc

#define DS3502_I2C_ADDRESS		0x28
#define WR_ADDRESS 0x00
#define WIPER_LOW 0x40
#define WIPER_HIGH 0x7F

#define WAIT_TIME 0x20000
#define STARTUP_TIME 0x900000

#define PRESCALE_US (48-1) //Used by PWM
#define PRESCALE_MS (48000-1) //Used by 32Bit-TMR0 for generating delay 
#define PWM_PERIOD 41360
//the amount subtracted is the amount of time its high, and the amount left is the amount of time its low

#define Status_lower 41360-10340
#define Status_hover 41360-20680
#define Status_raise 41360-31020
#define turnbias 6000
void initTimer(){
  LPC_TMR32B1->CTCR=0x0;
  LPC_TMR32B1->PR = 0x15;//for 50 HZ clock and 64000 MR3
  LPC_TMR32B1->MR3 = PWM_PERIOD; //1ms Period duration
	LPC_TMR32B1->MR0 = PWM_PERIOD-40000; //the period is between 2200 and 4300 this is the top LED
  LPC_TMR32B1->MR1 = PWM_PERIOD-2100;  //bottom led
	LPC_TMR32B1->MCR = (1<<10); //Reset on MR3 Match
	LPC_TMR32B1->PWMC = 0x3; //Enable PWM Mode for MAT0.0 and MAT1.0
	LPC_TMR32B1->TCR = 0x2; //Reset Timer
	LPC_TMR32B1->TCR = 0x1;
 
  
  LPC_TMR16B0->CTCR=0x0;
  LPC_TMR16B0->PR = 0x15;//for 50 HZ clock and 64000 MR3
  LPC_TMR16B0->MR3 = PWM_PERIOD; //1ms Period duration
	LPC_TMR16B0->MR0 = PWM_PERIOD-40000; //the period is between 2200 and 4300
  LPC_TMR16B0->MR1 = PWM_PERIOD-2100;
	LPC_TMR16B0->MCR = (1<<10); //Reset on MR3 Match
	LPC_TMR16B0->PWMC = 0x3; //Enable PWM Mode for MAT0.0 and MAT1.0
	LPC_TMR16B0->TCR = 0x2; //Reset Timer
	LPC_TMR16B0->TCR = 0x1;
}

void turn_left(){
  
}  

void delayMS(unsigned int milliseconds){
  LPC_TMR32B0->TCR =0x02;
  LPC_TMR32B0->TCR=0x01;
  while(LPC_TMR32B0->TC<milliseconds);
  LPC_TMR32B0->TCR=0x00;
}

void configureGPIO()
{
	//enable clocks to GPIO block
	LPC_SYSCON->SYSAHBCLKCTRL |= (1UL <<  6);
  LPC_SYSCON->SYSAHBCLKCTRL |= (1UL <<  7);
  LPC_SYSCON->SYSAHBCLKCTRL |= (1UL <<  10);
	LPC_SYSCON->SYSAHBCLKCTRL |= (1UL <<  16);
	//set port 0_7 to output (high current drain in LPC1114)
	LPC_GPIO0->DIR |= (1<<7);
  LPC_GPIO0->DIR |= (1<<3);
}

int main() {
	// I2C package
	// Initialize I2C block
	I2C_init();
  configureGPIO();
  SER_init();
  initTimer();
  LPC_IOCON->PIO0_8 |= 0x2;
  LPC_IOCON->PIO0_9 |= 0x2;
  LPC_IOCON->R_PIO1_1 |= 0x3;
  LPC_IOCON->R_PIO1_2 |= 0x3;
  float backrightf = 0;
  float backleftf =0;
  
  for (int j = 0; j < STARTUP_TIME; j++) {}	

  uint8_t write_accelerometer_mode[3];
  uint8_t read_roll1[3];
  uint8_t read_roll2[3];
  uint8_t read_pitch1[3];
  uint8_t read_pitch2[3];
  uint8_t read_heading1[3];
  uint8_t read_heading2[3];
    
  uint8_t read_Zaccel1[3];
  uint8_t read_Zaccel2[3];

  write_accelerometer_mode[0]=OPR_address;
  write_accelerometer_mode[1]=0x8b; //fusion mode
  
    
  I2C_write(BNO055_I2C_ADDRESS,write_accelerometer_mode,2);
    
  uint16_t backleft;
  uint16_t backright;
  printf("start \r\n");
  
  int floatstatus=0;
  int turnstatus=0;
while(1){

  read_Zaccel1[0]=Linear_Acceleration_Data1;
  read_Zaccel2[0]=Linear_Acceleration_Data2;
  I2C_read(BNO055_I2C_ADDRESS,read_Zaccel1,2);
  I2C_read(BNO055_I2C_ADDRESS,read_Zaccel2,2);
  printf("z-acceleration is %02X%02X \r\n",read_Zaccel2[2],read_Zaccel2[1]);
  
  read_roll1[0]=rolldata;
  read_roll2[0]=rolldata2;
  I2C_read(BNO055_I2C_ADDRESS,read_roll1,2);
  I2C_read(BNO055_I2C_ADDRESS,read_roll2,2);
  printf("roll is %02X%02X \r\n",read_roll1[2],read_roll1[1]);
  printf("roll2 is %02X%02X \r\n",read_roll2[1],read_roll2[0]);
  
  read_pitch1[0]=pitchdata;
  read_pitch2[0]=pitchdata2;
  I2C_read(BNO055_I2C_ADDRESS,read_pitch1,2);
  I2C_read(BNO055_I2C_ADDRESS,read_pitch2,2);
  printf("pitch is %02X%02X \r\n",read_pitch2[2],read_pitch2[1]);
  
  read_heading1[0]=Headingdata;
  read_heading2[0]=Headingdata2;
  I2C_read(BNO055_I2C_ADDRESS,read_heading1,2);
  I2C_read(BNO055_I2C_ADDRESS,read_heading2,2);
  printf("heading is %02X%02X \r\n",read_heading2[2],read_heading2[1]);
  
  
  for (int j = 0; j <1000000; j++) {}	
     
  //left and right tilts
  backleft =0;
  backright = 0;
  if ( (read_pitch1[1]&1000)>0){
    //printf("%02X \r\n",(((read_pitch1[1]) << 8)+(read_pitch1[2])) );
    backright = 0xFFFF^  (((read_pitch1[1]) << 8)+(read_pitch1[2]));
    backleft=0;
    backrightf = backright/0xFF;
      
    //printf("backright is %02X \r\n",backright);
    LPC_TMR16B0->MR1 = PWM_PERIOD-2200-backrightf*2100;
    LPC_GPIO0->DATA |= (1<<7);//off
    LPC_GPIO0->DATA &= ~(1<<3);//on
    
  }
  else{
    backleft = ((signed)(read_pitch1[1]) << 8) | ((signed)read_pitch1[0]);
    backleftf = backleft/0xFF;
    backright=0;
    LPC_TMR16B0->MR0 = PWM_PERIOD-2200-backleftf*2100;
    LPC_GPIO0->DATA &= ~(1<<7);//on
    LPC_GPIO0->DATA |= (1<<3);//off
  }
  

  LPC_TMR32B1->MR0=PWM_PERIOD-40001;
  LPC_TMR32B1->MR1=PWM_PERIOD-40001;
      
      
//  read_Zaccel1[0]=Linear_Acceleration_Data1;
//  read_Zaccel2[0]=Linear_Acceleration_Data2;
//  I2C_read(BNO055_I2C_ADDRESS,read_Zaccel1,2);
//  printf("balls2");
//  uint8_t read_esp32[2];
//  read_esp32[0]=0;
//  read_esp32[1]=0;
//  printf("balls3");
//  I2C_read(4,read_esp32,2);
//  printf("balls4");
//  printf("esp is %02X%02X \r\n",read_esp32[0],read_esp32[1]);
  
  /*
  switch(floatstatus){
    case 0x00:
      LPC_TMR32B1->MR0=Status_lower;
      LPC_TMR32B1->MR1=Status_lower;
    break;
    case 0x01:
      LPC_TMR32B1->MR0=Status_hover;
      LPC_TMR32B1->MR1=Status_hover;
    break;
    case 0x02:
      LPC_TMR32B1->MR0=Status_raise;
      LPC_TMR32B1->MR1=Status_raise;
    break;
  } 
    switch(turnstatus){
    case 0x00:
    break;
    case 0x01:
      LPC_TMR32B1->MR0= LPC_TMR32B1->MR0-turnbias;
    break;
    case 0x02:
      LPC_TMR32B1->MR1= LPC_TMR32B1->MR0-turnbias;
    break;
  } 
  */
}//end while(1)





}


