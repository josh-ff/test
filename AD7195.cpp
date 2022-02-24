#include "AD7195.h"

#include <chrono>
#include <thread>

LCState AD7195::getState()
{
  return state_;
}

void AD7195::setState(LCState state)
{
  state_ = state;
}

uint8_t AD7195::getErrCntr()
{
  return badReads_;
}

bool AD7195::checkDrdySPI7195(){
  getStatusReg();
  bool readReady = !(status_ & 0x80); 
  if (readReady) return true;
  return false;
}

// middleware work function
bool AD7195::readLCData(){
  NoRef_ = true;
  if (checkDrdySPI7195()){
    //getDataRef7195(); // packs data into Data_
    return (!DataErr_ && !NoRef_);
  }
  else {
    return false;
  }
}

int8_t AD7195::buildModeReg(uint8_t mode, uint16_t filtSpeed, bool dataStatus, bool enParity,
bool Sinc3, bool singleCycle, bool rej60, uint8_t clock)
{
  //Prefilter inputs to make sure valid values are getting assigned

  if (mode > 7)
  {
    perror("Invalid AD7195 Mode");
    return -3;
  }
  if (filtSpeed > 1023)
  {
    perror("Invalid AD7195 Filter Speed");
    return -4;
  }
  if (clock > 3)
  {
    perror("Invalid AD7195 Clock Setting");
    return -5;  
  }

  uint32_t dataRateCalcNoChop = 4800/filtSpeed;
  uint32_t dataRateCalcChop =  dataRateCalcNoChop/(3+(!Sinc3));


  modeWriteBytes_[0] = 0x08;//(filtSpeed & 0xFF); // TODO possibly uncomment
  modeWriteBytes_[1] = ((filtSpeed & 0x300)>>8) | (singleCycle<<3) | (enParity << 5) | (Sinc3 << 7) | (rej60 <<2);
  modeWriteBytes_[2] = ((clock & 0x03)<<2) | (dataStatus << 4) | ((mode & 0x07)<<5);

  printf("Mode Register Built [0x%02X 0x%02X 0x%02X]\n",modeWriteBytes_[2],modeWriteBytes_[1],modeWriteBytes_[0]);
  printf("    Mode: %d\n\
    Status in Data: %d\n\
    Clock Config: : %d\n\
    Use Sinc3: %d\n\
    Use Parity: %d\n\
    Single Cycle Conversion:: %d\n\
    Reject 60Hz Filter: %d\n\
    Filter Datarate (Chop [%s]/No Chop): %d Hz / %d Hz\n\n",  
    mode, dataStatus, clock, Sinc3, enParity, singleCycle, rej60, Sinc3 ? "SINC3" : "SINC4", dataRateCalcChop, dataRateCalcNoChop);
    return 0;
}

int8_t AD7195::buildConfigReg(uint8_t channel, uint8_t gain, bool refDetect, bool chop,
  bool acExcite, bool enBuffer, bool burnoutCurrents, bool unipolarMeasure)
{
  if (channel > 255)
  {
    perror("Invalid AD7195 Channel Mask");
    return -3;
  }
  if (gain > 7 || gain == 1 || gain == 2)
  {
    perror("Invalid AD7195 Gain Configuration");
    return -4;
  }
  if (burnoutCurrents && !enBuffer)
  {
    perror("Cannot enable burnout currents with buffer disabled");
    return -5;    
  }
  
  adcGain_ = pow(2,gain);
  unipolarMode_ = unipolarMeasure;

  confWriteBytes_[0] = (gain & 0x07) | (unipolarMeasure<<3) | (enBuffer<<4) | (refDetect<<6) | (burnoutCurrents<<7);
  confWriteBytes_[1] = channel;
  confWriteBytes_[2] = 0xC0 & ((acExcite<<6) | (chop<<7));

  printf("Configuration Register Built [0x%02X 0x%02X 0x%02X]\n",confWriteBytes_[2],confWriteBytes_[1],confWriteBytes_[0]);
  printf("    Chop: %d\n\
    AC Excitation: %d\n\
    Channel Mask: : %c%c%c%c%c%c%c%c\n\
    Burnout Currents: %d\n\
    Reference Detect: %d\n\
    Buffered Inputs:: %d\n\
    Unipolar Measurement: %d\n\n\
    Gain: %d\n", 
    chop, acExcite,   
  (channel & 0x80 ? '1' : '0'), 
  (channel & 0x40 ? '1' : '0'), 
  (channel & 0x20 ? '1' : '0'), 
  (channel & 0x10 ? '1' : '0'), 
  (channel & 0x08 ? '1' : '0'), 
  (channel & 0x04 ? '1' : '0'), 
  (channel & 0x02 ? '1' : '0'), 
  (channel & 0x01 ? '1' : '0'), 
  burnoutCurrents, refDetect, enBuffer, unipolarMeasure, adcGain_);


  return 0;
}


bool AD7195::init()
{
  printf("INITIALIZING CALL GUY");

  spi_.settings(SPI_MODE_3, 1000000);
  clearTXBuff();
  clearRXBuff();

  // printf("CALLING RESET\r\n");


  bool successRst = reset();
  if(!successRst)
  {
    errno = EIO;
    printf("Failed To Communicate with AD7195");
  }

  return successRst;

  //writeBuff_
}

