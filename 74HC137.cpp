#include "74HC137.h"
#include "gpio.h"

#include <iostream>

// takes 0-3 as input, directs SBC to talk to corresponding AD7195
void Mux_74HC137::set_addr(int chip_num){
	int i = 0; 
	int dir = 0;
	std::cout << "want this LC amp on: " << chip_num << std::endl;
	for (auto pin : PINS){
		std::cout << "Setting pin: " << pin << std::endl;
		dir = chip_num & (1 << i++); // cute mechanism to decode to binary and retreive the direction
		std::cout << "dir is: " << dir << std::endl;
		SetPin(pin, 1, dir); // all on bank 1
	}
}