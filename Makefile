# Build untuk Suspicious Account Detector (C++17).
#   make          build untuk OS saat ini
#   make win      cross-compile menjadi .exe dari Linux/macOS
#   make run      build lalu jalankan
#   make debug    build dengan -g dan tanpa optimasi
#   make clean    hapus hasil build

TARGET_OS ?= native
STD       := -std=c++17
WARN      := -Wall -Wextra -Wpedantic
CXXFLAGS  ?= -O2
CPPFLAGS  += -Isrc -MMD -MP

# Konfigurasi Cross-Compile ke Windows dari Linux/macOS
ifeq ($(TARGET_OS),windows)
    CXX      := x86_64-w64-mingw32-g++
    EXE      := .exe
    LDFLAGS  += -static-libgcc -static-libstdc++ -static
    BUILD    := build-win
else
    CXX      ?= g++
    BUILD    := build
    ifeq ($(OS),Windows_NT)
        EXE  := .exe
    else
        EXE  :=
    endif
endif

# Abstraksi Perintah Shell (CMD vs Unix)
ifeq ($(OS),Windows_NT)
    ifeq ($(SHELL),sh.exe)
        MKDIR = mkdir -p $(1)
        RM    = rm -rf $(1)
        RUN   = ./$(1)
    else
        MKDIR = if not exist "$(subst /,\,$(1))" mkdir "$(subst /,\,$(1))"
        RM    = if exist "$(subst /,\,$(1))" rmdir /s /q "$(subst /,\,$(1))" 2>nul || del /f /q "$(subst /,\,$(1))" 2>nul
        RUN   = $(subst /,\,$(1))
    endif
else
    MKDIR = mkdir -p $(1)
    RM    = rm -rf $(1)
    RUN   = ./$(1)
endif

TARGET   := detector$(EXE)

SRC_LIB  := $(wildcard src/*.cpp)
OBJ_LIB  := $(SRC_LIB:%.cpp=$(BUILD)/%.o)
OBJ_MAIN := $(BUILD)/main.o

.PHONY: all win run debug clean

all: $(TARGET)

# Target khusus cross-compile dari Linux ke Windows (.exe)
win:
	$(MAKE) TARGET_OS=windows all

$(TARGET): $(OBJ_MAIN) $(OBJ_LIB)
	$(CXX) $(STD) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD)/%.o: %.cpp
	@$(call MKDIR,$(dir $@))
	$(CXX) $(STD) $(WARN) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

run: $(TARGET)
	$(call RUN,$(TARGET))

debug: CXXFLAGS := -g -O0
debug: clean $(TARGET)

clean:
	@$(call RM,build)
	@$(call RM,build-win)
	@$(call RM,detector)
	@$(call RM,detector.exe)

-include $(OBJ_MAIN:.o=.d) $(OBJ_LIB:.o=.d)