bool AD7195::reset()
{
  memset(writeBuff_, 0xFF, 7);
  spi_.transfer(writeBuff_,7);
  clearTXBuff();
  clearRXBuff();

  return getIdReg();
}

uint8_t AD7195::getStatusReg()
{
  writeBuff_[0] = 0xFF;
  writeBuff_[1] = 0x40;
  clearRXBuff();
  spi_.transfer(writeBuff_,readBuff_,3,2);
  clearTXBuff();
  status_ = readBuff_[2];
  // printf("\nStatus reads as: 0x%2x\n",status_);
  clearRXBuff();
  return status_;
}


bool AD7195::getIdReg()
{
  writeBuff_[0] = 0xFF;
  writeBuff_[1] = 0x60;
  spi_.transfer(writeBuff_,readBuff_,3,2);
  clearTXBuff();
  uint8_t readByte = readBuff_[2];
  printf("\nID reads as: 0x%2x\n",readByte);
  clearRXBuff();
  return readByte == 0xA6;
}


uint32_t AD7195::getOffsetReg()
{
  clearTXBuff();
  writeBuff_[0] = 0x70;
  spi_.transfer(writeBuff_,readBuff_,4);
  clearTXBuff();
  uint32_t readBytes = ((readBuff_[1]<<16) + (readBuff_[2]<<8) + readBuff_[3]);
  clearRXBuff();
  return readBytes;
}

uint32_t AD7195::gettFullScaleReg()
{
  clearTXBuff();
  writeBuff_[0] = 0x78;
  spi_.transfer(writeBuff_,readBuff_,4);
  clearTXBuff();
  uint32_t readBytes = ((readBuff_[1]<<16) + (readBuff_[2]<<8) + readBuff_[3]);
  clearRXBuff();
  return readBytes;
}



void AD7195::clearTXBuff()
{
  memset(writeBuff_, 0, buffLen);
}

void AD7195::clearRXBuff()
{
  memset(readBuff_, 0, buffLen);
}

/*
bool AD7195::setModeReg()
{
  return 0;
}
*/

bool AD7195::writeSettings()
{
  writeBuff_[0] = 0x10; //Write to Config Reg
  writeBuff_[1] = confWriteBytes_[2];
  writeBuff_[2] = confWriteBytes_[1];
  writeBuff_[3] = confWriteBytes_[0];

  spi_.transfer(writeBuff_,4);
  clearTXBuff();

  writeBuff_[0] = 0x08; //Write to Mode Reg
  writeBuff_[1] = modeWriteBytes_[2];
  writeBuff_[2] = modeWriteBytes_[1];
  writeBuff_[3] = modeWriteBytes_[0];
  
  spi_.transfer(writeBuff_,4);
  clearTXBuff();

  printf("SettingWriter:\n    Mode Reg Read [0x%02X, 0x%02X, 0x%02X]\n\
  Config Reg Read [0x%02X, 0x%02X, 0x%02X]\n\n",
  modeWriteBytes_[2], modeWriteBytes_[1], modeWriteBytes_[0],
  confWriteBytes_[2], confWriteBytes_[1], confWriteBytes_[0]);

  //Verify everything you read back equals everything you sent out
  return (readSettings());
}


bool AD7195::readSettings()
{

  // verify mode settings 
  writeBuff_[0] = 0x48; //Read Back Mode Reg
  clearRXBuff();
  spi_.transfer(writeBuff_,readBuff_,4,1);
  modeReadBytes_[2] = readBuff_[1];
  modeReadBytes_[1] = readBuff_[2];
  modeReadBytes_[0] = readBuff_[3];

  clearRXBuff();
  writeBuff_[0] = 0x50; //Read Back Config Reg
  spi_.transfer(writeBuff_,readBuff_,4,1);
  confReadBytes_[2] = readBuff_[1];
  confReadBytes_[1] = readBuff_[2];
  confReadBytes_[0] = readBuff_[3];
  clearRXBuff();
  clearTXBuff();

  printf("SettingReader:\n    Mode Reg Read [0x%02X, 0x%02X, 0x%02X]\n\
    Config Reg Read [0x%02X, 0x%02X, 0x%02X]\n\n",
    modeReadBytes_[2], modeReadBytes_[1], modeReadBytes_[0],
    confReadBytes_[2], confReadBytes_[1], confReadBytes_[0]);


  return (confReadBytes_[2] == confWriteBytes_[2] && confReadBytes_[1] == confWriteBytes_[1] && 
    confReadBytes_[0] == confWriteBytes_[0] && 
    modeReadBytes_[2] == modeWriteBytes_[2] && 
    modeReadBytes_[1] == modeWriteBytes_[1] && 
    modeReadBytes_[0] == modeWriteBytes_[0]);
}


