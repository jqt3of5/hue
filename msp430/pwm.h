#define REDPIN 0x01
#define GREENPIN 0x08
#define BLUEPIN 0x10

void initPWM();
void enablePWM();
void disablePWM();

extern unsigned int R,G,B;
