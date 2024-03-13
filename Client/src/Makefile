
OUT_NAME=client
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

DEBUG_FLAGS=-g -ggdb3 -Wall -fPIC -Wl,-rpath='./' --std=c++11
RELEASE_FLAGS=-O2 -fPIC -Wl,-rpath='./' --std=c++11

OUT_DEBUG_FILE=$(OUT_PATH)$(DEBUG_PATH)$(OUT_NAME)
OUT_RELEASE_FILE=$(OUT_PATH)$(RELEASE_PATH)$(OUT_NAME)

OBJ_RELEASE_FILE=$(addprefix $(OBJ_PATH)$(RELEASE_PATH),$(notdir $(OBJ_FILE)))
OBJ_DEBUG_FILE=$(addprefix $(OBJ_PATH)$(DEBUG_PATH),$(notdir $(OBJ_FILE)))

LIB_DEBUG_FILE=
LIB_RELEASE_FILE=

all: debug release 

debug: pre_debug $(OUT_DEBUG_FILE) 

release: pre_release $(OUT_RELEASE_FILE) 

pre_debug:
	-$(shell mkdir $(OBJ_PATH) -p)
	-$(shell mkdir $(OBJ_PATH)$(DEBUG_PATH) -p)
	-$(shell mkdir $(OUT_PATH)$(DEBUG_PATH) -p)
	-$(shell mkdir $(OUT_PATH)$(DEBUG_PATH) -p)

$(OUT_DEBUG_FILE) : $(OBJ_DEBUG_FILE)
	$(CXX) $(DEBUG_FLAGS) $(addprefix $(OBJ_PATH)$(DEBUG_PATH),$(notdir $^)) -o $@ $(LIB_DEBUG_FILE)

$(OUT_RELEASE_FILE) : $(OBJ_RELEASE_FILE)
	$(CXX) $(RELEASE_FLAGS) $(addprefix $(OBJ_PATH)$(RELEASE_PATH),$(notdir $^)) -o $@ $(LIB_RELEASE_FILE)

$(OBJ_PATH)$(DEBUG_PATH)%.o : %.cpp
	$(CXX) -c $(DEBUG_FLAGS) $(INC_PATH) $(SRC_PATH)$< -o $@

$(OBJ_PATH)$(RELEASE_PATH)%.o : %.cpp
	$(CXX) -c $(RELEASE_FLAGS) $(INC_PATH) $(SRC_PATH)$< -o $@

pre_release:
	-$(shell mkdir $(OBJ_PATH) -p)
	-$(shell mkdir $(OBJ_PATH)$(RELEASE_PATH) -p)
	-$(shell mkdir $(OUT_PATH)$(RELEASE_PATH) -p)
	-$(shell mkdir $(OUT_PATH)$(RELEASE_PATH) -p)
  
clean:
	@echo " "
	@echo "make clean"
	@echo " "
	$(shell rm $(OBJ_PATH) -rf)
	$(shell rm $(OUT_PATH)$(DEBUG_PATH)$(OUT_NAME) -f)
	$(shell rm $(OUT_PATH)$(RELEASE_PATH)$(OUT_NAME) -f)
