V = 0
ifneq ($(V), 0)
CC := gcc
RM := rm -f
else
CC := @gcc
RM := @rm -f
endif

CFLAGS := -Wall -O2 -MMD

TARGET := cw-trainer

OBJS := \
alsa.o \
morse.o \
symbols.o \
sym-queue.o \
threads.o \
tty.o \

DEPS := ${OBJS:.o=.d}
LIBS := -lasound -lm -lpthread
CLEANUP := $(TARGET)

.PHONY: all clean

all:	$(TARGET)

-include $(DEPS)

%.o: %.c
	@echo [CC] $@
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	@echo [LD] $@
	$(CC) -o $(TARGET) $(OBJS) $(LIBS)

clean:
	$(RM) $(OBJS) $(DEPS) $(CLEANUP)
