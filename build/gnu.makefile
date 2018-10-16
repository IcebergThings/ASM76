#==============================================================================
# ■ ASM76 - gnu.makefile
#==============================================================================

TARGET = libASM76.so

MAKE76_ROOT?=.
include $(MAKE76_ROOT)/inc.makefile
include $(MAKE76_ROOT)/routine.makefile

LDFLAGS += -shared
CXXFLAGS += -I.. -fPIC

test: VMtest.cxx $(TARGET)
	$(CXX) -c VMtest.cxx -o VMtest.o $(CXXFLAGS)
	$(CCLD) VMtest.o ./libASM76.so -lstdc++ -flto -lm -o VMtest
	./VMtest

VMenv: VMexec.cxx VMc.cxx $(TARGET)
	$(CXX) -c VMexec.cxx -o VMexec.o $(CXXFLAGS)
	$(CCLD) VMexec.o ./libASM76.so -lstdc++ -flto -lm -o VMexec
	$(CXX) -c VMc.cxx -o VMc.o $(CXXFLAGS)
	$(CCLD) VMc.o ./libASM76.so -lstdc++ -flto -lm -o VMc

clean:
	rm -f $(OBJECTS) $(TARGET) VMtest

.PHONY: test
