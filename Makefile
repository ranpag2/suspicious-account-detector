# Build untuk Suspicious Account Detector (C++17).
#   make        build program
#   make run    build lalu jalankan
#   make test   build dan jalankan tests/test_invarian.cpp
#   make debug  build dengan -g dan tanpa optimasi
#   make clean  hapus hasil build

CXX      ?= g++
STD      := -std=c++17
WARN     := -Wall -Wextra -Wpedantic
CXXFLAGS ?= -O2
CPPFLAGS += -Isrc -MMD -MP

ifeq ($(OS),Windows_NT)
EXE := .exe
endif

TARGET      := detector$(EXE)
TEST_TARGET := test_invarian$(EXE)
BUILD       := build

# Semua .cpp di src/ ikut otomatis, jadi file baru tidak perlu didaftarkan.
SRC_LIB  := $(wildcard src/*.cpp)
OBJ_LIB  := $(SRC_LIB:%.cpp=$(BUILD)/%.o)
OBJ_MAIN := $(BUILD)/main.o
SRC_TEST := $(wildcard tests/*.cpp)
OBJ_TEST := $(SRC_TEST:%.cpp=$(BUILD)/%.o)

.PHONY: all run test debug clean

all: $(TARGET)

$(TARGET): $(OBJ_MAIN) $(OBJ_LIB)
	$(CXX) $(STD) $(CXXFLAGS) $^ -o $@

$(TEST_TARGET): $(OBJ_TEST) $(OBJ_LIB)
	$(CXX) $(STD) $(CXXFLAGS) $^ -o $@

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(STD) $(WARN) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

debug: CXXFLAGS := -g -O0
debug: clean $(TARGET)

clean:
	rm -rf $(BUILD) $(TARGET) $(TEST_TARGET)

# Dependensi header otomatis (dibuat oleh -MMD -MP).
-include $(OBJ_MAIN:.o=.d) $(OBJ_LIB:.o=.d) $(OBJ_TEST:.o=.d)
