
#include <cstdlib>
#include <iostream>

#include "../RF24.h"


//RF24 radio(9,10);
RF24 radio("/dev/spidev0.0",8000000 , 25);  //spi device, speed and CSN,only CSN is NEEDED in RPI


// sets the role of this unit in hardware.  Connect to GND to be the 'pong' receiver
// Leave open to be the 'ping' transmitter
const int role_pin = 7;

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xE7E7E7E7E7LL, 0xC2C2C2C2C2LL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.  The hardware itself specifies
// which node it is.
//
// This is done through the role_pin
//

// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role;

void setup(void)
{
  //
  // Role
  //

  // set up the role pin
 // pinMode(role_pin, INPUT);
  //digitalWrite(role_pin,HIGH);
 // delay(20); // Just to get a solid reading on the role pin

  // read the address pin, establish our role
  //if ( ! digitalRead(role_pin) )
    role = role_ping_out;
  //else
  //  role = role_pong_back;

  //
  // Print preamble:
  //

  //Serial.begin(115200);
  //printf_begin();
  printf("\n\rRF24/examples/pingpair/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);

  //
  // Setup and configure rf radio
  //

  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
//  radio.setPayloadSize(8);
  radio.setDataRate(RF24_2MBPS);
  radio.setChannel(0x02);
  radio.setPALevel(RF24_PA_MAX);

  //
  // Open pipes to other nodes for communication
  //

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)

  if ( role == role_ping_out )
  {
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  }
  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
}

int main(int argc, char** argv)
{
        setup();
	bool ok;
	char color[3] = {0};
	while (1)
	  {
	    unsigned long time = 0xFF8040;
	    color[0] = atoi(argv[1]);
	    color[1] = atoi(argv[2]);
	    color[2] = atoi(argv[3]);

	    printf("Now sending %b %b %b...",color[0], color[1], color[2]);
	    ok = radio.write(color, 3);
    
	    if (ok) {
	      printf("ok...\n");
	      break;
	    }
	    else
	      printf("fail...\n");
	  }
        return 0;
}


// vim:cin:ai:sts=2 sw=2 ft=cpp