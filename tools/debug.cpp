#include "debug.h"
#include <stdarg.h>
#include "timer.h"
#include <iostream>
#include <mutex>
#include <assert.h>
#include "descriptor.h"
#include "../ProtectedQueue.h"
#include "../Message.h"
#include "../common_fw.h"
#include "common_helpers.h"

static std::mutex print_lock;
static Descriptor fd("/home/usrFtp/code/log.txt");
static double firstDebug = 0;
static ProtectedQueue<Message> loggerQ;
static pthread_t tid;
static const int flushInterval = 10;	// sec

static void SendToLogger(uint16_t len, const char* msg)
{
	auto ptr = std::make_shared<Message>(len, msg);
	loggerQ.Put(ptr);
}

void* LoggerThread(void* arg)
{
	time_t last = time(NULL);

	pthread_detach(tid);

	// lower my priority
	setFIFOPrioSched(tid, 50);

	while (1) {
		auto ptr = loggerQ.Get();

		// log file already closed
		if (fd.getFd() == NULL)
			continue;

		{
		    std::lock_guard<std::mutex> guard(print_lock);
			fputs(ptr->GetValue(), fd.getFd());

			time_t now = time(NULL);

			// time to flush
			if (now - last >= flushInterval) {
				fd.flush();
				last = now;
			}
		}
	}
}

void closeLogFile() {
    std::lock_guard<std::mutex> guard(print_lock);
    fd.close();
}

void customLog(const char *format, ...) {
	if (firstDebug == 0) {
        fd.open();
		firstDebug = TimerMilliseconds();
		pthread_create(&tid, NULL, LoggerThread, NULL);
	}

	va_list arglist;
	va_start(arglist, format);

	char buffer[512];

	int last = sprintf(buffer, "[%.2f]\t", (TimerMilliseconds() - firstDebug)/1000.0);
	vsnprintf(buffer + last, sizeof(buffer) - last, format, arglist);
	va_end(arglist);

	// don't forget the terminating null
	SendToLogger(strlen(buffer) + 1, buffer);
}
