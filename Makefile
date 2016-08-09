CXX_FLAGS   := -std=c++11 -Os -Wall
CC_FLAGS    := -c -Os -Wall
LD_FLAGS    :=
SRC_DIR     := src
OBJ_DIR     := .build
BIN_DIR     := bin
BINARY      := $(BIN_DIR)/superfamicheck

rwildcard   = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

SRC         := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/**/*.cpp)
OBJ         := $(patsubst $(SRC_DIR)%,$(OBJ_DIR)%,$(patsubst %.cpp,%.o,$(SRC)))

.PHONY: clean

default: $(BINARY)

all: clean $(BINARY)

$(BINARY) : $(OBJ)
	@mkdir -pv $(dir $@)
	g++ -v $(LD_FLAGS) $^ -o $@

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	@mkdir -pv $(dir $@)
	g++ $(CXX_FLAGS) -c $< -o $@

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	@mkdir -pv $(dir $@)
	gcc $(CC_FLAGS) -c $< -o $@

clean:
	@rm -rf $(BIN_DIR) $(OBJ_DIR)
