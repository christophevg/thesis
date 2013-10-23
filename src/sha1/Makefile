TARGET  = test
LIBS    = -lm
CC      = clang
CFLAGS  = -g -Wall
LDFLAGS = -lssl -lcrypto

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

default: $(TARGET)
	./$(TARGET)

all: default

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)

.PHONY: default all clean
.PRECIOUS: $(TARGET) $(OBJECTS)
