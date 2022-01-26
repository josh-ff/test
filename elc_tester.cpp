#include <iostream>
#include <stdio.h>
#include "spi.h"
#include "AD7195.h"
#include "dioMaster.h"
#include "registers.h"
#include <cstring>
//#include <unistd.h>
#include <time.h>
#include "LoadCellManager.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <string>
#include <iomanip>

using namespace std;

//const int CS_PIN = 8;
const int SYNC_PIN = 4;

int main(int argc, char * argv[]){
	//For sleep
	uint32_t numSamples = 500;
	uint16_t sampleSpeed = 0x08;
	uint8_t delayTime = 2;
	std::ofstream dataFile;
	std::string fname;
	if(argc <= 1)
	{
		printf("No File Name Given terminating.\n");
	}
	if(argc >= 3) //If a non default number of samples is given
	{
		numSamples = stoi(argv[2]);
	}
	if(argc >= 4) //If a non default sampling speed is given
	{
		sampleSpeed = stoi(argv[3]);
	}
	fname = std::string(argv[1]);
	dataFile.open(fname);

	printf("\r\n---Starting Test Script---\n");
	auto dio_status = DIOMaster_Setup();	
	
	DIOMaster_DirSet(SYNC_PIN , 1);
	DIOMaster_DataSet(SYNC_PIN  , 1);	//Set CS HIGH
	auto spi = SPI();
	auto ad7195 = AD7195(spi, 1);
	(void) dio_status;

	/* AD7195 Init Begin*/

	bool elcOnline = ad7195.init();
	printf("Initialize AD7195 %s\n",elcOnline  ? "Succeeded" : "Failed");

	 if(!elcOnline)
	 {
	 	return -1; // Kill program, LC's aren't talking
	 }


	 ad7195.buildModeReg(0,sampleSpeed);
	 ad7195.buildConfigReg(3, 7, true, false);
	 //printf("Writing Settings to AD7195 %s\n",ad7195.writeSettings() ? "Succeeded" : "Failed");
	 ad7195.setConversionFactor(0.002, 20000.0);

	printf("Single Channel SamplingPeriod [%d us]", ad7195.getSamplingPeriodUs());
	dataFile << "Single Channel SamplingPeriod, " << ad7195.getSamplingPeriodUs() << ",us" << std::endl;

	LoadCellManager loadcellmanager(ad7195);
	/* AD7195 Init End*/
	loadcellmanager.begin_polling();
	loadcellmanager.startTare();
	while(loadcellmanager.is_taring())
	{
				//Block until taring is complete
	}
	loadcellmanager.end_polling();
	printf("\nTare Value Set At: %F g\nDataLogging Begins in %d seconds", loadcellmanager.getTareVal(),delayTime);

	std::this_thread::sleep_for (std::chrono::seconds(delayTime));
	uint8_t j =1;
	printf("\n---Run %d---\n", j);	
	printf("Enabling Load Cell Amp\n");
	loadcellmanager.begin_polling();
	auto startTime = std::chrono::high_resolution_clock::now();

	dataFile << "Tare: " << loadcellmanager.getTareVal() << "g" << std::endl;

	for(int i=0; i<(int)numSamples; i++)
	{
		while(!loadcellmanager.is_data_ready())
		{
			//Block until there's new data
		}
		double readVal = loadcellmanager.read();
		//printf("[%2d]Load Value: %0.8F\n", i, loadcellmanager.read());
		dataFile << i << "," << std::fixed << std::setw( 7 ) << std::setprecision( 1 ) << std::setfill( '0' ) <<readVal << std::endl;

	}
	auto finishTime = std::chrono::high_resolution_clock::now();
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count();
	double sampleFreq = (numSamples/microseconds)*1000000;
	printf("Disabling Load Cell Amp\n");
	printf("Effective Polling Frequency: %0.8F hz\n", sampleFreq);
	dataFile << "Measured Freq, " << std::setw( 7 ) << std::setprecision( 1 ) << std::setfill( '0' ) << sampleFreq << " Hz" << std::endl; 
	loadcellmanager.end_polling(true); //Blocking end polling

	//std::this_thread::sleep_for (std::chrono::seconds(1));
	
	printf("\r\n---Ending Test Script---\n");
	dataFile.close();
	printf("Data Saved to: %s\n", fname.c_str());
	return 0;
}