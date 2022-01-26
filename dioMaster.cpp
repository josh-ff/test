/*
 * dio.cpp
 *
 *  Created on: Feb 1, 2019
 *      Author: Arni
 */

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <mutex>
#include <sys/mman.h>
#include <unistd.h>

#include "debug.h"
#include "dioMaster.h"
#include "parameter.h"

using namespace std;

#define GPIO_ADDR 0xE8000000 //IO starts at 4

#define DIGIO_IN		0x4 / (sizeof(uint32_t))
#define DIGIO_OUT		0x8 / (sizeof(uint32_t))

#define PC104C_DATA		0x18/(sizeof(uint32_t))
#define PC104C_DIR		0x28/(sizeof(uint32_t))
#define PC104D_DATA		0x1C/(sizeof(uint32_t))
#define PC104D_DIR		0x2C/(sizeof(uint32_t))

#define PC104C_MUX		0x38/(sizeof(uint32_t))
#define PC104D_MUX		0x3C/(sizeof(uint32_t))

#define PC104CD_MASK	0x7FFFF

#define DIO_MINPIN		1
#define DIO_MAXPIN		16
#define PC104C_MINPIN	2
#define PC104C_MAXPIN	19

volatile static uint32_t *ioAddress32;
static bool init{false};

mutex dioMaster_lock;

int8_t BitSet(uint8_t pin_num, bool val, uint8_t reg) {
	// debug( "PIN %i, set: %i\n", pin_num, val);
	uint8_t offset = pin_num - 1; // pin numbers off by 1
	lock_guard<mutex> lock(dioMaster_lock); // Turn on the lock
	// Update the relevant output bit
	if (val) ioAddress32[reg] |= (1 << offset);
	else ioAddress32[reg] &= ~(1 << offset);
	return 0;
}

int8_t BitRead(uint8_t pin_num, uint8_t reg) {
	lock_guard<mutex> lock(dioMaster_lock); // Turn on the lock
	uint32_t value = ioAddress32[reg];
	// debug( "PIN %i, read: %i\n", pin_num, value & (1 << (pin_num-1)));
	if (value & (1 << (pin_num-1))) return 1;
    else return 0;
}

bool CheckPin(uint8_t pin_num, uint8_t min, uint8_t max) {
	if ( (pin_num < min) || (pin_num > max)) {
		debug("Pin number should be in [%hhu,%hhu], but requested %hhu\n", min, max, pin_num);
		return false;
	}
	return true;
}

// for manipulation of the registers on the DIO header
int8_t DIO_DataSet(uint8_t pin_num, bool val) {
	// ensure valid input
	if (!CheckPin(pin_num, DIO_MINPIN, DIO_MAXPIN)) return -1;
	return BitSet(pin_num, val, DIGIO_OUT);
}

void DIO_RegisterRead() {
	lock_guard<mutex> lock(dioMaster_lock); // Turn on the lock
	registers.DIO = (uint16_t)(ioAddress32[DIGIO_IN] & 0xFFFF);
	registers.DIOMode = (uint16_t)(ioAddress32[DIGIO_OUT] & 0xFFFF);
}

int8_t DIO_Read(uint8_t pin_num){
	if (!CheckPin(pin_num, DIO_MINPIN, DIO_MAXPIN)) return -1;
	return BitRead(pin_num, DIGIO_IN);
}

int PC104C_DirSet(int pin_num, bool val) {
	if (!CheckPin(pin_num, PC104C_MINPIN, PC104C_MAXPIN)) return -1;
	return BitSet(pin_num, val, PC104C_DIR);
}

int PC104C_DataSet(int pin_num, bool val) {
	if (!CheckPin(pin_num, PC104C_MINPIN, PC104C_MAXPIN)) return -1;
	PC104C_DirSet(pin_num, 1);  // Make the pin an output first
	return BitSet(pin_num, val, PC104C_DATA);
}

void PC104C_RegisterRead() {
	lock_guard<mutex> lock(dioMaster_lock); // Turn on the lock
	registers.DIO104C = (ioAddress32[PC104C_DATA] & PC104CD_MASK);
	registers.DIO104CMode = (ioAddress32[PC104C_DIR] & PC104CD_MASK);
}

