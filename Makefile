GEM5_PATH = gem5
ISA = x86
CC = gcc
TARGET = amx-workloads/load_test

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) -o $(TARGET) $(TARGET).c -I$(GEM5_PATH)/include -L$(GEM5_PATH)/util/m5/build/$(ISA)/out -lm5

clean:
	rm -f $(TARGET)