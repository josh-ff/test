#ifndef DEBUG_H_
#define DEBUG_H_

#include <stdio.h>

void customPrint(const char *format, ...);
void customLog(const char *format, ...);
void closeLogFile();
void flushLogFile();

#ifdef LOG
#define debug(fmt, ...) customLog(fmt, ##__VA_ARGS__)
#else
#define debug(fmt, ...) ((void)0)
#endif


#endif  // DEBUG_H_
