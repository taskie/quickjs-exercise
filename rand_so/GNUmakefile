PROGRAM := rand
SO := lib$(PROGRAM).so

include ../config.mk

OBJS := $(PROGRAM).o
LIBS :=

$(SO): $(OBJS)
	$(CXX) -shared $(LDFLAGS) $(OBJS) $(LIBS) -o $@

%.o: %.cc
	$(CXX) -c -fPIC $(CXXFLAGS) $<

.PHONY: run

run: $(SO)
	qjs index.js
