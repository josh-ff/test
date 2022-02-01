#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

volatile static uint32_t *pinctl = NULL;

void InitializeIO(){
    int mem = open("/dev/mem", O_RDWR | O_SYNC);
    
    pinctl = (volatile uint32_t*)mmap(0, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, mem, 0x80018000);

    close(mem);
}

//DIO2:0 MUX, CPU Bank 1, 16-18
//DIO3 LC_SYNC,  CPU Bank 1, 19


void SetPin(int pin, int bank, unsigned int val){//SPI is bank 2
    auto muxpin = pin;
    auto reg = 0;
    if(pin > 15) {
        reg = 1;
        muxpin = pin - 16;
    } 
    auto first = (((pinctl[((0x900) + (0x10 * bank))/4]) >> pin) & 0x1);
    pinctl[((0x100) + (0x20 * bank) + (0x10 * reg) + 0x4)/4] = (0x3 << (muxpin * 2));
    pinctl[((0x700) + (0x10 * bank) + (0x8 / (val+1)))/4] = (0x1 << pin);
    pinctl[((0xB00) + (0x10 * bank) + 0x4)/4] = (0x1 << pin);
    auto last = (((pinctl[((0x900) + (0x10 * bank))/4]) >> pin) & 0x1);
    if(last == first && last != val)
        printf("PIN%d_%02d DID NOT CHANGE [%d - %d], VAL: %d\n", bank, pin, first, last, val);

}


void SPI_Sync(bool high){
    auto val = high ? 1 : 0;
    SetPin(19, 1, val);
}