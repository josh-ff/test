/*
 * timer.c
 *
 *  Created on: Jul 18, 2018
 *      Author: mike.starkey
 */

#include "timer.h"
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "debug.h"


double TimerMilliseconds(){
	struct timespec start;
	double elapsed = 0;
	int ret = clock_gettime(CLOCK_MONOTONIC, &start);
	if (ret == -1) {
		debug("ERROR GETTING TIME, ERRNO: %d", errno);
	}
	elapsed = start.tv_sec + (start.tv_nsec / 1000000000.0);
	elapsed = elapsed * 1000;

	//registers.RtcTicks = (uint32_t)(elapsed); //TODO: remove line or populate if used
	return (elapsed);
}

uint16_t TimerMillisecondsU16(){
	double u = TimerMilliseconds();
	uint16_t time = (uint16_t)(((int)u) & 0xFFFF);
	return time;
}

uint32_t TimerMillisecondsU32(){
	double u = TimerMilliseconds();
	uint32_t time = (uint32_t)(((int64_t)u) & 0xFFFFFFFF);
	return time;
}

uint16_t TimerMillisecondElapsedU16(double start){
	double t = TimerMilliseconds();
	double elapsed = t - start;
	if(elapsed < 0){
		debug("NEGATIVE ELAPSED TIME: %f - %f \n", t, start);
	}
	return elapsed > 0 ? (uint16_t)elapsed : 0xFFFF;
}

uint32_t TimerMillisecondElapsedU32(double start){
	double t = TimerMilliseconds();
	double elapsed = t - start;
	if(elapsed < 0){
		debug("NEGATIVE ELAPSED TIME: %f - %f \n", t, start);
	}
	return elapsed > 0 ? (uint32_t)elapsed : 0xFFFFFFFF;
}

void TimerDelay(uint16_t t) {
    double t0 = TimerMilliseconds();
    while(TimerMillisecondElapsedU16(t0) < t){
    	usleep(1000);
    }
}
