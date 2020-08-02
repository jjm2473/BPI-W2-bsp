
RM = rm -f
HOSTCC = gcc
COMMON_SRC = common.c
COMMON_OBJS = $(patsubst %.c,%.o,$(COMMON_SRC))
TARGET := onchip

.PHONY: all clean
all: $(TARGET)

clean:
	$(RM) *.o $(TARGET)

%.o: %.cpp
	$(HOSTCC) -c $< $(CFLAGS) -o $@

%.o: %.c
	$(HOSTCC) -c $< $(CFLAGS) -o $@

$(TARGET): %: %.o $(COMMON_OBJS)
	$(HOSTCC) -o $@ $^ 
	chmod +x $@