int8_t PC104C_Read(uint8_t pin_num){
	if (!CheckPin(pin_num, PC104C_MINPIN, PC104C_MAXPIN)) return -1;
	return BitRead(pin_num, PC104C_DATA);
}

int8_t DIOMaster_Setup(){
	int mem = open("/dev/mem", O_RDWR|O_SYNC);
	if (mem < 0) {
        debug("DIOMaster failed to open /dev/mem with : %s\n",strerror(errno));
        return -1;
    }

    // maps IO pins of the on-board computer to virtual memory
    //  constants declared in computer board manual 7.3.2
	ioAddress32 = (volatile uint32_t*) mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, mem, GPIO_ADDR);
	if (ioAddress32 == nullptr) {
		debug("MMap Failed\n");
		return -1;
	}
    // Set the PC104 C and D row MUX to GPIO.  // Write 0 to first 19 bits
    //  In TS-7800-V2 manual 8.1.3
    ioAddress32[PC104C_MUX] &= ~PC104CD_MASK;
    ioAddress32[PC104D_MUX] &= ~PC104CD_MASK;

    close(mem);
    init = true;
    return 0;
}

	// Filter out the register we need to use
void SplitPinAndReg(uint16_t &pin_num, uint16_t &reg) {
	reg = (pin_num & 0xFF00) >> 8;
	pin_num = pin_num & 0xFF;
}

int8_t DIOMaster_DataSet(uint16_t pin_num, bool val) {
	if (!init) {
        debug("DIO not setup\n");
        return -1;
    }
    uint16_t reg;
	SplitPinAndReg(pin_num, reg);

	switch(reg) {
		case DIGIO_IN: {
			return DIO_DataSet(pin_num, val);
			break;
		}
		case PC104C_DATA: {
			return PC104C_DataSet(pin_num, val);
			break;
		}
	}
	debug("WRONG SHIT TO DIO MASTER DATASET\n");
	debug("set\n");
	debug("pinnum\t%x\n", pin_num);
	return -1;
}

int8_t DIOMaster_DirSet(uint16_t pin_num, bool val) {
	if (!init) {
        debug("DIO not setup\n");
        return -1;
    }
    uint16_t reg;
	SplitPinAndReg(pin_num, reg);

	switch(reg) {
		case DIGIO_IN: {
			if (!val) return DIO_DataSet(pin_num, !val); // If "input" set the data pin high.  Equivalent to input behavior on other pins
			else return 1;  // Can't change DIO pin dir separately.  Always good request
			break;
		}
		case PC104C_DATA: {
			return PC104C_DirSet(pin_num, val);
			break;
		}
	}
	debug("WRONG SHIT TO DIO MASTER DIRSET\n");
	debug("dir\n");
	debug("pinnum\t%x\n", pin_num);
	return -1;
}

int8_t DIOMaster_Read(uint16_t pin_num) {
	uint16_t reg;
	SplitPinAndReg(pin_num, reg);

	switch(reg) {
		case DIGIO_IN: {
			return DIO_Read(pin_num);
			break;
		}
		case PC104C_DATA: {
			return PC104C_Read(pin_num);
			break;
		}
	}
	debug("read\n");
	debug("pinnum\t%x\n", pin_num);
	debug("WRONG SHIT TO DIO MASTER\n");
	return -1;
}

void DIOMaster_RegisterRead() {
	DIO_RegisterRead();
	PC104C_RegisterRead();
}
void setEStop(bool turnOn) {
	int onOff;
#ifdef BAG2_FW
	onOff = turnOn ? 1 : 0;
#else
	onOff = turnOn ? 0 : 1;
#endif
	DIOMaster_DataSet(DIO_EStop, onOff);
}

void toggleEStop() {
#ifdef BAG2_FW
		if (!DIOMaster_Read(DIO_EStopStatus)) { //
			DIOMaster_DataSet(DIO_EStop, 1);
			debug("ESTOP ON\n");
		}
		else {
			DIOMaster_DataSet(DIO_EStop, 0);
			debug("ESTOP OFF\n");
		}
#else
		if (DIOMaster_Read(DIO_EStop)) { //
			DIOMaster_DataSet(DIO_EStop, 0);
			debug("ESTOP ON\n");
		}
		else {
			DIOMaster_DirSet(DIO_EStop, 0);  // Need to set to input.  Statically high, then we can read if button is pushed
			debug("ESTOP OFF\n");
		}
#endif
}
