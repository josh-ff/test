#include "spi.h"
#include <stdio.h>
#include <cstring>
SPI::SPI() {
	//struct spi_ioc_transfer xfer[2];
	file_ = open(filename, O_RDWR);
	if (file_ < 0) {
        // ERROR HANDLING; you can check errno to see what went wrong //
        char err[200];
        sprintf(err, "open('%s') in spi_init", filename);
        perror(err);
        return;
	}
}

// read and write in 2 spi frames
void SPI::transfer(uint8_t * txBuff, uint8_t * rxBuff, int len, int w_len){
	struct spi_ioc_transfer xfer[2];

    memset(xfer, 0, sizeof xfer);

    xfer[0].tx_buf = (unsigned long)txBuff;
    xfer[0].len = w_len;

    xfer[1].rx_buf = (unsigned long) rxBuff + w_len;
    xfer[1].len = len;
	int status = ioctl(file_, SPI_IOC_MESSAGE(2), xfer);
    // printf("TX: %02X -> ", txBuff[0]);
    for(auto x = 0; x < len; x++){
        // printf("%02X ", rxBuff[x]);
    }
    // printf("\n");
	if (status < 0) {
		printf("SPI READ FAIL");
		return;
	}
}

// pure write
void SPI::transfer(uint8_t * txBuff, int len)
{
	//struct spi_ioc_transfer	xfer[2];
	struct spi_ioc_transfer	xfer[1];
	int			status;

	memset(xfer, 0, sizeof xfer);

	xfer[0].tx_buf = (unsigned long)txBuff;
	xfer[0].len = len;
	

	//xfer[0].rx_buf = (unsigned long) rxBuff;
	//xfer[1].rx_buf = (unsigned long) txBuff;
	//xfer[1].len = len;

	status = ioctl(file_, SPI_IOC_MESSAGE(1), xfer);
	if (status < 0) {
		perror("SPI TX FAIL");
	}
}



// initializes configuration of SPI bus
void SPI::begin_transaction()
{
	__u8	lsb, bits;
	__u32	mode, speed;


	if (ioctl(file_, SPI_IOC_RD_MODE32, &mode) < 0) {
		perror("SPI rd_mode");
		return;
	}
	if (ioctl(file_, SPI_IOC_RD_LSB_FIRST, &lsb) < 0) {
		perror("SPI rd_lsb_fist");
		return;
	}
	if (ioctl(file_, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0) {
		perror("SPI bits_per_word");
		return;
	}
	if (ioctl(file_, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) {
		perror("SPI max_speed_hz");
		return;
	}

	printf("%s: spi mode 0x%x, %d bits %sper word, %d Hz max\n",
		filename, mode, bits, lsb ? "(lsb first) " : "", speed);
}

void SPI::settings(uint32_t spiMode, uint32_t clkSpeedHz)
{
	if (ioctl(file_, SPI_IOC_WR_MODE32, &spiMode) < 0) {
		printf("Error Setting SPI Mode");
		return;
	}	
		if (ioctl(file_, SPI_IOC_WR_MAX_SPEED_HZ, &clkSpeedHz) < 0) {
		printf("Error Setting SPI Clock Frequency");
		return;
	}
    //return;
	printf("SPI Settings Configured:\n-> ");
	this->begin_transaction();
}
