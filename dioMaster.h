/*
 * dioMaster.h
 *
 *  Created on: Jan 06, 2020
 *      Author: arni
 */

#ifndef DIOMASTER_H_
#define DIOMASTER_H_

#include <stdint.h>

int8_t DIOMaster_DataSet(uint16_t pin_num, bool val);
int8_t DIOMaster_DirSet(uint16_t pin_num, bool val);
int8_t DIOMaster_Setup();
int8_t DIOMaster_Read(uint16_t pin_num);
void DIOMaster_RegisterRead();

void toggleEStop();
void setEStop(bool turnOn);

#endif /* DIOMASTER_H_ */
