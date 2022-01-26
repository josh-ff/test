VERSION=0x03
#OBJDIR=/home/usrFtp/code/obj
#SRCDIR=/home/usrFtp/code
BUILDDIR = /home/usrFtp/code/build
DATE=$(shell date '+%Y%m%d' )
TRAY_TARGET=rover-tray-core
BAG_TARGET=rover-bag-core
BAG2_TARGET=rover-bag2-core
BAG_DEBUG=bag-debug
BAG2_DEBUG=bag2-debug
BAG_SOLO=bag-solo
TRAY_DEBUG=tray-debug
DIST_DEBUG=dist-debug
ELC_DEBUG=elc-debug
CYGPATH_W = echo
LINK_TARGET = $@_$(DATE)_$(VERSION)
STATIC ?= 0

# Docker image configuration
IMAGE_NAME := gcr.io/fulfil-fulfillment/lfr
IMAGE_TAG ?= $(subst /,-,$(shell git rev-parse --abbrev-ref HEAD))
DOCKER_BUILDER := multiarch-builder

SRC_FILES = $(wildcard *.cpp) $(wildcard tools/*.cpp) $(PROTOBUF_FILES)
SENSOR_FILES = $(wildcard sensors/*.cpp) $(wildcard sensors/VL53L1X/core/*.cpp) #$(filter-out sensors/VL53L1X/main.c, $(wildcard sensors/VL53L1X/*.c))
TRAY_SRC_FILES := $(filter-out DistSensors.cpp TCA9548A.cpp st-make.cpp bag2_fw.cpp rover_fw.cpp adc_ts7800.cpp muxInt.cpp loadcellstate.cpp BagLift2.cpp, $(SRC_FILES))
#TRAY_SRC_FILES := $(addprefix $(SRCDIR)/, $(TRAY_SRC_FILES))
BAG_SRC_FILES := $(filter-out DistSensors.cpp TCA9548A.cpp st-make.cpp bag2_fw.cpp lft_fw.cpp adc_ts7800.cpp muxInt.cpp, $(SRC_FILES))
#OBJ_FILES = $(addprefix $(OBJDIR)/, $(patsubst %.cpp, %.o, $(SRC_FILES)))
BAG2_SRC_FILES := $(filter-out st-make.cpp rover_fw.cpp lft_fw.cpp adc_ts7800.cpp muxInt.cpp, $(SRC_FILES))
DIST_SRC_FILES = tools/debug.cpp tools/timer.cpp tools/common_helpers.cpp muxInt.cpp i2c.cpp parameter.cpp dioMaster.cpp
DIST_SRC_FILES += $(SENSOR_FILES) $(PROTOBUF_FILES)

ELC_SRC_FILES = tools/debug.cpp tools/timer.cpp tools/common_helpers.cpp parameter.cpp dioMaster.cpp spi.cpp AD7195.cpp LoadCellManager.cpp elc_tester.cpp


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

.PHONY : all
all : $(BAG_TARGET) $(TRAY_TARGET) $(BAG2_TARGET)

#%.dep : %.cpp
#	$(CC) -M $(FLAGS) $< > $@
#	include $(OBJDIR)/$(OBJS:.o=.dep)
#	@echo $@

#REBUILDABLES = $(OBJ_FILES) $(TARGET)

#$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
#	@echo $@
#	$(CC) -g -pthread  -std=c++14 -O2 -o $@ -c $<

$(DIST_DEBUG): $(DIST_SRC_FILES)	
	@echo 
	@echo Making Dist Debug 
	$(CC) $(DEBUG_BUILD_OPTIONS) $@ $^
	@echo
	@echo Build done, Make version $(VERSION)
	@echo $(shell date)

$(ELC_DEBUG) : $(ELC_SRC_FILES)
	@echo 
	@echo Making ELC Debug 
	$(CC) $(DEBUG_BUILD_OPTIONS) $@ $^
	@echo
	@echo Build done, Make version $(VERSION)
	@echo $(shell date)

$(TRAY_TARGET):$(TRAY_SRC_FILES)
	$(git_info)
	@echo
	@echo Creating Rover Tray Core FW
	$(CC) -DTRAY_FW $(STANDARD_BUILD_OPTIONS) $@ $^
	@echo
	@echo Build done, Make version $(VERSION)
	@echo $(shell date)

$(BAG_TARGET) : $(BAG_SRC_FILES)
	$(git_info)
	@echo
	@echo Creating Rover Bag Core FW
	$(CC) -DBAG_FW $(STANDARD_BUILD_OPTIONS) $@ $^
	@echo
	@echo Build done, Make version $(VERSION)
	@echo $(shell date)

$(BAG2_TARGET) : $(BAG2_SRC_FILES)
	$(git_info)
	@echo
	@echo Creating Rover Bag2 Core FW
	$(CC) -DBAG2_FW $(STANDARD_BUILD_OPTIONS) $@ $^ 
	@echo
	@echo Build done, Make version $(VERSION)
	@echo $(shell date)

$(TRAY_DEBUG): $(TRAY_SRC_FILES)
	@echo
	@echo Creating Rover Tray Core FW
	$(CC) -g -DTRAY_FW $(DEBUG_BUILD_OPTIONS) $@ $^
	@echo
	@echo Build done, Make version $(VERSION)
	@echo $(shell date)

$(BAG_DEBUG) : $(BAG_SRC_FILES)
	@echo
	@echo Creating Rover Bag Core FW
	$(CC) -g -DBAG_FW $(DEBUG_BUILD_OPTIONS) $@ $^
	@echo
	@echo Build done, Make version $(VERSION)
	@echo $(shell date)

$(BAG2_DEBUG) : $(BAG2_SRC_FILES)
	@echo
	@echo Creating Rover Bag2 Core FW
	$(CC) -g -D_GLIBCXX_DEBUG -DBAG2_FW $(DEBUG_BUILD_OPTIONS)  $@ $^
	@echo
	@echo Build done, Make version $(VERSION)
	@echo $(shell date)

$(BAG_SOLO) : $(BAG_SRC_FILES)
	@echo
	@echo Creating Rover Bag Core FW
	$(CC) -DBAG_FW -DSOLO $(STANDARD_BUILD_OPTIONS)  $@ $^
	@echo
	@echo Build done, Make version $(VERSION)
	@echo $(shell date)

$(TAG_DEBUG) : $(TAG_SRC_FILES)
	@echo
	@echo Creating TAG_DEBUG FW
	g++ -DBAG2_FW -DTAG_DEBUG $(STANDARD_BUILD_OPTIONS)  $@ $^
	@echo
	@echo Build done, Make version $(VERSION)
	@echo $(shell date)

$(BUILDDIR)/$(TRAY_TARGET) : $(TRAY_SRC_FILES)
	@echo
	@echo mkdir -p $(BUILDDIR)
	@echo Creating Rover Tray Core FW
	$(CC) -g -DTRAY_FW $(STANDARD_BUILD_OPTIONS) $@ $^
	@echo
	@echo Build done, Make version $(VERSION)
	@echo $(shell date)

$(BUILDDIR)/$(BAG_TARGET) : $(BAG_SRC_FILES)
	@echo
	@echo mkdir -p $(BUILDDIR)
	@echo Creating Rover Bag Core FW
	$(CC) -DBAG_FW $(STANDARD_BUILD_OPTIONS) $@ $^
	@echo
	@echo Build done, Make version $(VERSION)
	@echo $(shell date)

$(BUILDDIR)/$(BAG2_TARGET) : $(BAG2_SRC_FILES)
	@echo
	@echo mkdir -p $(BUILDDIR)
	@echo Creating Rover Bag2 Core FW
	$(CC) -DBAG2_FW $(STANDARD_BUILD_OPTIONS) $@ $^
	@echo
	@echo Build done, Make version $(VERSION)
	@echo $(shell date)

# Build will compile the project using Docker.
.PHONY : build
build :
	docker build -f ./docker/Dockerfile.build -t $(IMAGE_NAME)/builder:latest .
	docker run --rm -v $(subst Fulfil.LFR.RoverFw,,$(PWD)):/src --workdir /src/Fulfil.LFR.RoverFw $(IMAGE_NAME)/builder $(BAG2_TARGET) STATIC=0 CROSS=1

# Debug will compile the debug targets using Docker.
.PHONY : debug
debug :
	docker build -f ./docker/Dockerfile.build -t $(IMAGE_NAME)/builder:latest .
	docker run --rm -v $(subst Fulfil.LFR.RoverFw,,$(PWD)):/src --workdir /src/Fulfil.LFR.RoverFw $(IMAGE_NAME)/builder $(BAG2_DEBUG) STATIC=0 CROSS=1

# Setup will configure a cross-architecture docker builder and QEMU emulator
# so we can build and run armhf containers.
.PHONY : setup
setup :
ifeq (,$(findstring $(DOCKER_BUILDER),$(BUILDERS)))
	docker buildx create --name $(DOCKER_BUILDER) --driver docker-container --use
	docker buildx inspect --bootstrap --builder $(DOCKER_BUILDER)
else
	@echo "Cross-arch docker builder already present"
endif
ifeq (,$(findstring arm/v7,$(EMULATORS)))
	docker run --rm --privileged tonistiigi/binfmt:latest --install all
else
	@echo "QEMU emulator for armhf already installed"
endif

# Base will build the runtime base image that the runtime image uses. This helps
# create a consistent image that rarely changes to make delta transfers more
# consistent.
.PHONY : base
base :
	@$(eval BUILDERS := $(shell docker buildx ls))
	@$(eval EMULATORS := $(shell docker run --privileged --rm tonistiigi/binfmt))
	@make setup BUILDERS='$(BUILDERS)' EMULATORS='$(EMULATORS)'
	docker buildx build -f docker/Dockerfile.base --platform linux/armhf -t $(IMAGE_NAME)/$(BAG2_TARGET)-base:latest -o type=docker ..

# Builds the docker images for all targets
# https://docs.docker.com/desktop/multi-arch/
.PHONY : image
image : build
	@$(eval BUILDERS := $(shell docker buildx ls))
	@$(eval EMULATORS := $(shell docker run --privileged --rm tonistiigi/binfmt))
	@make setup BUILDERS='$(BUILDERS)' EMULATORS='$(EMULATORS)'
# 	docker buildx build -f docker/Dockerfile --platform linux/armhf --build-arg TARGET=$(TRAY_TARGET) -t $(IMAGE_NAME)/$(TRAY_TARGET):$(IMAGE_TAG) ..
# 	docker buildx build -f docker/Dockerfile --platform linux/armhf --build-arg TARGET=$(BAG_TARGET) -t $(IMAGE_NAME)/$(BAG_TARGET):$(IMAGE_TAG) ..
	docker buildx build -f docker/Dockerfile --platform linux/armhf --build-arg TARGET=$(BAG2_TARGET) -t $(IMAGE_NAME)/$(BAG2_TARGET):$(IMAGE_TAG) -o type=docker .. 
# 	docker buildx build -f docker/Dockerfile --platform linux/armhf --build-arg TARGET=$(BAG2_DEBUG) -t $(IMAGE_NAME)/$(BAG2_TARGET):$(IMAGE_TAG)-debug -o type=docker .. 
ifdef GITHUB_TOKEN
# 	docker tag $(IMAGE_NAME)/$(TRAY_TARGET):$(IMAGE_TAG) $(IMAGE_NAME)/$(TRAY_TARGET):latest
# 	docker tag $(IMAGE_NAME)/$(BAG_TARGET):$(IMAGE_TAG) $(IMAGE_NAME)/$(BAG_TARGET):latest
	docker tag $(IMAGE_NAME)/$(BAG2_TARGET):$(IMAGE_TAG) $(IMAGE_NAME)/$(BAG2_TARGET):latest
# 	docker tag $(IMAGE_NAME)/$(BAG2_TARGET):$(IMAGE_TAG)-debug $(IMAGE_NAME)/$(BAG2_DEBUG):latest-debug
endif

# Pushes the docker images for all targets
.PHONY : push
push :
# 	docker push $(IMAGE_NAME)/$(TRAY_TARGET):$(IMAGE_TAG)
# 	docker push $(IMAGE_NAME)/$(BAG_TARGET):$(IMAGE_TAG)
	docker push $(IMAGE_NAME)/$(BAG2_TARGET):$(IMAGE_TAG)
# 	docker push $(IMAGE_NAME)/$(BAG2_TARGET):$(IMAGE_TAG)-debug
ifdef GITHUB_TOKEN
# 	docker push $(IMAGE_NAME)/$(TRAY_TARGET):latest
# 	docker push $(IMAGE_NAME)/$(BAG_TARGET):latest
	docker push $(IMAGE_NAME)/$(BAG2_TARGET):latest
# 	docker push $(IMAGE_NAME)/$(BAG2_TARGET):latest-debug
endif

# Pushes the docker base image
.PHONY : push-base
push-base :
	docker push $(IMAGE_NAME)/$(BAG2_TARGET)-base:latest

# Test will run tests against the project to ensure there are no errors.
.PHONY : test
test : build

# Deploy will build, image, push and deploy to the given target IP
.PHONY : deploy
deploy : check-deploy-env
	@echo "Pruning docker cache..."
	@ssh fulfil@$(DEPLOY_TARGET) "docker system prune -f"	
	@echo "Pulling down latest image of tag: $(IMAGE_TAG)..."
	ssh fulfil@$(DEPLOY_TARGET) "docker pull gcr.io/fulfil-fulfillment/lfr/rover-bag2-core:$(IMAGE_TAG)"
	@echo "Cycling docker container..."
	@ssh -t fulfil@$(DEPLOY_TARGET) "sudo sed -i '1s/.*/IMAGE_TAG=$(IMAGE_TAG)/' /home/usrFtp/RoverEnvironment.env && docker compose -f /home/usrFtp/code/docker-compose.yml down && IMAGE_TAG=$(IMAGE_TAG) docker compose -f /home/usrFtp/code/docker-compose.yml up -d"
	@echo "System is running:"	 
	ssh fulfil@$(DEPLOY_TARGET) "docker ps"

# Deploy-k8s will build, image, push, and deploy to the given target IP running Kubernetes.
.PHONY : deploy-k8s
deploy-k8s : check-deploy-env
	@echo "Pruning docker cache..."
	@ssh fulfil@$(DEPLOY_TARGET) "docker system prune -f | ccze -A"
	@echo "Pulling down latest image of tag: $(IMAGE_TAG)..."
	ssh fulfil@$(DEPLOY_TARGET) "docker pull gcr.io/fulfil-fulfillment/lfr/rover-bag2-core:$(IMAGE_TAG)"
	@echo "Cycling docker container..."
	@ssh -t fulfil@$(DEPLOY_TARGET) "sudo sed -i 's/image: gcr.io\/fulfil-fulfillment\/lfr\/rover-bag2-core:.*/image: gcr.io\/fulfil-fulfillment\/lfr\/rover-bag2-core:$(IMAGE_TAG)/g' /home/usrFtp/deployment.yaml && kubectl apply -f /home/usrFtp/deployment.yaml"
	@echo "Waiting for pod to become available"
	@ssh fulfil@$(DEPLOY_TARGET) "kubectl wait deploy/rover-bag2-core --for condition=available --timeout=300s | ccze -A"
	@echo "System is running:"	 
	@ssh fulfil@$(DEPLOY_TARGET) "kubectl get pods | ccze -A"

# Enables running the firmware via Kubernetes container
.PHONY : k8s-enable
k8s-enable : check-deploy-env
	@echo "Scaling up firmware deployment to 1"
	@ssh fulfil@$(DEPLOY_TARGET) "kubectl scale deployment rover-bag2-core --replicas=1"
	@echo "Waiting for pod to become available"
	@ssh fulfil@$(DEPLOY_TARGET) "kubectl wait deploy/rover-bag2-core --for condition=available --timeout=300s | ccze -A"
	@echo "Firmware status in Kubernetes:"	 
	@ssh fulfil@$(DEPLOY_TARGET) "kubectl get pods | ccze -A"

# Disables running the firmware via Kubernetes container
.PHONY : k8s-disable
k8s-disable : check-deploy-env
	@echo "Scaling firmware deployment to 0"
	@ssh fulfil@$(DEPLOY_TARGET) "kubectl scale deployment rover-bag2-core --replicas=0"
	@echo "Firmware status in Kubernetes:"	 
	@ssh fulfil@$(DEPLOY_TARGET) "kubectl get pods | ccze -A"

# Release will run semantic-release to auto-increment our version based on
# commit history, tag the release, and write a changelog.
.PHONY: release
release:
	npx semantic-release-plus

# Logs in to our Docker registry to allow pushing images.
.PHONY : login
login : check-login-env
	@echo "$${GCLOUD_SERVICE_KEY}" | docker login -u _json_key --password-stdin https://gcr.io

# Ensures that the GCLOUD_SERVICE_KEY environment variable is defined so we
# can log in to our Docker registry.
.PHONY : check-login-env
check-login-env :
ifndef GCLOUD_SERVICE_KEY
	$(error GCLOUD_SERVICE_KEY is undefined. Generate a Google Cloud access token and \
		export it in your shell before logging in. [Refer to \
		https://cloud.google.com/container-registry/docs/advanced-authentication])
endif

# Ensures that the DEPLOY_TARGET environment variable is defined so we
# can deploy a container to the given target
.PHONY : check-deploy-env
check-deploy-env :
ifndef DEPLOY_TARGET
	$(error DEPLOY_TARGET is undefined. Please re-run with 'make deploy DEPLOY_TARGET=192.168.x.x')
endif


#-static
.PHONY : clean all
clean :
	rm -f rover-*core bag-*debug tray-*debug bag-solo dist-debug elc-debug
	@echo All done
