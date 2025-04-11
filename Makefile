CC = gcc
CFLAGS = -Wall -O2
SRC = Mandalay.c
OBJ = Mandalay.o
TARGET = Mandalay

ALLEGRO_LIBS = -lalleg -lm -ldl -lpthread

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJ) $(ALLEGRO_LIBS)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC)

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
