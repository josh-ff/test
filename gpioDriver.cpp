#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DIO_Z 2
volatile unsigned int *pinctl = NULL;

void intialize_io(){
	auto devmem = open("/dev/mem", O_RDWR|O_SYNC);
	pinctl = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, devmem, 0x80018000);
}


/*******************************************************************************
* setdiopin: accepts a DIO register and value to place in that DIO pin.
*   Values can be 0 (low), 1 (high), or 2 (z - high impedance).
*******************************************************************************/
void setdiopin(int bank, int pin, int val)
{
	if(val == 2) {
		pinctl[((0xB00) + (0x10 * bank) + 0x8)/4] = (0x1 << pin);
	} else {
		pinctl[((0x700) + (0x10 * bank) + (0x8 / (val+1)))/4] =
		  (0x1 << pin);
		pinctl[((0xB00) + (0x10 * bank) + 0x4)/4] = (0x1 << pin);
	}	
}