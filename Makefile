CC=gcc

SRCS=$(wildcard *.cpp *.c)
TOBJS= $(patsubst %.cpp,%.o,$(SRCS))
OBJS= $(patsubst %.c,%.o,$(TOBJS))


TARGET=server
all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $@ $^


clean:
	@rm *.o $(TARGET) -f
