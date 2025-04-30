CXX = g++
CXXFLAGS = -Wall -Wextra -O3 -pthread
TARGET = prime_finder
SRC = src/main.cpp
OUT_DIR = out
SHARED_DIR = shared

all: clean $(OUT_DIR)/$(TARGET).exe copy_shared

$(OUT_DIR):
	if not exist $(OUT_DIR) mkdir $(OUT_DIR)

$(OUT_DIR)/$(TARGET).exe: $(SRC) | $(OUT_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

copy_shared: $(OUT_DIR)
	if exist $(SHARED_DIR) xcopy /E /I /Y $(SHARED_DIR)\* $(OUT_DIR)\

run:
	cd $(OUT_DIR) && $(TARGET).exe

measure:
	cd $(OUT_DIR) && powershell Measure-Command {start-process $(TARGET).exe -Wait}

clean:
	if exist $(OUT_DIR) rmdir /S /Q $(OUT_DIR)

.PHONY: all clean build copy_shared 