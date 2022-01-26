VERSION=0x03
#OBJDIR=/home/usrFtp/code/obj
#SRCDIR=/home/usrFtp/code

ELC_DEBUG=elc-debug

ELC_SRC_FILES =dioMaster.cpp spi.cpp AD7195.cpp LoadCellManager.cpp elc_tester.cpp


BAG2_SRC_FILES += $(wildcard openmv/src/*.cpp) $(filter-out openmv/openmvrpc_test.cpp, $(wildcard openmv/*.cpp)) $(SENSOR_FILES)
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
override CC=arm-linux-gnueabihf-g++-6
else
	ifeq ($(CROSS),1)
	override CC=arm-linux-gnueabihf-g++-6
endif
endif

.PHONY : clean
clean:
	@rm elc-debug

$(ELC_DEBUG) : $(ELC_SRC_FILES)
	@echo 
	@echo Making ELC Debug 
	$(CC) $(DEBUG_BUILD_OPTIONS) $@ $^
	@echo
	@echo Build done, Make version $(VERSION)
	@echo $(shell date)

