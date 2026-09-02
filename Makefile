
CC = gcc
CFLAGS = -O2 -Wall -Wextra

TARGET = BOX
SRC = rescuebox.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

install:
	mkdir -p bin
	cp $(TARGET) .

.PHONY: all clean install

