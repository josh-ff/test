#ifndef __AD7195__H_
#define __AD7195__H_

#include <memory>
#include "spi.h"
#include <cstring>
#include <math.h>
//#include <stdio.h>
//#include <iostream>

#define DEBUGPRINT false



enum struct LCState : int {INIT, SYNC, RUN, ERR, RST};

//Addresses of all registers on chip, these are bit faluse for RS2, RS1, RS0
enum AD7195_REG_ADDR{AD7195_STATUS_REG, 
AD7195_MODE_REG, AD7195_CONFIG_REG,
AD7195_DATA_REG, AD7195_ID_REG, AD7195_GPOCON_REG, 
AD7195_OFFSET_REG, AD7195_FULL_SCALE_REG};


enum AD7195_CALIBRATION_MODES{AD7195_INTERNAL_ZERO_SCALE, AD7195_INTERNAL_FULL_SCALE,
AD7195_SYSTEM_ZERO_SCALE, AD7195_SYSTEM_FULL_SCALE};


/* 
#define AD7195_STATUS_REG 0x00
#define AD7195_MODE_REG 0x01
#define AD7195_CONFIG_REG 0x02
#define AD7195_DATA_REG 0x03
#define AD7195_ID_REG 0x04
#define AD7195_GPOCON_REG 0x05
#define AD7195_OFFSET_REG 0x06
#define AD7195_FULL_SCALE_REG 0x07
*/


// class AD7195IFace {
// public:
//     virtual ~AD7195IFace() = default;

//     virtual bool is_data_ready() = 0;
//     virtual int32_t read_raw() = 0;
//     virtual void reset()=0;
//     virtual int16_t convert_to_units(int32_t raw) = 0;
//     virtual void tare() = 0;
//     virtual void force_tare(int32_t offset) = 0;
// };

//MINV 7310000
//MAXV 9470000

class AD7195 {
  public:
    AD7195(SPI& spi, int sync, int32_t minV = 1, int32_t maxV = 0xFFFFFE, LCState state = LCState::INIT) : 
      spi_(spi), sync_(sync), minV_(minV), maxV_(maxV), state_(state) {}
    // AD7195();

    uint32_t getData() {return Data_;}
    bool getValid() {return (!DataErr_ && !NoRef_);}

    uint8_t getStatus() {return status_;}
    uint8_t getDataErr() {return DataErr_;}
    uint8_t getNoRef() {return NoRef_;}
    bool readLCData();  
    LCState getState();
    void setState(LCState state);
    void incBadReads(uint8_t incVal = 1);
    void decBadReads(uint8_t decVal = 1);
    bool checkDrdySPI7195();
    
    // TODO both need to be implemented
    
    int16_t convert_to_units(int32_t raw){
      return 0;
    }

    uint8_t getErrCntr();

    int8_t buildModeReg(uint8_t mode, uint16_t filtSpeed, bool dataStatus = true, bool enParity = true,
    bool Sinc3 = false, bool singleCycle = false, bool rej60 = true, uint8_t clock = 0x2);

    int8_t buildConfigReg(uint8_t channel = 0x01, uint8_t gain = 0x00, bool refDetect = true, bool chop = false, 
      bool acExcite = false, bool enBuffer = false, bool burnoutCurrents = false, bool unipolarMeasure = false);

    uint8_t getStatusReg();

    bool getIdReg();

    uint32_t getOffsetReg();

    uint32_t gettFullScaleReg();

    bool reset(); 

    bool init();

    bool writeSettings();

    bool readSettings();

    uint32_t getSamplingPeriodUs();

    double setConversionFactor(double sensitivity, double fullLoad);

    int32_t read_raw(bool blocking = false);

    double read(bool blocking = false);

    uint8_t getChannel();

    int32_t runCalibration(uint8_t calMode, uint8_t channel, uint8_t gain = 0x00, bool blocking = true);
    
    void disable();

    
  private: 
    SPI& spi_;
    int sync_;
    uint32_t minV_;
    uint32_t maxV_;
    int DATARATE = 500000;
    //uint8_t badReadsMax_;
    uint8_t readBuff_[20];
    uint8_t writeBuff_[20];
    uint8_t buffLen = sizeof(readBuff_)/sizeof(readBuff_[0]);
    LCState state_;
    
    //struct AD7195_Mode_Reg modeRead;
    //struct AD7195_Config_Reg confRead;


    uint8_t modeWriteBytes_[3], confWriteBytes_[3];
    uint8_t modeReadBytes_[3], confReadBytes_[3];

    uint32_t getModeBytes();
    uint32_t getConfigBytes();

    uint8_t badReads_ = 0;
    
    uint8_t status_;
    int32_t DataRaw_;
    double Data_;
    bool DataErr_;
    bool NoRef_;


    bool unipolarMode_ = false;
    uint8_t adcGain_ = 1;
    double conversionFactor_ = 1;

    void clearTXBuff();
    void clearRXBuff();

    // TODO(josh): set reg check, validate written values are stored
  };
#endif