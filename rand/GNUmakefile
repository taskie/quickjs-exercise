PROGRAM := rand

include ../config.mk

OBJS := $(PROGRAM).o
LIBS := -l quickjs -l m

$(PROGRAM): $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

%.o: %.cc
	$(CXX) -c $(CXXFLAGS) $<

.PHONY: run

run: $(PROGRAM)
	./$(PROGRAM) index.js
