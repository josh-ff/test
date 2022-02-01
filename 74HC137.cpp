#include "74HC137.h"
#include "gpio.h"

// takes 0-3 as input, directs SBC to talk to corresponding AD7195
void Mux_74HC137::set_addr(int chip_num){
	int i, dir = 0;
	for (auto pin : PINS){
		dir = chip_num &  (1 << i++); // cute mechanism to decode to binary and retreive the direction
		SetPin(pin, 1, dir); // all on bank 1
	}
}