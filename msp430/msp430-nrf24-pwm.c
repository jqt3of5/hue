#include <msp430.h>

int waiter = 0;
void transfer(unsigned char * buffer, int count);
int g_i = 0;
int g_size = 0;
unsigned char *g_buffer;

int isFirst = 1;

int r,g,b;
unsigned int R=0,G=0,B=0;
const int RED=0x01, GREEN=0x08, BLUE=0x10;

void enablePWM()
{
  CCTL0 |= CCIE;
}

void disablePWM()
{
  CCTL0 &= ~CCIE;
}

void SetupClock()
{
  //setting up the internal clock
  DCOCTL = 0;                               // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_1MHZ;               // Set DCO
  DCOCTL = CALDCO_1MHZ;
}

void SetupPWMTimers()
{
 //Setting up the PWM
  CCR0 = 100; //100 clock ticks to interrupt
  TACTL = TASSEL_2 + MC_1; 
  r = g = b = 0;
  disablePWM();
}


int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;            // Stop watchdog
  if (CALBC1_1MHZ==0xFF)   // If calibration constants erased
    {
      while(1);                          // do not load, trap CPU!!
    }

  SetupClock();
  SetupPWMTimers();
  //pullups and io configs
  P1OUT = 0x03; 
  P1REN = 0x02;
  P2REN |= 0xFF;
  P1DIR = 0xFD;                        // Unused pins as outputs
  P2DIR = 0xFF;
  P2OUT = 0x40;

  //pin interrupt
  P1IE |= BIT1;
  P1IES |= BIT1;
  P1IFG &= ~BIT1;

  //USI Setup
  USICTL0 |= USIPE7 + USIPE6 + USIPE5 + USIMST + USIOE;
  USICTL1 |= USIIE + USICKPH;
  USICKCTL = USIDIV_2 + USISSEL_2;
  USICTL0 &= ~USISWRST;
  USICNT = 0;
  USISRL = 0x20;

  _EINT();

  //This msp430g2231 has a known defect to shift an extra bit for the first byte. 
  //do an initial flush
  USISRL  = 0x0;
  USICNT = 7;
  while (isFirst == 1);

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
void transfer(unsigned char * buffer, int count)
{
  g_buffer = buffer;
  g_size = count;
  g_i = 0;

  //  P2OUT |= 0x40; // cycle the CS pin
  //  for (waiter = 0; waiter < 100; ++waiter);
  P2OUT &= ~0x40;

  USISRL = g_buffer[0];
  USICNT |= 8;
    
  while (g_i != g_size)
    {
      _BIS_SR(LPM0_bits + GIE); 
    }

  P2OUT |= 0x40; // cycle the CS pin
}
#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void)
{
  P1IFG &= ~BIT1;
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
#pragma vector = USI_VECTOR
__interrupt void USI_TXRX (void)
{
  if (isFirst == 1 || g_buffer == 0)
    {
      isFirst = 0;
      USICTL1 &= ~USIIFG;                  // Clear pending flags
      return;
    }

  g_buffer[g_i] = USISRL;
  g_i++;
  if (g_i < g_size)
    {
      USISRL = g_buffer[g_i];
      USICNT |= 8;
    }
  else
    {
      LPM0_EXIT;
    }
   USICTL1 &= ~USIIFG;                  // Clear pending flags
}


#pragma vector = TIMERA0_VECTOR
__interrupt void TimerPWM(void)
{
  //RED PWM channel
  r += R;
  if (r >=256)
    {
      r -= 256;
      P1OUT |= RED;
    } 
  else 
    {
      P1OUT &= ~RED;
    }

  //GREEN PWM channel
  g += G;
  if (g >=256)
    {
      g -= 256;
      P1OUT |= GREEN;
    } 
  else 
    {
      P1OUT &= ~GREEN;
    }

  //BLUE PWM channel
  b += B;
  if (b >=256)
    {
      b -= 256;
      P1OUT |= BLUE;
    } 
  else 
    {
      P1OUT &= ~BLUE;
    }
}
