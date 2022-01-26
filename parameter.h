
#ifndef PARAMETER_H_
#define PARAMETER_H_

#ifdef BAG_FW
	#include "registers.h"
#elif defined(TRAY_FW)
	#include "tray-registers.h"
#elif defined(BAG2_FW)
	#include "bag2-registers.h"
#else
	#include "registers.h"
#endif

extern ConfigRegisters_t& configRegisters;
extern PersistentRegisters_t& persistRegisters;
extern Registers_t registers;

void SetConfigParameter(int index, int32_t value);
void ReadConfigParameters(void);
void SaveConfigParameters();
void SetPersistParameter(int index, int32_t value);
int32_t ReadPersistParameter(int index);
void ReadPersistParameters(void);
void SavePersistParameters();
void SaveRegisters(Registers_t tempReg);
void ReadRegisters(Registers_t &tempReg);
void SetStatusFlag(int flag, bool set);
bool ReadStatusFlag(int flag);

void SafeReboot();
#endif /* PARAMETER_H_ */
