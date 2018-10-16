#==============================================================================
# ■ ASM76 - mac.makefile
#==============================================================================

TARGET = libASM76.dylib

MAKE76_ROOT?=.
include $(MAKE76_ROOT)/inc.makefile
include $(MAKE76_ROOT)/routine.makefile

LDFLAGS += -shared -flto=thin
CXXFLAGS += -I.. -fPIC -Ofast

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
