CONFIG_MODULE_SIG=n
OUT_NAME=hook
obj-m += $(OUT_NAME).o
CXX=g++

OUT_PATH=../bin/
SRC_PATH=./
OBJ_PATH=../obj/

DEBUG_PATH=debug/
RELEASE_PATH=release/

SRC_FILE=$(wildcard $(SRC_PATH) *.cpp)
SRC_FILE_C=$(wildcard $(SRC_PATH) *.c)

ALL_FILE=$(notdir $(SRC_FILE))
ALL_FILE_C=$(notdir $(SRC_FILE_C))

OBJ_FILE=$(patsubst %.cpp,%.o,$(ALL_FILE))
OBJ_FILE_C=$(patsubst %.c,%.o,$(ALL_FILE_C))

DEBUG_FLAGS=-g -mcmodel=kernel -fpermissive -fno-builtin -fno-exceptions -fno-rtti -fno-pic -nostdinc -nostartfiles -DCLANG_DEFAULT_CXX_STDLIB=libc++ -lstdc++
RELEASE_FLAGS=-mcmodel=kernel -fpermissive -fno-builtin -fno-exceptions -fno-rtti -fno-pic -nostdinc -nostartfiles -DCLANG_DEFAULT_CXX_STDLIB=libc++ -lstdc++

OUT_DEBUG_FILE=$(OUT_PATH)$(DEBUG_PATH)$(OUT_NAME)
OUT_RELEASE_FILE=$(OUT_PATH)$(RELEASE_PATH)$(OUT_NAME)

OBJ_RELEASE_FILE=$(addprefix $(OBJ_PATH)$(RELEASE_PATH),$(notdir $(OBJ_FILE)))
OBJ_DEBUG_FILE=$(addprefix $(OBJ_PATH)$(DEBUG_PATH),$(notdir $(OBJ_FILE)))

$(OUT_NAME)-objs := invert.o ipHandle.o kernel_api.o ../obj/debug/memorypool.o ../obj/debug/cppmod.o ../obj/debug/unlockQueue.o
#$(OUT_NAME)-objs := $(OBJ_FILE_C) $(OBJ_DEBUG_FILE)

all: debug release 

debug: pre_debug $(OBJ_DEBUG_FILE) debug_module

release: pre_release $(OBJ_RELEASE_FILE) release_module

pre_debug:
	-$(shell mkdir $(OBJ_PATH) -p)
	-$(shell mkdir $(OBJ_PATH)$(DEBUG_PATH) -p)
	-$(shell mkdir $(OUT_PATH)$(DEBUG_PATH) -p)
	-$(shell mkdir $(INCLUDE_PATH) -p)
	-$(shell mkdir $(OUT_PATH)$(DEBUG_PATH) -p)
	
$(OBJ_PATH)$(DEBUG_PATH)%.o : %.cpp
	$(CXX) -c $(DEBUG_FLAGS) $(INC_PATH) $(SRC_PATH)$< -o $@

debug_module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) CFLAGS_EXTRA="-g" modules
	mv $(OBJ_FILE_C) $(OBJ_PATH)$(DEBUG_PATH)
	mv $(OUT_NAME).ko $(OUT_PATH)$(DEBUG_PATH)


pre_release:
	-$(shell mkdir $(OBJ_PATH) -p)
	-$(shell mkdir $(OBJ_PATH)$(RELEASE_PATH) -p)
	-$(shell mkdir $(OUT_PATH)$(RELEASE_PATH) -p)
	-$(shell mkdir $(OUT_PATH)$(RELEASE_PATH) -p)

$(OBJ_PATH)$(RELEASE_PATH)%.o : %.cpp
	$(CXX) -c $(RELEASE_FLAGS) $(INC_PATH) $(SRC_PATH)$< -o $@

release_module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) CFLAGS_EXTRA="-O2" modules
	mv $(OBJ_FILE_C) $(OBJ_PATH)$(RELEASE_PATH)
	mv $(OUT_NAME).ko $(OUT_PATH)$(RELEASE_PATH)
  
clean:
	@echo " "
	@echo "make clean"
	@echo " "
	$(shell rm $(OBJ_PATH) -rf)
	$(shell rm $(OUT_PATH)$(DEBUG_PATH)$(OUT_NAME) -f)
	$(shell rm $(OUT_PATH)$(RELEASE_PATH)$(OUT_NAME) -f)
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

compile:
	@echo $(shell rmmod Queue)
	@echo $(shell dmesg -c)
	@echo $(shell make)
run:
	@echo $(shell insmod Queue.ko)
