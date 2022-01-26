/*
 * timer.h
 *
 *  Created on: Mar 5, 2014
 *      Author: mike.starkey
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>

double TimerMilliseconds();
uint16_t TimerMillisecondElapsedU16(double start);
uint32_t TimerMillisecondElapsedU32(double start);
uint16_t TimerMillisecondsU16();
uint32_t TimerMillisecondsU32();
void TimerDelay(uint16_t t);

#endif /* TIMER_H_ */
