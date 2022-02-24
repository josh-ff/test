#include "LoadCellManager.h"
#include <iostream>
#include <chrono>

void LoadCellManager::begin_polling() {
	if(!thread_active)
	{
		thread_active.store(true);
		//printf("Checking Amp Configuration\n");
		//If Amp settings are different then what they should be reprogram
		if(!ad7195_.readSettings())	
		{
			//printf("Reconfiguring Amp\n");
			ad7195_.writeSettings();
			std::this_thread::sleep_for (std::chrono::milliseconds(1));
		}
		//TODO: add synchronization here

		std::this_thread::sleep_for (std::chrono::microseconds(500)); //Give slight offset from sync release
		pollingtimeUs_ =(ad7195_.getSamplingPeriodUs());
		//printf("Begin Polling\n");
		running.store(true);
		std::thread wthread(&LoadCellManager::LoadCellManagerWorkThread, this);
		wthread.detach();
	}

	else
	{
		errno = EINPROGRESS;
		perror("AD7195 Polling thread already running");
	}
}


void LoadCellManager::end_polling(bool blocking)
{
	running.store(false);
	//printf("Calling Thread Kill\n");
	if(blocking)
	{
		while (thread_active.load())
		{
			//Wait until Thread is killed
		}
	}
}


void LoadCellManager::LoadCellManagerWorkThread(){
	//printf("Poll Thread Started\n");
	while(running.load()){
		//printf("Poll Thread\n");
		std::this_thread::sleep_for (std::chrono::microseconds(pollingtimeUs_));
		if(!ad7195_.checkDrdySPI7195())
		{
			// printf("WORKTHREAD: Data Not Ready\n");
			std::this_thread::sleep_for (std::chrono::microseconds(pollingtimeUs_/5));
		}
		//If the amp has new lc data ready
		else
		{
			uint8_t readChannel = ad7195_.getChannel();
			if (!channelRead_[readChannel])
			{
				lcVals_[readChannel] = ad7195_.read();
				channelRead_[readChannel] = true;
			}

			/*
			If both channels are read calculate total load, 
			let system know data is ready, and reset read flags
			*/
			if(channelRead_[0])
			{
				
				
				channelRead_[0] = false;
				channelRead_[1] = false;
				if(taring_.load() == true)
				{
					// taringAdd(lcVals_[0] + lcVals_[1]);
					printf("Taring Regimine\r\n");
					taringAdd(lcVals_[0]);
					//printf("Poll Loop Tare\n");
				}
				else
				{	
					printf("Polling Regimine\n");
					// load_.store(lcVals_[0] + lcVals_[1] - tareVal_.load());
					load_.store(lcVals_[0] - tareVal_.load());
					data_new.store(true);
				}
			}
 		}
 		//printf("EOL Poll Thread\n");
 	}

 	//If end_polling() is called, put amp into low power mode and kill the thread
 	//Put Amp into low power mode when not polling
 	ad7195_.disable();
	channelRead_[0] = false;
	channelRead_[1] = false;
	lcVals_[0] = 0;
	lcVals_[1] = 0;
	thread_active.store(false);
	//printf("Poll Thread Ended\n");
 	return;
}

double LoadCellManager::read(){
	if(!running.load())
	{
		errno = EIO;
		printf("Attempting Load Cell Read while load cells are disabled!");
	}
	data_new.store(false);	// You've read the most recent data
	printf("returning read result: %f\r\n", load_.load());
	return(load_.load());
}

bool LoadCellManager::taringAdd(double inVal)
{
	tareVal_.store(tareVal_.load() + inVal);
	tareCntr_--;
	printf("Taring [%d] TareVal:%F\n", tareCntr_, tareVal_.load());
	if(tareCntr_ == 0)
	{
		tareVal_.store(tareVal_.load()/tareNum_);
		tareNum_ = 0;
		printf("another mechanism to set this shit false\r\n");
		taring_.store(false);
		tared_.store(true);
		//printf("Tare Calculated: %F\n", tareVal_.load());
		return true;
	}
	return false;
}

bool LoadCellManager::startTare(uint16_t numCycles)
{
	if(!running.load())
	{
		errno = EIO;
		perror("Attempted Taring Routine without polling routine running");
		return false;
	}
	if(taring_.load())
	{
		errno = EINPROGRESS;
		perror("Loadcell Manager Already in Tare Cycle call setTareVal() to kill tare routine");
		return false;
	}
	else
	{	
		//printf("Starting Tare\n");
		tareNum_ = numCycles;
		tareCntr_ = tareNum_;
		tareVal_.store(0);
		taring_.store(true);
		tared_.store(false);
		return true;
	}
}

void LoadCellManager:: setTareVal(double valIn)
{
	printf("Setting this shit false\r\n");
	taring_.store(false);
	tareCntr_ = 0;
	tareNum_ = 0;
	tareVal_.store(valIn);
	if(valIn == 0)
	{
		tared_.store(false);
	}
	else
	{
		tared_.store(true);
	}
}