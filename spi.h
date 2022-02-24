/*
 * i2c.h
 *
 *  Created on: October 5, 2020
 *      Author: josh 
 */

#ifndef SPI_H_
#define SPI_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stdint.h>

class SPI {
	public:
		SPI();
		// ~SPI();
		void transfer(uint8_t * txBuff, uint8_t * rxBuff, int len, int w_len=1);
		void transfer(uint8_t * txBuff, int len);
		void begin_transaction();
		void settings(uint32_t spiMode, uint32_t clkSpeedHz = 500000);
	private:
		int file_;
		struct spi_ioc_transfer xfer_[1];
		unsigned char buf[32], *bp;
		//int len, status;
		int status;
		const char *filename = "/dev/spidev0.0";

		//Spi Settings
		uint32_t speed = 500000;

};

#endif