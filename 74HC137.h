#ifndef 74HC137_H_
#define 74HC137_H_

#include <vector>
#include <tuple>

#include "dioMaster.h"

// middle ware for MUX: https://www.mouser.com/datasheet/2/302/74HC137_3-185343.pdf

class Mux_74HC137{
public:
Mux_74HC137() {
	DIOMaster_DirSet(MUX_A0, 1);
	DIOMaster_DirSet(MUX_A1, 1);
	DIOMaster_DirSet(MUX_A2, 1);

	// might also have to make sure we keep pin 39 high
}

// takes 0-3 as input, directs SBC to talk to corresponding AD7195
void set_addr(int chip_num);

private:
  // pin mappings to SBC frame: https://docs.embeddedts.com/TS-7400-V2#DIO
  int MUX_A0 = 0x01;
  int MUX_A1 = 0x03;
  int MUX_A2 = 0x04;
  std::vector<int> PINS = {MUX_A0, MUX_A1, MUX_A2};
}


#endif /* 74HC137_H_ */