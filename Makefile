CXX_FLAGS   := -std=c++11 -Os -Wall
CC_FLAGS    := -c -Os -Wall
LD_FLAGS    :=
SRC_DIR     := src
BUILD_DIR   := build
OBJ_DIR     := $(BUILD_DIR)/o
BINARY      := superfamicheck

rwildcard   = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

SRC         := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/**/*.cpp)
OBJ         := $(patsubst $(SRC_DIR)%,$(OBJ_DIR)%,$(patsubst %.cpp,%.o,$(SRC)))

.PHONY: clean

all : $(BINARY)

clean:
	rm -f $(BINARY) $(OBJ)

$(BINARY) : $(OBJ)
	g++ -v $(LD_FLAGS) $^ -o $@

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	@mkdir -pv $(dir $@)
	g++ $(CXX_FLAGS) -c $< -o $@

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	@mkdir -pv $(dir $@)
	gcc $(CC_FLAGS) -c $< -o $@
