#include <msp430.h>
#include "nrf24.h"
#include "pwm.h"

int waiter = 0;

void SetupClock()
{
  //setting up the internal clock
  DCOCTL = 0;                               // Select lowest DCOx and MODx settings
  // BCSCTL1 = CALBC1_1MHZ;               // Set DCO
  //DCOCTL = CALDCO_1MHZ;
  BCSCTL1 = 0x0f;
  DCOCTL = 0x60;
  
}

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;            // Stop watchdog
  if (CALBC1_1MHZ==0xFF)   // If calibration constants erased
    {
      while(1);                          // do not load, trap CPU!!
    }

  SetupClock();
  
  //pullups and io configs
  P1OUT = 0x03; 
  P1REN = 0x02;
  P2REN |= 0xFF;
  P1DIR = 0xFD;                        // Unused pins as outputs
  P2DIR = 0xFF;
  P2OUT = 0x40;

  initUSI();
  initPWM();

  _EINT();

  //write to config
  unsigned char buffer[36] = {0x20, 0x0f};
  transfer(buffer, 2);

  buffer[0] = 0x00;
  transfer(buffer, 2);
  if (buffer[1] != 0x0f)
    {
      return 0;
    }
  //flush RX
  buffer[0] = 0xe2;
  transfer (buffer, 1);

  //clear the RX  bit
  buffer[0] = 0x27;
  buffer[1] =  buffer[0] | 0x40; 
  transfer(buffer,2);
  
  //set payload size 32 bytes
  buffer[0] = 0x31;
  buffer[1] = 32;
  transfer(buffer, 2);

  //read payload size
  buffer[0] = 0x11;
  transfer(buffer, 2);
  if (buffer[1] != 0x20)
    {
      return 0;
    }
  P2OUT |= 0x80; //recieve mode
  //enablePWM();
  while (1)
    {
      _BIS_SR(LPM0_bits + GIE); 
    }

  P2OUT &= ~0x80;
}
//******************************************************************************
// USI interrupt service routine
//******************************************************************************
#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void)
{
  P1IFG &= ~IRQPIN;
  //We have data waiting
  disablePWM();
  
  unsigned char buffer[33] = {0};
  buffer[0] = 0x61; //read RX payload
  transfer (buffer, 33);

  R = buffer[1];
  G = buffer[2];
  B = buffer[3];

  //clear the RX  bit
  buffer[0] = 0x27;
  buffer[1] =  buffer[0] | 0x40; 
  transfer(buffer,2);
  
  //flush RX
  buffer[0] = 0xe2;
  transfer (buffer, 1);

  enablePWM();

}