uint32_t AD7195::getSamplingPeriodUs()
{
  uint16_t chop  = ((confWriteBytes_[2] & 0x80) >>7);
  uint16_t syncN = ((confWriteBytes_[1] & 0x80) >>7);
  uint16_t filtSpeed = ((modeWriteBytes_[1] & 0x03)<<8) + modeWriteBytes_[0];

  //uint32_t dataRateCalcNoChop = (4800/(filtSpeed * (1 + ((3-syncN) * chop));
  
  return (filtSpeed * 10000 * (1 + ((3-syncN) * chop))) / 48;

}




//Calculate the conversion factor, 
//sensitivity is mv/V
//Full load is the full scale load of the sensor in the units you want raw to output 
//(IE a 20kg loadcell would be 20000 if you want return units to be in grams)
//RefVoltage is the ADC reference voltage (default 5.0V)
double AD7195::setConversionFactor(double sensitivity, double fullLoad)
{
    int unipolarInt = 0;
    if (unipolarMode_)
    {
      unipolarInt = 1;
    }
    unipolarInt++;
    //conversionFactor_ = ((1 /pow(2,(24-unipolarInt))- (double(unipolarInt)))/(adcGain_); //Value in volts
    conversionFactor_ = fullLoad/((pow(2,(24-unipolarInt))*adcGain_*sensitivity));
    printf("ADC Conversion Factor Calculated: %1.10F\n\n", conversionFactor_);
    return conversionFactor_;
  
}

int32_t AD7195::read_raw(bool blocking)
{
  if(blocking)
  {
    while(!checkDrdySPI7195())
    {

    }
  }
  clearTXBuff();
  writeBuff_[0] = 0x58; //Write to Mode Reg
  spi_.transfer(writeBuff_,readBuff_,5);
  clearTXBuff();
  //modeReadBytes_[3], confReadBytes_[3];
  printf("Data Register Read 0x%02X 0x%02X 0x%02X 0x%02X\n",readBuff_[1], readBuff_[2], readBuff_[3],readBuff_[4] );
  DataRaw_ = ((readBuff_[1]<<16) + (readBuff_[2]<<8) + readBuff_[3]) - 8388608; //Encoded from 0 = -V/Gain -> 2^24 = +v/Gain
  status_ = readBuff_[4];
  clearRXBuff();
  return DataRaw_;
}


double AD7195::read(bool blocking)
{
  return read_raw(blocking)*conversionFactor_;
}

uint8_t AD7195::getChannel()
{
  return status_ & 0x07;
}


//AD7195_INTERNAL_ZERO_SCALE, AD7195_INTERNAL_FULL_SCALE, AD7195_SYSTEM_ZERO_SCALE, AD7195_SYSTEM_FULL_SCALE
int32_t AD7195::runCalibration(uint8_t calMode, uint8_t channel, uint8_t gain, bool blocking)
{
  uint8_t calType = 0x00;
  uint8_t regVal = 0x00;
  if (gain > 0x07)
{
  perror("Invalid Gain");
  return -1;
}
  switch(calMode)
  {
    case AD7195_INTERNAL_ZERO_SCALE:
      printf("Internal Zero Scale Calibration\n");
      calType = 0x04;
      regVal = 0x06;
      break;
    case AD7195_INTERNAL_FULL_SCALE:
      printf("Internal Full Scale Calibration\n");
      calType = 0x05;
      regVal = 0x07;
      break;
    case AD7195_SYSTEM_ZERO_SCALE:
      printf("System Zero Scale Calibration\n");
      calType = 0x06;
      regVal = 0x06;
      break;
    case AD7195_SYSTEM_FULL_SCALE:
      //calVal = 0x07;
      printf("System Full Scale Calibration Not Implemented\n");
      break;
    default:    
      perror("AD7195 Calibration Mode does not exist");        
    }



    uint32_t calVal = 0;
    if (calType == 0x00)
    {
      perror("No Valid Calibration selected!");
      return 0;
    }  
    else
    {
      buildConfigReg(channel, gain);  //Set MUX to selected channel
      buildModeReg(calType, 0x200); //Set Mode to selected calibration
      writeSettings();
      if(blocking)
        {
        while(checkDrdySPI7195())
        {

        }
        printf("Calibration Started\n");
        while(!checkDrdySPI7195())
        {

        }
      
      if(regVal ==6)
      {
        calVal = getOffsetReg();
      }
      else
      {
        calVal = gettFullScaleReg();
      }
      printf("Register [0x%01X] Value : 0x%06X\n\n", regVal, calVal);
      return calVal;
      }
      else
      {
        printf("Non Blocking Calibration Used, Confirm that ADC has finished Calibration\n\n");
        return 0;
      }
      return -2; //shouldn't get here
    }
  
}

void AD7195::disable()
{
  writeBuff_[0] = 0x08; //Write to Mode Reg
  writeBuff_[1] = (modeWriteBytes_[2] & 0x1F) + 0x60;
  writeBuff_[2] = modeWriteBytes_[1];
  writeBuff_[3] = modeWriteBytes_[0];

  printf("Mode: 0x%02X | WriteByte: 0x%02X\n", modeWriteBytes_[2], writeBuff_[1]);

  spi_.transfer(writeBuff_,4);
  clearTXBuff();
  //modeReadBytes_[3], confReadBytes_[3];
  clearRXBuff();
}