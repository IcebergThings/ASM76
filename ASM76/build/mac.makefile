#==============================================================================
# ■ ASM76 - mac.makefile
#==============================================================================

TARGET = libASM76.dylib

include ../inc.makefile
include ../routine.makefile

LDFLAGS += -shared
CXXFLAGS += -I.. -fPIC

test: VMtest.cxx $(TARGET)
	$(CXX) -c VMtest.cxx -o VMtest.o $(CXXFLAGS)
	$(CCLD) -flto VMtest.o ./libASM76.dylib -lstdc++ -m -o VMtest
	./VMtest

VMenv: VMexec.cxx $(TARGET)
	$(CXX) -c VMexec.cxx -o VMexec.o $(CXXFLAGS)
	$(CCLD) VMexec.o ./libASM76.so -lstdc++ -flto -lm -o VMexec

clean:
	rm -f $(OBJECTS) $(TARGET) VMtest

.PHONY: test
