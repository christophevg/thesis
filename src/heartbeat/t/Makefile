LIBS    = -lm
CC      = clang
CFLAGS  = -g -Wall
LDFLAGS = -lssl -lcrypto

all: 01_heartbeat
	./01_heartbeat

01_heartbeat: 01_heartbeat.o ../heartbeat.o ../../sha1/sha1.o
	$(CC) $(LDFLAGS) $^ -Wall $(LIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@-rm -f 01_heartbeat.o ../heartbeat.o ../../sha1/sha1.o
	@-rm -f $(TARGET)

.PHONY: default all clean
.PRECIOUS: $(TARGET) $(OBJECTS)
