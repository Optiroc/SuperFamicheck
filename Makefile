SRC_DIR   := src
BIN_DIR   := bin
OBJ_DIR   := .build

CXX       ?= g++

CXX_FLAGS := -std=c++20 -Wall -Wextra
LD_FLAGS  :=
TARGET    :=

ifeq ($(DEBUG), 1)
  CXX_FLAGS += -O0 -g
else
  CXX_FLAGS += -O3 -flto
endif

OBJ       := $(OBJ_DIR)/sfcRom.o $(OBJ_DIR)/superfamicheck.o
HEADERS   := $(wildcard $(SRC_DIR)/*.h)

.PHONY: clean

default: superfamicheck

all: clean superfamicheck

superfamicheck: $(BIN_DIR)/superfamicheck

$(BIN_DIR)/superfamicheck : $(OBJ) | $(BIN_DIR)
	$(CXX) $(LD_FLAGS) $(TARGET) $^ -o $@

$(OBJ_DIR)/%.o : ./**/%.cpp $(HEADERS) | $(OBJ_DIR)
	$(CXX) $(CXX_FLAGS) $(TARGET) -c $< -o $@

$(BIN_DIR):
	@mkdir -pv $@

$(OBJ_DIR):
	@mkdir -pv $@

clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
