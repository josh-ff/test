#ifndef LOADCELLMANAGER_H_
#define LOADCELLMANAGER_H_

#include <atomic>
#include <thread>
#include "AD7195.h"

class LoadCellManager {
public:
	LoadCellManager (AD7195& ad7195) : ad7195_(ad7195) {
		tareCntr_ = 0;
		tareVal_.store(0.0);
		taring_.store(false);
		tared_.store(false);
		running.store(false);
		thread_active.store(false);
	}
	void begin_polling();

	void end_polling(bool blocking = true);

	/*
	is_data_ready() reveals when a new load value is ready
	calling read() sets this to false until another read is availible
	*/
	bool is_data_ready() {return data_new.load();};
	double read();
	bool startTare(uint16_t numCycles = 20);
	void setTareVal(double valIn);
	bool is_taring() {return taring_.load();};
	double getTareVal() {return tareVal_.load();};

private:
	AD7195& ad7195_;
	std::atomic<double> load_;
	std::atomic<double> tareVal_;
	double lcVals_[2];
	std::atomic_bool taring_;
	std::atomic_bool tared_;
	uint16_t tareCntr_;
	uint16_t tareNum_;
	std::atomic_bool data_new;
	std::atomic_bool running;
	std::atomic_bool thread_active;

	bool taringAdd(double inVal);

	void LoadCellManagerWorkThread();
	uint32_t pollingtimeUs_;

	bool channelRead_[2];
};





#endif /* LOADCELLMANAGER */