#include "nrf24.h"
#include <msp430.h>

int g_i = 0;
int g_size = 0;
unsigned char *g_buffer;

int isFirst = 1;

void initUSI()
{
  P1DIR |= (CEPIN | CSPIN);
  P1DIR &= ~IRQPIN;
  P1REN &= ~(CSPIN | CEPIN);
  P1REN |= IRQPIN;
  P1OUT |= CSPIN | IRQPIN;
  P1OUT &= ~CEPIN;

  //pin interrupt
  P1IE |= IRQPIN;
  P1IES |= IRQPIN;
  P1IFG &= ~IRQPIN;

  USICTL0 |= USIPE7 + USIPE6 + USIPE5 + USIMST + USIOE;
  USICTL1 |= USIIE + USICKPH;
  USICKCTL = USIDIV_2 + USISSEL_2;
  USICTL0 &= ~USISWRST;
  USICNT = 0;
  USISRL = 0x20;
}

void transfer(unsigned char * buffer, int count)
{
  g_buffer = buffer;
  g_size = count;
  g_i = 0;

  //This msp430g2231 has a known defect to shift an extra bit for the first byte. 
  //do an initial flush

  if (isFirst == 1)
    {
      USISRL = 0x0;
      USICNT = 7;
      _BIS_SR(LPM0_bits + GIE); 
    }

  P2OUT &= ~CSPIN;

  USISRL = g_buffer[0];
  USICNT |= 8;
    
  while (g_i != g_size)
    {
      _BIS_SR(LPM0_bits + GIE); 
    }

  P2OUT |= CSPIN; // cycle the CS pin
}


#pragma vector = USI_VECTOR
__interrupt void USI_TXRX (void)
{
  if (g_buffer == 0)
    {
      USICTL1 &= ~USIIFG;                  // Clear pending flags
      return;
    }

  if (isFirst == 1)
    {
      isFirst = 0;
      LPM0_EXIT;
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
