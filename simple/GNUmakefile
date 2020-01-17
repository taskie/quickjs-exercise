PROGRAM := simple

include ../config.mk

OBJS := $(PROGRAM).o
LIBS := -l quickjs -l m

$(PROGRAM): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $<

.PHONY: run

run: $(PROGRAM)
	./$(PROGRAM)
