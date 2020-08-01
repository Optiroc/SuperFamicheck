SRC_DIR   := src
BIN_DIR   := bin
OBJ_DIR   := .build

CC        ?= gcc
CXX       ?= g++

CXX_FLAGS := -std=gnu++11 -Wall
CC_FLAGS  := -std=gnu99 -Wall
LD_FLAGS  :=

ifeq ($(DEBUG), 1)
  CXX_FLAGS += -O0 -g
  CC_FLAGS += -O0 -g
else
  CXX_FLAGS += -O3 -flto
  CC_FLAGS += -O3 -flto
endif

OBJ       := $(OBJ_DIR)/sfcRom.o $(OBJ_DIR)/superfamicheck.o
HEADERS   := $(wildcard $(SRC_DIR)/*.h)

.PHONY: clean

default: superfamicheck

all: clean superfamicheck

superfamicheck: $(BIN_DIR)/superfamicheck

$(BIN_DIR)/superfamicheck : $(OBJ) | $(BIN_DIR)
	$(CXX) $(LD_FLAGS) $^ -o $@

$(OBJ_DIR)/%.o : ./**/%.cpp $(HEADERS) | $(OBJ_DIR)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(OBJ_DIR)/%.o : ./**/%.c $(HEADERS) | $(OBJ_DIR)
	$(CC) $(CC_FLAGS) -c $< -o $@

$(BIN_DIR):
	@mkdir -pv $@

$(OBJ_DIR):
	@mkdir -pv $@

clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
