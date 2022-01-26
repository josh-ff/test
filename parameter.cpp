#include "parameter.h"
#include <iostream>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <string.h>
#include "common_helpers.h"
#include "debug.h"
#include "dioMaster.h"
static std::string configFile = "/home/usrFtp/config.reg";
static std::string persistFile = "/home/usrFtp/persist.reg";
static std::string tempPersistFile = "/home/usrFtp/persist_temp.reg";
static std::string regFile = "/home/usrFtp/reg.reg";

/*
Big idea:
  We update the nonvolatile parameters right before updating the firmware.
  But what if we added new nonvolatile parameters, which the CURRENT version of the firmware
    is unaware of? (IE, two new fields at the end of the struct)
  We want to write these two new fields at the end of persistent.reg, so they'll be loaded
    on reboot.
  So, what we do is we allocate TWO PersistentRegisters_t in global data (and make sure they're
    not optimized away). Extra values will be put into the SECOND array entry (deliberate buffer overflow)
  Then, upon saving, the entirety of both buffers are saved to registers.
  That way, "extra" fields will be saved to persistent memory.
  NOTE: A bunch of 0's are also saved -- these are the empty entries of the second buffer.
*/
// we do this to allow intentional overflow of writing registers. Useful when saving.
#pragma GCC push_options
#pragma GCC optimize ("O0")
ConfigRegisters_t configRegistersArr[2];
ConfigRegisters_t& configRegisters = configRegistersArr[0];
PersistentRegisters_t persistRegistersArr[2];
PersistentRegisters_t& persistRegisters = persistRegistersArr[0];
Registers_t registers;
#pragma GCC pop_options

std::mutex save_lock;
std::mutex status_lock;
// saves the config registers into the file "config.reg"
void SaveConfigParameters(void){
	std::lock_guard<std::mutex> lock (save_lock);
	const int32_t* buf = (int32_t*)&configRegisters;
	std::ofstream outfile;
	outfile.open(configFile, std::ofstream::out);

	if(outfile.fail()){
		std::cerr << "Error saving parameters: " << strerror(errno);
		return;
	}

	// the *2 ensures that values from the extra register buffer are also saved saved
	for(int i = 0; i < NUM_REGISTERS_ConfigParameters * 2; i++){
		int32_t value = buf[i];
		outfile << value << "\n";
	}
	outfile.close();
}

// sets the parameter in the buffer
void SetConfigParameter(int index, int32_t value) {

	if(index > NUM_REGISTERS_ConfigParameters)
		debug("Writing beyond standard nonvol registers. Thus, expecting an imminent FW update\n");
	int32_t* buf = (int32_t*)&configRegisters;

	buf[index] = value;
}

void ReadConfigParameters(){
	memset(&configRegisters, 0, sizeof(ConfigRegisters_t));
	int32_t* buf = (int32_t*)&configRegisters;
	std::string line;
	std::ifstream infile(configFile);
	int reg = 0;

	// read all valid registers into memory
	while (getline(infile, line) && reg < NUM_REGISTERS_ConfigParameters){
		std::istringstream iss(line);

		int32_t value;
		if (!(iss >> value )) break;
		buf[reg] = value;
		reg += 1;

	}
	infile.close();
}

// saves the persistent registers into the file "persist.reg"
void SavePersistParameters(void){
	std::lock_guard<std::mutex> lock (save_lock);
	const int32_t* buf = (int32_t*)&persistRegisters;
	std::ofstream outfile;
	try {
		outfile.open(tempPersistFile, std::ofstream::out);

		if(outfile.fail()){
			std::cerr << "Error saving parameters: " << strerror(errno);
			return;
		}

		// the *2 ensures that values from the extra register buffer are also saved saved
		for(int i = 0; i < NUM_REGISTERS_PersistentRegisters * 2; i++){
			int32_t value = buf[i];
			outfile << value << "\n";
		}
		outfile.close();
		std::stringstream ss;
		ss << "cp -f " << tempPersistFile << " " << persistFile;
		std::string cp_cmd = ss.str();

		systemCall(cp_cmd);

	} catch (int e) {
		printf("Error opening parameter file\n");
		return;
	}
}

