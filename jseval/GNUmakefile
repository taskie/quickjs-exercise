PROGRAM := jseval

include ../config.mk

OBJS := $(PROGRAM).o
LIBS := -l quickjs -l m

$(PROGRAM): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $<

.PHONY: run

run: $(PROGRAM)
	./$(PROGRAM) '3**2 + 4**2'
	-./$(PROGRAM) foo
	./$(PROGRAM) 'undefined'
	./$(PROGRAM) '[3, 4, 5].map(x => x ** 10).forEach(x => console.log(x))'
