firmware : msp430-nrf24-pwm.c nrf24 pwm
	msp430-gcc -mmcu=msp430g2231 msp430-nrf24-pwm.c ../bin/nrf24 ../bin/pwm  -o ../bin/msp430-firmware
nrf24 : nrf24.c nrf24.h
	msp430-gcc -mmcu=msp430g2231 nrf24.c -c -o ../bin/nrf24
pwm : pwm.c pwm.h
	msp430-gcc -mmcu=msp430g2231 pwm.c -c -o ../bin/pwm