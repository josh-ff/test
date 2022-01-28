#include "74HC137.h"

// takes 0-3 as input, directs SBC to talk to corresponding AD7195
void set_addr(int chip_num){
	int i, dir = 0;
	for (auto pin : PINS){
		dir = chip_num &  (1 << i++); // cute mechanism to decode to binary and retreive the direction
		DIOMaster_DataSet(pin, dir);
	}
}