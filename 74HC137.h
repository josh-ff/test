#ifndef Mux_74HC137_H_
#define Mux_74HC137_H_

#include <vector>

// middle ware for MUX: https://www.mouser.com/datasheet/2/302/74HC137_3-185343.pdf

class Mux_74HC137{
public:
	Mux_74HC137() {
		// might also have to make sure we keep pin 39 high
		;
	}
	// takes 0-3 as input, directs SBC to talk to corresponding AD7195
	void set_addr(int chip_num);

private:
  // pin mappings to SBC frame: https://docs.embeddedts.com/TS-7400-V2#DIO
  int MUX_A0 = 0x10;
  int MUX_A1 = 0x11;
  int MUX_A2 = 0x12;
  std::vector<int> PINS = {MUX_A0, MUX_A1, MUX_A2};
};


#endif /* Mux_74HC137_H_ */