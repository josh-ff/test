VERSION=0x03
#OBJDIR=/home/usrFtp/code/obj
#SRCDIR=/home/usrFtp/code

ELC_DEBUG=elc-debug
GPIO_DEBUG=gpio-debug

ELC_SRC_FILES =gpio.cpp spi.cpp AD7195.cpp LoadCellManager.cpp elc_tester.cpp 74HC137.cpp
GPIO_SRC_FILES = toggleled.cpp

ifeq ($(SOLO), 1)
STANDARD_BUILD_OPTIONS+= -DSOLO
endif

git_info=$(shell ./git_info.sh)

ifeq ($(STATIC),1)
	STANDARD_BUILD_OPTIONS += -static
endif

INCLUDE =-Itools
STANDARD_BUILD_OPTIONS+=$(INCLUDE) -DLOG -Wall -Werror -pthread -std=c++14 -O2 -o
DEBUG_BUILD_OPTIONS+=$(INCLUDE) -DLOG -Wall -Werror -pthread -std=c++14 -O0 -o
CC=g++

ifeq ($(shell lsb_release -si),Ubuntu)
override CC=arm-linux-gnueabi-g++-6
else
	ifeq ($(CROSS),1)
	override CC=arm-linux-gnueabi-g++-6
endif
endif

.PHONY : clean
clean:
	@rm elc-debug gpio-debug

$(ELC_DEBUG) : $(ELC_SRC_FILES)
	@echo 
	@echo Making ELC Debug 
	$(CC) $(DEBUG_BUILD_OPTIONS) $@ $^
	@echo
	@echo Build done, Make version $(VERSION)
	@echo $(shell date)

$(GPIO_DEBUG) : $(GPIO_SRC_FILES)
	@echo 
	@echo Making GPIO Debug 
	$(CC) $(DEBUG_BUILD_OPTIONS) $@ $^
	@echo
	@echo Build done, Make version $(VERSION)
	@echo $(shell date)