// sets the parameter in the buffer
void SetPersistParameter(int index, int32_t value) {
	std::lock_guard<std::mutex> lock (save_lock);
	if(index > NUM_REGISTERS_PersistentRegisters)
		debug("Writing beyond standard nonvol registers. Thus, expecting an imminent FW update\n");
	int32_t* buf = (int32_t*)&persistRegisters;

	buf[index] = value;
}

// reads the parameter in the buffer
int32_t ReadPersistParameter(int index) {
	std::lock_guard<std::mutex> lock (save_lock);
	if(index > NUM_REGISTERS_PersistentRegisters)
		debug("Writing beyond standard nonvol registers. Thus, expecting an imminent FW update\n");
	int32_t* buf = (int32_t*)&persistRegisters;
	return buf[index];
}

void ReadPersistParameters(){
	std::lock_guard<std::mutex> lock (save_lock);
	memset(&persistRegisters, 0, sizeof(PersistentRegisters_t));
	int32_t* buf = (int32_t*)&persistRegisters;
	std::string line;
	std::ifstream infile(persistFile);
	int reg = 0;
	int sum = 0;
	// read all valid registers into memory
	while (getline(infile, line) && reg < NUM_REGISTERS_PersistentRegisters){
		std::istringstream iss(line);

		int32_t value;
		if (!(iss >> value )) break;
		buf[reg] = value;
		reg += 1;
		sum += value;
	}
	infile.close();
	if (sum == 0) { // Persist file was empty!  Try the temp one!
		debug("PERSIST PARAMETERS ARE EMPTY, LOADING UP TEMP VALUES\n");
		std::ifstream infile(tempPersistFile);
		while (getline(infile, line) && reg < NUM_REGISTERS_PersistentRegisters){
			std::istringstream iss(line);

			int32_t value;
			if (!(iss >> value )) break;
			buf[reg] = value;
			reg += 1;
		}
	}

	infile.close();
}

// saves the ram registers into the file "reg.reg"
void SaveRegisters(Registers_t tempReg){
	std::lock_guard<std::mutex> lock (save_lock);
	const uint16_t* buf = (uint16_t*)&tempReg;
	std::ofstream outfile;
	outfile.open(regFile, std::ofstream::out);

	if(outfile.fail()){
		std::cerr << "Error saving parameters: " << strerror(errno);
		return;
	}

	// the *2 ensures that values from the extra register buffer are also saved saved
	for(int i = 0; i < NUM_REGISTERS_Registers; i++){
		uint16_t value = buf[i];
		outfile << value << "\n";
		// debug("Saving RamRegister location %u, value: %u\n", i, value);
	}
	outfile.close();
}

void ReadRegisters(Registers_t &tempReg){
	// memset(&registers, 0, sizeof(Registers_t));
	uint16_t* buf = (uint16_t*)&tempReg;
	std::string line;
	std::ifstream infile(regFile);
	int reg = 0;

	// read all valid registers into memory
	while (getline(infile, line) && reg < NUM_REGISTERS_Registers){
		std::istringstream iss(line);

		uint16_t value;
		if (!(iss >> value )) break;

		// debug("Reading RamRegister location %u, value: %u\n", reg, value);
		buf[reg] = value;
		reg += 1;

	}
	infile.close();
	Registers_t tempReg2;
	memset(&tempReg2, 0, sizeof(Registers_t));
	SaveRegisters(tempReg2);
}

void SetStatusFlag(int flag, bool set){
    std::lock_guard<std::mutex> lock (status_lock);
	registers.Status &= ~flag;
	if(!set)return;
	registers.Status |= flag;
}

bool ReadStatusFlag(int flag){
    std::lock_guard<std::mutex> lock (status_lock);
	return registers.Status & flag;
}

void SafeReboot() {
	debug("In safe reboot\n");
	// If we are in the middle of a rover command or lift command, we should fail to let people know!
	if ((registers.RoverCommandNo > registers.CompletedRoverCommandNo)
		|| (registers.LiftCommandNo > registers.CompletedLiftCommandNo) ) {
		SetStatusFlag(FLAGS_Failed, true);
		SetStatusFlag(FLAGS_CommandTimeout, true);
	}
	Registers_t tempReg = registers;
	tempReg.CurrentZero = -tempReg.CurrentPosAbs;

	SaveRegisters(tempReg);
    debug("ESTOP ON\n");
    setEStop(true);
    debug("CALLING SYNC\n");
    systemCall("sync");
    debug("CLOSING LOG\n");
    closeLogFile();
    systemCall("reboot");
}

