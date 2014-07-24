firmware : main.c nrf24.o pwm.o
	msp430-gcc -mmcu=msp430g2231 main.c nrf24.o pwm.o -g -o ../bin/msp430-firmware
nrf24.o : nrf24.c nrf24.h
	msp430-gcc -mmcu=msp430g2231 nrf24.c -g -c -o nrf24.o
pwm.o : pwm.c pwm.h
	msp430-gcc -mmcu=msp430g2231 pwm.c -g -c -o pwm.o